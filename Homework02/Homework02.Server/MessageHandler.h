#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <string>
#include <winsock2.h>
#include "Helper.h"
#include "AccountManager.h"
#include "ResponseCode.h"

class MessageHandler
{
private:
	AccountManager* accountManager;
	CRITICAL_SECTION* critical;
	std::string username;
	bool isLogin;
	int posOfAccount;

public:
	MessageHandler(AccountManager* accountManager, CRITICAL_SECTION* critical);

	/**
	* @function handleMessage: handles message from client.
	*
	* @param message: Message that need to be handled.
	* @param connSock: A socket that is connecting with server.
	* @param buff: Application's buffer.
	*
	* @return: true, if client is reponsed by success codes (10, 20, 30).
	*		   false, otherwise.
	**/
	bool handleMessage(std::string& message, SOCKET connSock, char* buff);

	/**
	* @function handleUserMessage: handles login message from client.
	*
	* @param messageComponents: All components in the message.
	* @param connSock: A socket that is connecting with server.
	* @param buff: Application's buffer.
	*
	* @return: position of account in list of account managed by AccountManager, if login success.
	*		   -1, otherwise.
	**/
	int handleUserMessage(std::vector<std::string> messageComponents, SOCKET connSock, char* buff);

	/**
	* @function handlePostMessage: handles post message from client.
	*
	* @param messageComponents: All components in the message.
	* @param connSock: A socket that is connecting with server.
	* @param buff: Application's buffer.
	*
	* @return: true, if client is reponsed by success code.
	*		   false, otherwise.
	**/
	bool handlePostMessage(std::vector<std::string> messageComponents, SOCKET connSock, char* buff);

	/**
	* @function handleByeMessage: handles bye message from client.
	*
	* @param shouldSendToClient: A boolean paramter for deciding if server should send response to client or not.
	* @param connSock: A socket that is connecting with server.
	* @param buff: Application's buffer.
	*
	* @return: true, if client is logout-ed successfully.
	*		   false, otherwise.
	**/
	bool handleByeMessage(bool shouldSendToClient, SOCKET connSock, char* buff);
};

#endif // !MESSAGE_HANDLER_H
