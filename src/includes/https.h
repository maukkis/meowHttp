#ifndef _HTTPS_H
#define _HTTPS_H
#include "client.h"
#include "enum.h"
#include <optional>
#include <unordered_map>

namespace meowHttp {

class Https : private sslSocket{
public:
  using sslSocket::enableLogging;
  Https &setWriteData(std::string *writeData);
  Https &setUrl(const std::string& url);
  Https &setPostfields(const std::string& post);
  Https &setHeader(const std::string_view header);
  Https &setCustomMethod(const std::string& method);
  enum HTTPCODES getLastStatusCode();
  meow perform();
  std::unordered_map<std::string, std::string> headers;
private:
  std::string url;
  std::string *writeData = nullptr;
  std::optional<std::string> postFields;
  size_t lastStatusCode;
  std::unordered_map<std::string, std::string> sheaders{
    {"accept: ", "*/*"},
    {"user-agent: ", "meowHttp"},
  };
  std::optional<std::string> customMethod;
};
}
#endif
