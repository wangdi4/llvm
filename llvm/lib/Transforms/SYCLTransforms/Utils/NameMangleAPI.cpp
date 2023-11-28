//===- NameMangleAPI.cpp - Name mangle APIs -------------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/Utils/NameMangleAPI.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::reflection;

namespace {
#define MAX_TOKEN_LENGTH (64)
struct TypeInfo {
  char Token[MAX_TOKEN_LENGTH];
  unsigned int TokenLength;
  TypePrimitiveEnum PrimitiveType;
  TypeInfo **TypeInfoMap;
};

class DemangleParser {
public:
  // Constructor.
  // \param Parameters type vector to store demangled parameters.
  DemangleParser(TypeVector &Parameters, bool IsSpir12Name = false);

  // Destructor.
  ~DemangleParser() {}

  // Parse string that represents mangled parameters store demangled parameters
  // in type vector given to this class upon creation.
  // \param mangledString mangled parameter string to demangle.
  // \return true if succeeded, false otherwise.
  bool demangle(StringRef mangledString);

private:
  // Default Constructor.
  DemangleParser();

  // Set error status to true reaching this function means demangle parsing
  // failed!
  void setError();

  // Parse and get next Type.
  // \return Next demangled type.
  RefParamType getNextType();

  // Parse and get primitive Type.
  // \param primitiveType identifier of parsed primitive type.
  // \return primitive demangled type.
  RefParamType createPrimitiveType(TypePrimitiveEnum primitiveType);

  // Parse and get pointer Type.
  // \return pointer demangled type.
  RefParamType createPointerType();

  // Parse and get vector Type.
  // \return vector demangled type.
  RefParamType createVectorType();

  // Parse and get atomics Type.
  // \return atomic demangled type.
  RefParamType createAtomicType();

  // Parse and get block Type.
  // \return block demangled type.
  RefParamType createBlockType();

  // Parse and get user-defined Type.
  // \param userDefinedNameLength length of user-defined type name.
  // \return user-defined demangled type.
  RefParamType createUserDefinedType(unsigned int UserDefinedNameLength);

  // Parse next token as number.
  // \param num [OUT] parsed number.
  // \return return true is succeeded, false otherwise.
  bool getNextNumber(unsigned int &num);

  // Parse duplication and get duplication number.
  // \param num [OUT] parsed number.
  // \return return true is succeeded, false otherwise.
  bool getDuplicationSNumber(unsigned int &num);

  // Parse next token as address space.
  // \param attrAddressSpace [OUT] parsed address space.
  // \return return true is succeeded, false otherwise.
  bool getAddressSpace(TypeAttributeEnum &attrAddressSpace);

  // Parse next token as address qualifier.
  // \param attrQualifier [OUT] parsed address qualifier.
  // \return return true is succeeded, false otherwise.
  std::vector<TypeAttributeEnum> getAddressQualifiers();

  // Match next token according to given type info.
  // Also increment current index of mangled string accordingly.
  // \param pTypeInfo type mangling information (token, length, etc.)
  // \param primitiveType[OUT] set primitive type in this entry if it is not
  // NULL.
  // \return return true is succeeded, false otherwise.
  bool match(TypeInfo *pTypeInfo, TypePrimitiveEnum *pPrimitiveType = NULL);

private:
  /// holder for mangled parameter types (output data).
  TypeVector &Parameters;
  /// holder for mangled non-primitive types (intermediate data).
  TypeVector SignList;
  /// holder for mangled string (intermediate data).
  StringRef MangledString;
  /// holder for current index in mangled string (intermediate data).
  unsigned int CurrentIndex;
  /// holder for length of mangled string (intermediate data).
  size_t MangledStringLength;
  /// holder for error status (output data).
  bool Err;

  /// indicates whether the source name is mangled using SPIR12 rules.
  const bool IsSpir12Name;

  /// translation table between API names and CLANG-mangled names.
  StringMap<TypePrimitiveEnum> ImageTypeNameTranslate;
};

class MangleVisitor : public reflection::TypeVisitor {
public:
  MangleVisitor(raw_string_ostream &S) : OS(S) {}

  void operator()(const reflection::ParamType *t) { t->accept(this); }

  // visit methods
  void visit(const reflection::PrimitiveType *t) override;
  void visit(const reflection::PointerType *p) override;
  void visit(const reflection::VectorType *v) override;
  void visit(const reflection::AtomicType *p) override;
  void visit(const reflection::BlockType *p) override;
  void visit(const reflection::UserDefinedType *pTy) override;

private:
  int getTypeIndex(const reflection::ParamType *t) const;

  // holds the mangled string representing the prototype of the function
  raw_string_ostream &OS;
  // list of types 'seen' so far
  reflection::DuplicatedTypeList DupList;
};

} // namespace

static StringRef PREFIX = "_Z";
static size_t PREFIX_LEN = 2;

// parsing methods for the raw mangled name
static bool peelPrefix(StringRef &s) {
  if (PREFIX != s.substr(0, PREFIX_LEN))
    return false;
  s = s.substr(PREFIX_LEN, s.size() - PREFIX_LEN);
  return true;
}

static StringRef peelNameLen(StringRef &s) {
  int len = 0;
  StringRef::const_iterator it = s.begin();
  while (isDigit(*it++))
    ++len;
  StringRef ret = s.substr(0, len);
  s = s.substr(len, s.size() - len);
  return ret;
}

#define INIT_PRIMITIVE_TYPE_INFO_TOKEN(varName, token, id, typeInfoMap)        \
  static TypeInfo varName = {token, (unsigned int)strlen(token), id,           \
                             typeInfoMap}

#define INIT_TYPE_INFO_TOKEN(varName, token)                                   \
  static TypeInfo varName = {token, (unsigned int)strlen(token),               \
                             PRIMITIVE_NONE, NULL}

INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_b, "b", PRIMITIVE_BOOL, NULL);
INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_h, "h", PRIMITIVE_UCHAR, NULL);
INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_c, "c", PRIMITIVE_CHAR, NULL);
INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_t, "t", PRIMITIVE_USHORT, NULL);
INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_s, "s", PRIMITIVE_SHORT, NULL);
INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_j, "j", PRIMITIVE_UINT, NULL);
INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_i, "i", PRIMITIVE_INT, NULL);
INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_m, "m", PRIMITIVE_ULONG, NULL);
INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_l, "l", PRIMITIVE_LONG, NULL);
INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_f, "f", PRIMITIVE_FLOAT, NULL);
INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_d, "d", PRIMITIVE_DOUBLE, NULL);
INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_v, "v", PRIMITIVE_VOID, NULL);
INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_z, "z", PRIMITIVE_VAR_ARG, NULL);

INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_Dh, "h", PRIMITIVE_HALF, NULL);

// This maps letters from 'a' to 'z'
static TypeInfo *PrimitiveTypeInfoMapLower_D[] = {
    NULL /*'a'*/, NULL /*'b'*/, NULL /*'c'*/,  NULL /*'d'*/, NULL /*'e'*/,
    NULL /*'f'*/, NULL /*'g'*/, &g_Dh /*'h'*/, NULL /*'i'*/, NULL /*'j'*/,
    NULL /*'k'*/, NULL /*'l'*/, NULL /*'m'*/,  NULL /*'n'*/, NULL /*'o'*/,
    NULL /*'p'*/, NULL /*'q'*/, NULL /*'r'*/,  NULL /*'s'*/, NULL /*'t'*/,
    NULL /*'u'*/, NULL /*'v'*/, NULL /*'w'*/,  NULL /*'x'*/, NULL /*'y'*/,
    NULL /*'z'*/};

INIT_PRIMITIVE_TYPE_INFO_TOKEN(g_D, "D", PRIMITIVE_NONE,
                               PrimitiveTypeInfoMapLower_D);

// This maps letters from 'a' to 'z'
static TypeInfo *g_primitiveTypeInfoMapLower[] = {
    NULL /*'a'*/, &g_b /*'b'*/, &g_c /*'c'*/, &g_d /*'d'*/, NULL /*'e'*/,
    &g_f /*'f'*/, NULL /*'g'*/, &g_h /*'h'*/, &g_i /*'i'*/, &g_j /*'j'*/,
    NULL /*'k'*/, &g_l /*'l'*/, &g_m /*'m'*/, NULL /*'n'*/, NULL /*'o'*/,
    NULL /*'p'*/, NULL /*'q'*/, NULL /*'r'*/, &g_s /*'s'*/, &g_t /*'t'*/,
    NULL /*'u'*/, &g_v /*'v'*/, NULL /*'w'*/, NULL /*'x'*/, NULL /*'y'*/,
    &g_z /*'z'*/};

// This maps letters from 'A' to 'Z'
static TypeInfo *g_primitiveTypeInfoMapUpper[] = {
    NULL /*'A'*/, NULL /*'B'*/, NULL /*'C'*/, &g_D /*'D'*/, NULL /*'E'*/,
    NULL /*'F'*/, NULL /*'G'*/, NULL /*'H'*/, NULL /*'I'*/, NULL /*'J'*/,
    NULL /*'K'*/, NULL /*'L'*/, NULL /*'M'*/, NULL /*'N'*/, NULL /*'O'*/,
    NULL /*'P'*/, NULL /*'Q'*/, NULL /*'R'*/, NULL /*'S'*/, NULL /*'T'*/,
    NULL /*'U'*/, NULL /*'V'*/, NULL /*'W'*/, NULL /*'X'*/, NULL /*'Y'*/,
    NULL /*'Z'*/};

struct QualifierInfo {
  char Token[MAX_TOKEN_LENGTH];
  TypeAttributeEnum Attr;
};

static QualifierInfo QualifierInfoList[] = {
    {"r", ATTR_RESTRICT}, {"V", ATTR_VOLATILE}, {"K", ATTR_CONST}};

#define QUALIFIER_INFO_LIST_LENGTH                                             \
  (sizeof(QualifierInfoList) / sizeof(QualifierInfo))

INIT_TYPE_INFO_TOKEN(TI_ADDRESS_SPACE_PREFIX, "U");
INIT_TYPE_INFO_TOKEN(TI_ADDRESS_SPACE_SUFFIX, "AS");
INIT_TYPE_INFO_TOKEN(TI_PARAM_DUP_PREFIX, "S");
INIT_TYPE_INFO_TOKEN(TI_PARAM_DUP_SUFFIX, "_");
INIT_TYPE_INFO_TOKEN(TI_VECTOR_SUFFIX, "_");
// Currently supports only blcks that returns void
INIT_TYPE_INFO_TOKEN(TI_BLOCK_PREFIX, "Fv");
INIT_TYPE_INFO_TOKEN(TI_BLOCK_SUFFIX, "E");

INIT_TYPE_INFO_TOKEN(TI_Pointer, "P");
INIT_TYPE_INFO_TOKEN(TI_Vector, "Dv");
INIT_TYPE_INFO_TOKEN(TI_Block, "U13block_pointer");
INIT_TYPE_INFO_TOKEN(TI_Atomic, "U7_Atomic");

DemangleParser::DemangleParser(TypeVector &Parameters, bool IsSpir12Name)
    : Parameters(Parameters), CurrentIndex(0), MangledStringLength(0),
      Err(false), IsSpir12Name(IsSpir12Name) {
  ImageTypeNameTranslate["ocl_image1d"] = PRIMITIVE_IMAGE_1D_T;
  ImageTypeNameTranslate["ocl_image1d_ro"] = PRIMITIVE_IMAGE_1D_RO_T;
  ImageTypeNameTranslate["ocl_image1d_wo"] = PRIMITIVE_IMAGE_1D_WO_T;
  ImageTypeNameTranslate["ocl_image1d_rw"] = PRIMITIVE_IMAGE_1D_RW_T;
  ImageTypeNameTranslate["ocl_image2d"] = PRIMITIVE_IMAGE_2D_T;
  ImageTypeNameTranslate["ocl_image2d_ro"] = PRIMITIVE_IMAGE_2D_RO_T;
  ImageTypeNameTranslate["ocl_image2d_wo"] = PRIMITIVE_IMAGE_2D_WO_T;
  ImageTypeNameTranslate["ocl_image2d_rw"] = PRIMITIVE_IMAGE_2D_RW_T;
  ImageTypeNameTranslate["ocl_image2ddepth"] = PRIMITIVE_IMAGE_2D_DEPTH_T;
  ImageTypeNameTranslate["ocl_image2d_depth_ro"] =
      PRIMITIVE_IMAGE_2D_DEPTH_RO_T;
  ImageTypeNameTranslate["ocl_image2d_depth_wo"] =
      PRIMITIVE_IMAGE_2D_DEPTH_WO_T;
  ImageTypeNameTranslate["ocl_image2d_depth_rw"] =
      PRIMITIVE_IMAGE_2D_DEPTH_RW_T;
  ImageTypeNameTranslate["ocl_image3d"] = PRIMITIVE_IMAGE_3D_T;
  ImageTypeNameTranslate["ocl_image3d_ro"] = PRIMITIVE_IMAGE_3D_RO_T;
  ImageTypeNameTranslate["ocl_image3d_wo"] = PRIMITIVE_IMAGE_3D_WO_T;
  ImageTypeNameTranslate["ocl_image3d_rw"] = PRIMITIVE_IMAGE_3D_RW_T;
  ImageTypeNameTranslate["ocl_image1dbuffer"] = PRIMITIVE_IMAGE_1D_BUFFER_T;
  ImageTypeNameTranslate["ocl_image1d_buffer_ro"] =
      PRIMITIVE_IMAGE_1D_BUFFER_RO_T;
  ImageTypeNameTranslate["ocl_image1d_buffer_wo"] =
      PRIMITIVE_IMAGE_1D_BUFFER_WO_T;
  ImageTypeNameTranslate["ocl_image1d_buffer_rw"] =
      PRIMITIVE_IMAGE_1D_BUFFER_RW_T;
  ImageTypeNameTranslate["ocl_image1darray"] = PRIMITIVE_IMAGE_1D_ARRAY_T;
  ImageTypeNameTranslate["ocl_image1d_array_ro"] =
      PRIMITIVE_IMAGE_1D_ARRAY_RO_T;
  ImageTypeNameTranslate["ocl_image1d_array_wo"] =
      PRIMITIVE_IMAGE_1D_ARRAY_WO_T;
  ImageTypeNameTranslate["ocl_image1d_array_rw"] =
      PRIMITIVE_IMAGE_1D_ARRAY_RW_T;
  ImageTypeNameTranslate["ocl_image2darray"] = PRIMITIVE_IMAGE_2D_ARRAY_T;
  ImageTypeNameTranslate["ocl_image2d_array_ro"] =
      PRIMITIVE_IMAGE_2D_ARRAY_RO_T;
  ImageTypeNameTranslate["ocl_image2d_array_wo"] =
      PRIMITIVE_IMAGE_2D_ARRAY_WO_T;
  ImageTypeNameTranslate["ocl_image2d_array_rw"] =
      PRIMITIVE_IMAGE_2D_ARRAY_RW_T;
  ImageTypeNameTranslate["ocl_image2darraydepth"] =
      PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T;
  ImageTypeNameTranslate["ocl_image2d_array_depth_ro"] =
      PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_RO_T;
  ImageTypeNameTranslate["ocl_image2d_array_depth_wo"] =
      PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_WO_T;
  ImageTypeNameTranslate["ocl_image2d_array_depth_rw"] =
      PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_RW_T;
  ImageTypeNameTranslate["ocl_event"] = PRIMITIVE_EVENT_T;
  ImageTypeNameTranslate["ocl_clkevent"] = PRIMITIVE_CLK_EVENT_T;
  ImageTypeNameTranslate["ocl_queue"] = PRIMITIVE_QUEUE_T;
  ImageTypeNameTranslate["ocl_pipe_ro"] = PRIMITIVE_PIPE_RO_T;
  ImageTypeNameTranslate["ocl_pipe_wo"] = PRIMITIVE_PIPE_WO_T;
  ImageTypeNameTranslate["memory_order"] = PRIMITIVE_MEMORY_ORDER;
  ImageTypeNameTranslate["memory_scope"] = PRIMITIVE_MEMORY_SCOPE;
  ImageTypeNameTranslate["ocl_sampler"] = PRIMITIVE_SAMPLER_T;
}

bool DemangleParser::demangle(StringRef mangledString) {
  if (mangledString.empty())
    return false;

  MangledString = mangledString;
  CurrentIndex = 0;
  MangledStringLength = mangledString.size();
  for (RefParamType type = getNextType(); type; type = getNextType()) {
    Parameters.push_back(type);
  }
  // Check for success and terminate accordingly
  return (!Err);
}

void DemangleParser::setError() {
  // assert(false && "Error during parsing");
  Err = true;
}

bool DemangleParser::match(TypeInfo *TyInfo, TypePrimitiveEnum *PrimitiveType) {
  if (!TyInfo)
    return false;
  if ((MangledStringLength - CurrentIndex) < TyInfo->TokenLength)
    return false;
  // TokenLength is usually small (mostly 1),
  // what justifies the loop rather than strncmp (for example).
  for (unsigned int i = 0; i < TyInfo->TokenLength; ++i) {
    if (MangledString[CurrentIndex + i] != TyInfo->Token[i])
      return false;
  }
  // Found a type, increment current index & set Primitive
  CurrentIndex += TyInfo->TokenLength;
  if (PrimitiveType)
    *PrimitiveType = TyInfo->PrimitiveType;
  // Check if this TypeInfo is not a complete type
  if (TyInfo->TypeInfoMap) {
    // We only match a prefix of the type, need to check the suffix
    const char ch = MangledString[CurrentIndex];
    TypeInfo *pSuffixTypeInfo = NULL;
    if (ch >= 'a' && ch <= 'z') {
      // Currently assume only lower case letters as suffix
      pSuffixTypeInfo = TyInfo->TypeInfoMap[(int)(ch - 'a')];
    }
    if (!match(pSuffixTypeInfo, PrimitiveType)) {
      // Failed to match suffix, need to revert changes and return false
      CurrentIndex -= TyInfo->TokenLength;
      return false;
    }
  }
  return true;
}

RefParamType DemangleParser::getNextType() {
  if (CurrentIndex == MangledStringLength) {
    // No more types to read, return an empty RefParamType,
    // that is equal to NULL value, to indecate end of parsing.
    return RefParamType();
  }

  // Check if next type is a primitive
  const char ch = MangledString[CurrentIndex];
  if (ch >= 'a' && ch <= 'z') {
    TypeInfo *pPrimitiveTypeInfo = g_primitiveTypeInfoMapLower[(int)(ch - 'a')];
    TypePrimitiveEnum primitiveType = PRIMITIVE_NONE;
    if (match(pPrimitiveTypeInfo, &primitiveType)) {
      return createPrimitiveType(primitiveType);
    }
  } else if (ch >= 'A' && ch <= 'Z') {
    TypeInfo *pPrimitiveTypeInfo = g_primitiveTypeInfoMapUpper[(int)(ch - 'A')];
    TypePrimitiveEnum primitiveType = PRIMITIVE_NONE;
    if (match(pPrimitiveTypeInfo, &primitiveType)) {
      return createPrimitiveType(primitiveType);
    }
  }
  // Reaching here only if next type is not primitive
  // Check if if next type is a vector
  if (match(&TI_Vector)) {
    return createVectorType();
  }
  // Check if if next type is a pointer
  if (match(&TI_Pointer)) {
    return createPointerType();
  }
  // Check if if next type is an atomic
  if (match(&TI_Atomic)) {
    return createAtomicType();
  }
  // Check if if next type is a block
  if (match(&TI_Block)) {
    return createBlockType();
  }

  // Check if if next type is a duplicate type
  if (match(&TI_PARAM_DUP_PREFIX)) {
    // duplication supports only S_ and S#_ where # is one letter [0-9]|[A-Z]
    unsigned int dupIndex = 0;
    if (!getDuplicationSNumber(dupIndex)) {
      // Reachning here is not expected, set error and return NULL;
      setError();
      return RefParamType();
    }
    if (dupIndex >= SignList.size()) {
      // Reachning here is not expected, set error and return NULL;
      setError();
      return RefParamType();
    }
    return SignList[dupIndex];
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

RefParamType
DemangleParser::createPrimitiveType(TypePrimitiveEnum primitiveType) {
  return RefParamType(new PrimitiveType(primitiveType));
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
  if (!pType) {
    // Reachning here is not expected, set error and return NULL;
    setError();
    return RefParamType();
  }

  assert(AttrAddressSpace != ATTR_NONE && "No addr space.");

  // All pointee types are qualified due to address spaces, so we always need
  // to add them as substitution candidates.
  //
  // TODO: implement correct way to handle qualified pointees
  // When we mangle/demangle a type like "const clk_event_t *"
  // we should treat as substitution candidates 3 types:
  // clk_event_t : base type
  // const clk_event_t : qualified base type (all qualifiers should be here)
  // const clk_event_t * : full pointer type
  // Now we can't properly handle "const clk_event_t" type due to only
  // object of PointerType keeps qualifiers. To preserve correct substitution
  // order we add dummy value to substitution list because we don't have any
  // cases in OpenCL now when "const clk_event_t" can be used, but it is still
  // wrong way to handle this situation
  SignList.push_back(nullptr);

  PointerType *pPointer = new PointerType(pType);
  pPointer->addAttribute(AttrAddressSpace);

  for (const auto &Attr : AttrQualifiers)
    pPointer->addAttribute(Attr);

  RefParamType refPointer(pPointer);

  // Add pointer to substitutions list *after* parsing a pointee type
  SignList.push_back(RefParamType(new PointerType(*pPointer)));
  return refPointer;
}

RefParamType DemangleParser::createVectorType() {
  unsigned int vectorLength = 0;
  if (!getNextNumber(vectorLength)) {
    // Reachning here is not expected, set error and return NULL;
    setError();
    return RefParamType();
  }
  if (!match(&TI_VECTOR_SUFFIX)) {
    // Reachning here is not expected, set error and return NULL;
    setError();
    return RefParamType();
  }
  // TODO: vector allowed only on primitive type.
  //      This implementation does not fail if this asumption does not hold.
  RefParamType pPrimitiveType = getNextType();
  if (!pPrimitiveType) {
    // Reachning here is not expected, set error and return NULL;
    setError();
    return RefParamType();
  }
  ParamType *pVector = new VectorType(pPrimitiveType, vectorLength);
  RefParamType refVector(pVector);
  // Push vector type to end of sign list
  // It is important to do this after parsing the scalar type
  // in case the scalar is a non-primitive type, it should
  // be pushed first to the sign list.
  SignList.push_back(refVector);
  return refVector;
}

RefParamType DemangleParser::createAtomicType() {
  RefParamType pBaseParamType = getNextType();
  if (!pBaseParamType) {
    // Reachning here is not expected, set error and return NULL;
    setError();
    return RefParamType();
  }
  // Atomic is supported only on primitive types
  if (!dyn_cast<PrimitiveType>(pBaseParamType.get())) {
    // Reachning here is not expected, set error and return NULL;
    setError();
    return RefParamType();
  }
  AtomicType *pAtomicType = new AtomicType(pBaseParamType);
  RefParamType refAtomic(pAtomicType);

  // Push atomic type to end of sign list
  SignList.push_back(refAtomic);
  return refAtomic;
}

RefParamType DemangleParser::createBlockType() {
  if (!match(&TI_BLOCK_PREFIX)) {
    // Reachning here is not expected, set error and return NULL;
    setError();
    return RefParamType();
  }
  BlockType *pBlockType = new BlockType();
  RefParamType refBlock(pBlockType);
  int index = 0;
  while (!match(&TI_BLOCK_SUFFIX)) {
    RefParamType pBlockParamType = getNextType();
    if (!pBlockParamType) {
      // Reachning here is not expected, set error and return NULL;
      setError();
      return RefParamType();
    }
    pBlockType->setParam(index++, pBlockParamType);
  }
  // Push block type to end of sign list
  // It is important to do this after parsing the block parameter types
  // in case some of the parameters are non-primitive type, it should
  // be pushed first to the sign list.
  SignList.push_back(refBlock);
  return refBlock;
}

RefParamType
DemangleParser::createUserDefinedType(unsigned int UserDefinedNameLength) {
  if (MangledStringLength - CurrentIndex < UserDefinedNameLength) {
    // Reachning here is not expected, set error and return NULL;
    setError();
    return RefParamType();
  }
  StringRef TypeName =
      MangledString.substr(CurrentIndex, UserDefinedNameLength);
  // Found a type, increment current index
  CurrentIndex += UserDefinedNameLength;

  auto It = ImageTypeNameTranslate.find(TypeName);
  if (It != ImageTypeNameTranslate.end()) {
    // We have a special Primitive Type
    if (!IsSpir12Name) {
      RefParamType refImageType = createPrimitiveType(It->second);
      SignList.push_back(refImageType);
      return refImageType;
    } else
      return createPrimitiveType(It->second);
  }

  ParamType *UserDefined = new UserDefinedType(TypeName);
  RefParamType RefUserDefined(UserDefined);
  // Push user-defined type to end of sign list
  SignList.push_back(RefUserDefined);
  return RefUserDefined;
}

bool DemangleParser::getNextNumber(unsigned int &Num) {
  const char *Start = MangledString.data() + CurrentIndex;
  char *End;
  Num = (unsigned int)strtol(Start, &End, 10);
  if (Start == End)
    return false;
  // Found a number, increment current index
  CurrentIndex += (End - Start);
  return true;
}

bool DemangleParser::getDuplicationSNumber(unsigned int &Num) {
  if (match(&TI_PARAM_DUP_SUFFIX)) {
    Num = 0;
    return true;
  }
  // Reaching here means there is a duplicate Number
  if (CurrentIndex == MangledStringLength) {
    // Expecting at least one character left in demangled string
    return false;
  }
  char DupChar = MangledString[CurrentIndex];
  if (DupChar >= '0' && DupChar <= '9') {
    Num = 1 + (unsigned int)(DupChar - '0');
  } else if (DupChar >= 'A' && DupChar <= 'Z') {
    Num = 11 + (unsigned int)(DupChar - 'A');
  } else {
    setError();
    return false;
  }
  // Found a duplicate number, increment current index
  CurrentIndex++;
  if (match(&TI_PARAM_DUP_SUFFIX)) {
    return true;
  }
  // Reachning here is not expected, set error and return false;
  setError();
  return false;
}

bool DemangleParser::getAddressSpace(TypeAttributeEnum &AttrAddressSpace) {
  AttrAddressSpace = ATTR_NONE;
  // Check if it is an address space
  if (!match(&TI_ADDRESS_SPACE_PREFIX)) {
    AttrAddressSpace = ATTR_PRIVATE;
    return true;
  }
  unsigned int AddressSpaceLength = 0;
  if (!getNextNumber(AddressSpaceLength) ||
      (MangledStringLength - CurrentIndex) < AddressSpaceLength ||
      AddressSpaceLength < TI_ADDRESS_SPACE_SUFFIX.TokenLength ||
      !match(&TI_ADDRESS_SPACE_SUFFIX)) {
    // Reachning here is not expected, set error and return false;
    setError();
    return false;
  }
  AddressSpaceLength -= TI_ADDRESS_SPACE_SUFFIX.TokenLength;
  StringRef AddressSpaceId =
      MangledString.substr(CurrentIndex, AddressSpaceLength);
  int AddressSpace;
  if (!to_integer(AddressSpaceId, AddressSpace)) {
    setError();
    return false;
  }
  // Found an AddressSpace, increment current index
  CurrentIndex += AddressSpaceLength;
  if (AddressSpace >= ATTR_ADDR_SPACE_FIRST &&
      AddressSpace <= ATTR_ADDR_SPACE_LAST) {
    AttrAddressSpace = (TypeAttributeEnum)AddressSpace;
    return true;
  }
  assert(false && "Unsupported address space!");
  setError();
  return false;
}

std::vector<TypeAttributeEnum> DemangleParser::getAddressQualifiers() {
  std::vector<TypeAttributeEnum> FoundAttrs;
  // Reached the end of the string
  if (CurrentIndex == MangledStringLength)
    return FoundAttrs;

  for (unsigned int i = 0; i < QUALIFIER_INFO_LIST_LENGTH; ++i) {
    assert(
        strlen(QualifierInfoList[i].Token) == 1 &&
        "Assumed Qualifier is one char, failing this force fixing this code!");
    if (QualifierInfoList[i].Token[0] == MangledString[CurrentIndex]) {
      // Found a match, increment current index
      CurrentIndex++;
      FoundAttrs.push_back(QualifierInfoList[i].Attr);
    }
  }
  return FoundAttrs;
}

// visit methods
void MangleVisitor::visit(const reflection::PrimitiveType *Ty) {
  int TyIndex = getTypeIndex(Ty);
  if (-1 != TyIndex) {
    OS << reflection::getDuplicateString(TyIndex);
    return;
  }
  OS << reflection::mangledPrimitiveString(Ty->getPrimitive());
  if (Ty->getPrimitive() >= reflection::PRIMITIVE_STRUCT_FIRST &&
      Ty->getPrimitive() <= reflection::PRIMITIVE_LAST)
    DupList.push_back(Ty);
}

void MangleVisitor::visit(const reflection::PointerType *Ty) {
  int TyIndex = getTypeIndex(Ty);
  if (-1 != TyIndex) {
    OS << reflection::getDuplicateString(TyIndex);
    return;
  }
  OS << "P";
  for (unsigned int i = 0; i < Ty->getAttributes().size(); ++i) {
    OS << getMangledAttribute(Ty->getAttributes()[i]);
  }
  Ty->getPointee()->accept(this);

  assert(Ty->getAttributes().size() > 0 &&
         "Pointers always have attributes (at least address space)");

  // Add dummy type to preserve substitutions order.
  // TODO: implement correct way to handle substitutions with qualifiers.
  // See more details in DemangleParser.
  DupList.push_back(nullptr);

  DupList.push_back(Ty);
}

void MangleVisitor::visit(const reflection::VectorType *Ty) {
  int TyIndex = getTypeIndex(Ty);
  if (-1 != TyIndex) {
    OS << reflection::getDuplicateString(TyIndex);
    return;
  }
  OS << "Dv" << Ty->getLength() << "_";
  Ty->getScalarType()->accept(this);
  DupList.push_back(Ty);
}

void MangleVisitor::visit(const reflection::AtomicType *Ty) {
  int TyIndex = getTypeIndex(Ty);
  if (-1 != TyIndex) {
    OS << reflection::getDuplicateString(TyIndex);
    return;
  }
  OS << "U"
     << "7_Atomic";
  Ty->getBaseType()->accept(this);
  DupList.push_back(Ty);
}

void MangleVisitor::visit(const reflection::BlockType *Ty) {
  int TyIndex = getTypeIndex(Ty);
  if (-1 != TyIndex) {
    OS << reflection::getDuplicateString(TyIndex);
    return;
  }
  OS << "U"
     << "13block_pointerFv";
  for (unsigned I = 0; I < Ty->getNumOfParams(); ++I) {
    Ty->getParam(I)->accept(this);
  }
  DupList.push_back(Ty);
  OS << "E";
}

void MangleVisitor::visit(const reflection::UserDefinedType *Ty) {
  int TyIndex = getTypeIndex(Ty);
  if (-1 != TyIndex) {
    OS << reflection::getDuplicateString(TyIndex);
    return;
  }
  std::string Name = Ty->toString();
  OS << Name.size() << Name;
  DupList.push_back(Ty);
}

int MangleVisitor::getTypeIndex(const reflection::ParamType *Ty) const {
  for (unsigned I = 0; I < DupList.size(); ++I) {
    if (Ty->equals(DupList[I]))
      return I;
  }
  return -1;
}

// Implementation of an API function.
// Indicating whether the given string is a mangled function name
bool NameMangleAPI::isMangledName(StringRef RawString) {
  if (!RawString.startswith(PREFIX))
    return false;
  return isDigit(RawString[PREFIX_LEN]);
}

std::string NameMangleAPI::mangle(const reflection::FunctionDescriptor &FD) {
  if (FD.isNull())
    return reflection::FunctionDescriptor::nullString().str();
  std::string Str;
  raw_string_ostream S(Str);
  S << "_Z" << FD.Name.size() << FD.Name;
  MangleVisitor visitor(S);
  for (unsigned int i = 0; i < FD.Parameters.size(); ++i) {
    FD.Parameters[i]->accept(&visitor);
  }
  return Str;
}

// Implementation of an API function.
// converts the given mangled name string to a FunctionDescriptor object.
reflection::FunctionDescriptor NameMangleAPI::demangle(StringRef Rawstring,
                                                       bool IsSpir12Name) {
  if (Rawstring.empty() ||
      reflection::FunctionDescriptor::nullString() == Rawstring)
    return reflection::FunctionDescriptor::null();

  StringRef MangledName(Rawstring);
  // making sure it starts with _Z
  if (!peelPrefix(MangledName)) {
    return reflection::FunctionDescriptor::null();
  }
  StringRef nameLen = peelNameLen(MangledName);
  // cutting the prefix
  int Len = atoi(nameLen.data());
  StringRef FName = MangledName.substr(0, Len);
  StringRef Parameters = MangledName.substr(Len, MangledName.size() - Len);

  reflection::FunctionDescriptor Ret;

  DemangleParser parser(Ret.Parameters, IsSpir12Name);

  if (!parser.demangle(Parameters.data())) {
    return reflection::FunctionDescriptor::null();
  }

  Ret.Name = FName.str();
  Ret.Width = reflection::width::NONE;

  return Ret;
}

StringRef NameMangleAPI::stripName(StringRef Rawstring) {
  StringRef MangledName(Rawstring);
  // making sure it starts with _Z
  if (!peelPrefix(MangledName))
    llvm_unreachable("invalid prefix");
  StringRef NameLen = peelNameLen(MangledName);
  // cutting the prefix
  int Len;
  bool Res = to_integer(NameLen, Len);
  (void)Res;
  assert(Res && "expect integer in Rawstring");
  return Rawstring.substr(PREFIX_LEN + NameLen.size(), Len);
}
