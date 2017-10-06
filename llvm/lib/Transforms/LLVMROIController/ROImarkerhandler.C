#include <iostream>
using namespace std;
static int regionNumber = 0;
static bool inRegion = false;
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
