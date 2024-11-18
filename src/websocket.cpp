#include "includes/websocket.h"
#include "includes/enum.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <endian.h>
#include <iostream>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <unistd.h>
#include <stdio.h>
#include <sstream>
namespace meowWs {

struct Frame{
  uint8_t *buffer;
  size_t frameLen;
  size_t totalLen;
};

Frame *constructFrame(const std::string& payload, opCodes opCode){
  size_t payloadLen = payload.length();
  uint8_t frame[10];
  struct Frame *frameStruct = new Frame;
  switch(opCode){
    case meowWS_TEXT:
      frame[0] = 0x81; // fuck you we are only sending text not anymore!
    break;
    case meowWS_PING:
      frame[0] = 0x89; //ping!
    break;
  }
  frame[0] = 0x81; // fuck you we are only sending text
  if (payloadLen <= 125){ // small payload owo
    frame[1] = 0x80 | payloadLen; // we set the masking bit and the payloadLen
    frameStruct->frameLen = 2;
  }
  else if(payloadLen <= 65535){ // maximum decimal number a 16 bit binary value can hold
    frame[1] = 0x80 | 126; // set mask bit and set extended payload to a 16 bit payload
    uint16_t len = htons(payloadLen); // make a 16 bit value thats actually 16 bits
    memcpy(&frame[2], &len, 2);
    frameStruct->frameLen = 4;
  }
  else { // biggest payload owo so big
    frame[1] = 0x80 | 127; // set masking bit and set payload length to a 64 bit payload
    uint64_t len = htobe64(payloadLen); // make a 64 bit value that is 64 bits wow shocking
    memcpy(&frame[2], &len, 8);
    frameStruct->frameLen = 10;
  }
  uint8_t maskingKey[4] = {0x00, 0x00, 0x00, 0x00}; // fuck you again we arent masking anything
  memcpy(&frame[frameStruct->frameLen], maskingKey, sizeof(maskingKey)); // copy the masking key to the frame
  frameStruct->frameLen += sizeof(maskingKey);
  frameStruct->buffer = (uint8_t *)malloc(payloadLen + frameStruct->frameLen); // allocate enough memory to fit everything
  memcpy(frameStruct->buffer, frame, frameStruct->frameLen); // copy frame to the buffer
  for(size_t i = 0; i < payloadLen; ++i){ // iterate over the string and add it to the buffer
    frameStruct->buffer[frameStruct->frameLen + i] = payload[i];
  }
  frameStruct->totalLen = payloadLen + frameStruct->frameLen;
  return frameStruct;
}

size_t websocket::wsSend(const std::string& payload, opCodes opCode){
  struct Frame *constructedFrame = constructFrame(payload, opCode);
  size_t sLen = SSL_write(ssl, constructedFrame->buffer, constructedFrame->totalLen);
  free(constructedFrame->buffer);
  delete constructedFrame;
  return sLen;
}

size_t websocket::wsRecv(std::string& buf, size_t bufSize){
  size_t rlen = read(buf, bufSize, 50);
  if(rlen < 1){
    return rlen;
  }
  if(buf[1] < 126){
    rlen = buf[1];
    buf = buf.substr(2);
  }
  else if(buf[1] == 126){
    rlen = buf[2] + buf[3];
    buf = buf.substr(4);
  }
  return rlen;
}

size_t parseStatusCode(std::string_view meow){
  size_t startPos{0};
  size_t endPos{meow.find("\r\n")};
  std::string status{meow.substr(startPos, endPos)};
  std::istringstream bark{status};
  bark.ignore(256, ' ');
  size_t httpStatusCode;
  bark >> httpStatusCode;
  return httpStatusCode; 
}
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
  b64Nonce[bufferPtr->length - 1] = '\0';
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
  "\r\nSec-WebSocket-Version: 13\r\n\r\n";
  free(b64Nonce);
  // send the request
  if(write(request, request.length()) < 1){
    log(ERROR, "failed to send a request");
    return ERR_SEND_FAILED;
  }
  std::string buf;
  size_t rlen = read(buf, 8192, 50);
  while(rlen < 1){
    rlen = read(buf, 8192, 50);
  }
  if(parseStatusCode(buf) == 101){
    return OK;
  }
  return ERR_CONNECT_FAILED;
}


}
