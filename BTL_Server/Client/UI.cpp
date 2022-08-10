#include "UI.h"




void UI::App(PAGE page) {
	switch (page)
	{
	case UI::LOGIN_PAGE:
		UI::login();
		break;
	case UI::BREAK:
		break;
	case UI::HOME_PAGE:
		UI::homepage();
		break;
	case UI::DEVICE_MANAGER_PAGE:
		UI::deviceManager();
		break;
	case UI::PARAM_MANAGER_PAGE:
		UI::paramManagerPage();
		break;
	case UI::CHANGE_TIME_POINT:
		UI::pageTimePoint();
		break;
	case UI::SET_VALUE_PARAM:
		UI::setValueParam();
		break;
	case UI::ADD_TIME_POINT:
		UI::addTimePoint();
		break;
	case UI::REMOVE_TIME_POINT:
		UI::removeTimePoint();
		break;
	case UI::CHANGE_TIME_RANGE:
		UI::pageTimeRange();
		break;
	case UI::ADD_TIME_RANGE:
		UI::addTimeRange();
		break;
	case UI::REMOVE_TIME_RANGE:
		UI::removeTimeRange();
		break;
	case CONTROL_DEVICE:
		UI::deviceControl();
		break;
	case DATA_SENSOR:
		UI::dataSensor();
		break;
	default:
		break;
	}
}
void UI::login() {
	currentPage = UI::LOGIN_PAGE;
	Notification::header("\n<-------------------MENU------------------>\n");
	std::cout << "1. Login\n2. Exit\n";
	int ret;
	int userchoice = InputHandler::inputIntInRange("\nSelect function: ", 1, 2);
	if (userchoice == 1) {
		system("cls");
		Notification::header("<-------Login------->");
		std::string username = InputHandler::inputString("User name: ");
		std::string password = InputHandler::inputString("Password: ");
		ret = MessageSender::sendLoginMessage(username, password);
		HandleActionClient::handleLogin(ret);
		if (isLogin) {
			App(UI::HOME_PAGE);
		}
		else {
			App(currentPage);
		}
	}
	else if (userchoice == 2) {
		UI::App(UI::BREAK);
	}

}

void UI::homepage() {

	Notification::header("\n<-------------------MENU------------------>\n");
	std::cout << "1. Show all device\n2. Change password\n3. Device control\n4. Logout\n5. Exit\n";
	int ret;
	int userChoice = InputHandler::inputIntInRange("\nSelect function: ", 1, 4);
	if (userChoice == 1) {
		currentPage = UI::HOME_PAGE;
		App(UI::DEVICE_MANAGER_PAGE);
	}
	else if (userChoice == 2) {
		currentPage = UI::HOME_PAGE;
		system("cls");
		Notification::header("<-------Change password------->");
		std::string currentpasswd = InputHandler::inputString("Current password: ");
		std::string newpasswd = InputHandler::inputString("New password: ");
		std::string confirmpasswd = InputHandler::inputString("Confirm password: ");
		ret = MessageSender::sendChangePasswordMessage(currentpasswd, newpasswd, confirmpasswd);
		HandleActionClient::handleChangePassword(ret);
		App(currentPage);
	}
	else if (userChoice == 3) {
		currentPage = UI::HOME_PAGE;
		App(UI::CONTROL_DEVICE);
	}
	else if (userChoice == 4) {
		ret = MessageSender::sendLogoutMessage();
		HandleActionClient::handleLogout(ret);
		currentPage = UI::HOME_PAGE;
		App(UI::LOGIN_PAGE);
	}
	else if (userChoice == 5) {
		ret = MessageSender::sendLogoutMessage();
		HandleActionClient::handleLogout(ret);
		return;
	}

}

void UI::deviceManager() {

	std::vector<Device> devices;
	int ret = MessageSender::sendGetAllDeviceMessage();
	HandleActionClient::handleGetAllDevice(ret, devices);
	int numberOfDevice = devices.size();
	for (int i = 0; i < numberOfDevice; i++) {
		std::cout << i + 1 << ". " << devices[i].name << "\n";
	}
	std::cout << numberOfDevice + 1 << ". Back" << "\n";
	int userChoice = InputHandler::inputIntInRange("\nSelect function: ", 1, numberOfDevice + 1);
	if (userChoice == numberOfDevice + 1) {
		App(currentPage);
	}
	else {
		currentPage = UI::DEVICE_MANAGER_PAGE;
		idDevice = devices[userChoice - 1].id;
		nameDevice = devices[userChoice - 1].name;
		if (nameDevice == "Sensor") {
			App(UI::DATA_SENSOR);
		}
		else {
			App(PARAM_MANAGER_PAGE);
		}
	}

}
void UI::dataSensor() {


	Notification::header("<------------------" + nameDevice + "------------------>");
	std::string oxi, ph, salt;

	std::vector<std::string> res = HandleActionClient::getSingleValueOfParam(idDevice, SALT);
	std::cout << "SALT:" << "\n";
	salt = res[0];
	for (int j = 0; j < res.size(); j++) {
		std::cout << "\t" << res[j] << "\n";
	}
	res = HandleActionClient::getSingleValueOfParam(idDevice, OXI);
	oxi = res[0];
	std::cout << "OXI:" << "\n";

	for (int j = 0; j < res.size(); j++) {
		std::cout << "\t" << res[j] << "\n";
	}
	res = HandleActionClient::getSingleValueOfParam(idDevice, PH);
	ph = res[0];
	std::cout << "PH:" << "\n";

	for (int j = 0; j < res.size(); j++) {
		std::cout << "\t" << res[j] << "\n";
	}


	std::cout << 1 << ". " << "Back" << "\n";
	int userChoice = InputHandler::inputIntInRange("\nSelect function: ", 1, 1);
	Helper::writeDataOnFile(oxi, ph, salt, URL);
	App(UI::DEVICE_MANAGER_PAGE);
}
void UI::paramManagerPage() {

	std::vector<Param> params;
	int ret = MessageSender::sendGetParamOfDevice(idDevice);
	HandleActionClient::handleGetParamOfDevice(ret, params, idDevice);
	int numberOfParam = params.size();
	Notification::header("<------------------" + nameDevice + "------------------>");
	for (int i = 0; i < numberOfParam; i++) {
		std::cout << i + 1 << ". " << params[i].fullName << "(" << params[i].name << ")" << "\n";
	}
	std::cout << numberOfParam + 1 << ". " << "Back" << "\n";
	int userChoice = InputHandler::inputIntInRange("\nSelect function: ", 1, numberOfParam + 1);
	if (userChoice == numberOfParam + 1) {
		App(UI::HOME_PAGE);
	}
	else {
		UI::paramSelect = params[userChoice - 1];
		if (params[userChoice - 1].name == "Time") {
			App(UI::CHANGE_TIME_POINT);
		}
		else if (params[userChoice - 1].name == "TimeRange") {
			App(UI::CHANGE_TIME_RANGE);
		}
		else {
			App(UI::SET_VALUE_PARAM);
		}
	}




}

void UI::setValueParam() {
	currentPage = UI::PARAM_MANAGER_PAGE;
	Notification::header("<------------------" + nameDevice + "------------------>");
	std::vector<std::string> res = HandleActionClient::getSingleValueOfParam(idDevice, paramSelect.name);
	std::cout << paramSelect.fullName << "(" << paramSelect.name << ")" << ":\n";
	int size = res.size();
	for (int i = 0; i < size; i++) {
		std::cout << "\t" << res[i] << "\n";
	}
	std::cout << "1. " << "Set value" << " " << paramSelect.fullName << "\n";
	std::cout << "2. " << "Back" << '\n';
	int userChoice = InputHandler::inputIntInRange("\nSelect function: ", 1, 2);
	if (userChoice == 1) {
		system("cls");
		Notification::header("Set value " + paramSelect.fullName + "(" + paramSelect.name + ")" + "\n");
		std::string value = InputHandler::inputString("Value: ");
		HandleActionClient::handleSetValueParam(idDevice, paramSelect.name, value);
		App(currentPage);
	}
	else {
		App(currentPage);
	}
}

void UI::pageTimePoint() {
	currentPage = UI::PARAM_MANAGER_PAGE;
	Notification::header("\n<------------------" + nameDevice + "------------------>\n");
	// std::vector<Time> res = HandleActionClient::getTimeValueOfParam(idDevice, paramSelect.name);
	std::cout << paramSelect.fullName << "(" << paramSelect.name << ")" << ":\n";

	std::cout << "1. " << "Add time point" << " " << paramSelect.fullName << "\n";
	std::cout << "2. " << "Remove time point" << " " << paramSelect.fullName << "\n";
	std::cout << "3. " << "Back" << '\n';
	int userChoice = InputHandler::inputIntInRange("\nSelect function: ", 1, 3);
	if (userChoice == 1) {
		App(UI::ADD_TIME_POINT);
	}
	else if (userChoice == 2) {
		App(UI::REMOVE_TIME_POINT);
	}
	else {
		App(currentPage);
	}
}

void UI::addTimePoint() {
	Notification::header("\n<------------------" + nameDevice + "------------------>\n");
	std::string h, m, s;
	do {
		h = InputHandler::inputString("Hours: ");
		m = InputHandler::inputString("Minute: ");
		s = InputHandler::inputString("Second: ");
	} while (h.length() > 2 || m.length() > 2 || s.length() > 2);
	h = std::string(2 - (2 > h.length() ? h.length() : 2), '0') + h;
	m = std::string(2 - (2 > m.length() ? m.length() : 2), '0') + m;
	s = std::string(2 - (2 > s.length() ? s.length() : 2), '0') + s;
	std::string time = h + ":" + m + ":" + s;
	HandleActionClient::handleAddTimePoint(idDevice, time);
	App(UI::PARAM_MANAGER_PAGE);
};
void UI::removeTimePoint() {
	Notification::header("\n<------------------" + nameDevice + "------------------>\n");
	std::vector<Time> res = HandleActionClient::getTimeValueOfParam(idDevice, paramSelect.name);
	std::cout << paramSelect.fullName << "(" << paramSelect.name << ")" << ":\n";
	int size = res.size();
	for (int i = 0; i < size; i++) {
		std::cout << i + 1 << "\t" << res[i].value << "\n";
	}
	int index = InputHandler::inputIntInRange("Enter your order time point: ", 1, size);
	HandleActionClient::handleRemoveTimePoint(idDevice, res[index - 1].id);
	App(UI::PARAM_MANAGER_PAGE);
};
void UI::pageTimeRange() {
	currentPage = UI::PARAM_MANAGER_PAGE;
	Notification::header("\n<------------------" + nameDevice + "------------------>\n");

	std::cout << paramSelect.fullName << "(" << paramSelect.name << ")" << ":\n";

	std::cout << "1. " << "Add time time range" << " " << paramSelect.fullName << "\n";
	std::cout << "2. " << "Remove time range" << " " << paramSelect.fullName << "\n";
	std::cout << "3. " << "Back" << '\n';
	int userChoice = InputHandler::inputIntInRange("\nSelect function: ", 1, 3);
	if (userChoice == 1) {
		App(UI::ADD_TIME_RANGE);
	}
	else if (userChoice == 2) {
		App(UI::REMOVE_TIME_RANGE);
	}
	else {
		App(currentPage);
	}
}


void UI::addTimeRange() {
	Notification::header("\n<------------------" + nameDevice + "------------------>\n");
	std::string hStart, mStart, sStart;
	std::cout << "Enter start time: \n";
	do {
		hStart = InputHandler::inputString("Hours: ");
		mStart = InputHandler::inputString("Minute: ");
		sStart = InputHandler::inputString("Second: ");
	} while (hStart.length() > 2 || mStart.length() > 2 || sStart.length() > 2);
	hStart = std::string(2 - (2 > hStart.length() ? hStart.length() : 2), '0') + hStart;
	mStart = std::string(2 - (2 > mStart.length() ? mStart.length() : 2), '0') + mStart;
	sStart = std::string(2 - (2 > sStart.length() ? sStart.length() : 2), '0') + sStart;
	std::string startTime = hStart + ":" + mStart + ":" + sStart;
	std::string hEnd, mEnd, sEnd;
	std::cout << "Enter end time: \n";
	do {
		hEnd = InputHandler::inputString("Hours: ");
		mEnd = InputHandler::inputString("Minute: ");
		sEnd = InputHandler::inputString("Second: ");
	} while (hEnd.length() > 2 || mEnd.length() > 2 || sEnd.length() > 2);
	hEnd = std::string(2 - (2 > hEnd.length() ? hEnd.length() : 2), '0') + hEnd;
	mEnd = std::string(2 - (2 > mEnd.length() ? mEnd.length() : 2), '0') + mEnd;
	sEnd = std::string(2 - (2 > sEnd.length() ? sEnd.length() : 2), '0') + sEnd;
	std::string endTime = hEnd + ":" + mEnd + ":" + sEnd;
	HandleActionClient::handleAddTimeRange(idDevice, startTime, endTime);
	App(UI::PARAM_MANAGER_PAGE);
}
void UI::removeTimeRange() {
	Notification::header("\n<------------------" + nameDevice + "------------------>\n");
	std::vector<TimeRange> res = HandleActionClient::getTimeRangeValueOfParam(idDevice, paramSelect.name);
	std::cout << paramSelect.fullName << "(" << paramSelect.name << ")" << ":\n";
	int size = res.size();
	for (int i = 0; i < size; i++) {
		std::cout << i + 1 << "\t" << res[i].startTime << " ---> " << res[i].endTime << "\n";
	}
	int index = InputHandler::inputIntInRange("Enter your order time point: ", 1, size);
	HandleActionClient::handleRemoveTimeRange(idDevice, res[index - 1].id);
	App(UI::PARAM_MANAGER_PAGE);
}


void UI::deviceControl() {
	std::vector<Device> devices;
	int ret = MessageSender::sendGetAllDeviceMessage();
	HandleActionClient::handleGetAllDevice(ret, devices);
	int numberOfDevice = devices.size();
	for (int i = 0; i < numberOfDevice; i++) {
		std::cout << i + 1 << ". " << devices[i].name << "\n";
	}
	std::cout << numberOfDevice + 1 << ". Back" << "\n";
	int userChoice = InputHandler::inputIntInRange("\nSelect function: ", 1, numberOfDevice + 1);
	if (userChoice == numberOfDevice + 1) {
		App(UI::HOME_PAGE);
	}
	else {
		Notification::header("Change status " + nameDevice);
		std::string status;
		do {
			status = InputHandler::inputString("Status(ON/OFF): ");
		} while (status != "ON" && status != "OFF");

		HandleActionClient::handleControlDevice(idDevice, status);
		App(UI::HOME_PAGE);
	}

};