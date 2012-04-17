#ifndef COMMONKNFSTENCILFACTORY_H
#define COMMONKNFSTENCILFACTORY_H

#include <vector>
#include "StencilFactory.h"

// ****************************************************************************
// Class:  CommonKNFStencilFactory
//
// Purpose:
//   KNF implementation of stencil factory.
//
// ****************************************************************************
template<class T>
class CommonKNFStencilFactory : public StencilFactory<T>
{
protected:
    void ExtractOptions( const OptionParser& options,
                            T& wCenter,
                            T& wCardinal,
                            T& wDiagonal,
                            std::vector<long long>& devices );

public:
    CommonKNFStencilFactory( std::string _sname )
      : StencilFactory<T>( _sname )
    {
        // nothing else to do
    }

    virtual void CheckOptions( const OptionParser& opts ) const;
};

#endif // COMMONKNFSTENCILFACTORY_H

