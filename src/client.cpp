#include "includes/client.h"
#include <cstring>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#ifdef WIN32
#define poll WSAPoll
#define closeSock(x) ::closesocket(x)
#define SHUT_RDWR SD_BOTH
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#define closeSock(x) ::close(x)
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <iostream>
#include "includes/enum.h"


meow sslSocket::initializeSsl(){
  method = TLS_client_method(); 
  ctx = SSL_CTX_new(method);
  ssl = SSL_new(ctx);
  if(!ssl){
    log(ERR, "failed to create ssl");
    return ERR_SSL_FAILED;
  }
  return OK;
}

void sslSocket::log(enum log meow, const std::string& message){
  if(bark) std::cout << logEnumToString(meow) << " [*] " << message << '\n';  
}

inline const std::string sslSocket::logEnumToString(enum log meow){
  switch(meow){
    case INFO:
      return "info";
    break;
    case ERR:
      return "error";
    break;
    default:
      return "idk";
    break;
  }
}

void sslSocket::freeSSL(){
  if(ssl){
    SSL_CTX_free(ctx);
    SSL_free(ssl);
    ssl = nullptr;
    ctx = nullptr;
    method = nullptr;
  }
}

void sslSocket::close(){
  if(ssl)
    SSL_shutdown(ssl);
  this->freeSSL();
  shutdown(sockfd, SHUT_RDWR);
  closeSock(sockfd);
}

ssize_t sslSocket::read(std::string& buf){
  if(!ssl) throw meowHttp::Exception("already closed");
  size_t recv = 0;
  size_t meow = 0; 
  bool bark;
  struct pollfd pfd[2];
  while(true){
    pfd[0].fd = sockfd;
    pfd[0].events = POLLIN;
    pfd[1].events = 0;
    int ret = poll(pfd, 1, 5); //check if fd is readable this prevents receiving partial data
    if(ret > 0){
      if(pfd[0].revents & POLLIN){ // if fd is readable read from it till we get an error
        do{
          char buff[8192];
          bark = false;
          recv = SSL_read(ssl, buff, 8192);
          int woof = SSL_get_error(ssl, recv);
          switch(woof){
            case SSL_ERROR_NONE:
              if (recv > 0){
                buf.append(buff, recv);
                meow += recv;
                continue;
              }
            break;
            case SSL_ERROR_ZERO_RETURN:
              close();
              return meow;
            break;
            case SSL_ERROR_WANT_READ:
              bark = true;
            break;
            case SSL_ERROR_SSL:
            case SSL_ERROR_SYSCALL:
              freeSSL();
              closeSock(sockfd);
              return meow;
            default:
              int error = SSL_get_error(ssl,recv);
              std::cout << "error: " << error << '\n';
              close();
              throw(meowHttp::Exception("openssl error", true));
              return meow;
            break;
          }
        } while(SSL_pending(ssl) && !bark);
      }
      else if(
        pfd[0].revents & POLLHUP ||
        pfd[0].revents & POLLERR || 
        pfd[0].revents & POLLNVAL
      ){
        close();
        throw(meowHttp::Exception("connection closed", true));
        break;
      }
    }
    else if (ret == 0){
      break;
    }
    else if(ret < 0){
      throw(meowHttp::Exception("internal poll error", false));
      break;
    }
  }
  return meow;
}

ssize_t sslSocket::readTillClosed(std::string& buf){
  if(!ssl) throw meowHttp::Exception("already closed");
  size_t recv = 0;
  size_t meow = 0; 
  bool bark;
  struct pollfd pfd[2];
  while(true){
    pfd[0].fd = sockfd;
    pfd[0].events = POLLIN;
    pfd[1].events = 0;
    int ret = poll(pfd, 1, -1); //check if fd is readable this prevents receiving partial data
    if(ret > 0){
      if(pfd[0].revents & POLLIN){ // if fd is readable read from it till we get an error
        do{
          char buff[8192];
          bark = false;
          recv = SSL_read(ssl, buff, 8192);
          int woof = SSL_get_error(ssl, recv);
          switch(woof){
            case SSL_ERROR_NONE:
              if (recv > 0){
                buf.append(buff, recv);
                meow += recv;
                continue;
              }
            break;
            case SSL_ERROR_ZERO_RETURN:
              close();
              return meow;
            break;
            case SSL_ERROR_WANT_READ:
              bark = true;
            break;
            case SSL_ERROR_SSL:
            case SSL_ERROR_SYSCALL:
              freeSSL();
              closeSock(sockfd);
              return meow;
            default:
              int error = SSL_get_error(ssl,recv);
              std::cout << "error: " << error << '\n';
              close();
              throw(meowHttp::Exception("openssl error", true));
              return meow;
            break;
          }
        } while(SSL_pending(ssl) && !bark);
      }
      else if(
        pfd[0].revents & POLLHUP ||
        pfd[0].revents & POLLERR || 
        pfd[0].revents & POLLNVAL
      ){
        close();
        break;
      }
    }
    else if (ret == 0){
      break;
    }
    else if(ret < 0){
      throw(meowHttp::Exception("internal poll error", false));
      break;
    }
  }
  return meow;
}


ssize_t sslSocket::write(const std::string& data, ssize_t buffersize){
  if(!ssl) throw meowHttp::Exception("already closed");
  ssize_t sent = 0;
  while(sent < buffersize){
    struct pollfd pfd[2];
    pfd[0].fd = sockfd;
    pfd[0].events = POLLOUT;
    pfd[1].events = 0;
    int ret = poll(pfd, 1, -1);
    if(ret > 0){
      if(pfd[0].revents & POLLOUT){
        ssize_t val = SSL_write(ssl, data.data(), buffersize);
        switch(SSL_get_error(ssl, val)){
          case SSL_ERROR_NONE:
            sent += val;
          break;
          case SSL_ERROR_WANT_READ:
          break;
          case SSL_ERROR_WANT_WRITE:
          break;
          case SSL_ERROR_SSL:
          case SSL_ERROR_SYSCALL:
            freeSSL();
            closeSock(sockfd);
            return sent;
        }
      }
      else if(
        pfd[0].revents & POLLHUP ||
        pfd[0].revents & POLLERR || 
        pfd[0].revents & POLLNVAL
      ) {
        close();
        throw(meowHttp::Exception("connection closed", true));
        break;
      }

    }
    else if(ret == 0){
      break;
    }
    else if(ret == -1){
      close();
      break;
    }
  }
  return sent;
}

ssize_t sslSocket::write(const void* data, ssize_t buffersize){
  if(!ssl) throw meowHttp::Exception("already closed");
  ssize_t sent = 0;
  while(sent < buffersize){
    struct pollfd pfd[2];
    pfd[0].fd = sockfd;
    pfd[0].events = POLLOUT;
    pfd[1].events = 0;
    int ret = poll(pfd, 1, -1);
    if(ret > 0){
      if(pfd[0].revents & POLLOUT){
        ssize_t val = SSL_write(ssl, data, buffersize);
        switch(SSL_get_error(ssl, val)){
          case SSL_ERROR_NONE:
            sent += val;
          break;
          case SSL_ERROR_WANT_READ:
          break;
          case SSL_ERROR_WANT_WRITE:
          break;
          case SSL_ERROR_SYSCALL:
            freeSSL();
            closeSock(sockfd);
            return sent;
        }
      }
      else if(
        pfd[0].revents & POLLHUP ||
        pfd[0].revents & POLLERR || 
        pfd[0].revents & POLLNVAL
      ) {
        throw(meowHttp::Exception("connection closed", true));
        close();
        break;
      }

    }
    else if(ret == 0){
      break;
    }
    else if(ret == -1){
      close();
      break;
    }
  }
  return sent;
}



meow sslSocket::connect(const std::string& url, const std::string& protocol, size_t port){
  #ifdef WIN32
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(res != 0)
      return ERR_CONNECT_FAILED;
  #endif
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in meow;
  meow.sin_port = htons(port);
  meow.sin_family = AF_INET;
  meow.sin_addr.s_addr = resolveHostName(url, protocol);
  if(::connect(sockfd, (sockaddr *)&meow, sizeof(meow)) != 0){
    log(ERR, "failed to connect");
    return ERR_CONNECT_FAILED;
  }
  return OK;
}

in_addr_t sslSocket::resolveHostName(const std::string& hostname, const std::string& protocol){
  struct addrinfo hints;
  struct addrinfo *result;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  ssize_t value;
  if((value = getaddrinfo(hostname.c_str(), protocol.c_str(), &hints, &result)) != 0){
    return ERR_CONNECT_FAILED;
  }
  log(INFO, "resolved hostname");
  struct sockaddr_in woof = *(struct sockaddr_in *)result->ai_addr;
  freeaddrinfo(result);
  return woof.sin_addr.s_addr; 
}

