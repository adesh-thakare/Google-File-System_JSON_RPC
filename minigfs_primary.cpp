
// miniNFS

// for Json::value
#include <json/json.h>
#include <json/reader.h>
#include <json/writer.h>
#include <json/value.h>
#include <string>

// for JsonRPCCPP
#include <iostream>
#include "minigfs_server.h"
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <stdio.h>
#include "Replica.h"

// ecs251
#include "Core.h"
#include "Directory.h"
#include "Shadow_Directory.h"
#include "Shadow_Replica.h"
#include <chrono>

using namespace jsonrpc;
using namespace std;

// globals
Shadow_Replica* gfs_secondary_A;
Shadow_Replica* gfs_secondary_B;

class Myminigfs_Server : public minigfs_Server, public Replica
{
public:
  Myminigfs_Server(AbstractServerConnector &connector, serverVersion_t type);
  virtual Json::Value ObtainChunkURL(const std::string& action, const std::string& arguments, const std::string& chunkindex, const std::string& class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID);
  virtual Json::Value PushChunk2Replica(const std::string& action, const std::string& arguments, const std::string& chunk, const std::string& chunkindex, const std::string& class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID);
  virtual Json::Value CommitAbort(const std::string& action, const std::string& arguments, const std::string& chunkindex, const std::string& class_id, const std::string& commitorabort, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID);
  virtual Json::Value LookUp(const std::string& action, const std::string& arguments, const std::string& class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID);
  virtual Json::Value Create(const std::string& action, const std::string& arguments, const std::string& class_id, const std::string& created_class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID, const std::string& sattr);
  virtual Json::Value dumpJ(const std::string& action, const std::string& arguments, const std::string& class_id, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID);
};

Myminigfs_Server::Myminigfs_Server(AbstractServerConnector &connector, serverVersion_t type)
  : minigfs_Server(connector, type)
{
  std::cout << "Myminigfs_Server Object created" << std::endl;
}

Directory *mounted;

Replica *replica;
// member function

Json::Value
Myminigfs_Server::ObtainChunkURL
(const std::string& action, const std::string& arguments, const std::string& chunkindex, const std::string& class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID)
{
  Json::Value result;
  //
  //result["primary"]     = (this->the_chunk).chunk_url_primary;

  return result;
}

Json::Value
Myminigfs_Server::PushChunk2Replica
(const std::string& action, const std::string& arguments, const std::string& chunk, const std::string& chunkindex, const std::string& class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID)
{
  Json::Value result_A, result_B, result;

  std::string timestamp { std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) };

  std::cout <<" Line " << __LINE__ << " Inside the Primary.PushChunk2Replica " << timestamp << std::endl;
  result_A = gfs_secondary_A->PushChunk2Replica(filename, fhandle, chunkindex + timestamp, chunk);
  result_B = gfs_secondary_B->PushChunk2Replica(filename, fhandle, chunkindex + timestamp, chunk);

  if (result_A["status"] == "success" || result_B["status"] == "success") {
    result["status"] = "success";
  }

  std::cout << " Line " << __LINE__ << " primay PushChunk2Replica" << "\n";
  return result;
}

Json::Value
Myminigfs_Server::CommitAbort
(const std::string& action, const std::string& arguments, const std::string& chunkindex, const std::string& class_id, const std::string& commitorabort, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID)
{
  Json::Value result, result_A, result_B;
  //

  std::cout <<" Line " << __LINE__ << " Inside the Primary.CommitAbort" << std::endl;

  // result = replica->CommitAbort(filename, fhandle, chunkindex, commitorabort);
  result_A = gfs_secondary_A->CommitAbort(filename, fhandle, chunkindex, commitorabort);
  result_B = gfs_secondary_B->CommitAbort(filename, fhandle, chunkindex, commitorabort);

  if (result_A["vote"] == "commit" || result_B["vote"] == "commit") {
    result["vote"] = "commit";
  }

  return result;
}

Json::Value
Myminigfs_Server::LookUp(const std::string& action, const std::string& arguments, const std::string& class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID)
{
  Json::Value result;
  std::cout << action << " " << arguments << " " << owner_vsID << std::endl;

  std::cout << "SFelixWu receiving LookUp" << std::endl;

  if (fhandle != "00000002") // inode 2 is the root
    {
      result["status"] = "NFSERR_STALE";
    }
  else
    {
      result = mounted->LookUp(fhandle, filename);
    }

  return result;
}

Json::Value
Myminigfs_Server::Create(const std::string& action, const std::string& arguments, const std::string& class_id, const std::string& created_class_id, const std::string& fhandle, const std::string& filename, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID, const std::string& sattr)
{
  Json::Value result;
  std::cout << action << " " << arguments << " " << owner_vsID << std::endl;

  std::cout << "SFelixWu receiving Create" << std::endl;

  if (fhandle != "00000002")
    {
      result["status"] = "NFSERR_STALE";
    }
  else
    {
      result = mounted->Create(fhandle, filename, sattr);
    }

  return result;
}

Json::Value
Myminigfs_Server::dumpJ(const std::string& action, const std::string& arguments, const std::string& class_id, const std::string& host_url, const std::string& object_id, const std::string& owner_vsID)
{
  Json::Value result;
  std::cout << action << " " << arguments << " " << object_id << std::endl;

  std::cout << "SFelixWu receiving dumpJ primary" << std::endl;

  if (object_id != "00000002")
    {
      result["status"] = "NFSERR_STALE";
    }
  else
    {
      Json::Value *myv_ptr = mounted->dumpJ();
      if (myv_ptr != NULL)
	{
	  result = *myv_ptr;
	  result["status"] = "NFS_OK";
	}
      else
	{
	  result["status"] = "NFSERR_STALE";
	}
    }

  return result;
}

int
main()
{

  Directory NFS_root
  { "http://169.237.6.102", "1234567890", "Directory", "00000000", "root", "00000002" };

  mounted = (&NFS_root);

  Replica GFS;


  replica = (&GFS);

  Shadow_Replica main_gfs_secondary_A
    { "http://127.0.0.1:8301", "1234567890", "Replica", "00000002" };

  Shadow_Replica main_gfs_secondary_B
    { "http://127.0.0.1:8302", "1234567890", "Replica", "00000002" };

  gfs_secondary_A = &(main_gfs_secondary_A);
  gfs_secondary_B = &(main_gfs_secondary_B);

  HttpServer httpserver(8300);
  Myminigfs_Server s(httpserver,
		JSONRPC_SERVER_V1V2); // hybrid server (json-rpc 1.0 & 2.0)
  s.StartListening();
  std::cout << "Hit enter to stop the server" << endl;
  getchar();

  s.StopListening();
  return 0;
}
