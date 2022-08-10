#include "ProtocolHandler.h"

const std::string ProtocolHandler::DEVICE_APPROVED = "02";
const std::string ProtocolHandler::NOT_APPROVED = "03";
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
const std::string ProtocolHandler::ADDTIME_R_OK = "60";
const std::string ProtocolHandler::ADDTIME_R_INVALID_DEVICE_ID = "61";
const std::string ProtocolHandler::ADDTIME_R_INVALID_TIME_RANGE = "62";
const std::string ProtocolHandler::RMVTIME_R_INVALID_VALUE_ID = "63";
const std::string ProtocolHandler::GET_OK = "70";
const std::string ProtocolHandler::GET_INVALID_DEVICE_ID = "71";
const std::string ProtocolHandler::GET_INVALID_PARAMETER = "72";
const std::string ProtocolHandler::COMMAND_OK = "80";
const std::string ProtocolHandler::COMMAND_INVALID_DEVICE_ID = "81";
const std::string ProtocolHandler::COMMAND_INVALID_PARAMETER = "82";
const std::string ProtocolHandler::COMMAND_FAILED = "83";
const std::string ProtocolHandler::GET_DEVICE_PARAM_OK = "90";

const std::vector<std::string> ProtocolHandler::messageTypes = { "SET", "ADDTIME_P", "RMVTIME_P", "ADDTIME_R", "RMVTIME_R", "GET_I", "GET_D", "COMMAND", "GET_PARAM" };

std::string ProtocolHandler::handleMessage(std::string message, Device& device)
{
	std::cout << message << '\n';
	if (message == "WORKING")
		return message;
	//used to store message type and all paramters of message
	std::vector<std::string> messageComponents;
	std::string messageCopy = message;
	messageCopy.pop_back();
	char postfix = message[message.length() - 1];
	//most type of message has 2 components: message type and content (username, article), so we only need to split message into 2 substrings
	//therefore, value of numOfComponent is usually 2
	int numOfComponent = splitString(messageCopy, " ", 1, messageComponents, -1);
	std::string response;
	if (numOfComponent < 2)
	{
		return MESSAGE_INVALID_FORMAT + postfix;
	}

	std::string& messageType = messageComponents[0];
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
		return MESSAGE_TYPE_NOT_EXIST + postfix;
	}

	int deviceId;
	if (!isInt(messageComponents[1], deviceId))
		return MESSAGE_INVALID_FORMAT + postfix;

	if (deviceId != device.getId())
	{
		std::cout << device.getId() << '\n';
		return SET_INVALID_DEVICE_ID + postfix;
	}

	if (messageType == "SET")
	{
		if (numOfComponent != 4)
			return MESSAGE_INVALID_FORMAT + postfix;
		return handleSetMessage(messageComponents[2], messageComponents[3], device) + postfix;
	}
	else if (messageType == "ADDTIME_P")
	{
		if (numOfComponent != 3)
			return MESSAGE_INVALID_FORMAT + postfix;
		Time time;
		if (!isTime(messageComponents[2], time))
			return ADDTIME_P_INVALID_FORMAT_VALUE + postfix;
		return handleAddTimeMessage(messageComponents[2], time, device) + postfix;
	}
	else if (messageType == "ADDTIME_R")
	{
		if (numOfComponent != 3)
			return MESSAGE_INVALID_FORMAT + postfix;
		TimeRange timerange;
		if (!isTimeRange(messageComponents[2], timerange))
			return ADDTIME_P_INVALID_FORMAT_VALUE + postfix;
		return handleAddTimeRangeMessage(messageComponents[2], timerange, device) + postfix;
	}
	else if (messageType == "RMVTIME_P")
	{
		if (numOfComponent != 2)
			return MESSAGE_INVALID_FORMAT + postfix;
		int timeId;
		if (!isInt(messageComponents[1], timeId))
			return RMVTIME_P_INVALID_VALUE_ID + postfix;
		return handleRmvTimeMessage(timeId, device) + postfix;
	}
	else if (messageType == "RMVTIME_R")
	{
		if (numOfComponent != 2)
			return MESSAGE_INVALID_FORMAT + postfix;
		int timerangeId;
		if (!isInt(messageComponents[1], timerangeId))
			return RMVTIME_R_INVALID_VALUE_ID + postfix;
		return handleRmvTimeRangeMessage(timerangeId, device) + postfix;
	}
	else if (messageType == "GET_I")
	{
		if (numOfComponent != 3)
			return MESSAGE_INVALID_FORMAT + postfix;
		return handleGetEnvironmentMessage(messageComponents[2], device) + postfix;
	}
	else if (messageType == "GET_D")
	{
		if (numOfComponent != 3)
			return MESSAGE_INVALID_FORMAT + postfix;
		return handleGetDeviceParamMessage(messageComponents[2], device) + postfix;
	}
	else if (messageType == "COMMAND")
	{
		if (numOfComponent != 3)
			return MESSAGE_INVALID_FORMAT + postfix;
		if (messageComponents[2] != "ON" && messageComponents[2] != "OFF")
			return COMMAND_INVALID_PARAMETER + postfix;
		return handleCommandMessage(messageComponents[2] == "ON", device) + postfix;
	}
	else if (messageType == "GET_PARAM")
	{
		std::cout << numOfComponent << std::endl;
		if (numOfComponent != 2)
			return MESSAGE_INVALID_FORMAT + postfix;
		return handleGetAllParamMessage(device) + postfix;
	}
	return MESSAGE_TYPE_NOT_EXIST + postfix;
}

std::string ProtocolHandler::handleSetMessage(std::string paramName, std::string paramValue, Device & device)
{
	if (!device.isParameterExist(paramName))
		return SET_INVALID_PARAMETER_NAME;
	if (!device.setParameter(paramName, paramValue))
		return SET_INVALID_VALUE;
	return SET_OK;
}

std::string ProtocolHandler::handleAddTimeMessage(std::string timeStr, Time time, Device & device)
{
	DeviceParameter* deviceParam = device.getParameterReference("Time");
	if (deviceParam == nullptr)
		return ADDTIME_P_INVALID_DEVICE_ID;
	if (deviceParam->getParameterType() != DeviceParameterType::Multiple)
		return ADDTIME_P_INVALID_DEVICE_ID;
	MultipleDeviceParameter<Time>* multiDeviceParam = (MultipleDeviceParameter<Time>*)deviceParam;
	if (multiDeviceParam->isValueExist(time))
		return ADDTIME_P_VALUE_ALREADY_EXIST;
	multiDeviceParam->setValue(timeStr);

	return ADDTIME_P_INVALID_FORMAT_VALUE;
}

std::string ProtocolHandler::handleRmvTimeMessage(int timeId, Device & device)
{
	DeviceParameter* deviceParam = device.getParameterReference("Time");
	if (deviceParam == nullptr)
		return ADDTIME_P_INVALID_DEVICE_ID;
	if (deviceParam->getParameterType() != DeviceParameterType::Multiple)
		return ADDTIME_P_INVALID_DEVICE_ID;
	MultipleDeviceParameter<Time>* multiDeviceParam = (MultipleDeviceParameter<Time>*)deviceParam;
	if (multiDeviceParam->removeValue(timeId))
		return ADDTIME_P_OK;
	return RMVTIME_P_INVALID_VALUE_ID;
}

std::string ProtocolHandler::handleAddTimeRangeMessage(std::string timerangeStr, TimeRange timerange, Device & device)
{
	DeviceParameter* deviceParam = device.getParameterReference("TimeRange");
	if (deviceParam == nullptr)
		return ADDTIME_R_INVALID_DEVICE_ID;
	if (deviceParam->getParameterType() != DeviceParameterType::Multiple)
		return ADDTIME_R_INVALID_DEVICE_ID;
	MultipleDeviceParameter<TimeRange>* multiDeviceParam = (MultipleDeviceParameter<TimeRange>*)deviceParam;
	if (multiDeviceParam->isValueExist(timerange))
		return ADDTIME_P_VALUE_ALREADY_EXIST;
	multiDeviceParam->setValue(timerangeStr);

	return ADDTIME_P_INVALID_FORMAT_VALUE;
}

std::string ProtocolHandler::handleRmvTimeRangeMessage(int timerangeId, Device & device)
{
	DeviceParameter* deviceParam = device.getParameterReference("Time");
	if (deviceParam == nullptr)
		return ADDTIME_R_INVALID_DEVICE_ID;
	if (deviceParam->getParameterType() != DeviceParameterType::Multiple)
		return ADDTIME_R_INVALID_DEVICE_ID;
	MultipleDeviceParameter<Time>* multiDeviceParam = (MultipleDeviceParameter<Time>*)deviceParam;
	if (multiDeviceParam->removeValue(timerangeId))
		return ADDTIME_R_OK;
	return RMVTIME_R_INVALID_VALUE_ID;
}

std::string ProtocolHandler::handleGetEnvironmentMessage(std::string dataName, Device & device)
{
	if (device.getDeviceType() != "Sensor")
		return GET_INVALID_DEVICE_ID;
	Sensor* sensor = (Sensor*)&device;
	bool isFound;
	double value = sensor->getEnvironmentParameterValue(dataName, isFound);
	if (!isFound)
		return GET_INVALID_PARAMETER;
	return GET_OK + ' ' + std::to_string(value);
}

std::string ProtocolHandler::handleGetDeviceParamMessage(std::string paramName, Device & device)
{
	DeviceParameter* deviceParam = device.getParameterReference(paramName);
	if (deviceParam == nullptr)
		return GET_INVALID_PARAMETER;
	return GET_OK + ' ' + deviceParam->getValue();
}

std::string ProtocolHandler::handleCommandMessage(bool status, Device & device)
{
	std::cout << status << '\n';
	if (device.setStatus(status))
		return COMMAND_OK;
	return COMMAND_FAILED;
}

std::string ProtocolHandler::handleGetAllParamMessage(Device & device)
{
	return GET_OK + ' ' + device.getAllParam();
}


