#pragma once
#include "animcore/util/namespace.h"
#include "animcore/memory/pointers.h"

ANIM_NAMESPACE_BEGIN

class Message;

class IPipeServer
{
public:
	IPipeServer(const IPipeServer&) = delete;
	IPipeServer& operator=(const IPipeServer&) = delete;
	virtual ~IPipeServer() {}

	typedef UniquePtr<Message>(*DispatchCallback)(UniquePtr<Message>);
	virtual DispatchCallback SetDispatchCallback(DispatchCallback callback) = 0;
	virtual void RunServer(const char* pipeName, size_t numInstances) = 0;
	static UniquePtr<IPipeServer> CreateServer();
protected:
	IPipeServer() {}
};

ANIM_NAMESPACE_END
