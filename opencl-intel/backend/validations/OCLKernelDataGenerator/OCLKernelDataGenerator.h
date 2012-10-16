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

File Name: OCLKernelDataGenerator.h

\*****************************************************************************/


#ifndef __OCL_KERNEL_DATA_GENERATOR_H__
#define __OCL_KERNEL_DATA_GENERATOR_H__

#include <string.h>
#include <map>
#include "IDataReader.h"
#include "IContainerVisitor.h"
#include "RandomUniformProvider.h"
#include "OpenCLKernelArgumentsParser.h"
#include "OCLKernelBufferContainerListAllocator.h"
#include "GeneratorConfig.h"
#include "Generators.h"
#include "IBufferContainerList.h"
#include "Image.h"
#include "Buffer.h"

namespace Validation
{
    //map buffer or image to generators
    typedef std::map<const IMemoryObject *, AbstractGenerator *> AbstractGeneratorMap;

    ///class that represents OCLKernelDataGenerator
    class OCLKernelDataGenerator: public IDataReader,IContainerVisitor
    {
    public:
        ///@brief ctor
        ///creates generators by given configs
        ///if configs not given, the creates it(if defaultconfig flag is set up)
        ///@param [in] arguments is a list of kernel arguments
        ///@param [in] config is a reference to OCLKernelDataGeneratorConfig
        OCLKernelDataGenerator(const OCLKernelArgumentsList &arguments, const OCLKernelDataGeneratorConfig &config);
        ///@brief dtor
        ///deletes generators
        virtual ~OCLKernelDataGenerator();

        ///@brief allocates memory and generates data
        ///@param [in out] p is a pointer to BufferContainerList
        virtual void Read(IContainer *p);

        ///@brief visit Buffer
        ///@param [in out] p is a Buffer
        virtual void visitBuffer(const IMemoryObject* p)
        {
            visitBufferAndImage(p);
        }

        virtual void visitBufferContainerList(const IBufferContainerList* p){
        }
        virtual void visitBufferContainer(const IBufferContainer* p){
        }
        ///@brief visit Image
        ///@param [in out] p is an Image
        virtual void visitImage(const IMemoryObject* p)
        {
            visitBufferAndImage(p);
        }

        inline AbstractGenerator* GetArgumentFillMethod(int indx)
        {
            return m_GeneratorsVector[indx];
        }
    private:
        ///@brief visit Buffer And Image
        ///@param [in out] p is a Buffer or Image
        void visitBufferAndImage(const IMemoryObject* p)
        {
            AbstractGenerator * pGen = m_GeneratorMap[p];
            pGen->Generate(p);
        }
        ///@brief checks whether configuration is valid or not
        ///throws error if not
        ///@param [in] elemDesc is a pointer to descriptor of the current element
        ///@param [in] cfg is a pointer to node of config of the current element
        void checkConfig(TypeDesc* elemDesc, AbstractGeneratorConfig* cfg);
        //RandomUniformProvider
        RandomUniformProvider m_RandomUniformProvider;
        //map IMemoryObject* and AbstractGenerator*
        AbstractGeneratorMap m_GeneratorMap;
        //generators vector
        std::vector<AbstractGenerator *> m_GeneratorsVector;
        //list of arguments
        OCLKernelArgumentsList m_KernelArgumentsList;
    };
}
#endif
