#ifndef MESSAGE_SENDER_H
#define MESSAGE_SENDER_H

#include <iostream>
#include <string>
#include <winsock2.h>
#include "Helper.h"
#include "ResponseCode.h"

namespace MessageSender
{
	/**
	* @function sendUserMessage: sends login message to server.
	*
	* @param connSockPtr: A socket that is connecting with server.
	* @param username: Username for login.
	* @param buff: Application's buffer.
	*
	* @return: number of bytes sent if no error occurs.
	*		   SOCKET_ERROR if an error occurs.
	**/
	int sendUserMessage(SOCKET connSock, const std::string& username, char* buff);

	/**
	* @function sendUserMessage: sends a new article to server.
	*
	* @param connSockPtr: A socket that is connecting with server.
	* @param article: Content that need to be posted to server.
	* @param buff: Application's buffer.
	*
	* @return: number of bytes sent if no error occurs.
	*		   SOCKET_ERROR if an error occurs.
	**/
	int sendPostMessage(SOCKET connSockPtr, const std::string& article, char* buff);

	/**
	* @function sendUserMessage: sends logout message to server.
	*
	* @param connSockPtr: A socket that is connecting with server.
	* @param buff: Application's buffer.
	*
	* @return: number of bytes sent if no error occurs.
	*		   SOCKET_ERROR if an error occurs.
	**/
	int sendByeMessage(SOCKET connSockPtr, char* buff);
}

#endif // !MESSAGE_SENDER_H
