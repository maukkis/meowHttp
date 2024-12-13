#include "../src/includes/https.h"

int main(){
  std::string writeData;
  const char postData[] = "mrrp mrrp mraoow!";
  auto https = meowHttp::https()
    .setUrl("https://echo.girlsmell.xyz")
    .setPostfields(postData)
    .setWriteData(&writeData);
  if(https.perform() != OK || https.getLastStatusCode() != HTTP_OK){
    return 1;
  }
  size_t pos = writeData.find(postData);
  if(pos != std::string::npos){
    writeData = writeData.substr(pos);
  }
  else{
    return 1;
  }
  if(writeData != postData){
    return 1;
  }
  return 0;
}
