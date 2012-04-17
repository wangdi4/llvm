#include "KNFStencil.h"

template<class T>
KNFStencil<T>::KNFStencil( T _wCenter,
                    T _wCardinal,
                    T _wDiagonal,
                    int _device )
  : Stencil<T>( _wCenter, _wCardinal, _wDiagonal ),
    device( _device )
{
    // nothing else to do
}

template<class T>
void
KNFStencil<T>::DoPreIterationWork( T* currBuf, // in device global memory
                                    T* altBuf,  // in device global memory
                                    Matrix2D<T>& mtx,
                                    unsigned int iter )
{
    // in single-process version, nothing for us to do
}

