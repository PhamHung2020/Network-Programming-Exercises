#include "UIHandler.h"

void UIHandler::displayMenu(bool isLogin)
{
	printf("\n\n------------------- MENU ------------------\n");
	if (!isLogin)
	{
		printf("1. Login\n2. Exit\n");
	}
	else
	{
		printf("1. Post a new article\n2. Logout\n3. Exit\n");
	}
}

void UIHandler::displayResponse(const std::string& responseCode)
{
	if (responseCode == USER_LOGIN_OK)
	{
		printf("You login-ed successfully!\n");
	}
	else if (responseCode == USER_ACCOUNT_NOT_ACTIVE)
	{
		printf("This account is not active. Please login by another account\n");
	}
	else if (responseCode == USER_ACCOUNT_NOT_EXIST)
	{
		printf("This account is not exist. Please check username and try again\n");
	}
	else if (responseCode == USER_ACCOUNT_ALREADY_LOGIN)
	{
		printf("This account is already login-ed in another client\n");
	}
	else if (responseCode == USER_LOGINED)
	{
		printf("You've already login-ed");
	}
	else if (responseCode == POST_OK)
	{
		printf("Your new article has been posted successfully\n");
	}
	else if (responseCode == POST_ACCOUNT_NOT_LOGIN_YET)
	{
		printf("You've not login-ed yet\n");
	}
	else if (responseCode == BYE_OK)
	{
		printf("You logout-ed successfully\n");
	}
	else if (responseCode == MESSAGE_INVALID_FORMAT)
	{
		printf("Your input must not be empty or only contain white space\n");
	}
	else
	{
		printf("Something's wrong now. Please try again or restart the program");
	}
}