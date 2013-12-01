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

File Name: OCLKernelDataGenerator.cpp

\*****************************************************************************/

#include "OCLKernelDataGenerator.h"
#include "RandomUniformProvider.h"
#include "Generators.h"
#include "BufferContainerList.h"

namespace Validation{
    OCLKernelDataGenerator::OCLKernelDataGenerator(const OCLKernelArgumentsList &arguments,
        const OCLKernelDataGeneratorConfig &config)
        : m_RandomUniformProvider(config.getSeed()),
        m_KernelArgumentsList(arguments)
    {
        // set generators vector size to size of config vector
        m_GeneratorsVector.resize(arguments.size());
        if(config.getConstConfigVector().size()!=arguments.size())
        {
            throw Exception::InvalidArgument(
                "[OCLKernelDataGenerator::OCLKernelDataGenerator]number of configs is not equal to \
                number of arguments");
        }
        // loop over configs
        AbstractGeneratorConfigVector::const_iterator it;
        size_t cnt;
        TypeDesc headDesc;
        for (it = config.getConstConfigVector().begin(), cnt=0;
            it != config.getConstConfigVector().end(); ++it, ++cnt)
        {
            IMemoryObjectDescPtr& pDesc = m_KernelArgumentsList[cnt];

            if(pDesc->GetName() == ImageDesc::GetImageDescName()){
                //assign new image desc taken from xml config
                pDesc = static_cast<ImageRandomGeneratorConfig*>(*it)->getImageDescriptor();
            }
            // call generator factory for each config
            AbstractGenerator * gen  = GeneratorFactory::create(*it, m_RandomUniformProvider);
            // set generator to vector
            m_GeneratorsVector[cnt] = gen;

            IMemoryObjectDesc* elemDesc = pDesc.get();
            checkConfig(elemDesc, *it); //check for config errors
        }
    }

    OCLKernelDataGenerator::~OCLKernelDataGenerator(){
        std::vector<AbstractGenerator*>::iterator it;
        m_GeneratorMap.clear();
        for(it = m_GeneratorsVector.begin(); it!=m_GeneratorsVector.end(); ++it)
            delete (*it); //call desctructor for each generator
    }

    void OCLKernelDataGenerator::Read(IContainer *p)
    {
        // allocate memory
        OCLKernelBufferContainerListAllocator allocator(m_KernelArgumentsList);
        allocator.Read(p);

        // clear map
        m_GeneratorMap.clear();

        // loop over arguments
        BufferContainerList* kernelArgValues  = static_cast<BufferContainerList*>(p);
        OCLKernelArgumentsList::const_iterator it;
        size_t cnt;
        IBufferContainer * pBC = kernelArgValues ->GetBufferContainer(0);
        for (it = m_KernelArgumentsList.begin(), cnt=0;
            it != m_KernelArgumentsList.end(); ++it, ++cnt)
        {
            // map generator to IMemoryObject
            m_GeneratorMap[pBC->GetMemoryObject(cnt)] = m_GeneratorsVector[cnt];
        }

        // call visit methods
        kernelArgValues ->Accept(*this);

        return;
    }

    void OCLKernelDataGenerator::checkConfig(IMemoryObjectDesc* elemDesc, AbstractGeneratorConfig* cfg){
        if(elemDesc->GetName() == BufferDesc::GetBufferDescName())
        {
            TypeDesc nextElemDesc = (static_cast<BufferDesc*>(elemDesc)->GetElementDescription());
            //order is important
            if(nextElemDesc.GetType() == TPOINTER)
            {
                nextElemDesc = nextElemDesc.GetSubTypeDesc(0);
            }
            if(nextElemDesc.GetType() == TARRAY)
            {
                nextElemDesc = nextElemDesc.GetSubTypeDesc(0);
            }
            if(nextElemDesc.GetType() == TVECTOR)
            {
                nextElemDesc = nextElemDesc.GetSubTypeDesc(0);
            }
            cfg->checkConfig(&nextElemDesc);
        }
        else if(elemDesc->GetName() == ImageDesc::GetImageDescName())
        {
            cfg->checkConfig(static_cast<ImageDesc*>(elemDesc));
        }
        else
        {
            throw Exception::InvalidArgument("[OCLKernelDataGenerator]Unsupported Memory object : "+ elemDesc->GetName());
        }
    }
}

