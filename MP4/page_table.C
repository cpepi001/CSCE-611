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
    page_directory = (unsigned long *) ((process_mem_pool->get_frames(1)) * PAGE_SIZE);
    unsigned long *page_table = (unsigned long *) ((process_mem_pool->get_frames(1)) * PAGE_SIZE);

    unsigned long address = 0;
    // Map the first 4MB of memory
    for (int i = 0; i < 1024; ++i) {
        page_table[i] = address | 3; //Attribute set to: supervisor level, read/write, present (011 in binary)
        address = address + PAGE_SIZE;
    }

    page_directory[0] = (unsigned long) page_table;
    page_directory[0] = page_directory[0] | 3; //Attribute set to: supervisor level, read/write, present (011 in binary)

    // Mark rest of the page table as not present in the page directory
    for (int i = 1; i < 1024; ++i) {
        //attribute set to: supervisor level, read/write, not present (010 in binary)
        page_directory[i] = 2;
    }

    // Last entry pointing to it self
    page_directory[1023] = (unsigned long) page_directory | 3;
    vm_pool_number = 0;

    for (int i = 0; i < VM_POOL_SIZE; ++i) {
        vm_array[i] = NULL;
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
    if (_r->err_code & 0xFFFFFFFE) {

        bool is_address_legitimate = false;
        unsigned long page_fault_address = read_cr2();
        VMPool **vm_array = current_page_table->vm_array;

        for (int i = 0; i < current_page_table->vm_pool_number; ++i) {
            if (vm_array[i] != NULL) {
                if (vm_array[i]->is_legitimate(page_fault_address)) {
                    is_address_legitimate = true;
                    break;
                }
            }
        }

        if (!is_address_legitimate) {
            Console::puts("address is not legitimate\n");
            assert(false)
        }

        unsigned long *page_directory = (unsigned long *) (0xFFFFF000);
        unsigned long page_directory_entry = page_fault_address >> 22;
        unsigned long page_table_entry = (page_fault_address >> 12) & 0x3FF;

        if (((page_directory[page_directory_entry]) & 0x1) == 1) {
            unsigned long *page_table = (unsigned long *) (0xFFC00000 | (page_directory_entry << 12));
            page_table[page_table_entry] = (PageTable::process_mem_pool->get_frames(1) * PAGE_SIZE) | 3;
        } else {
            page_directory[page_directory_entry] = (unsigned long) (process_mem_pool->get_frames(1) * PAGE_SIZE) | 3;
            unsigned long *page_table = (unsigned long *) (0xFFC00000 | (page_directory_entry << 12));

            for (int i = 0; i < 1024; ++i) {
                page_table[i] = 4;
            }

            page_table[page_table_entry] = (process_mem_pool->get_frames(1) * PAGE_SIZE) | 3;
        }
    }
    Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool *_vm_pool) {
    if (vm_pool_number >= VM_POOL_SIZE) {
        Console::puts("cannot registered VM pool\n");
        return;
    }

    vm_array[vm_pool_number++] = _vm_pool;
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
    unsigned long *page_table_entry = (unsigned long *) (
            (0xFFC00000 | (((_page_no >> 10) & 0x003FF000) | ((_page_no >> 10) & 0x00000FFC))) & 0xFFFFFFFC);
    if ((*page_table_entry & 0x1)) {
        process_mem_pool->release_frames(*page_table_entry >> 12);
        *page_table_entry = 2;

        // Flush TLB
        write_cr3((unsigned long) page_directory);
        Console::puts("freed page\n");
    }
}