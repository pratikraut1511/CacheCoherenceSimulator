/*
 * Snooper.cpp
 *
 *  Created on: 30-Oct-2019
 *      Author: prati
 */

#include "Snooper.h"
#include "CACHE.h"
#include <iostream>
using namespace std;

Snooper *Snooper::get_protocol(int protocol_type)
{
    if (protocol_type == 0)
        return new MSI_protocol;
    else if (protocol_type == 1)
        return new MESI_protocol;
    else
        return new Dragon_protocol;
}

//MSI Protocol functions
MSI_protocol::MSI_protocol()
{
    // cout << "MSI objected created" << endl;
}

MSI_protocol::~MSI_protocol()
{
    cout << "MSI objected destroyed" << endl;
}

BUS_OPERATION MSI_protocol::cache_request(vector<CACHE *> myCacheObj,
        unsigned int proc_id, unsigned int operation, unsigned int num_proc,
        unsigned int address)
{
    BUS_OPERATION busStatus = BUS_NONE;
    if (operation == 'r')
    {
        myCacheObj[proc_id]->findAddress(address);
        myCacheObj.at(proc_id)->readFromAddress();
        //if address is present in cache means status is modified or shared so
        //no need to change status
        if (!(myCacheObj[proc_id]->isFound))
        {//miss
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == MODIFIED)
            {
                myCacheObj[proc_id]->writeBackFromMemory++;
            }
            myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                    SHARED;
            for (unsigned int i = 0; i < num_proc; i++)
            {
                if (i != proc_id)
                {
                    //BUSRD
                    snooper_request(myCacheObj, BUSRD, i, num_proc, address);
                }
            }
        }
        myCacheObj[proc_id]->isFound = false;
    }
    else if (operation == 'w')
    {
        myCacheObj[proc_id]->findAddress(address);
        myCacheObj.at(proc_id)->writeToAddress();
        if (!(myCacheObj[proc_id]->isFound))
        {
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == MODIFIED)
            {
                myCacheObj[proc_id]->writeBackFromMemory++;
            }
            myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                    MODIFIED;
            myCacheObj[proc_id]->BusRdxTrans++;
            for (unsigned int i = 0; i < num_proc; i++)
            {
                if (i != proc_id)
                {
                    //BUSRDX
                    snooper_request(myCacheObj, BUSRDX, i, num_proc, address);
                }
            }
        }
        else
        {
            //if status is modified so no need to change the status
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == SHARED)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        MODIFIED;
                myCacheObj[proc_id]->BusRdxTrans++;
                myCacheObj[proc_id]->memTransactions++;
                for (unsigned int i = 0; i < num_proc; i++)
                {
                    if (i != proc_id)
                    {
                        //BUSRDX
                        snooper_request(myCacheObj, BUSRDX, i, num_proc,
                                address);
                    }
                }

            }
        }
        myCacheObj[proc_id]->isFound = false;
    }
    return busStatus;
}

void MSI_protocol::snooper_request(vector<CACHE *> myCacheObj,
        BUS_OPERATION busOperation, unsigned int proc_id, unsigned int num_proc,
        unsigned int address)
{
    //other cache status updation
    myCacheObj[proc_id]->findAddress(address);
    if ((myCacheObj[proc_id]->isFound))
    {
        if (busOperation == BUSRD)
        {
            //if status is shared then no need to change the status
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == MODIFIED)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        SHARED;
                myCacheObj[proc_id]->intervations++;
                myCacheObj[proc_id]->writeBackFromMemory++;
                myCacheObj[proc_id]->flushes++;
            }
            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == INVALIDATE)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        SHARED;
            }
        }
        else if (busOperation == BUSRDX)
        {
            // if status is invalidate no change of status is required
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == MODIFIED)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        INVALIDATE;
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].valid =
                        0;
                myCacheObj[proc_id]->invalidations++;
                myCacheObj[proc_id]->writeBackFromMemory++;
                myCacheObj[proc_id]->flushes++;
            }
            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == SHARED)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        INVALIDATE;
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].valid =
                        0;
                myCacheObj[proc_id]->invalidations++;
            }
        }
    }
}
//MESI Protocol functions
MESI_protocol::MESI_protocol()
{
    cout << "MESI objected created" << endl;
}

MESI_protocol::~MESI_protocol()
{
    cout << "MESI_protocol objected destroyed" << endl;
}

BUS_OPERATION MESI_protocol::cache_request(vector<CACHE *> myCacheObj,
        unsigned int proc_id, unsigned int operation, unsigned int num_proc,
        unsigned int address)
{
    bool isPresent = false;
    BUS_OPERATION busStatus = BUS_NONE;
    //check for exculsive status
    for (unsigned int i = 0; i < num_proc; i++)
    {
        if (i != proc_id)
        {
            myCacheObj[i]->findAddress(address);
            if (myCacheObj[i]->isFound)
            {
                isPresent = true;
                break;
            }
        }

    }
    if (operation == 'r')
    {
        myCacheObj[proc_id]->findAddress(address);
        if (!isPresent && !(myCacheObj[proc_id]->isFound))
        {        //exclusive
            myCacheObj[proc_id]->readFromAddress();
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == MODIFIED)
            {
                myCacheObj[proc_id]->writeBackFromMemory++;
            }
            myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                    EXCLUSIVE;
        }
        //need to check if required or not
        else if (!isPresent && (myCacheObj[proc_id]->isFound)
                && (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                        == EXCLUSIVE))
        {
            myCacheObj[proc_id]->readFromAddress();
            myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                    EXCLUSIVE;
        }
        else
        {
            //Shared status
            // myCacheObj[proc_id]->findAddress(address);
            myCacheObj.at(proc_id)->readFromAddress();
            if (!(myCacheObj[proc_id]->isFound))
            {
                if (isPresent)
                    myCacheObj.at(proc_id)->cacheToCache++;
                if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                        == MODIFIED)
                {
                    myCacheObj[proc_id]->writeBackFromMemory++;
                }
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        SHARED;
                for (unsigned int i = 0; i < num_proc; i++)
                {
                    if (i != proc_id)
                    {
                        //BUSRD
                        snooper_request(myCacheObj, BUSRD, i, num_proc,
                                address);
                    }
                }
            }
        }
    }
    else
    {
        //write operation
        myCacheObj[proc_id]->findAddress(address);
        myCacheObj.at(proc_id)->writeToAddress();
        if (!(myCacheObj[proc_id]->isFound))
        {        //miss case
            //SHARED case
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == MODIFIED)
            {
                myCacheObj[proc_id]->writeBackFromMemory++;
            }
            myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                    MODIFIED;
            if (isPresent)
                myCacheObj[proc_id]->cacheToCache++;
            myCacheObj[proc_id]->BusRdxTrans++;
            for (unsigned int i = 0; i < num_proc; i++)
            {
                if (i != proc_id)
                {
                    //BUSRDX
                    snooper_request(myCacheObj, BUSRDX, i, num_proc, address);
                }
            }
        }
        else
        {        //hit case
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == SHARED)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        MODIFIED;
//                myCacheObj[proc_id]->BusRdxTrans++;
//                myCacheObj[proc_id]->memTransactions++;
                for (unsigned int i = 0; i < num_proc; i++)
                {
                    if (i != proc_id)
                    {
                        //BUSRUPG
                        snooper_request(myCacheObj, BUSUPG, i, num_proc,
                                address);
                    }
                }
            }
            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == EXCLUSIVE)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        MODIFIED;
                for (unsigned int i = 0; i < num_proc; i++)
                {
                    if (i != proc_id)
                    {
                        //BUSRDX
                        snooper_request(myCacheObj, BUSRDX, i, num_proc,
                                address);
                    }
                }
            }
        }
    }
    return busStatus;
}

void MESI_protocol::snooper_request(vector<CACHE *> myCacheObj,
        BUS_OPERATION busOperation, unsigned int proc_id, unsigned int num_proc,
        unsigned int address)
{
    //other cache status updation
    myCacheObj[proc_id]->findAddress(address);
    if ((myCacheObj[proc_id]->isFound))
    {
        if (busOperation == BUSRD)
        {
            //if status is shared then no need to change the status
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == MODIFIED)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        SHARED;
                myCacheObj[proc_id]->intervations++;
                myCacheObj[proc_id]->writeBackFromMemory++;
                myCacheObj[proc_id]->flushes++;
            }
//            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
//                    == INVALIDATE)
//            {
//                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
//                        SHARED;
//            }
            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == EXCLUSIVE)
            {
                //flush is optional
                //myCacheObj[proc_id]->flushes++;
                myCacheObj[proc_id]->intervations++;
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        SHARED;
            }
        }
        else if (busOperation == BUSRDX)
        {
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == EXCLUSIVE)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        INVALIDATE;
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].valid =
                        0;
                myCacheObj[proc_id]->invalidations++;
            }
            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == MODIFIED)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        INVALIDATE;
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].valid =
                        0;
                myCacheObj[proc_id]->invalidations++;
                myCacheObj[proc_id]->writeBackFromMemory++;
                myCacheObj[proc_id]->flushes++;
            }
        }
        else if (busOperation == BUSUPG)
        {
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == SHARED)
            {
                myCacheObj[proc_id]->invalidations++;
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        INVALIDATE;
                //or valid field can be used
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].valid =
                        0;
            }
        }
    }
}

//Dragon_protocol Protocol functions
Dragon_protocol::Dragon_protocol()
{
    //cout << "Dragon_protocol objected created" << endl;
}

Dragon_protocol::~Dragon_protocol()
{
    //cout << "Dragon_protocol objected destroyed" << endl;
}

BUS_OPERATION Dragon_protocol::cache_request(vector<CACHE *> myCacheObj,
        unsigned int proc_id, unsigned int operation, unsigned int num_proc,
        unsigned int address)
{
    BUS_OPERATION busStatus = BUS_NONE;
    bool isPresent = false;
    //check for exculsive status
    for (unsigned int i = 0; i < num_proc; i++)
    {
        if (i != proc_id)
        {
            myCacheObj[i]->findAddress(address);
            if (myCacheObj[i]->isFound)
            {
                isPresent = true;
                break;
            }
        }
    }

    if (operation == 'r')
    {
        //right operation
        myCacheObj[proc_id]->findAddress(address);
        myCacheObj[proc_id]->readFromAddress();
        if (!(myCacheObj[proc_id]->isFound))
        {                //miss case
            if (!isPresent)
            {
                //exclusive
                if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                        == MODIFIED
                        || myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                                == SM)
                {
                    myCacheObj[proc_id]->writeBackFromMemory++;
                }
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        EXCLUSIVE;
                for (unsigned int i = 0; i < num_proc; i++)
                {
                    if (i != proc_id)
                    {
                        //BUSRD
                        snooper_request(myCacheObj, BUSRD, i, num_proc,
                                address);
                    }
                }
            }
            else
            {
                //myCacheObj[proc_id]->cacheToCache++;
                if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                        == MODIFIED
                        || myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                                == SM)
                {
                    myCacheObj[proc_id]->writeBackFromMemory++;
                }
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        SC;
                for (unsigned int i = 0; i < num_proc; i++)
                {
                    if (i != proc_id)
                    {
                        //BUSRD
                        snooper_request(myCacheObj, BUSRD, i, num_proc,
                                address);
                    }
                }
            }
        }
        else
        {
            //hit case
            //keep status as same
        }
    }
    else
    {
        //write operation
        myCacheObj[proc_id]->findAddress(address);
        myCacheObj.at(proc_id)->writeToAddress();
        if (!(myCacheObj[proc_id]->isFound))
        {
            //miss case
            if (!isPresent)
            {
                //M (PrWrMiss/BUSD and is present = false
                if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                        == MODIFIED
                        || myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                                == SM)
                {
                    myCacheObj[proc_id]->writeBackFromMemory++;
                }
                //exclusive to modified
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        MODIFIED;
                for (unsigned int i = 0; i < num_proc; i++)
                {
                    if (i != proc_id)
                    {
                        //BUSRD
                        snooper_request(myCacheObj, BUSRD, i, num_proc,
                                address);
                    }
                }

            }
            else
            {
                //myCacheObj[proc_id]->cacheToCache++;
                //Sm PrWrMiss BusRd and is present = true
                if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                        == MODIFIED
                        || myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                                == SM)
                {
                    myCacheObj[proc_id]->writeBackFromMemory++;
                }
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        SM;
                for (unsigned int i = 0; i < num_proc; i++)
                {
                    if (i != proc_id)
                    {
                        //BUSRD
                        snooper_request(myCacheObj, BUSRD, i, num_proc,
                                address);
                    }
                }
                //BUSUPD
                for (unsigned int i = 0; i < num_proc; i++)
                {
                    if (i != proc_id)
                    {
                        //BUSUPD
                        snooper_request(myCacheObj, BUSUPD, i, num_proc,
                                address);
                    }
                }
            }
        }
        else
        {
            //hit case
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == SC && isPresent == false)
            {
                //SC->Sm PrWr BUSUPG
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        MODIFIED;
                //BUSUPG
                for (unsigned int i = 0; i < num_proc; i++)
                {
                    if (i != proc_id)
                    {
                        //BUSUPD
                        snooper_request(myCacheObj, BUSUPD, i, num_proc,
                                address);
                    }
                }
            }
            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == SC && isPresent == true)
            {
                //SC->Sm PrWr BUSUPG
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        SM;
                //BUSUPG
                for (unsigned int i = 0; i < num_proc; i++)
                {
                    if (i != proc_id)
                    {
                        //BUSUPD
                        snooper_request(myCacheObj, BUSUPD, i, num_proc,
                                address);
                    }
                }
            }
            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == SM && isPresent == false)
            {
                //Sm->M PrWr BUSUPG
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        MODIFIED;
                //BUSUPG
                for (unsigned int i = 0; i < num_proc; i++)
                {
                    if (i != proc_id)
                    {
                        //BUSUPD
                        snooper_request(myCacheObj, BUSUPD, i, num_proc,
                                address);
                    }
                }
            }
            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == SM && isPresent == true)
            {
                for (unsigned int i = 0; i < num_proc; i++)
                {
                    if (i != proc_id)
                    {
                        //BUSUPD
                        snooper_request(myCacheObj, BUSUPD, i, num_proc,
                                address);
                    }
                }

            }
            //M->M no bus
            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == MODIFIED)
            {
                //remian in same status
                //not required
            }
            //E->M no bus
            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == EXCLUSIVE)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        MODIFIED;
            }
        }
    }
    return busStatus;
}

void Dragon_protocol::snooper_request(vector<CACHE *> myCacheObj,
        BUS_OPERATION busOperation, unsigned int proc_id, unsigned int num_proc,
        unsigned int address)
{
    myCacheObj[proc_id]->findAddress(address);
    if ((myCacheObj[proc_id]->isFound))
    {
        if (busOperation == BUSRD)
        {
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == EXCLUSIVE /*&& isPresent == true*/)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        SC;
                myCacheObj[proc_id]->intervations++;
            }
            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == SM)
            {
                myCacheObj[proc_id]->flushes++;
            }
            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == MODIFIED)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        SM;
                myCacheObj[proc_id]->intervations++;
                myCacheObj[proc_id]->flushes++;
            }
        }
        else if (busOperation == BUSUPD)
        {
            if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == SM)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status =
                        SC;
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].tag =
                        myCacheObj[proc_id]->tagValReq;
               //myCacheObj[proc_id]->intervations++;
            }
            else if (myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].status
                    == SC)
            {
                myCacheObj[proc_id]->cache_memory_details[myCacheObj[proc_id]->IndexValReq][myCacheObj[proc_id]->jValIndex].tag =
                        myCacheObj[proc_id]->tagValReq;
            }
        }
    }
}
