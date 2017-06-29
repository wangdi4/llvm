/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "DemangleParser.h"
#include "Utils.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string>

namespace reflection {

  #define MAX_TOKEN_LENGTH (64)
  struct TypeInfo {
    char m_token[MAX_TOKEN_LENGTH];
    unsigned int m_tokenLength;
    TypePrimitiveEnum m_primitiveType;
    TypeInfo** m_pTypeInfoMap;
  };

  #define INIT_PRIMITIVE_TYPE_INFO_TOKEN(varName, token, id, typeInfoMap) \
    TypeInfo varName = {token, (unsigned int)strlen(token), id, typeInfoMap}

  #define INIT_TYPE_INFO_TOKEN(varName, token) \
    TypeInfo varName = {token, (unsigned int)strlen(token), PRIMITIVE_NONE, NULL}

  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_b,  "b", PRIMITIVE_BOOL,   NULL);
  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_h,  "h", PRIMITIVE_UCHAR,  NULL);
  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_c,  "c", PRIMITIVE_CHAR,   NULL);
  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_t,  "t", PRIMITIVE_USHORT, NULL);
  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_s,  "s", PRIMITIVE_SHORT,  NULL);
  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_j,  "j", PRIMITIVE_UINT,   NULL);
  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_i,  "i", PRIMITIVE_INT,    NULL);
  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_m,  "m", PRIMITIVE_ULONG,  NULL);
  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_l,  "l", PRIMITIVE_LONG,   NULL);
  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_f,  "f", PRIMITIVE_FLOAT,  NULL);
  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_d,  "d", PRIMITIVE_DOUBLE, NULL);
  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_v,  "v", PRIMITIVE_VOID,   NULL);
  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_z,  "z", PRIMITIVE_VAR_ARG,NULL);

  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_Dh, "h", PRIMITIVE_HALF,   NULL);

  //This maps letters from 'a' to 'z'
  static TypeInfo* g_primitiveTypeInfoMapLower_D[] = {
    NULL /*'a'*/, NULL  /*'b'*/, NULL /*'c'*/, NULL /*'d'*/, NULL /*'e'*/, NULL /*'f'*/,
    NULL /*'g'*/, &g_Dh /*'h'*/, NULL /*'i'*/, NULL /*'j'*/, NULL /*'k'*/, NULL /*'l'*/,
    NULL /*'m'*/, NULL  /*'n'*/, NULL /*'o'*/, NULL /*'p'*/, NULL /*'q'*/, NULL /*'r'*/,
    NULL /*'s'*/, NULL  /*'t'*/, NULL /*'u'*/, NULL /*'v'*/, NULL /*'w'*/, NULL /*'x'*/,
    NULL /*'y'*/, NULL  /*'z'*/};

  INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_D,  "D", PRIMITIVE_NONE,   g_primitiveTypeInfoMapLower_D);

  //This maps letters from 'a' to 'z'
  static TypeInfo* g_primitiveTypeInfoMapLower[] = {
    NULL /*'a'*/, &g_b /*'b'*/, &g_c /*'c'*/, &g_d /*'d'*/, NULL /*'e'*/, &g_f /*'f'*/,
    NULL /*'g'*/, &g_h /*'h'*/, &g_i /*'i'*/, &g_j /*'j'*/, NULL /*'k'*/, &g_l /*'l'*/,
    &g_m /*'m'*/, NULL /*'n'*/, NULL /*'o'*/, NULL /*'p'*/, NULL /*'q'*/, NULL /*'r'*/,
    &g_s /*'s'*/, &g_t /*'t'*/, NULL /*'u'*/, &g_v /*'v'*/, NULL /*'w'*/, NULL /*'x'*/,
    NULL /*'y'*/, &g_z /*'z'*/};

  //This maps letters from 'A' to 'Z'
  static TypeInfo* g_primitiveTypeInfoMapUpper[] = {
    NULL /*'A'*/, NULL /*'B'*/, NULL /*'C'*/, &g_D /*'D'*/, NULL /*'E'*/, NULL /*'F'*/,
    NULL /*'G'*/, NULL /*'H'*/, NULL /*'I'*/, NULL /*'J'*/, NULL /*'K'*/, NULL /*'L'*/,
    NULL /*'M'*/, NULL /*'N'*/, NULL /*'O'*/, NULL /*'P'*/, NULL /*'Q'*/, NULL /*'R'*/,
    NULL /*'S'*/, NULL /*'T'*/, NULL /*'U'*/, NULL /*'V'*/, NULL /*'W'*/, NULL /*'X'*/,
    NULL /*'Y'*/, NULL /*'Z'*/};


  struct QualifierInfo {
    char m_token[MAX_TOKEN_LENGTH];
    TypeAttributeEnum m_attribute;
  };

  static QualifierInfo g_qualifierInfoList[] = {
    {"r", ATTR_RESTRICT},
    {"V", ATTR_VOLATILE},
    {"K", ATTR_CONST}
  };
  #define QUALIFIER_INFO_LIST_LENGTH (sizeof(g_qualifierInfoList)/sizeof(QualifierInfo))

  INIT_TYPE_INFO_TOKEN(g_ADDRESS_SPACE_PREFIX, "U");
  INIT_TYPE_INFO_TOKEN(g_ADDRESS_SPACE_SUFFIX, "AS");
  INIT_TYPE_INFO_TOKEN(g_PARAM_DUP_PREFIX,     "S");
  INIT_TYPE_INFO_TOKEN(g_PARAM_DUP_SUFFIX,     "_");
  INIT_TYPE_INFO_TOKEN(g_VECTOR_SUFFIX,        "_");
  //Currently supports only blcks that returns void
  INIT_TYPE_INFO_TOKEN(g_BLOCK_PREFIX,         "Fv");
  INIT_TYPE_INFO_TOKEN(g_BLOCK_SUFFIX,         "E");

  INIT_TYPE_INFO_TOKEN(g_Pointer, "P");
  INIT_TYPE_INFO_TOKEN(g_Vector, "Dv");
  INIT_TYPE_INFO_TOKEN(g_Block, "U13block_pointer");
  INIT_TYPE_INFO_TOKEN(g_Atomic, "U7_Atomic");

  DemangleParser::DemangleParser(TypeVector& parameters)
    : m_parameters(parameters), m_currentIndex(0), m_error(false) {
      m_imageTypeNameTranslate["ocl_image1d"] = PRIMITIVE_IMAGE_1D_T;
      m_imageTypeNameTranslate["ocl_image1d_ro"] = PRIMITIVE_IMAGE_1D_RO_T;
      m_imageTypeNameTranslate["ocl_image1d_wo"] = PRIMITIVE_IMAGE_1D_WO_T;
      m_imageTypeNameTranslate["ocl_image1d_rw"] = PRIMITIVE_IMAGE_1D_RW_T;
      m_imageTypeNameTranslate["ocl_image2d"] = PRIMITIVE_IMAGE_2D_T;
      m_imageTypeNameTranslate["ocl_image2d_ro"] = PRIMITIVE_IMAGE_2D_RO_T;
      m_imageTypeNameTranslate["ocl_image2d_wo"] = PRIMITIVE_IMAGE_2D_WO_T;
      m_imageTypeNameTranslate["ocl_image2d_rw"] = PRIMITIVE_IMAGE_2D_RW_T;
      m_imageTypeNameTranslate["ocl_image2ddepth"] = PRIMITIVE_IMAGE_2D_DEPTH_T;
      m_imageTypeNameTranslate["ocl_image2d_depth_ro"] = PRIMITIVE_IMAGE_2D_DEPTH_RO_T;
      m_imageTypeNameTranslate["ocl_image2d_depth_wo"] = PRIMITIVE_IMAGE_2D_DEPTH_WO_T;
      m_imageTypeNameTranslate["ocl_image2d_depth_rw"] = PRIMITIVE_IMAGE_2D_DEPTH_RW_T;
      m_imageTypeNameTranslate["ocl_image3d"] = PRIMITIVE_IMAGE_3D_T;
      m_imageTypeNameTranslate["ocl_image3d_ro"] = PRIMITIVE_IMAGE_3D_RO_T;
      m_imageTypeNameTranslate["ocl_image3d_wo"] = PRIMITIVE_IMAGE_3D_WO_T;
      m_imageTypeNameTranslate["ocl_image3d_rw"] = PRIMITIVE_IMAGE_3D_RW_T;
      m_imageTypeNameTranslate["ocl_image1dbuffer"] = PRIMITIVE_IMAGE_1D_BUFFER_T;
      m_imageTypeNameTranslate["ocl_image1d_buffer_ro"] = PRIMITIVE_IMAGE_1D_BUFFER_RO_T;
      m_imageTypeNameTranslate["ocl_image1d_buffer_wo"] = PRIMITIVE_IMAGE_1D_BUFFER_WO_T;
      m_imageTypeNameTranslate["ocl_image1d_buffer_rw"] = PRIMITIVE_IMAGE_1D_BUFFER_RW_T;
      m_imageTypeNameTranslate["ocl_image1darray"] = PRIMITIVE_IMAGE_1D_ARRAY_T;
      m_imageTypeNameTranslate["ocl_image1d_array_ro"] = PRIMITIVE_IMAGE_1D_ARRAY_RO_T;
      m_imageTypeNameTranslate["ocl_image1d_array_wo"] = PRIMITIVE_IMAGE_1D_ARRAY_WO_T;
      m_imageTypeNameTranslate["ocl_image1d_array_rw"] = PRIMITIVE_IMAGE_1D_ARRAY_RW_T;
      m_imageTypeNameTranslate["ocl_image2darray"] = PRIMITIVE_IMAGE_2D_ARRAY_T;
      m_imageTypeNameTranslate["ocl_image2d_array_ro"] = PRIMITIVE_IMAGE_2D_ARRAY_RO_T;
      m_imageTypeNameTranslate["ocl_image2d_array_wo"] = PRIMITIVE_IMAGE_2D_ARRAY_WO_T;
      m_imageTypeNameTranslate["ocl_image2d_array_rw"] = PRIMITIVE_IMAGE_2D_ARRAY_RW_T;
      m_imageTypeNameTranslate["ocl_image2darraydepth"] = PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T;
      m_imageTypeNameTranslate["ocl_image2d_array_depth_ro"] = PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_RO_T;
      m_imageTypeNameTranslate["ocl_image2d_array_depth_wo"] = PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_WO_T;
      m_imageTypeNameTranslate["ocl_image2d_array_depth_rw"] = PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_RW_T;
      m_imageTypeNameTranslate["ocl_event"] = PRIMITIVE_EVENT_T;
      m_imageTypeNameTranslate["ocl_clkevent"] = PRIMITIVE_CLK_EVENT_T;
      m_imageTypeNameTranslate["ocl_queue"] = PRIMITIVE_QUEUE_T;
      m_imageTypeNameTranslate["ocl_pipe"] = PRIMITIVE_PIPE_T;
      m_imageTypeNameTranslate["ocl_sampler"] = PRIMITIVE_SAMPLER_T;
  }

  DemangleParser::~DemangleParser() {
  }

  bool DemangleParser::demangle(const char* mangledString) {
    if (!mangledString) return false;

    m_pMangledString = mangledString;
    m_currentIndex = 0;
    m_mangledStringLength = strlen(mangledString);
    for (RefParamType type = getNextType(); !type.isNull(); type = getNextType()) {
      m_parameters.push_back(type);
    }
    //Check for success and terminate accordingly
    return (!m_error);
  }

  void DemangleParser::setError() {
    //assert(false && "Error during parsing");
    m_error = true;
  }

  bool DemangleParser::match(TypeInfo* pTypeInfo, TypePrimitiveEnum *pPrimitiveType) {
    if (!pTypeInfo) return false;
    if ((m_mangledStringLength - m_currentIndex) < pTypeInfo->m_tokenLength) return false;
    //m_tokenLength is usually small (mostly 1),
    //what justifies the loop rather than strncmp (for example).
    for (unsigned int i=0; i < pTypeInfo->m_tokenLength; ++i) {
      if (m_pMangledString[m_currentIndex+i] != pTypeInfo->m_token[i]) return false;
    }
    //Found a type, increment current index & set Primitive
    m_currentIndex += pTypeInfo->m_tokenLength;
    if(pPrimitiveType) *pPrimitiveType = pTypeInfo->m_primitiveType;
    //Check if this TypeInfo is not a complete type
    if(pTypeInfo->m_pTypeInfoMap) {
      //We only match a prefix of the type, need to check the suffix
      const char ch = m_pMangledString[m_currentIndex];
      TypeInfo* pSuffixTypeInfo = NULL;
      if (ch >= 'a' && ch <= 'z') {
        //Currently assume only lower case letters as suffix
        pSuffixTypeInfo = pTypeInfo->m_pTypeInfoMap[(int)(ch - 'a')];
      }
      if (!match(pSuffixTypeInfo, pPrimitiveType)) {
        //Failed to match suffix, need to revert changes and return false
        m_currentIndex -= pTypeInfo->m_tokenLength;
        return false;
      }
    }
    return true;
  }

  RefParamType DemangleParser::getNextType() {
    if (m_currentIndex == m_mangledStringLength) {
      //No more types to read, return an empty RefParamType,
      //that is equal to NULL value, to indecate end of parsing.
      return RefParamType();
    }

    // Check if next type is a primitive
    const char ch = m_pMangledString[m_currentIndex];
    if (ch >= 'a' && ch <= 'z') {
      TypeInfo* pPrimitiveTypeInfo = g_primitiveTypeInfoMapLower[(int)(ch - 'a')];
      TypePrimitiveEnum primitiveType = PRIMITIVE_NONE;
      if (match(pPrimitiveTypeInfo, &primitiveType)) {
        return createPrimitiveType(primitiveType);
      }
    } else if (ch >= 'A' && ch <= 'Z') {
      TypeInfo* pPrimitiveTypeInfo = g_primitiveTypeInfoMapUpper[(int)(ch - 'A')];
      TypePrimitiveEnum primitiveType = PRIMITIVE_NONE;
      if (match(pPrimitiveTypeInfo, &primitiveType)) {
        return createPrimitiveType(primitiveType);
      }
    }
    // Reaching here only if next type is not primitive
    // Check if if next type is a vector
    if (match(&g_Vector)) {
      return createVectorType();
    }
    // Check if if next type is a pointer
    if (match(&g_Pointer)) {
      return createPointerType();
    }
    // Check if if next type is an atomic
    if (match(&g_Atomic)) {
      return createAtomicType();
    }
    // Check if if next type is a block
    if (match(&g_Block)) {
      return createBlockType();
    }

    // Check if if next type is a duplicate type
    if (match(&g_PARAM_DUP_PREFIX)) {
      // duplication supports only S_ and S#_ where # is one letter [0-9]|[A-Z]
      unsigned int dupIndex = 0;
      if (!getDuplicationSNumber(dupIndex)) {
        // Reachning here is not expected, set error and return NULL;
        setError();
        return RefParamType();
      }
      if (dupIndex >= m_signList.size()) {
        // Reachning here is not expected, set error and return NULL;
        setError();
        return RefParamType();
      }
      return m_signList[dupIndex];
    }
    // Reaching here means we have a user-defined type
    unsigned int userDefinedNameLength = 0;
    if (!getNextNumber(userDefinedNameLength)) {
      // Reachning here is not expected, set error and return NULL;
      setError();
      return RefParamType();
    }
    return createUserDefinedType(userDefinedNameLength);
  }

  RefParamType DemangleParser::createPrimitiveType(TypePrimitiveEnum primitiveType) {
    return new PrimitiveType(primitiveType);
  }

  RefParamType DemangleParser::createPointerType() {
    // Try to parse CVR-qualifier first to support SPIR1.2 mangling scheme
    std::vector<TypeAttributeEnum> AttrQualifiers = getAddressQualifiers();

    // Try to parse Address Space qualifiers
    TypeAttributeEnum AttrAddressSpace;
    if (!getAddressSpace(AttrAddressSpace)) {
      // Reachning means parsing Address Space failed, return NULL;
      return RefParamType();
    }
    // Try to parse CVR-qualifier one more time since
    // Clang 4.0 mangling scheme assumes CV-qualifiers *after*
    // vendor extensions (i.e. address space qualifiers in OpenCL case).
    if (AttrQualifiers.empty()) {
      AttrQualifiers = getAddressQualifiers();
    }

    RefParamType pType = getNextType();
    if (pType.isNull()) {
      // Reachning here is not expected, set error and return NULL;
      setError();
      return RefParamType();
    }
    PointerType* pPointer = new PointerType(pType);
    //Push pointer type to end of sign list
    //It is important to do this after parsing the pointee type
    //in case the pointee is a non-primitive type, it should
    //be pushed first to the sign list.
    m_signList.push_back(new PointerType(*pPointer));

    assert(AttrAddressSpace != ATTR_NONE && "No addr space.");
    if (AttrAddressSpace != ATTR_PRIVATE)
      pPointer->addAttribute(AttrAddressSpace);

    for (auto &Attr : AttrQualifiers)
      pPointer->addAttribute(Attr);

    RefParamType refPointer(pPointer);
    if (!AttrQualifiers.empty() || AttrAddressSpace != ATTR_PRIVATE)
      m_signList.push_back(new PointerType(*pPointer));
    return refPointer;
  }

  RefParamType DemangleParser::createVectorType() {
    unsigned int vectorLength = 0;
    if (!getNextNumber(vectorLength)) {
      // Reachning here is not expected, set error and return NULL;
      setError();
      return RefParamType();
    }
    if (!match(&g_VECTOR_SUFFIX)) {
      // Reachning here is not expected, set error and return NULL;
      setError();
      return RefParamType();
    }
    //TODO: vector allowed only on primitive type.
    //      This implementation does not fail if this asumption does not hold.
    RefParamType pPrimitiveType = getNextType();
    if (pPrimitiveType.isNull()) {
      // Reachning here is not expected, set error and return NULL;
      setError();
      return RefParamType();
    }
    ParamType* pVector = new VectorType(pPrimitiveType, vectorLength);
    RefParamType refVector(pVector);
    //Push vector type to end of sign list
    //It is important to do this after parsing the scalar type
    //in case the scalar is a non-primitive type, it should
    //be pushed first to the sign list.
    m_signList.push_back(refVector);
    return refVector;
  }

  RefParamType DemangleParser::createAtomicType() {
    RefParamType pBaseParamType = getNextType();
    if (pBaseParamType.isNull()) {
      // Reachning here is not expected, set error and return NULL;
      setError();
      return RefParamType();
    }
    //Atomic is supported only on primitive types
    if(!dyn_cast<PrimitiveType>(pBaseParamType)) {
      // Reachning here is not expected, set error and return NULL;
      setError();
      return RefParamType();
    }
    AtomicType* pAtomicType = new AtomicType(pBaseParamType);
    RefParamType refAtomic(pAtomicType);

    //Push atomic type to end of sign list
    m_signList.push_back(refAtomic);
    return refAtomic;
  }

  RefParamType DemangleParser::createBlockType() {
    if (!match(&g_BLOCK_PREFIX)) {
      // Reachning here is not expected, set error and return NULL;
      setError();
      return RefParamType();
    }
    BlockType* pBlockType = new BlockType();
    RefParamType refBlock(pBlockType);
    int index = 0;
    while(!match(&g_BLOCK_SUFFIX)) {
      RefParamType pBlockParamType = getNextType();
      if (pBlockParamType.isNull()) {
        // Reachning here is not expected, set error and return NULL;
        setError();
        return RefParamType();
      }
      pBlockType->setParam(index++, pBlockParamType);
    }
    //Push block type to end of sign list
    //It is important to do this after parsing the block parameter types
    //in case some of the parameters are non-primitive type, it should
    //be pushed first to the sign list.
    m_signList.push_back(refBlock);
    return refBlock;
  }

  RefParamType DemangleParser::createUserDefinedType(unsigned int userDefinedNameLength) {
    if (m_mangledStringLength - m_currentIndex < userDefinedNameLength ) {
      // Reachning here is not expected, set error and return NULL;
      setError();
      return RefParamType();
    }
    std::string typeName(&m_pMangledString[m_currentIndex], userDefinedNameLength);
    //Found a type, increment current index
    m_currentIndex += userDefinedNameLength;

    std::map<std::string, TypePrimitiveEnum>::iterator itr = m_imageTypeNameTranslate.find(typeName);
    if (itr != m_imageTypeNameTranslate.end()) {
      // We have a special Primitive Type
#ifdef SUBSTITUTE_OPENCL_TYPES
      RefParamType refImageType = createPrimitiveType(itr->second);
      m_signList.push_back(refImageType);
      return refImageType;
#else
      return createPrimitiveType(itr->second);
#endif
    }

    ParamType* pUserDefined = new UserDefinedType(typeName);
    RefParamType refUserDefined(pUserDefined);
    //Push user-defined type to end of sign list
    m_signList.push_back(refUserDefined);
    return refUserDefined;
  }

  bool DemangleParser::getNextNumber(unsigned int& num) {
    const char *pStart = &m_pMangledString[m_currentIndex];
    char *pEnd;
    num = (unsigned int)strtol(pStart, &pEnd, 10);
    if (pStart == pEnd) return false;
    //Found a number, increment current index
    m_currentIndex += (pEnd - pStart);
    return true;
  }

  bool DemangleParser::getDuplicationSNumber(unsigned int& num) {
    if (match(&g_PARAM_DUP_SUFFIX)) {
      num = 0;
      return true;
    }
    // Reaching here means there is a duplicate number
    if (m_currentIndex == m_mangledStringLength) {
      // Expecting at least one character left in demangled string
      return false;
    }
    char dupChar = m_pMangledString[m_currentIndex];
    if (dupChar >= '0' && dupChar <= '9') {
      num = 1 + (unsigned int) (dupChar - '0');
    } else if (dupChar >= 'A' && dupChar <= 'Z') {
      num = 11 + (unsigned int) (dupChar - 'A');
    } else {
      setError();
      return false;
    }
    //Found a duplicate number, increment current index
    m_currentIndex++;
    if (match(&g_PARAM_DUP_SUFFIX)) {
      return true;
    }
    //Reachning here is not expected, set error and return false;
    setError();
    return false;
  }

  bool DemangleParser::getAddressSpace(TypeAttributeEnum& attrAddressSpace) {
    attrAddressSpace = ATTR_NONE;
    //Check if it is an address space
    if (!match(&g_ADDRESS_SPACE_PREFIX)) {
      attrAddressSpace = ATTR_PRIVATE;
      return true;
    }
    unsigned int addressSpaceLength = 0;
    if (!getNextNumber(addressSpaceLength) ||
        (m_mangledStringLength - m_currentIndex) < addressSpaceLength ||
        addressSpaceLength < g_ADDRESS_SPACE_SUFFIX.m_tokenLength ||
        !match(&g_ADDRESS_SPACE_SUFFIX)) {
      //Reachning here is not expected, set error and return false;
      setError();
      return false;
    }
    addressSpaceLength -= g_ADDRESS_SPACE_SUFFIX.m_tokenLength;
    std::string addressSpaceId(&m_pMangledString[m_currentIndex], addressSpaceLength);
    char *endString;
    int addressSpace = (int) strtol(&addressSpaceId[0], &endString, 10);
    if ((unsigned int)(endString -  &addressSpaceId[0]) != addressSpaceLength) {
      //Reachning here is not expected, set error and return false;
      setError();
      return false;
    }
    //Found an addressSpace, increment current index
    m_currentIndex += addressSpaceLength;
    if (addressSpace >= ATTR_ADDR_SPACE_FIRST && addressSpace <= ATTR_ADDR_SPACE_LAST) {
      attrAddressSpace = (TypeAttributeEnum)addressSpace;
      return true;
    }
    assert(false && "Unsupported address space!");
    setError();
    return false;
  }

  std::vector<TypeAttributeEnum> DemangleParser::getAddressQualifiers() {
    std::vector<TypeAttributeEnum> FoundAttrs;
    // Reached the end of the string
    if (m_currentIndex == m_mangledStringLength)
      return FoundAttrs;

    for (unsigned int i=0; i < QUALIFIER_INFO_LIST_LENGTH; ++i) {
      assert(strlen(g_qualifierInfoList[i].m_token) == 1 &&
        "Assumed Qualifier is one char, failing this force fixing this code!");
      if (g_qualifierInfoList[i].m_token[0] == m_pMangledString[m_currentIndex]) {
        // Found a match, increment current index
        m_currentIndex++;
        FoundAttrs.push_back(g_qualifierInfoList[i].m_attribute);
      }
    }
    return FoundAttrs;
  }

} // namespace reflection
