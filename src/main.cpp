#include "includes/https.h"
#include <iostream>
/* example code */
int main(){
  meowHttp::https meow;
  meow.setOpt<meowHttp::options::URL>("url here");
  if(meow.perform() != OK){
    std::cout << "failed to receive\n";
    return 1;
  }
  std::cout << "status code: " << meow.getOpt<meowHttp::getOptions::STATUSCODE>() << '\n';
  return 0;
}

