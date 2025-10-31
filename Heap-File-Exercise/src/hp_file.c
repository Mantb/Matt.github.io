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
  //but first for the first page we have to do it differently
  if (hp_info->pageCount==0){
    //we have to allocate the first block
    CALL_BF(BF_AllocateBlock(file_handle, block));
    hp_info->pageCount=1;
    hp_info->firstFreePage=1;
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
  if (hp_info->firstFreePage== UINT32_MAX){
    target_space=hp_info->firstFreePage;
  } else {
    target_space=hp_info->pageCount;
  }
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(file_handle, target_space, block));
  char* data = BF_Block_GetData(block);
  int* recCountPtr = (int*)data;
  int recCount = *recCountPtr;
  int maxRecs = (hp_info->pageSize - sizeof(int)) / rec_size;
  if (recCount== maxRecs) {
    // current block full — allocate a new page
        BF_Block_SetDirty(block);
        CALL_BF(BF_UnpinBlock(block));
        BF_Block_Destroy(&block);
        //now creating a new block
        BF_Block_Init(&block);
        CALL_BF(BF_AllocateBlock(file_handle, block));
        char *new_data = BF_Block_GetData(block);
        int *newCount = (int *)new_data;
        *newCount = 0;

        hp_info->pageCount += 1;
        hp_info->firstFreePage = hp_info->pageCount;

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
  out.currentPage=1;//block 0 has the metadata
  out.currentSlot=0;
  out.totalPages=header_info->pageCount;
  out.record_id=id;
  return out;
}


int HeapFile_GetNextRecord(    HeapFileIterator* heap_iterator, Record** record){ 
  if( heap_iterator==NULL || record==NULL){
    return 0; // Invalid arguments
  }
  *record= NULL;
  if(heap_iterator->totalPages==0) return 0; // Empty heap file
  while(heap_iterator->currentPage <= heap_iterator->totalPages){
    BF_Block* block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(heap_iterator->file_handle, heap_iterator->currentPage, block));

    char* data = BF_Block_GetData(block);
    int recCount= *(int*)data;
    int rec_size = sizeof(Record);
    while(heap_iterator->currentSlot < recCount){
      int offset = sizeof(int) + heap_iterator->currentSlot * rec_size;
      Record* current_record = (Record*)malloc(sizeof(Record));
      memcpy(current_record, data + offset, rec_size);
      heap_iterator->currentSlot++;
      //check if the current record matches the desired id
      if(current_record->id == heap_iterator->record_id){
        *record = current_record;
        BF_UnpinBlock(block);
        BF_Block_Destroy(&block);
        return 1; // Record found
      }
      free(current_record); // Free if not matching
    }
    //move to the next page
    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);
    heap_iterator->currentPage++;
    heap_iterator->currentSlot=0;
  }
  return 1;
}
 

