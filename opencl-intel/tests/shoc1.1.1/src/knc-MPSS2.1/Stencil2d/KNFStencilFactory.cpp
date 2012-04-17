#include <iostream>
#include <string>
#include <cassert>
#include "KNFStencilFactory.h"
#include "KNFStencil.h"


template<class T>
Stencil<T>*
KNFStencilFactory<T>::BuildStencil( const OptionParser& options )
{
    // get options for base class
    T wCenter;
    T wCardinal;
    T wDiagonal;
    std::vector<long long int> devs;
    ExtractOptions( options,
                    wCenter,
                    wCardinal,
                    wDiagonal,
                    devs );

    // determine whcih device to use
    // We would really prefer this to be done in main() but
    // since BuildStencil is a virtual function, we cannot change its 
    // signature, and OptionParser provides no way to override an
    // options' value after it is set during parsing.
    int chosenDevice = 0;//(int)devs[0];

    return new KNFStencil<T>( wCenter, 
                                wCardinal, 
                                wDiagonal, 
                                chosenDevice );
}


