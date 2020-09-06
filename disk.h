#ifndef DISK_H
#define DISK_H 1
#include <string>
#include <vector>

class disk {
public:
  class status {
  public:
    unsigned long total;
    unsigned long used;
  };

  class node {
  public:
    std::string id;
    std::string path;
    std::string type;
    std::string name;
    std::string created;
    unsigned long size;
    std::string file;

    std::vector<node*> nodes;
  };

  virtual status* info() = 0;
  virtual node* ls(const char* path) = 0;
  virtual bool mkdir(const char* path) = 0;
  virtual bool rm(const char* path) = 0;

  virtual int read(const char* file, char* data, size_t len, off_t offset) = 0;
  //virtual int write(const char* file, const char* data, size_t len) = 0;

  //virtual bool cp(const char* from, const char* to) = 0;
  //virtual bool mv(const char* from, const char* to) = 0;

  //virtual node* trash() = 0;//get trash elements
  //virtual bool empty() = 0;//empty trash
  //virtual bool restore(const char* path) = 0;//restore element from trash

  disk(std::string access_token) {
    token = access_token;
  }

 virtual ~disk() { }
protected:
  std::string token;
};
#endif
