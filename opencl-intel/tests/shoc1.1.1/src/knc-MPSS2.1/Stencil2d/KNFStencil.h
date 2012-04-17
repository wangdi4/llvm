#ifndef KNFSTENCIL_H
#define KNFSTENCIL_H

#include "Stencil.h"

// ****************************************************************************
// Class:  KNFStencil
//
// Purpose:
//   KNF implementation of 9-point stencil.
//
// Programmer:  Phil Roth
// Creation:    October 28, 2009
//
// ****************************************************************************
template<class T>
class KNFStencil : public Stencil<T>
{
private:
    int device;

protected:
    virtual void DoPreIterationWork( T* currBuf,    // in device global memory
                                        T* altBuf,  // in device global memory
                                        Matrix2D<T>& mtx,
                                        unsigned int iter );

public:
    KNFStencil( T _wCenter,
                    T _wCardinal,
                    T _wDiagonal,
                    int _device );

    virtual void operator()( Matrix2D<T>&, unsigned int nIters );
};

#endif /*KNFSTENCIL_H */
