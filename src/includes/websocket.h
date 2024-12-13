#ifndef _WEBSOCKET_H
#define _WEBSOCKET_H
#include "client.h"
#include "enum.h"
namespace meowWs {


enum opCodes{
  meowWS_TEXT,
  meowWS_PING,
  meowWS_CLOSE,
  meowWS_PONG,
};

struct meowWsFrame {
  // placeholder value
  // size_t bytesLeft;
  enum opCodes opCode;
  bool fragmented = false;
};
enum class options{
  URL,
  POSTFIELDS,
};
class Websocket : sslSocket{
public:
  ~Websocket();
  meow perform();
  size_t wsRecv(std::string& buf, struct meowWsFrame *frame);
  size_t wsSend(const std::string& payload, opCodes opCode);
  Websocket &setUrl(const std::string& url);
private:
  std::string url;
  std::string *moreData = nullptr;
};
}
#endif
