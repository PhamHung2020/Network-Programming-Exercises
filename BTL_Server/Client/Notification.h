#pragma once
#include <iostream>
#include <string>
#include <Windows.h>
#include "ResponseCode.h"
#include "Constrain.h"
namespace Notification {
	void header(std::string message);
	void successMessage(std::string message);
	void warningMessage(std::string message);
	void errorMessage(std::string message);
	void displayResponse(std::string responseCode);
}