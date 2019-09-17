/***************************************************************************
 * *    Inf2C-CS Coursework 2: Cache Simulation
 * *
 * *    Instructor: Boris Grot
 * *
 * *    TA: Siavash Katebzadeh
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
/* Do not add any more header files */

/*
 * Various structures
 */
typedef enum
{
    FIFO,
    LRU,
    Random
} replacement_p;

const char *get_replacement_policy(uint32_t p)
{
    switch (p)
    {
    case FIFO:
        return "FIFO";
    case LRU:
        return "LRU";
    case Random:
        return "Random";
    default:
        assert(0);
        return "";
    };
    return "";
}

typedef struct
{
    uint32_t address;
} mem_access_t;

// These are statistics for the cache and should be maintained by you.
typedef struct
{
    uint32_t cache_hits;
    uint32_t cache_misses;
} result_t;

/*
 * Parameters for the cache that will be populated by the provided code skeleton.
 */

replacement_p replacement_policy = FIFO;
uint32_t associativity = 0;
uint32_t number_of_cache_blocks = 0;
uint32_t cache_block_size = 0;

/*
 * Each of the variables below must be populated by you.
 */
uint32_t g_num_cache_tag_bits = 0;
uint32_t g_cache_offset_bits = 0;
result_t g_result;

/* Reads a memory access from the trace file and returns
 * 32-bit physical memory address
 */
mem_access_t read_transaction(FILE *ptr_file)
{
    char buf[1002];
    char *token = NULL;
    char *string = buf;
    mem_access_t access;

    if (fgets(buf, 1000, ptr_file) != NULL)
    {
        /* Get the address */
        token = strsep(&string, " \n");
        access.address = (uint32_t)strtoul(token, NULL, 16);
        return access;
    }

    /* If there are no more entries in the file return an address 0 */
    access.address = 0;
    return access;
}

void print_statistics(uint32_t num_cache_tag_bits, uint32_t cache_offset_bits, result_t *r)
{
    /* Do Not Modify This Function */

    uint32_t cache_total_hits = r->cache_hits;
    uint32_t cache_total_misses = r->cache_misses;
    printf("CacheTagBits:%u\n", num_cache_tag_bits);
    printf("CacheOffsetBits:%u\n", cache_offset_bits);
    printf("Cache:hits:%u\n", r->cache_hits);
    printf("Cache:misses:%u\n", r->cache_misses);
    printf("Cache:hit-rate:%2.1f%%\n", cache_total_hits / (float)(cache_total_hits + cache_total_misses) * 100.0);
}

/*
 *
 * Add any global variables and/or functions here as needed.
 *
 */

typedef struct
{
    uint32_t tag;
    int valid;
} block_t;

typedef struct
{
    int number_of_blocks;
    block_t *block_pointer;
} set_t;

int main(int argc, char **argv)
{
    time_t t;
    /* Intializes random number generator */
    /* Important: *DO NOT* call this function anywhere else. */
    srand((unsigned)time(&t));
    /* ----------------------------------------------------- */
    /* ----------------------------------------------------- */

    /*
     *
     * Read command-line parameters and initialize configuration variables.
     *
     */
    int improper_args = 0;
    char file[10000];
    if (argc < 6)
    {
        improper_args = 1;
        printf("Usage: ./mem_sim [replacement_policy: FIFO LRU Random] [associativity: 1 2 4 8 ...] [number_of_cache_blocks: 16 64 256 1024] [cache_block_size: 32 64] mem_trace.txt\n");
    }
    else
    {
        /* argv[0] is program name, parameters start with argv[1] */
        if (strcmp(argv[1], "FIFO") == 0)
        {
            replacement_policy = FIFO;
        }
        else if (strcmp(argv[1], "LRU") == 0)
        {
            replacement_policy = LRU;
        }
        else if (strcmp(argv[1], "Random") == 0)
        {
            replacement_policy = Random;
        }
        else
        {
            improper_args = 1;
            printf("Usage: ./mem_sim [replacement_policy: FIFO LRU Random] [associativity: 1 2 4 8 ...] [number_of_cache_blocks: 16 64 256 1024] [cache_block_size: 32 64] mem_trace.txt\n");
        }
        associativity = atoi(argv[2]);
        number_of_cache_blocks = atoi(argv[3]);
        cache_block_size = atoi(argv[4]);
        strcpy(file, argv[5]);
    }
    if (improper_args)
    {
        exit(-1);
    }
    assert(number_of_cache_blocks == 16 || number_of_cache_blocks == 64 || number_of_cache_blocks == 256 || number_of_cache_blocks == 1024);
    assert(cache_block_size == 32 || cache_block_size == 64);
    assert(number_of_cache_blocks >= associativity);
    assert(associativity >= 1);

    printf("input:trace_file: %s\n", file);
    printf("input:replacement_policy: %s\n", get_replacement_policy(replacement_policy));
    printf("input:associativity: %u\n", associativity);
    printf("input:number_of_cache_blocks: %u\n", number_of_cache_blocks);
    printf("input:cache_block_size: %u\n", cache_block_size);
    printf("\n");

    /* Open the file mem_trace.txt to read memory accesses */
    FILE *ptr_file;
    ptr_file = fopen(file, "r");
    if (!ptr_file)
    {
        printf("Unable to open the trace file: %s\n", file);
        exit(-1);
    }

    /* result structure is initialized for you. */
    memset(&g_result, 0, sizeof(result_t));

    /* Do not delete any of the lines below.
     * Use the following snippet and add your code to finish the task. */

    /* You may want to setup your Cache structure here. */

    //calculating and setting tag bits and offset bits
    g_cache_offset_bits = log2(cache_block_size);

    int number_of_sets = number_of_cache_blocks / associativity;
    int blocks_in_set = number_of_cache_blocks / number_of_sets;
    int set_index_total = log2(number_of_sets);

    g_num_cache_tag_bits = 32 - g_cache_offset_bits - set_index_total;

    //cache is an array of set pointers, each with pointers to the blocks in each set
    set_t *cache = malloc(sizeof(set_t) * number_of_sets);
    for (int i = 0; i < number_of_sets; i++)
    {
        set_t *current_set = cache + i;
        current_set->block_pointer = malloc(sizeof(block_t) * blocks_in_set);
        for (int j = 0; j < blocks_in_set; j++)
        {
            block_t *current_block = (current_set->block_pointer) + j;
            current_block->valid = 0;
        }
    }

    //getting the bitmask for the address
    int bitmask = pow(2, set_index_total) - 1;

    mem_access_t access;
    /* Loop until the whole trace file has been read. */
    while (1)
    {
        access = read_transaction(ptr_file);
        // If no transactions left, break out of loop.
        if (access.address == 0)
            break;

        /* Add your code here */

        //get correct bit mask
        uint32_t index_address = (access.address >> g_cache_offset_bits) & bitmask;
        uint32_t access_tag = (access.address >> (g_cache_offset_bits + set_index_total));
        set_t *current_set = cache + index_address; //pointer to current set

        //random replacement policy
        if (replacement_policy == Random)
        {
            int block_set = 0; //boolean for if the block has been set in the cache

            //checks if the block is already in the cache
            for (int i = 0; i < blocks_in_set; i++)
            {
                block_t *current_block = (current_set->block_pointer) + i;
                uint32_t tag = (current_block->tag);
                if (current_block->valid == 1 && tag == access_tag)
                {
                    g_result.cache_hits++;
                    block_set = 1;
                    break;
                }
            }

            //checks if there is a block that has not yet been set
            if (block_set == 0)
            {
                g_result.cache_misses++;

                for (int i = 0; i < blocks_in_set; i++)
                {
                    block_t *current_block = (current_set->block_pointer) + i;
                    if (current_block->valid == 0)
                    {
                        current_block->valid = 1;
                        current_block->tag = access_tag;
                        block_set = 1;
                        break;
                    }
                }
                //if there is no empty block, remove one randomly and set the new block
                if (block_set == 0)
                {
                    int rand_block = rand() % blocks_in_set;
                    block_t *random_block = (current_set->block_pointer) + rand_block;
                    random_block->tag = access_tag;
                    random_block->valid = 1;
                }
            }
        }

        //FIFO replacement
        else if (replacement_policy == FIFO)
        {
            int block_set = 0;
            //hit
            for (int i = 0; i < blocks_in_set; i++)
            {
                block_t *current_block = (current_set->block_pointer) + i;
                uint32_t tag = (current_block->tag);
                if (current_block->valid == 1 && tag == access_tag)
                {
                    g_result.cache_hits++;
                    block_set = 1;
                    break;
                }
            }

            if (block_set == 0)
            {
                g_result.cache_misses++;

                //checks if there is a free block
                for (int i = 0; i < blocks_in_set; i++)
                {
                    block_t *current_block = (current_set->block_pointer) + i;
                    if ((current_block->valid) == 0)
                    {
                        current_block->valid = 1;
                        current_block->tag = access_tag;
                        block_set = 1;
                        break;
                    }
                }

                //reordering cache so that the first block in this set of the cache is evicted
                if (block_set == 0)
                {
                    for (int i = 0; i < blocks_in_set; i++)
                    {
                        block_t *current_block = (current_set->block_pointer) + i;
                        block_t *next_block = (current_set->block_pointer) + (i + 1);
                        (current_block->tag) = (next_block->tag);
                    }
                    block_t *lastblock = (current_set->block_pointer) + (blocks_in_set - 1);
                    lastblock->tag = access_tag;
                }
            }
        }

        // LRU replacement
        else if (replacement_policy == LRU)
        {
            int block_set = 0;
            int index = 0;
            //hit checking
            for (int i = 0; i < blocks_in_set; i++)

            {
                block_t *current_block = (current_set->block_pointer) + i;
                uint32_t tag = (current_block->tag);

                if (current_block->valid == 1 && tag == access_tag)
                {
                    g_result.cache_hits++;
                    block_set = 1;
                    index = i;

                    break;
                }
            }

            if (block_set == 1)
            {
                block_t *lastblock = (current_set->block_pointer) + (blocks_in_set - 1);
                int last_valid_index = blocks_in_set;
                if ((lastblock->valid) == 0)
                {
                    block_t *current_block = (current_set->block_pointer);
                    for (int i = 0; i < blocks_in_set; i++)
                    {
                        if ((current_block + i)->valid == 0)
                        {
                            last_valid_index = i;
                            break;
                        }
                    }
                }

                for (int i = last_valid_index - 1; i > index; i--)
                {
                    block_t *current_block = (current_set->block_pointer) + i;
                    block_t *prev_block = (current_set->block_pointer) + (i - 1);
                    (prev_block->tag) = (current_block->tag);
                }

                lastblock->tag = access_tag;
            }

            //miss
            else if (block_set == 0)
            {
                g_result.cache_misses++;

                //checks if there is a free block
                //if this runs fully, no free space is left in this set of the cache
                for (int i = 0; i < blocks_in_set; i++)
                {
                    block_t *current_block = (current_set->block_pointer) + i;
                    if (current_block->valid == 0)
                    {
                        current_block->valid = 1;
                        current_block->tag = access_tag;
                        block_set = 1;
                        break;
                    }
                }

                //since cache is updated for lru every iteration, we can just evict as with fifo
                //when there is a miss and there are no free blocks
                if (block_set == 0)
                {
                    for (int i = 0; i < blocks_in_set - 1; i++)
                    {
                        block_t *current_block = (current_set->block_pointer) + i;
                        block_t *next_block = (current_set->block_pointer) + (i + 1);
                        (current_block->tag) = (next_block->tag);
                    }
                    block_t *lastblock = (current_set->block_pointer) + (blocks_in_set - 1);
                    lastblock->valid = 1;
                    lastblock->tag = access_tag;
                }
            }
        }
    }

    //deallocating memory
    for (int i = 0; i < number_of_sets; i++)
    {
        set_t *current_set = cache + i;
        free(current_set->block_pointer);
    }
    free(cache);

    /* Do not modify code below. */
    /* Make sure that all the parameters are appropriately populated. */
    print_statistics(g_num_cache_tag_bits, g_cache_offset_bits, &g_result);

    /* Close the trace file. */
    fclose(ptr_file);
    return 0;
}
