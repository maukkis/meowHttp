#ifndef _CLIENT_H
#define _CLIENT_H

#include <cstdint>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "enum.h"
#include <string>
#include <exception>
namespace meowHttp {
class Exception : std::exception {
public:
  Exception(const std::string& a, bool b = false) : msg{a}, close{b} {}
  const char *what() const throw() {
    return msg.c_str();
  }
  bool closed() const  {
    return close;
  }
private:
  std::string msg;
  bool close = false;;
};
};

class sslSocket{
protected:
  meow initializeSsl();
  enum log{
    INFO,
    ERROR,
  };
  in_addr_t resolveHostName(const std::string& hostname, const std::string& protocol);
  meow connect(const std::string& url, const std::string& protocol, size_t port = 443);
  ssize_t read(std::string& buf);
  ssize_t write(const std::string& data, ssize_t buffersize);
  ssize_t write(const void* data, ssize_t buffersize);
  void log(log meow, const std::string& message);
  const std::string logEnumToString(enum log meow);
  void enableLogging(){ bark = true; }
  void close();
  int sockfd;
  SSL *ssl;
  bool bark = false;
  const SSL_METHOD *method;
  SSL_CTX *ctx;
};
#endif
