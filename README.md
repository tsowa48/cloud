## CloudFS

###### FUSE module for mount many cloud drives (*google*, *dropbox*, *onedrive*, *yandex*, etc) as one disk space.
###### It can be used with same cloud drives.

### Dependencies:
- fuse3
- libfuse3
- libfuse3-dev
- libssl-dev

### Installation:
```sh
$ make
$ make install
```

### Config example:
```
dropbox:ACCESS_TOKEN_1
onedrive:ACCESS_TOKEN_2
onedrive:ACCESS_TOKEN_3
google:ACCESS_TOKEN_4
yandex:ACCESS_TOKEN_5
google:ACCESS_TOKEN_6
```

### Usage:
```sh
cloudfs <MOUNT_POINT>
```

### Supported cloud drives:
- yandex

### TODO:
- json parser
- install as a service
