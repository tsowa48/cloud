#ifndef HTTP_H
#define HTTP_H 1
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <cstdlib>

#include <iostream>

enum {
  HTTP_GET,
  HTTP_POST,
  HTTP_PUT,
  HTTP_DELETE
};

class http {
public:
  class response {
  public:
    size_t length;
    //char* headers;
    char* data;

    response() {
      length = 0;
      data = new char[0];
    }
  };

  static response* send(int method, std::string url, std::vector<std::string> headers, std::string data) {
    bool is_ssl = url.substr(0, url.find("://")).compare("https") == 0;
    std::string dns = url.substr(url.find("://") + 3,
                                 url.find("/", url.find("://") + 3) - url.find("://") - 3);
    std::string host = inet_ntoa(*((struct in_addr*)gethostbyname(dns.c_str())->h_addr_list[0]));
    std::string payload;
    switch(method) {
      case HTTP_POST:
        payload = "POST";
        break;
      case HTTP_PUT:
        payload = "PUT";
        break;
      case HTTP_DELETE:
        payload = "DELETE";
        break;
      case HTTP_GET:
      default:
        payload = "GET";
    }
    payload += " " + url.substr(url.find(dns) + dns.length()) + " HTTP/1.1\r\n";
    payload += "Host: " + dns + "\r\n";
    for(std::string header : headers) {
      payload += header + "\r\n";
    }
    payload += "\r\n";
    if(!data.empty()) {
      payload += data + "\r\n";
    }

    if(is_ssl) {
      return sendHttps(host, payload);
    } else {
      return sendHttp(host, payload);
    }
  }

  static std::string urlencode(std::string s) {
    int length = s.length();
    std::string result("");
    for(int i = 0; i < length; i++) {
      if((s.at(i) > 47 && s.at(i) < 58) || (s.at(i) > 64 && s.at(i) < 91) || (s.at(i) > 96 && s.at(i) < 123)) {
        result.push_back(s.at(i));
      } else {
        char buf[2];
        sprintf(buf, "%x", (int)((unsigned char)s.at(i)));
        result.append("%");
        result.append(buf);
      }
    }
    return result;
  }

private:
  static response* sendHttp(std::string host, std::string raw) {
    struct sockaddr_in server_address;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int port = 80;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(host.c_str());//inet_pton(AF_INET, host.c_str(), &server_address.sin_addr);
    connect(sock, (struct sockaddr*)&server_address, sizeof(server_address));
    int len = write(sock, raw.c_str(), raw.length());
    int BUF_LEN = 1024;
    char* buf = new char[BUF_LEN];
    memset(buf, 0, BUF_LEN);
    response* resp = new response();
    do {
      len = read(sock, buf, BUF_LEN);
      resp->length += len;
      resp->data = (char*)realloc(resp->data, sizeof(char) * resp->length);
      memcpy(&(resp->data[resp->length - len]), buf, len);
      memset(buf, 0, BUF_LEN);
    } while(len > BUF_LEN - 1);
    close(sock);
    return resp;
  }

  static response* sendHttps(std::string host, std::string raw) {
    struct sockaddr_in server_address;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int port = 443;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(host.c_str());
    connect(sock, (struct sockaddr*)&server_address, sizeof(server_address));
    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD *meth = TLS_client_method();//TLSv1_2_client_method();
    SSL_CTX *ctx = SSL_CTX_new(meth);
    SSL* ssl = SSL_new(ctx);
    SSL_get_fd(ssl);
    SSL_set_fd(ssl, sock);
    SSL_connect(ssl);

    int len = SSL_write(ssl, raw.c_str(), strlen(raw.c_str()));//send

    int BUF_LEN = 1024;
    char* buf = new char[BUF_LEN];
    memset(buf, 0, BUF_LEN);
    response* resp = new response();
    do {
      len = SSL_read(ssl, buf, BUF_LEN);
      //resp->length += len;
      //char* tmp = new char[resp->length];
      //memcpy(tmp, resp->data, resp->length - len);
      //tmp += (resp->length - len);
      //memcpy(tmp, buf, len);
      //resp->data = new char[resp->length];
      //tmp -= (resp->length - len);
      //memcpy(resp->data, tmp, resp->length);
      //delete[] tmp;
resp->data = (char*)realloc(resp->data, sizeof(char) * (resp->length + len));
resp->data += resp->length;
memcpy(resp->data, buf, len);
resp->data -= resp->length;
resp->length +=len;
      memset(buf, 0, BUF_LEN);
    } while(len > BUF_LEN - 1);
    SSL_shutdown(ssl);
int fp = open("/work1/gcg/cloud/out.bin", O_RDWR | O_APPEND);
write(fp, resp->data, resp->length);
write(fp, "\r\n\r\n\r\n\r\n", 8);
close(fp);

    return resp;
  }
};
#endif
