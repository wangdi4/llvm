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

#ifndef __GENERATOR_CONFIG_H__
#define __GENERATOR_CONFIG_H__

#include "FloatOperations.h"
#include "ImageDesc.h"
#include "OpenCLKernelArgumentsParser.h"
#include "TypeDesc.h"
#include "tinyxml_wrapper.h"
#include <string.h>
#include <utility>
#include <vector>

namespace Validation {
/// abstract class that describes method of generation
class AbstractGeneratorConfig : public TiXmlVisitor {
public:
  virtual ~AbstractGeneratorConfig() {}
  ///@brief read xml node into config
  virtual bool VisitEnter(const TiXmlElement &,
                          const TiXmlAttribute *) override = 0;
  ///@brief function to determinate generator name
  /// in runtime
  ///@return string with generator name
  virtual std::string getName() const = 0;
  ///@brief check whether config matches given type or not
  ///@param [in] Ty type descriptor
  virtual void checkConfig(const IMemoryObjectDesc *elem) = 0;
};

// vector of configs
typedef std::vector<AbstractGeneratorConfig *> AbstractGeneratorConfigVector;

/// implements factory method
/// needed to create instances of generator configs
class GeneratorConfigFactory {
public:
  ///@brief creates instance of Generator Config types
  /// with the same Generator name as given in args
  ///@param [in] name is the name of Data Generator which
  /// config we want to create
  ///@return new instance of desired Config
  static AbstractGeneratorConfig *create(const std::string &name);
};

/// describes config of the Generator and encapsulates
/// configs that belong to generators of each kernel argument
class OCLKernelDataGeneratorConfig : public TiXmlVisitor {
public:
  ///@brief default ctor
  OCLKernelDataGeneratorConfig() : m_seed(0) {}
  ///@brief ctor
  /// set up new seed
  explicit OCLKernelDataGeneratorConfig(uint64_t seed) : m_seed(seed) {}
  ///@brief deletes all sub configs that were generated with factory
  ~OCLKernelDataGeneratorConfig() {
    AbstractGeneratorConfigVector::iterator it;
    for (it = m_GeneratorConfigVector.begin();
         it != m_GeneratorConfigVector.end(); ++it)
      delete *it; // delete each config
  }

  OCLKernelDataGeneratorConfig(const OCLKernelDataGeneratorConfig &) = delete;
  OCLKernelDataGeneratorConfig &
  operator=(const OCLKernelDataGeneratorConfig &) = delete;

  ///@brief Read OCL Kernel Data Generator Config from file
  ///@param [in] ConfigFile - specified file to read config from.
  explicit OCLKernelDataGeneratorConfig(const TiXmlNode *ConfigNode);
  ///@brief allows to get access to sub configs vector
  /// that has been encapsulated in the class
  ///@return reference to the sub configs vector
  AbstractGeneratorConfigVector &getConfigVector(void) {
    return m_GeneratorConfigVector;
  }
  ///@brief allows to get read access to sub configs vector
  /// that has been encapsulated in the class
  ///@return const reference to the sub configs vector
  const AbstractGeneratorConfigVector &getConstConfigVector(void) const {
    return m_GeneratorConfigVector;
  }
  ///@brief get generator seed.
  ///@return seed
  uint64_t getSeed() const { return m_seed; }
  ///@brief sets up the seed
  ///@param [in] seed is a new seed
  void setSeed(const uint64_t seed) { m_seed = seed; }
  ///@brief creates default config for data generator by given arguments
  ///@param [in] arguments
  ///@return new instance of generator config
  static OCLKernelDataGeneratorConfig *
  defaultConfig(const OCLKernelArgumentsList &args);

private:
  static AbstractGeneratorConfig *defaultConfig(const TypeDesc &Ty);
  /// generator's seed
  uint64_t m_seed;
  /// list of configs. each kernel argument is assigned a configuration
  AbstractGeneratorConfigVector m_GeneratorConfigVector;
  ///@brief read xml node into config
  virtual bool VisitEnter(const TiXmlElement &,
                          const TiXmlAttribute *) override;
  typedef enum { SEED = 0, CONFIGS, OCLKERNELDGCONFIG, LASTFIELD } XMLField;
  // number of Successfully Loaded Fileds from XML file
  uint64_t m_SuccessfullyLoadedFileds[LASTFIELD];
};

/// store config of Constant Generator
template <typename T>
class BufferConstGeneratorConfig : public AbstractGeneratorConfig {
public:
  ///@brief sets the value that will fill data in the buffer
  ///@param [in] Value is the constant, buffer will be filled with
  void SetFillValue(const T Value) { m_FillValue = Value; }
  ///@brief returns copy of the filled value
  ///@return copy of the filled value
  T GetFillValue() const { return m_FillValue; }
  ///@brief returns name of Generator with specific type T
  ///@return name of Generator
  static std::string getStaticName() {
    IsScalarType<T> tmp;  // produces compile error if not scalar
    UNUSED_ARGUMENT(tmp); // to suppress compiler warnings
    return "BufferConstGenerator" +
           TypeValWrapper(InstTypeVal<T>())
               .ToString(); // get name of type(e.g., i64)
  }
  ///@brief function to determinate generator name
  /// in runtime
  ///@return string with generator name
  virtual std::string getName() const override { return getStaticName(); }
  ///@brief check whether config matches given type or not
  ///@param [in] Ty type descriptor
  virtual void checkConfig(const IMemoryObjectDesc *elem) override {
    const TypeDesc *Ty = static_cast<const TypeDesc *>(elem);
    if (getStaticName() !=
        "BufferConstGenerator" + TypeValWrapper(Ty->GetType()).ToString())
      throw Exception::InvalidArgument(
          "[BufferConstGeneratorConfig::checkConfig]\
                                                 config does not match type");
  }

private:
  ///@brief read xml node into config
  virtual bool VisitEnter(const TiXmlElement &,
                          const TiXmlAttribute *) override;
  /// value that will fill data in the buffer
  T m_FillValue;
};

/// store config of Random Generator
template <typename T>
class BufferRandomGeneratorConfig : public AbstractGeneratorConfig {
public:
  ///@brief returns name of Generator
  /// with specific type T
  ///@return name of Generator
  static std::string getStaticName() {
    IsScalarType<T> tmp;
    UNUSED_ARGUMENT(tmp);
    return "BufferRandomGenerator" +
           TypeValWrapper(InstTypeVal<T>()).ToString();
  }
  ///@brief function to determinate generator name
  /// in runtime
  ///@return string with generator name
  virtual std::string getName() const override { return getStaticName(); }
  ///@brief check whether config matches given type or not
  ///@param [in] Ty type descriptor
  virtual void checkConfig(const IMemoryObjectDesc *elem) override {
    const TypeDesc *Ty = static_cast<const TypeDesc *>(elem);
    if (getStaticName() !=
        "BufferRandomGenerator" + TypeValWrapper(Ty->GetType()).ToString())
      throw Exception::InvalidArgument("[BufferRandomGenerator::checkConfig]\
                                                 config does not match type");
  }

private:
  ///@brief read xml node into config
  virtual bool VisitEnter(const TiXmlElement &,
                          const TiXmlAttribute *) override {
    return true;
  }
};

/// responsible for storing information about how to generate structure
/// encapsulates Config for each sub type of the struct
class BufferStructureGeneratorConfig : public AbstractGeneratorConfig {
public:
  ///@brief dtor
  /// it deletes tree of the config
  ~BufferStructureGeneratorConfig() {
    AbstractGeneratorConfigVector::iterator it;
    for (it = m_subConfigs.begin(); it != m_subConfigs.end(); ++it)
      delete *it; // delete each config
  }
  ///@brief allows to get access to sub configs vector
  /// that has been encapsulated in the class
  ///@return reference to the sub configs vector
  AbstractGeneratorConfigVector &getConfigVector(void) { return m_subConfigs; }
  ///@brief returns name of Generator
  ///@return name of Generator
  static std::string getStaticName() { return "BufferStructureGenerator"; }
  ///@brief function to determinate generator name
  /// in runtime
  ///@return string with generator name
  virtual std::string getName() const override { return getStaticName(); }
  ///@brief check whether config matches given type or not
  ///@param [in] Ty type descriptor
  virtual void checkConfig(const IMemoryObjectDesc *elem) override {
    const TypeDesc *Ty = static_cast<const TypeDesc *>(elem);
    if (Ty->GetType() != TSTRUCT)
      throw Exception::InvalidArgument(
          "[BufferStructureGeneratorConfig::checkConfig]\
                                                 config does not match type");
    AbstractGeneratorConfigVector::const_iterator c_it;
    uint64_t cnt = 0;
    for (c_it = m_subConfigs.begin(); c_it != m_subConfigs.end();
         ++c_it, ++cnt) {
      TypeDesc nextElemDesc = Ty->GetSubTypeDesc(cnt);
      if (nextElemDesc.GetType() == TPOINTER) {
        throw Exception::GeneratorBadTypeException(
            "[BufferStructureGeneratorConfig::checkConfig]\
                                                           pointers within structs are not supported");
      }
      if (nextElemDesc.GetType() == TARRAY) {
        nextElemDesc = nextElemDesc.GetSubTypeDesc(0);
      }
      if (nextElemDesc.GetType() == TVECTOR) {
        nextElemDesc = nextElemDesc.GetSubTypeDesc(0);
      }
      (*c_it)->checkConfig(&nextElemDesc);
    }
  }

private:
  ///@brief read xml node into config
  virtual bool VisitEnter(const TiXmlElement &,
                          const TiXmlAttribute *) override;
  AbstractGeneratorConfigVector
      m_subConfigs; // configs for sub types of structure
};

/// store config of Random Generator
class ImageRandomGeneratorConfig : public AbstractGeneratorConfig {
public:
  virtual ~ImageRandomGeneratorConfig() { delete m_ObjDesc; }
  ///@brief returns name of Image Generator
  ///@return name of Generator
  static std::string getStaticName() { return "ImageRandomGenerator"; }
  ///@brief function to determinate generator name
  /// in runtime
  ///@return string with generator name
  virtual std::string getName() const override { return getStaticName(); }
  ///@brief check whether config matches given type or not
  ///@param [in] Ty type descriptor
  virtual void checkConfig(const IMemoryObjectDesc *elem) override {
    const ImageDesc *imdesc = static_cast<const ImageDesc *>(elem);
    if ((*imdesc) != (*m_ObjDesc))
      throw Exception::InvalidArgument(
          "[ImageRandomGeneratorConfig::checkConfig]\
                                                 Corrupted image descriptor inside config");
  }
  ImageDesc *getImageDescriptor() { return m_ObjDesc; }

private:
  ///@brief read xml node into config
  virtual bool VisitEnter(const TiXmlElement &,
                          const TiXmlAttribute *) override;
  ///@brief read xml node into config
  virtual bool VisitExit(const TiXmlElement &element) override { return false; }
  // image descriptor. new image will be created with this one
  ImageDesc *m_ObjDesc;
};
} // namespace Validation
#endif //__GENERATOR_CONFIG_H__
