#include "includes/websocket.h"
#include <cstring>
#include <iostream>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <unistd.h>
#include <stdio.h>
namespace meowHttp {

meow websocket::perform(){
  // parse url
  std::string protocol = url.substr(0, url.find("://"));
  std::string hostname = url.substr(url.find("://") + strlen("://"));
  size_t pathPos = hostname.find("/");
  std::string path;
  if (pathPos != std::string::npos) {
    path = hostname.substr(pathPos); 
    hostname = hostname.substr(0, hostname.length() - path.length());
  } 
  else {
    path = '/';
  }
  if(connect(hostname, protocol) != OK){
    log(ERROR, "failed to connect");
    return ERR_CONNECT_FAILED;
  }
  log(INFO, "connected");
  if(initializeSsl() != OK){
    log(ERROR, "failed to initializeSsl");
    return ERR_SSL_FAILED;
  }
  SSL_set_tlsext_host_name(ssl, hostname.c_str());
  SSL_set_fd(ssl, sockfd);
  size_t nya = SSL_connect(ssl);
  if(nya != 1){
    std::cout << SSL_get_error(ssl, nya) << '\n';
    return ERR_SSL_FAILED;
  }
  if (!BIO_socket_nbio(sockfd, 1)) {
    sockfd = -1;
  }
  std::string meow = "connection using: ";
  meow.append(SSL_get_cipher(ssl));
  log(INFO, meow);
  //generate nonce and encode it in base64
  unsigned char nonce[16];
  int rc = RAND_bytes(nonce, sizeof(nonce));
  if(rc != 1){
    log(ERROR, "failed to generate nonce");
    return ERR_SSL_FAILED;
  }
  BIO *b64 = BIO_new(BIO_f_base64());
  BIO *bio = BIO_new(BIO_s_mem());
  BIO_push(b64, bio);
  BIO_write(b64, nonce, sizeof(nonce));
  BIO_set_close(b64, BIO_NOCLOSE);
  BIO_flush(b64);
  BUF_MEM *bufferPtr;
  BIO_get_mem_ptr(b64, &bufferPtr);
  char *b64Nonce = (char*)malloc(sizeof(char) * bufferPtr->length + 1);
  memcpy(b64Nonce, bufferPtr->data, bufferPtr->length);
  b64Nonce[bufferPtr->length] = '\0';
  BUF_MEM_free(bufferPtr);
  BIO_free_all(b64);
  std::string request = "GET " + path + " HTTP/1.1"
  "\r\nHost: " + hostname + 
  "\r\nUser-Agent: meow browser"
  "\r\nAccept: application/json,text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"
  "\r\nAccept-Language: en-US,en;q=0.5"
  "\r\nUpgrade: websocket"
  "\r\nConnection: Upgrade"
  "\r\nSec-WebSocket-Key: " + b64Nonce +
  "\rSec-WebSocket-Version: 13"
  "\r\nCache-Control: max-age=0\r\n\r\n";
  free(b64Nonce);
  std::cout << request << '\n';
  [[maybe_unused]]size_t timeout = 300;
  return OK;
}


}
