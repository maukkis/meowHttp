# meowHttp
a simple C++ library to send https and wss requests<br>
# Dependencies
- openssl
# Usage
## https get request
```cpp
#include "includes/https.h"

int main(){
  auto meow = meowHttp::Https()
    .setUrl("https://example.tld");
  meow.perform();
  return 0;
}
```
## https post request
```cpp
#include "includes/https.h"

int main(){
  auto meow = meowHttp::Https()
    .setUrl("https://url.here")
    .setPostfields("post here");
  meow.perform();
  return 0;
}
```
## websockets
```cpp
#include "includes/websocket.h"
#include <iostream>

int main(){
  auto websocket = meowWs::Websocket()
    .setUrl("https://url here nya");
  if(websocket.perform() != OK){
    return 1;
  }
  std::string buf;
  struct meowWs::meowWsFrame frame;
  while(true){
    if(websocket.wsRecv(buf, &frame) >= 1){
      std::cout << "received data breaking\n";
      break;
    } 
  }
  std::cout << buf << '\n';
  buf.resize(0);
  websocket.wsSend("ping!", meowWs::meowWS_PING);
  while(true){
    if(websocket.wsRecv(buf, &frame) >= 1){
      std::cout << "received data breaking\n";
      break;
    } 
  }
  std::cout << buf << '\n'; 
  return 0;
}
```

