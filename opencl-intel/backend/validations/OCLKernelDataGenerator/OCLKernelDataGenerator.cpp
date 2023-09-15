// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "OCLKernelDataGenerator.h"
#include "BufferContainerList.h"
#include "Generators.h"
#include "RandomUniformProvider.h"

namespace Validation {
OCLKernelDataGenerator::OCLKernelDataGenerator(
    const OCLKernelArgumentsList &arguments,
    const OCLKernelDataGeneratorConfig &config)
    : m_RandomUniformProvider(config.getSeed()),
      m_KernelArgumentsList(arguments) {
  // set generators vector size to size of config vector
  m_GeneratorsVector.resize(arguments.size());
  if (config.getConstConfigVector().size() != arguments.size()) {
    throw Exception::InvalidArgument(
        "[OCLKernelDataGenerator::OCLKernelDataGenerator]number of configs is not equal to \
                number of arguments");
  }
  // loop over configs
  AbstractGeneratorConfigVector::const_iterator it;
  size_t cnt;
  TypeDesc headDesc;
  for (it = config.getConstConfigVector().begin(), cnt = 0;
       it != config.getConstConfigVector().end(); ++it, ++cnt) {
    IMemoryObjectDescPtr &pDesc = m_KernelArgumentsList[cnt];

    if (pDesc->GetName() == ImageDesc::GetImageDescName()) {
      // assign new image desc taken from xml config
      pDesc =
          static_cast<ImageRandomGeneratorConfig *>(*it)->getImageDescriptor();
    }
    // call generator factory for each config
    AbstractGenerator *gen =
        GeneratorFactory::create(*it, m_RandomUniformProvider);
    // set generator to vector
    m_GeneratorsVector[cnt] = gen;

    IMemoryObjectDesc *elemDesc = pDesc.get();
    checkConfig(elemDesc, *it); // check for config errors
  }
}

OCLKernelDataGenerator::~OCLKernelDataGenerator() {
  std::vector<AbstractGenerator *>::iterator it;
  m_GeneratorMap.clear();
  for (it = m_GeneratorsVector.begin(); it != m_GeneratorsVector.end(); ++it)
    delete (*it); // call desctructor for each generator
}

void OCLKernelDataGenerator::Read(IContainer *p) {
  // allocate memory
  OCLKernelBufferContainerListAllocator allocator(m_KernelArgumentsList);
  allocator.Read(p);

  // clear map
  m_GeneratorMap.clear();

  // loop over arguments
  BufferContainerList *kernelArgValues = static_cast<BufferContainerList *>(p);
  OCLKernelArgumentsList::const_iterator it;
  size_t cnt;
  IBufferContainer *pBC = kernelArgValues->GetBufferContainer(0);
  for (it = m_KernelArgumentsList.begin(), cnt = 0;
       it != m_KernelArgumentsList.end(); ++it, ++cnt) {
    // map generator to IMemoryObject
    m_GeneratorMap[pBC->GetMemoryObject(cnt)] = m_GeneratorsVector[cnt];
  }

  // call visit methods
  kernelArgValues->Accept(*this);

  return;
}

void OCLKernelDataGenerator::checkConfig(IMemoryObjectDesc *elemDesc,
                                         AbstractGeneratorConfig *cfg) {
  if (elemDesc->GetName() == BufferDesc::GetBufferDescName()) {
    TypeDesc nextElemDesc =
        (static_cast<BufferDesc *>(elemDesc)->GetElementDescription());
    // order is important
    if (nextElemDesc.GetType() == TPOINTER) {
      nextElemDesc = nextElemDesc.GetSubTypeDesc(0);
    }
    if (nextElemDesc.GetType() == TARRAY) {
      nextElemDesc = nextElemDesc.GetSubTypeDesc(0);
    }
    if (nextElemDesc.GetType() == TVECTOR) {
      nextElemDesc = nextElemDesc.GetSubTypeDesc(0);
    }
    cfg->checkConfig(&nextElemDesc);
  } else if (elemDesc->GetName() == ImageDesc::GetImageDescName()) {
    cfg->checkConfig(static_cast<ImageDesc *>(elemDesc));
  } else {
    throw Exception::InvalidArgument(
        "[OCLKernelDataGenerator]Unsupported Memory object : " +
        elemDesc->GetName());
  }
}
} // namespace Validation
