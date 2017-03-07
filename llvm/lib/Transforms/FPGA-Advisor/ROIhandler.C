
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

using namespace std;

FILE *currentTraceFile = NULL;
bool inRegion = false;
extern "C" {
int ROIprintf(const char *fmt, ...)
{
    if(!inRegion) return 0;
    va_list args;
    va_start(args, fmt);
    vfprintf(currentTraceFile, fmt, args);
    va_end(args);
    return 1;
}
}

void        ROIProgramStart(unsigned int firstBbId, string firstBbDesc)
{
    cerr << "Starting program firstbb: " << firstBbId << " firstbbDesc: " << firstBbDesc << endl;
}

void ROIHandler (unsigned int regionNumber, unsigned int bbId,
    uint64_t bbCount, double weight, string bbDesc, bool isStarting )
{
    if(isStarting)
        cerr << "\t **** Starting ROI " << regionNumber; 
    else
        cerr << "\t **** Ending ROI " << regionNumber; 
    cerr << " bbDesc " << bbDesc;
    cerr << " bbCount " << bbCount;
    cerr << " weight " << weight << endl;
    if(isStarting)
    {
        istringstream iss(bbDesc);
        string fnName;
        if ( !getline(iss, fnName, ':'))
        {
            cerr << " Function name not found in bbDesc." << "\n";
            exit(1);
        }
        stringstream ss;
        ss << "region" << dec << regionNumber << "-" <<  "weight" << weight
           << "-" << bbDesc << ".trace.log";
        string fileName = ss.str();
        currentTraceFile = fopen(fileName.c_str(), "w");
        if ( !currentTraceFile)
        {
            cerr << " Could not create file " << fileName << " for writing\n";
            exit(1);
        }
        fprintf(currentTraceFile, "Entering Function: %s\n", fnName.c_str());
        inRegion = true;
    }
    else
    {
        fclose(currentTraceFile);
        inRegion = false;
        currentTraceFile = NULL;
    }
}
