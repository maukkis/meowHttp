#ifndef _WEBSOCKET_H
#define _WEBSOCKET_H
#include "client.h"
#include "enum.h"
#include <cstdint>
#include <optional>
namespace meowWs {


enum opcodes{
  meowWS_TEXT,
  meowWS_BINARY,
  meowWS_PING,
  meowWS_CLOSE,
  meowWS_PONG,
};

struct meowWsFrame {
  size_t frameLen;
  size_t payloadLen;
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
  meow wsClose(const uint16_t closeCode = 1000, const std::string& aa = "");
  size_t wsRecv(std::string& buf, struct meowWsFrame *frame);
  size_t wsSend(const std::string& payload, opcodes opCode);
  Websocket &setUrl(const std::string& url);
private:
  std::string url;
  std::optional<std::string> moreData;
  int parseWs(std::string& buf, meowWsFrame* frame, size_t len);
};
}
#endif
