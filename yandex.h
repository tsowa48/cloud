#ifndef YANDEX_H
#define YANDEX_H 1
#include "disk.h"
#include "http.h"

#include <string>
#include <iostream>
#include <algorithm>

#include "json.h"

using namespace std;

class yandex : public disk {
public:
  yandex(string access_token) : disk(access_token) { }

  ~yandex() { }

  status* info() {
    status* stat = new status();
    http::response* resp = http::send(HTTP_GET, "https://cloud-api.yandex.net/v1/disk?fields=total_space,used_space",
                                      {"Accept: application/json", "Authorization: OAuth " + token}, "");
    string data = string(resp->data, resp->length);
    data = data.substr(data.find("\r\n\r\n") + 4);
    string total = data.substr(data.find("total_space") + 13);
    total = total.substr(0, json::find_end_token(total));
    string used = data.substr(data.find("used_space") + 12);
    used = used.substr(0, json::find_end_token(used));
    stat->total = strtoul(total.c_str(), NULL, 10);
    stat->used = strtoul(used.c_str(), NULL, 10);

    return stat;
  }

  node* ls(const char* path) {
    string encoded_path = http::urlencode(path);
    http::response* resp = http::send(HTTP_GET, "https://cloud-api.yandex.net/v1/disk/resources?path=" + encoded_path
                                      + "&fields=path%2C_embedded%2C%20type%2Cname%2Ccreated%2C%20_embedded.items.type%2C_embedded.items.name%2C_embedded.items.created%2C_embedded.items.path%2Csize%2C_embedded.items.size%2Cresource_id%2C_embedded.items.resource_id%2Cfile%2C_embedded.items.file&sort=name",
                                      {"Accept: application/json", "Authorization: OAuth " + token}, "");

    string data = string(resp->data, resp->length);
    data = data.substr(data.find("\r\n\r\n") + 4);
    if(!(data.find("DiskNotFoundError") == string::npos && data.find("\"error\":") == string::npos)) {
      return nullptr;
    }

    string sub_data = data;
    if(data.find("\"_embedded\":") != string::npos) {
      sub_data = sub_data.replace(data.find("\"_embedded\":") - 1, data.find("]", data.find("\"_embedded\":")) + 2, "");
    }
    node* n = parse_node(sub_data);

    if(data.find("\"items\":") != string::npos) {
      string items = data.substr(data.find("\"items\":") + 9);
      items = items.substr(0, items.find("]"));

      size_t pos = 0;
      while((pos = items.find("}")) != string::npos) {
        node* sub_node = parse_node(items);
        n->nodes.push_back(sub_node);
        items = items.substr(pos + 1);
      }
    }
    return n;
  }

  bool mkdir(const char* path) {
    string encoded_path = http::urlencode(path);
    http::response* resp = http::send(HTTP_PUT, "https://cloud-api.yandex.net/v1/disk/resources?path=" + encoded_path,
                                      {"Content-Type: application/json", "Accept: application/json", "Authorization: OAuth " + token}, "");
    string head = string(resp->data, resp->length);
    return head.find("201 CREATED") != string::npos;
  }

  bool rm(const char* path) {
    string encoded_path = http::urlencode(path);
    http::response* resp = http::send(HTTP_DELETE, "https://cloud-api.yandex.net/v1/disk/resources?path=" + encoded_path + "&force_async=true&permanently=true",
                                      {"Accept: application/json", "Authorization: OAuth " + token}, "");
    string head = string(resp->data, resp->length);
    return !(head.find("204 NO CONTENT") == string::npos || head.find("202 ACCEPTED") == string::npos);
  }

  int read(const char* uri, char* data, size_t len, off_t offset) {
    http::response* resp;
    if(read_uri.empty()) {
      resp = http::send(HTTP_GET, uri, { }, "");
      string head = string(resp->data, resp->length);
      if(head.find("302 Found") != string::npos) {
        head = head.substr(head.find("Location: ") + 10, head.find("\r\n", head.find("Location: ") + 10) - head.find("Location: ") - 10);
        read_uri = head;
      }
    }
    resp = http::send(HTTP_GET, read_uri, {"Range: bytes=" + to_string(offset) + "-" + to_string(len + offset)}, "");
    string head = string(resp->data, resp->length);
    if(head.find("200 OK") != string::npos || head.find("206 Partial Content") != string::npos) {
      char* payload = strstr(resp->data, "\r\n\r\n") + 4;
      int t_len = payload - resp->data;
//cout << "\n\nDATA_RAW='" << payload << "'\n\n";
//for(int i = 0; i < len; i++)
//cout << (int)(unsigned char)(payload[i]) << " ";
//delete[] payload;
//return 0;
//      data = new char[len];

//FIXME ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ???
      memcpy(data, resp->data + t_len, len);
      return len;
    }
    return 0;
  }

private:
  string read_uri = "";

  node* parse_node(string j) {
    node* n = new node();
    string t = j.substr(j.find("\"", j.find("\"resource_id\":") + 14) + 1);
    n->id = t.substr(0, min(t.find("\","), t.find("\"}")));
    t = j.substr(j.find("\"", j.find("\"path\":") + 7) + 1);
    t = t.substr(0, min(t.find("\","), t.find("\"}")));
    n->path = t.replace(t.find("disk:"), 5, "");
    t = j.substr(j.find("\"", j.find("\"type\":") + 7) + 1);
    n->type = t.substr(0, min(t.find("\","), t.find("\"}")));
    t = j.substr(j.find("\"", j.find("\"name\":") + 7) + 1);
    n->name = t.substr(0, min(t.find("\","), t.find("\"}")));
    t = j.substr(j.find("\"", j.find("\"created\":") + 10) + 1);
    n->created = t.substr(0, min(t.find("\","), t.find("\"}")));//TODO: "2012-04-04T20:00:00+00:00" to LONG
    unsigned long size = 4096;
    if(n->type.compare("dir") == 0) {
      size = 4096;
    } else {
      t = j.substr(j.find("\"", j.find("\"file\":") + 7) + 1);
      n->file = t.substr(0, min(t.find("\","), t.find("\"}")));
      t = j.substr(j.find("\"size\":") + 7);
      size = stoul(t.substr(0, min(t.find(","), t.find("}"))));
    }
    n->size = size;
    return n;
  }
};
#endif
