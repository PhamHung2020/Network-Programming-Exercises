#ifndef MESSAGE_SENDER_H
#define MESSAGE_SENDER_H

#include <iostream>
#include <string>
#include <WinSock2.h>
#include "Helper.h"
#include "ResponseCode.h"


namespace MessageSender
{
	/**
	* @function sendUserMessage: sends login message to server.
	*
	* @param connSockPtr: A socket that is connecting with server.
	* @param username: Username for login.
	* @param buff: Application's buffer.
	*
	* @return: number of bytes sent if no error occurs.
	*		   SOCKET_ERROR if an error occurs.
	**/
	int sendLoginMessage(const std::string& username, const std::string& password);


	int sendChangePasswordMessage(const std::string& currentpasswd, const std::string& newpasswd, const std::string& confirmpasswd);

	int sendLogoutMessage();

	int sendGetAllDeviceMessage();

	int sendGetParamOfDevice(std::string& id_device);

	int sendGetValueParam(std::string& id_device, std::string& name, std::string type);

	int sendSetValueParam(std::string idDevice, std::string nameParam, std::string valueParam);

	int sendRemoveTimePoint(std::string idDevice, std::string idTime);

	int sendAddTimePoint(std::string idDevice, std::string time);

	int sendAddTimeRange(std::string idDevice, std::string start, std::string end);

	int sendRemoveTimeRange(std::string idDeive, std::string idTimeRange);

	int sendDeviceControl(std::string idDevice, std::string status);

	int sendHello();
}

#endif