/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: GeneratorConfig.cpp

\*****************************************************************************/


#include"GeneratorConfig.h"
#include"BufferDesc.h"
#include"ImageDesc.h"
#include<assert.h>
using namespace Validation;

#define BUFFERCONSTGENERATORCONFIG_FACTORY(Ty) else if(name == BufferConstGeneratorConfig<Ty>::getStaticName())\
    res = new BufferConstGeneratorConfig<Ty>;
#define BUFFERRANDOMGENERATORCONFIG_FACTORY(Ty) else if(name == BufferRandomGeneratorConfig<Ty>::getStaticName())\
    res = new BufferRandomGeneratorConfig<Ty>;

AbstractGeneratorConfig * GeneratorConfigFactory::create(std::string name)
{
    AbstractGeneratorConfig *res = 0;
    if(name == BufferConstGeneratorConfig<float>::getStaticName())
    {
        res = new BufferConstGeneratorConfig<float>;
    }
    BUFFERCONSTGENERATORCONFIG_FACTORY(double)
        BUFFERCONSTGENERATORCONFIG_FACTORY(uint8_t)
        BUFFERCONSTGENERATORCONFIG_FACTORY(int8_t)
        BUFFERCONSTGENERATORCONFIG_FACTORY(uint16_t)
        BUFFERCONSTGENERATORCONFIG_FACTORY(int16_t)
        BUFFERCONSTGENERATORCONFIG_FACTORY(uint32_t)
        BUFFERCONSTGENERATORCONFIG_FACTORY(int32_t)
        BUFFERCONSTGENERATORCONFIG_FACTORY(uint64_t)
        BUFFERCONSTGENERATORCONFIG_FACTORY(int64_t)

        BUFFERRANDOMGENERATORCONFIG_FACTORY(float)
        BUFFERRANDOMGENERATORCONFIG_FACTORY(double)
        BUFFERRANDOMGENERATORCONFIG_FACTORY(uint8_t)
        BUFFERRANDOMGENERATORCONFIG_FACTORY(int8_t)
        BUFFERRANDOMGENERATORCONFIG_FACTORY(uint16_t)
        BUFFERRANDOMGENERATORCONFIG_FACTORY(int16_t)
        BUFFERRANDOMGENERATORCONFIG_FACTORY(uint32_t)
        BUFFERRANDOMGENERATORCONFIG_FACTORY(int32_t)
        BUFFERRANDOMGENERATORCONFIG_FACTORY(uint64_t)
        BUFFERRANDOMGENERATORCONFIG_FACTORY(int64_t)

    else if(name == BufferStructureGeneratorConfig::getStaticName())
    {
        res = new BufferStructureGeneratorConfig();
    }
    else{
        throw Exception::GeneratorBadTypeException("[GeneratorConfigFactory::create] bad config name");
    }
    return res;
}
#undef BUFFERCONSTGENERATORCONFIG_FACTORY
#undef BUFFERRANDOMGENERATORCONFIG_FACTORY


OCLKernelDataGeneratorConfig* OCLKernelDataGeneratorConfig::defaultConfig(const OCLKernelArgumentsList& args){
    IMemoryObjectDesc *argDesc;

    OCLKernelArgumentsList::const_iterator c_it;
    OCLKernelDataGeneratorConfig* result =  new OCLKernelDataGeneratorConfig();

    for(c_it = args.begin(); c_it!=args.end(); ++c_it){
        argDesc = (*c_it).get();

        if(argDesc->GetName() == ImageDesc::GetImageDescName())
        {
            throw Exception::NotImplemented(
                "[OCLKernelDataGeneratorConfig::defaultConfig] Images are not supported");
        }
        else if(argDesc->GetName() == BufferDesc::GetBufferDescName())
        {
            TypeDesc elemDesc = static_cast<BufferDesc*>(argDesc)->GetElementDescription();
            result->getConfigVector().push_back(defaultConfig(elemDesc));
        }
    }
    return result;
}

#define DEFAULTCONFIGMACROS(TyVal, Ty) case TyVal: {ret = GeneratorConfigFactory::create(BufferRandomGeneratorConfig<Ty>::getStaticName()); break;}

AbstractGeneratorConfig *OCLKernelDataGeneratorConfig::defaultConfig(const TypeDesc& Ty){
    TypeDesc subElem;
    AbstractGeneratorConfig* ret;
    switch(Ty.GetType()){
        //scalar data types
    DEFAULTCONFIGMACROS(TCHAR, int8_t)
    DEFAULTCONFIGMACROS(TUCHAR, uint8_t)
    DEFAULTCONFIGMACROS(TSHORT, int16_t)
    DEFAULTCONFIGMACROS(TUSHORT, uint16_t)
    DEFAULTCONFIGMACROS(TINT, int32_t)
    DEFAULTCONFIGMACROS(TUINT, uint32_t)
    DEFAULTCONFIGMACROS(TLONG, int64_t)
    DEFAULTCONFIGMACROS(TULONG, uint64_t)
    DEFAULTCONFIGMACROS(TFLOAT, float)
    DEFAULTCONFIGMACROS(TDOUBLE, double)
        //aggregate data types
    case TARRAY:
    case TPOINTER:
    case TVECTOR:
    {
        subElem = Ty.GetSubTypeDesc(0);
        ret = defaultConfig(subElem);
        break;
    }

    case TSTRUCT:
    {
        uint64_t n_elems = Ty.GetNumOfSubTypes();
        BufferStructureGeneratorConfig* structCFG = new BufferStructureGeneratorConfig;
        for(uint64_t i =0 ;i<n_elems;++i)
        {
            structCFG->getConfigVector().push_back(defaultConfig(Ty.GetSubTypeDesc(i)));
        }
        ret = structCFG;
        break;
    }
    default:
        throw Exception::GeneratorBadTypeException(
            "[OCLKernelDataGeneratorConfig::defaultConfig] Bad type within buffer");
    }
    return ret;
}

#undef DEFAULTCONFIGMACROS
