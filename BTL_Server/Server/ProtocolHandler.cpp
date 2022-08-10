#include "ProtocolHandler.h"

const std::string ProtocolHandler::CLIENT_APPROVED = "01";
const std::string ProtocolHandler::DEVICE_APPROVED = "02";
const std::string ProtocolHandler::NOT_APPROVED = "03";
const std::string ProtocolHandler::DEVICE_NOT_CONNECT = "04";
const std::string ProtocolHandler::MESSAGE_TYPE_NOT_EXIST = "99";
const std::string ProtocolHandler::MESSAGE_INVALID_FORMAT = "98";
const std::string ProtocolHandler::USER_LOGIN_OK = "10";
const std::string ProtocolHandler::USER_LOGGED_IN = "11";
const std::string ProtocolHandler::USER_ACCOUNT_ALREADY_LOGIN = "12";
const std::string ProtocolHandler::USER_INVALID_CREDENTIALS = "13";
const std::string ProtocolHandler::PASSWD_OK = "20";
const std::string ProtocolHandler::USER_NOT_LOGIN = "21";
const std::string ProtocolHandler::PASSWD_FAILED = "22";
const std::string ProtocolHandler::PASSWD_PASSWORD_NOT_CHANGE = "23";
const std::string ProtocolHandler::BYE_OK = "30";
const std::string ProtocolHandler::SET_OK = "40";
const std::string ProtocolHandler::SET_INVALID_DEVICE_ID = "41";
const std::string ProtocolHandler::SET_INVALID_PARAMETER_NAME = "42";
const std::string ProtocolHandler::SET_INVALID_VALUE = "43";
const std::string ProtocolHandler::ADDTIME_P_OK = "50";
const std::string ProtocolHandler::ADDTIME_P_INVALID_DEVICE_ID = "51";
const std::string ProtocolHandler::ADDTIME_P_INVALID_FORMAT_VALUE = "52";
const std::string ProtocolHandler::ADDTIME_P_VALUE_ALREADY_EXIST = "53";
const std::string ProtocolHandler::RMVTIME_P_INVALID_VALUE_ID = "54";
const std::string ProtocolHandler::GET_OK = "70";
const std::string ProtocolHandler::GET_INVALID_PARAMETER = "71";
const std::string ProtocolHandler::COMMAND_OK = "80";
const std::string ProtocolHandler::COMMAND_INVALID_PARAMETER = "81";
const std::string ProtocolHandler::GET_DEVICE_PARAM_OK = "90";

const std::vector<std::string> ProtocolHandler::messageTypes = { "USER", "PASSWD", "BYE", "SET", "ADDTIME_P", "RMVTIME_P", "ADDTIME_R", "RMVTIME_R", "GET_I", "GET_D", "COMMAND", "GET_DEVICE", "GET_PARAM" };
std::unordered_map<std::string, std::pair<std::string, bool>> ProtocolHandler::accounts;
CRITICAL_SECTION ProtocolHandler::criticalAccount;
std::vector<DeviceInfo> ProtocolHandler::deviceInfos;
std::vector<CRITICAL_SECTION> ProtocolHandler::criticalDevices;
int ProtocolHandler::postfix = 0;

std::string ProtocolHandler::handleMessage(std::string message, SessionInfo& session)
{
	//used to store message type and all paramters of message
	std::vector<std::string> messageComponents;
	//most type of message has 2 components: message type and content (username, article), so we only need to split message into 2 substrings
	//therefore, value of numOfComponent is usually 2
	std::string messageCopy = message;
	int numOfComponent = splitString(messageCopy, " ", 1, messageComponents, -1);
	if (numOfComponent < 1)
	{
		return MESSAGE_INVALID_FORMAT;
	}

	std::string& messageType = messageComponents[0];
	if (messageType == "CLIENT" || messageType == "DEVICE")
		return NOT_APPROVED;

	bool isFound = false;
	for (auto& messageTypeDefined : messageTypes)
	{
		if (messageType == messageTypeDefined)
		{
			isFound = true;
			break;
		}
	}
	if (!isFound)
	{
		return MESSAGE_TYPE_NOT_EXIST;
	}

	if (messageType == "USER")
	{
		if (numOfComponent != 3)
			return MESSAGE_INVALID_FORMAT;
		// only need number of paramter, not message type --> messageComponents.size() - 1
		// for example: USER tungbt --> 2 component ('USER' and 'tungbt'), but 1 paramter (tungbt)
		return handleUserMessage(messageComponents[1], messageComponents[2], session);
	}
	else
	{
		if (!session.getLoginStatus())
		{
			return USER_NOT_LOGIN;
		}
		if (messageType == "PASSWD")
		{
			if (numOfComponent != 4)
				return MESSAGE_INVALID_FORMAT;
			return handlePasswdMessage(messageComponents[1], messageComponents[2], messageComponents[3], session);
		}
		else if (messageType == "BYE")
		{
			return handleByeMessage(session);
		}
		else if (messageType == "GET_DEVICE")
		{
			if (numOfComponent != 1)
				return MESSAGE_INVALID_FORMAT;
			return handleGetDeviceMessage();
		}
		else
		{
			int deviceId;
			if (numOfComponent < 2 || !isInt(messageComponents[1], deviceId))
				return MESSAGE_INVALID_FORMAT;
			return handleDeviceMessage(message, deviceId, session);
		}
	}
}

std::string ProtocolHandler::handleUserMessage(std::string username, std::string password, SessionInfo& session)
{
	//if the user who owns this message handler object is already login-ed.
	if (session.getLoginStatus())
	{
		return USER_LOGGED_IN;
	}

	EnterCriticalSection(&criticalAccount);
	//get account status by username in hash table
	std::unordered_map<std::string, std::pair<std::string, bool>>::iterator account = accounts.find(username);
	if (account == accounts.end()) //account not found
	{
		LeaveCriticalSection(&criticalAccount);
		return USER_INVALID_CREDENTIALS;
	}

	if (account->second.second) //account is already logged in in another client
	{
		LeaveCriticalSection(&criticalAccount);
		return USER_ACCOUNT_ALREADY_LOGIN;
	}

	if (password != account->second.first)
	{
		LeaveCriticalSection(&criticalAccount);
		return USER_INVALID_CREDENTIALS;
	}

	account->second.second = true;
	LeaveCriticalSection(&criticalAccount);
	session.setLoginStatus(true);
	session.setUsername(username);
	return USER_LOGIN_OK;
}

std::string ProtocolHandler::handlePasswdMessage(std::string currentPasswd, std::string newPasswd, std::string confirmPasswd, SessionInfo& session)
{
	EnterCriticalSection(&criticalAccount);
	//get account status by username in hash table
	std::unordered_map<std::string, std::pair<std::string, bool>>::iterator account = accounts.find(session.getUsername());
	if (account == accounts.end()) //account not found
	{
		LeaveCriticalSection(&criticalAccount);
		return USER_INVALID_CREDENTIALS;
	}

	if (account->second.first != currentPasswd)
	{
		LeaveCriticalSection(&criticalAccount);
		return PASSWD_FAILED;
	}

	if (newPasswd != confirmPasswd)
	{
		LeaveCriticalSection(&criticalAccount);
		return PASSWD_FAILED;
	}

	if (currentPasswd == newPasswd)
	{
		LeaveCriticalSection(&criticalAccount);
		return PASSWD_PASSWORD_NOT_CHANGE;
	}

	account->second.first = newPasswd;
	LeaveCriticalSection(&criticalAccount);
	return PASSWD_OK;
}

std::string ProtocolHandler::handleByeMessage(SessionInfo & session)
{
	EnterCriticalSection(&criticalAccount);
	accounts[session.getUsername()].second = false;
	LeaveCriticalSection(&criticalAccount);
	session.setUsername("");
	session.setLoginStatus(false);
	return BYE_OK;
}

std::string ProtocolHandler::handleGetDeviceMessage()
{
	std::string retVal = GET_DEVICE_PARAM_OK + " device ";
	for (auto& device : deviceInfos)
	{
		if (!device.sessionInfo.getApprovedStatus())
			continue;
		retVal += (std::to_string(device.deviceId) + '#' + device.deviceName);
		retVal += 6;
	}
	retVal.pop_back();
	return retVal;
}

std::string ProtocolHandler::handleDeviceMessage(std::string message, int deviceId, SessionInfo & sessionInfo)
{
	int i = 0;
	for (auto& deviceInfo : deviceInfos)
	{
		if (deviceInfo.deviceId == deviceId)
		{
			if (!deviceInfo.sessionInfo.getApprovedStatus())
				return DEVICE_NOT_CONNECT;
			EnterCriticalSection(&criticalDevices[i]);
			SOCKET connSock = deviceInfo.sessionInfo.getConnSock();
			int ret = sendMessage(connSock, message + std::to_string(postfix), deviceInfo.buff);
			++postfix;
			if (postfix > 9)
				postfix = 0;

			if (ret == SOCKET_ERROR)
			{
				LeaveCriticalSection(&criticalDevices[i]);
				if (WSAGetLastError() == WSAETIMEDOUT)
				{
					printf("Send data to device %d: Time-out!\n", deviceId);
					return DEVICE_NOT_CONNECT;
				}
				if (WSAGetLastError() == WSAECONNABORTED)
					printf("Device %d is unavailable for now\n", deviceId);
				else
					printf("Error %d: Cannot send data to device %d\n", WSAGetLastError(), deviceId);
				closesocket(deviceInfo.sessionInfo.getConnSock());
				deviceInfo.sessionInfo.reset();
				printf("Device %d disconnect\n", deviceId);
				return DEVICE_NOT_CONNECT;
			}
			while (1)
			{
				ret = recv(connSock, deviceInfo.buff, DATA_BUFSIZE - 1, 0);
				if (ret == SOCKET_ERROR)
				{
					LeaveCriticalSection(&criticalDevices[i]);
					if (WSAGetLastError() == WSAETIMEDOUT)
					{
						printf("Receive data from device %d: Time-out!\n", deviceId);
						return DEVICE_NOT_CONNECT;
					}
					if (WSAGetLastError() == WSAECONNABORTED)
						printf("Device %d is unavailable for now\n", deviceId);
					else
						printf("Error %d: Cannot receive data from device %d.\n", WSAGetLastError(), deviceId);
					closesocket(deviceInfo.sessionInfo.getConnSock());
					deviceInfo.sessionInfo.reset();
					printf("Device %d disconnect\n", deviceId);
					return DEVICE_NOT_CONNECT;
				}
				else if (ret == 0)
				{
					LeaveCriticalSection(&criticalDevices[i]);
					printf("Device %d is unavailable for now\n", deviceId);
					closesocket(deviceInfo.sessionInfo.getConnSock());
					deviceInfo.sessionInfo.reset();
					printf("Device %d disconnect\n", deviceId);
					return DEVICE_NOT_CONNECT;
				}
				else if (strlen(deviceInfo.buff) > 0)
				{
					deviceInfo.buff[ret] = 0;
					deviceInfo.sessionInfo.receiveMessages(deviceInfo.buff);
					if (deviceInfo.sessionInfo.getMessageNumber() > 0)
					{
						std::string response = deviceInfo.sessionInfo.getNextMessage();
						LeaveCriticalSection(&criticalDevices[i]);
						char responsePostfix = response[response.length() - 1] - '0';
						if ((postfix == 0 && responsePostfix == 9) || (postfix - 1 == responsePostfix))
						{
							response.pop_back();
							return response;
						}
						else
						{
							printf("Response from device %d is ignored\n", deviceId);
						}
					}
				}
			}
		}
		++i;
	}
	return SET_INVALID_DEVICE_ID;
}

std::string ProtocolHandler::handleHelloMessage(std::string message, SessionInfo & session)
{
	//used to store message type and all paramters of message
	std::vector<std::string> messageComponents;
	//most type of message has 2 components: message type and content (username, article), so we only need to split message into 2 substrings
	//therefore, value of numOfComponent is usually 2
	int numOfComponent = splitString(message, " ", 1, messageComponents, -1);
	if (numOfComponent < 1)
	{
		return MESSAGE_INVALID_FORMAT;
	}
	if (messageComponents[0] == "CLIENT")
	{
		if (numOfComponent != 1)
			return MESSAGE_INVALID_FORMAT;
		session.setApprovedStatus(true);
		printf("Client [%s:%d] connected!", &(session.getIPAddress())[0], session.getPort());
		return CLIENT_APPROVED;
	}
	else if (messageComponents[0] == "DEVICE")
	{
		if (numOfComponent != 3)
			return MESSAGE_INVALID_FORMAT;
		int deviceId;
		if (!isInt(messageComponents[1], deviceId))
			return MESSAGE_INVALID_FORMAT;
		for (auto& deviceInfo : deviceInfos)
		{
			if (deviceInfo.deviceId == deviceId && deviceInfo.devicePass == messageComponents[2])
			{
				if (deviceInfo.sessionInfo.getApprovedStatus())
				{
					int ret = sendMessage(deviceInfo.sessionInfo.getConnSock(), "WORKING", deviceInfo.buff);
					if (ret == SOCKET_ERROR)
					{
						closesocket(deviceInfo.sessionInfo.getConnSock());
						deviceInfo.sessionInfo.reset();
						printf("Device %d disconnect\n", deviceId);
					}
					else
					{
						while (1)
						{
							ret = recv(deviceInfo.sessionInfo.getConnSock(), deviceInfo.buff, DATA_BUFSIZE - 1, 0);
							if (ret == SOCKET_ERROR || ret == 0)
							{
								closesocket(deviceInfo.sessionInfo.getConnSock());
								deviceInfo.sessionInfo.reset();
								printf("Device %d disconnect\n", deviceId);
								break;
							}
							else
							{
								deviceInfo.buff[ret] = 0;
								deviceInfo.sessionInfo.receiveMessages(deviceInfo.buff);
								if (deviceInfo.sessionInfo.getMessageNumber() > 0)
									break;
							}
						}
						std::string message = deviceInfo.sessionInfo.getNextMessage();
						if (message == "WORKING")
							return NOT_APPROVED;
						else
						{
							closesocket(deviceInfo.sessionInfo.getConnSock());
							deviceInfo.sessionInfo.reset();
							printf("Device %d disconnect\n", deviceId);
						}
					}

				}

				deviceInfo.sessionInfo.setApprovedStatus(true);
				deviceInfo.sessionInfo.setConnSock(session.getConnSock());
				deviceInfo.sessionInfo.setIPAddress(session.getIPAddress());
				deviceInfo.sessionInfo.setPort(session.getPort());
				printf("Device %d connected!", deviceId);
				return DEVICE_APPROVED;
			}
		}
		return NOT_APPROVED;
	}
	return MESSAGE_TYPE_NOT_EXIST;
}
