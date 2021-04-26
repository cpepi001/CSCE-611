/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk *_disk) {
    Console::puts("Mounting file system form disk\n");

    disk = _disk;
    return true;
}

bool FileSystem::Format(SimpleDisk *_disk, unsigned int _size) {
    Console::puts("Formatting disk\n");

    disk = _disk;
    memset(buf, 0, BLOCK_SIZE);
    total_blocks = (_size / BLOCK_SIZE) / BYTE;

    for (int i = 0; i < total_blocks; ++i) {
        block_map[i] = 0;
        disk->write(i, buf);
    }

    block_map[0] = 0xFF;
    return true;
}

File *FileSystem::LookupFile(int _file_id) {
    Console::puts("Looking up file\n");

    for (int i = 0; i < total_blocks; ++i) {
        set_iNode(i);

        for (int j = 0; j < total_blocks; ++j) {
            if (block_map[j] == 0xFF && iNode[j].file_id == _file_id) {
                File *file = (File *) new File();

                file->position = 0;
                file->file_id = _file_id;
                file->size = iNode[j].size;
                file->current_block = iNode[j].block[0];

                for (int k = 0; k < INDEX_SIZE; ++k) {
                    file->blocks[k] = iNode[j].block[k];
                }

                Console::puts("File ");
                Console::putui(_file_id);
                Console::puts(" found\n");

                file->file_system = FILE_SYSTEM;
                return file;
            }
        }
    }

    return NULL;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("Creating file\n");

    if (LookupFile(_file_id) != NULL) {
        Console::puts("File already exists\n");
        return false;
    }

    for (int i = 0; i < total_blocks; ++i) {
        set_iNode(i);

        for (int j = 0; j < total_blocks; ++j) {
            if (iNode[j].file_id == 0) {
                iNode[j].file_id = _file_id;

                memset(iNode[j].block, -1, INDEX_SIZE * BYTE);
                iNode[j].block[0] = GetBlock();

                Console::puts("File ");
                Console::putui(_file_id);
                Console::puts(" created\n");

                disk->write(i, buf);
                return true;
            }
        }
    }

    return false;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("Deleting file\n");

    for (int i = 0; i < total_blocks; ++i) {
        set_iNode(i);

        for (int j = 0; j < total_blocks; ++j) {
            if (iNode[j].file_id == _file_id) {
                iNode[j].file_id = 0;
                iNode[j].size = 0;

                for (int k = 0; k < INDEX_SIZE; ++k) {
                    if (iNode[j].block[k] != -1) FreeBlock(iNode[j].block[k]);
                }

                memset(iNode[j].block, -1, INDEX_SIZE * BYTE);

                Console::puts("File ");
                Console::putui(_file_id);
                Console::puts(" deleted\n");

                disk->write(i, buf);
                return true;
            }
        }
    }
    return false;
}

void FileSystem::EraseFile(int _file_id) {
    Console::puts("Erasing file\n");

    for (int i = 0; i < total_blocks; ++i) {
        set_iNode(i);

        for (int j = 0; j < total_blocks; ++j) {
            if (iNode[j].file_id == _file_id) {
                iNode[j].size = 0;

                unsigned char temp_buf[BLOCK_SIZE];
                memset(temp_buf, 0, BLOCK_SIZE);

                for (int k = 1; k < INDEX_SIZE; ++k) {
                    if (iNode[j].block[k] != -1) {
                        disk->write(iNode[j].block[k], temp_buf);
                        FreeBlock(iNode[j].block[k]);
                        iNode[j].block[k] = -1;
                    }
                }

                Console::puts("File ");
                Console::putui(_file_id);
                Console::puts(" erased\n");

                disk->write(i, buf);
                return;
            }
        }
    }
}

void FileSystem::UpdateFile(File *file, int _file_id, unsigned int size) {
    Console::puts("Updating file\n");

    for (int i = 0; i < total_blocks; ++i) {
        set_iNode(i);

        for (int j = 0; j < total_blocks; ++j) {
            if (iNode[j].file_id == _file_id) {
                iNode[j].size += size;
                file->size = iNode[j].size;

                Console::puts("File ");
                Console::putui(_file_id);
                Console::puts(" updated\n");

                disk->write(i, buf);
                return;
            }
        }
    }
}

int FileSystem::GetBlock() {
    int block = NULL;
    for (int i = 0; i < total_blocks; ++i) {
        if (block_map[i] != 0xFF) {
            block_map[i] = 0xFF;
            block = i;
            break;
        }
    }

    if (block == NULL)
        assert(false)

    Console::puts("Block ");
    Console::putui(block);
    Console::puts(" acquired\n");

    return block;
}

void FileSystem::FreeBlock(unsigned long block) {
    block_map[block] = 0;

    Console::puts("Block ");
    Console::putui(block);
    Console::puts(" freed\n");
}

void FileSystem::set_iNode(int i) {
    memset(buf, 0, BLOCK_SIZE);

    disk->read(i, buf);
    iNode = (i_node *) buf;
}