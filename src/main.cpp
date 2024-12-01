#include "includes/https.h"
#include "includes/websocket.h"
#include <string>
#include <iostream>
/* example code */



/*
int main(int argc, char **argv){
  if (argc < 2){
    return 1;
  }
  meowIrc::irc irc;
  irc.setOpt<meowIrc::options::URL>(argv[2]);
  irc.setOpt<meowIrc::options::USERNAME>("lunaMeow");
  irc.perform();
  while (true){
    std::string buf1;
    size_t rlen = irc.readMeow(buf1, 8192);
    if(rlen > 0){
      std::stringstream woofs;
      woofs.str(buf1);
      std::string buf;
        while(std::getline(woofs, buf)){
        if(buf.find("PRIVMSG") != std::string::npos){
          size_t exclamationMarkPos = buf.find('!');
          std::string username = buf.substr(1, exclamationMarkPos - 1);
          size_t channelStart = buf.find("PRIVMSG") + strlen("PRIVMSG ");
          size_t channelEnd = buf.find(' ', channelStart);
          std::string channel = buf.substr(channelStart, channelEnd - channelStart);
          size_t messageStart = buf.find(':', channelEnd) + 1;
          std::string message = buf.substr(messageStart, buf.find("\r\n") - messageStart);
          std::cout << channel << ' ' << username << ' ' << message << '\n';
          continue;
        }
          size_t meow = buf.find(":" + irc.url);
          if (meow != std::string::npos) {
            size_t messageStart = buf.find(':', meow + irc.url.length() + 1) + 1;
            if (messageStart != std::string::npos) {
              size_t messageEnd = buf.find("\r\n", messageStart);
              std::cout << buf.substr(messageStart, messageEnd - messageStart) << '\n';
            }
          }
        if(buf.find("PING ") != std::string::npos){
          std::cout << "got ping!\n";
          irc.writeMeow("PONG irc.atl.chat\r\n", strlen("PONG irc.atl.chat\r\n"));
        }
      }
    }
  }
}*/


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

