#include "includes/https.h"
#include <iostream>
/* example code */
int main(){
  meowHttp::https meow;
  meow.setOpt<meowHttp::options::URL>("https://url.here/paws/at/you");
  if(meow.perform() != OK){
    std::cout << "failed to receive\n";
    return 1;
  }
  std::cout << "status code: " << meow.getOpt<meowHttp::getOptions::STATUSCODE>() << '\n';
  /*
  meowHttp::websocket websocket;
  websocket.setOpt<meowHttp::options::URL>("https://echo.websocket.org/");
  websocket.perform();
  */
  return 0;
}

