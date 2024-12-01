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
class Websocket : sslSocket{
public:
  meow perform();
  size_t wsRecv(std::string& buf, size_t bufSize);
  size_t wsSend(const std::string& payload, opCodes opCode);
  Websocket &setUrl(const std::string& url);
private:
  std::string url;
  std::string *moreData = nullptr;
};
}
#endif
