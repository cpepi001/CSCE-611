/* 
    File: file_system.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 10/04/05

    Description: Simple File System.
    

*/

#ifndef _FILE_SYSTEM_H_                   // include file only once
#define _FILE_SYSTEM_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define BYTE 8
#define INDEX_SIZE 16
#define BLOCK_SIZE  512

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "file.H"
#include "simple_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

typedef struct node {
    int file_id;
    unsigned int size;
    unsigned long block[INDEX_SIZE];
} i_node;

/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */
/*--------------------------------------------------------------------------*/

class File;

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/

class FileSystem {

    friend class File;

private:
    /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */

    i_node *iNode;
    SimpleDisk *disk;
    unsigned long total_blocks;
    unsigned char buf[BLOCK_SIZE];
    unsigned char block_map[BLOCK_SIZE];

    void set_iNode(int);

public:

    FileSystem();

    /* Just initializes local data structures. Does not connect to disk yet. */

    bool Mount(SimpleDisk *_disk);

    /* Associates this file system with a disk. Limit to at most one file system per disk.
     Returns true if operation successful (i.e. there is indeed a file system on the disk.) */

    bool Format(SimpleDisk *_disk, unsigned int _size);

    /* Wipes any file system from the disk and installs an empty file system of given size. */

    File *LookupFile(int _file_id);

    /* Find file with given id in file system. If found, return the initialized
     file object. Otherwise, return null. */

    bool CreateFile(int _file_id);

    /* Create file with given id in the file system. If file exists already,
     abort and return false. Otherwise, return true. */

    bool DeleteFile(int _file_id);
    /* Delete file with given id in the file system; free any disk block occupied by the file. */

    void EraseFile(int);

    void UpdateFile(File *, int, unsigned int);

    int GetBlock();

    void FreeBlock(unsigned long);

};

#endif
