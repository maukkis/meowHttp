#include "../src/includes/websocket.h"

int main(){
  auto websocket = meowWs::Websocket()
    .setUrl("https://echo.girlsmell.xyz/");
  if(websocket.perform() != OK){
    return 1;
  }
  struct meowWs::meowWsFrame frame;
  std::string buf;
  constexpr char payload[] = "mrrrp mrpp meoow!";
  if(websocket.wsSend(payload, meowWs::meowWS_TEXT) < 1){
    return 1;
  }
  while(true){
    if(websocket.wsRecv(buf, &frame) > 0){
      break;
    }
  }
  if(frame.opcode == meowWs::meowWS_TEXT && buf == payload){
    return 0;
  }
  return 1;
}
