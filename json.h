#ifndef JSON_H
#define JSON_H 1
#include <string>
#include <cstdint>
#include <algorithm>
#include <any>
#include <map>

#include <iostream>

using namespace std;

class json {
public:
  json(string s) {
    source = s;
  }

  map<string, any> parse(string j) {
    map<string, any> data;
    string jj = j.substr(j.find("{") + 1);
//ПЕРЕПИСАТЬ БЛЯТЬ!
    while(jj.length() > 1) {
      size_t s_key = jj.find("\"") + 1;
      size_t e_key = jj.find("\":", s_key);
      string key = jj.substr(s_key, e_key - s_key);
      any value;
      jj = jj.substr(e_key + 2);
      size_t s_value = find_s_value(jj);
      jj = jj.substr(s_value);
      if(jj.at(0) == '{') {
        value = parse(jj);
      } else if(jj.at(0) == '[') {
        //todo: parse array
      } else if(jj.at(0) == '\"') {
        size_t e_value = jj.find("\"", s_value);
        value = make_any<string>(jj.substr(s_value, e_value - s_value));
        jj = jj.substr(e_value + 1);
cout << "key=" << key << "\tv=" << any_cast<string>(value) << endl;
      } else {
cout << "OTHER=" << jj << endl;
      }
      data.insert({key, value});
      // = make_any<string>("Hello");
    }
cout << "JJ" << jj << endl;
    return data;
  }

  static size_t find_end_token(string v) {
    size_t pos = SIZE_MAX;
    pos = min(pos, v.find("}"));
    pos = min(pos, v.find(","));
    return pos;
  }
private:
  string source;

  bool is_num(char c) {
    return c > 47 && c < 58;
  }

  size_t find_s_value(string v) {
    size_t pos = SIZE_MAX;
    pos = min(pos, v.find("\""));
    pos = min(pos, v.find("{"));
    pos = min(pos, v.find("["));
    pos = min(pos, v.find("true"));
    pos = min(pos, v.find("false"));
    pos = min(pos, v.find("null"));
    pos = min(pos, v.find("-"));
    pos = min(pos, v.find("0"));
    pos = min(pos, v.find("1"));
    pos = min(pos, v.find("2"));
    pos = min(pos, v.find("3"));
    pos = min(pos, v.find("4"));
    pos = min(pos, v.find("5"));
    pos = min(pos, v.find("6"));
    pos = min(pos, v.find("7"));
    pos = min(pos, v.find("8"));
    pos = min(pos, v.find("9"));
    return pos;
  }
};
#endif
