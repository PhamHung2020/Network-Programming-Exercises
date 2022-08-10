#ifndef MESSAGE_SENDER_H
#define MESSAGE_SENDER_H

#include <iostream>
#include <string>
#include <winsock2.h>
#include "Helper.h"
#include "ResponseCode.h"

namespace MessageSender
{
	int sendUserMessage(SOCKET connSock, const std::string& username, char* buff);
	int sendPostMessage(SOCKET connSockPtr, const std::string& article, char* buff);
	int sendByeMessage(SOCKET connSockPtr, char* buff);
}

#endif // !MESSAGE_SENDER_H
