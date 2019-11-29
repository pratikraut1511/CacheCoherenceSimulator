/*
 * Snooper.h
 *
 *  Created on: 30-Oct-2019
 *      Author: prati
 */

#ifndef SNOOPER_H_
#define SNOOPER_H_
#include <vector>
#include "CACHE.h"

using namespace std;




typedef enum BUS_OPERATION_T
{
    BUSRD = 0, BUSRDX, BUSUPG, BUSUPD, BUS_NONE
} BUS_OPERATION;

class Snooper
{
public:
    //virtual ~Snooper();
    // Factory Method
    static Snooper *get_protocol(int protocol_type);
    virtual BUS_OPERATION cache_request(vector<CACHE *> myCacheObj,
            unsigned int proc_num, unsigned int operation,
            unsigned int num_proc, unsigned int address) = 0;
    virtual void snooper_request(vector<CACHE *> myCacheObj,
            BUS_OPERATION busOperation, unsigned int proc_num,
            unsigned int num_proc, unsigned int address) = 0;
};

class MSI_protocol: public Snooper
{
public:
    MSI_protocol();
    virtual ~MSI_protocol();
    BUS_OPERATION cache_request(vector<CACHE *> myCacheObj,
            unsigned int proc_num, unsigned int operation,
            unsigned int num_proc, unsigned int address);
    void snooper_request(vector<CACHE *> myCacheObj, BUS_OPERATION busOperation,
            unsigned int proc_num, unsigned int num_proc, unsigned int address);
};

class MESI_protocol: public Snooper
{
public:
    MESI_protocol();
    virtual ~MESI_protocol();
    BUS_OPERATION cache_request(vector<CACHE *> myCacheObj,
            unsigned int proc_num, unsigned int operation,
            unsigned int num_proc, unsigned int address);
    void snooper_request(vector<CACHE *> myCacheObj, BUS_OPERATION busOperation,
            unsigned int proc_num, unsigned int num_proc, unsigned int address);
};

class Dragon_protocol: public Snooper
{
public:
    Dragon_protocol();
    virtual ~Dragon_protocol();
    BUS_OPERATION cache_request(vector<CACHE *> myCacheObj,
            unsigned int proc_num, unsigned int operation,
            unsigned int num_proc, unsigned int address);
    void snooper_request(vector<CACHE *> myCacheObj, BUS_OPERATION busOperation,
            unsigned int proc_num, unsigned int num_proc, unsigned int address);
};
#endif /* SNOOPER_H_ */
