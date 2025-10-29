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
{
    int fd1;
    BF_Block* block;
    void* data;
    CALL_BF(BF_CreateFile(fileName));
    CALL_BF(BF_OpenFile(fileName, &fd1));

    BF_Block_Init(&block);
    BF_AllocateBlock(fd1, block);
    
    HeapFileHeader hdr;
    hdr.pageCount = 0;
    hdr.pageSize = BF_BLOCK_SIZE;
    hdr.firstFreePage = UINT32_MAX; /* no free page yet */
    data = BF_Block_GetData(block);
    // write header info to block data using HeapFileHeader struct pointer to data 
    HeapFileHeader* header_ptr = (HeapFileHeader*)data;
    header_ptr->pageCount = hdr.pageCount;
    header_ptr->pageSize = hdr.pageSize;
    header_ptr->firstFreePage = hdr.firstFreePage;
    BF_Block_SetDirty(block);

    /* unpin and destroy block object */
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    CALL_BF(BF_CloseFile(fd1));
    return 1;
}

int HeapFile_Open(const char *fileName, int *file_handle, HeapFileHeader** header_info)
{
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
    BF_Block* block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(file_handle, 0, block)); // pinned
    void* data = BF_Block_GetData(block);
    memcpy(data, hp_info, sizeof(HeapFileHeader));
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    
    CALL_BF(BF_CloseFile(file_handle));
    free(hp_info);
  return 1;
}

int HeapFile_InsertRecord(int file_handle, HeapFileHeader *hp_info, const Record record)
{
  int rec_size= sizeof(Record);
  BF_Block* block;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(file_handle, 0, block)); // pinned
  char* data = BF_Block_GetData(block);
  BF_Block* block;
  /* page-local record count stored at first 2 bytes */
  uint16_t* recCountPtr = (uint16_t*)data;
  uint16_t recCount = *recCountPtr;
  /* offset inside THIS page */
  int offset = sizeof(uint16_t) + recCount * rec_size;
  return 1;
}


HeapFileIterator HeapFile_CreateIterator(    int file_handle, HeapFileHeader* header_info, int id)
{
  HeapFileIterator out;
  return out;
}


int HeapFile_GetNextRecord(    HeapFileIterator* heap_iterator, Record** record)
{
    * record=NULL;
    return 1;
}

