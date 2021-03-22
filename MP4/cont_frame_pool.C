/*
 File: ContFramePool.C

 Author: Chrysanthos Pepi
 Date  : Feb 08 2021

 */

/*--------------------------------------------------------------------------*/
/*
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates
 *single* frames at a time. Because it does allocate one frame at a time,
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.

 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.

 This can be done in many ways, ranging from extensions to bitmaps to
 free-lists of frames etc.

 IMPLEMENTATION:

 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame,
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool.
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.

 NOTE: If we use this scheme to allocate only single frames, then all
 frames are marked as either FREE or HEAD-OF-SEQUENCE.

 NOTE: In SimpleFramePool we needed only one bit to store the state of
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work,
 revisit the implementation and change it to using two bits. You will get
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.

 DETAILED IMPLEMENTATION:

 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:

 Constructor: Initialize all frames to FREE, except for any frames that you
 need for the management of the frame pool, if any.

 get_frames(_n_frames): Traverse the "bitmap" of states and look for a
 sequence of at least _n_frames entries that are FREE. If you find one,
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.

 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.

 needed_info_frames(_n_frames): This depends on how many bits you need
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.

 A WORD ABOUT RELEASE_FRAMES():

 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e.,
 not associated with a particular frame pool.

 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete

 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

const unsigned int FRAMES_IN_BYTE = 4;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

ContFramePool *ContFramePool::head;
ContFramePool *ContFramePool::cont_frame_pool;

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames) {

    assert(_n_frames <= FRAME_SIZE * FRAMES_IN_BYTE)

    base_frame_no = _base_frame_no;
    n_frames = _n_frames;
    info_frame_no = _info_frame_no;
    n_info_frames = _n_info_frames;

    n_free_frames = _n_frames;

    if (info_frame_no == 0) {
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
    } else {
        bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }

    assert ((n_frames % FRAMES_IN_BYTE) == 0)

    //Let them FREE
    for (int i = 0; i < n_frames; i++) {
        bitmap[i] = 0x00;
    }

    // Mark the first frame as being used if it is being used
    if (info_frame_no == 0) {
        bitmap[0] = 0x80;
        n_free_frames--;
    }

    if (head == NULL) {
        head = this;
    } else {
        cont_frame_pool->next = this;
    }
    cont_frame_pool = this;
    Console::puts("Frame Pool initialized!\n");
}

void ContFramePool::set_state(int bitmap_idx, int byte_idx, STATE state) {
    unsigned char state_mask;
    unsigned char mask = 0xC0 >> (2 * byte_idx);

    switch (state) {
        case FREE:
            bitmap[bitmap_idx] ^= (bitmap[bitmap_idx] & (0xC0 >> (2 * byte_idx)));
            return;
        case ALLOCATED:
            state_mask = 0x40 >> (2 * byte_idx);
            break;
        case HEAD_OF_SEQUENCE:
            state_mask = 0x80 >> (2 * byte_idx);
            break;
        case INACCESSIBLE:
            state_mask = 0xC0 >> (2 * byte_idx);
            break;
    }

    bitmap[bitmap_idx] = (bitmap[bitmap_idx] & ~mask) | state_mask;
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames) {
    bool available = false;
    int frames_needed = _n_frames;

    //First let's check if we have _n_frames available
    if (_n_frames > n_free_frames) {
        Console::puts("Too many frames!\n");
        assert(_n_frames < n_free_frames)
    }

    int begin_bitmap_idx = -1;
    int end_bitmap_idx = -1;
    int begin_byte_idx = -1;
    int end_byte_idx = -1;

    //Check for contiguous frames
    for (int i = 0; i < n_frames / FRAMES_IN_BYTE; ++i) {
        unsigned char mask = 0xC0;
        for (int j = 0; j < FRAMES_IN_BYTE; ++j) {
            if ((bitmap[i] & mask) != 0x00) {
                frames_needed = _n_frames;
                begin_bitmap_idx = -1;
                end_bitmap_idx = -1;
                begin_byte_idx = -1;
                end_byte_idx = -1;
            } else {
                if (begin_bitmap_idx == -1) {
                    begin_bitmap_idx = i;
                    begin_byte_idx = j;
                }

                frames_needed--;
                if (frames_needed == 0) {
                    end_bitmap_idx = i;
                    end_byte_idx = j;
                    available = true;
                    break;
                }
            }
            mask >>= 2;
        }

        if (available) break;
    }

    if (!available) {
        Console::puts("Contiguous frames not found!\n");
        return 0;
    }

    //Let's allocate them
    int k = begin_byte_idx + 1;
    frames_needed = _n_frames - 1;
    set_state(begin_bitmap_idx, begin_byte_idx, HEAD_OF_SEQUENCE);
    for (int i = begin_bitmap_idx; i <= end_bitmap_idx; ++i) {
        for (int j = k; j < FRAMES_IN_BYTE; ++j) {
            if (frames_needed <= 0) {
                n_free_frames -= _n_frames;

                //Let's return the frame
                return (base_frame_no + end_bitmap_idx * FRAMES_IN_BYTE + end_byte_idx) - _n_frames + 1;
            }

            set_state(i, j, ALLOCATED);
            frames_needed--;
        }
        if (frames_needed <= 0) {
            n_free_frames -= _n_frames;

            //Let's return the frame
            int last_frame = base_frame_no + end_bitmap_idx * FRAMES_IN_BYTE + end_byte_idx;
            return last_frame - _n_frames + 1;
        }
        k = 0;
    }
    return 0;
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames) {
    int i = 0;
    int b = _base_frame_no - base_frame_no;
    set_state(b / FRAMES_IN_BYTE, i++, INACCESSIBLE);
    for (int j = i; j < _n_frames; ++j) {
        set_state(b / FRAMES_IN_BYTE, j % FRAMES_IN_BYTE, INACCESSIBLE);
        if (j % FRAMES_IN_BYTE == FRAMES_IN_BYTE - 1)
            b += 4;
    }

    n_free_frames -= _n_frames;
}

void ContFramePool::release_frames(unsigned long _first_frame_no) {
    //Now, let's find who you are
    ContFramePool *current = ContFramePool::head;
    while ((current->base_frame_no + current->n_frames <= _first_frame_no)) {
        if (current->next == NULL) {
            Console::puts("Frame not found!\n");
            return;
        } else {
            current = current->next;
        }
    }

    //Let's do a sanity check, this boy has to be HOS
    int frame = (_first_frame_no - (current->base_frame_no));
    unsigned char hos_state_mask = 0x80;
    unsigned char free_state_mask = 0x40;
    unsigned char mask = 0xC0;

    if ((current->bitmap[frame / FRAMES_IN_BYTE] & (mask >> (frame % FRAMES_IN_BYTE) * 2)) !=
        (hos_state_mask >> (frame % FRAMES_IN_BYTE) * 2)) {
        Console::puts("Frame not Head of sequence!\n");
        current->print_bitmap();
        assert(false)
    }

    //Let them FREE
    int freed_frames = 1;
    current->set_state(((_first_frame_no - (current->base_frame_no)) / FRAMES_IN_BYTE),
                       ((_first_frame_no - (current->base_frame_no)) % FRAMES_IN_BYTE), FREE);

    int next_frame_no = _first_frame_no + 1;
    int i = (next_frame_no - (current->base_frame_no)) / FRAMES_IN_BYTE;
    while (i < (current->n_frames / FRAMES_IN_BYTE)) {
        i = (next_frame_no - (current->base_frame_no)) / FRAMES_IN_BYTE;
        int j = (next_frame_no - (current->base_frame_no)) % FRAMES_IN_BYTE;
        mask = 0xC0 >> (j * 2);
        if ((current->bitmap[i] & mask) == (free_state_mask >> (j * 2))) {
            current->set_state(i, j, FREE);
            freed_frames++;
        } else {
            break;
        }
        next_frame_no++;
    }

    current->n_free_frames += freed_frames;
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames) {
    return _n_frames / (16 * 1024) + (_n_frames % (16 * 1024) > 0 ? 1 : 0);
}

void ContFramePool::print_bitmap() {
    Console::puts("\n--- Begin ---\n|");
    for (int i = 0; i * FRAMES_IN_BYTE < n_frames; i++) {
        Console::putui(bitmap[i]);
        Console::puts("|");
    }
    Console::puts("\n---- End ----\n");
}
