#ifndef _HTTPS_H
#define _HTTPS_H
#include "client.h"
#include "enum.h"
class https : private sslSocket{
public:
  enum options{
    URL,
    POSTFIELDS,
  };
  https();
  template<https::options T>
  meow setOpt(const std::string& option){
    switch(T){
      case URL:
        url = option; 
      break;
      case POSTFIELDS:
        log(ERROR, "TODO");
      break;
    }
    return OK;
  }
  meow perform();
private:
  std::string url;

};

#endif
