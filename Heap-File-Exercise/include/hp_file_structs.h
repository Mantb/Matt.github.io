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
    int pageCount;     // Number of pages in the heap file
    int pageSize;      // Size of each page in bytes
    int firstFreePage; // Index of the first page with free space
} HeapFileHeader;

/**
 * @brief Iterator for scanning through records in a heap file
 */
typedef struct HeapFileIterator{
    int file_handle;    //file descriptor for the heap file
    int currentPage;  //current page index (1..pageCount) */
    int currentSlot;  //current slot index within current page (0..slotCount-1) */
    int totalPages;   // Cached total number of pages to know when to stop
    int record_id; //just for the search implemetation for the hp_main
} HeapFileIterator;

#endif /* HP_FILE_STRUCTS_H */
