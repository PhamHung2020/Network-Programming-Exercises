#include "Notification.h"


void Notification::header(std::string message) {
	SetConsoleTextAttribute(hConsole, 15 | BACKGROUND_BLUE);
	std::cout << message << "\n";
	SetConsoleTextAttribute(hConsole, 15);
}

void Notification::successMessage(std::string message) {
	SetConsoleTextAttribute(hConsole, 9);
	std::cout << message << "\n";
	SetConsoleTextAttribute(hConsole, 15);
}

void Notification::warningMessage(std::string message) {
	SetConsoleTextAttribute(hConsole, 14);
	std::cout << message << "\n";
	SetConsoleTextAttribute(hConsole, 15);
}

void Notification::errorMessage(std::string message) {
	SetConsoleTextAttribute(hConsole, 12);
	std::cout << message << "\n";
	SetConsoleTextAttribute(hConsole, 15);
}

void Notification::displayResponse(std::string responseCode) {
	if (responseCode == USER_LOGIN_OK) {
		Notification::successMessage("You login-ed successfully!\n");
	}
	if (responseCode == USER_ACCOUNT_ALREADY_LOGIN) {
		Notification::errorMessage("This account is already login - ed in another client\n");
	}
	if (responseCode == USER_INVALID_CREDENTIALS) {
		Notification::errorMessage("This account is already login - ed in another client\n");
	}
	if (responseCode == USER_LOGGED_IN) {
		Notification::warningMessage("You've already login-ed");
	}



	// ---------------------------------------------------- //

	if (responseCode == PASSWD_OK) {
		Notification::successMessage("Change password successfully\n");
	}
	if (responseCode == PASSWD_FAILED) {
		Notification::errorMessage("Password change failed\n");
	}
	if (responseCode == PASSWD_PASSWORD_NOT_CHANGE) {
		Notification::warningMessage("Password not changed\n");
	}
	if (responseCode == USER_NOT_LOGIN) {
		Notification::errorMessage("User is not logged in\n");
	}
	// ---------------------------------------------------- //

	if (responseCode == BYE_OK) {
		Notification::successMessage("You logout-ed successfully\n");
	}



}