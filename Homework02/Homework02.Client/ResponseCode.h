#ifndef RESPONSE_CODE_H
#define RESPONSE_CODE_H

#define MESSAGE_TYPE_NOT_EXIST "99"
#define MESSAGE_INVALID_FORMAT "98"
#define USER_LOGIN_OK "10"
#define USER_ACCOUNT_NOT_ACTIVE "11"
#define USER_ACCOUNT_NOT_EXIST "12"
// user login to an account which has been login-ed in another client
#define USER_ACCOUNT_ALREADY_LOGIN "13"
// user've login-ed then login to the same account
#define USER_LOGINED "14"
// user've login-ed then login to another account
#define USER_LOGINED_DIFFERENT_ACCOUNT "15"
#define POST_OK "20"
#define POST_ACCOUNT_NOT_LOGIN_YET "21"
#define BYE_OK "30"

#endif // !RESPONSE_CODE_H
