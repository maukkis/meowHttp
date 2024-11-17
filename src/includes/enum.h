#ifndef _ENUM_H
#define _ENUM_H
enum meow{
  OK = 0,
  ERR_SSL_FAILED = 1,
  ERR_CONNECT_FAILED,
  ERR_RECEIVE_FAILED,
  ERR_SEND_FAILED,
};
enum HTTPCODES{
  HTTP_OK = 200,
  NotFound = 404,
  Unathorized = 401,
};
#endif
