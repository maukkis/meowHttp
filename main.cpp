#include <cstring>
#include <iostream>
#include <string_view>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

std::string_view parseBody(std::string_view meow){
  size_t startPos{meow.find("\r\n\r\n") + strlen("\r\n\r\n")};
  return meow.substr(startPos);
}


int main () {
  int sockfd{socket(AF_INET, SOCK_STREAM, 0)};
  struct sockaddr_in *meow = new sockaddr_in;
  meow->sin_port = htons(80);
  meow->sin_family = AF_INET;
  meow->sin_addr.s_addr = inet_addr("146.190.62.39");
  if(connect(sockfd, (sockaddr*)meow, sizeof(*meow)) != 0){
    std::cout << "failed to connect\n";
    close(sockfd);
    return 1;
  }
  std::cout << "connected!\n";
  std::string request {"GET /index.html HTTP/1.1\r\nHost: 146.190.62.39\r\nUser-Agent: sigma skibidi browser\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\n\r\n"};
  ssize_t nya = send(sockfd, request.c_str(), request.length(), 0);
  if (nya <= 0){
    std::cout << "failed to send\n";
    close(sockfd);
    return 1;
  }
  std::cout << "sent headers: " << nya << " bytes\n";
  char *buffer = (char *)malloc(sizeof(char) * 4096);
  size_t bufferSize = 4096;
  ssize_t woof = recv(sockfd, buffer, 4096-1, 0);
  if (woof <= 0){
    std::cout << "didnt receive\n";
    close(sockfd);
    return 1;
  }
  std::cout << "received: " << woof << " bytes\n";
  if (woof == 4096-1){
    for(size_t nya;; ++nya){
      char buffer1[4096];
      ssize_t receiveSize = recv(sockfd, buffer1, 4096-1, 0);
      if(realloc(buffer, bufferSize + receiveSize) == 0){
        std::cout << "woof\n";
      }
      bufferSize += receiveSize;
      std::cout << "received: " << receiveSize << " bytes\n";
      woof += receiveSize;
      strncat(buffer, buffer1, receiveSize);
      if (receiveSize < 0){
        std::cout << "did not receive\n";
        break;
      }
      if(receiveSize < 4096-1){
        break;
      }
    }
  }
  buffer[woof] = '\0';
  std::cout << parseBody(buffer) << '\n';
  free(buffer);
  free(meow);
  return 0;
}
