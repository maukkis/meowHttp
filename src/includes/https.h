#ifndef _HTTPS_H
#define _HTTPS_H
#include "client.h"
#include "enum.h"
namespace meowHttp {

enum class options{
  URL,
  POSTFIELDS,
};
enum class getOptions{
  STATUSCODE,
};
class https : private sslSocket{
public:
  template<options T>
  meow setOpt(const std::string& option){
    switch(T){
    case options::URL:
        url = option; 
      break;
    case options::POSTFIELDS:
        postFields = new std::string{option};
    break;
    }
    return OK;
  }
  template<options T>
  meow setOpt(std::string *option){
    switch(T){
    case options::URL:
        url = *option; 
      break;
    case options::POSTFIELDS:
        postFields = option;
    break;
    }
    return OK;
  }
  template<getOptions T>
  auto getOpt(){
    switch(T){
    case getOptions::STATUSCODE:
      return lastStatusCode;
    break;
    }
  }
  meow perform();
private:
  std::string url;
  std::string *postFields = nullptr;
  size_t lastStatusCode;
};
}
#endif
