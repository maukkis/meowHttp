#ifndef _WEBSOCKET_H
#define _WEBSOCKET_H
#include "client.h"
#include "enum.h"
namespace meowWs {
enum opCodes{
  meowWS_TEXT,
  meowWS_PING,
};

enum class options{
  URL,
  POSTFIELDS,
};
class websocket : sslSocket{
public:
  meow perform();
  size_t wsRecv(std::string& buf, size_t bufSize);
  size_t wsSend(const std::string& payload, opCodes opCode);
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
