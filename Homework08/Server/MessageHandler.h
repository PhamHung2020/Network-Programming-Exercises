#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <winsock2.h>
#include "Helper.h"
#include "ResponseCode.h"

class MessageHandler
{
private:
	std::string username;
	bool isLogin;
	std::string remainMessage;
	std::vector<std::string> messageQueue;

public:
	//a hash table to store information of all accounts, key is username, value is status (locked or not)
	// accountStatus[username] = status
	static std::unordered_map<std::string, bool> accountStatus;
	SOCKET connSock;

	MessageHandler(SOCKET connSock);
	MessageHandler();

	/**
	* @function handleReceivedMessages: handle messages from client (split message and push to message queue).
	*
	* @param buff: Application's buffer.
	*
	**/
	void handleReceivedMessages(char* buff);

	/**
	* @function handleMessage: handle a message of client from message queue.
	*
	* @return: response to the message.
	**/
	std::string handleMessage();

	/**
	* @function handleUserMessage: handles login message from client.
	*
	* @param numOfParameter: Number of parameter in the message.
	* @param username: Username for login
	*
	* @return: response to the message.
	**/
	std::string handleUserMessage(int numOfParameter, std::string& username);

	/**
	* @function handlePostMessage: handles post message from client.
	*
	* @param numOfParameter: Number of parameter in the message.
	*
	* @return: response to the message.
	**/
	std::string handlePostMessage(int numOfParameter);

	/**
	* @function handlePostMessage: handles bye message from client.
	*
	* @param shouldSendToClient: A boolean paramter for deciding if server should send response to client or not.
	*
	* @return: response to the message.
	**/
	std::string handleByeMessage(bool shouldSendToClient);

	/**
	* @function reset: reset all properties of MessageHandler object.
	*
	**/
	void reset();

	/**
	* @function getMessageNumber: get number of messages in message queue.
	*
	* @return: number of messages in message queue.
	**/
	int getMessageNumber();
};

#endif // !MESSAGE_HANDLER_H
