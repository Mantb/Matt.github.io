#ifndef HP_FILE_STRUCTS_H
#define HP_FILE_STRUCTS_H
#include <stdint.h>
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
    uint32_t pageCount;     // Number of pages in the heap file
    uint32_t pageSize;      // Size of each page in bytes
    uint32_t firstFreePage; // Index of the first page with free space
} HeapFileHeader;

/**
 * @brief Iterator for scanning through records in a heap file
 */
typedef struct HeapFileIterator{
    int file_handle;       /* BF file handle */
    uint32_t currentPage;  /* current page index (1..pageCount) */
    uint32_t currentSlot;  /* current slot index within current page (0..slotCount-1) */
    uint32_t totalPages;   /* cached total pages for quick EOF detection */ // Current record slot in the page   // Cached total number of pages to know when to stop
} HeapFileIterator;

#endif /* HP_FILE_STRUCTS_H */
