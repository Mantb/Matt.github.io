#ifndef HP_FILE_STRUCTS_H
#define HP_FILE_STRUCTS_H
#include <record.h>

/**
 * @file hp_file_structs.h
 * @brief Data structures for heap file management
 */

/* -------------------------------------------------------------------------- */
/*                              Data Structures                               */
/* -------------------------------------------------------------------------- */

/**
 * @brief Heap file header containing metadata about the file organization
 */
typedef struct HeapFileHeader {
    int blockCount;     // Number of blocks in the heap file
    int blockSize;      // Size of each block in bytes
    int firstFreeBlock; // Index of the first block with free space (-1 if none)
} HeapFileHeader;

/**
 * @brief Iterator for scanning through records in a heap file
 */
typedef struct HeapFileIterator{
    int file_handle;    //file descriptor for the heap file
    int currentBlock;  //current block index (1..blockCount-1) */
    int currentRecord;  //current record index within current block (0..recordCount-1) */
    int totalBlocks;   // Cached total number of blocks to know when to stop
    //those three above are to help with the next recort implementation
    int record_id; //just for the search implemetation for search in the hp_main
} HeapFileIterator;

#endif /* HP_FILE_STRUCTS_H */
