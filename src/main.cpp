#include "includes/websocket.h"
#include <cstring>
#include <iostream>
#include <iostream>
#include <string>
/* example code */
int main(){
  meowWs::websocket websocket;
  websocket.setOpt<meowWs::options::URL>("https://url.uwu");
  if(websocket.perform() != OK){
    return 1;
  }
  websocket.wsSend("woofs at you", meowWs::meowWS_TEXT);
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

