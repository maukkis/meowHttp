#include "includes/client.h"
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <iostream>
#include "includes/enum.h"
#include <unistd.h>
#include <poll.h>

meow sslSocket::initializeSsl(){
  method = TLS_client_method(); 
  ctx = SSL_CTX_new (method);
  ssl = SSL_new (ctx);
  if(!ssl){
    log(ERROR, "failed to create ssl");
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
    case ERROR:
      return "error";
    break;
    default:
      return "idk";
    break;
  }
}


void sslSocket::close(){
  if(ssl){
    SSL_shutdown(ssl);
    SSL_CTX_free(ctx);
    SSL_free(ssl);
    ssl = nullptr;
  }
  ::close(sockfd);
}

ssize_t sslSocket::read(std::string& buf){
  size_t recv;
  size_t meow = 0; 
  bool bark;
  struct pollfd pfd[2];
  while(true){
    pfd[0].fd = sockfd;
    pfd[0].events = POLLIN;
    pfd[1].events = 0;
    int ret = poll(pfd, 1, 50); //check if fd is readable this prevents receiving partial data
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
            default:
              int error = SSL_get_error(ssl,recv);
              std::cout << "error: " << error << '\n';
              close();
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
        std::cout << "got poll error owo closing :3\n";
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

ssize_t sslSocket::write(const std::string& data, ssize_t buffersize){
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
        }
      }
      else if(
        pfd[0].revents & POLLHUP ||
        pfd[0].revents & POLLERR || 
        pfd[0].revents & POLLNVAL
      ) {
        std::cout << "got poll error owo closing :3\n";
        close();
        throw(meowHttp::Exception("connection closed", true));
        break;
      }

    }
    else if(ret == 0){
      break;
    }
    else if(ret == -1){
      std::cout << "Poll err = " << strerror(errno) << std::endl;
      close();
      break;
    }
  }
  return sent;
}

ssize_t sslSocket::write(const void* data, ssize_t buffersize){
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
        }
      }
      else if(
        pfd[0].revents & POLLHUP ||
        pfd[0].revents & POLLERR || 
        pfd[0].revents & POLLNVAL
      ) {
        std::cout << "got poll error owo closing :3\n";
        throw(meowHttp::Exception("connection closed", true));
        close();
        break;
      }

    }
    else if(ret == 0){
      break;
    }
    else if(ret == -1){
      std::cout << "Poll err = " << strerror(errno) << std::endl;
      close();
      break;
    }
  }
  return sent;
}



meow sslSocket::connect(const std::string& url, const std::string& protocol, size_t port){
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in meow;
  meow.sin_port = htons(port);
  meow.sin_family = AF_INET;
  meow.sin_addr.s_addr = resolveHostName(url, protocol);
  if(::connect(sockfd, (sockaddr *)&meow, sizeof(meow)) != 0){
    log(ERROR, "failed to connect");
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
    return ERROR;
  }
  log(INFO, "resolved hostname");
  struct sockaddr_in woof = *(struct sockaddr_in *)result->ai_addr;
  freeaddrinfo(result);
  return woof.sin_addr.s_addr; 
}

