#include "HandleActionClient.h"

void HandleActionClient::handleRecv(int ret) {
	std::string messageQueue = "";
	if (ret == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAECONNABORTED)
			printf("Server is unavailable for now. Please restart program or use this program in another time\n");
		else
			printf("Error %d: Cannot send data. Please try again\n", WSAGetLastError());
		return;
	}

	while (1) {
		//Receive message from server until the message queue is empty
		ret = recv(client, buff, BUFF_SIZE - 1, 0);

		if (ret == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAECONNABORTED)
				printf("Server is unavailable for now. Please restart program or use this program in another time\n");
			else
				printf("Error %d: Cannot receive data. Please restart program or use this program in another time\n", WSAGetLastError());
			break;
		}
		else if (ret == 0)
		{
			printf("Server is unavailable for now. Please restart program or use this program in another time\n");
			break;
		}
		else if (strlen(buff) > 0)
		{
			buff[ret] = 0;
			messageQueue.append(buff);
			//used to store messages splitted from message queue
			std::vector<std::string> messages = std::vector<std::string>();
			int numOfMessages = Helper::splitString(messageQueue, ENDING_DELIMITER, ENDING_DELIMITER_LENGTH, messages, -1);
			if (numOfMessages == 0)
				return;
			if (messageQueue.empty())
			{
				response = messages[numOfMessages - 1];
				return;
			}
		}
	}
}


void HandleActionClient::handleLogin(int ret) {
	handleRecv(ret);

	if (response == USER_LOGIN_OK) {
		system("cls");
		Notification::displayResponse(response);
		isLogin = true;
	}
	else {
		system("cls");
		isLogin = false;
		Notification::displayResponse(response);
	}
}

void HandleActionClient::handleChangePassword(int ret) {
	handleRecv(ret);
	system("cls");
	Notification::displayResponse(response);
}

void HandleActionClient::handleLogout(int ret) {
	handleRecv(ret);
	system("cls");
	if (response == BYE_OK) {
		isLogin = false;
	}
	Notification::displayResponse(response);
}

void HandleActionClient::handleGetAllDevice(int ret, std::vector<Device> &devices) {
	handleRecv(ret);
	int index;
	if ((index = response.find(GET_DEVICE_HEADER)) != std::string::npos) {
		std::vector<std::string> messages = std::vector<std::string>();
		std::string data = response.substr(index + GET_DEVICE_HEADER.length() + 1, response.length() - (index + GET_DEVICE_HEADER.length()));
		int numOfMessages = Helper::splitString(data, SPLIT_CODE.c_str(), SPLIT_CODE.length(), messages, -1);
		if (numOfMessages == 0) {
			Notification::errorMessage("The returned data is not in the correct format\n");
			return;
		}
		for (int i = 0; i < numOfMessages; i++) {
			std::vector<std::string> dev = std::vector<std::string>();
			int size = Helper::splitString(messages[i], DEVICE_DELIMITER, DEVICE_DELIMITER_LENGTH, dev, -1);
			if (size == 2) {
				Device newDevice(dev[0], dev[1]);
				devices.push_back(newDevice);
			}
			else {
				Notification::errorMessage("The returned data is not in the correct format\n");
				return;
			}

		}
	}
}

void HandleActionClient::handleGetParamOfDevice(int ret, std::vector<Param> &params, std::string id) {
	handleRecv(ret);
	int index;

	if ((index = response.find(GET_PARAM_HEADER)) != std::string::npos) {
		std::vector<std::string> messages = std::vector<std::string>();
		std::string data = response.substr(index + GET_PARAM_HEADER.length() + 1, response.length() - (index + GET_PARAM_HEADER.length()));
		int numOfMessages = Helper::splitString(data, SPLIT_CODE.c_str(), SPLIT_CODE.length(), messages, -1);
		if (numOfMessages == 0) {
			Notification::errorMessage("The returned data is not in the correct format\n");
			return;
		}
		for (int i = 0; i < numOfMessages; i++) {
			std::vector<std::string> dev = std::vector<std::string>();
			int size = Helper::splitString(messages[i], DEVICE_DELIMITER, DEVICE_DELIMITER_LENGTH, dev, -1);
			if (size == 2) {
				Param newParam(dev[0], dev[1]);
				params.push_back(newParam);
			}
			else {
				Notification::errorMessage("The returned data is not in the correct format\n");
				return;
			}
		}
	}
}

std::vector<std::string> HandleActionClient::getSingleValueOfParam(std::string id, std::string name) {
	int ret;

	std::vector<std::string> values;
	if (name == OXI || name == PH || name == SALT) {
		ret = MessageSender::sendGetValueParam(id, name, "I");
	}
	else {
		ret = MessageSender::sendGetValueParam(id, name, "D");
	}

	handleRecv(ret);

	if (response == GET_INVALID_ID || response == GET_INVALID_PARAMETER) {
		Notification::errorMessage("Invalid value!");
		return values;
	}
	else {
		int index = response.find(" ");
		if (index != std::string::npos) {
			std::string value = response.substr(index + 1, response.length() - index);
			values.push_back(value);
		}

		return values;
	}


}

std::vector<Time> HandleActionClient::getTimeValueOfParam(std::string id, std::string name) {
	int ret;
	ret = MessageSender::sendGetValueParam(id, name, "D");
	handleRecv(ret);
	std::vector<Time> values;
	if (response == GET_INVALID_ID || response == GET_INVALID_PARAMETER) {
		Notification::errorMessage("Invalid value!");
		return values;
	}
	else {
		int index = response.find(" ");
		if (index != std::string::npos) {
			std::string value = response.substr(index + 1, response.length() - index);
			std::vector<std::string> messages;
			int numOfMessages = Helper::splitString(value, SPLIT_CODE.c_str(), SPLIT_CODE.length(), messages, -1);
			if (numOfMessages == 0) {
				Notification::errorMessage("The returned data is not in the correct format\n");
				return values;
			}
			for (int i = 0; i < numOfMessages; i++) {
				std::vector<std::string> dev = std::vector<std::string>();
				int size = Helper::splitString(messages[i], DEVICE_DELIMITER, DEVICE_DELIMITER_LENGTH, dev, -1);
				if (size == 2) {
					Time newTime(dev[0], dev[1]);
					values.push_back(newTime);
				}
				else {
					Notification::errorMessage("The returned data is not in the correct format\n");
					return values;
				}
			}
			return values;
		}
	}
}
std::vector<TimeRange> HandleActionClient::getTimeRangeValueOfParam(std::string id, std::string name) {
	int ret;
	std::vector<TimeRange> values;
	ret = MessageSender::sendGetValueParam(id, name, "D");
	handleRecv(ret);
	if (response == GET_INVALID_ID || response == GET_INVALID_PARAMETER) {
		Notification::errorMessage("Invalid value!");
		return values;
	}
	else {
		int index = response.find(" ");
		if (index != std::string::npos) {
			std::string value = response.substr(index + 1, response.length() - index);
			std::vector<std::string> messages;
			int numOfMessages = Helper::splitString(value, SPLIT_CODE.c_str(), SPLIT_CODE.length(), messages, -1);
			if (numOfMessages == 0) {
				Notification::errorMessage("The returned data is not in the correct format\n");
				return values;
			}
			for (int i = 0; i < numOfMessages; i++) {
				std::vector<std::string> dev = std::vector<std::string>();
				int size = Helper::splitString(messages[i], DEVICE_DELIMITER, DEVICE_DELIMITER_LENGTH, dev, -1);
				if (size == 3) {
					TimeRange newTimeRange(dev[0], dev[1], dev[3]);
					values.push_back(newTimeRange);
				}
				else {
					Notification::errorMessage("The returned data is not in the correct format\n");
					return values;
				}
			}
			return values;
		}
	}
}

void HandleActionClient::handleSetValueParam(std::string id, std::string nameParam, std::string value) {
	int ret;
	ret = MessageSender::sendSetValueParam(id, nameParam, value);
	handleRecv(ret);

	if (response == SET_OK) {
		Notification::successMessage("Set up successfully!");
	}
	else if (response == SET_INVALID_DEVICE_ID) {
		Notification::errorMessage("Invalid device id!");
	}
	else if (response == SET_INVALID_VALUE) {
		Notification::errorMessage("Invalid value!");
	}
	else if (response == SET_INVALID_PARAMETER_NAME) {
		Notification::errorMessage("Invalid parameter name!");
	}
};


void HandleActionClient::handleRemoveTimePoint(std::string idDevice, std::string idTime) {
	int ret;
	ret = MessageSender::sendRemoveTimePoint(idDevice, idTime);
	handleRecv(ret);

	if (response == ADDTIME_P_OK) {
		Notification::successMessage("Delete successfully!");
	}
	else if (response == ADDTIME_P_INVALID_DEVICE_ID) {
		Notification::errorMessage("Invalid device id!");
	}
	else if (response == ADDTIME_P_INVALID_FORMAT_VALUE) {
		Notification::errorMessage("Invalid format value!");
	}

}

void HandleActionClient::handleAddTimePoint(std::string idDevice, std::string time) {
	int ret;
	ret = MessageSender::sendAddTimePoint(idDevice, time);
	handleRecv(ret);

	if (response == ADDTIME_P_OK) {
		Notification::successMessage("Delete successfully!");
	}
	else if (response == ADDTIME_P_INVALID_DEVICE_ID) {
		Notification::errorMessage("Invalid device id!");
	}
	else if (response == ADDTIME_P_INVALID_FORMAT_VALUE) {
		Notification::errorMessage("Invalid format value!");
	}
	else if (response == ADDTIME_P_VALUE_ALREADY_EXIST) {
		Notification::warningMessage("Value already exist!");
	}
};

void HandleActionClient::handleAddTimeRange(std::string idDevice, std::string start, std::string end) {
	int ret;
	ret = MessageSender::sendAddTimeRange(idDevice, start, end);
	handleRecv(ret);

	if (response == ADDTIME_R_OK) {
		Notification::successMessage("Delete successfully!");
	}
	else if (response == ADDTIME_R_INVALID_DEVICE_ID) {
		Notification::errorMessage("Invalid device id!");
	}
	else if (response == ADDTIME_P_INVALID_FORMAT_VALUE) {
		Notification::errorMessage("Invalid format value!");
	}
	else if (response == ADDTIME_P_VALUE_ALREADY_EXIST) {
		Notification::warningMessage("Value already exist!");
	}
};

void HandleActionClient::handleRemoveTimeRange(std::string idDevice, std::string idTimeRange) {
	int ret;
	ret = MessageSender::sendRemoveTimeRange(idDevice, idTimeRange);
	handleRecv(ret);

	if (response == ADDTIME_R_OK) {
		Notification::successMessage("Delete successfully!");
	}
	else if (response == ADDTIME_R_INVALID_DEVICE_ID) {
		Notification::errorMessage("Invalid device id!");
	}
	else if (response == ADDTIME_P_INVALID_FORMAT_VALUE) {
		Notification::errorMessage("Invalid format value!");
	}
	else if (response == RMVTIME_R_INVALID_VALUE_ID) {
		Notification::warningMessage("Invalid time range id!");
	}
};

void HandleActionClient::handleControlDevice(std::string idDevice, std::string status) {
	int ret;
	ret = MessageSender::sendDeviceControl(idDevice, status);
	handleRecv(ret);

	if (response == COMMAND_OK) {
		Notification::successMessage("Command device successfully!");
	}
	else if (response == COMMAND_INVALID_DEVICE_ID) {
		Notification::errorMessage("Invalid device id!");
	}
	else if (response == COMMAND_INVALID_PARAMETER) {
		Notification::errorMessage("Invalid format value!");
	}

};

