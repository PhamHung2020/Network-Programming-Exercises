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
	std::string username;
	bool isLogin;
	std::string remainMessage;
	std::vector<std::string> messageQueue;
	SOCKET connSock;
	bool isApproved;
	std::string ipAddress;
	int port;
public:
	SessionInfo(SOCKET connSock);
	SessionInfo();

	std::string getUsername();
	void setUsername(std::string username);
	bool getLoginStatus();
	void setLoginStatus(bool loginStatus);
	SOCKET getConnSock();
	void setConnSock(SOCKET connSock);
	bool getApprovedStatus();
	void setApprovedStatus(bool isApproved);
	std::string getIPAddress();
	void setIPAddress(std::string ipAddress);
	int getPort();
	void setPort(int port);
	void reset();

	void receiveMessages(char* message);
	int getMessageNumber();
	std::string getNextMessage();
};

#endif // !SESSION_INFO_H
