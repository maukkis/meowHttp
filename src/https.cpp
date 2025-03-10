#include "includes/https.h"
#include "includes/client.h"
#include "includes/enum.h"
#include <openssl/conf.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unistd.h>


namespace meowHttp {

Https https() {
    return Https();
}


Https &Https::setUrl(const std::string& url){
  this->url = url;
  return *this;
}

Https &Https::setWriteData(std::string *writeData){
  this->writeData = writeData;
  return *this;
}

Https &Https::setPostfields(const std::string& post){
  postFields = post;
  return *this;
}

enum HTTPCODES Https::getLastStatusCode(){
  return static_cast<HTTPCODES>(lastStatusCode);
}

std::string parseHeaders(const std::string& buffer, const std::string& headerToParse){
  size_t startpos = buffer.find(headerToParse) + headerToParse.length();
  if (startpos != std::string::npos){
    size_t endPos = buffer.find("\r\n", startpos);
    return buffer.substr(startpos, endPos - startpos);
  }
  return "";
}

std::string parseBody(std::string meow){
  size_t startPos{meow.find("\r\n\r\n") + strlen("\r\n\r\n")};
  if(parseHeaders(meow, "Transfer-Encoding: ") == "chunked"){
    std::string parsedBuffer;
    std::string woofs = meow.substr(startPos, meow.find("\r\n", startPos) - startPos);
    std::stringstream meows;
    size_t woof = std::stoul(woofs, nullptr, 16);
    parsedBuffer.append(meow.substr(meow.find("\r\n", startPos) + strlen("\r\n"), woof));
    return parsedBuffer;
  }
  return meow.substr(startPos);
}

size_t parseStatusCode(std::string_view meow){
  size_t startPos{0};
  size_t endPos{meow.find("\r\n")};
  std::string status{meow.substr(startPos, endPos)};
  std::istringstream bark{status};
  bark.ignore(256, ' ');
  size_t httpStatusCode;
  bark >> httpStatusCode;
  return httpStatusCode; 
}

meow Https::perform(){
  // parse url
  std::string protocol = url.substr(0, url.find("://"));
  std::string hostname = url.substr(url.find("://") + strlen("://"));
  size_t pathPos = hostname.find("/");
  std::string path;
  if (pathPos != std::string::npos) {
    path = hostname.substr(pathPos); 
    hostname = hostname.substr(0, hostname.length() - path.length());
  } 
  else {
    path = '/';
  }
  if(connect(hostname, protocol) != OK){
    log(ERROR, "failed to connect");
    return ERR_CONNECT_FAILED;
  }
  log(INFO, "connected");
  if(initializeSsl() != OK){
    log(ERROR, "failed to initializeSsl");
    return ERR_SSL_FAILED;
  }
  SSL_set_tlsext_host_name(ssl, hostname.c_str());
  SSL_set_fd(ssl, sockfd);
  size_t nya = SSL_connect(ssl);
  if(nya != 1){
    std::cout << SSL_get_error(ssl, nya) << '\n';
    return ERR_SSL_FAILED;
  }
  if (!BIO_socket_nbio(sockfd, 1)) {
    sockfd = -1;
  }
  std::string meow = "connection using: ";
  meow.append(SSL_get_cipher(ssl));
  log(INFO, meow);
  std::string request;
  if (!postFields){
    request = "GET " + path + " HTTP/1.1"
    "\r\nHost: " + hostname + 
    "\r\nUser-Agent: meow browser"
    "\r\nAccept: */*\r\n\r\n";
  }
  else {
    request = "POST " + path + " HTTP/1.1"
    "\r\nHost: " + hostname +
    "\r\nUser-Agent:  meow browser"
    "\r\nAccept: */*"
    "\r\nContent-Type: application/json"
    "\r\nContent-Length: " + std::to_string(postFields->length())
    + "\r\n\r\n" 
    + *postFields;
  }
  size_t sentLen = write(request, request.length());
  if(sentLen < 1){
    log(ERROR, "failed to send");
    return ERR_SEND_FAILED;
  }
  log(INFO, "sent headers");
  std::string buffer;
  size_t rlen = read(buffer);
  while(rlen < 1){
    rlen = read(buffer);
  }
  lastStatusCode = parseStatusCode(buffer);
  if(writeData){
    *writeData = parseBody(buffer);
  }
  else{
    std::cout << parseBody(buffer);
  }
  if(postFields){
    postFields->resize(0);
    postFields->shrink_to_fit();
    postFields.reset();
  }
  close(); 
  return OK;
}


}
