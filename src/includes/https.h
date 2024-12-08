#ifndef _HTTPS_H
#define _HTTPS_H
#include "client.h"
#include "enum.h"
namespace meowHttp {

class Https : private sslSocket{
public:
  Https &setWriteData(std::string *writeData);
  Https &setUrl(const std::string& url);
  Https &setPostfields(const std::string& post);
  meow perform();
private:
  std::string url;
  std::string *writeData = nullptr;
  std::string postFields;
  size_t lastStatusCode;
  bool hasPost = false;
};
Https https();
}
#endif
