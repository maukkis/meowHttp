#include "../src/includes/websocket.h"

int main(){
  auto websocket = meowWs::Websocket()
    .setUrl("https://echo.websocket.org/");
  if(websocket.perform() != OK){
    return 1;
  }
  struct meowWs::meowWsFrame frame;
  std::string buf;
  if(websocket.wsRecv(buf, &frame) < 1){
    return 1;
  }
  buf.clear();
  constexpr char payload[] = "mrrrp mrpp meoow!";
  if(websocket.wsSend(payload, meowWs::meowWS_TEXT) < 1){
    return 1;
  }
  while(true){
    if(websocket.wsRecv(buf, &frame) > 0){
      break;
    }
  }
  if(frame.opCode == meowWs::meowWS_TEXT && buf == payload){
    return 0;
  }
  return 1;
}
