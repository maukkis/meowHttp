#include "../src/includes/https.h"
#include <string>

int main(){
  auto https = meowHttp::https()
    .setUrl("https://echo.girlsmell.xyz");
  if(https.perform() != OK){
    return 1;
  }
  if(https.getLastStatusCode() != HTTP_OK){
    return 1;
  }
  return 0;
}
