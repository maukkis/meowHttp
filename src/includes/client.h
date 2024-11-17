#ifndef _CLIENT_H
#define _CLIENT_H

#include <openssl/ssl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "enum.h"
#include <string>

class sslSocket{
protected:
  meow initializeSsl();
  enum log{
    INFO,
    ERROR,
  };
  in_addr_t resolveHostName(const std::string& hostname, const std::string& protocol);
  meow connect(const std::string& url, const std::string& protocol);
  size_t read(std::string& buf, size_t buffersize, size_t timeout);
  size_t write(const std::string& data, size_t buffersize);
  void log(log meow, const std::string& message);
  const std::string logEnumToString(enum log meow);
  void close();
  int sockfd;
  SSL *ssl;
  const SSL_METHOD *method;
  SSL_CTX *ctx;
};
#endif
