#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <vector>

#define DEBUG 0

#define FIRST 1
#define SECOND 2
#define THIRD 3

#define NUM_ARGS 4

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
        cout << "Usage: vmsim <reference range> <# references> <output filename>." << endl;
        return 1;
    }
    int range, numTotal;
    string tempStr;
    // Parse range 
    tempStr = argv[FIRST];
    stringstream convert1(tempStr);
    convert1 >> range;
    // Parse numTotal 
    tempStr = argv[SECOND];
    stringstream convert2(tempStr);
    convert2 >> numTotal;
    // Parse outFileName 
    tempStr = argv[THIRD];
    string outFileName(tempStr);
    if (DEBUG)
    {
        cout << "outFileName: " << outFileName << endl;
    }
    // Generate page references
    vector<int> pageRefs;
    for (int i=0; i<numTotal; i++) 
    {
        int tmpRef = rand() % range;
        pageRefs.push_back(tmpRef);
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


