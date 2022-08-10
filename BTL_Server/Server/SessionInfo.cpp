#include "SessionInfo.h"

SessionInfo::SessionInfo(SOCKET connSock)
	: connSock(connSock)
{
	username = "";
	isLogin = false;
	remainMessage = "";
	ipAddress = "";
	port = 0;
	isApproved = false;
}

SessionInfo::SessionInfo()
{
	username = "";
	isLogin = false;
	remainMessage = "";
	connSock = 0;
	ipAddress = "";
	port = 0;
	isApproved = false;
}

std::string SessionInfo::getUsername()
{
	return username;
}

void SessionInfo::setUsername(std::string username)
{
	this->username = username;
}

bool SessionInfo::getLoginStatus()
{
	return isLogin;
}

void SessionInfo::setLoginStatus(bool loginStatus)
{
	isLogin = loginStatus;
}

SOCKET SessionInfo::getConnSock()
{
	return connSock;
}

void SessionInfo::setConnSock(SOCKET connSock)
{
	this->connSock = connSock;
}

bool SessionInfo::getApprovedStatus()
{
	return isApproved;
}

void SessionInfo::setApprovedStatus(bool isApproved)
{
	this->isApproved = isApproved;
}

std::string SessionInfo::getIPAddress()
{
	return ipAddress;
}

void SessionInfo::setIPAddress(std::string ipAddress)
{
	this->ipAddress = ipAddress;
}

int SessionInfo::getPort()
{
	return port;
}

void SessionInfo::setPort(int port)
{
	this->port = port;
}

void SessionInfo::reset()
{
	username = "";
	isLogin = false;
	remainMessage = "";
	connSock = 0;
	messageQueue.clear();
	isApproved = false;
	ipAddress = "";
	port = 0;
}

void SessionInfo::receiveMessages(char * message)
{
	remainMessage.append(message);
	//used to store messages splitted from message queue
	std::vector<std::string> messages;
	int numOfMessages = splitString(remainMessage, ENDING_DELIMITER, ENDING_DELIMITER_LENGTH, messages, -1);
	if (numOfMessages == 0)
		return;

	for (int i = 0; i < numOfMessages - 1; ++i)
	{
		messageQueue.push_back(messages[i]);
	}

	// if message queue is empty after splitting,
	// that means the last message in messages array is a complete message
	// (complete message = message ends with ending delimiter)
	if (remainMessage.empty()) {
		messageQueue.push_back(messages[numOfMessages - 1]);
	}
}

int SessionInfo::getMessageNumber()
{
	return messageQueue.size();
}

std::string SessionInfo::getNextMessage()
{
	std::string message = messageQueue.front();
	messageQueue.erase(messageQueue.begin());
	return message;
}
