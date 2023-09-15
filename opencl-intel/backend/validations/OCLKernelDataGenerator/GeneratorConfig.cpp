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

#include "GeneratorConfig.h"
#include "BufferDesc.h"
#include "ImageDesc.h"
#include <assert.h>
#include <time.h>

using namespace Validation;

OCLKernelDataGeneratorConfig::OCLKernelDataGeneratorConfig(
    const TiXmlNode *ConfigNode)
    : m_seed(0) {
  for (uint64_t i = 0; i < LASTFIELD; ++i) {
    m_SuccessfullyLoadedFileds[i] = 0;
  }
  ConfigNode->Accept(this);
}

bool OCLKernelDataGeneratorConfig::VisitEnter(
    const TiXmlElement &element, const TiXmlAttribute *firstAttribute) {
  if (element.ValueStr() == "OCLKernelDataGeneratorConfig") {
    ++m_SuccessfullyLoadedFileds[OCLKERNELDGCONFIG];
    if (m_SuccessfullyLoadedFileds[OCLKERNELDGCONFIG] > 1) {
      throw Exception::IOError(
          "[OCLKernelDataGeneratorConfig]OCLKernelDataGeneratorConfig "
          "couldn\'t be rewrited");
    }
    return true;
  }
  if (element.ValueStr() == "Seed") {
    ++m_SuccessfullyLoadedFileds[SEED];
    if (m_SuccessfullyLoadedFileds[SEED] > 1) {
      throw Exception::IOError(
          "[OCLKernelDataGeneratorConfig]Seed is already set");
    }
    std::stringstream ss(element.GetText());
    ss >> m_seed;
    if (!m_seed)
      m_seed = time(NULL);
    return true;
  } else if (element.ValueStr() == "GeneratorConfiguration") {
    std::string GeneratorName;
    std::string GeneratorType;
    uint32_t attributeState;

    ++m_SuccessfullyLoadedFileds[CONFIGS];

    attributeState = element.QueryStringAttribute("Name", &GeneratorName);
    if (attributeState != TIXML_SUCCESS || GeneratorName == "") {
      throw Exception::IOError(
          "[OCLKernelDataGeneratorConfig]Bad Name attribute");
    }

    // dont check type here becouse BufferStructureGenerator have no Type
    // check it in factory
    element.QueryStringAttribute("Type", &GeneratorType);

    AbstractGeneratorConfig *cfg =
        GeneratorConfigFactory::create(GeneratorName + GeneratorType);
    // manually iterate through node children.
    for (const TiXmlNode *node = element.FirstChild(); node;
         node = node->NextSibling()) {
      if (!node->Accept(cfg))
        break;
    }
    m_GeneratorConfigVector.push_back(cfg);
    return true;
  }
  // dont visit childs and siblings if function recieves non top-level attribute
  // listed above
  return false;
}

template <typename T>
bool BufferConstGeneratorConfig<T>::VisitEnter(
    const TiXmlElement &element, const TiXmlAttribute *firstAttribute) {
  if (element.ValueStr() == "Value") {
    std::stringstream ss(element.GetText());
    ss >> m_FillValue;
  }
  return false;
}

bool BufferStructureGeneratorConfig::VisitEnter(
    const TiXmlElement &element, const TiXmlAttribute *firstAttribute) {
  if (element.ValueStr() == "SubGeneratorConfiguration") {
    std::string GeneratorName;
    std::string GeneratorType;
    uint32_t attributeState;

    attributeState = element.QueryStringAttribute("Name", &GeneratorName);
    if (attributeState == TIXML_WRONG_TYPE ||
        attributeState == TIXML_NO_ATTRIBUTE || GeneratorName == "") {
      throw Exception::IOError(
          "[BufferStructureGeneratorConfig]Bad Name attribute");
    }

    element.QueryStringAttribute("Type", &GeneratorType);

    AbstractGeneratorConfig *cfg =
        GeneratorConfigFactory::create(GeneratorName + GeneratorType);
    // manually iterate through structure sub-elements. This implementation does
    // not visit xml element related to StructureConfig, it iterates only
    // sub-configs Also it doesn't stop iterating through sibling of child if
    // one of the children return false
    for (const TiXmlNode *node = element.FirstChild(); node;
         node = node->NextSibling()) {
      node->Accept(cfg);
    }
    m_subConfigs.push_back(cfg);
  }
  return false;
}
bool ImageRandomGeneratorConfig::VisitEnter(
    const TiXmlElement &element, const TiXmlAttribute *firstAttribute) {
  uint64_t ParseState = 0;
  ImageChannelOrderVal ChannelOrder = INVALID_CHANNEL_ORDER;
  ImageChannelDataTypeVal ChannelDataType = INVALID_IMAGE_DATA_TYPE;
  ImageTypeVal ImageType = INVALID_MEM_OBJECT_IMAGE;
  ImageSizeDesc ImageSize;
  uint64_t imageWidth = 0, imageHeight = 0, imageDepth = 0, imageArraySize = 0,
           imageRowPitch = 0, imageSlicePitch = 0;

  for (const TiXmlElement *ElementToParse = &element; ElementToParse != NULL;
       ElementToParse = ElementToParse->NextSiblingElement()) {
    if (ElementToParse->ValueStr() == "ChannelOrder") {
      std::string str(ElementToParse->GetText());
      ChannelOrder = ImageChannelOrderValWrapper::ValueOf(str);
      ParseState |= 0x01;
    } else if (ElementToParse->ValueStr() == "ChannelDataType") {
      std::string str(ElementToParse->GetText());
      ChannelDataType = ImageChannelDataTypeValWrapper::ValueOf(str);
      ParseState |= (0x01 << 1);
    } else if (ElementToParse->ValueStr() == "ImageType") {
      std::string str(ElementToParse->GetText());
      ImageType = ImageTypeValWrapper::ValueOf(str);
      ParseState |= (0x01 << 2);
    } else if (ElementToParse->ValueStr() == "Size") {
      // read in format "image_width [image_height [image_depth
      // [image_array_size [image_row_pitch [image_slice_pitch] ] ] ] ]"
      std::stringstream ss(ElementToParse->GetText());
      ss >> imageWidth;
      ss >> imageHeight;
      ss >> imageDepth;
      ss >> imageArraySize;
      ss >> imageRowPitch;
      ss >> imageSlicePitch;
      ParseState |= (0x01 << 3);
    }
  }
  if (ParseState != 0x0F)
    throw Exception::IOError(
        "[ImageRandomGeneratorConfig]Not all required fields were provided");

  ImageSize.Init(ImageType, imageWidth, imageHeight, imageDepth, imageRowPitch,
                 imageSlicePitch, imageArraySize);
  this->m_ObjDesc =
      new ImageDesc(ImageType, ImageSize, ChannelDataType, ChannelOrder, false);

  return false;
}

#define BUFFERCONSTGENERATORCONFIG_FACTORY(Ty)                                 \
  else if (name == BufferConstGeneratorConfig<Ty>::getStaticName()) res =      \
      new BufferConstGeneratorConfig<Ty>;
#define BUFFERRANDOMGENERATORCONFIG_FACTORY(Ty)                                \
  else if (name == BufferRandomGeneratorConfig<Ty>::getStaticName()) res =     \
      new BufferRandomGeneratorConfig<Ty>;

AbstractGeneratorConfig *
GeneratorConfigFactory::create(const std::string &name) {
  AbstractGeneratorConfig *res = 0;
  if (name == BufferConstGeneratorConfig<float>::getStaticName()) {
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

  else if (name == BufferStructureGeneratorConfig::getStaticName()) {
    res = new BufferStructureGeneratorConfig();
  }
  else if (name == ImageRandomGeneratorConfig::getStaticName()) {
    res = new ImageRandomGeneratorConfig();
  }
  else {
    throw Exception::GeneratorBadTypeException(
        "[GeneratorConfigFactory::create] bad config name - " + name);
  }
  return res;
}
#undef BUFFERCONSTGENERATORCONFIG_FACTORY
#undef BUFFERRANDOMGENERATORCONFIG_FACTORY

OCLKernelDataGeneratorConfig *OCLKernelDataGeneratorConfig::defaultConfig(
    const OCLKernelArgumentsList &args) {
  IMemoryObjectDesc *argDesc;

  OCLKernelArgumentsList::const_iterator c_it;
  OCLKernelDataGeneratorConfig *result = new OCLKernelDataGeneratorConfig();

  for (c_it = args.begin(); c_it != args.end(); ++c_it) {
    argDesc = (*c_it).get();

    if (argDesc->GetName() == ImageDesc::GetImageDescName()) {
      throw Exception::NotImplemented(
          "[OCLKernelDataGeneratorConfig::defaultConfig] Default configs for "
          "images are not supported");
    } else if (argDesc->GetName() == BufferDesc::GetBufferDescName()) {
      TypeDesc elemDesc =
          static_cast<BufferDesc *>(argDesc)->GetElementDescription();
      result->getConfigVector().push_back(defaultConfig(elemDesc));
    }
  }
  return result;
}

#define DEFAULTCONFIGMACROS(TyVal, Ty)                                         \
  case TyVal: {                                                                \
    ret = GeneratorConfigFactory::create(                                      \
        BufferRandomGeneratorConfig<Ty>::getStaticName());                     \
    break;                                                                     \
  }

AbstractGeneratorConfig *
OCLKernelDataGeneratorConfig::defaultConfig(const TypeDesc &Ty) {
  TypeDesc subElem;
  AbstractGeneratorConfig *ret;
  switch (Ty.GetType()) {
    // scalar data types
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
    // aggregate data types
  case TARRAY:
  case TPOINTER:
  case TVECTOR: {
    subElem = Ty.GetSubTypeDesc(0);
    ret = defaultConfig(subElem);
    break;
  }

  case TSTRUCT: {
    uint64_t n_elems = Ty.GetNumOfSubTypes();
    BufferStructureGeneratorConfig *structCFG =
        new BufferStructureGeneratorConfig;
    for (uint64_t i = 0; i < n_elems; ++i) {
      structCFG->getConfigVector().push_back(
          defaultConfig(Ty.GetSubTypeDesc(i)));
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
