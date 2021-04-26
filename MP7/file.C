/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
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
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File() {
    /* We will need some arguments for the constructor, maybe pointer to disk
     block with file management and allocation data. */
    Console::puts("In file constructor.\n");

    file_id = -1;
    size = 0;

    position = 0;
    current_block = -1;

    file_system = NULL;
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("Reading from file\n");

    if (current_block == -1) {
        Console::puts("File not initialized\n");
        assert(false)
    }

    int block = 0;
    memset(buf, 0, BLOCK_SIZE);
    file_system->disk->read(current_block, buf);

    while (!EoF() && (_n > 0)) {
        _buf[block++] = (char) buf[position++];
        _n--;
    }

    return block;
}


void File::Write(unsigned int _n, const char *_buf) {
    Console::puts("Writing to file\n");

    if (current_block == -1) {
        Console::puts("File not initialized\n");
        assert(false)
    }

    int block = 0;
    memset(buf, 0, BLOCK_SIZE);

    while (_n > 0) {
        buf[position++] = _buf[block++];
        _n--;
    }

    file_system->disk->write(current_block, buf);
    file_system->UpdateFile(this, file_id, block);
}

void File::Reset() {
    Console::puts("Reset current position in file\n");

    position = 0;
    current_block = blocks[0];
}

void File::Rewrite() {
    Console::puts("Erase content of file\n");

    file_system->EraseFile(file_id);
    for (int i = 1; i < INDEX_SIZE; ++i) {
        blocks[i] = -1;
    }
}


bool File::EoF() {
    Console::puts("Testing end-of-file condition\n");
    return position > size;
}
