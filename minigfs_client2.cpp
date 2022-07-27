
// ecs251 miniGFS

#include <iostream>
#include "Shadow_Directory.h"
#include "Shadow_Replica.h"

using namespace std;

int
main()
{
  Shadow_Directory gfs_master
  { "http://127.0.0.1:8384", "1234567890", "Directory", "00000002" };

  Json::Value result, result_P, result_A, result_B;

  std::cout<<" **************************************************************************"<<std::endl;
  std::cout<<" ******* Step 1 of GFS Paper Call from Client to Master for 3 URLs ********"<<std::endl;
  std::cout<<" **************************************************************************"<<std::endl;

  result = gfs_master.ObtainChunkURL("my_ecs251_file", "00000002", "0");

  std::cout<<" *****************************************************************************************"<<std::endl;
  std::cout<<" ******** Step 1 & 2 of GFS Diagram Complete URLs Obtained *******************************"<<std::endl;
  std::cout<<" ******************************************************************************************"<<std::endl;


  std::string url_primary = (result["primary"]).asString();
  Shadow_Replica gfs_primary
  { url_primary, "1234567890", "Replica", "00000002" };

  std::string url_secondary_A = (result["secondary_A"]).asString();
  Shadow_Replica gfs_secondary_A
  { url_secondary_A, "1234567890", "Replica", "00000002" };

  std::string url_secondary_B = (result["secondary_B"]).asString();
  Shadow_Replica gfs_secondary_B
  { url_secondary_B, "1234567890", "Replica", "00000002" };

  std::string my_chunk_data = { "c1-150-ecs251 data" };

  std::cout<<" ***********************************************************************************************************************************************************"<<std::endl;
  std::cout<<" **** Step 3 of GFS Paper Call from Client to Primary, Secondary A, Secondary B for PushChunk2Replica to push the chunks and get acknowledgment from all ****"<<std::endl;
  std::cout<<" ***********************************************************************************************************************************************************"<<std::endl;

  std::cout<<" ----------------------------------------------- " <<std::endl;

  std::cout<<"******* Acknowlegment Vote from Primary ***********" << std::endl;
  result_P = gfs_primary.PushChunk2Replica("my_ecs251_file", "00000002", "0", my_chunk_data);
  std::cout<<" VOTE = " << result_P["status"]<<std::endl;

  std::cout<<" ----------------------------------------------- " <<std::endl;

  // std::cout<<"******* Acknowlegment Vote from Secondary A ***********" << std::endl;
  // result_A = gfs_secondary_A.PushChunk2Replica("my_ecs251_file", "00000002", "0", my_chunk_data);
  // std::cout<<" VOTE = " << result_A["vote"]<<std::endl;

  // std::cout<<" ----------------------------------------------- " <<std::endl;

  // std::cout<<"******* Acknowlegment Vote from Secondary B ***********" << std::endl;
  // result_B = gfs_secondary_B.PushChunk2Replica("my_ecs251_file", "00000002", "0", my_chunk_data);
  // std::cout<<" VOTE = " << result_B["vote"]<<std::endl;

  // std::cout<<" ----------------------------------------------- " <<std::endl;

  std::cout<<" **************************************"<<std::endl;
  std::cout<<" **** Step 3 of GFS Paper Complete ****"<<std::endl;
  std::cout<<" **************************************"<<std::endl;

  std::cout<<" ----------------------------------------------- " <<std::endl;

  std::cout<<" ***********************************************************************************************************************************************************"<<std::endl;
  std::cout<<" ******** Step 4,5,6,7 of GFS Paper Call from Client to Primary, Secondary A, Secondary B for CommitAbort to commit the data based on the result of the votes ****"<<std::endl;
  std::cout<<" ***********************************************************************************************************************************************************"<<std::endl;

  if (((result_P["status"]).asString() == "success"))
    {
      result_P = gfs_primary.CommitAbort("my_ecs251_file", "00000002", "0", "commit");
      // result_A = gfs_secondary_A.CommitAbort("my_ecs251_file", "00000002", "0", "commit");
      // result_B = gfs_secondary_B.CommitAbort("my_ecs251_file", "00000002", "0", "commit");
    }

  else
    {
     result_P = gfs_primary.CommitAbort("my_ecs251_file", "00000002", "0", "abort");
    //  result_A = gfs_secondary_A.CommitAbort("my_ecs251_file", "00000002", "0", "abort");
    //  result_B = gfs_secondary_B.CommitAbort("my_ecs251_file", "00000002", "0", "abort");
    }

  return 0;
}
