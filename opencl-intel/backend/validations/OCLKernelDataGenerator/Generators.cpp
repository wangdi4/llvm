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

File Name: Generators.cpp

\*****************************************************************************/

#include "Generators.h"
#include "BufferDesc.h"
#include "FloatOperations.h"
#include "Buffer.h"
#include "Image.h"

namespace Validation {

    void AbstractBufferGenerator::Generate(const IMemoryObject *ptr)
    {
        // check IMemoryObject *ptr is Buffer
        if(ptr->GetName() != Buffer::GetBufferName())
        {
            throw Exception::InvalidArgument(
                "[AbstractBufferGenerator::Generate]Buffer expected");
        }
        BufferDesc buffDesc = GetBufferDescription(ptr->GetMemoryObjectDesc());
        uint64_t n_elemsTotal=buffDesc.NumOfElements(); // number of elements to generate


        if(0==n_elemsTotal) //dont know how to generate null-size argument
            throw Exception::InvalidArgument(
            "[AbstractBufferGenerator::Generate]null-size argument");

        TypeDesc elemDesc = buffDesc.GetElementDescription();

        //order of ifs is important for dereferencing
        //possible variants - pointer, pointer to vector
        //vector, pointer
        //__kernel void example(__global float* in)
        if(elemDesc.GetType() == TPOINTER){
            n_elemsTotal *= elemDesc.GetNumberOfElements();
            elemDesc = elemDesc.GetSubTypeDesc(0);
        }
        //pointer to array or simple array
        if(elemDesc.GetType() == TARRAY){
            throw Exception::GeneratorBadTypeException("[AbstractBufferGenerator::Generate]\
                                                       arrays at kernel argument are not supported");
        }
        //pointer to vectors or simple vector
        //__kernel void example(__global float4* in)
        //__kernel void example(float4 in)
        if(elemDesc.GetType() == TVECTOR){
            n_elemsTotal *= elemDesc.GetNumberOfElements();
            elemDesc = elemDesc.GetSubTypeDesc(0);
        }

        void *p=ptr->GetDataPtr();

        m_headDesc = elemDesc;
        GenerateBuffer(p, n_elemsTotal, elemDesc.GetSizeInBytes());
    }

    template<typename T>
    void BufferConstGenerator<T>::GenerateBuffer(void *p, uint64_t n_elems, uint64_t stride)
    {
        uint8_t* ptr =(uint8_t*)p;
        for(uint64_t i=0; i<n_elems; ++i){
            ptr = (uint8_t*)p + i*stride;
            *((T*)ptr) = m_fillVal;
        }
    }

    template <> float BufferRandomGenerator<float>::GetRandomValue(const RandomUniformProvider& r) const
    { return r.sample_f32(); }
    template <> double BufferRandomGenerator<double>::GetRandomValue(const RandomUniformProvider& r) const
    { return r.sample_f64(); }
    template <typename T> T BufferRandomGenerator<T>::GetRandomValue(const RandomUniformProvider& r) const
    { 
        IsIntegerType<T> _notUsed;
        UNUSED_ARGUMENT(_notUsed);
        return r.sample_u64(); 
    }

    template<typename T>
    void BufferRandomGenerator<T>::GenerateBuffer(void *p, uint64_t n_elems, uint64_t stride)
    {
        IsScalarType<T> _notUsed;
        UNUSED_ARGUMENT(_notUsed);
        uint8_t* ptr =(uint8_t*)p;
        for(uint64_t i=0; i<n_elems; ++i){
            ptr = (uint8_t*)p + i*stride;
            *((T*)ptr) = GetRandomValue(GetRandomUniformProvider());
        }
    }

    void BufferStructureGenerator::GenerateBuffer(void *p, uint64_t n_elems, uint64_t stride){
        uint8_t* ptr=(uint8_t*)p;
        uint8_t *local_ptr;

        TypeDesc subElem;
        if(GetElementDesc().GetType()!=TSTRUCT)
            throw Exception::InvalidArgument(
            "[BufferStructureGenerator::GenerateBuffer] incorrect type descriptor in structure generator");


        for(uint64_t i =0; i< getSubGenerators().size(); ++i){
            uint64_t numDereferencedElems=1;
            subElem=m_headDesc.GetSubTypeDesc(i);

            if(subElem.GetType() == TPOINTER){
                throw Exception::GeneratorBadTypeException("[BufferStructureGenerator::GenerateBuffer]\
                                                           pointers within structs are not supported");
            }
            //order of ifs is important
            //simple array
            if(subElem.GetType() == TARRAY){
                numDereferencedElems *= subElem.GetNumberOfElements();
                subElem = subElem.GetSubTypeDesc(0);
            }
            //array of vectors or simple vector
            if(subElem.GetType() == TVECTOR){
                numDereferencedElems *= subElem.GetNumberOfElements();
                subElem = subElem.GetSubTypeDesc(0);
            }

            getSubGenerators()[i]->SetElementDesc(GetElementDesc().GetSubTypeDesc(i));
            local_ptr =ptr;
            for(uint64_t j=0; j<numDereferencedElems; ++j)
            {
                getSubGenerators()[i]->GenerateBuffer(local_ptr, n_elems, GetElementDesc().GetSizeInBytes());
                local_ptr+=subElem.GetSizeInBytes();
            }
            ptr+=GetElementDesc().GetSubTypeDesc(i).GetSizeInBytes(); //goto next structure member
        }
    }

#define BUFFERCONSTGENERATOR_FACTORY(Ty) else if(name == BufferConstGeneratorConfig<Ty>::getStaticName())\
    res = new BufferConstGenerator<Ty>(rng, static_cast<const BufferConstGeneratorConfig<Ty> *> (cfg));
#define BUFFERRANDOMGENERATOR_FACTORY(Ty) else if(name == BufferRandomGeneratorConfig<Ty>::getStaticName())\
    res = new BufferRandomGenerator<Ty>(rng);

    AbstractGenerator * GeneratorFactory::create(const AbstractGeneratorConfig* cfg, const RandomUniformProvider& rng)
    {
        std::string name = cfg->getName();
        AbstractGenerator * res = 0;
        if(name == BufferConstGeneratorConfig<float>::getStaticName())
        {
            res = new BufferConstGenerator<float>(rng, static_cast<const BufferConstGeneratorConfig<float> *> (cfg));
        }
        BUFFERCONSTGENERATOR_FACTORY(double)
            BUFFERCONSTGENERATOR_FACTORY(uint8_t)
            BUFFERCONSTGENERATOR_FACTORY(int8_t)
            BUFFERCONSTGENERATOR_FACTORY(uint16_t)
            BUFFERCONSTGENERATOR_FACTORY(int16_t)
            BUFFERCONSTGENERATOR_FACTORY(uint32_t)
            BUFFERCONSTGENERATOR_FACTORY(int32_t)
            BUFFERCONSTGENERATOR_FACTORY(uint64_t)
            BUFFERCONSTGENERATOR_FACTORY(int64_t)

            BUFFERRANDOMGENERATOR_FACTORY(float)
            BUFFERRANDOMGENERATOR_FACTORY(double)
            BUFFERRANDOMGENERATOR_FACTORY(uint8_t)
            BUFFERRANDOMGENERATOR_FACTORY(int8_t)
            BUFFERRANDOMGENERATOR_FACTORY(uint16_t)
            BUFFERRANDOMGENERATOR_FACTORY(int16_t)
            BUFFERRANDOMGENERATOR_FACTORY(uint32_t)
            BUFFERRANDOMGENERATOR_FACTORY(int32_t)
            BUFFERRANDOMGENERATOR_FACTORY(uint64_t)
            BUFFERRANDOMGENERATOR_FACTORY(int64_t)

        else if(name == BufferStructureGeneratorConfig::getStaticName()){
            AbstractGeneratorConfigVector::const_iterator it;
            AbstractGeneratorConfig* acfg = const_cast<AbstractGeneratorConfig*>(cfg);

            BufferStructureGenerator* g =new BufferStructureGenerator(rng);

            for(it = static_cast<BufferStructureGeneratorConfig*>(acfg)->getConfigVector().begin(); it!=static_cast<BufferStructureGeneratorConfig*>(acfg)->getConfigVector().end(); ++it)
            {
                g->getSubGenerators().push_back(static_cast<AbstractBufferGenerator *>(create(*it, rng)));
            }

            res=g;
        }
        else
            throw Exception::GeneratorBadTypeException("[GeneratorFACTORY::create]wrong generator name");

        return res;
    }
#undef BUFFERCONSTGENERATOR_FACTORY
#undef BUFFERRANDOMGENERATOR_FACTORY
}
