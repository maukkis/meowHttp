#include "includes/https.h"
#include <iostream>
/* example code */
int main(){
  meowHttp::https meow;
  meow.setOpt<meowHttp::options::URL>("url!");
  std::string postFields = R"({"content": "barks!","username": "woofs"})";
  meow.setOpt<meowHttp::options::POSTFIELDS>(&postFields); // change request type to post
  meow.perform();
  std::cout << "status code: " << meow.getOpt<meowHttp::getOptions::STATUSCODE>() << '\n'; // get status code
  return 0;
}

