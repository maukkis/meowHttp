#include "includes/websocket.h"
#include "includes/client.h"
#include "includes/enum.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <optional>
#ifdef WIN32
#define htole64(x) _byteswap_uint64(x)
#define htobe64(x) _byteswap_uint64(x)
#endif
#if defined(__linux__) || defined(__OpenBSD__)
#include <endian.h>
#elif defined(_AIX) || defined(__sun)
#include <sys/types.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#include <sys/endian.h>
#endif
#include <iostream>
#include <memory>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <string>
#include <type_traits>
#include <stdio.h>
#include <sstream>


namespace meowWs {


Websocket::~Websocket(){
  wsClose();
}

Websocket &Websocket::setUrl(const std::string& url){
  this->url = url;
  return *this;
}

struct Frame{
  std::unique_ptr<uint8_t[]> buffer;
  size_t frameLen;
  ssize_t totalLen;
};



void Websocket::parseWs(std::string& buf, meowWsFrame* frame, size_t rlen){
  size_t startPos;
  const char *bufC = buf.data();
  switch(static_cast<uint8_t>(buf[0])){
    case 0x81:
      frame->opcode = meowWS_TEXT;
    break;
    case 0x88:
      frame->opcode = meowWS_CLOSE;
    break;
    case 0x89:
      frame->opcode = meowWS_PING;
    break;
    case 0x8A:
      frame->opcode = meowWS_PONG;
    break;
  }
  if(bufC[1] < 126){
    startPos = 2;
    frame->frameLen = 2;
    frame->payloadLen = bufC[1];
  }
  else if(bufC[1] == 126){
    startPos = 4;
    frame->frameLen = 4;
    uint16_t pLen;
    std::memcpy(&pLen, &bufC[2], 2);
    frame->payloadLen = ntohs(pLen);
  }
  else{
    startPos = 10;
    frame->frameLen = 10;
    uint64_t pLen;
    std::memcpy(&pLen, &bufC[2], 8);
    #if defined(_AIX) || defined(__sun)
    frame->payloadLen = ntohll(pLen);
    #else
    frame->payloadLen = htole64(pLen);
    #endif
  }
  if(rlen > (frame->frameLen + frame->payloadLen)){
    moreData = std::make_optional(buf.substr(frame->frameLen + frame->payloadLen));
  }
  buf = buf.substr(startPos, frame->payloadLen);

}

template<typename T>
std::unique_ptr<Frame> constructFrame(const T* payload, opcodes opCode, size_t payloadLen){
  uint8_t frame[14];
  auto frameStruct = std::make_unique<Frame>();
  switch(opCode){
    case meowWS_TEXT:
      frame[0] = 0x81; // fuck you we arent only sending text 
    break;
    case meowWS_PING:
      frame[0] = 0x89; //set the payload to a ping
    break;
    case meowWS_PONG:
      frame[0] = 0x8A;
    break;
    case meowWS_CLOSE:
      frame[0] = 0x88;
    break;
  }
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
    #if defined(_AIX) || defined(__sun)
    uint64_t len = htonll(payloadLen);
    #else
    uint64_t len = htobe64(payloadLen); // make a 64 bit value that is 64 bits wow shocking
    #endif    
    memcpy(&frame[2], &len, 8);
    frameStruct->frameLen = 10;
  }
  constexpr uint8_t maskingKey[4] = {0x00, 0x00, 0x00, 0x00}; // fuck you again we arent masking anything
  memcpy(&frame[frameStruct->frameLen], maskingKey, sizeof(maskingKey)); // copy the masking key to the frame
  frameStruct->frameLen += sizeof(maskingKey);
  frameStruct->buffer = std::make_unique<uint8_t[]>(payloadLen + frameStruct->frameLen); // allocate enough memory to fit everything
  memcpy(frameStruct->buffer.get(), frame, frameStruct->frameLen); // copy frame to the buffer
  if constexpr (std::is_same_v<T, std::string>)
    memcpy(&frameStruct->buffer[frameStruct->frameLen], payload->data(), payloadLen);
  else
    memcpy(&frameStruct->buffer[frameStruct->frameLen], payload, payloadLen);
  frameStruct->totalLen = payloadLen + frameStruct->frameLen;
  return frameStruct;
}


meow Websocket::wsClose(const uint16_t closeCode, const std::string& aa){
  if(!ssl) return ERR_ALREADY_CLOSED;
  uint16_t beClose = htons(closeCode);
  if(aa.length() > 127) return ERR_TOO_LARGE_CLOSE; 
  auto payload = std::make_unique<uint8_t[]>(2 + aa.length());
  memcpy(payload.get(), &beClose, 2);
  memcpy(&payload[2], aa.data(), aa.length());
  auto frame = constructFrame(payload.get(), meowWS_CLOSE, 2 + aa.length());
  ssize_t sLen = write(frame->buffer.get(), frame->totalLen);
  if(sLen == frame->totalLen) {
    close();
    return OK;
  }
  close();
  return ERR_SEND_FAILED;
}

size_t Websocket::wsSend(const std::string& payload, opcodes opCode){
  if(!ssl) throw(meowHttp::Exception("already closed :3", true));
  auto constructedFrame = constructFrame(&payload, opCode, payload.length());
  ssize_t sLen = write(constructedFrame->buffer.get(), constructedFrame->totalLen);
  return sLen;
}

size_t Websocket::wsRecv(std::string& buf, struct meowWsFrame *frame){
  if(!ssl) throw(meowHttp::Exception("already closed :3", true));
  size_t rlen;
  if(moreData){
    buf = *moreData;
    rlen = moreData->length();
    moreData->resize(0);
    moreData->shrink_to_fit();
    moreData.reset();
  }
  else{
    rlen = read(buf);
  }
  if(rlen < 1){
    return rlen;
  }
  parseWs(buf, frame, rlen);
  return frame->payloadLen;
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
meow Websocket::perform(){
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
    log(ERR, "failed to connect");
    return ERR_CONNECT_FAILED;
  }
  log(INFO, "connected");
  if(initializeSsl() != OK){
    log(ERR, "failed to initializeSsl");
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
    log(ERR, "failed to generate nonce");
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
  char *b64Nonce = (char *)malloc(sizeof(char) * bufferPtr->length + 1);
  memcpy(b64Nonce, bufferPtr->data, bufferPtr->length);
  b64Nonce[bufferPtr->length - 1] = '\0';
  BUF_MEM_free(bufferPtr);
  BIO_free_all(b64);
  std::string request { 
    "GET " + path + " HTTP/1.1"
    "\r\nHost: " + hostname + 
    "\r\nUser-Agent: meow browser"
    "\r\nAccept: */*"
    "\r\nUpgrade: websocket"
    "\r\nConnection: Upgrade"
    "\r\nSec-WebSocket-Key: " + b64Nonce +
    "\r\nSec-WebSocket-Version: 13\r\n\r\n"
  };
  free(b64Nonce);
  // send the request
  if(write(request, request.length()) < 1){
    log(ERR, "failed to send a request");
    return ERR_SEND_FAILED;
  }
  std::string buf;
  size_t rlen = read(buf);
  while(rlen < 1){
    rlen = read(buf);
  }
  if(parseStatusCode(buf) == SWITCHING_PROTOCOLS){
    if(buf.find("\r\n\r\n") + strlen("\r\n\r\n") >= buf.length()){
      return OK;
    }
    moreData = std::make_optional(buf.substr(buf.find("\r\n\r\n") + 4));
    return OK;
  }
  return ERR_CONNECT_FAILED;
}


}
