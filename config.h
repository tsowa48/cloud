#ifndef CONFIG_H
#define CONFIG_H 1
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <map>

class config {
public:
  static std::map<std::string, std::string> get() {
    std::map<std::string, std::string> tokens;
    char* buf = new char[1024 * 1024];
    memset(buf, 0, 1024 * 1024);
    int fd = open("config", O_RDONLY);
    read(fd, buf, 1024 * 1024);
    close(fd);
    std::vector<std::string> lines;
    char* line = strtok(buf, "\r\n");
    while(line != NULL) {
      if(strlen(line) > 2)
        lines.push_back(line);
      line = strtok(NULL, "\r\n");
    }
    for(std::string kv : lines) {
      size_t splitter = kv.find_first_of(":");
      std::string v = kv.substr(0, splitter);
      std::string k = kv.substr(splitter + 1);
      tokens.insert({k, v});
    }
    return tokens;
  }
};
#endif
