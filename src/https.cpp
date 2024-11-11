#include "includes/https.h"
#include "includes/enum.h"
#include <openssl/ssl.h>
#include <iostream>
https::https(){ 
}
std::string_view parseBody(std::string_view meow){
  size_t startPos{meow.find("\r\n\r\n") + strlen("\r\n\r\n")};
  return meow.substr(startPos);
}

/*
 size_t parseStatusCode(std::string_view meow){
  size_t startPos{0};
  size_t endPos{meow.find("\r\n")};
  std::string status{meow.substr(startPos, endPos)};
  std::istringstream bark{status};
  bark.ignore(256, ' ');
  size_t httpStatusCode;
  bark >> httpStatusCode;
  return httpStatusCode; 
}*/

meow https::perform(){
  // parse url
  std::string protocol {url.substr(0, url.find("://"))};
  std::string hostname = url.substr(url.find("://") + strlen("://"));
  std::string path = hostname.substr(hostname.find("/"));
  hostname = hostname.substr(0, hostname.length() - path.length());
  if(connect(hostname, protocol) != OK){
    log(ERROR, "failed to connect");
    throw "woof";
  }
  log(INFO, "connected");
  if(initializeSsl() != OK){
    log(ERROR, "failed to initializeSsl");
    throw "meoow";
  }
  SSL_set_fd(ssl, sockfd);
  size_t nya = SSL_connect(ssl);
  if(nya != 1){
    std::cout << SSL_get_error(ssl, nya) << '\n';
    throw "meow";
  }
  if (!BIO_socket_nbio(sockfd, 1)) {
    sockfd = -1;
  }
  std::string meow = "connection using: ";
  meow.append(SSL_get_cipher(ssl));
  log(INFO, meow);
  std::string request {"GET " + path + " HTTP/1.1\r\nHost: " + hostname + "\r\nUser-Agent: sigma skibidi browser\r\nAccept: application/json,text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\n\r\n"};
  size_t sentLen = write(request.c_str(), request.length());
  if(sentLen < 1){
    log(ERROR, "failed to send");
    throw "woof";
  }
  log(INFO, "sent headers");
  std::string buffer;
  size_t rlen = read(buffer, 8192);
  if(rlen < 1){
    log(ERROR, "failed to receive");
    throw "woof";
  }
  std::cout << parseBody(buffer);
  return OK;
}
