#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <vector>

#define DEBUG 2

#define FIRST 1
#define SECOND 2
#define THIRD 3
#define FOURTH 4
#define FIFTH 5
#define SIXTH 6

#define NUM_ARGS 7

using namespace std;

// When passing char arrays as parameters they must be pointers
int main(int argc, char* argv[]) 
{
    //Seed the random number generator
    srand (time(NULL));
    //Prase CLA
    if (argc != NUM_ARGS)
    {
        cout << "Error: wrong format for command line arguments." << endl;
        cout << "Usage: vmgenWS <# frames> <lowerbound> <upperbound> <reference range> <# references> <output filename>." << endl;
        return 1;
    }
    int ws_size, lowerBound, upperBound, range, numTotal;
    string tempStr;
    // Parse ws_size
    tempStr = argv[FIRST];
    stringstream convert1(tempStr);
    convert1 >> ws_size;
    // Parse lowerBound 
    tempStr = argv[SECOND];
    stringstream convert2(tempStr);
    convert2 >> lowerBound;
    // Parse upperBound 
    tempStr = argv[THIRD];
    stringstream convert3(tempStr);
    convert3 >> upperBound;
    // Parse range 
    tempStr = argv[FOURTH];
    stringstream convert4(tempStr);
    convert4 >> range;
    // Parse numTotal 
    tempStr = argv[FIFTH];
    stringstream convert5(tempStr);
    convert5 >> numTotal;
    // Parse outFileName 
    tempStr = argv[SIXTH];
    string outFileName(tempStr);
    if (DEBUG)
    {
        cout << "ws_size: " << ws_size << endl;
        cout << "outFileName: " << outFileName << endl;
    }
    // Generate page references
    vector<int> pageRefs;
    vector<int> refSet;
    while (pageRefs.size() < numTotal)
    {
        int num_gen = rand() % upperBound + lowerBound;
        if (DEBUG)
            cout << "num_gen: " << num_gen << endl;
        for (int i=0; i<ws_size; i++)
        {
            int tmpRef = rand() % range;
            refSet.push_back(tmpRef);
            if (DEBUG == 2)
                cout << "Set member " << i << ": " << tmpRef << endl;
        }
        for (int i=0; i<num_gen; i++)
        {
            int idx = rand() % ws_size;
            pageRefs.push_back(refSet[idx]);
            if (pageRefs.size() == numTotal)
            { 
                break;
            }
        }
    } 
    // Write references to output file
    ofstream outFile;
    outFile.open(outFileName.c_str());
    for (int i=0; i<pageRefs.size(); i++)
    {
        outFile << pageRefs[i] << " ";
    }
    // Close the output file
    outFile.close();
    return 0;
}

