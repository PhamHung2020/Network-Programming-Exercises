#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <winsock2.h>
#include "SessionInfo.h"
#include "DeviceInfoHandler.h"
#include "Constant.h"
#include "Utility.h"
#include "Validation.h"

class ProtocolHandler
{
public:
	static const std::string CLIENT_APPROVED;
	static const std::string DEVICE_APPROVED;
	static const std::string NOT_APPROVED;
	static const std::string DEVICE_NOT_CONNECT;
	static const std::string MESSAGE_TYPE_NOT_EXIST;
	static const std::string MESSAGE_INVALID_FORMAT;
	static const std::string USER_LOGIN_OK;
	static const std::string USER_LOGGED_IN;
	static const std::string USER_ACCOUNT_ALREADY_LOGIN;
	static const std::string USER_INVALID_CREDENTIALS;
	static const std::string PASSWD_OK;
	static const std::string USER_NOT_LOGIN;
	static const std::string PASSWD_FAILED;
	static const std::string PASSWD_PASSWORD_NOT_CHANGE;
	static const std::string BYE_OK;
	static const std::string SET_OK;
	static const std::string SET_INVALID_DEVICE_ID;
	static const std::string SET_INVALID_PARAMETER_NAME;
	static const std::string SET_INVALID_VALUE;
	static const std::string ADDTIME_P_OK;
	static const std::string ADDTIME_P_INVALID_DEVICE_ID;
	static const std::string ADDTIME_P_INVALID_FORMAT_VALUE;
	static const std::string ADDTIME_P_VALUE_ALREADY_EXIST;
	static const std::string RMVTIME_P_INVALID_VALUE_ID;
	static const std::string GET_OK;
	static const std::string GET_INVALID_PARAMETER;
	static const std::string COMMAND_OK;
	static const std::string COMMAND_INVALID_PARAMETER;
	static const std::string GET_DEVICE_PARAM_OK;
	static const std::vector<std::string> messageTypes;
	static std::unordered_map<std::string, std::pair<std::string, bool>> accounts;
	static CRITICAL_SECTION criticalAccount;
	static std::vector<DeviceInfo> deviceInfos;
	static std::vector<CRITICAL_SECTION> criticalDevices;
	static int postfix;

	static std::string handleMessage(std::string message, SessionInfo& session);
	static std::string handleUserMessage(std::string username, std::string password, SessionInfo& session);
	static std::string handlePasswdMessage(std::string currentPasswd, std::string newPasswd, std::string confirmPasswd, SessionInfo& session);
	static std::string handleByeMessage(SessionInfo& session);
	static std::string handleGetDeviceMessage();
	static std::string handleDeviceMessage(std::string message, int deviceId, SessionInfo& sessionInfo);
	static std::string handleHelloMessage(std::string message, SessionInfo& session);
};

#endif // !PROTOCOL_HANDLER_H
