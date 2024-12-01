#include "includes/https.h"
#include "includes/websocket.h"
#include <string>
#include <iostream>
/* example code */


int main(){
  auto websocket = meowWs::Websocket()
    .setUrl("https://echo.websocket.org");
  if(websocket.perform() != OK){
    return 1;
  }
  std::string buf;
  while(true){
    if(websocket.wsRecv(buf, 8192) >= 1){
      std::cout << "received data breaking\n";
      break;
    } 
  }
  std::cout << buf << '\n';
  buf.resize(0);
  websocket.wsSend("ping!", meowWs::meowWS_PING);
  while(true){
    if(websocket.wsRecv(buf, 8192) >= 1){
      std::cout << "received data breaking\n";
      break;
    } 
  }
  std::cout << buf << '\n'; 
  return 0;
}

