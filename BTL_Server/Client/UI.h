#ifndef UI_H
#define UI_H



#include <iostream>
#include <string>
#include <vector>
#include <math.h>

#include "Model.h"
#include "InputHandler.h"
#include "MessageSender.h"
#include "HandleActionClient.h"
#include "Notification.h"

namespace UI {
	enum PAGE {
		LOGIN_PAGE,
		BREAK,
		HOME_PAGE,
		CURRENT_PAGE,
		DEVICE_MANAGER_PAGE,
		PARAM_MANAGER_PAGE,
		SET_VALUE_PARAM,
		CHANGE_TIME_POINT,
		REMOVE_TIME_POINT,
		ADD_TIME_POINT,
		ADD_TIME_RANGE,
		REMOVE_TIME_RANGE,
		CHANGE_TIME_RANGE,
		CONTROL_DEVICE,
		DATA_SENSOR

	};
	const std::string SALT = "SALT";
	const std::string OXI = "OXI";
	const std::string PH = "PH";
	const std::string URL = "sensor.txt";
	static PAGE currentPage;
	static std::string idDevice;
	static std::string nameDevice;
	static Param paramSelect;
	void App(PAGE page);
	void login();
	void homepage();
	void deviceManager();
	void paramManagerPage();
	void setValueParam();
	void pageTimePoint();
	void removeTimePoint();
	void addTimePoint();
	void pageTimeRange();
	void addTimeRange();
	void removeTimeRange();
	void dataSensor();
	void deviceControl();
}

#endif