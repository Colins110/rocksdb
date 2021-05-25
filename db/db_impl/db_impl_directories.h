#pragma once
// #include "db/column_family.h"
#include "rocksdb/file_system.h"
#include "rocksdb/io_status.h"

namespace ROCKSDB_NAMESPACE {
IOStatus DirCreateAndNewDirectory(FileSystem* fs, const std::string& dirname,
                                  std::unique_ptr<FSDirectory>* directory);
// Class to maintain directories for all database paths other than main one.
class Directories {
 public:
  IOStatus SetDirectories(FileSystem* fs, const std::string& dbname,
                          const std::string& wal_dir,
                          const std::vector<DbPath>& data_paths);

  FSDirectory* GetDataDir(size_t path_id) const {
    assert(path_id < data_dirs_.size());
    FSDirectory* ret_dir = data_dirs_[path_id].get();
    if (ret_dir == nullptr) {
      // Should use db_dir_
      return db_dir_.get();
    }
    return ret_dir;
  }

  FSDirectory* GetWalDir() {
    if (wal_dir_) {
      return wal_dir_.get();
    }
    return db_dir_.get();
  }

  FSDirectory* GetDbDir() { return db_dir_.get(); }

 private:
  std::unique_ptr<FSDirectory> db_dir_;
  std::vector<std::unique_ptr<FSDirectory>> data_dirs_;
  std::unique_ptr<FSDirectory> wal_dir_;
};

inline IOStatus Directories::SetDirectories(
    FileSystem* fs, const std::string& dbname, const std::string& wal_dir,
    const std::vector<DbPath>& data_paths) {
  IOStatus io_s = DirCreateAndNewDirectory(fs, dbname, &db_dir_);
  if (!io_s.ok()) {
    return io_s;
  }
  if (!wal_dir.empty() && dbname != wal_dir) {
    io_s = DirCreateAndNewDirectory(fs, wal_dir, &wal_dir_);
    if (!io_s.ok()) {
      return io_s;
    }
  }

  data_dirs_.clear();
  for (auto& p : data_paths) {
    const std::string db_path = p.path;
    if (db_path == dbname) {
      data_dirs_.emplace_back(nullptr);
    } else {
      std::unique_ptr<FSDirectory> path_directory;
      io_s = DirCreateAndNewDirectory(fs, db_path, &path_directory);
      if (!io_s.ok()) {
        return io_s;
      }
      data_dirs_.emplace_back(path_directory.release());
    }
  }
  assert(data_dirs_.size() == data_paths.size());
  return IOStatus::OK();
}

inline IOStatus DirCreateAndNewDirectory(
    FileSystem* fs, const std::string& dirname,
    std::unique_ptr<FSDirectory>* directory) {
  // We call CreateDirIfMissing() as the directory may already exist (if we
  // are reopening a DB), when this happens we don't want creating the
  // directory to cause an error. However, we need to check if creating the
  // directory fails or else we may get an obscure message about the lock
  // file not existing. One real-world example of this occurring is if
  // env->CreateDirIfMissing() doesn't create intermediate directories, e.g.
  // when dbname_ is "dir/db" but when "dir" doesn't exist.
  IOStatus io_s = fs->CreateDirIfMissing(dirname, IOOptions(), nullptr);
  if (!io_s.ok()) {
    return io_s;
  }
  return fs->NewDirectory(dirname, IOOptions(), directory, nullptr);
}
}  // namespace ROCKSDB_NAMESPACE
