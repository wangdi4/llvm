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

#ifndef __OCL_KERNEL_DATA_GENERATOR_H__
#define __OCL_KERNEL_DATA_GENERATOR_H__

#include "Buffer.h"
#include "GeneratorConfig.h"
#include "Generators.h"
#include "IBufferContainerList.h"
#include "IContainerVisitor.h"
#include "IDataReader.h"
#include "Image.h"
#include "OCLKernelBufferContainerListAllocator.h"
#include "OpenCLKernelArgumentsParser.h"
#include "RandomUniformProvider.h"
#include <map>
#include <string.h>

namespace Validation {
// map buffer or image to generators
typedef std::map<const IMemoryObject *, AbstractGenerator *>
    AbstractGeneratorMap;

/// class that represents OCLKernelDataGenerator
class OCLKernelDataGenerator : public IDataReader, IContainerVisitor {
public:
  ///@brief ctor
  /// creates generators by given configs
  /// if configs not given, the creates it(if defaultconfig flag is set up)
  ///@param [in] arguments is a list of kernel arguments
  ///@param [in] config is a reference to OCLKernelDataGeneratorConfig
  OCLKernelDataGenerator(const OCLKernelArgumentsList &arguments,
                         const OCLKernelDataGeneratorConfig &config);
  ///@brief dtor
  /// deletes generators
  virtual ~OCLKernelDataGenerator();

  ///@brief allocates memory and generates data
  ///@param [in out] p is a pointer to BufferContainerList
  virtual void Read(IContainer *p) override;

  ///@brief visit Buffer
  ///@param [in out] p is a Buffer
  virtual void visitBuffer(const IMemoryObject *p) override {
    visitBufferAndImage(p);
  }

  virtual void
  visitBufferContainerList(const IBufferContainerList *p) override {}
  virtual void visitBufferContainer(const IBufferContainer *p) override {}
  ///@brief visit Image
  ///@param [in out] p is an Image
  virtual void visitImage(const IMemoryObject *p) override {
    visitBufferAndImage(p);
  }

  inline AbstractGenerator *GetArgumentFillMethod(int indx) {
    return m_GeneratorsVector[indx];
  }

private:
  ///@brief visit Buffer And Image
  ///@param [in out] p is a Buffer or Image
  void visitBufferAndImage(const IMemoryObject *p) {
    AbstractGenerator *pGen = m_GeneratorMap[p];
    pGen->Generate(p);
  }
  ///@brief checks whether configuration is valid or not
  /// throws error if not
  ///@param [in] elemDesc is a pointer to descriptor of
  /// the current element(image or element of the buffer)
  ///@param [in] cfg is a pointer to node of config of the current element
  void checkConfig(IMemoryObjectDesc *elemDesc, AbstractGeneratorConfig *cfg);
  // RandomUniformProvider
  RandomUniformProvider m_RandomUniformProvider;
  // map IMemoryObject* and AbstractGenerator*
  AbstractGeneratorMap m_GeneratorMap;
  // generators vector
  std::vector<AbstractGenerator *> m_GeneratorsVector;
  // list of arguments
  OCLKernelArgumentsList m_KernelArgumentsList;
};
} // namespace Validation
#endif
