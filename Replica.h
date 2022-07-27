
#ifndef _REPLICA_H_
#define _REPLICA_H_

#include "Core.h"

// Replica.h
#include <iostream>
#include <array>

class FileChunk
{
 private:
 public:
  std::string data {"A"}; // base64 encoding
  FileChunk() = default;
  virtual Json::Value * dumpJ();
  virtual bool Jdump(Json::Value *);
};

class Chunk : public FileChunk
{
 private:
 public:
  int64_t timestamp;
  bool valid { false };
  int chunk_index;
  Chunk();
  Chunk(std::string);
};

class Replica : public Core
{
 private:
 public:
  std::string name;
  std::string fhandle;
  // std::array<Chunk, 2> client_chunks;
  std::array<Chunk, 2> uncommitted_data; // per client
  std::array<FileChunk, 2> committed_data; // actual chunks of the file
  Replica() = default;
  Replica(std::string, std::string, std::string,
	  std::string);
  Replica(std::string, std::string, std::string,
	  std::string, std::string);
  virtual Json::Value PushChunk2Replica(std::string, std::string, std::string, std::string);
  virtual Json::Value CommitAbort(std::string, std::string, std::string, std::string);
  virtual Json::Value * dumpJ();
  virtual bool Jdump(Json::Value *);
};

#endif  /* _REPLICA_H_ */
