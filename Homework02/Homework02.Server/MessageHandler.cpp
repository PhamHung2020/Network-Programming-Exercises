#include "MessageHandler.h"
MessageHandler::MessageHandler(AccountManager* accountManager, CRITICAL_SECTION* critical)
	: accountManager(accountManager), critical(critical)
{
	username = "";
	isLogin = false;
	posOfAccount = -1;
}
bool MessageHandler::handleMessage(std::string& message, SOCKET connSock, char* buff)
{
	//used to store message type and all paramters of message
	std::vector<std::string> messageComponents = std::vector<std::string>();
	//most type of message has 2 components: message type and content (username, article), so we only need to split message into 2 substrings
	//therefore, value of numOfComponent must be 2 for USER and POST, or 1 for BYE message
	int numOfComponent = Helper::splitString(message, " ", 1, messageComponents, 2);
	if (numOfComponent < 1)
	{
		Helper::sendMessage(connSock, MESSAGE_INVALID_FORMAT, buff);
		return false;
	}

	std::string& messageType = messageComponents[0];
	if (messageType == "USER")
	{
		int posOfFoundAccount = handleUserMessage(messageComponents, connSock, buff);
		return (posOfFoundAccount == -1) ? false : true;
	}
	else if (messageType == "POST")
	{
		return handlePostMessage(messageComponents, connSock, buff);
	}
	else if (messageType == "BYE")
	{
		return handleByeMessage(true, connSock, buff);
	}
	else
	{
		Helper::sendMessage(connSock, MESSAGE_TYPE_NOT_EXIST, buff);
	}
	return false;
}
int MessageHandler::handleUserMessage(std::vector<std::string> messageComponents, SOCKET connSock, char* buff)
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
			Helper::sendMessage(connSock, USER_LOGINED_DIFFERENT_ACCOUNT, buff);
		return -1;
	}
	
	//find account by username
	Account account;
	int posOfFoundAccount = accountManager->getAccount(messageComponents[1], account);
	//handle wrong cases
	if (posOfFoundAccount == -1)
	{
		Helper::sendMessage(connSock, USER_ACCOUNT_NOT_EXIST, buff);
		return -1;
	}
	if (account.isLocked)
	{
		Helper::sendMessage(connSock, USER_ACCOUNT_NOT_ACTIVE, buff);
		return -1;
	}
	if (account.isLogin)
	{
		Helper::sendMessage(connSock, USER_ACCOUNT_ALREADY_LOGIN, buff);
		return -1;
	}

	//login process
	EnterCriticalSection(critical);
	accountManager->changeLoginStatus(true, posOfFoundAccount);
	LeaveCriticalSection(critical);

	isLogin = true;
	posOfAccount = posOfFoundAccount;
	username = messageComponents[1];
	Helper::sendMessage(connSock, USER_LOGIN_OK, buff);
	return posOfFoundAccount;
}
bool MessageHandler::handlePostMessage(std::vector<std::string> messageComponents, SOCKET connSock, char* buff)
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
bool MessageHandler::handleByeMessage(bool shouldSendToClient, SOCKET connSock, char* buff)
{
	//if user not login-ed yet
	if (!isLogin)
	{
		if (shouldSendToClient)
			Helper::sendMessage(connSock, POST_ACCOUNT_NOT_LOGIN_YET, buff);
		return false;
	}

	//logout process
	EnterCriticalSection(critical);
	accountManager->changeLoginStatus(false, posOfAccount);
	LeaveCriticalSection(critical);

	isLogin = false;
	posOfAccount = -1;
	username = "";
	if (shouldSendToClient)
		Helper::sendMessage(connSock, BYE_OK, buff);
	return true;
}