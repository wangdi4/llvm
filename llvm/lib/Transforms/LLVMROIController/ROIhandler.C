#include <iostream>
using namespace std;
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
    cerr << " bbId " << bbId; 
    cerr << " bbCount " << bbCount;
    cerr << " weight " << weight;
    cerr << " bbDesc " << bbDesc << endl;
}
