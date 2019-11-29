/*
 * CACHE.h
 *
 *  Created on: 17-Sep-2019
 *      Author: prati
 */

#ifndef CACHE_H_
#define CACHE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "defination.h"

using namespace std;

#define NUM_OF_BITS 32
//MSI, MESI, Dragon protocol States
typedef enum
{
    MODIFIED,
    EXCLUSIVE,
    SHARED,
    INVALIDATE,
    SM,
    SC,
}PROTOCOL_STATE;

//structure to hold tag, valid and LRU
typedef struct
{
    unsigned int tag;
    unsigned int valid;
    unsigned int LRUVal;
    unsigned int dirty;
    unsigned int status;
} cache_info;

//CACHE Class
class CACHE
{
public:
    CACHE(unsigned long int blockSizeIn, unsigned long int l1_sizeIn,
            unsigned long int l1_assocIn);
    CACHE(unsigned long int sizeIn, unsigned long int assocIn,
            unsigned long int dataBlock, unsigned long int addrTag);
    virtual ~CACHE();

    //friend class declaration
    friend class MSI_protocol;
    friend class MESI_protocol;
    friend class Dragon_protocol;

    void findAddress(unsigned int addr);
    bool readFromAddress();
    bool writeToAddress();
    void printL1Contet(unsigned int protocol);
    void swap(cache_info *val1, cache_info *val2);
    bool sortVal(cache_info *val);
    unsigned long int getBlock_size()
    {
        return block_size;
    }
    unsigned long int get_assoc()
    {
        return assoc;
    }
    unsigned long int get_size()
    {
        return size;
    }

private:
    //Common params
    unsigned long int size;
    unsigned long int assoc;
    //params for L1 cache
    unsigned long int block_size;
    unsigned long int num_of_sets;
    //Bits related to params
    unsigned int bits_index;
    unsigned int bits_offset;
    unsigned int bits_tag;
    unsigned int indexMask;
    unsigned int blockMask;
    //2d dynamic aaray
    cache_info **cache_memory_details;

    //hit/miss related variables
    unsigned int reads;
    unsigned int readMiss;
    unsigned int write;
    unsigned int writeMiss;
    unsigned int writeBackFromMemory;
    unsigned int missRate;
    unsigned int totalMemoryTrafic;
    //coherence variable
    unsigned int invalidations;
    unsigned int intervations;
    unsigned int flushes;
    unsigned int cacheToCache;
    unsigned int BusRdxTrans;
    unsigned int memTransactions;
    //flags
    bool isFound = false;
    //computational variables
    unsigned int tagValReq;
    unsigned int blockValReq;
    unsigned int IndexValReq;
    unsigned int jValIndex;
};

#endif /* CACHE_H_ */
