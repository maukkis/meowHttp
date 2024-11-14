#ifndef _WEBSOCKET_H
#define _WEBSOCKET_H
#include "client.h"
#include "enum.h"
namespace meowHttp {


enum class options{
  URL,
  POSTFIELDS,
};
class websocket : sslSocket{
public:
  meow perform();
  template<options T>
  meow setOpt(const std::string& option){
    switch(T){
    case options::URL:
      url = option; 
    break;
    }
    return OK;
  }

private:
  std::string url;
};
}
#endif
