/*
 * CACHE.cpp
 *
 *  Created on: 17-Sep-2019
 *      Author: prati
 */

#include "CACHE.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <bits/stdc++.h>
#include "defination.h"

using namespace std;

CACHE::CACHE(unsigned long int blockSizeIn, unsigned long int l1_sizeIn,
        unsigned long int assocIn)
{
    block_size = blockSizeIn;
    assoc = assocIn;
    size = l1_sizeIn;
    num_of_sets = round((size / (assoc * block_size * 1.0)));
    //round ceil for round off of value
    bits_offset = ceil(log2(block_size));
    bits_index = ceil(log2(num_of_sets));
    bits_tag = NUM_OF_BITS - bits_index - bits_offset;
    indexMask = (unsigned int) pow(2, bits_index) - 1;
    blockMask = (unsigned int) pow(2, bits_offset) - 1;
#if DEBUG1
    cout << "----- Cache value-----" << endl;
    cout << "bits_offset   " << bits_offset << " bits_index    " << bits_index
    << endl;
#endif
    cache_memory_details = new cache_info*[num_of_sets];
    for (unsigned int i = 0; i < num_of_sets; i++)
        cache_memory_details[i] = new cache_info[assoc];

    for (unsigned long int i = 0; i < num_of_sets; i++)
    {
        for (unsigned long int j = 0; j < assoc; j++)
        {
            cache_memory_details[i][j].valid = 0;
            cache_memory_details[i][j].tag = 0;
            cache_memory_details[i][j].LRUVal = 0;
            cache_memory_details[i][j].dirty = 0;
            cache_memory_details[i][j].status = INVALIDATE;
        }
    }
    //Initial Values
    reads = 0;
    readMiss = 0;
    write = 0;
    writeMiss = 0;
    writeBackFromMemory = 0;
    missRate = 0;
    totalMemoryTrafic = 0;
    isFound = false;
    tagValReq = 0;
    blockValReq = 0;
    IndexValReq = 0;
    jValIndex = 0;
    invalidations = 0;
    intervations = 0;
    flushes = 0;
    cacheToCache = 0;
    memTransactions = 0;
    BusRdxTrans = 0;
}

CACHE::~CACHE()
{
    //free memory
    for (unsigned int i = 0; i < num_of_sets; i++)
    {
        delete[] cache_memory_details[i];
    }
    delete[] cache_memory_details;
}

void CACHE::findAddress(unsigned int addr)
{
    isFound = false;
    tagValReq = (unsigned int) addr >> (bits_index + bits_offset);
    blockValReq = (unsigned int) addr >> (bits_offset);
    IndexValReq = (unsigned int) (blockValReq & indexMask);

    //Find index in cache
    for (unsigned int j = 0; j < assoc; j++)
    {
        if (cache_memory_details[IndexValReq][j].valid == 1)
        {
            if (cache_memory_details[IndexValReq][j].tag == tagValReq)
            {
                isFound = true;
                jValIndex = j;
                break;
            }
        }
    }
}

bool CACHE::readFromAddress()
{
    unsigned int oldLURVal = 0;
    unsigned int refLURVal = 0;
    unsigned int LRUIndex = 0;
    bool isInvalid = false;

#if DEBUG
    cout << " read " << hex << addr << endl;
    cout << "L1 read: " << hex << addr << " (tag " << hex << tagValReq << dec
    << ", index " << IndexValReq << ")" << endl;
#endif
    //increment L1 Reads value
    reads++;

    //Update LRU if isFound == true
    if (isFound)
    {
        oldLURVal = cache_memory_details[IndexValReq][jValIndex].LRUVal;
        cache_memory_details[IndexValReq][jValIndex].LRUVal = 0;
        cache_memory_details[IndexValReq][jValIndex].dirty = 0;
#if DEBUG
        cout << "L1 hit" << endl;
        cout << "L1 update LRU" << endl;
#endif
        for (unsigned int j = 0; j < assoc; j++)
        {
            if (jValIndex != j
                    && cache_memory_details[IndexValReq][j].LRUVal < oldLURVal)
            {
                cache_memory_details[IndexValReq][j].LRUVal++;
            }
        }
    }
    else
    {
        readMiss++;
#if DEBUG
        cout << "L1 miss" << endl;
#endif
        //check for any invalid field
        for (unsigned int j = 0; j < assoc; j++)
        {
            if (cache_memory_details[IndexValReq][j].valid == 0)
            {
                isInvalid = true;
                jValIndex = j;
                break;
            }
        }
        if (isInvalid)
        {
            //update tag value
            cache_memory_details[IndexValReq][jValIndex].tag = tagValReq;
            cache_memory_details[IndexValReq][jValIndex].valid = 1;
            cache_memory_details[IndexValReq][jValIndex].LRUVal = 0;
            cache_memory_details[IndexValReq][jValIndex].dirty = 0;
            //Update LRU value
            for (unsigned int j = 0; j < assoc; j++)
            {
                if (jValIndex != j)
                {
                    cache_memory_details[IndexValReq][j].LRUVal++;
                }
            }
#if DEBUG
            cout << "L1 update LRU" << endl;
#endif
        }
        else
        {
            //Remove the Least freq used tag value and replace it with new tag value
            for (unsigned int j = 0; j < assoc; j++)
            {
                if (cache_memory_details[IndexValReq][j].LRUVal > refLURVal)
                {
                    refLURVal = cache_memory_details[IndexValReq][j].LRUVal;
                    LRUIndex = j;
                }
            }
            //update tag value

            cache_memory_details[IndexValReq][LRUIndex].tag = tagValReq;
            cache_memory_details[IndexValReq][LRUIndex].valid = 1;
            cache_memory_details[IndexValReq][LRUIndex].LRUVal = 0;
            if (cache_memory_details[IndexValReq][LRUIndex].dirty == 1)
            {
                cache_memory_details[IndexValReq][LRUIndex].dirty = 0;
               // writeBackFromMemory++;
            }
            for (unsigned int j = 0; j < assoc; j++)
            {
                if (j != LRUIndex)
                {
                    cache_memory_details[IndexValReq][j].LRUVal++;
                }
            }
            jValIndex = LRUIndex;
#if DEBUG

            cout << "L1 update LRU   " << refLURVal << endl;
            //cout << "out returning to main" << endl;
#endif
        }
    }
    return true;
}

bool CACHE::writeToAddress()
{
    unsigned int oldLURVal = 0;
    unsigned int refLURVal = 0;
    unsigned int LRUIndex = 0;
    bool isInvalid = false;

#if DEBUG
    cout << "write " << hex << addr << endl;
    cout << "L1 write: " << addr << " (tag " << tagValReq << dec << ", index "
    << IndexValReq << ")" << endl;
#endif
    write++;

    //Update LRU if isFound == true
    if (isFound)
    {
        oldLURVal = cache_memory_details[IndexValReq][jValIndex].LRUVal;
        cache_memory_details[IndexValReq][jValIndex].LRUVal = 0;
        cache_memory_details[IndexValReq][jValIndex].dirty = 1;
        for (unsigned int j = 0; j < assoc; j++)
        {
            if (jValIndex != j
                    && cache_memory_details[IndexValReq][j].LRUVal < oldLURVal)
            {
                cache_memory_details[IndexValReq][j].LRUVal++;
            }
        }
#if DEBUG
        cout << "L1 hit" << endl;
        cout << "L1 set dirty" << endl;
        cout << "L1 update LRU" << endl;
#endif
    }
    else
    {
        writeMiss++;
#if DEBUG
        cout << "L1 miss" << endl;
#endif
        //check for any invalid field
        for (unsigned int j = 0; j < assoc; j++)
        {
            if (cache_memory_details[IndexValReq][j].valid == 0)
            {
                isInvalid = true;
                jValIndex = j;
                break;
            }
        }
        if (isInvalid)
        {
            //update tag value
            cache_memory_details[IndexValReq][jValIndex].tag = tagValReq;
            cache_memory_details[IndexValReq][jValIndex].valid = 1;
            cache_memory_details[IndexValReq][jValIndex].LRUVal = 0;
            cache_memory_details[IndexValReq][jValIndex].dirty = 1;
#if DEBUG
            cout << "L1 set dirty" << endl;
#endif
            //Update LRU value
            for (unsigned int j = 0; j < assoc; j++)
            {
                if (jValIndex != j)
                {
                    cache_memory_details[IndexValReq][j].LRUVal++;
                }
            }
#if DEBUG
            cout << "L1 update LRU   " << endl;
#endif
        }
        else
        {
            //Remove the Least freq used tag value and replace it with new tag value
            for (unsigned int j = 0; j < assoc; j++)
            {
                if (cache_memory_details[IndexValReq][j].LRUVal > refLURVal)
                {
                    refLURVal = cache_memory_details[IndexValReq][j].LRUVal;
                    LRUIndex = j;
                }
            }
            //update tag value
            cache_memory_details[IndexValReq][LRUIndex].tag = tagValReq;
            cache_memory_details[IndexValReq][LRUIndex].valid = 1;
            cache_memory_details[IndexValReq][LRUIndex].LRUVal = 0;
            if (cache_memory_details[IndexValReq][LRUIndex].dirty == 1)
            {
               // writeBackFromMemory++;
            }
            cache_memory_details[IndexValReq][LRUIndex].dirty = 1;
#if DEBUG
            cout << "L1 set dirty" << endl;
#endif
            for (unsigned int j = 0; j < assoc; j++)
            {
                if (j != LRUIndex)
                {
                    cache_memory_details[IndexValReq][j].LRUVal++;
                }
            }
            jValIndex = LRUIndex;
#if DEBUG
            cout << "L1 update LRU   " << refLURVal << endl;
#endif
        }
    }
    return true;
}

void CACHE::swap(cache_info *val1, cache_info *val2)
{
    cache_info temp;
    temp.LRUVal = val1->LRUVal;
    temp.tag = val1->tag;
    temp.dirty = val1->dirty;

    val1->LRUVal = val2->LRUVal;
    val1->tag = val2->tag;
    val1->dirty = val2->dirty;

    val2->LRUVal = temp.LRUVal;
    val2->tag = temp.tag;
    val2->dirty = temp.dirty;
}

bool CACHE::sortVal(cache_info *val)
{
    cache_info swappedVals[assoc];
    for (unsigned int i = 0; i < assoc; i++)
    {
        swappedVals[i].LRUVal = val[i].LRUVal;
        swappedVals[i].tag = val[i].tag;
        swappedVals[i].dirty = val[i].dirty;
    }
    for (unsigned int i = 0; i < assoc - 1; i++)
    {
        for (unsigned int j = 0; j < assoc - i - 1; j++)
        {
            if (swappedVals[j].LRUVal > swappedVals[j + 1].LRUVal)
            {
                swap(&swappedVals[j], &swappedVals[j + 1]);
            }
        }
    }

    for (unsigned int i = 0; i < assoc; i++)
    {
        cout << hex << swappedVals[i].tag;
        swappedVals[i].dirty == 1 ? cout << " D\t ||" : cout << " N\t ||";
    }
    cout << endl;
    return true;
}

void CACHE::printL1Contet(unsigned int protocol)
{
    float rate;
    static int x = 0;
    cout << "\n=====cache " << x << " contents=====\n";

    for (unsigned int i = 0; i < num_of_sets; i++)
    {
        cout << "Set  " << dec << i << ": " << "\t\t";
        for (unsigned int j = 0; j < assoc; j++)
        {
            cout << hex << "  " << cache_memory_details[i][j].tag;
            cout << " " << cache_memory_details[i][j].valid;
            cout << dec << " " << cache_memory_details[i][j].LRUVal;
            cout << " " << cache_memory_details[i][j].dirty;
            if (cache_memory_details[i][j].status == INVALIDATE)
                cout << " " << "I" << "\t";
            else if (cache_memory_details[i][j].status == SHARED)
                cout << " " << "S" << "\t";
            else if (cache_memory_details[i][j].status == MODIFIED)
                cout << " " << "M" << "\t";
            else if(cache_memory_details[i][j].status == EXCLUSIVE)
                cout << " " << "E" << "\t";
            else if(cache_memory_details[i][j].status == SC)
                cout << " " << "Sc" << "\t";
            else if(cache_memory_details[i][j].status == SM)
                cout << " " << "Sm" << "\t";

        }
        cout << endl;
    }
//    for (unsigned int i = 0; i < num_of_sets; i++)
//    {
//        cout << "set\t" << dec << i << ":\t";
//        sortVal(cache_memory_details[i]);
//    }
    float miss = readMiss + writeMiss;
    float request = (write + reads);
    rate = (miss / request) * 100;
    //cout << endl;
    cout << "============ Simulation results (Cache " << x++ << ") ============"
            << endl;
    cout << "01. number of reads:                   " << dec << reads << endl;
    cout << "02. number of read misses:             " << dec << readMiss
            << endl;
    cout << "03. number of writes:                  " << dec << write << endl;
    cout << "04. number of write misses:            " << dec << writeMiss
            << endl;
    cout << "05. total miss rate:                   ";
    printf("%.2f%%\n", rate);
    cout << "06. number of writebacks:              " << dec
            << writeBackFromMemory << endl;
    cout << "07. number of cache-to-cache transfers:" << cacheToCache << endl;
    cout << "08. number of memory transactions:     " ;
    if(protocol == 0)
           cout << dec << (readMiss + writeMiss + writeBackFromMemory + memTransactions) << endl;
    else if(protocol == 1)
        cout << dec << (readMiss + writeMiss + writeBackFromMemory - cacheToCache) << endl;
    else
        cout << dec << (readMiss + writeMiss + writeBackFromMemory) << endl;
    cout << "09. number of interventions:           " << intervations << endl;
    cout << "10. number of invalidations:           " << invalidations << endl;
    cout << "11. number of flushes:                 " << flushes << endl;
    cout << "12. number of BusRdX:                  " << BusRdxTrans << endl;
}
