#include "MessageSender.h"

int MessageSender::sendUserMessage(SOCKET connSock, const std::string& username, char* buff)
{
	std::string message = "USER " + username;
	return Helper::sendMessage(connSock, message, buff);
}

int MessageSender::sendPostMessage(SOCKET connSock, const std::string& article, char* buff)
{
	std::string message = "POST " + article;
	return Helper::sendMessage(connSock, message, buff);
}

int MessageSender::sendByeMessage(SOCKET connSock, char* buff)
{
	return Helper::sendMessage(connSock, "BYE", buff);
}