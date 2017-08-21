#include "pipe_client.h"
#include "animcore/serialization/serialization.h"
#include "animcore/remoteprotocol/message.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef SendMessage

ANIM_NAMESPACE_BEGIN

static constexpr size_t Buffer_Size = 4096;

class PipeClient : public IPipeClient
{
public:
	PipeClient();
	virtual ~PipeClient();
	virtual Message* SendMessage(const Message* msgToSend) override;
	virtual bool ConnectToServer(const char* pipeName) override;
	virtual void DisconnectFromServer() override;
private:
	HANDLE pipeInstanceHandle_;
	uint8_t inputBuffer[Buffer_Size];
	uint8_t outputBuffer[Buffer_Size];
};

PipeClient::PipeClient()
	: pipeInstanceHandle_(INVALID_HANDLE_VALUE)
{
}

UniquePtr<IPipeClient> IPipeClient::CreateClient()
{
	return UniquePtr<PipeClient>::MakeUnique();
}

bool PipeClient::ConnectToServer(const char* pipeName)
{
	while (1)
	{
		pipeInstanceHandle_ = CreateFile(
			pipeName,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (pipeInstanceHandle_ != INVALID_HANDLE_VALUE)
		{
			break;
		}

		if (GetLastError() != ERROR_PIPE_BUSY)
			return false;

		if (!WaitNamedPipe(pipeName, 500))
			return false;
	}

	DWORD pipeMode = PIPE_READMODE_MESSAGE;
	BOOL success = SetNamedPipeHandleState(
		pipeInstanceHandle_,
		&pipeMode,
		NULL,
		NULL);

	if (!success)
		return false;

	return true;
}

PipeClient::~PipeClient()
{
	DisconnectFromServer();
}

void PipeClient::DisconnectFromServer()
{
	CloseHandle(pipeInstanceHandle_);
	pipeInstanceHandle_ = INVALID_HANDLE_VALUE;
}

Message* PipeClient::SendMessage(const Message* msgToSend)
{
	DWORD numBytesWritten = 0;
	Message* response = nullptr;

	ANIM_ASSERT(msgToSend != nullptr);
	{
		MemoryStream memStream(outputBuffer, Buffer_Size);
		Serialization::Serializer res(memStream);
		res.serialize(msgToSend);
		numBytesWritten = static_cast<DWORD>(memStream.getNumBytesWritten());
	}

	BOOL success = WriteFile(
		pipeInstanceHandle_,
		outputBuffer,
		numBytesWritten,
		&numBytesWritten,
		NULL);

	do
	{
		success = ReadFile(
			pipeInstanceHandle_,
			inputBuffer,
			Buffer_Size,
			&numBytesWritten,
			NULL);

		if (!success && GetLastError() != ERROR_MORE_DATA)
			break;
	} while (!success);

	if (!success)
		return nullptr;

	{
		MemoryStream memStream(inputBuffer, Buffer_Size);
		Serialization::Deserializer res(memStream);
		res.deserialize(response);
	}
	return response;
}

ANIM_NAMESPACE_END
