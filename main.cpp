/*
 * main.cpp
 *
 *  Created on: 17-Sep-2019
 *      Author: prati
 */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "Snooper.h"
#include "CACHE.h"
#include "defination.h"

using namespace std;

int main(int argc, char *argv[])
{
    //Local variable declaration
    FILE *pFileHandler;                 // File handler for input trace file
    char *trace_file;                   // to store file name from command line
    char operation;                     // r or w operation
    unsigned int addr;                  // memory location
    bool retVal = false;
    int operationCount = 0;
    unsigned int proc_id;
    //Snooper class
    Snooper* snooperObj;

    //Expected input pattern
    // ./smp_cache <cache_size>  <assoc> <block_size> <num_processors> <protocol> <trace_file>
    if (argc != 7) // Checks if correct number of inputs have been given.
                   // Throw error and exit if wrong
    {
        cerr << "Error: Expected inputs:6 Given inputs:" << argc - 1 << endl;
        return 1;
    }

    cout << "===== 506 Personal information =====" << endl;
    cout << "Pratik Suresh Raut" << endl;
    cout << "UnityID 200322512" << endl;
    cout << "ECE492 Students? NO" << endl;

    unsigned int cache_size = atoi(argv[1]);
    unsigned int cache_assoc = atoi(argv[2]);
    unsigned int blk_size = atoi(argv[3]);
    unsigned int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
    unsigned int protocol = atoi(argv[5]); /*0:MSI, 1:MESI, 2:Dragon*/
    trace_file = argv[6];

    cout << "===== 506 SMP Simulator configuration =====" << endl;
    cout << "L1_SIZE                   : " << cache_size << endl;
    cout << "L1_ASSOC                  : " << cache_assoc << endl;
    cout << "L1_BLOCKSIZE              : " << blk_size << endl;
    cout << "NUMBER OF PROCESSORS      : " << num_processors << endl;
    cout << "COHERENCE PROTOCOL        : ";
    if (protocol == 0)
        cout << "MSI" << endl;
    else if (protocol == 1)
        cout << "MESI" << endl;
    else
        cout << "Dragon" << endl;
    cout << "TRACE FILE                : " << trace_file << endl;

    //vector to hold n cache objects
    vector<CACHE *> myCacheObj;

    //create snooper obj as per protocol type
    snooperObj = Snooper::get_protocol(protocol);

    for (unsigned int i = 0; i < num_processors; i++)
    {
        CACHE *cacheL1 = new CACHE(blk_size, cache_size, cache_assoc);
        myCacheObj.push_back(cacheL1);
    }

    //open trace fine
    pFileHandler = fopen(trace_file, "r");

    //If FP == NULL then there is issue will opening trace_file
    if (pFileHandler == NULL)
    {
        // Throw error and exit if fopen() failed
        cout << "Error: Unable to open file " << trace_file << endl;
        return 1;
    }

    //operation
    while ((fscanf(pFileHandler, "%d %c %x", &proc_id, &operation, &addr))
            != EOF)
    {
        operationCount++;
#if DEBUG
        cout << "------------------------------------------------" << endl;
        cout << "# " << operationCount << ": ";
#endif
        snooperObj->cache_request(myCacheObj, proc_id, operation,
                num_processors, addr);
    }
    //print
    for (unsigned int i = 0; i < num_processors; i++)
    {
        myCacheObj[i]->printL1Contet(protocol);
    }

    fclose(pFileHandler);
    for (unsigned int i = 0; i < num_processors; i++)
    {
        delete myCacheObj[i];
    }
    return retVal;
}
