#ifndef _WEBSOCKET_H
#define _WEBSOCKET_H
#include "client.h"
#include "enum.h"
namespace meowWs {


enum opcodes{
  meowWS_TEXT,
  meowWS_PING,
  meowWS_CLOSE,
  meowWS_PONG,
};

struct meowWsFrame {
  // placeholder value
  // size_t bytesLeft;
  enum opcodes opcode;
  bool fragmented = false;
};
enum class options{
  URL,
  POSTFIELDS,
};
class Websocket : sslSocket{
public:
  using sslSocket::enableLogging;
  ~Websocket();
  meow perform();
  meow wsClose(const size_t closeCode = 1000, const std::string& aa = "");
  size_t wsRecv(std::string& buf, struct meowWsFrame *frame);
  size_t wsSend(const std::string& payload, opcodes opCode);
  Websocket &setUrl(const std::string& url);
private:
  std::string url;
  std::string *moreData = nullptr;
};
}
#endif
