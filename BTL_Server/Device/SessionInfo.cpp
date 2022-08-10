#include "SessionInfo.h"

SessionInfo::SessionInfo(SOCKET connSock)
	: connSock(connSock)
{
	trusted = false;
	remainMessage = "";
}

SessionInfo::SessionInfo()
{
	trusted = false;
	remainMessage = "";
	connSock = 0;
}

bool SessionInfo::isTrusted()
{
	return trusted;
}

void SessionInfo::setTrusted(bool trusted)
{
	this->trusted = trusted;
}

SOCKET SessionInfo::getConnSock()
{
	return connSock;
}

void SessionInfo::setConnSock(SOCKET connSock)
{
	this->connSock = connSock;
}

void SessionInfo::reset()
{
	trusted = false;
	remainMessage = "";
	connSock = 0;
	messageQueue.clear();
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
