#include "MessageHandler.h"

std::unordered_map<std::string, bool> MessageHandler::accountStatus;

MessageHandler::MessageHandler(SOCKET connSock = 0)
	: connSock(connSock)
{
	username = "";
	isLogin = false;
	remainMessage = "";
}

MessageHandler::MessageHandler() {
	username = "";
	isLogin = false;
	remainMessage = "";
	connSock = 0;
}

void MessageHandler::handleReceivedMessages(char* buff)
{
	remainMessage.append(buff);
	//used to store messages splitted from message queue
	std::vector<std::string> messages;
	int numOfMessages = Helper::splitString(remainMessage, ENDING_DELIMITER, ENDING_DELIMITER_LENGTH, messages, -1);
	if (numOfMessages == 0)
		return;

	for (int i = 0; i < numOfMessages - 1; ++i)
	{
		messageQueue.push_back(messages[i]);
		//handleMessage(messages[i], buff);
	}

	// if message queue is empty after splitting,
	// that means the last message in messages array is a complete message
	// (complete message = message ends with ending delimiter)
	if (remainMessage.empty()) {
		messageQueue.push_back(messages[numOfMessages - 1]);
		//handleMessage(messages[numOfMessages - 1], buff);
	}
	//std::cout << messageQueue.front() << '\n';
}

std::string MessageHandler::handleMessage()
{
	//std::cout << messageQueue.front() << '\n';
	//printf("%d\n", messageQueue.size());
	if (messageQueue.empty()) {
		return MESSAGE_INVALID_FORMAT;
	}
	
	std::string message = messageQueue.front();
	messageQueue.erase(messageQueue.begin());
	//used to store message type and all paramters of message
	std::vector<std::string> messageComponents;
	//most type of message has 2 components: message type and content (username, article), so we only need to split message into 2 substrings
	//therefore, value of numOfComponent is usually 2
	int numOfComponent = Helper::splitString(message, " ", 1, messageComponents, 2);
	if (numOfComponent < 1)
	{
		return MESSAGE_INVALID_FORMAT;
		//Helper::sendMessage(connSock, MESSAGE_INVALID_FORMAT, buff);
		//return false;
	}

	std::string& messageType = messageComponents[0];
	if (messageType == "USER")
	{
		// only need number of paramter, not message type --> messageComponents.size() - 1
		// for example: USER tungbt --> 2 component ('USER' and 'tungbt'), but 1 paramter (tungbt)
		return handleUserMessage(messageComponents.size() - 1, messageComponents[1]);
		//return (posOfFoundAccount == -1) ? false : true;
	}
	else if (messageType == "POST")
	{
		return handlePostMessage(messageComponents.size() - 1);
	}
	else if (messageType == "BYE")
	{
		return handleByeMessage(true);
	}
	else
	{
		return MESSAGE_TYPE_NOT_EXIST;
		//Helper::sendMessage(connSock, MESSAGE_TYPE_NOT_EXIST, buff);
	}
	//return false;
}

std::string MessageHandler::handleUserMessage(int numOfParameter, std::string& usernameLogin)
{
	//login message need only 1 paramter: username --> numOfParameter must be 1
	if (numOfParameter != 1)
	{
		return MESSAGE_INVALID_FORMAT;
		//Helper::sendMessage(connSock, MESSAGE_INVALID_FORMAT, buff);
		//return -1;
	}
	//if the user who owns this message handler object is already login-ed.
	if (isLogin)
	{
		if (usernameLogin == username) {
			return USER_LOGINED;
			//Helper::sendMessage(connSock, USER_LOGINED, buff);
		}
		else {
			return USER_ACCOUNT_ALREADY_LOGIN;
			//Helper::sendMessage(connSock, USER_ACCOUNT_ALREADY_LOGIN, buff);
		}
		//return -1;
	}

	//get account status by username in hash table
	std::unordered_map<std::string, bool>::const_iterator account = accountStatus.find(usernameLogin);
	if (account == accountStatus.end()) //account not found
	{
		return USER_ACCOUNT_NOT_EXIST;
		//Helper::sendMessage(connSock, USER_ACCOUNT_NOT_EXIST, buff);
		//return -1;
	}

	if (account->second) //account is locked
	{
		return USER_ACCOUNT_NOT_ACTIVE;
		//Helper::sendMessage(connSock, USER_ACCOUNT_NOT_ACTIVE, buff);
		//return -1;
	}

	isLogin = true;
	username = usernameLogin;
	return USER_LOGIN_OK;
	//Helper::sendMessage(connSock, USER_LOGIN_OK, buff);
	//return 0;
}

std::string MessageHandler::handlePostMessage(int numOfParameter)
{
	//login message need only 1 paramter: content of the post --> numOfParameter must be 1
	if (numOfParameter != 1)
	{
		return MESSAGE_INVALID_FORMAT;
		//Helper::sendMessage(connSock, MESSAGE_INVALID_FORMAT, buff);
		//return false;
	}
	//if user not login-ed yet
	if (!isLogin)
	{
		return POST_ACCOUNT_NOT_LOGIN_YET;
		//Helper::sendMessage(connSock, POST_ACCOUNT_NOT_LOGIN_YET, buff);
		//return false;
	}
	return POST_OK;
	//Helper::sendMessage(connSock, POST_OK, buff);
	//return true;
}

std::string MessageHandler::handleByeMessage(bool shouldSendToClient)
{
	if (!isLogin)
	{
		if (shouldSendToClient) {
			return POST_ACCOUNT_NOT_LOGIN_YET;
			//Helper::sendMessage(connSock, POST_ACCOUNT_NOT_LOGIN_YET, buff);
		}
		//return false;
		return "";
	}

	isLogin = false;
	username = "";
	if (shouldSendToClient) {
		return BYE_OK;
		//Helper::sendMessage(connSock, BYE_OK, buff);
	}
	return "";
}

void MessageHandler::reset()
{
	username = "";
	remainMessage = "";
	connSock = 0;
	isLogin = false;
	/*while (!messageQueue.empty()) {
		messageQueue.pop_back();
	}*/
	messageQueue.clear();
}

int MessageHandler::getMessageNumber() {
	return messageQueue.size();
}