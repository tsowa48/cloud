#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

#include <ctime>
#include <sys/statvfs.h>
#include <vector>
#include "config.h"
#include "disk.h"
#include "yandex.h"

static std::vector<disk*> disks;

static unsigned long used_size;

static void* cloud_init(struct fuse_conn_info* conn, struct fuse_config* cfg) {
  (void) conn;
  cfg->kernel_cache = 1;
  return NULL;
};

static void cloud_destroy(void* private_data) {
  //todo?
}

static int cloud_open(const char *path, struct fuse_file_info *fi) {
	//if (strcmp(path+1, options.filename) != 0)
		//return -ENOENT;
	//if ((fi->flags & O_ACCMODE) != O_RDONLY)
		//return -EACCES;
	return 0;
}

static int cloud_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
//INFO: http://ccfit.nsu.ru/~deviv/courses/unix/unix/ng7c229.html
  (void) fi;
  int res = 0;
  memset(stbuf, 0, sizeof(struct stat));
  time_t tm_time_now;
  time(&tm_time_now);
  mode_t type = S_IFDIR;
  unsigned long size = 4096;
  unsigned long sub_nodes = 1;
  for(disk* d : disks) {
    disk::node* n = d->ls(path);
    if(n == nullptr) {
      continue;
    }
    type = n->type.compare("dir") == 0 ? S_IFDIR : S_IFREG;
    size = n->size;
    if(type == S_IFDIR) {
      sub_nodes += n->nodes.size();
    }
  }
  stbuf->st_mode = type | (type == S_IFDIR ? 0755 : 0644);
  stbuf->st_uid = getuid();
  stbuf->st_gid = getgid();
  stbuf->st_nlink = sub_nodes;//info: number of dirs & files inside?

  stbuf->st_atime = tm_time_now;//read
  stbuf->st_mtime = tm_time_now;//write
  stbuf->st_ctime = tm_time_now;//change state
  stbuf->st_size = strcmp(path, "/") == 0 ? used_size : size;

  //res = -ENOENT;
  return res;
}

static int cloud_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
  std::cout << "PATH[readdir]=" << path << std::endl;
  (void) offset;
  (void) fi;
  (void) flags;
  filler(buf, ".", NULL, 0, static_cast<fuse_fill_dir_flags>(0));
  filler(buf, "..", NULL, 0, static_cast<fuse_fill_dir_flags>(0));
  for(disk* d : disks) {
    disk::node* n = d->ls(path);
    if(n == nullptr)
      continue;
    for(disk::node* sn : n->nodes) {
      filler(buf, sn->name.c_str(), NULL, 0, static_cast<fuse_fill_dir_flags>(0));
    }
  }
  return 0;
}

static int cloud_statfs(const char* path, struct statvfs* st)
{
  unsigned long used = 0;
  for(disk* d : disks) {
    used += d->info()->used;
  }
  st->f_bsize = 1;
  st->f_blocks = total_size;
  st->f_bavail = total_size - used;//st.f_bavail;
  //st->files = -1;//st.f_files;
  //st->files_free = -1;//st.f_ffree;
  //st->namelen = 64;//st.f_namelen;
  return 0;
}

//static int cloud_getdir(const char* path, fuse_dirh_t dirh, fuse_dirfil_t dirfil) {
  ////DIR *dp;
  ////struct dirent *de;
  //int res = 0;
  ////dp = opendir(path);
  ////if(dp == NULL)
    ////return -errno;
  ////while((de = readdir(dp)) != NULL) {
    ////res = filler(h, de->d_name, de->d_type);
    ////if(res != 0)
      ////break;
  ////}
  ////closedir(dp);
  //return res;
//};

static int cloud_opendir(const char* path, struct fuse_file_info* fi) {
  cout << "PATH[opendir]=" << path << endl;
  //todo
  return 0;
}

static int cloud_mkdir(const char* path, mode_t m) {
  //todo
  return 0;
}

static int cloud_rmdir(const char* path) {
  bool ret = false;
  for(disk* d : disks) {
    ret &= d->rm(path);
  }
  return ret - 1;
}

static int cloud_releasedir(const char* path, struct fuse_file_info* fi) {
  //todo
  return 0;
}

static int cloud_release(const char* path, struct fuse_file_info* fi) {
  //todo
  return 0;
}

//static int cloud_getxattr(const char* path, const char* name, char* value, size_t size) {
  ////cout << "xattr=" << name << " value=" << value << endl;
  ////todo
  //return 0;
//}

static int cloud_access(const char* path, int mode) {
  //cout << "ACCESS x1=" << path << " mode=" << mode << endl;
  //todo
  return 0;
}

static int cloud_read(const char* path, char* data, size_t len, off_t offset, struct fuse_file_info* fi) {
  disk::node* file;
  for(disk* d : disks) {
    file = d->ls(path);
    if(file != nullptr) {
      return d->read(file->file.c_str(), data, len, offset);
    }
  }
  return 0;
}

static int cloud_write(const char* path, const char* data, size_t len, off_t offset, struct fuse_file_info* fi) {
  cout <<endl<< "WRITE:" << path << " DATA=" << data << " SIZE=" << len << " OFFSET=" << offset << endl<<endl;
  //todo: write file
  return 0;
}

//static int cloud_flush(const char* path, struct fuse_file_info* fi) {
  //todo: stub ???
  //return 0;
//}


static const struct fuse_operations cloud_operations = {
  //.getdir = cloud_getdir
  .getattr = cloud_getattr,
  .mkdir = cloud_mkdir,
  .rmdir = cloud_rmdir,
  .open = cloud_open,
  .read = cloud_read,
  .write = cloud_write,
  .statfs = cloud_statfs,
  //.flush = cloud_flush,
  .release = cloud_release,
  //.getxattr = cloud_getxattr,
  .opendir = cloud_opendir,
  .readdir = cloud_readdir,
  .releasedir = cloud_releasedir,
  .init = cloud_init,

//  //int (*getattr) (const char *, struct stat *);
//  //int (*readlink) (const char *, char *, size_t);
//  //int (*mknod) (const char *, mode_t, dev_t);
//  //int (*unlink) (const char *);
//  //int (*symlink) (const char *, const char *);
//  //int (*rename) (const char *, const char *);
//  //int (*link) (const char *, const char *);
//  //int (*chmod) (const char *, mode_t);
//  //int (*chown) (const char *, uid_t, gid_t);
//  //int (*truncate) (const char *, off_t);
//  //int (*utime) (const char *, struct utimbuf *);
//  //int (*open) (const char *, struct fuse_file_info *);
//  //int (*read) (const char *, char *, size_t, off_t, struct fuse_file_info *);
//  //int (*write) (const char *, const char *, size_t, off_t,struct fuse_file_info *);
//  //int (*flush) (const char *, struct fuse_file_info *);
//  //int (*fsync) (const char *, int, struct fuse_file_info *);
//  //int (*setxattr) (const char *, const char *, const char *, size_t, int);
//  //int (*getxattr) (const char *, const char *, char *, size_t);
//  //int (*listxattr) (const char *, char *, size_t);
//  //int (*removexattr) (const char *, const char *);
//ACCESS
//LOOKUP
  .destroy = cloud_destroy,
  .access = cloud_access
};

int main(int argc, char* argv[]) {
  auto tokens = config::get();
  used_size = 0UL;
  for(auto item : tokens) {
    disk* d;
    if(item.second.compare("yandex") == 0) {
      d = new yandex(item.first);
    } else {
      std::cout << "Unknown disk" << std::endl;
      continue;
    }
    used_size += d->info()->total;
    disks.push_back(d);
  }

  ////struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  return fuse_main(argc, argv, &cloud_operations, NULL);
  ////fuse_opt_free_args(&args);
}
