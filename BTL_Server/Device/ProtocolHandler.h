#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <winsock2.h>
#include "Time.h"
#include "Device.h"
#include "Sensor.h"
#include "SessionInfo.h"
#include "Constant.h"
#include "Utility.h"
#include "Validation.h"

class ProtocolHandler
{
public:
	static const std::string DEVICE_APPROVED;
	static const std::string NOT_APPROVED;
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
	static const std::string ADDTIME_R_OK;
	static const std::string ADDTIME_R_INVALID_DEVICE_ID;
	static const std::string ADDTIME_R_INVALID_TIME_RANGE;
	static const std::string RMVTIME_R_INVALID_VALUE_ID;
	static const std::string GET_OK;
	static const std::string GET_INVALID_DEVICE_ID;
	static const std::string GET_INVALID_PARAMETER;
	static const std::string COMMAND_OK;
	static const std::string COMMAND_INVALID_DEVICE_ID;
	static const std::string COMMAND_INVALID_PARAMETER;
	static const std::string COMMAND_FAILED;
	static const std::string GET_DEVICE_PARAM_OK;
	static const std::vector<std::string> messageTypes;
	static std::string serverCenterIP;
	static std::string handleMessage(std::string message, Device& device);
	static std::string handleSetMessage(std::string paramName, std::string paramValue, Device& device);
	static std::string handleAddTimeMessage(std::string timeStr, Time time, Device& device);
	static std::string handleRmvTimeMessage(int timeId, Device& device);
	static std::string handleAddTimeRangeMessage(std::string timerangeStr, TimeRange timerange, Device& device);
	static std::string handleRmvTimeRangeMessage(int timerangeId, Device& device);
	static std::string handleGetEnvironmentMessage(std::string dataName, Device& device);
	static std::string handleGetDeviceParamMessage(std::string paramName, Device& device);
	static std::string handleCommandMessage(bool status, Device& device);
	static std::string handleGetAllParamMessage(Device& device);
};

#endif // !PROTOCOL_HANDLER_H
