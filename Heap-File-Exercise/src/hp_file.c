#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file_structs.h"
#include "record.h"

#define CALL_BF(call)         \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      return 0;        \
    }                         \
  }

int HeapFile_Create(const char* fileName)
{   //Basically creating and initializing the header block(block 0) of the heap file
    int fd1;
    BF_Block* block;
    void* data;
    CALL_BF(BF_CreateFile(fileName));
    CALL_BF(BF_OpenFile(fileName, &fd1));

    BF_Block_Init(&block);
    BF_AllocateBlock(fd1, block);
    //initialize header info 
    HeapFileHeader hdr;
    hdr.blockCount = 0;
    hdr.blockSize = BF_BLOCK_SIZE;
    hdr.firstFreeBlock = -1; 
    data = BF_Block_GetData(block);
    // write header info to block data using memcpy
    memcpy(data, &hdr, sizeof(HeapFileHeader));
    //write back to disk
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    CALL_BF(BF_CloseFile(fd1));
    return 1;
}

int HeapFile_Open(const char *fileName, int *file_handle, HeapFileHeader** header_info)
{   //basically opening the file and retriving the header info from block 0
    if (fileName == NULL || file_handle == NULL || header_info == NULL) {
        return 0; // Invalid arguments
    }
    CALL_BF(BF_OpenFile(fileName, file_handle));
    BF_Block* block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(*file_handle, 0, block)); // pinned
    void* data = BF_Block_GetData(block);
    *header_info = (HeapFileHeader*)malloc(sizeof(HeapFileHeader));
    if (*header_info == NULL) {
        CALL_BF(BF_UnpinBlock(block));
        BF_Block_Destroy(&block);
        return 0; // Memory allocation failed
    }
    memcpy(*header_info, data, sizeof(HeapFileHeader));
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
  return 1;
}

int HeapFile_Close(int file_handle, HeapFileHeader *hp_info)
{
    if (hp_info == NULL) {
        return 0; // Invalid argument
    }
    //writing back to the header i.e. block 0 the last changes made to hp_info
    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(file_handle, 0, block)); // pinned
    void* data = BF_Block_GetData(block);
    memcpy(data, hp_info, sizeof(HeapFileHeader));
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    //freeing the heapfile from the memory
    CALL_BF(BF_CloseFile(file_handle));
    free(hp_info);
  return 1;
}

int HeapFile_InsertRecord(int file_handle, HeapFileHeader *hp_info, const Record record)
{
  int rec_size= sizeof(Record);//the first step of the insertion is to calculate the size of the record
  BF_Block* block;// And then calculating the offset where the new record will be inserted
  BF_Block_Init(&block);
  //but first for the first block we have to do it differently
  if (hp_info->blockCount==0){
    //we have to allocate the first block
    CALL_BF(BF_AllocateBlock(file_handle, block));
    hp_info->blockCount=1;
    hp_info->firstFreeBlock=1;
    //initialize record count to 0
    char* data = BF_Block_GetData(block);
    int* recCountPtr = (int*)data;
    *recCountPtr = 0;
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    //now we can proceed to insert the record in the newly allocated block
  }
  int target_space;
  //checking if the heapfile has only the header and gettign the 
  if (hp_info->firstFreeBlock== -1){
    target_space=hp_info->firstFreeBlock;
  } else {
    target_space=hp_info->blockCount;
  }//now fetching the block where we will insert the record from the disc using the heapfile info
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(file_handle, target_space, block));
  char* data = BF_Block_GetData(block); //reading the first 4 bytes to get the record count
  int* recCountPtr = (int*)data;
  int recCount = *recCountPtr;
  int maxRecs = (hp_info->blockSize - sizeof(int)) / rec_size;
  if (recCount== maxRecs) {
    // current block full — allocate a new block
        BF_Block_SetDirty(block);
        CALL_BF(BF_UnpinBlock(block));
        BF_Block_Destroy(&block);
        //now creating a new block
        BF_Block_Init(&block);
        CALL_BF(BF_AllocateBlock(file_handle, block));
        char *new_data = BF_Block_GetData(block);
        int *newCount = (int *)new_data;
        *newCount = 0;
      //update heapfile header info
        hp_info->blockCount += 1;
        hp_info->firstFreeBlock = hp_info->blockCount;

        data = new_data;
        recCountPtr = newCount;
        recCount = 0;
  }
  // Lastly insert the record
  int offset = sizeof(int) + recCount * rec_size;
  memcpy(data + offset, &record, rec_size);
  (*recCountPtr)++;

  BF_UnpinBlock(block);
  BF_Block_SetDirty(block);
  BF_Block_Destroy(&block);
  //update header info 
    BF_Block *hdr_block;
    BF_Block_Init(&hdr_block);
    CALL_BF(BF_GetBlock(file_handle, 0, hdr_block));
    char *hdr_data = BF_Block_GetData(hdr_block);
    memcpy(hdr_data, hp_info, sizeof(HeapFileHeader));
    BF_Block_SetDirty(hdr_block);
    CALL_BF(BF_UnpinBlock(hdr_block));
    BF_Block_Destroy(&hdr_block);

  return 0;
}


HeapFileIterator HeapFile_CreateIterator(    int file_handle, HeapFileHeader* header_info, int id)
{
  HeapFileIterator out;
  out.file_handle=file_handle;
  out.currentBlock=1;//block 0 has the metadata
  out.currentRecord=0;
  out.totalBlocks=header_info->blockCount;
  out.record_id=id;
  return out;
}


int HeapFile_GetNextRecord(    HeapFileIterator* heap_iterator, Record** record){ 
  if( heap_iterator==NULL || record==NULL){
    return 0; // Invalid arguments
  }
  *record= NULL;
  if(heap_iterator->totalBlocks==0) return 0; // Empty heap file
  //first looping through the blocks
  while(heap_iterator->currentBlock <= heap_iterator->totalBlocks){
    BF_Block* block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(heap_iterator->file_handle, heap_iterator->currentBlock, block));

    char* data = BF_Block_GetData(block);
    int recCount= *(int*)data;
    int rec_size = sizeof(Record);
    //scaning the records in the current block
    while(heap_iterator->currentRecord < recCount){
      int offset = sizeof(int) + heap_iterator->currentRecord * rec_size;
      Record* current_record = (Record*)malloc(sizeof(Record));
      memcpy(current_record, data + offset, rec_size);
      heap_iterator->currentRecord++;
      //check if the current record matches the desired id
      if(current_record->id == heap_iterator->record_id){
        *record = current_record;
        BF_UnpinBlock(block);
        BF_Block_Destroy(&block);
        return 1; // Record found
      }
      free(current_record); // Free if not matching
    }
    //move to the next block
    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);
    heap_iterator->currentBlock++;
    heap_iterator->currentRecord=0;
  }
  return 0; // No more records
}
 

