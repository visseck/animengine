#include "pipe_server.h"
#include "animcore/containers/array.h"
#include "animcore/serialization/serialization.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

ANIM_NAMESPACE_BEGIN

static constexpr size_t Buffer_Size = 4096;
static constexpr uint32_t Pipe_Timeout = 5000;

class PipeServer : public IPipeServer
{
public:
	PipeServer();
	~PipeServer();
	virtual DispatchCallback SetDispatchCallback(DispatchCallback callback) override;
	virtual void RunServer(const char* pipeName, size_t numInstances) override;
private:
	bool ConnectToNewClient(size_t pipeIndex);
	void DisconnectAndReconnect(size_t pipeIndex);
	size_t GetAnswerToRequest(size_t pipeIndex);

	std::string pipeName_;
	DispatchCallback dispatchCallback_;

	enum class PipeInstanceState
	{
		CONNECTING,
		READING,
		WRITING
	};
	struct PipeInstance
	{
		OVERLAPPED overLapped_;
		HANDLE pipeInstanceHandle_;
		uint8_t inputBuffer_[Buffer_Size];
		uint8_t outputBuffer_[Buffer_Size];
		DWORD numBytesToRead_;
		DWORD numBytesToWrite_;
		PipeInstanceState pipeState_;
		BOOL pendingIO_;
	};
	Array<PipeInstance> pipeInstances_;
	Array<HANDLE> pipeEvents_;
	bool shutdownServer_;
};

PipeServer::PipeServer()
	: shutdownServer_(false)
	, dispatchCallback_(nullptr)
{
}

IPipeServer::DispatchCallback PipeServer::SetDispatchCallback(IPipeServer::DispatchCallback callback)
{
	auto tmp = dispatchCallback_;
	dispatchCallback_ = callback;
	return tmp;
}

size_t PipeServer::GetAnswerToRequest(size_t pipeIndex)
{
	Message* msg = nullptr;
	{
		MemoryStream memStream(pipeInstances_[pipeIndex].inputBuffer_, Buffer_Size);
		Serialization::Deserializer deserializer(memStream);
		deserializer.deserialize(msg);
	}
	size_t numBytesWritten = 0;
	ASSETCACHE_ASSERT(msg != nullptr);
	auto responseMsg = dispatchCallback_(std::unique_ptr<Message>(msg));
	{
		MemoryStream memStream(pipeInstances_[pipeIndex].outputBuffer_, Buffer_Size);
		Serialization::Serializer serializer(memStream);
		serializer.serialize(responseMsg.get());
		numBytesWritten = memStream.getNumBytesWritten();
	}
	return numBytesWritten;
}

UniquePtr<IPipeServer> IPipeServer::CreateServer()
{
	return UniquePtr<PipeServer>::MakeUnique();
}

// returns false if already connected, true otherwise
bool PipeServer::ConnectToNewClient(size_t pipeIndex)
{
	BOOL isConnected = ConnectNamedPipe(pipeInstances_[pipeIndex].pipeInstanceHandle_, &pipeInstances_[pipeIndex].overLapped_);

	if (isConnected)
	{
		return false;
	}

	bool ioPending = false;
	switch (GetLastError())
	{
		// The overlapped connection in progress. 
	case ERROR_IO_PENDING:
		ioPending = true;
		break;

		// Client is already connected, so signal an event. 
	case ERROR_PIPE_CONNECTED:
		if (SetEvent(pipeEvents_[pipeIndex]))
			break;

		// If an error occurs during the connect operation... 
	default:
	{
		return false;
	}
	}
	return ioPending;
}

void PipeServer::DisconnectAndReconnect(size_t pipeIndex)
{
	BOOL result = DisconnectNamedPipe(pipeInstances_[pipeIndex].pipeInstanceHandle_);
	ANIM_ASSERT(result);

	pipeInstances_[pipeIndex].pendingIO_ = ConnectToNewClient(pipeIndex);

	pipeInstances_[pipeIndex].pipeState_ =
		pipeInstances_[pipeIndex].pendingIO_ ?
		PipeServer::PipeInstanceState::CONNECTING :
		PipeServer::PipeInstanceState::READING;
}

void PipeServer::RunServer(const char* pipeName, size_t numInstances)
{
	pipeName_ = pipeName;
	pipeInstances_.Resize(numInstances);
	pipeEvents_.Resize(numInstances);

	for (size_t i = 0; i < numInstances; ++i)
	{
		pipeEvents_[i] = CreateEvent(
			NULL,
			TRUE,
			TRUE,
			NULL);

		ANIM_ASSERT(pipeEvents_[i] != NULL);

		pipeInstances_[i].overLapped_.hEvent = pipeEvents_[i];
		pipeInstances_[i].pipeInstanceHandle_ = CreateNamedPipe(
			pipeName,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			static_cast<DWORD>(numInstances),
			Buffer_Size,
			Buffer_Size,
			Pipe_Timeout,
			NULL);
		ANIM_ASSERT(pipeInstances_[i].pipeInstanceHandle_ != INVALID_HANDLE_VALUE);

		pipeInstances_[i].pendingIO_ = ConnectToNewClient(i);
		pipeInstances_[i].pipeState_ = pipeInstances_[i].pendingIO_ ?
			PipeServer::PipeInstanceState::CONNECTING :
			PipeServer::PipeInstanceState::READING;
	}

	while (!shutdownServer_)
	{
		DWORD dwait = WaitForMultipleObjects(
			static_cast<DWORD>(numInstances),
			pipeEvents_.GetBuffer(),
			FALSE,
			INFINITE);

		size_t pipeIndex = dwait - WAIT_OBJECT_0;
		ANIM_ASSERT(pipeIndex >= 0 && pipeIndex < numInstances);

		if (pipeInstances_[pipeIndex].pendingIO_)
		{
			DWORD bytesTransferred;
			BOOL success = GetOverlappedResult(
				pipeInstances_[pipeIndex].pipeInstanceHandle_,
				&pipeInstances_[pipeIndex].overLapped_,
				&bytesTransferred, FALSE);

			switch (pipeInstances_[pipeIndex].pipeState_)
			{
			case PipeInstanceState::CONNECTING:
			{
				ANIM_ASSERT(success);
				pipeInstances_[pipeIndex].pipeState_ = PipeInstanceState::READING;
			}
			break;
			case PipeInstanceState::READING:
			{
				if (!success || bytesTransferred == 0)
				{
					DisconnectAndReconnect(pipeIndex);
					continue;
				}
				pipeInstances_[pipeIndex].numBytesToRead_ = bytesTransferred;
				pipeInstances_[pipeIndex].pipeState_ = PipeInstanceState::WRITING;
			}
			break;

			case PipeInstanceState::WRITING:
			{
				if (!success || bytesTransferred != pipeInstances_[pipeIndex].numBytesToWrite_)
				{
					DisconnectAndReconnect(pipeIndex);
					continue;
				}
				pipeInstances_[pipeIndex].pipeState_ = PipeInstanceState::READING;
			}
			break;
			default:
				ANIM_ASSERT(false);
			}
		}

		switch (pipeInstances_[pipeIndex].pipeState_)
		{
		case PipeInstanceState::READING:
		{
			BOOL success = ReadFile(
				pipeInstances_[pipeIndex].pipeInstanceHandle_,
				pipeInstances_[pipeIndex].inputBuffer_,
				Buffer_Size,
				&pipeInstances_[pipeIndex].numBytesToRead_,
				&pipeInstances_[pipeIndex].overLapped_);

			if (success && pipeInstances_[pipeIndex].numBytesToRead_ != 0)
			{
				pipeInstances_[pipeIndex].pendingIO_ = false;
				pipeInstances_[pipeIndex].pipeState_ = PipeInstanceState::WRITING;
				continue;
			}

			if (!success && (GetLastError() == ERROR_IO_PENDING))
			{
				pipeInstances_[pipeIndex].pendingIO_ = true;
				continue;
			}

			// Error
			DisconnectAndReconnect(pipeIndex);
		}
		break;

		case PipeInstanceState::WRITING:
		{
			pipeInstances_[pipeIndex].numBytesToWrite_ = (DWORD)GetAnswerToRequest(pipeIndex);

			DWORD bytesWritten;
			BOOL success = WriteFile(
				pipeInstances_[pipeIndex].pipeInstanceHandle_,
				pipeInstances_[pipeIndex].outputBuffer_,
				pipeInstances_[pipeIndex].numBytesToWrite_,
				&bytesWritten,
				&pipeInstances_[pipeIndex].overLapped_);

			if (success && bytesWritten == pipeInstances_[pipeIndex].numBytesToWrite_)
			{
				pipeInstances_[pipeIndex].pendingIO_ = false;
				pipeInstances_[pipeIndex].pipeState_ = PipeInstanceState::READING;
				continue;
			}

			if (!success && (GetLastError() == ERROR_IO_PENDING))
			{
				pipeInstances_[pipeIndex].pendingIO_ = true;
				continue;
			}

			DisconnectAndReconnect(pipeIndex);
		}
		break;

		default:
			ANIM_ASSERT(false);
		}
	}
}

PipeServer::~PipeServer()
{
	for (const auto& pipeInstance : pipeInstances_)
	{
		CloseHandle(pipeInstance.pipeInstanceHandle_);
	}
	for (const auto& handle : pipeEvents_)
	{
		CloseHandle(handle);
	}
}

ANIM_NAMESPACE_END
