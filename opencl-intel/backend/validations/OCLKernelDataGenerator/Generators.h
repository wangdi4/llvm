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

#ifndef __ABSTRACT_GENERATOR_H__
#define __ABSTRACT_GENERATOR_H__

#include "GeneratorConfig.h"
#include "IMemoryObject.h"
#include "RandomUniformProvider.h"
#include <string.h>
#include <time.h>

namespace Validation {
/// abstract generator class
/// declares methods that needed to generate data
class AbstractGenerator {
public:
  ///@brief ctor
  ///@param [in] reference of RandomUniformProvider class
  ///             RandomUniformProvider have a method that generates
  ///             uniformly distributed random values
  AbstractGenerator(const RandomUniformProvider &Randgen) : m_rng(Randgen) {}

  ///@brief dtor
  virtual ~AbstractGenerator() {}

  ///@brief responsible for data generating preporation
  /// calculates num of elements in the buffer, pointer to memory
  /// calls GenerateBufferCaller
  ///@param [in out] ptr is a pointer to IMemoryObject(buffer or image)
  virtual void Generate(const IMemoryObject *ptr) = 0;

protected:
  ///@brief return reference to encapsulated instance
  /// of RandomUniformProvider class
  ///@return reference to RandomUniformProvider
  const RandomUniformProvider &GetRandomUniformProvider() const {
    return m_rng;
  }

private:
  /// reference to RandomUniformProvider
  const RandomUniformProvider &m_rng;
};

/// implements GenerateBufferCaller that declared in
/// AbstractGenerator class. declares GenerateBuffer
class AbstractBufferGenerator : public AbstractGenerator {
public:
  ///@brief ctor
  /// for more info see AbstractGenerator ctor
  AbstractBufferGenerator(const RandomUniformProvider &Randgen)
      : AbstractGenerator(Randgen) {}
  ///@brief responsible for data generating preporation
  ///@param [in] ptr is a pointer to IMemoryObject(buffer of image)
  virtual void Generate(const IMemoryObject *ptr) override;
  ///@brief generates data into buffer
  ///@param [in out] p is a pointer to buffer data
  ///@param [in] n_elems - number of elements to generate
  ///@param [in] stride - stride between these elements
  virtual void GenerateBuffer(void *p, uint64_t n_elems, uint64_t stride) = 0;
  ///@brief set element that the generator responsible for
  ///@param [in] type descriptor of element
  void SetElementDesc(const TypeDesc &ty) { m_headDesc = ty; }
  ///@brief get access to Descriptor of element
  /// generator responsible for
  ///@return type descriptor of element
  const TypeDesc &GetElementDesc() const { return m_headDesc; }

protected:
  /// descriptor of element, generator responsible for
  TypeDesc m_headDesc;
};

/// declares GenerateImage method
class AbstractImageGenerator : public AbstractGenerator {
public:
  ///@brief ctor
  /// for more info see AbstractGenerator ctor
  AbstractImageGenerator(const RandomUniformProvider &Randgen)
      : AbstractGenerator(Randgen) {}
  ///@brief responsible for data generating preparation
  ///@param [in] ptr is a pointer to IMemoryObject(buffer of image)
  virtual void Generate(const IMemoryObject *ptr) override;
  ///@brief generates data into image
  virtual void GenerateImage(const void *p, const uint64_t pixels_in_row,
                             const ImageDesc &imdesc) = 0;
};

/// responsible for filling image with random values
class ImageRandomGenerator : public AbstractImageGenerator {
public:
  ///@brief ctor
  ///@param [in] Randgen is a reference to RandomUniformProvider
  ImageRandomGenerator(const RandomUniformProvider &Randgen)
      : AbstractImageGenerator(Randgen) {}

protected:
  ///@brief generates data into image
  ///@param [in out] p is a pointer to image data
  ///@param [in] pixels_in_row - number of elements to generate
  ///@param [in] imdesc - image description
  virtual void GenerateImage(const void *p, const uint64_t pixels_in_row,
                             const ImageDesc &imdesc) override;

private:
  ///@brief generate and pack one single pixel
  /// param [in out] p is a pointer to image data
  /// param [in] imdesc is image description
  void GenerateAndPackPixel(void *p, const ImageDesc &imdesc);
};

/// responsible for filling buffer with constant value
template <typename T>
class BufferConstGenerator : public AbstractBufferGenerator {
public:
  ///@brief ctor
  ///@param [in] Randgen is a reference to RandomUniformProvider
  ///@param [in] cfg is a instance of BufferConstGeneratorConfig related
  /// to current Const Generator
  BufferConstGenerator(const RandomUniformProvider &Randgen,
                       const BufferConstGeneratorConfig<T> *cfg)
      : AbstractBufferGenerator(Randgen), m_fillVal(cfg->GetFillValue()) {}

protected:
  ///@brief generates data into buffer
  ///@param [in out] p is a pointer to buffer data
  ///@param [in] n_elems - number of elements to generate
  ///@param [in] stride - stride between these elements
  virtual void GenerateBuffer(void *p, uint64_t n_elems,
                              uint64_t stride) override;

private:
  T m_fillVal;
};

/// responsible for filling buffer with random values
template <typename T>
class BufferRandomGenerator : public AbstractBufferGenerator {
public:
  ///@brief ctor
  ///@param [in] Randgen is a reference to RandomUniformProvider
  BufferRandomGenerator(const RandomUniformProvider &Randgen)
      : AbstractBufferGenerator(Randgen) {}

protected:
  ///@brief generates data into buffer
  ///@param [in out] p is a pointer to buffer data
  ///@param [in] n_elems - number of elements to generate
  ///@param [in] stride - stride between these elements
  virtual void GenerateBuffer(void *p, uint64_t n_elems,
                              uint64_t stride) override;
};

/// implements factory method
/// needed to create instances of generators
class GeneratorFactory {
public:
  ///@brief creates new instance of Generator type which name stored
  /// in Config variable cfg and RandomUniformProvider rng
  ///@param [in] cfg is a related config
  ///@param [in] rng is a reference to the instance of RandomUniformProvider
  /// class
  static AbstractGenerator *create(const AbstractGeneratorConfig *cfg,
                                   const RandomUniformProvider &rng);
};

class AbstractAggregateGenerator : public AbstractBufferGenerator {
public:
  ///@brief ctor
  ///@param [in] Randgen is a reference to RandomUniformProvider
  AbstractAggregateGenerator(const RandomUniformProvider &Randgen)
      : AbstractBufferGenerator(Randgen) {}
  std::vector<AbstractBufferGenerator *> &getSubGenerators(void) {
    return sub_Generators;
  }
  ~AbstractAggregateGenerator() {
    std::vector<AbstractBufferGenerator *>::iterator it;
    for (it = sub_Generators.begin(); it != sub_Generators.end(); ++it)
      delete *it; // delete all sub generators
  }

private:
  std::vector<AbstractBufferGenerator *> sub_Generators;
};

/// responsible for filling buffer of the struct with specified values
class BufferStructureGenerator : public AbstractAggregateGenerator {
public:
  ///@brief ctor
  ///@param [in] Randgen is a reference to RandomUniformProvider
  BufferStructureGenerator(const RandomUniformProvider &Randgen)
      : AbstractAggregateGenerator(Randgen) {}

protected:
  ///@brief generates data into buffer
  ///@param [in out] p is a pointer to buffer data
  ///@param [in] n_elems - number of elements to generate
  ///@param [in] stride - stride between these elements
  virtual void GenerateBuffer(void *p, uint64_t n_elems,
                              uint64_t stride) override;
};
} // namespace Validation

#endif //__ABSTRACT_GENERATOR_H__
