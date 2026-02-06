#include "includes/https.h"
#include "includes/client.h"
#include "includes/enum.h"
#include <algorithm>
#include <exception>
#include <expected>
#include <openssl/conf.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <iostream>
#include <optional>
#include <print>
#include <sstream>
#include <string>
#include <string_view>


namespace meowHttp {
namespace {
enum errors {
  MissingData,
  ReadTillClosed,
};
void lowerString(std::string& a){
  std::transform(a.begin(), a.end(), a.begin(), [](char c){
    return std::tolower(c);
  });
}
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
  sheaders.insert({"content-length: ", std::to_string(post.length())});
  return *this;
}

Https &Https::setHeader(const std::string_view header){
  ssize_t colon = header.find(": ");
  const std::string_view key = header.substr(0, colon+2);
  const std::string_view value = header.substr(colon+2);
  sheaders[std::string(key)] = std::string(value);
  return *this;
}

Https &Https::setCustomMethod(const std::string& method){
  customMethod = method;
  return *this;
}

enum HTTPCODES Https::getLastStatusCode(){
  return static_cast<HTTPCODES>(lastStatusCode);
}

void parseHeaders(std::string& buffer, 
                  std::unordered_map<std::string, std::string>& headermap){
  std::istringstream a{buffer};
  std::string temp;
  // skip first line
  std::getline(a, temp);
  while(std::getline(a, temp)){
    std::erase(temp, '\r');
    size_t pos = temp.find(": ");
    std::string key = temp.substr(0, pos);
    lowerString(key);
    std::string value = temp.substr(pos+2);
    headermap.insert({std::move(key), std::move(value)});
  }
}

std::expected<std::string, enum errors> parseBody(const std::string& meow,
                                                  std::unordered_map<std::string, std::string>& map,
                                                  bool readAll = false){
  auto end = meow.find("\r\n\r\n");
  if(end == std::string::npos) return std::unexpected(MissingData);
  size_t startPos{end + 4};
  std::string headers = meow.substr(0, startPos-4);
  parseHeaders(headers, map);
  if(map.contains("content-length")){
    size_t len = std::stoul(map["content-length"], nullptr, 10);
    if(meow.length() - startPos < len){
      return std::unexpected(MissingData);
    }
    return meow.substr(startPos, len);
  }
  if(map.contains("transfer-encoding") && map["transfer-encoding"] == "chunked"){
    std::string parsedBuffer;
    std::string_view a = meow;
    a = a.substr(startPos);
    parsedBuffer.reserve(a.length());
    size_t woof;
    try {
      do {
        size_t pos = a.find("\r\n");
        if(pos == std::string::npos) return std::unexpected(MissingData);
        std::string woofs = std::string(a.substr(0, pos));
        woof = std::stoul(woofs, nullptr, 16);
        if(woof > a.length() - 2) return std::unexpected(MissingData);
        if(woof + 2 > a.length()) return std::unexpected(MissingData);
        if(woof < 1) break;
        parsedBuffer.append(a.substr(pos + 2, woof));
        a = a.substr(pos + 4 + woof); // 4 is here because a chunk is followed by a \r\n which we have to skip over
      } while(woof > 0);
    } catch(std::exception& e){
      return std::unexpected(MissingData);
    }
    return parsedBuffer;
  }
  if(!readAll) return std::unexpected(ReadTillClosed);
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
  if(ssl) close();
  // parse url
  int port = 443;
  std::string protocol = url.substr(0, url.find("://"));
  std::string hostname = url.substr(url.find("://") + strlen("://"));
  size_t pathPos = hostname.find("/");
  std::string path;
  if (pathPos != std::string::npos) {
    sheaders.insert({"host: ", hostname.substr(0, pathPos)});
    path = hostname.substr(pathPos); 
    hostname = hostname.substr(0, hostname.length() - path.length());
  } 
  else {
    sheaders.insert({"host: ", hostname});
    path = '/';
  }
  if(size_t pos = hostname.find(":"); pos != std::string::npos){
    std::string portStr = hostname.substr(hostname.find(":")+1);
    hostname = hostname.substr(0, hostname.find(":"));
    port = std::stoi(portStr);
  }
  if(connect(hostname, protocol, port) != OK){
    log(ERR, "failed to connect");
    return ERR_CONNECT_FAILED;
  }
  log(INFO, "connected");
  if(initializeSsl() != OK){
    log(ERR, "failed to initializeSsl");
    return ERR_SSL_FAILED;
  }
  SSL_set_tlsext_host_name(ssl, hostname.c_str());
  SSL_set_fd(ssl, sockfd);
  size_t nya = SSL_connect(ssl);
  if(nya != 1){
    std::cout << SSL_get_error(ssl, nya) << '\n';
    freeSSL();
    return ERR_SSL_FAILED;
  }
  if (!BIO_socket_nbio(sockfd, 1)) {
    sockfd = -1;
  }
  std::string meow = "connection using: ";
  meow.append(SSL_get_cipher(ssl));
  log(INFO, meow);
  std::string request;
  if(!postFields && !customMethod)
    request = "GET " + path + " HTTP/1.1\r\n";
  else if(postFields && !customMethod)
    request = "POST " + path + " HTTP/1.1\r\n";
  else 
    request = *customMethod + ' ' + path + " HTTP/1.1\r\n";
  for(auto& a : sheaders){
    request += a.first + a.second + "\r\n";
  }
  request += "\r\n";
  if(postFields){
    request += *postFields;
  }
  try {
    ssize_t sentLen = write(request, request.length());
    
    if(sentLen < 1){
      log(ERR, "failed to send");
      return ERR_SEND_FAILED;
    }
  } catch(meowHttp::Exception&){
    return ERR_SEND_FAILED;
  }
  log(INFO, "sent headers");
  std::string buffer;
  // TODO: fix this hacky piece of shit by making sslSocket::read() not fucking throw
  try {
    size_t rlen = read(buffer);
    while(rlen < 1){
      rlen = read(buffer);
    }
    lastStatusCode = parseStatusCode(buffer);
    bool readAll = false;
    while(true){
      auto a = parseBody(buffer, this->headers, readAll);
      if(a.has_value()){
        if(writeData)
          *writeData = std::move(a.value());
        else
          std::cout << a.value();
        break;
      }
      if(a.error() == MissingData){
        read(buffer);
      }
      else if(a.error() == ReadTillClosed){
        readTillClosed(buffer);
        readAll = true;
      }
    }
  } catch(meowHttp::Exception& e){
    log(ERR, e.what());
    std::println("{}", e.what());
    return ERR_RECEIVE_FAILED;
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
