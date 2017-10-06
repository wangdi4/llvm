
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

using namespace std;

FILE *currentTraceFile = NULL;
bool inRegion = false;
int regionNumber = 0;
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

// Used for manually marking ROIs in application code.
void ROIMarkerHandler (string functionName, bool isStarting )
{
    if(isStarting)
    {
        if(!inRegion)
            cerr << "\t **** Starting ROI " << regionNumber;
        else
        {
            cerr << "\t **** Already in an ROI \n\t Ignoring ROI Start marker " << regionNumber;
            return;
        }
    }
    else
    {
        if(inRegion)
            cerr << "\t **** Ending ROI " << regionNumber;
        else
        {
            cerr << "\t **** Not in an ROI \n\t Ignoring ROI End marker " << regionNumber;
            return;
        }
    }
    if(isStarting)
    {
        regionNumber++;
        stringstream ss;
        ss << "region" << dec << regionNumber << "-"
           << "-" << functionName.c_str() << ".trace.log";
        string fileName = ss.str();
        currentTraceFile = fopen(fileName.c_str(), "w");
        if ( !currentTraceFile)
        {
            cerr << " Could not create file " << fileName << " for writing\n";
            exit(1);
        }
        fprintf(currentTraceFile, "Entering Function: %s\n", functionName.c_str());
        inRegion = true;
    }
    else
    {
        fclose(currentTraceFile);
        inRegion = false;
        currentTraceFile = NULL;
    }
}

/*
void ROIMarkerHandler (string fnName, bool isStarting )
{
    if(isStarting)
    {
        if(inRegion){
          cerr << "\t **** Already in region " << regionNumber << ". IGNORING START\n";
        }
        else {
          regionNumber++;
          inRegion = true;
          cerr << "\t **** Starting ROI " << regionNumber; 
        }
    }
    else
    {
        if(!inRegion){
          cerr << "\t **** NOT in a region. IGNORING STOP\n";
        }
        else {
          cerr << "\t **** Ending ROI " << regionNumber; 
          inRegion = false;
        }
    }
}
*/
