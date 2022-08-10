#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <string>
#include <unordered_map>
#include <winsock2.h>
#include "Helper.h"
#include "ResponseCode.h"

class MessageHandler
{
private:
	std::string username;
	bool isLogin;
	std::string messageQueue;

public:
	//a hash table to store information of all accounts, key is username, value is status (locked or not)
	// accountStatus[username] = status
	static std::unordered_map<std::string, bool> accountStatus;
	SOCKET connSock;

	MessageHandler(SOCKET connSock);

	/**
	* @function handleReceivedMessages: handle messages from client (handle stream, get each message from messageQueue to process).
	*
	* @param buff: Application's buffer.
	*
	**/
	void handleReceivedMessages(char* buff);

	/**
	* @function handleMessage: handle a message from client.
	*
	* @param message: Message that need to be handled.
	* @param buff: Application's buffer.
	*
	* @return: true, if client is reponsed by success codes (10, 20, 30).
	*		   false, otherwise.
	**/
	bool handleMessage(std::string& message, char* buff);

	/**
	* @function handleUserMessage: handles login message from clien.
	*
	* @param messageComponents: All components in the message.
	* @param buff: Application's buffer.
	*
	* @return: 0, if login success.
	*		   -1, otherwise.
	**/
	int handleUserMessage(std::vector<std::string> messageComponents, char* buff);

	/**
	* @function handlePostMessage: handles post message from client.
	*
	* @param messageComponents: All components in the message.
	* @param buff: Application's buffer.
	*
	* @return: true, if client is reponsed by success code.
	*		   false, otherwise.
	**/
	bool handlePostMessage(std::vector<std::string> messageComponents, char* buff);

	/**
	* @function handlePostMessage: handles bye message from client.
	*
	* @param shouldSendToClient: A boolean paramter for deciding if server should send response to client or not.
	* @param buff: Application's buffer.
	*
	* @return: true, if client is logout-ed successfully.
	*		   false, otherwise.
	**/
	bool handleByeMessage(bool shouldSendToClient, char* buff);

	/**
	* @function reset: reset all properties of MessageHandler object.
	*
	**/
	void reset();
};

#endif // !MESSAGE_HANDLER_H
