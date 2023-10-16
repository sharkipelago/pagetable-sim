#include"config.h"
#include"mlpt.h"
#include<stdlib.h>
#include<stdio.h>
#include<assert.h>

/* After Code Review:
 *	- Testing variables and print statments removed
 *	- 8 space tabs replaced with 4 spaces
 *	- camel case variable names replaced with snake case
 * */

size_t ptbr = 0;

void page_allocate(size_t va){
    
    // In bytes
    size_t page_size = 1 << POBITS;
    if (ptbr == 0) 
        //1 following 0 times pobits 2^pobits
        posix_memalign((void *)&ptbr, page_size, page_size);

    size_t *table_entry = (size_t *)ptbr;

    // 512 Entries = 2^9, need 9 bits for 12 POBITS
    int index_bits = POBITS - 3;
    size_t vp_index = (va >> POBITS);

    int current_level = 0;
    size_t level_index_mask = (size_t) ~0 >> (64 - index_bits);

    while(current_level < LEVELS) {
        //E.g. vp_index = AAABBBCCC
        //E.g. after this vp_index = AAA for level 1 or AAABBB for level 2 
        size_t currvp_index = (vp_index >> (((LEVELS  - current_level) - 1) * index_bits));
        //E.g. now vp_index = AAA for level 1 or BBB for level 2 
        currvp_index &= level_index_mask;
    
        //Check that the address is allocated
        if ((table_entry[currvp_index] & 1) == 0) {
            //Because new page will be aligned with page_size (which has to be at least 4) last 4 bits are guaranteed to be 0
            posix_memalign((void *)&table_entry[currvp_index], page_size, page_size);
            // Set valid bit to 1
            table_entry[currvp_index] |= 1;
        }
            
        //To account for valid bit check -0x1
        table_entry = (size_t *)(table_entry[currvp_index] - 0x1);
        current_level += 1;    
    }
}

size_t translate(size_t va){
    if (ptbr == 0) 
        return ~0;
    
    size_t *table_entry = (size_t *)ptbr;

    // This is because each page table entry is 8 bytes 
    // entries = 2^POBITS/8 btyes = 2^Pobits
    // Num of entries each page table = 2^index_bits
    int index_bits = POBITS - 3;     
    size_t vp_index = (va >> POBITS);
    
    int current_level = 0;
    size_t level_index_mask = (size_t) ~0 >> (64 - index_bits);
    while (current_level < LEVELS) {
        size_t currvp_index = (vp_index >> (((LEVELS  - current_level) - 1) * index_bits));

        currvp_index &= level_index_mask;
        //Think this is wrong: size_t currvp_index = (vp_index >> (index_bits * current_level)) & level_index_mask;
        //Check that the address is allocated
        if (table_entry[currvp_index] == 0)
            return (size_t) ~0;

        // Check that valid bit is set
        if (!(table_entry[currvp_index] & 1))
            return (size_t) ~0;

        table_entry = (size_t *)(table_entry[currvp_index] -0x1);
        current_level += 1;    
    }

    size_t offset_mask = ~0;
    //Results all 0 followed by POBITS number of 1
    offset_mask = offset_mask >> (64 - POBITS);
    size_t ppn = ((size_t)table_entry & ~offset_mask);
    size_t vp_offset = va & offset_mask; 
    return ppn | vp_offset;
}

