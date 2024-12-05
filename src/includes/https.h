#ifndef _HTTPS_H
#define _HTTPS_H
#include "client.h"
#include "enum.h"
namespace meowHttp {

class Https : private sslSocket{
public:
  ~Https(){
    if(allocated){
      delete postFields;
    }
  }
  Https &setWriteData(std::string *writeData);
  Https &setUrl(const std::string& url);
  Https &setPostfields(std::string *post);
  Https &setPostfields(const std::string& post);
  meow perform(size_t timeout = 50);
private:
  std::string url;
  std::string *writeData = nullptr;
  std::string *postFields = nullptr;
  size_t lastStatusCode;
  bool allocated = false;
};
Https https();
}
#endif
