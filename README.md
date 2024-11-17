# meowHttp
a simple C++ library to send https and wss requests<br>
# Dependencies
- openssl
# Usage
## https get request
```cpp
#include "includes/https.h"

int main(){
  meowHttp::https meow;
  meow.setOpt<meowHttp::options::URL>("https://url.com/here");
  meow.perform();
  return 0;
}
```
## https post request
```cpp
#include "includes/https.h"

int main(){
  meowHttp::https meow;
  meow.setOpt<meowHttp::options::URL>("https://url.com/here");
  const std::string postFields = "post stuff here";
  meow.setOpt<meowHttp::options::POSTFIELDS>(postFields);
  meow.perform();
  return 0;
}
```
## websockets
```cpp
#include "includes/websocket.h"
#include <iostream>
#include <string>

int main(){
  meowWs::websocket websocket;
  websocket.setOpt<meowWs::options::URL>("https://url.here/path");
  if(websocket.perform() != OK){
    return 1;
  }
  websocket.wsSend("woofs at you");
  std::string buf;
  while(true){
    if(websocket.wsRecv(buf, 8192) >= 1){
      std::cout << "received data breaking\n";
      break;
    } 
  }
  std::cout << buf << '\n';
  return 0;
}
```

