/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __DEMANGLE_PARSER_H__
#define __DEMANGLE_PARSER_H__

#include "ParameterType.h"
#include "FunctionDescriptor.h"
#include <map>
#include <vector>

namespace reflection {

  struct TypeInfo;
  class DemangleParser {
  public:
    // @brief Constructor
    // @param parameters type vector to store demangled parameters
    DemangleParser(TypeVector& parameters);

    // @brief Destructor
    ~DemangleParser();

    // @brief parse string that represents mangled parameters
    //        store demangled parameters in type vector given to this
    //        class upon creation.
    // @param mangledString mangled parameter string to demangle
    // @return true if succeeded, false otherwise
    bool demangle(const char* mangledString);

  private:
    // @brief Default Constructor
    DemangleParser();

    // @brief Set error status to true
    //        reaching this function means demangle parsing failed!
    void setError();

    // @brief Parse and get next Type
    // @return Next demangled type
    RefParamType getNextType();

    // @brief Parse and get primitive Type
    // @param primitiveType identifier of parsed primitive type
    // @return primitive demangled type
    RefParamType createPrimitiveType(TypePrimitiveEnum primitiveType);

    // @brief Parse and get pointer Type
    // @return pointer demangled type
    RefParamType createPointerType();

    // @brief Parse and get vector Type
    // @return vector demangled type
    RefParamType createVectorType();

    // @brief Parse and get atomics Type
    // @return atomic demangled type
    RefParamType createAtomicType();

    // @brief Parse and get block Type
    // @return block demangled type
    RefParamType createBlockType();

    // @brief Parse and get user-defined Type
    // @param userDefinedNameLength length of user-defined type name
    // @return user-defined demangled type
    RefParamType createUserDefinedType(unsigned int userDefinedNameLength);

    // @brief Parse next token as number
    // @param num [OUT] parsed number
    // @return return true is succeeded, false otherwise
    bool getNextNumber(unsigned int& num);

    // @brief Parse duplication and get duplication number
    // @param num [OUT] parsed number
    // @return return true is succeeded, false otherwise
    bool getDuplicationSNumber(unsigned int& num);

    // @brief Parse next token as address space
    // @param attrAddressSpace [OUT] parsed address space
    // @return return true is succeeded, false otherwise
    bool getAddressSpace(TypeAttributeEnum& attrAddressSpace);

    // @brief Parse next token as address qualifier
    // @param attrQualifier [OUT] parsed address qualifier
    // @return return true is succeeded, false otherwise
    std::vector<TypeAttributeEnum> getAddressQualifiers();

    // @brief Match next token according to given type info.
    //        Also increment current index of mangled string accordingly.
    // @param pTypeInfo type mangling information (token, length, etc.)
    // @param primitiveType[OUT] set primitive type in this entry if it is not NULL.
    // @return return true is succeeded, false otherwise
    bool match(TypeInfo* pTypeInfo, TypePrimitiveEnum* pPrimitiveType = NULL);

  private:
    // @brief holder for mangled parameter types (output data)
    TypeVector& m_parameters;
    // @brief holder for mangled non-primitive types (intermediate data)
    TypeVector m_signList;
    // @brief holder for mangled string (intermediate data)
    const char* m_pMangledString;
    // @brief holder for current index in mangled string (intermediate data)
    unsigned int m_currentIndex;
    // @brief holder for length of mangled string (intermediate data)
    unsigned int m_mangledStringLength;
    // @brief holder for error status (output data)
    bool m_error;

    // translation table between API names and CLANG-mangled names
    std::map<std::string, TypePrimitiveEnum> m_imageTypeNameTranslate;
  };

} // namespace reflection {

#endif //__DEMANGLE_PARSER_H__
