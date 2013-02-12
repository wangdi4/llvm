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

File Name: Generators.h

\*****************************************************************************/

#ifndef __ABSTRACT_GENERATOR_H__
#define __ABSTRACT_GENERATOR_H__

#include<string.h>
#include"IMemoryObject.h"
#include"RandomUniformProvider.h"
#include"GeneratorConfig.h"
#include<time.h>

namespace Validation
{
    ///abstract generator class
    ///declares methods that needed to generate data
    class AbstractGenerator
    {
    public:
        ///@brief ctor
        ///@param [in] reference of RandomUniformProvider class
        ///             RandomUniformProvider have a method that generates
        ///             uniformly distributed random values
        AbstractGenerator(const RandomUniformProvider& Randgen)
            : m_rng(Randgen)
        {}

        ///@brief dtor
        virtual ~AbstractGenerator(){}

        ///@brief responsible for data generating preporation
        ///calculates num of elements in the buffer, pointer to memory
        ///calls GenerateBufferCaller
        ///@param [in out] ptr is a pointer to IMemoryObject(buffer or image)
        virtual void Generate(const IMemoryObject *ptr)=0;
    protected:
        ///@brief return reference to encapsulated instance
        ///of RandomUniformProvider class
        ///@return reference to RandomUniformProvider
        const RandomUniformProvider& GetRandomUniformProvider() const { return m_rng; }
    private:
        ///reference to RandomUniformProvider
        const RandomUniformProvider& m_rng;
    };

    ///implements GenerateBufferCaller that declared in
    ///AbstractGenerator class. declares GenerateBuffer
    class AbstractBufferGenerator: public AbstractGenerator
    {
    public:
        ///@brief ctor
        ///for more info see AbstractGenerator ctor
        AbstractBufferGenerator(const RandomUniformProvider& Randgen)
            : AbstractGenerator(Randgen)
        {}
        ///@brief responsible for data generating preporation
        ///@param [in] ptr is a pointer to IMemoryObject(buffer of image)
        virtual void Generate(const IMemoryObject *ptr);
        ///@brief generates data into buffer
        ///@param [in out] p is a pointer to buffer data
        ///@param [in] n_elems - number of elements to generate
        ///@param [in] stride - stride between these elements
        virtual void GenerateBuffer(void *p, uint64_t n_elems, uint64_t stride)=0;
        ///@brief set element that the generator responsible for
        ///@param [in] type descriptor of element
        void SetElementDesc(const TypeDesc& ty){
            m_headDesc = ty;
        }
        ///@brief get access to Descriptor of element
        ///generator responsible for
        ///@return type descriptor of element
        const TypeDesc& GetElementDesc() const{
            return m_headDesc;
        }
    protected:
        /// descriptor of element, generator responsible for
        TypeDesc m_headDesc;
    };

    ///declares GenerateImage method
    class AbstractImageGenerator: public AbstractGenerator
    {
    public:
        ///@brief generates data into image
        virtual void GenerateImage()=0;
    };
    ///responsible for filling buffer with constant value
    template<typename T>
    class BufferConstGenerator: public AbstractBufferGenerator
    {
    public:
        ///@brief ctor
        ///@param [in] Randgen is a reference to RandomUniformProvider
        ///@param [in] cfg is a instance of BufferConstGeneratorConfig related
        ///to current Const Generator
        BufferConstGenerator(const RandomUniformProvider& Randgen, const BufferConstGeneratorConfig<T>* cfg)
            : AbstractBufferGenerator(Randgen), m_fillVal(cfg->GetFillValue())
        {}
    protected:
        ///@brief generates data into buffer
        ///@param [in out] p is a pointer to buffer data
        ///@param [in] n_elems - number of elements to generate
        ///@param [in] stride - stride between these elements
        virtual void GenerateBuffer(void *p, uint64_t n_elems, uint64_t stride);
    private:
        T m_fillVal;
    };

    ///responsible for filling buffer with random values
    template<typename T>
    class BufferRandomGenerator: public AbstractBufferGenerator
    {
    public:
        ///@brief ctor
        ///@param [in] Randgen is a reference to RandomUniformProvider
        BufferRandomGenerator(const RandomUniformProvider& Randgen)
            : AbstractBufferGenerator(Randgen)
        {}
    protected:
        ///@brief generates data into buffer
        ///@param [in out] p is a pointer to buffer data
        ///@param [in] n_elems - number of elements to generate
        ///@param [in] stride - stride between these elements
        virtual void GenerateBuffer(void *p, uint64_t n_elems, uint64_t stride);
    private:
        // get random sample of type T
        T GetRandomValue(const RandomUniformProvider& r) const;
    };

    ///implements factory method
    ///needed to create instances of generators
    class GeneratorFactory
    {
    public:
        ///@brief creates new instance of Generator type which name stored
        ///in Config variable cfg and RandomUniformProvider rng
        ///@param [in] cfg is a related config
        ///@param [in] rng is a reference to the instance of RandomUniformProvider class
        static AbstractGenerator * create( const AbstractGeneratorConfig* cfg, const RandomUniformProvider& rng);
    };

    class AbstractAggregateGenerator:public AbstractBufferGenerator{
    public:
        ///@brief ctor
        ///@param [in] Randgen is a reference to RandomUniformProvider
        AbstractAggregateGenerator(const RandomUniformProvider& Randgen):AbstractBufferGenerator(Randgen){}
        std::vector<AbstractBufferGenerator*>& getSubGenerators(void){ return sub_Generators;}
        ~AbstractAggregateGenerator(){
            std::vector<AbstractBufferGenerator*>::iterator it;
            for(it = sub_Generators.begin(); it!=sub_Generators.end(); ++it)
                delete *it; //delete all sub generators
        }
    private:
        std::vector<AbstractBufferGenerator*> sub_Generators;
    };

    ///responsible for filling buffer of the struct with specified values
    class BufferStructureGenerator: public AbstractAggregateGenerator{
    public:
        ///@brief ctor
        ///@param [in] Randgen is a reference to RandomUniformProvider
        BufferStructureGenerator(const RandomUniformProvider& Randgen): AbstractAggregateGenerator(Randgen)
        {}
    protected:
        ///@brief generates data into buffer
        ///@param [in out] p is a pointer to buffer data
        ///@param [in] n_elems - number of elements to generate
        ///@param [in] stride - stride between these elements
        virtual void GenerateBuffer(void *p, uint64_t n_elems, uint64_t stride);
    };
}

#endif //__ABSTRACT_GENERATOR_H__
