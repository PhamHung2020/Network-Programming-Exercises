#include "MessageHandler.h"

std::unordered_map<std::string, bool> MessageHandler::accountStatus;

MessageHandler::MessageHandler(SOCKET connSock)
	: connSock(connSock)
{
	username = "";
	isLogin = false;
	messageQueue = "";
}

void MessageHandler::handleReceivedMessages(char* buff)
{
	messageQueue.append(buff);
	//used to store messages splitted from message queue
	std::vector<std::string> messages = std::vector<std::string>();
	int numOfMessages = Helper::splitString(messageQueue, ENDING_DELIMITER, ENDING_DELIMITER_LENGTH, messages, -1);
	if (numOfMessages == 0)
		return;

	for (int i = 0; i < numOfMessages - 1; ++i)
	{
		handleMessage(messages[i], buff);
	}

	// if message queue is empty after splitting,
	// that means the last message in messages array is a complete message
	// (complete message = message ends with ending delimiter)
	if (messageQueue.empty())
		handleMessage(messages[numOfMessages - 1], buff);
}

bool MessageHandler::handleMessage(std::string& message, char* buff)
{
	//used to store message type and all paramters of message
	std::vector<std::string> messageComponents = std::vector<std::string>();
	//most type of message has 2 components: message type and content (username, article), so we only need to split message into 2 substrings
	//therefore, value of numOfComponent is usually 2
	int numOfComponent = Helper::splitString(message, " ", 1, messageComponents, 2);
	if (numOfComponent < 1)
	{
		Helper::sendMessage(connSock, MESSAGE_INVALID_FORMAT, buff);
		return false;
	}

	std::string& messageType = messageComponents[0];
	if (messageType == "USER")
	{
		int posOfFoundAccount = handleUserMessage(messageComponents, buff);
		return (posOfFoundAccount == -1) ? false : true;
	}
	else if (messageType == "POST")
	{
		return handlePostMessage(messageComponents, buff);
	}
	else if (messageType == "BYE")
	{
		return handleByeMessage(true, buff);
	}
	else
	{
		Helper::sendMessage(connSock, MESSAGE_TYPE_NOT_EXIST, buff);
	}
	return false;
}

int MessageHandler::handleUserMessage(std::vector<std::string> messageComponents, char* buff)
{
	int numOfParameter = (int)messageComponents.size() - 1;
	//login message need only 1 paramter: username --> numOfParameter must be 1
	if (numOfParameter != 1)
	{
		Helper::sendMessage(connSock, MESSAGE_INVALID_FORMAT, buff);
		return -1;
	}
	//if the user who owns this message handler object is already login-ed.
	if (isLogin)
	{
		if (messageComponents[1] == username)
			Helper::sendMessage(connSock, USER_LOGINED, buff);
		else
			Helper::sendMessage(connSock, USER_ACCOUNT_ALREADY_LOGIN, buff);
		return -1;
	}

	//get account status by username in hash table
	std::unordered_map<std::string, bool>::const_iterator account = accountStatus.find(messageComponents[1]);
	if (account == accountStatus.end()) //account not found
	{
		Helper::sendMessage(connSock, USER_ACCOUNT_NOT_EXIST, buff);
		return -1;
	}

	if (account->second) //account is locked
	{
		Helper::sendMessage(connSock, USER_ACCOUNT_NOT_ACTIVE, buff);
		return -1;
	}
	
	isLogin = true;
	username = messageComponents[1];
	Helper::sendMessage(connSock, USER_LOGIN_OK, buff);
	return 0;
}

bool MessageHandler::handlePostMessage(std::vector<std::string> messageComponents, char* buff)
{
	int numOfParameter = messageComponents.size() - 1;
	//login message need only 1 paramter: content of the post --> numOfParameter must be 1
	if (numOfParameter != 1)
	{
		Helper::sendMessage(connSock, MESSAGE_INVALID_FORMAT, buff);
		return false;
	}
	//if user not login-ed yet
	if (!isLogin)
	{
		Helper::sendMessage(connSock, POST_ACCOUNT_NOT_LOGIN_YET, buff);
		return false;
	}
	Helper::sendMessage(connSock, POST_OK, buff);
	return true;
}

bool MessageHandler::handleByeMessage(bool shouldSendToClient, char* buff)
{
	if (!isLogin)
	{
		if (shouldSendToClient)
			Helper::sendMessage(connSock, POST_ACCOUNT_NOT_LOGIN_YET, buff);
		return false;
	}

	isLogin = false;
	username = "";
	if (shouldSendToClient)
		Helper::sendMessage(connSock, BYE_OK, buff);
	return true;
}

void MessageHandler::reset()
{
	username = "";
	messageQueue = "";
	connSock = 0;
	isLogin = false;
}