#pragma once
#include "animcore/util/namespace.h"
#include "animcore/memory/pointers.h"
ANIM_NAMESPACE_BEGIN

class Message;
class IPipeClient
{
public:
	IPipeClient(const IPipeClient&) = delete;
	IPipeClient& operator=(const IPipeClient&) = delete;
	virtual ~IPipeClient() {}

	static UniquePtr<IPipeClient> CreateClient();
	virtual Message* SendMessage(const Message* msgToSend) = 0;
	virtual bool ConnectToServer(const char* pipeName) = 0;
	virtual void DisconnectFromServer() = 0;
protected:
	IPipeClient() {}
};

ANIM_NAMESPACE_END