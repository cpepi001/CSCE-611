#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable *PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool *PageTable::kernel_mem_pool = NULL;
ContFramePool *PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;


void PageTable::init_paging(ContFramePool *_kernel_mem_pool,
                            ContFramePool *_process_mem_pool,
                            const unsigned long _shared_size) {
    kernel_mem_pool = _kernel_mem_pool;
    process_mem_pool = _process_mem_pool;
    shared_size = _shared_size;
    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable() {
    page_directory = (unsigned long *) ((kernel_mem_pool->get_frames(1)) * PAGE_SIZE);
    unsigned long *page_table = (unsigned long *) ((kernel_mem_pool->get_frames(1)) * PAGE_SIZE);

    unsigned long address = 0;
    // Map the first 4MB of memory
    for (int i = 0; i < 1024; i++) {
        page_table[i] = address | 3; //Attribute set to: supervisor level, read/write, present (011 in binary)
        address = address + 4096; //4kb
    }

    page_directory[0] = (unsigned long) page_table;
    page_directory[0] = page_directory[0] | 3; //Attribute set to: supervisor level, read/write, present (011 in binary)

    // Mark rest of the page table as not present in the page directory
    for (int j = 1; j < 1024; j++) {
        //attribute set to: supervisor level, read/write, not present (010 in binary)
        page_directory[j] = 0 | 2;
    }
    Console::puts("Constructed Page Table object\n");
}


void PageTable::load() {
    current_page_table = this;
    write_cr3((unsigned long) page_directory);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging() {
    paging_enabled = 1;
    write_cr0(read_cr0() | 0x80000000);
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS *_r) {
    unsigned long page_addr = read_cr2();
    unsigned long pd_idx = page_addr >> 22;
    unsigned long pt_idx = page_addr >> 12 & 0x03FF;
    unsigned long * page_directory = (unsigned long *) read_cr3();

    if ((_r->err_code & 0x1) == 0) {
        if ((page_directory[pd_idx] & 0x1) == 0) {
            unsigned long * page_table = (unsigned long *) ((kernel_mem_pool->get_frames(1)) * PAGE_SIZE);
            //attribute set to: supervisor level, read/write, present (011 in binary)
            page_directory[pd_idx] = ((unsigned long) page_table) | 3;
        }

        unsigned long * pt_address = ((unsigned long *) (page_directory[pd_idx] & 0xFFFFF000));

        if ((pt_address[pt_idx] & 0x1) == 0) {
            unsigned long * page = (unsigned long *) ((process_mem_pool->get_frames(1)) * PAGE_SIZE);
            //attribute set to: supervisor level, read/write, present (011 in binary)
            pt_address[pt_idx] = ((unsigned long) page) | 3;
        }
    }
    Console::puts("handled page fault\n");
}

