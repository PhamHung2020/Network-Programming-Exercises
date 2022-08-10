#ifndef HANDLE_ACTION_CLIENT_H
#define HANDLE_ACTION_CLIENT_H

#include <iostream>
#include <string>
#include <winsock2.h>
#include <vector>
#include "Constrain.h"
#include "Helper.h"
#include "Model.h"


namespace HandleActionClient {
	const std::string SPLIT_CODE(static_cast<char>(6), 1);
	const std::string SALT = "SALT";
	const std::string OXI = "OXI";
	const std::string PH = "PH";
	void handleRecv(int ret);
	void handleLogin(int ret);
	void handleChangePassword(int ret);
	void handleLogout(int ret);
	void handleGetAllDevice(int ret, std::vector<Device> &devices);
	void handleGetParamOfDevice(int ret, std::vector<Param> &params, std::string id);
	std::vector<std::string> getSingleValueOfParam(std::string id, std::string name);
	std::vector<Time> getTimeValueOfParam(std::string id, std::string name);
	std::vector<TimeRange> getTimeRangeValueOfParam(std::string id, std::string name);
	void handleSetValueParam(std::string id, std::string nameParam, std::string value);
	void handleRemoveTimePoint(std::string idDevice, std::string idTime);
	void handleAddTimePoint(std::string idDevice, std::string time);
	void handleAddTimeRange(std::string idDevice, std::string start, std::string end);
	void handleRemoveTimeRange(std::string idDevice, std::string idTimeRange);
	void handleControlDevice(std::string idDevice, std::string status);
}

#endif
