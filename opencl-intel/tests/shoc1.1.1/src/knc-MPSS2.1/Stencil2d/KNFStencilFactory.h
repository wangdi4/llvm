#ifndef KNFSTENCILFACTORY_H
#define KNFSTENCILFACTORY_H

#include "CommonKNFStencilFactory.h"

template<class T>
class KNFStencilFactory : public CommonKNFStencilFactory<T>
{
public:
    KNFStencilFactory( void )
      : CommonKNFStencilFactory<T>( "KNFStencil" )
    {
        // nothing else to do
    }

    virtual Stencil<T>* BuildStencil( const OptionParser& opts );
};

#endif // KNFSTENCILFACTORY_H

