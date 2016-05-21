#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <stdio.h>
#include <iomanip>

// Set only one of the below to a non-zero value for debug printout
#define LRU_DEBUG 0
#define FIFO_DEBUG 0
#define CLOCK_DEBUG 0
#define OPT_DEBUG 0

#define FIRST 1
#define SECOND 2
#define THIRD 3

#define NUM_ARGS 4

using namespace std;


void incrementAges(int * allRefs, int currNumFrames);
int findLRU(int * refAges, int numFrames, int currRefVal);
void printMem(int * memory, int numFrames, int refsIdx, int currNumFrames);
int findCLOCK(int * clockAges, int * clockPtr, int numFrames, int currRefVal, int * memory);
void printRefAges(int * refAges, int replaceIdx, int numFrames);
int isInMem(int * memory, int thisRef, int numFrames);
int findOPT(int * memory, int numFrames, vector<int> memoryRefs, int refsIdx, int * timeToNext);

// When passing char arrays as parameters they must be pointers
int main(int argc, char* argv[]) 
{
    //Seed the random number generator
    srand (time(NULL));
    int DEBUG = CLOCK_DEBUG; 
    //Parse CLA
    if (argc != NUM_ARGS)
    {
        cout << "Error: wrong format for command line arguments." << endl;
        cout << "Usage: vmsim <# frames> <input filename> <LRU|CLOCK|OPT|FIFO>." << endl;
        return 1;
    }
    int numFrames;
    string tempStr;
    // Parse maxFrames 
    tempStr = argv[FIRST];
    stringstream convert1(tempStr);
    convert1 >> numFrames;
    // Parse outFileName 
    tempStr = argv[SECOND];
    string inFileName(tempStr);
    // Parse chooseAlgo 
    tempStr = argv[THIRD];
    string chooseAlgo(tempStr);
    if (DEBUG > 0)
    {
        cout << "inFileName: " << inFileName << endl;
    }
    if (numFrames > 100)
    {
        cout << "Error: maximum number of physical memory frame." << endl;
        return 1;
    }
    // Read memory references from inFileName 
    vector<int> memoryRefs;
    string line;
    ifstream inFile(inFileName.c_str());
    getline(inFile, line);
    cout << "line: " << line << endl;
    istringstream iss(line);
    int tmpRef;
    while (iss >> tmpRef)
    {
        memoryRefs.push_back(tmpRef);
    }
    // Read refs into memory
    int refsIdx = 0;
    int currNumFrames = 0;
    int memoryIdx = 0;
    int replaceIdx = 0;
    int refAges[numFrames];
    int fifoPtr = 0;
    int clockHand = 0;
    int * clockPtr = &clockHand;
    int refFIFO[numFrames];
    int clockAges[numFrames];
    int clock[numFrames];
    int timeToNext[numFrames];
    int memory[numFrames];
    int lookupIdx;
    int numFaults = 0;
    int rateDenom;
    bool faultOccured = false;
    //Initialize FIFO, ages, and clock
    for (int i=0; i<numFrames; i++)
    {
        refAges[i] = 0;
        clockAges[i] = 0;
        refFIFO[i] = 0;
        memory[i] = 0;
    }
    // Read all the refs
    while (refsIdx < memoryRefs.size())
    {
        faultOccured = false;
        lookupIdx = isInMem(memory, memoryRefs[refsIdx], numFrames);  
        if (currNumFrames < numFrames)  // Memory not yet filled
        {
            if(DEBUG)
                cout << "Memory not yet filled." << endl;
            // Only add if ref not already in memory
            if(!lookupIdx >=0)
            {
                memory[currNumFrames] = memoryRefs[refsIdx];
            }
            incrementAges(refAges, currNumFrames);
            refAges[currNumFrames] = 0;
            currNumFrames++;
            if (currNumFrames == numFrames)
                rateDenom = memoryRefs.size() - refsIdx; 
            refsIdx++;
            clockAges[*clockPtr]++;
            *clockPtr = (*clockPtr + 1) % numFrames;
            if(DEBUG > 2)
                printRefAges(refFIFO, replaceIdx, numFrames);
            memoryIdx = currNumFrames;
            printMem(memory, numFrames, refsIdx, currNumFrames);
            cout << endl;
        }
        else // Choose a ref to replace
        {
            if (!(chooseAlgo.compare("LRU")) | !(chooseAlgo.compare("LRU")))
            {
                if(DEBUG > 0)
                    cout << "Memory filled, replace LRU." << endl;
                replaceIdx = findLRU(refAges, numFrames, memoryRefs[refsIdx]);
                if(DEBUG > 0)
                    printRefAges(refAges, replaceIdx, numFrames);
                incrementAges(refAges, currNumFrames);
            }
            else if (!(chooseAlgo.compare("FIFO")) | !(chooseAlgo.compare("fifo")))
            {
                if(DEBUG > 0)
                {
                    cout << "Memory filled, replace FIFO." << endl;
                    cout << "Old fifoPtr: " << fifoPtr << endl;
                }
                // Only update FIFO if not in memory
                if (lookupIdx < 0)
                {
                    replaceIdx = fifoPtr;  
                    fifoPtr = (fifoPtr + 1) % numFrames;
                }
                if(DEBUG > 0)
                    cout << "New fifoPtr: " << fifoPtr << endl;
            }
            else if (!(chooseAlgo.compare("OPT")) | !(chooseAlgo.compare("opt")))
            {
                if(DEBUG > 0)
                    cout << "Memory filled, replace OPT." << endl;
                replaceIdx = findOPT(memory, numFrames, memoryRefs, refsIdx, timeToNext);
                if(DEBUG > 0)
                    printRefAges(timeToNext, replaceIdx, numFrames);
            }
            else if (!(chooseAlgo.compare("CLOCK")) | !(chooseAlgo.compare("clock")))
            {
                if(DEBUG > 0)
                    cout << "Memory filled, replace CLOCK." << endl;
                if(lookupIdx == -1)
                {
                    replaceIdx = findCLOCK(clockAges, clockPtr, numFrames, memoryRefs[refsIdx], memory);
                }
                else
                {
                    clockAges[lookupIdx] = 1;
                }
                if(DEBUG > 0)
                    printRefAges(clockAges, replaceIdx, numFrames);
            }
            // Only add if ref not already in memory
            if(lookupIdx == -1)
            {
                memory[replaceIdx] = memoryRefs[refsIdx];
                faultOccured = true;
                numFaults++;
                refAges[replaceIdx] = 0;
            }
            else // Otherwise, only update age 
            {
                refAges[lookupIdx] = 0;
            }
            refsIdx++;
            memoryIdx = replaceIdx;
            printMem(memory, numFrames, refsIdx, currNumFrames);
            if (faultOccured)
                cout << " F" << endl;
            else
                cout << endl;

        }
    }
    double missRate = 100 * ((double)numFaults/(double)rateDenom);
    cout << endl;
    cout << "Miss rate = " << numFaults << "/" << rateDenom << " = " << missRate << "%" << endl; 
    return 0;
}

void incrementAges(int * refAges, int currNumFrames) 
{
    for (int i = 0; i<currNumFrames; i++)
    {
        refAges[i]++;
    }
}

int findCLOCK(int * clockAges, int * clockPtr, int numFrames, int currRefVal, int * memory) 
{
    int currIdx = *clockPtr;
    while (1)
    {
        if (clockAges[currIdx] == 1) 
        {
            clockAges[currIdx] = 0;
            *clockPtr = (*clockPtr + 1) % numFrames;
            currIdx = *clockPtr;
        }
        else if ((clockAges[currIdx] == 0) && (memory[currIdx] == currRefVal))
        {
            clockAges[currIdx] = 1;
            return currIdx;
        }
        else if ((clockAges[currIdx] == 0) && (memory[currIdx] != currRefVal))
        {
            clockAges[currIdx] = 1;
            *clockPtr = (*clockPtr + 1) % numFrames;
            return currIdx;
        }
    }
}

int findOPT(int * memory, int numFrames, vector<int> memoryRefs, int refsIdx, int * timeToNext)
{
    for (int i = 0; i<numFrames; i++)
    {
        int offsetIdx = refsIdx;
        timeToNext[i] = memoryRefs.size();
        while(true)
        {
            //cout << "offsetIdx: " << offsetIdx << " | size: " << memoryRefs.size() << endl;
            if (offsetIdx == memoryRefs.size())
                break;
            else if(memoryRefs[offsetIdx] == memory[i])
            {
                timeToNext[i] = offsetIdx - refsIdx;
                break;
            }
            offsetIdx++;
        }
    }
    int retIdx = 0;
    int maxTime = 0;
    for (int i = 0; i<numFrames; i++)
    {
        if(timeToNext[i] > maxTime)
        {
           maxTime = timeToNext[i];
           retIdx = i;
        }
    }
    return retIdx;
}

int findLRU(int * refAges, int numFrames, int currRefVal) 
{
    int currLRU_idx = 0;
    for (int i = 0; i<numFrames; i++)
    {
        if (refAges[i] >= refAges[currLRU_idx])
        {
            currLRU_idx = i;
        }
    }
    return currLRU_idx;
}

void printMem(int * memory, int numFrames, int refsIdx, int currNumFrames)
{
    int printIdx = 0;
    // Print first ref
    cout << refsIdx << ": [";
    if(memory[printIdx] < 10)
    {
        cout << " " << memory[printIdx];
    }
    else
        cout << memory[printIdx];
    printIdx++;
    //Print remaining refs
    for (int i = printIdx; i<numFrames; i++)
    {
        if (i < currNumFrames) // This location has been accessed before
        {
            if(memory[printIdx] < 10)
            {
                cout << "| " << memory[printIdx];
            }
            else
                cout << memory[printIdx];
        }
        else  // Location never accessed before, print blank.
            cout << "|  ";
        printIdx++;
    }
    cout << "]";
}

void printRefAges(int * refAges, int replaceIdx, int numFrames)
{
    cout << "Idx to replace: " << replaceIdx << endl;
    for(int i=0; i<numFrames; i++)
    {
        cout << "refAges[" << i << "]:" << refAges[i] << " *** ";
    }
    cout << endl;
}

int isInMem(int * memory, int thisRef, int numFrames)
{
    int retVal = -1;
    for(int i=0; i<numFrames; i++)
    {
        if(memory[i] == thisRef)
        {
            retVal = i;
        }
    }
    return retVal;
}
