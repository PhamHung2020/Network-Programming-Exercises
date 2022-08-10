#pragma once
#include <string>

const std::string  CLIENT_APPROVED = "01";
const std::string  MESSAGE_TYPE_NOT_EXIST = "99";
const std::string  MESSAGE_INVALID_FORMAT = "98";
const std::string  USER_LOGIN_OK = "10";
const std::string  USER_LOGGED_IN = "11";
const std::string  USER_ACCOUNT_ALREADY_LOGIN = "12";
const std::string  USER_INVALID_CREDENTIALS = "13";
const std::string  PASSWD_OK = "20";
const std::string  USER_NOT_LOGIN = "21";
const std::string  PASSWD_FAILED = "22";
const std::string  PASSWD_PASSWORD_NOT_CHANGE = "23";
const std::string  BYE_OK = "30";
const std::string  SET_OK = "40";
const std::string  SET_INVALID_DEVICE_ID = "41";
const std::string  SET_INVALID_PARAMETER_NAME = "42";
const std::string  SET_INVALID_VALUE = "43";
const std::string  ADDTIME_P_OK = "50";
const std::string  ADDTIME_P_INVALID_DEVICE_ID = "51";
const std::string  ADDTIME_P_INVALID_FORMAT_VALUE = "52";
const std::string  ADDTIME_P_VALUE_ALREADY_EXIST = "53";
const std::string  RMVTIME_P_INVALID_VALUE_ID = "54";
const std::string  ADDTIME_R_OK = "60";
const std::string  ADDTIME_R_INVALID_DEVICE_ID = "61";
const std::string  RMVTIME_R_INVALID_VALUE_ID = "63";
const std::string  GET_OK = "70";
const std::string  GET_INVALID_ID = "71";
const std::string  GET_INVALID_PARAMETER = "72";
const std::string  COMMAND_OK = "80";
const std::string  COMMAND_INVALID_DEVICE_ID = "81";
const std::string  COMMAND_INVALID_PARAMETER = "82";
const std::string  GET_DEVICE_HEADER = "90 device";
const std::string  GET_PARAM_HEADER = "90 param";
