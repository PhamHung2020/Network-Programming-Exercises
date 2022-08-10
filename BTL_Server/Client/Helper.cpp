#include "Helper.h"



int Helper::checkCommandLineArgument(int argc, char* argv[]) {
	if (argc != 3)
	{
		printf("This program accepts 2 arguments: IP address and port number");
		return -1;
	}

	int portNumber;
	try
	{
		portNumber = std::stoi(argv[2]);
	}
	catch (const std::invalid_argument&)
	{
		printf("The second argument must be a number, which is a port number");
		return -1;
	}

	if (portNumber < 0 || portNumber > 65536)
	{
		printf("Port number must be in range [0, 65536]");
		return -1;
	}
	return portNumber;
}


int Helper::sendMessage(std::string message)
{
	// this code is inspired by Mr.Tung's lecture
	message += ENDING_DELIMITER;
	int leftBytes = message.length();
	int index = 0, ret = 0;
	while (leftBytes > 0)
	{
		int numCopiedChar = message.copy(buff, BUFF_SIZE, index);
		buff[numCopiedChar] = 0;
		ret = send(client, buff, numCopiedChar, 0);
		if (ret == SOCKET_ERROR)
		{
			index = ret;
			break;
		}
		leftBytes -= ret;
		index += ret;
	}
	return index;
}


int Helper::splitString(std::string& str, const char* delimiter, int delimiterLength, std::vector<std::string>& arrStr, int count)
{
	//used to store first occurrence of ending delimiter in message
	int posOfDelimiter = std::string::npos;
	//used to store number of substrings splitted
	int numSubString = 0;

	while (count != 1 && (posOfDelimiter = str.find(delimiter)) != std::string::npos)
	{
		std::string substring = str.substr(0, posOfDelimiter);
		arrStr.push_back(substring);
		++numSubString;
		--count;
		str.erase(0, posOfDelimiter + delimiterLength);
	}
	//the last substring in str after splitting (e.g: 123<delimiter>46 --> 46 is the last substring)
	// or the initial value of input string doesn't contain any delimiter
	if (count != 0 && !str.empty())
	{
		++numSubString;
		arrStr.push_back(str);
	}
	return numSubString;
}



std::string Helper::convertToString(char* a, int size) {
	std::string s = "";
	for (int i = 0; i < size; i++) {
		s = s + a[i];
	}
	return s;
}

bool Helper::isTime(std::string value) {
	// Time HH:mm:ss --> length = 8
	if (value.length() != 8)
		return false;

	if (value[2] != ':' || value[5] != ':')
		return false;

	int hour = (value[0] - '0') * 10 + (value[1] - '0');
	if (hour < 0 || hour > 23)
		return false;

	int minute = (value[3] - '0') * 10 + (value[4] - '0');
	if (minute < 0 || minute > 59)
		return false;

	int second = (value[6] - '0') * 10 + (value[7] - '0');
	if (0 < second || second > 59)
		return false;
	return true;
}

void Helper::getDataOnFile(std::string fileName) {
	std::ifstream myFile(fileName);

	std::string myText;
	while (std::getline(myFile, myText)) {
		std::vector<std::string> res;
		int number = splitString(myText, " ", 1, res, -1);
		if (number == 3) {
			sensor.oxi.push_back(res[0]);
			sensor.ph.push_back(res[1]);
			sensor.salt.push_back(res[2]);
		}
		else {
			Notification::errorMessage("Data is not in the correct format");
		}

	}

	// Close the file 
	myFile.close();
}

void Helper::writeDataOnFile(std::string oxi, std::string ph, std::string salt, std::string fileName) {

	std::ofstream myfile(fileName);
	if (myfile.is_open()) {
		std::string mess = oxi + " " + ph + " " + salt + "\n";
		myfile << mess;
	}
	myfile.close();
};