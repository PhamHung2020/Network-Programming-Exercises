#ifndef SESSION_INFO_H
#define SESSION_INFO_H

#include <string>
#include <vector>
#include <winsock2.h>
#include "Constant.h"
#include "Utility.h"

class SessionInfo
{
private:
	bool trusted;
	std::string remainMessage;
	std::vector<std::string> messageQueue;
	SOCKET connSock;
public:
	SessionInfo(SOCKET connSock);
	SessionInfo();

	bool isTrusted();
	void setTrusted(bool trusted);
	SOCKET getConnSock();
	void setConnSock(SOCKET connSock);
	void reset();

	void receiveMessages(char* message);
	int getMessageNumber();
	std::string getNextMessage();
};

#endif // !SESSION_INFO_H
