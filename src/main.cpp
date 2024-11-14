#include "includes/https.h"
#include <iostream>
/* example code */
int main(){
  meowHttp::https meow;
  meow.setOpt<meowHttp::options::URL>("url!");
  meow.perform();
  std::cout << "status code: " << meow.getOpt<meowHttp::getOptions::STATUSCODE>() << '\n';
  return 0;
}

