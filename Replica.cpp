
#include "Replica.h"
#include <string>
#include <mutex>

std::mutex commmit_abort_mtx {};
std::mutex push_chunk_mtx {};

#define SERIALIZE_WRITE

int commit_count = 0;

Chunk::Chunk()
{
  this->data = "";
}

Chunk::Chunk(std::string arg_data)
{
  this->data = arg_data;
}

Json::Value *
FileChunk::dumpJ()
{
  Json::Value * result_ptr = new Json::Value();

  if (this->data != "")
    {
      (*result_ptr)["data"] = this->data;
    }

  return result_ptr;
}

bool
FileChunk::Jdump(Json::Value *input_json_ptr)
{
  if ((input_json_ptr == NULL) ||
      ((*input_json_ptr).isNull() == true) ||
      ((*input_json_ptr).isObject() != true))
    {
      return false;
    }

  if ((((*input_json_ptr)["data"]).isNull() == true) ||
      (((*input_json_ptr)["data"]).isString() != true))
    {
      return false;
    }

  this->data = ((*input_json_ptr)["data"]).asString();

  return true;
}

Replica::Replica
(std::string core_arg_host_url, std::string core_arg_owner_vsID,
 std::string core_arg_class_id, std::string core_arg_object_id)
  : Core { core_arg_host_url, core_arg_owner_vsID,
    core_arg_class_id, core_arg_object_id }
{
  std::cout << "a shadow has been created" << std::endl;
}

Replica::Replica
(std::string core_arg_host_url, std::string core_arg_owner_vsID,
 std::string core_arg_class_id, std::string core_arg_object_id,
 std::string arg_data)
  : Core { core_arg_host_url, core_arg_owner_vsID,
    core_arg_class_id, core_arg_object_id }
{
  (this->committed_data[0]).data = arg_data;
  (this->committed_data[1]).data = arg_data;
}

Json::Value
Replica::CommitAbort
(std::string arg_name, std::string arg_fhandle, std::string arg_chunk_index,
 std::string arg_commitorabort)
{
  Json::Value result;

  // this lock is for muti-thread sync, NOT write ordering serialization
  std::unique_lock<std::mutex> commmit_abort_lock_section { commmit_abort_mtx };
  // std::cout << arg_chunk_index.substr(1, 1).data() << '\n'
  int chunk_num = std::atoi(arg_chunk_index.substr(0, 1).data());

  if (arg_commitorabort != "commit") {
    result["status"] = "abort";
    return result;
  }

  commit_count += 1;
  std::cout << std::boolalpha << uncommitted_data[0].valid << " " << uncommitted_data[1].valid << '\n';
  std::cout << "Committ to chunk " << chunk_num << '\n';

  #ifdef SERIALIZE_WRITE

  if (commit_count == 2) {
    // both clients have tried to commit, sequence them now
    // in general, in the GFS paper, this is handled through locking "lease"
    // from master. Active and valid changes to leased files are in an LRU cache
    if (uncommitted_data[0].timestamp < uncommitted_data[1].timestamp) {
      // commit 0 and then 1
      if (uncommitted_data[0].valid)
        committed_data[uncommitted_data[0].chunk_index].data = uncommitted_data[0].data;
      if (uncommitted_data[1].valid)
        committed_data[uncommitted_data[1].chunk_index].data = uncommitted_data[1].data;
    }
    else {
      // commit 1 and then 0
      if (uncommitted_data[1].valid)
        committed_data[uncommitted_data[1].chunk_index].data = uncommitted_data[1].data;
      if (uncommitted_data[0].valid)
        committed_data[uncommitted_data[0].chunk_index].data = uncommitted_data[0].data;
    }

    // invalidate both uncommitted data
    uncommitted_data[0].valid = false;
    uncommitted_data[1].valid = false;
    std::cout << "Chunk 1: " << committed_data[0].data << '\n';
    std::cout << "Chunk 2: " << committed_data[1].data << '\n';
  }


  #else // we just flush the chunk of that client/request
  for (auto& data : uncommitted_data) {
    if (data.valid && data.chunk_index == chunk_num) {
      committed_data[chunk_num].data = data.data;
      data.valid = false;
    }
  }

  std::cout << "Chunk " << chunk_num << ": " << committed_data[chunk_num].data << '\n';
  #endif

  result["vote"] = "commit";
  return result;
}

Json::Value
Replica::PushChunk2Replica
(std::string arg_name, std::string arg_fhandle, std::string arg_chunk_index, std::string arg_chunk)
{
  Json::Value result;

  // this lock is for muti-thread sync, NOT write ordering serialization
  std::unique_lock<std::mutex> push_chunk_lock { push_chunk_mtx };

  std::cout <<" Line "<< __LINE__ << " Inside the Replica.PushChunk2Replica" << std::endl;
  std::cout << arg_chunk << " " << arg_chunk_index << '\n';

  int client_num = std::atoi(arg_chunk.substr(1, 1).data()); // chuck is of form "cx-data" where x is client number
  int chunk_num = std::atoi(arg_chunk_index.substr(0, 1).data());

  std::cout << "Client " << client_num << " pushed to chunk " << chunk_num << " " << arg_chunk << '\n';

  #ifdef SERIALIZE_WRITE

  int64_t timestamp = std::stoll(arg_chunk_index.substr(1, arg_chunk_index.size() - 1).data());

  if (uncommitted_data[client_num].valid) {
    if (timestamp > uncommitted_data[client_num].timestamp) {
      // the chunk just pushed has a newer timestamp, so replace the uncomitted with this
      uncommitted_data[client_num].data = arg_chunk;
      uncommitted_data[client_num].timestamp = timestamp;
      uncommitted_data[client_num].chunk_index = chunk_num;
    }
    else {
      // this means that the chunk we just received has an older timestamp, arrived late
      // so just ignore it
    }
  }
  else {
    // new chunk received, add to "queue" (LRU buffer in paper)
    uncommitted_data[client_num].valid = true;
    uncommitted_data[client_num].data = arg_chunk;
    uncommitted_data[client_num].timestamp = timestamp;
    uncommitted_data[client_num].chunk_index = chunk_num;
  }

  #else
  // concurrent, just replace the chunk
  uncommitted_data[client_num].data = arg_chunk;
  uncommitted_data[client_num].chunk_index = chunk_num;
  uncommitted_data[client_num].valid = true;
  #endif


  result["status"] = "success";
  return result;
}

Json::Value *
Replica::dumpJ()
{
  Json::Value * result_ptr = new Json::Value();

  if (this->name != "")
    {
      (*result_ptr)["name"] = this->name;
    }

  if (this->fhandle != "")
    {
      (*result_ptr)["fhandle"] = this->fhandle;
    }

  // if (this->chunk_index != "")
    // {
      (*result_ptr)["name"] = this->committed_data[0].data + ", " + this->committed_data[1].data;
    // }

  return result_ptr;
}

bool
Replica::Jdump(Json::Value *input_json_ptr)
{
  if ((input_json_ptr == NULL) ||
      ((*input_json_ptr).isNull() == true) ||
      ((*input_json_ptr).isObject() != true))
    {
      return false;
    }

  if ((((*input_json_ptr)["name"]).isNull() == true) ||
      (((*input_json_ptr)["fhandle"]).isNull() == true) ||
      (((*input_json_ptr)["name"]).isString() != true) ||
      (((*input_json_ptr)["fhandle"]).isString() != true))
    {
      return false;
    }

  this->name    = ((*input_json_ptr)["name"]).asString();
  this->fhandle = ((*input_json_ptr)["fhandle"]).asString();
  return true;
}
