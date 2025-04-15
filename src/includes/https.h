#ifndef _HTTPS_H
#define _HTTPS_H
#include "client.h"
#include "enum.h"
#include <optional>
#include <vector>

namespace meowHttp {

class Https : private sslSocket{
public:
  using sslSocket::enableLogging;
  Https &setWriteData(std::string *writeData);
  Https &setUrl(const std::string& url);
  Https &setPostfields(const std::string& post);
  Https &setHeader(const std::string& header);
  Https &setCustomMethod(const std::string& method);
  enum HTTPCODES getLastStatusCode();
  meow perform();
private:
  std::string url;
  std::string *writeData = nullptr;
  std::optional<std::string> postFields;
  size_t lastStatusCode;
  std::vector<std::string> headers;
  std::optional<std::string> customMethod;
};
}
#endif
