#include "MessageSender.h"


int MessageSender::sendLoginMessage(const std::string& username, const std::string& password) {

	std::string message = "USER " + username + " " + password;
	return Helper::sendMessage(message);

};

int MessageSender::sendChangePasswordMessage(const std::string& currentpasswd, const std::string& newpasswd, const std::string& confirmpasswd) {
	std::string message = "PASSWD " + currentpasswd + " " + newpasswd + " " + confirmpasswd;
	return Helper::sendMessage(message);
}

int MessageSender::sendLogoutMessage() {
	std::string message = "BYE";
	return Helper::sendMessage(message);
}

int MessageSender::sendGetAllDeviceMessage() {
	std::string message = "GET_DEVICE";
	return Helper::sendMessage(message);
}

int MessageSender::sendGetParamOfDevice(std::string& id_device) {
	std::string message = "GET_PARAM " + id_device;
	return Helper::sendMessage(message);
}

int MessageSender::sendGetValueParam(std::string& id_device, std::string& name, std::string type) {
	std::string message = "GET_" + type + " " + id_device + " " + name;
	return Helper::sendMessage(message);
}

int MessageSender::sendSetValueParam(std::string idDevice, std::string nameParam, std::string valueParam) {
	std::string message = "SET " + idDevice + " " + nameParam + " " + valueParam;
	return Helper::sendMessage(message);
}

int MessageSender::sendRemoveTimePoint(std::string idDevice, std::string idTime) {
	std::string message = "RMVTIME_P " + idDevice + " " + idTime;
	return Helper::sendMessage(message);
}

int MessageSender::sendAddTimePoint(std::string idDevice, std::string time) {
	std::string message = "ADDTIME_P " + idDevice + " " + time;
	return Helper::sendMessage(message);
};

int MessageSender::sendAddTimeRange(std::string idDevice, std::string start, std::string end) {
	std::string message = "ADDTIME_R " + idDevice + " " + start + "#" + end;
	return Helper::sendMessage(message);
};

int MessageSender::sendRemoveTimeRange(std::string idDeive, std::string idTimeRange) {
	std::string message = "RMVTIME_R " + idDeive + " " + idTimeRange;
	return Helper::sendMessage(message);
};

int MessageSender::sendDeviceControl(std::string idDevice, std::string status) {
	std::string message = "COMMAND " + idDevice + " " + status;
	return Helper::sendMessage(message);
};

int MessageSender::sendHello() {
	std::string message = "CLIENT";
	return Helper::sendMessage(message);
};