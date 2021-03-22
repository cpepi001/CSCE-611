/*
 File: vm_pool.C
 
 Author: Chrysanthos Pepi
 Date  : Mar 16 2021
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long _base_address,
               unsigned long _size,
               ContFramePool *_frame_pool,
               PageTable *_page_table) {
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;

    region_number = 0;
    memory_list = (memory_region *) (base_address);

    page_table->register_pool(this);
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    if (region_number >= MAX_REGIONS) {
        Console::puts("Max number of regions.\n");
        assert(false)
    }

    if (_size == 0) {
        return 0;
    }

    if (region_number == 0) {
        memory_list[region_number].start_address = base_address + Machine::PAGE_SIZE;
        memory_list[region_number].size = Machine::PAGE_SIZE;
        return memory_list[region_number++].start_address;
    }

    memory_list[region_number].start_address =
            memory_list[region_number - 1].start_address + memory_list[region_number - 1].size;

    // How many pages?
    int quotient = (_size / Machine::PAGE_SIZE);
    int remainder = (_size % Machine::PAGE_SIZE);
    memory_list[region_number].size = quotient * Machine::PAGE_SIZE + ((remainder == 0) ? 0 : Machine::PAGE_SIZE);

    Console::puts("Allocated region of memory.\n");
    return memory_list[region_number++].start_address;
}

void VMPool::release(unsigned long _start_address) {
    int region = -1;
    // Let's find the region
    for (int i = 0; i < MAX_REGIONS; ++i) {
        if (memory_list[i].start_address == _start_address) {
            region = i;
            break;
        }
    }

    if (region == -1) {
        Console::puts("Region not found.\n");
        assert(false)
    }

    // Let them FREE
    for (int j = 0; j < memory_list[region].size; j += Machine::PAGE_SIZE) {
        page_table->free_page(memory_list[region].start_address + j);
    }

    for (int j = region; j < region_number - 1; ++j) {
        memory_list[j] = memory_list[j + 1];
    }
    region_number--;

    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    bool flag = false;
    if ((_address >= base_address) && (_address < base_address + size)) {
        flag = true;
    }

    Console::puts("Checked whether address is part of an allocated region.\n");
    return flag;
}

