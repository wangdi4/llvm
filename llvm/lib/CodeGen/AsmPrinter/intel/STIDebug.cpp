//===-- STIDebug.cpp - Symbol And Type Info -------*- C++ -*--===//
//
//===----------------------------------------------------------------------===//
//
// This file contains support for writing symbol and type information
// compatible with Visual Studio.
//
//===----------------------------------------------------------------------===//

#include "STIDebug.h"
#include "STI.h"
#include "STIIR.h"
#include "pdbInterface.h"
#include "../DbgValueHistoryCalculator.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/PointerUnion.h" // dyn_cast
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/LexicalScopes.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetSubtargetInfo.h"

#include <map>
#include <vector>
#include <deque>

// FIXME: We should determine which getcwd to use during configuration and not
//        at compile time.
//
#ifdef LLVM_ON_WIN32
#include <direct.h>
#define GETCWD _getcwd
#else
#include <unistd.h>
#define GETCWD getcwd
#endif

using namespace llvm;

//===----------------------------------------------------------------------===//
// Options
//===----------------------------------------------------------------------===//

// EmitFunctionIDs
//
// When true, functions are emitted using S_GPROC32_ID/S_LPROC32_ID and a
// seperate LF_FUNC_ID/LF_MFUNC_ID record is generated. Otherwise the functions
// are emitted as S_GPROC32/S_LPROC32 and no function id is emitted.
//
// NOTE: These records are currently emitted to the .debug$T section, which is
//       incompatible with the PDB support where they are supposed to be
//       written to an "ID's" section.
//
static cl::opt<bool> EmitFunctionIDs (
    "debug-emit-function-ids",
    cl::Hidden,
    cl::desc("Emit function ID records"),
    cl::init(false));

//===----------------------------------------------------------------------===//
// Helper Routines
//===----------------------------------------------------------------------===//

#define PDB_DEFAULT_FILE_NAME "vc110.pdb"
#define PDB_DEFAULT_DLL_NAME "mspdb110.dll"

static size_t paddingBytes(const size_t size) {
  const uint32_t alignment = 4;
  return (alignment - (size % alignment)) % alignment;
}

static void getFullFileName(const DIFile *file, std::string &path) {
  path = (file->getDirectory() + Twine("\\") + file->getFilename()).str();
  std::replace(path.begin(), path.end(), '/', '\\');
  size_t index = 0;
  while ((index = path.find("\\\\", index)) != std::string::npos) {
    path.erase(index, 1);
  }
}

static std::string getRealName(std::string name) {
  std::string prefix = ".?AV"; //".?AU"
  std::string sufix = "@";
  std::string realName = sufix;

  while (std::size_t pos = name.find("::")) {
    if (pos == std::string::npos) {
      realName = (Twine(prefix) + Twine(name.substr(0, pos)) + Twine(sufix) +
                  Twine(realName)).str();
      break;
    }
    realName =
        (Twine(name.substr(0, pos)) + Twine(sufix) + Twine(realName)).str();
    name = name.substr(pos + 2);
  }
  return realName;
}

static bool isStaticMethod(StringRef linkageName) {
  // FIXME: this is a temporary WA to partial demangle linkageName
  //        Clang should mark static method using MDSubprogram (DPD200372369)
  size_t pos = linkageName.rfind("@@");
  if (pos != StringRef::npos && (pos + 2) < linkageName.size()) {
    switch (linkageName[pos + 2]) {
    case 'T':
    case 'S':
    case 'K':
    case 'L':
    case 'C':
    case 'D':
      return true;
    }
  }
  return false;
}

static unsigned getFunctionAttribute(const DISubprogram *SP,
                                     const DICompositeType *llvmParentType,
                                     bool introduced) {
  unsigned attribute = 0;
  unsigned virtuality = SP->getVirtuality();

  if (SP->isProtected())
    attribute = attribute | STI_ACCESS_PRIVATE;
  else if (SP->isPrivate())
    attribute = attribute | STI_ACCESS_PROTECT;
  else if (SP->isPublic())
    attribute = attribute | STI_ACCESS_PUBLIC;
  // Otherwise C++ member and base classes are considered public.
  else if (llvmParentType->getTag() == dwarf::DW_TAG_class_type)
    attribute = attribute | STI_ACCESS_PRIVATE;
  else
    attribute = attribute | STI_ACCESS_PUBLIC;

  if (SP->isArtificial()) {
    attribute = attribute | STI_COMPGENX;
  }

  switch (virtuality) {
  case dwarf::DW_VIRTUALITY_none:
    break;
  case dwarf::DW_VIRTUALITY_virtual:
    if (introduced) {
      attribute = attribute | STI_MPROP_INTR_VRT;
    } else {
      attribute = attribute | STI_MPROP_VIRTUAL;
    }
    break;
  case dwarf::DW_VIRTUALITY_pure_virtual:
    if (introduced) {
      attribute = attribute | STI_MPROP_PURE_INTR_VRT;
    } else {
      attribute = attribute | STI_MPROP_PURE_VRT;
    }
    break;
  default:
    assert(!"unhandled virtuality case");
    break;
  }

  if (isStaticMethod(SP->getLinkageName())) {
    attribute = attribute | STI_MPROP_STATIC;
  }

  return attribute;
}

static unsigned getTypeAttribute(const DIDerivedType *llvmType,
                                 const DICompositeType *llvmParentType) {
  unsigned attribute = 0;

  if (llvmType->isProtected())
    attribute = attribute | STI_ACCESS_PROTECT;
  else if (llvmType->isPrivate())
    attribute = attribute | STI_ACCESS_PRIVATE;
  else if (llvmType->isPublic())
    attribute = attribute | STI_ACCESS_PUBLIC;
  // Otherwise C++ member and base classes are considered public.
  else if (llvmParentType->getTag() == dwarf::DW_TAG_class_type)
    attribute = attribute | STI_ACCESS_PRIVATE;
  else
    attribute = attribute | STI_ACCESS_PUBLIC;

  if (llvmType->isArtificial()) {
    attribute = attribute | STI_COMPGENX;
  }

  if (llvmType->isStaticMember()) {
    attribute = attribute | STI_MPROP_STATIC;
  }

  return attribute;
}

static bool isIndirectExpression(const DIExpression *Expr) {
  if (!Expr || (Expr->getNumElements() == 0)) {
    return false;
  }

  if (Expr->getNumElements() != 1) {
    // Looking for DW_OP_deref expression only.
    return false;
  }

  DIExpression::expr_op_iterator I = Expr->expr_op_begin();
  DIExpression::expr_op_iterator E = Expr->expr_op_end();

  for (; I != E; ++I) {
    switch (I->getOp()) {
    case dwarf::DW_OP_bit_piece:
    case dwarf::DW_OP_plus:
      return false;
    case dwarf::DW_OP_deref:
      return true;
    default:
      llvm_unreachable("unhandled opcode found in DIExpression");
    }
  }
  return false;
}

//===----------------------------------------------------------------------===//
// truncateName(name)
//
// Truncates the specified name if it is too long to encode in individual
// symbol entries.
//
//===----------------------------------------------------------------------===//

static void truncateName(std::string& name) {
  const size_t MAXIMUM_NAME_SIZE = 4096 - 1; // remove one for null terminator
  if (name.size() > MAXIMUM_NAME_SIZE) {
    name.resize(MAXIMUM_NAME_SIZE); 
  }
}

//===----------------------------------------------------------------------===//
// Printing/Debugging Routines
//===----------------------------------------------------------------------===//

static StringRef toString(STISubsectionID subsectionID) {
  StringRef string;

  switch (subsectionID) {
#define X(KIND, VALUE)                                                         \
  case (KIND):                                                                 \
    string = #KIND;                                                            \
    break;
    STI_SUBSECTION_KINDS
#undef X
  default:
    string = "<invalid subsection kind>";
    break;
  }

  return string;
}

static StringRef toString(STIMachineID machineID) {
  StringRef string;

  switch (machineID) {
#define X(KIND, VALUE)                                                         \
  case (KIND):                                                                 \
    string = #KIND;                                                            \
    break;
    STI_MACHINE_KINDS
#undef X
  default:
    string = "<invalid machine kind>";
    break;
  }

  return string;
}

static StringRef toString(STISymbolID symbolID) {
  StringRef string;

  switch (symbolID) {
#define X(KIND, VALUE)                                                         \
  case (KIND):                                                                 \
    string = #KIND;                                                            \
    break;
    X(S_OBJNAME,        0x0000) // FIXME: define these in STI.h with values
    X(S_COMPILE3,       0x0000)
    X(S_GPROC32_ID,     0x0000)
    X(S_LPROC32_ID,     0x0000)
    X(S_FRAMEPROC,      0x0000)
    X(S_BLOCK32,        0x0000)
    X(S_REGREL32,       0x0000)
    X(S_REGISTER,       0x0000)
    X(S_BPREL32,        0x0000)
    X(S_LDATA32,        0x0000)
    X(S_GDATA32,        0x0000)
    X(S_PROC_ID_END,    0x0000)
    X(S_CONSTANT,       0x0000)
#undef X
  default:
    string = "<invalid symbol kind>";
    break;
  }

  return string;
}

//===----------------------------------------------------------------------===//
// toMachineID(architecture)
//===----------------------------------------------------------------------===//

static STIMachineID toMachineID(Triple::ArchType architecture) {
  STIMachineID machineID;

  switch (architecture) {
#define MAP(ARCH, MACHINE)                                                     \
  case (Triple::ARCH):                                                         \
    machineID = MACHINE;                                                       \
    break
    MAP(x86,    STI_MACHINE_INTEL_PENTIUM_III);
    MAP(x86_64, STI_MACHINE_INTEL64);
#undef MAP
  default:
    assert(!"Architecture cannot be mapped to an STI machine type!");
    break;
  }

  return machineID;
}

//===----------------------------------------------------------------------===//
// numericLength(numeric)
//
// Returns the encoded length, in bytes, of the specified numeric leaf.
//
//===----------------------------------------------------------------------===//

static size_t numericLength(const STINumeric* numeric) {
  size_t length;

  // Start with the length of the encoding kind.
  //
  length  = numeric->getLeafID() != LF_INTEL_NONE ? 2 : 0;

  // Add the size of the numeric data.
  //
  length += numeric->getSize();

  // The minimum encoded size of the leaf must be two bytes long.
  //
  length  = std::max<size_t>(length, 2);

  return length;
}


//===----------------------------------------------------------------------===//
// calculateFieldLength(field)
//
// Calculate the encoded length, in bytes, of the specified field.
//
//===----------------------------------------------------------------------===//

static size_t calculateFieldLength(const STITypeFieldListItem *field) {
  size_t length;

  switch (field->getKind()) {
  case (STI_OBJECT_KIND_TYPE_VBASECLASS): {
    const STITypeVBaseClass *vBaseClass =
        static_cast<const STITypeVBaseClass*>(field);
    const STINumeric *offset = vBaseClass->getVbpOffset();
    const STINumeric *index  = vBaseClass->getVbIndex();

    length = 12 + numericLength(offset) + numericLength(index);
    }
    break;

  case (STI_OBJECT_KIND_TYPE_BASECLASS): {
    const STITypeBaseClass *baseClass =
        static_cast<const STITypeBaseClass*>(field);
    const STINumeric *offset = baseClass->getOffset();

    length = 8 + numericLength(offset);
    }
    break;

  case (STI_OBJECT_KIND_TYPE_VFUNCTAB):
    length = 8;
    break;

  case (STI_OBJECT_KIND_TYPE_MEMBER): {
    const STITypeMember *member = static_cast<const STITypeMember*>(field);
    const STINumeric *offset   = member->getOffset();
    StringRef         name     = member->getName();
    bool              isStatic = member->isStatic();

    length = 8
           + (isStatic ? 0 : numericLength(offset))
           + name.size() + 1;
    }
    break;

  case (STI_OBJECT_KIND_TYPE_METHOD): {
    const STITypeMethod *method = static_cast<const STITypeMethod*>(field);
    StringRef name = method->getName();

    length = 8 + name.size() + 1;
    }
    break;

  case (STI_OBJECT_KIND_TYPE_ONEMETHOD): {
    const STITypeOneMethod *method =
        static_cast<const STITypeOneMethod*>(field);
    StringRef name = method->getName();
    bool isVirtual = method->getVirtuality();

    length = 8 + (isVirtual ? 4 : 0) + name.size() + 1;
    }
    break;

  case (STI_OBJECT_KIND_TYPE_ENUMERATOR): {
    const STITypeEnumerator *enumerator =
        static_cast<const STITypeEnumerator*>(field);
    const STINumeric *value = enumerator->getValue();
    StringRef         name  = enumerator->getName();

    length = 4 + numericLength(value) + name.size() + 1;
    }
    break;

  case (STI_OBJECT_KIND_TYPE_INDEX):
    length = 8;
    break;

  default:
    assert(!"Invalid field list item kind!");
    length = 0;
    break;
  }

  return length;
}

//===----------------------------------------------------------------------===//
// calculateFieldListLength(fieldList)
//
// Calculate the encoded length, in bytes, of the specified field list.
//
//===----------------------------------------------------------------------===//

static size_t calculateFieldListLength(const STITypeFieldList *fieldList) {
  size_t length = 4;

  for (const STITypeFieldListItem *field : fieldList->getFields()) {
    size_t fieldLength;

    fieldLength = calculateFieldLength(field);
    fieldLength += paddingBytes(fieldLength);

    length += fieldLength;
  }

  return length;
}

//===----------------------------------------------------------------------===//
// STIFieldListBuilder
//
// This class contains contains code for constructing field list types.
//
// If the field list length approaches the maximum size, then create
// additional field lists and string them together using index entries,
// like this:
//
//     0x1000 LF_FIELDLIST
//            list[0] = LF_ENUMERATE ...
//            ...
//            list[n] = LF_ENUMERATE ...
//     0x1001 LF_FIELDLIST
//            list[0] = LF_ENUMERATE ...
//            ...
//            LF_INDEX, type: 0x1000
//     0x1002 LF_FIELDLIST
//            list[0] = LF_ENUMERATE ...      <-- first field
//            ...
//            LF_INDEX, type: 0x1001
//     0x1003 LF_ENUM, type: 0x1002
//
//
// Usage:
// {
//   STIFieldListBuilder builder;
//
//   // Add all of the field lists items to the builder.
//   //
//   for (STIFieldListItem *item : getFieldListItems()) {
//     builder.append(item);
//   }
//
//   // Append each field list created by the builder to the types table.
//   //
//   for (STITypeFieldList *fieldList : builder.getFieldLists()) {
//     appendType(fieldList);
//   }
// }
//
//===----------------------------------------------------------------------===//

class STIFieldListBuilder {
public:
  typedef std::deque<STITypeFieldList*> FieldLists;

private:
  FieldLists    _fieldLists;        // Field lists created by this builder.
  size_t        _fieldListLength;   // Current length of "_fieldLists.front()"

public:
  STIFieldListBuilder() :
      _fieldLists       (),
      _fieldListLength  (initialFieldListLength()) {
    _fieldLists.push_back(createTypeFieldList()); // Add initial field list.
  }

  ~STIFieldListBuilder() {
    // It is the callers responsibility to deallocate field lists created by
    // this builder.
  }

  // append(field)
  //
  // Appends the specified field to the field list being built.
  //
  void append(STITypeFieldListItem *field) {
    size_t fieldLength;

    // Calculate the encoded length in bytes of the specified field, including
    // padding.
    //
    fieldLength  = calculateFieldLength(field);
    fieldLength += paddingBytes(fieldLength);

    // Check to see if adding this field will 'overflow' the maximum allowable
    // size of a field list type record.  We reserve "8" bytes at the end to
    // allow for the creation of an LF_INDEX entry in the field list.
    //
    if (_fieldListLength + fieldLength + 8 > UINT16_MAX) {
      createFieldListExtension();
    }

    // Append the item to the current field list and update the length.
    //
    _fieldLists.front()->append(field);
    _fieldListLength += fieldLength;
  }

  // getFieldLists()
  //
  // Returns a sequential container which can be traversed, in order, to
  // emit the field lists created by this builder.
  //
  FieldLists getFieldLists() const { return _fieldLists; }

private:
  // createTypeFieldList()
  //
  // Returns a newly constructed (empty) field list type.
  //
  static STITypeFieldList *createTypeFieldList() {
    return STITypeFieldList::create();
  }

  // createTypeIndex(type)
  //
  // Creates a new LF_INDEX type record referring to the specified type.
  //
  static STITypeIndex *createTypeIndex(STIType *type) {
    STITypeIndex *index;

    index = STITypeIndex::create();
    index->setType(type);

    return index;
  }

  // initialFieldListLength()
  //
  // Length in bytes of a field list which contains no fields.
  //
  static size_t initialFieldListLength() {
    return 4;
  }

  // createFieldListExtension()
  //
  // Creates an extension of the current field list, which can be used when
  // the size of the fields overflows the maximum allowable size of a single
  // field list.
  //
  void createFieldListExtension() {
    STITypeFieldList *fieldList = createTypeFieldList();

    // The existing field list is connected to the next field list using an
    // LF_INDEX type entry.  This entries requires 8 bytes, so make sure we
    // have room for it.
    //
    assert(_fieldListLength + 8 <= UINT16_MAX);
    _fieldLists.front()->append(createTypeIndex(fieldList));

    // Prepend the new field list before the others.
    //
    _fieldLists.push_front(fieldList);

    // Reset the length to match the newly created field list.
    //
    _fieldListLength = initialFieldListLength();
  }
};

//===----------------------------------------------------------------------===//
// STITypeTable
//
// A table containing every type entry which will be emitted to the .debug$T
// section (or PDB file).
//
//===----------------------------------------------------------------------===//

typedef std::vector<STIType *> STITypeTable;

//===----------------------------------------------------------------------===//
// STITypeMap
// STITypeScopedMap
// STIDclToDefTypeMap
// LabelMap
//===----------------------------------------------------------------------===//

typedef DenseMap<const MDNode*,  STIType*>   STITypeMap;
typedef DenseMap<const DIType*,  STITypeMap> STITypeScopedMap;
typedef DenseMap<const STIType*, STIType*>   STIDclToDefTypeMap;
typedef DenseMap<const MachineInstr *, MCSymbol *> LabelMap;

//===----------------------------------------------------------------------===//
// ClassInfo
//===----------------------------------------------------------------------===//

struct ClassInfo {
  typedef std::vector<const DIDerivedType *> BaseClassList;
  struct VBaseClassInfo {
    const MDNode *llvmInheritance;
    unsigned vbIndex;
    bool indirect;

    VBaseClassInfo() : llvmInheritance(nullptr), vbIndex(0), indirect(false) {}

    VBaseClassInfo(const MDNode *N, unsigned I, bool InDir)
        : llvmInheritance(N), vbIndex(I), indirect(InDir) {}
  };
  // llvmClassType -> {llvmInheritance, vbIndex, indirect}
  typedef MapVector<const MDNode *, VBaseClassInfo> VBaseClassList;
  // [<llvmMemberType, baseOffset>]
  typedef std::vector<std::pair<const MDNode *, unsigned> > MemberList;
  // methodName -> [<llvmSubprogram, introduced>]
  typedef std::map<StringRef, std::vector<std::pair<const MDNode *, bool> > >
      MethodsMap;
  // methodName -> [llvmSubprogram]
  typedef std::map<StringRef, std::vector<const MDNode *> > VMethodsMap;

  // non-virtual base classes
  BaseClassList baseClasses;
  // virtual base classes (direct and indirect).
  VBaseClassList vBaseClasses;
  // offset of virtual base pointer
  int vbpOffset;
  // virtual function table (only if have introduced virtual methods)
  const MDNode *vFuncTab;
  // direct members
  MemberList members;
  // direct methods (gathered by name), for each function: (introduced?)
  MethodsMap methods;
  // virtual methods (gathered by name), for DTOR use "~" name.
  VMethodsMap vMethods;
  // FIXME: add support to: CONSTRUCTOR, OVERLOAD, OVERLOADED ASSIGNMENT, etc.
  // Class has Constructor
  bool hasCTOR;
  // Class has Destructor
  bool hasDTOR;
  // This class is nested within another aggregate type.
  bool isNested;
  // Number of virtual methods (length of virtual function table)
  unsigned vMethodsCount;

  ClassInfo()
      : vbpOffset(~0), vFuncTab(nullptr), hasCTOR(false), hasDTOR(false),
        isNested(false), vMethodsCount(0) {}
};

//===----------------------------------------------------------------------===//
// STIWriter
//===----------------------------------------------------------------------===//

class STIWriter {
public:
  virtual void emitInt8(int32_t value) = 0;
  virtual void emitInt16(int32_t value) = 0;
  virtual void emitInt32(int32_t value) = 0;
  virtual void emitString(StringRef string) = 0;
  virtual void emitBytes(size_t size, const char* data) = 0;
  virtual void emitFill(size_t size, const uint8_t byte) = 0;
  virtual void emitComment(StringRef comment) = 0;
  virtual void emitLabel(MCSymbol *symbol) = 0;
  virtual void emitValue(const MCExpr *value, unsigned int sizeInBytes) = 0;

  virtual void idBegin(const STIType* type) = 0;
  virtual void idEnd(const STIType* type) = 0;
  virtual void typeBegin(const STIType* type) = 0;
  virtual void typeEnd(const STIType* type) = 0;

  virtual ~STIWriter();

protected:
  STIWriter();
};

STIWriter::STIWriter() {
}

STIWriter::~STIWriter() {
}

//===----------------------------------------------------------------------===//
// STIAsmWriter
//===----------------------------------------------------------------------===//

class STIAsmWriter : public STIWriter {
public:
  STIAsmWriter(AsmPrinter* asmPrinter);
  virtual ~STIAsmWriter();

  static STIAsmWriter* create(AsmPrinter* asmPrinter);

  AsmPrinter* ASM() const;

  virtual void emitInt8(int32_t value);
  virtual void emitInt16(int32_t value);
  virtual void emitInt32(int32_t value);
  virtual void emitString(StringRef string);
  virtual void emitBytes(size_t size, const char* data);
  virtual void emitFill(size_t size, const uint8_t byte);
  virtual void emitComment(StringRef comment);
  virtual void emitLabel(MCSymbol *symbol);
  virtual void emitValue(const MCExpr *value, unsigned int sizeInBytes);

  virtual void idBegin(const STIType* type);
  virtual void idEnd(const STIType* type);
  virtual void typeBegin(const STIType* type);
  virtual void typeEnd(const STIType* type);

private:
  AsmPrinter* _asmPrinter;
};

STIAsmWriter::STIAsmWriter(AsmPrinter* asmPrinter) :
    STIWriter   (),
    _asmPrinter (asmPrinter) {
}

STIAsmWriter::~STIAsmWriter() {
}

STIAsmWriter* STIAsmWriter::create(AsmPrinter* asmPrinter) {
  return new STIAsmWriter(asmPrinter);
}

AsmPrinter* STIAsmWriter::ASM() const {
  return _asmPrinter;
}

void STIAsmWriter::emitInt8(int32_t value) {
  ASM()->EmitInt8(value);
}

void STIAsmWriter::emitInt16(int32_t value) {
  ASM()->EmitInt16(value);
}

void STIAsmWriter::emitInt32(int32_t value) {
  ASM()->EmitInt32(value);
}

void STIAsmWriter::emitString(StringRef string) {
  ASM()->OutStreamer->EmitBytes(string);
  ASM()->EmitInt8(0);
}

void STIAsmWriter::emitBytes(size_t size, const char* data) {
  ASM()->OutStreamer->EmitBytes(StringRef(data, size));
}

void STIAsmWriter::emitFill(size_t size, const uint8_t byte) {
  ASM()->OutStreamer->emitFill(size, byte);
}

void STIAsmWriter::emitComment(StringRef comment) {
  ASM()->OutStreamer->AddComment(comment);
}

void STIAsmWriter::emitLabel(MCSymbol *symbol) {
  ASM()->OutStreamer->EmitLabel(symbol);
}

void STIAsmWriter::emitValue(const MCExpr *value, unsigned int sizeInBytes) {
  ASM()->OutStreamer->EmitValue(value, sizeInBytes);
}

void STIAsmWriter::idBegin(const STIType* type) {
}

void STIAsmWriter::idEnd(const STIType* type) {
}

void STIAsmWriter::typeBegin(const STIType* type) {
}

void STIAsmWriter::typeEnd(const STIType* type) {
}

//===----------------------------------------------------------------------===//
// STIPdbWriter
//===----------------------------------------------------------------------===//

class STIPdbWriter : public STIWriter {
public:
  STIPdbWriter();
  virtual ~STIPdbWriter();

  static STIPdbWriter* create();

  virtual void emitInt8(int32_t value);
  virtual void emitInt16(int32_t value);
  virtual void emitInt32(int32_t value);
  virtual void emitString(StringRef string);
  virtual void emitBytes(size_t size, const char* data);
  virtual void emitFill(size_t size, const uint8_t byte);
  virtual void emitComment(StringRef comment);
  virtual void emitLabel(MCSymbol *symbol);
  virtual void emitValue(const MCExpr *value, unsigned int sizeInBytes);

  virtual void idBegin(const STIType* type);
  virtual void idEnd(const STIType* type);
  virtual void typeBegin(const STIType* type);
  virtual void typeEnd(const STIType* type);

private:
  std::vector<char> _buffer;
};

STIPdbWriter::STIPdbWriter() :
    STIWriter   (),
    _buffer     () {
}

STIPdbWriter::~STIPdbWriter() {
}

STIPdbWriter* STIPdbWriter::create() {
  return new STIPdbWriter();
}

void STIPdbWriter::emitInt8(int32_t value) {
  emitBytes(1, reinterpret_cast<const char*>(&value));
}

void STIPdbWriter::emitInt16(int32_t value) {
  emitBytes(2, reinterpret_cast<const char*>(&value));
}

void STIPdbWriter::emitInt32(int32_t value) {
  emitBytes(4, reinterpret_cast<const char*>(&value));
}

void STIPdbWriter::emitString(StringRef string) {
  _buffer.insert(_buffer.end(), string.begin(), string.end());
  _buffer.push_back('\0');
}

void STIPdbWriter::emitBytes(size_t size, const char* data) {
  _buffer.insert(_buffer.end(), data, data + size);
}

void STIPdbWriter::emitFill(size_t size, const uint8_t byte) {
  // Fill bytes are not emitted to the PDB writer.
}

void STIPdbWriter::emitComment(StringRef comment) {
  // Comments are not emitted to the PDB writer.
}

void STIPdbWriter::emitLabel(MCSymbol *symbol) {
  // Labels are not emitted to the PDB writer.
}

void STIPdbWriter::emitValue(const MCExpr *value, unsigned int sizeInBytes) {
  // This is currently only used for emitting label diffs, which are not used
  // when writing type information to the PDB writer.
}

void STIPdbWriter::idBegin(const STIType* type) {
  assert(_buffer.size() == 0);
}

void STIPdbWriter::idEnd(const STIType* type) {
  unsigned long index;

  // Buffer must minimally contain a type length.
  assert(_buffer.size() > 2);

  pdb_write_id(_buffer.data(), &index);

  const_cast<STIType *>(type)->setIndex(index);

  _buffer.clear();
}

void STIPdbWriter::typeBegin(const STIType* type) {
  assert(_buffer.size() == 0);
}

void STIPdbWriter::typeEnd(const STIType* type) {
  unsigned long index;

  // Buffer must minimally contain a type length.
  assert(_buffer.size() > 2);

  pdb_write_type(_buffer.data(), &index);

  const_cast<STIType *>(type)->setIndex(index);

  _buffer.clear();
}

//===----------------------------------------------------------------------===//
// STIDebugFixupKind
//===----------------------------------------------------------------------===//

enum STIDebugFixupKindEnum {
  STI_DEBUG_FIXUP_KIND_NONE     = 0,
  STI_DEBUG_FIXUP_KIND_NESTED
};
typedef enum STIDebugFixupKindEnum STIDebugFixupKind;

//===----------------------------------------------------------------------===//
// STIDebugFixup
//
// Base class for debug information fixup records.
//
//===----------------------------------------------------------------------===//

class STIDebugFixup {
private:
  STIDebugFixupKind _kind;

public:
  virtual ~STIDebugFixup() {}

  STIDebugFixupKind kind() const { return _kind; }

protected:
  STIDebugFixup(STIDebugFixupKind kind) : _kind (kind) {}
};

//===----------------------------------------------------------------------===//
// STIDebugFixupNested
//
// Fixup record for assigned IS_NESTED and CNESTED (contains nested) properties
// on type records (structs and enumerations).
//
//===----------------------------------------------------------------------===//

class STIDebugFixupNested : public STIDebugFixup {
private:
  const DICompositeType *_nestedType;

public:
  STIDebugFixupNested(const DICompositeType *nestedType) :
      STIDebugFixup (STI_DEBUG_FIXUP_KIND_NESTED),
      _nestedType   (nestedType) {}

  virtual ~STIDebugFixupNested() {}

  const DICompositeType *getNestedType() const { return _nestedType; }
};

//===----------------------------------------------------------------------===//
// STIDebugFixupTable
//
// Table of fix-up records.
//
//===----------------------------------------------------------------------===//

typedef std::vector<STIDebugFixup*> STIDebugFixupTable;

//===----------------------------------------------------------------------===//
// STIDebugImpl
//===----------------------------------------------------------------------===//

class STIDebugImpl : public STIDebug {
private:
  typedef DenseMap<const Function *, STISymbolProcedure *> FunctionMap;
  typedef DenseMap<const MDNode *, STIScope *> STIScopeMap;
  typedef DenseMap<const MDNode *, ClassInfo *> ClassInfoMap;
  typedef DenseMap<const MDNode *, std::string> StringNameMap;
  typedef DenseMap<const DISubprogram *, Function *> DISubprogramMap;

  AsmPrinter *_asmPrinter;
  STISymbolProcedure *_currentProcedure;
  DbgValueHistoryMap _valueHistory;
  FunctionMap _functionMap;
  DISubprogramMap _subprogramMap;
  STISymbolTable _symbolTable;
  STITypeTable _typeTable;
  STIStringTable _stringTable;
  STIChecksumTable _checksumTable;
  STIScopeMap _scopeMap;
  STITypeScopedMap _typeMap;
  STIDclToDefTypeMap _defTypeMap;
  STIType *_voidType;
  STIType *_vbpType;
  unsigned int _blockNumber;
  LexicalScopes _lexicalScopes;
  LabelMap _labelsBeforeInsn;
  LabelMap _labelsAfterInsn;
  const MachineInstr *_curMI;
  STISubsection *_currentSubsection;
  unsigned _ptrSizeInBits;
  ClassInfoMap _classInfoMap;
  StringNameMap _stringNameMap;
  unsigned _uniqueNameCounter;
  std::vector<char> _pdbBuff;
  StringRef _pdbFileName;
  StringRef _objFileName;
  bool _usePDB;
  STIWriter* _writer;
  STIDebugFixupTable _fixupTable;

  static const char* _unnamedType;

public:
  STIDebugImpl(AsmPrinter *asmPrinter);
  virtual ~STIDebugImpl();

  void setSymbolSize(const MCSymbol *Symbol, uint64_t size);
  void beginModule();
  void endModule();
  void beginFunction(const MachineFunction *MF);
  void endFunction(const MachineFunction *MF);
  void beginInstruction(const MachineInstr *MI);
  void endInstruction();

protected:
  AsmPrinter *ASM();
  const AsmPrinter *ASM() const;
  MachineModuleInfo *MMI() const;
  const Module *getModule() const;
  const TargetRegisterInfo *getTargetRegisterInfo();
  STISymbolTable *getSymbolTable();
  const STISymbolTable *getSymbolTable() const;
  STITypeTable *getTypeTable();
  const STITypeTable *getTypeTable() const;
  STIDebugFixupTable *getFixupTable();
  void appendFixup(STIDebugFixup* fixup);
  STIStringTable *getStringTable();
  const STIStringTable *getStringTable() const;
  STIChecksumTable *getChecksumTable();
  const STIChecksumTable *getChecksumTable() const;
  bool hasScope(const MDNode *llvmNode) const;
  STIScope *getScope(const MDNode *llvmNode);
  void addScope(const MDNode *llvmNode, STIScope *object);
  STITypeMap *getTypeMap(const DIType *llvmClass = nullptr);
  STIDclToDefTypeMap *getDefTypeMap();
  const STIDclToDefTypeMap *getDefTypeMap() const;
  ClassInfoMap *getClassInfoMap();
  const ClassInfoMap *getClassInfoMap() const;
  StringNameMap *getStringNameMap();
  const StringNameMap *getStringNameMap() const;
  STIWriter* writer() const;
  void setWriter(STIWriter* writer);

  std::string getUniqueName();
  char *getCWD() const;
  std::string getPDBFullPath() const;
  std::string getOBJFullPath() const;
  StringRef getMDStringValue(StringRef MDName) const;
  std::string nameForAggregateType(const DICompositeType *llvmType);

  STISymbolCompileUnit *getCompileUnit() { // FIXME:
    STISymbolModule *module =
        static_cast<STISymbolModule *>(getSymbolTable()->getRoot());
    STISymbolCompileUnit *compileUnit = module->getCompileUnits()->back();
    return compileUnit;
  }

  STIScope *getOrCreateScope(const DIScope* llvmScope);
  std::string getScopeFullName(const DIScope* llvmScope, StringRef name,
                               bool useClassName = false);
  STIType *getClassScope(const DIScope* llvmScope);

  STIRegID toSTIRegID(unsigned int regnum) const;
  STISymbolVariable *createSymbolVariableFromDbgValue(
          const DILocalVariable *DIV,
          const MachineInstr *DVInsn);
  STISymbolVariable *createSymbolVariableFromFrameIndex(
          const DILocalVariable *DIV,
          int frameIndex);
  STISymbolVariable *createSymbolVariable(
          const DILocalVariable *DIV,
          STILocation *location);

  STISymbolProcedure *getCurrentProcedure() const;
  void setCurrentProcedure(STISymbolProcedure *procedure);

  void setSymbolModule(STISymbolModule *module);
  STISymbolModule *getSymbolModule() const;

  size_t getPaddingSize(const STIChecksumEntry *entry) const;

  void clearValueHistory();

  void collectModuleInfo();
  void collectGlobalVariableInfo(const DICompileUnit* CU);
  void collectRoutineInfo();

  ClassInfo &collectClassInfo(const DICompositeType *llvmType);
  void collectClassInfoFromInheritance(ClassInfo &info,
                                       const DIDerivedType *inherTy,
                                       bool &finalizedOffset);
  void collectMemberInfo(ClassInfo &info, const DIDerivedType *DDTy);
  bool isEqualVMethodPrototype(
      const DISubroutineType *typeA,
      const DISubroutineType *typeB);

  const DIType *getUnqualifiedDIType(const DIType *ditype);

  STINumeric* createNumericUnsignedInt(const uint64_t value);
  STINumeric* createNumericSignedInt(const int64_t value);
  STINumeric* createNumericAPInt(const DIType *ditype, const APInt& value);
  STINumeric* createNumericAPFloat(const DIType *ditype, const APFloat& value);

  STISymbolProcedure *getOrCreateSymbolProcedure(const DISubprogram *SP);
  STISymbolProcedure *createSymbolThunk(const DISubprogram *SP);
  STISymbolBlock *createSymbolBlock(const DILexicalBlockBase *LB);
  STIChecksumEntry *getOrCreateChecksum(StringRef path);

  void appendType(STIType* type);
  void mapLLVMTypeToSTIType(
          const DIType *llvmType,
          STIType      *stiType,
          const DIType *classType = nullptr);
  STIType* getMappedSTIType(
          const DIType *llvmType,
          const DIType *classType = nullptr);
  bool isDefnInProgress(const DIType *llvmType);
  void setDefnInProgress(const DIType *llvmType, bool inProgress);

  STIType *toTypeDefinition(STIType *type);

  STISymbolUserDefined *createSymbolUserDefined(STIType *type, StringRef name);

  STITypeVShape *lowerTypeVShape(unsigned vMethodsCount);

  STITypeBasic *createTypeBasic(
        STITypeBasic::Primitive primitive,
        uint32_t sizeInBits) const;
  STITypeStructure *createTypeStructure(
        const uint16_t leaf,
        std::string name,
        STITypeFieldList *fieldType,
        STITypeVShape *vshapeType,
        const uint16_t count,
        STITypeStructure::Properties  properties,
        STINumeric *size,
        const uint32_t sizeInBits);
  STITypeStructure *lowerTypeStructureDecl(
        const DICompositeType *llvmType,
        ClassInfo& classInfo);
  STITypeStructure *lowerTypeStructureDefn(
        const DICompositeType *llvmType,
        ClassInfo& classInfo);

  STITypeBasic::Primitive toBasicPrimitive(const DIBasicType *llvmType);
  STITypeBasic::Primitive toPointerPrimitive(const DIDerivedType *llvmType);

  STITypeArgumentList *lowerTypeSubroutineArgumentList(
      DITypeRefArray elements,
      uint16_t       firstArgIndex);
  STITypeFieldList* lowerTypeStructureFieldList(
      const DICompositeType *llvmType,
      ClassInfo             &info,
      STITypeVShape         *vShapeType);

  STIType *lowerTypeBasic(const DIBasicType *llvmType);
  STIType *lowerTypePointer(const DIDerivedType *llvmType);
  STIType *lowerTypePointerToBasic(const DIDerivedType *llvmType);
  STIType *lowerTypeModifier(const DIDerivedType *llvmType);
  STIType *lowerTypeArray(const DICompositeType *llvmType);
  STIType *lowerTypeStructure(const DICompositeType *llvmType);
  STITypeEnumerator *lowerTypeEnumerator(const DIEnumerator *enumerator);
  STIType *lowerTypeEnumerationFieldList(const DICompositeType *llvmType);
  STIType *lowerTypeEnumeration(const DICompositeType *llvmType);
  STIType *lowerTypeSubroutine(const DISubroutineType *llvmType);
  STIType* lowerTypeRestrict(const DIDerivedType* llvmType);
  STIType *lowerTypeAlias(const DIDerivedType *llvmType);
  STIType *lowerTypeMemberFunction(const DISubroutineType *llvmType,
                                   const DIType *llvmClass);
  STIType* lowerTypeUnspecified(const DIType* llvmType);
  STIType *lowerType(const DIType *llvmType);

  STIType *lowerSubprogramType(const DISubprogram *subprogram);

  uint64_t getBaseTypeSize(const DIDerivedType *Ty) const;

  STIType *getVoidType() const;
  STIType *getVbpType() const;
  unsigned getPointerSizeInBits() const;

  void fixSymbolUserDefined(STISymbolUserDefined *type) const;
  void fixupNested(const STIDebugFixupNested *fixup);
  void fixup();

  void layout();
  void emit();

  // Used with _typeIdentifierMap for type resolution, not clear why?
  template <typename T> T *resolve(TypedDINodeRef<T> ref) const;

  // Routines for emitting atomic data.
  void emitAlign(unsigned int byteAlignment) const;
  void emitPadding(unsigned int padByteCount) const;
  void emitInt8(int value) const;
  void emitInt16(int value) const;
  void emitInt32(int value) const;
  void emitString(StringRef string) const;
  void emitComment(StringRef comment) const;
  void emitValue(const MCExpr *expr, unsigned int byteSize) const;
  void emitLabel(MCSymbol *symbol) const;
  void emitLabelDiff(const MCSymbol *begin,
                     const MCSymbol *end,
                     unsigned sizeInBytes = 4) const;
  void emitSymbolID(const STISymbolID symbolID) const;
  void emitBytes(const char *data, size_t size) const;
  void emitFill(size_t size, const uint8_t byte) const;
  void emitSecRel32(MCSymbol *symbol) const;
  void emitSectionIndex(MCSymbol *symbol) const;
  void emitNumeric(const uint32_t num) const;
  void emitNumeric(const STINumeric* numeric) const;

  // Routines for emitting atomic PDB data.
  void idBegin(const STIType* type) const;
  void idEnd(const STIType* type) const;
  void typeBegin(const STIType* type) const;
  void typeEnd(const STIType* type) const;
  bool usePDB() const;

  // Routines for emitting sections.
  void emitSectionBegin(MCSection *section) const;

  // Routines for emitting subsections.
  void emitSubsectionBegin(STISubsection *subsection) const;
  void emitSubsectionEnd(STISubsection *subsection) const;
  void emitSubsection(STISubsectionID id) const;
  void closeSubsection() const;

  // Routines for emitting the .debug$S section.
  void emitSymbols() const;
  void emitStringTable() const;
  void emitStringEntry(const STIStringEntry *entry) const;
  void emitChecksumTable() const;
  void emitChecksumEntry(const STIChecksumEntry *entry) const;
  void emitLineEntry(const STISymbolProcedure *procedure,
                     const STILineEntry *entry) const;
  void emitLineBlock(const STISymbolProcedure *procedure,
                     const STILineBlock *block) const;
  void emitLineSlice(const STISymbolProcedure *procedure) const;
  void walkSymbol(const STISymbol *symbol) const;
  void emitSymbolModule(const STISymbolModule *module) const;
  void emitSymbolCompileUnit(const STISymbolCompileUnit *compileUnit) const;
  void emitSymbolConstant(const STISymbolConstant *symbol) const;
  void emitSymbolProcedure(const STISymbolProcedure *procedure) const;
  void emitSymbolThunk(const STISymbolThunk *thunk) const;
  void emitSymbolProcedureEnd() const;
  void emitSymbolFrameProc(const STISymbolFrameProc *frame) const;
  void emitSymbolBlock(const STISymbolBlock *block) const;
  void emitSymbolScopeEnd() const;
  void emitSymbolVariable(const STISymbolVariable *variable) const;
  void emitSymbolUserDefined(const STISymbolUserDefined *type) const;

  // Routines for emiting the .debug$T section.
  void emitTypes() const;
  void emitTypesSignature() const;
  void emitTypesPDBTypeServer() const;
  void emitTypesPDBBegin(STIWriter** savedWriter) const;
  void emitTypesPDBEnd(STIWriter** savedWriter) const;
  void emitTypesTable() const;
  void emitType(const STIType *type) const;
  void emitTypeBasic(const STITypeBasic *type) const;
  void emitTypeModifier(const STITypeModifier *type) const;
  void emitTypePointer(const STITypePointer *type) const;
  void emitTypeArray(const STITypeArray *type) const;
  void emitTypeStructure(const STITypeStructure *type) const;
  void emitTypeEnumeration(const STITypeEnumeration *type) const;
  void emitTypeVShape(const STITypeVShape *type) const;
  void emitTypeBitfield(const STITypeBitfield *type) const;
  void emitTypeMethodList(const STITypeMethodList *type) const;
  void emitTypeFieldList(const STITypeFieldList *type) const;
  void emitTypeFieldListItem(const STITypeFieldListItem *field) const;
  void emitTypeFunctionID(const STITypeFunctionID *type) const;
  void emitTypeProcedure(const STITypeProcedure *type) const;
  void emitTypeMemberFunction(const STITypeMemberFunction *type) const;
  void emitTypeArgumentList(const STITypeArgumentList *type) const;
  void emitTypeServer(const STITypeServer *type) const;
  void emitTypeVBaseClass(const STITypeVBaseClass *vBaseClass) const;
  void emitTypeBaseClass(const STITypeBaseClass *baseClass) const;
  void emitTypeVFuncTab(const STITypeVFuncTab *vFuncTab) const;
  void emitTypeMember(const STITypeMember *member) const;
  void emitTypeMethod(const STITypeMethod *method) const;
  void emitTypeOneMethod(const STITypeOneMethod *method) const;
  void emitTypeEnumerator(const STITypeEnumerator *enumerator) const;
  void emitTypeIndex(const STITypeIndex *index) const;
};

//===----------------------------------------------------------------------===//
// STIDebugImpl::_unnamedType
//===----------------------------------------------------------------------===//

const char* STIDebugImpl::_unnamedType = "<unnamed-type>";

//===----------------------------------------------------------------------===//
// STIDebugImpl Public Routines
//===----------------------------------------------------------------------===//

STIDebugImpl::STIDebugImpl(AsmPrinter *asmPrinter) :
    STIDebug            (),
    _asmPrinter         (asmPrinter),
    _currentProcedure   (nullptr),
    _valueHistory       (),
    _functionMap        (),
    _subprogramMap      (),
    _symbolTable        (),
    _typeTable          (),
    _stringTable        (),
    _checksumTable      (),
    _scopeMap           (),
    _typeMap            (),
    _defTypeMap         (),
    _voidType           (nullptr),
    _vbpType            (nullptr),
    _blockNumber        (0),
    _lexicalScopes      (),
    _labelsBeforeInsn   (),
    _labelsAfterInsn    (),
    _curMI              (nullptr),
    _currentSubsection  (nullptr),
    _ptrSizeInBits      (0),
    _classInfoMap       (),
    _stringNameMap      (),
    _uniqueNameCounter  (0),
    _pdbBuff            (),
    _pdbFileName        (),
    _objFileName        (),
    _usePDB             (false),
    _writer             (STIAsmWriter::create(asmPrinter)) {
  // If module doesn't have named metadata anchors or COFF debug section
  // is not available, skip any debug info related stuff.
  if (!MMI()->getModule()->getNamedMetadata("llvm.dbg.cu") ||
      !ASM()->getObjFileLowering().getCOFFDebugSymbolsSection())
    return;

  _ptrSizeInBits = getModule()->getDataLayout().getPointerSizeInBits();

  beginModule();
}

STIDebugImpl::~STIDebugImpl() {
  for (STIType *type : *getTypeTable()) {
    delete type;
  }
  for (auto entry : *getClassInfoMap()) {
    delete entry.second;
  }
  for (const STIDebugFixup* fixup : *getFixupTable()) {
    delete fixup;
  }
  delete _writer;
}

void STIDebugImpl::setSymbolSize(const MCSymbol *Symbol, uint64_t size) {}

void STIDebugImpl::beginModule() {
  _objFileName = getMDStringValue("llvm.dbg.ms.obj");

  // Unless pdb file type is present, we should not use PDB.
  _usePDB = getMDStringValue("llvm.dbg.ms.filetype") == "pdb";
    
  if (usePDB()) {
    _pdbFileName = getMDStringValue("llvm.dbg.ms.pdb");
    if (_pdbFileName.empty()) {
      _pdbFileName = PDB_DEFAULT_FILE_NAME;
    }
    pdb_set_default_dll_name(PDB_DEFAULT_DLL_NAME);
    if (!pdb_open(_pdbFileName.data())) {
      _usePDB = false;
    }
  }

  // Collect all of the initial module information.
  collectModuleInfo();

  // Tell MMI to make the debug information available.
  MMI()->setDebugInfoAvailability(true);
}

void STIDebugImpl::endModule() {
  if (!MMI()->hasDebugInfo())
    return;

  fixup();
  layout();
  emit();

  if (usePDB()) {
    pdb_close();
  }
}

void STIDebugImpl::beginFunction(const MachineFunction *MF) {
  if (!MMI()->hasDebugInfo())
    return;

  STISymbolProcedure *procedure;

  // Locate the symbol for this function.
  FunctionMap::iterator Itr = _functionMap.find(MF->getFunction());
  if (Itr == _functionMap.end()) {
    // If function has no debug info, skip it.
    setCurrentProcedure(nullptr);
    return;
  }

  _lexicalScopes.initialize(*MF);

  // if (_lexicalScopes.empty())
  //  return;

  procedure = Itr->second;
  procedure->setLabelBegin(ASM()->getFunctionBegin());

  // Record this as the current procedure.
  setCurrentProcedure(procedure);

  calculateDbgValueHistory(MF, getTargetRegisterInfo(), _valueHistory);
}

void STIDebugImpl::endFunction(const MachineFunction *MF) {
  if (!MMI()->hasDebugInfo())
    return;

  STISymbolProcedure *procedure = getCurrentProcedure();
  if (procedure == nullptr)
    return;

  // Record the label marking the end of the procedure.
  procedure->setLabelEnd(ASM()->getFunctionEnd());

  // If a routine does not contain a valid machine instruction with a source
  // correlation, then we may not have found an end-of-prologue instruction.
  // In this case set it to the beginning of the procedure.
  //
  if (procedure->getLabelPrologEnd() == nullptr) {
    procedure->setLabelPrologEnd(procedure->getLabelBegin());
  }

  // Collect information about this routine.
  collectRoutineInfo();

  clearValueHistory();
}

void STIDebugImpl::clearValueHistory() { _valueHistory.clear(); }

void STIDebugImpl::beginInstruction(const MachineInstr *MI) {
  DebugLoc location = MI->getDebugLoc();
  STISymbolProcedure *procedure;
  DIScope *scope;
  MCSymbol *label;
  std::string path;
  uint32_t line;
  STIChecksumEntry *checksum;
  STILineSlice *slice;
  STILineBlock *block;
  STILineEntry *entry;

  assert(_curMI == nullptr);

  if (MI->isDebugValue() || getCurrentProcedure() == nullptr) {
    return;
  }

  _curMI = MI;

  if (location == DebugLoc()) {
    label = MMI()->getContext().createTempSymbol();
    emitLabel(label);

    // Handle Scope
    _labelsBeforeInsn.insert(std::make_pair(_curMI, label));
    return;
  }

  procedure = getCurrentProcedure();
  slice = procedure->getLineSlice();
  line = location.getLine();

  scope = cast<DIScope>(location.getScope());
  getFullFileName(scope->getFile(), path);

  label = MMI()->getContext().createTempSymbol();
  emitLabel(label);

  if (slice->getBlocks().size() == 0 ||
      slice->getBlocks().back()->getFilename() != path) {
    checksum = getOrCreateChecksum(path);

    block = STILineBlock::create();
    block->setChecksumEntry(checksum);

    // We don't get source correlation information for the prologue and
    // epilogue.  Visual Studio requires source correlation for the very
    // first instruction in the routine or it thinks there is no debug
    // information available and steps over the routine.  The following
    // code is a hack to correlate the prologue with the first line
    // number which occurs in the routine.  This should be fixed in LLVM
    // to propagate the source correlation for the opening curly brace.
    //
    if (slice->getBlocks().size() == 0) {
      entry = STILineEntry::create();
      entry->setLabel(procedure->getLabelBegin());
      entry->setLineNumStart(procedure->getScopeLineNumber());
      block->appendLine(entry);
    }

    slice->appendBlock(block);
  }

  block = slice->getBlocks().back();

  entry = block->getLines().empty() ? nullptr : block->getLines().back();
  if (line != 0 && (entry == nullptr || entry->getLineNumStart() != line)) {
    entry = STILineEntry::create();
    entry->setLabel(label);
    entry->setLineNumStart(line);

    block->appendLine(entry);
  }

  if (!MI->getFlag(MachineInstr::FrameSetup) &&
      procedure->getLabelPrologEnd() == nullptr) {
    procedure->setLabelPrologEnd(label);
  }

  // Handle Scope
  _labelsBeforeInsn.insert(std::make_pair(_curMI, label));
}

void STIDebugImpl::endInstruction() {
  MCSymbol *label;

  if (_curMI == nullptr) {
    return;
  }

  label = MMI()->getContext().createTempSymbol();
  emitLabel(label);

  // Handle Scope
  _labelsAfterInsn.insert(std::make_pair(_curMI, label));

  _curMI = nullptr;
}

STISymbolProcedure *STIDebugImpl::getCurrentProcedure() const {
  return _currentProcedure;
}

void STIDebugImpl::setCurrentProcedure(STISymbolProcedure *procedure) {
  _currentProcedure = procedure;
}

AsmPrinter *STIDebugImpl::ASM() { return _asmPrinter; }

const AsmPrinter *STIDebugImpl::ASM() const { return _asmPrinter; }

MachineModuleInfo *STIDebugImpl::MMI() const { return ASM()->MMI; }

const Module *STIDebugImpl::getModule() const { return MMI()->getModule(); }

const TargetRegisterInfo *STIDebugImpl::getTargetRegisterInfo() {
  return ASM()->MF->getSubtarget().getRegisterInfo();
}

STISymbolTable *STIDebugImpl::getSymbolTable() { return &_symbolTable; }

const STISymbolTable *STIDebugImpl::getSymbolTable() const {
  return &_symbolTable;
}

STITypeTable *STIDebugImpl::getTypeTable() { return &_typeTable; }

const STITypeTable *STIDebugImpl::getTypeTable() const { return &_typeTable; }

STIStringTable *STIDebugImpl::getStringTable() { return &_stringTable; }

const STIStringTable *STIDebugImpl::getStringTable() const {
  return &_stringTable;
}

STIChecksumTable *STIDebugImpl::getChecksumTable() { return &_checksumTable; }

const STIChecksumTable *STIDebugImpl::getChecksumTable() const {
  return &_checksumTable;
}

STIDebugFixupTable *STIDebugImpl::getFixupTable() {
  return &_fixupTable;
}

void STIDebugImpl::appendFixup(STIDebugFixup* fixup) {
  _fixupTable.push_back(fixup);
}

bool STIDebugImpl::hasScope(const MDNode *llvmNode) const {
  return _scopeMap.count(llvmNode);
}

STIScope *STIDebugImpl::getScope(const MDNode *llvmNode) {
  assert(hasScope(llvmNode) && "LLVM node has no STI object mapped yet!");
  return _scopeMap[llvmNode];
}

void STIDebugImpl::addScope(const MDNode *llvmNode, STIScope *object) {
  assert(!hasScope(llvmNode) && "LLVM node already mapped to STI object!");
  _scopeMap[llvmNode] = object;
}

STITypeMap *STIDebugImpl::getTypeMap(const DIType *llvmClass) {
  return &_typeMap[llvmClass];
}

STIDclToDefTypeMap *STIDebugImpl::getDefTypeMap() { return &_defTypeMap; }

const STIDclToDefTypeMap *STIDebugImpl::getDefTypeMap() const {
  return &_defTypeMap;
}

STIDebugImpl::ClassInfoMap *STIDebugImpl::getClassInfoMap() {
  return &_classInfoMap;
}

const STIDebugImpl::ClassInfoMap *STIDebugImpl::getClassInfoMap() const {
  return &_classInfoMap;
}

STIDebugImpl::StringNameMap *STIDebugImpl::getStringNameMap() {
  return &_stringNameMap;
}

const STIDebugImpl::StringNameMap *STIDebugImpl::getStringNameMap() const {
  return &_stringNameMap;
}

STIWriter* STIDebugImpl::writer() const {
  return _writer;
}

void STIDebugImpl::setWriter(STIWriter* writer) {
  _writer = writer;
}

std::string STIDebugImpl::getUniqueName() {
  return (Twine("<unnamed-tag>") + Twine(_uniqueNameCounter++)).str();
}

//===----------------------------------------------------------------------===//
// toSTIRegID(llvmID)
//
// Maps the specified llvm register identifier to an equivalent STI register
// identifier.
//
// When the compiler is built, the X86 definitions for llvmID are defined in:
//   * builds/xmain[variant]/llvm/lib/Target/X86/X86GenRegisterInfo.inc
//
//===----------------------------------------------------------------------===//

STIRegID STIDebugImpl::toSTIRegID(unsigned int llvmID) const {
  STIRegID stiID;

  switch (llvmID) {
#define MAP(LLVMID, STIID) case LLVMID: stiID = STIID; break
    MAP(0x01, STI_REGISTER_AH);
    MAP(0x02, STI_REGISTER_AL);
    MAP(0x03, STI_REGISTER_AX);
    MAP(0x04, STI_REGISTER_BH);
    MAP(0x05, STI_REGISTER_BL);
    MAP(0x06, STI_REGISTER_BP);
    MAP(0x07, STI_REGISTER_BPL);
    MAP(0x08, STI_REGISTER_BX);
    MAP(0x09, STI_REGISTER_CH);
    MAP(0x0a, STI_REGISTER_CL);
    MAP(0x0b, STI_REGISTER_CS);
    MAP(0x0c, STI_REGISTER_CX);
    MAP(0x0d, STI_REGISTER_DH);
    MAP(0x0e, STI_REGISTER_DI);
    MAP(0x0f, STI_REGISTER_DIL);
    MAP(0x10, STI_REGISTER_DL);
    MAP(0x11, STI_REGISTER_DS);
    MAP(0x12, STI_REGISTER_DX);
    MAP(0x13, STI_REGISTER_EAX);
    MAP(0x14, STI_REGISTER_EBP);
    MAP(0x15, STI_REGISTER_EBX);
    MAP(0x16, STI_REGISTER_ECX);
    MAP(0x17, STI_REGISTER_EDI);
    MAP(0x18, STI_REGISTER_EDX);
    MAP(0x19, STI_REGISTER_EFLAGS);
 // MAP(0x1a, STI_REGISTER_EIP);
 // MAP(0x1b, STI_REGISTER_EIZ);
    MAP(0x1c, STI_REGISTER_ES);
    MAP(0x1d, STI_REGISTER_ESI);
    MAP(0x1e, STI_REGISTER_ESP);
 // MAP(0x1f, STI_REGISTER_FPSW);
    MAP(0x20, STI_REGISTER_FS);
    MAP(0x21, STI_REGISTER_GS);
 // MAP(0x22, STI_REGISTER_IP);
    MAP(0x23, STI_REGISTER_RAX);
    MAP(0x24, STI_REGISTER_RBP);
    MAP(0x25, STI_REGISTER_RBX);
    MAP(0x26, STI_REGISTER_RCX);
    MAP(0x27, STI_REGISTER_RDI);
    MAP(0x28, STI_REGISTER_RDX);
 // MAP(0x29, STI_REGISTER_RIP);
 // MAP(0x2a, STI_REGISTER_RIZ);
    MAP(0x2b, STI_REGISTER_RSI);
    MAP(0x2c, STI_REGISTER_RSP);
    MAP(0x2d, STI_REGISTER_SI);
    MAP(0x2e, STI_REGISTER_SIL);
    MAP(0x2f, STI_REGISTER_SP);
    MAP(0x30, STI_REGISTER_SPL);
    MAP(0x31, STI_REGISTER_SS);

//  MAP(0x32, STI_REGISTER_BND0);
//  MAP(0x33, STI_REGISTER_BND1);
//  MAP(0x34, STI_REGISTER_BND2);
//  MAP(0x35, STI_REGISTER_BND3);

    MAP(0x36, STI_REGISTER_CR0);
    MAP(0x37, STI_REGISTER_CR1);
    MAP(0x38, STI_REGISTER_CR2);
    MAP(0x39, STI_REGISTER_CR3);
    MAP(0x3a, STI_REGISTER_CR4);
 // MAP(0x3b, STI_REGISTER_CR5);
 // MAP(0x3c, STI_REGISTER_CR6);
 // MAP(0x3d, STI_REGISTER_CR7);
    MAP(0x3e, STI_REGISTER_CR8);
 // MAP(0x3f, STI_REGISTER_CR9);
 // MAP(0x40, STI_REGISTER_CR10);
 // MAP(0x41, STI_REGISTER_CR11);
 // MAP(0x42, STI_REGISTER_CR12);
 // MAP(0x43, STI_REGISTER_CR13);
 // MAP(0x44, STI_REGISTER_CR14);
 // MAP(0x45, STI_REGISTER_CR15);
    MAP(0x46, STI_REGISTER_DR0);
    MAP(0x47, STI_REGISTER_DR1);
    MAP(0x48, STI_REGISTER_DR2);
    MAP(0x49, STI_REGISTER_DR3);
    MAP(0x4a, STI_REGISTER_DR4);
    MAP(0x4b, STI_REGISTER_DR5);
    MAP(0x4c, STI_REGISTER_DR6);
    MAP(0x4d, STI_REGISTER_DR7);
    MAP(0x4e, STI_REGISTER_DR8);
    MAP(0x4f, STI_REGISTER_DR9);
    MAP(0x50, STI_REGISTER_DR10);
    MAP(0x51, STI_REGISTER_DR11);
    MAP(0x52, STI_REGISTER_DR12);
    MAP(0x53, STI_REGISTER_DR13);
    MAP(0x54, STI_REGISTER_DR14);
    MAP(0x55, STI_REGISTER_DR15);

 // MAP(0x56, STI_REGISTER_FP0);
 // MAP(0x57, STI_REGISTER_FP1);
 // MAP(0x58, STI_REGISTER_FP2);
 // MAP(0x59, STI_REGISTER_FP3);
 // MAP(0x5a, STI_REGISTER_FP4);
 // MAP(0x5b, STI_REGISTER_FP5);
 // MAP(0x5c, STI_REGISTER_FP6);
 // MAP(0x5d, STI_REGISTER_FP7);

 // MAP(0x5e, STI_REGISTER_K0);
 // MAP(0x5f, STI_REGISTER_K1);
 // MAP(0x60, STI_REGISTER_K2);
 // MAP(0x61, STI_REGISTER_K3);
 // MAP(0x62, STI_REGISTER_K4);
 // MAP(0x63, STI_REGISTER_K5);
 // MAP(0x64, STI_REGISTER_K6);
 // MAP(0x65, STI_REGISTER_K7);

    MAP(0x66, STI_REGISTER_MM0);
    MAP(0x67, STI_REGISTER_MM1);
    MAP(0x68, STI_REGISTER_MM2);
    MAP(0x69, STI_REGISTER_MM3);
    MAP(0x6a, STI_REGISTER_MM4);
    MAP(0x6b, STI_REGISTER_MM5);
    MAP(0x6c, STI_REGISTER_MM6);
    MAP(0x6d, STI_REGISTER_MM7);

    MAP(0x6e, STI_REGISTER_R8);
    MAP(0x6f, STI_REGISTER_R9);
    MAP(0x70, STI_REGISTER_R10);
    MAP(0x71, STI_REGISTER_R11);
    MAP(0x72, STI_REGISTER_R12);
    MAP(0x73, STI_REGISTER_R13);
    MAP(0x74, STI_REGISTER_R14);
    MAP(0x75, STI_REGISTER_R15);

    MAP(0x76, STI_REGISTER_ST0);
    MAP(0x77, STI_REGISTER_ST1);
    MAP(0x78, STI_REGISTER_ST2);
    MAP(0x79, STI_REGISTER_ST3);
    MAP(0x7a, STI_REGISTER_ST4);
    MAP(0x7b, STI_REGISTER_ST5);
    MAP(0x7c, STI_REGISTER_ST6);
    MAP(0x7d, STI_REGISTER_ST7);

    MAP(0x7e, STI_REGISTER_XMM0);
    MAP(0x7f, STI_REGISTER_XMM1);
    MAP(0x80, STI_REGISTER_XMM2);
    MAP(0x81, STI_REGISTER_XMM3);
    MAP(0x82, STI_REGISTER_XMM4);
    MAP(0x83, STI_REGISTER_XMM5);
    MAP(0x84, STI_REGISTER_XMM6);
    MAP(0x85, STI_REGISTER_XMM7);
    MAP(0x86, STI_REGISTER_XMM8);
    MAP(0x87, STI_REGISTER_XMM9);
    MAP(0x88, STI_REGISTER_XMM10);
    MAP(0x89, STI_REGISTER_XMM11);
    MAP(0x8a, STI_REGISTER_XMM12);
    MAP(0x8b, STI_REGISTER_XMM13);
    MAP(0x8c, STI_REGISTER_XMM14);
    MAP(0x8d, STI_REGISTER_XMM15);
//  MAP(0x8e, STI_REGISTER_XMM16);
//  MAP(0x8f, STI_REGISTER_XMM17);
//  MAP(0x90, STI_REGISTER_XMM18);
//  MAP(0x91, STI_REGISTER_XMM19);
//  MAP(0x92, STI_REGISTER_XMM20);
//  MAP(0x93, STI_REGISTER_XMM21);
//  MAP(0x94, STI_REGISTER_XMM22);
//  MAP(0x95, STI_REGISTER_XMM23);
//  MAP(0x96, STI_REGISTER_XMM24);
//  MAP(0x97, STI_REGISTER_XMM25);
//  MAP(0x98, STI_REGISTER_XMM26);
//  MAP(0x99, STI_REGISTER_XMM27);
//  MAP(0x9a, STI_REGISTER_XMM28);
//  MAP(0x9b, STI_REGISTER_XMM29);
//  MAP(0x9c, STI_REGISTER_XMM30);
//  MAP(0x9dj STI_REGISTER_XMM31);

//  MAP(0x9e, STI_REGISTER_YMM0);
//  MAP(0x9f, STI_REGISTER_YMM1);
//  MAP(0xa0, STI_REGISTER_YMM2);
//  MAP(0xa1, STI_REGISTER_YMM3);
//  MAP(0xa2, STI_REGISTER_YMM4);
//  MAP(0xa3, STI_REGISTER_YMM5);
//  MAP(0xa4, STI_REGISTER_YMM6);
//  MAP(0xa5, STI_REGISTER_YMM7);
//  MAP(0xa6, STI_REGISTER_YMM8);
//  MAP(0xa7, STI_REGISTER_YMM9);
//  MAP(0xa8, STI_REGISTER_YMM10);
//  MAP(0xa9, STI_REGISTER_YMM11);
//  MAP(0xaa, STI_REGISTER_YMM12);
//  MAP(0xab, STI_REGISTER_YMM13);
//  MAP(0xac, STI_REGISTER_YMM14);
//  MAP(0xad, STI_REGISTER_YMM15);
//  MAP(0xae, STI_REGISTER_YMM16);
//  MAP(0xaf, STI_REGISTER_YMM17);
//  MAP(0xb0, STI_REGISTER_YMM18);
//  MAP(0xb1, STI_REGISTER_YMM19);
//  MAP(0xb2, STI_REGISTER_YMM20);
//  MAP(0xb3, STI_REGISTER_YMM21);
//  MAP(0xb4, STI_REGISTER_YMM22);
//  MAP(0xb5, STI_REGISTER_YMM23);
//  MAP(0xb6, STI_REGISTER_YMM24);
//  MAP(0xb7, STI_REGISTER_YMM25);
//  MAP(0xb8, STI_REGISTER_YMM26);
//  MAP(0xb9, STI_REGISTER_YMM27);
//  MAP(0xba, STI_REGISTER_YMM28);
//  MAP(0xbb, STI_REGISTER_YMM29);
//  MAP(0xbc, STI_REGISTER_YMM30);
//  MAP(0xbd, STI_REGISTER_YMM31);

//  MAP(0xbe, STI_REGISTER_ZMM0);
//  MAP(0xbf, STI_REGISTER_ZMM1);
//  MAP(0xc0, STI_REGISTER_ZMM2);
//  MAP(0xc1, STI_REGISTER_ZMM3);
//  MAP(0xc2, STI_REGISTER_ZMM4);
//  MAP(0xc3, STI_REGISTER_ZMM5);
//  MAP(0xc4, STI_REGISTER_ZMM6);
//  MAP(0xc5, STI_REGISTER_ZMM7);
//  MAP(0xc6, STI_REGISTER_ZMM8);
//  MAP(0xc7, STI_REGISTER_ZMM9);
//  MAP(0xc8, STI_REGISTER_ZMM10);
//  MAP(0xc9, STI_REGISTER_ZMM11);
//  MAP(0xca, STI_REGISTER_ZMM12);
//  MAP(0xcb, STI_REGISTER_ZMM13);
//  MAP(0xcc, STI_REGISTER_ZMM14);
//  MAP(0xcd, STI_REGISTER_ZMM15);
//  MAP(0xce, STI_REGISTER_ZMM16);
//  MAP(0xcf, STI_REGISTER_ZMM17);
//  MAP(0xd0, STI_REGISTER_ZMM18);
//  MAP(0xd1, STI_REGISTER_ZMM19);
//  MAP(0xd2, STI_REGISTER_ZMM20);
//  MAP(0xd3, STI_REGISTER_ZMM21);
//  MAP(0xd4, STI_REGISTER_ZMM22);
//  MAP(0xd5, STI_REGISTER_ZMM23);
//  MAP(0xd6, STI_REGISTER_ZMM24);
//  MAP(0xd7, STI_REGISTER_ZMM25);
//  MAP(0xd8, STI_REGISTER_ZMM26);
//  MAP(0xd9, STI_REGISTER_ZMM27);
//  MAP(0xda, STI_REGISTER_ZMM28);
//  MAP(0xdb, STI_REGISTER_ZMM29);
//  MAP(0xdc, STI_REGISTER_ZMM30);
//  MAP(0xdd, STI_REGISTER_ZMM31);

    MAP(0xde, STI_REGISTER_R8B);
    MAP(0xdf, STI_REGISTER_R9B);
    MAP(0xe0, STI_REGISTER_R10B);
    MAP(0xe1, STI_REGISTER_R11B);
    MAP(0xe2, STI_REGISTER_R12B);
    MAP(0xe3, STI_REGISTER_R13B);
    MAP(0xe4, STI_REGISTER_R14B);
    MAP(0xe5, STI_REGISTER_R15B);

    MAP(0xe6, STI_REGISTER_R8D);
    MAP(0xe7, STI_REGISTER_R9D);
    MAP(0xe8, STI_REGISTER_R10D);
    MAP(0xe9, STI_REGISTER_R11D);
    MAP(0xea, STI_REGISTER_R12D);
    MAP(0xeb, STI_REGISTER_R13D);
    MAP(0xec, STI_REGISTER_R14D);
    MAP(0xed, STI_REGISTER_R15D);

    MAP(0xee, STI_REGISTER_R8W);
    MAP(0xef, STI_REGISTER_R9W);
    MAP(0xf0, STI_REGISTER_R10W);
    MAP(0xf1, STI_REGISTER_R11W);
    MAP(0xf2, STI_REGISTER_R12W);
    MAP(0xf3, STI_REGISTER_R13W);
    MAP(0xf4, STI_REGISTER_R14W);
    MAP(0xf5, STI_REGISTER_R15W);

#undef MAP
  default:
    assert(llvmID != llvmID); // unrecognized llvm register number
    break;
  }

  return stiID;
}

//===----------------------------------------------------------------------===//
// PRIMITIVE_BASIC_TYPE_MAPPINGS
//
// Maps LLVM basic types to STI primitive type encodings.
//
//===----------------------------------------------------------------------===//

#define PRIMITIVE_BASIC_TYPE_MAPPINGS                                         \
  X(dwarf::DW_ATE_address,          4,      T_32PVOID)                        \
  X(dwarf::DW_ATE_boolean,          1,      T_BOOL08)                         \
  X(dwarf::DW_ATE_boolean,          2,      T_BOOL16)                         \
  X(dwarf::DW_ATE_boolean,          4,      T_BOOL32)                         \
  X(dwarf::DW_ATE_boolean,          8,      T_BOOL64)                         \
  X(dwarf::DW_ATE_complex_float,    4,      T_CPLX32)                         \
  X(dwarf::DW_ATE_complex_float,    8,      T_CPLX64)                         \
  X(dwarf::DW_ATE_complex_float,   10,      T_CPLX80)                         \
  X(dwarf::DW_ATE_complex_float,   16,      T_CPLX128)                        \
  X(dwarf::DW_ATE_float,            4,      T_REAL32)                         \
  X(dwarf::DW_ATE_float,            6,      T_REAL48)                         \
  X(dwarf::DW_ATE_float,            8,      T_REAL64)                         \
  X(dwarf::DW_ATE_float,           10,      T_REAL80)                         \
  X(dwarf::DW_ATE_float,           16,      T_REAL128)                        \
  X(dwarf::DW_ATE_decimal_float,    4,      T_REAL32)                         \
  X(dwarf::DW_ATE_decimal_float,    6,      T_REAL48)                         \
  X(dwarf::DW_ATE_decimal_float,    8,      T_REAL64)                         \
  X(dwarf::DW_ATE_decimal_float,   10,      T_REAL80)                         \
  X(dwarf::DW_ATE_decimal_float,   16,      T_REAL128)                        \
  X(dwarf::DW_ATE_signed,           1,      T_CHAR)                           \
  X(dwarf::DW_ATE_signed,           2,      T_SHORT)                          \
  X(dwarf::DW_ATE_signed,           4,      T_INT4)                           \
  X(dwarf::DW_ATE_signed,           8,      T_QUAD)                           \
  X(dwarf::DW_ATE_signed_char,      1,      T_CHAR)                           \
  X(dwarf::DW_ATE_unsigned,         1,      T_UCHAR)                          \
  X(dwarf::DW_ATE_unsigned,         2,      T_USHORT)                         \
  X(dwarf::DW_ATE_unsigned,         4,      T_UINT4)                          \
  X(dwarf::DW_ATE_unsigned,         8,      T_UQUAD)                          \
  X(dwarf::DW_ATE_unsigned_char,    1,      T_UCHAR)
// FIXME: dwarf::DW_ATE_imaginary_float
// FIXME: dwarf::DW_ATE_packed_decimal
// FIXME: dwarf::DW_ATE_numeric_string
// FIXME: dwarf::DW_ATE_edited
// FIXME: dwarf::DW_ATE_signed_fixed
// FIXME: dwarf::DW_ATE_unsigned_fixed
// FIXME: dwarf::DW_ATE_UTF

//===----------------------------------------------------------------------===//
// PRIMITIVE_POINTER_TYPE_MAPPINGS
//
// Maps LLVM pointers referencing basic types to primitive type encodings.
//
//===----------------------------------------------------------------------===//

#define PRIMITIVE_POINTER_TYPE_MAPPINGS                                       \
  X(dwarf::DW_ATE_boolean,          1,   4, T_32PBOOL08)                      \
  X(dwarf::DW_ATE_boolean,          2,   4, T_32PBOOL16)                      \
  X(dwarf::DW_ATE_boolean,          4,   4, T_32PBOOL32)                      \
  X(dwarf::DW_ATE_boolean,          8,   4, T_32PBOOL64)                      \
  X(dwarf::DW_ATE_complex_float,    4,   4, T_32PCPLX32)                      \
  X(dwarf::DW_ATE_complex_float,    8,   4, T_32PCPLX64)                      \
  X(dwarf::DW_ATE_complex_float,   10,   4, T_32PCPLX80)                      \
  X(dwarf::DW_ATE_complex_float,   16,   4, T_32PCPLX128)                     \
  X(dwarf::DW_ATE_float,            4,   4, T_32PREAL32)                      \
  X(dwarf::DW_ATE_float,            6,   4, T_32PREAL48)                      \
  X(dwarf::DW_ATE_float,            8,   4, T_32PREAL64)                      \
  X(dwarf::DW_ATE_float,           10,   4, T_32PREAL80)                      \
  X(dwarf::DW_ATE_float,           16,   4, T_32PREAL128)                     \
  X(dwarf::DW_ATE_decimal_float,    4,   4, T_32PREAL32)                      \
  X(dwarf::DW_ATE_decimal_float,    6,   4, T_32PREAL48)                      \
  X(dwarf::DW_ATE_decimal_float,    8,   4, T_32PREAL64)                      \
  X(dwarf::DW_ATE_decimal_float,   10,   4, T_32PREAL80)                      \
  X(dwarf::DW_ATE_decimal_float,   16,   4, T_32PREAL128)                     \
  X(dwarf::DW_ATE_signed,           1,   4, T_32PCHAR)                        \
  X(dwarf::DW_ATE_signed,           2,   4, T_32PSHORT)                       \
  X(dwarf::DW_ATE_signed,           4,   4, T_32PINT4)                        \
  X(dwarf::DW_ATE_signed,           8,   4, T_32PQUAD)                        \
  X(dwarf::DW_ATE_signed_char,      1,   4, T_32PCHAR)                        \
  X(dwarf::DW_ATE_unsigned,         1,   4, T_32PUCHAR)                       \
  X(dwarf::DW_ATE_unsigned,         2,   4, T_32PUSHORT)                      \
  X(dwarf::DW_ATE_unsigned,         4,   4, T_32PUINT4)                       \
  X(dwarf::DW_ATE_unsigned,         8,   4, T_32PUQUAD)                       \
  X(dwarf::DW_ATE_unsigned_char,    1,   4, T_32PUCHAR)                       \
                                                                              \
  X(dwarf::DW_ATE_boolean,          1,   8, T_64PBOOL08)                      \
  X(dwarf::DW_ATE_boolean,          2,   8, T_64PBOOL16)                      \
  X(dwarf::DW_ATE_boolean,          4,   8, T_64PBOOL32)                      \
  X(dwarf::DW_ATE_boolean,          8,   8, T_64PBOOL64)                      \
  X(dwarf::DW_ATE_complex_float,    4,   8, T_64PCPLX32)                      \
  X(dwarf::DW_ATE_complex_float,    8,   8, T_64PCPLX64)                      \
  X(dwarf::DW_ATE_complex_float,   10,   8, T_64PCPLX80)                      \
  X(dwarf::DW_ATE_complex_float,   16,   8, T_64PCPLX128)                     \
  X(dwarf::DW_ATE_float,            4,   8, T_64PREAL32)                      \
  X(dwarf::DW_ATE_float,            6,   8, T_64PREAL48)                      \
  X(dwarf::DW_ATE_float,            8,   8, T_64PREAL64)                      \
  X(dwarf::DW_ATE_float,           10,   8, T_64PREAL80)                      \
  X(dwarf::DW_ATE_float,           16,   8, T_64PREAL128)                     \
  X(dwarf::DW_ATE_decimal_float,    4,   8, T_64PREAL32)                      \
  X(dwarf::DW_ATE_decimal_float,    6,   8, T_64PREAL48)                      \
  X(dwarf::DW_ATE_decimal_float,    8,   8, T_64PREAL64)                      \
  X(dwarf::DW_ATE_decimal_float,   10,   8, T_64PREAL80)                      \
  X(dwarf::DW_ATE_decimal_float,   16,   8, T_64PREAL128)                     \
  X(dwarf::DW_ATE_signed,           1,   8, T_64PCHAR)                        \
  X(dwarf::DW_ATE_signed,           2,   8, T_64PSHORT)                       \
  X(dwarf::DW_ATE_signed,           4,   8, T_64PINT4)                        \
  X(dwarf::DW_ATE_signed,           8,   8, T_64PQUAD)                        \
  X(dwarf::DW_ATE_signed_char,      1,   8, T_64PCHAR)                        \
  X(dwarf::DW_ATE_unsigned,         1,   8, T_64PUCHAR)                       \
  X(dwarf::DW_ATE_unsigned,         2,   8, T_64PUSHORT)                      \
  X(dwarf::DW_ATE_unsigned,         4,   8, T_64PUINT4)                       \
  X(dwarf::DW_ATE_unsigned,         8,   8, T_64PUQUAD)                       \
  X(dwarf::DW_ATE_unsigned_char,    1,   8, T_64PUCHAR)

/// If this type is derived from a base type then return base type size.
uint64_t STIDebugImpl::getBaseTypeSize(const DIDerivedType *Ty) const {
  unsigned Tag = Ty->getTag();

  if (Tag != dwarf::DW_TAG_member && Tag != dwarf::DW_TAG_typedef &&
      Tag != dwarf::DW_TAG_const_type && Tag != dwarf::DW_TAG_volatile_type &&
      Tag != dwarf::DW_TAG_restrict_type)
    return Ty->getSizeInBits();

  DIType *BaseType = resolve(Ty->getBaseType());

  // If this type is not derived from any type or the type is a declaration then
  // take conservative approach.
  if (!BaseType || BaseType->isForwardDecl())
    return Ty->getSizeInBits();

  // If this is a derived type, go ahead and get the base type, unless it's a
  // reference then it's just the size of the field. Pointer types have no need
  // of this since they're a different type of qualification on the type.
  if (BaseType->getTag() == dwarf::DW_TAG_reference_type ||
      BaseType->getTag() == dwarf::DW_TAG_rvalue_reference_type)
    return Ty->getSizeInBits();

  if (DIDerivedType *DT = dyn_cast<DIDerivedType>(BaseType)) {
    return getBaseTypeSize(DT);
  }

  return BaseType->getSizeInBits();
}

bool STIDebugImpl::isEqualVMethodPrototype(const DISubroutineType *typeA,
                                           const DISubroutineType *typeB) {
  DITypeRefArray ElementsA = typeA->getTypeArray();
  DITypeRefArray ElementsB = typeB->getTypeArray();

  if (ElementsA.size() != ElementsB.size()) {
    return false;
  }

  assert(ElementsA.size() >= 2 && "non-trevial method");

  for (unsigned i = 2, N = ElementsA.size(); i < N; ++i) {
    const DIType *ElementA = resolve(ElementsA[i]);
    const DIType *ElementB = resolve(ElementsB[i]);
    if (ElementA != ElementB) {
      return false;
    }
  }
  return true;
}

void STIDebugImpl::collectClassInfoFromInheritance(ClassInfo &info,
                                                   const DIDerivedType *inherTy,
                                                   bool &finalizedOffset) {
  bool isVirtual = inherTy->isVirtual();

  const DIType *BaseTy = resolve(inherTy->getBaseType());

  // Base type of inheritance entry might be a typedef entry.
  // Skip all typedef entries to get to the class entry.
  while (!isa<DICompositeType>(BaseTy)) {
    assert(isa<DIDerivedType>(BaseTy) && "Base type expected to be derived type");
    const DIDerivedType *DTy = cast<DIDerivedType>(BaseTy);
    assert(DTy->getTag() == dwarf::DW_TAG_typedef);
    BaseTy = resolve(DTy->getBaseType());
  }
  const DICompositeType *DDTy = dyn_cast<DICompositeType>(BaseTy);
  ClassInfo &inherInfo = collectClassInfo(DDTy);

  for (auto &itr : inherInfo.vBaseClasses) {
    if (!info.vBaseClasses.count(itr.first)) {
      int vbIndex = info.vBaseClasses.size() + 1;
      info.vBaseClasses[itr.first] = ClassInfo::VBaseClassInfo(
          itr.second.llvmInheritance, vbIndex, true /*indirect*/);
    }
  }

  if (isVirtual) {
    auto vbClass = info.vBaseClasses.find(DDTy);
    if (vbClass != info.vBaseClasses.end()) {
      vbClass->second.indirect = false;
    } else {
      int vbIndex = info.vBaseClasses.size() + 1;
      info.vBaseClasses[DDTy] =
          ClassInfo::VBaseClassInfo(inherTy, vbIndex, false /*indirect*/);
    }
  } else {
    if (!finalizedOffset) {
      if (inherInfo.vBaseClasses.size()) {
        finalizedOffset = true;
        info.vbpOffset = (inherTy->getOffsetInBits() >> 3) + inherInfo.vbpOffset;
        info.vMethodsCount = inherInfo.vMethodsCount;
      } else {
        info.vbpOffset = (inherTy->getOffsetInBits() + DDTy->getSizeInBits()) >> 3;
      }
    }
    info.baseClasses.push_back(inherTy);
  }

  // append "inherInfo.vMethods" to "info.vMethods"
  for (auto &itr : inherInfo.vMethods) {
    StringRef methodName = itr.first;
    auto &vMethodsDst = info.vMethods[methodName];

    for (unsigned i = 0, Ni = itr.second.size(); i < Ni; ++i) {
      const DISubroutineType *SPTy =
          dyn_cast<const DISubroutineType>(itr.second[i]);
      bool found = false;
      for (unsigned j = 0, Nj = vMethodsDst.size(); j < Nj; ++j) {
        if (isEqualVMethodPrototype(
            dyn_cast<const DISubroutineType>(vMethodsDst[j]),
            SPTy)) {
          // virtual method is not introduced.
          found = true;
          break;
        }
      }
      if (!found) {
        vMethodsDst.push_back(SPTy);
      }
    }
  }
}

void STIDebugImpl::collectMemberInfo(ClassInfo &info,
                                     const DIDerivedType *DDTy) {
  if (!DDTy->getName().empty()) {
    info.members.push_back(std::make_pair(DDTy, 0));
    return;
  }
  // Member with no name, must be nested structure/union, collects its memebers
  assert((DDTy->getOffsetInBits() % 8) == 0 && "Unnamed bitfield member!");
  unsigned offset = DDTy->getOffsetInBits() >> 3;
  const DIType *Ty = resolve(DDTy->getBaseType());
  assert(dyn_cast<DICompositeType>(Ty) && "Expects structure or union type");
  const DICompositeType *DCTy = dyn_cast<DICompositeType>(Ty);
  ClassInfo &nestedInfo = collectClassInfo(DCTy);
  ClassInfo::MemberList &members = nestedInfo.members;
  for (unsigned i = 0, e = members.size(); i != e; ++i) {
    auto itr = members[i];
    info.members.push_back(std::make_pair(itr.first, itr.second + offset));
  }
  //TODO: do we need to create the type of the unnamed member?
  //(void)lowerType(Ty);
}

//===----------------------------------------------------------------------===//
// stripScopesFromName(name)
//
// Subprogram names used to contain only the name, now they are fully
// qualified.  We need to strip off the qualification to get the bare name.
//
// FIXME: The names should be fixed in CLANG and then this can be removed.
//
//===----------------------------------------------------------------------===//

static StringRef stripScopesFromName(StringRef name) {
  StringRef::size_type pos;
  unsigned int         template_level;

  if (name.empty())
    return name;
  
  for (pos = name.size() - 1, template_level = 0; pos > 1; --pos) {
    switch (name[pos]) {
    case '>':
      template_level++;
      break;
    case '<':
      if (template_level > 0)
        template_level--;
      break;
    case ':':
      if (template_level == 0 && name[pos - 1] == ':')
        return name.substr(pos + 1);
      break;
    default:
      break;
    }
  }

  return name;
}

ClassInfo &STIDebugImpl::collectClassInfo(const DICompositeType *llvmType) {
  auto *CIM = getClassInfoMap();
  auto itr = CIM->find(llvmType);
  if (itr != CIM->end()) {
    return *itr->second;
  }

  CIM->insert(std::make_pair(llvmType, new ClassInfo()));
  ClassInfo &info = *(CIM->find(llvmType)->second);

  std::string constructorName = llvmType->getName();
  std::string destructorName = (Twine("~") + Twine(llvmType->getName())).str();
  std::string virtualTableName =
      (Twine("_vptr$") + Twine(llvmType->getName())).str();

  bool finalizedOffset = false;

  // Add elements to structure type.
  DINodeArray Elements = llvmType->getElements();
  for (unsigned i = 0, N = Elements.size(); i < N; ++i) {
    DINode *Element = Elements[i];
    if (const DISubprogram *subprogram = dyn_cast<DISubprogram>(Element)) {
      // FIXME: implement this case
      // getOrCreateSubprogramDIE(Element);
      StringRef methodName = stripScopesFromName(subprogram->getName());
      info.methods[methodName].push_back(
          std::make_pair(subprogram, true /*introduced*/));

      if (methodName == constructorName)
        info.hasCTOR = true;
      if (methodName == destructorName)
        info.hasDTOR = true;

    } else if (DIDerivedType *DDTy = dyn_cast<DIDerivedType>(Element)) {
      if (DDTy->getTag() == dwarf::DW_TAG_friend) {
        // FIXME: implement this case
        // DIE &ElemDie = createAndAddDIE(dwarf::DW_TAG_friend, Buffer);
        // addType(ElemDie, resolve(DDTy->getBaseType()),
        //        dwarf::DW_AT_friend);
        assert(!"FIXME: implement this case");
      } else {
        if (DDTy->getName() == virtualTableName) {
          assert(!info.vFuncTab && "Class has more than one virtual table.");
          info.vFuncTab = DDTy;
        } else if (DDTy->getTag() == dwarf::DW_TAG_inheritance) {
          collectClassInfoFromInheritance(info, DDTy, finalizedOffset);
        } else {
          collectMemberInfo(info, DDTy);
        }
      }
    }
  }
  bool hasVFuncTab = false;
  for (auto &itr : info.methods) {
    StringRef methodName = itr.first;
    if (methodName == destructorName) {
      methodName = "~";
    }

    auto &vMethods = info.vMethods[methodName];
    for (unsigned i = 0, Ni = itr.second.size(); i < Ni; ++i) {
      auto &methodInfo = itr.second[i];
      const DISubprogram *subprogram = dyn_cast<DISubprogram>(methodInfo.first);

      if (subprogram->getVirtuality() == dwarf::DW_VIRTUALITY_none) {
        // non-virtual method, nothing to update. Just skip it.
        continue;
      }
      const DISubroutineType *SPTy = subprogram->getType();

      for (unsigned j = 0, Nj = vMethods.size(); j < Nj; ++j) {
        if (isEqualVMethodPrototype(
            dyn_cast<DISubroutineType>(vMethods[j]),
            SPTy)) {
          // virtual method is not introduced.
          methodInfo.second = false;
        }
      }
      if (methodInfo.second) {
        // an introduced virtual function, update counter and add to vMethods.
        info.vMethodsCount++;
        vMethods.push_back(SPTy);
        hasVFuncTab = true;
      }
    }
  }

  if (!hasVFuncTab) {
    info.vFuncTab = nullptr;
  }

  if (info.vBaseClasses.size() > 0 && info.vbpOffset < 0) {
    if (info.vFuncTab) {
      // Class has virtual function pointer, add pointer size.
      info.vbpOffset = getPointerSizeInBits() >> 3;
    } else {
      info.vbpOffset = 0;
    }
  }

  // If this type is contained within another type, then record it as being
  // nested.
  //
  const DIScope *llvmScope = resolve(llvmType->getScope());
  if (llvmScope) {
    const DIType* parentType = dyn_cast<DIType>(llvmScope);
    if (parentType) {
      info.isNested = true;
    }
  }

  return info;
}

STIType *STIDebugImpl::getVoidType() const {
  if (_voidType == nullptr) {
    STITypeBasic *voidType;

    voidType = const_cast<STIDebugImpl*>(this)->createTypeBasic(T_VOID, 0);

    const_cast<STIDebugImpl *>(this)->appendType(voidType);
    const_cast<STIDebugImpl *>(this)->_voidType = voidType;
  }

  return _voidType;
}

STIType *STIDebugImpl::getVbpType() const {
  if (_vbpType == nullptr) {
    STITypeBasic *int4Type = createTypeBasic(T_INT4, 4);
    const_cast<STIDebugImpl *>(this)->appendType(int4Type);

    STITypeModifier *constInt4Type = STITypeModifier::create();
    constInt4Type->setQualifiedType(int4Type);
    constInt4Type->setIsConstant(true);
    const_cast<STIDebugImpl *>(this)->appendType(constInt4Type);

    STITypePointer *vbpType = STITypePointer::create();
    vbpType->setPointerTo(constInt4Type);
    vbpType->setSizeInBits(getPointerSizeInBits());
    const_cast<STIDebugImpl *>(this)->appendType(vbpType);

    const_cast<STIDebugImpl *>(this)->_vbpType = vbpType; // FIXME
  }
  return _vbpType;
}

unsigned STIDebugImpl::getPointerSizeInBits() const { return _ptrSizeInBits; }

//===----------------------------------------------------------------------===//
// appendType(type)
//
// Appends the specified type to the end of the type table which will be
// emitted to the .debug$T section.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::appendType(STIType* type) {
  getTypeTable()->push_back(type);
}

//===----------------------------------------------------------------------===//
// mapLLVMTypeToSTIType(llvmType, stiType)
//
// Map an LLVM type to an STI type.  Future references to the LLVM type will
// reuse the existing STI type from the map.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::mapLLVMTypeToSTIType(
        const DIType  *llvmType,
        STIType       *stiType,
        const DIType  *classType)
{
  getTypeMap(classType)->insert(std::make_pair(llvmType, stiType));
}

//===----------------------------------------------------------------------===//
// getMappedSTIType(llvmType)
//
// If the specified LLVM type has already been mapped to an existing STI
// type then the STI type is returned.  Otherwise nullptr is returned.
//
//===----------------------------------------------------------------------===//

STIType* STIDebugImpl::getMappedSTIType(
        const DIType *llvmType,
        const DIType *classType) {
  STITypeMap *TM = getTypeMap(classType);

  auto itr = TM->find(llvmType);
  if (itr == TM->end()) {
    return nullptr;
  }

  return itr->second;  /* second is the STI type */
}

//===----------------------------------------------------------------------===//
// isDefnInProgress(llvmType)
//
// Returns TRUE if a definition for the specified LLVM type is currently
// in progress (we are lowering the LLVM type but it does not yet have a
// valid STI type in the types section).
//
//===----------------------------------------------------------------------===//

bool STIDebugImpl::isDefnInProgress(const DIType *llvmType) {
  STITypeMap *TM = getTypeMap();

  auto itr = TM->find(llvmType);
  if (itr != TM->end() && itr->second == nullptr) {
    // The type exists in the mapping with an undefined STI type.
    //
    return true;
  }

  return false;
}

//===----------------------------------------------------------------------===//
// setDefnInProgress(llvmType, inProgress)
//
// When inProgress is TRUE, the specified llvmType is marked as having a
// definition in progress.  This is done by adding a placeholder entry to
// the mapping table which maps the LLVM type to an undefined nullptr STI
// type.
//
// When inProgress is FALSE, an existing placeholder added to the mapping
// table for this LLVM type is removed.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::setDefnInProgress(const DIType *llvmType, bool inProgress) {
  STITypeMap *TM = getTypeMap();

  if (inProgress) {
    TM->insert(std::make_pair(llvmType, nullptr));
  } else {
    auto iter = TM->find(llvmType);
    if (iter != TM->end() && iter->second == nullptr) {
      TM->erase(iter);
    }
  }
}

//===----------------------------------------------------------------------===//
// resolve(ref)
//===----------------------------------------------------------------------===//

template <typename T> T* STIDebugImpl::resolve(TypedDINodeRef<T> ref) const {
  return ref.resolve();
}

//===----------------------------------------------------------------------===//
// toTypeDefinition(type)
//
// If the specified type is a structure declaration, then this routine
// returns the type entry for the corresponding structure definition.
//
// Otherwise the specified STI type is returned.
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::toTypeDefinition(STIType *type) {
  auto     mapping = getDefTypeMap()->find(type);
  STIType *definition;

  if (mapping != getDefTypeMap()->end()) {
     definition = mapping->second; // "second" is the definition.
  } else {
     definition = type;
  }

  return definition;
}

//===----------------------------------------------------------------------===//
// leafForAggregateType(llvmType)
//
// Returns the STI leaf identifier for the specified composite type, which must
// be an aggregate type (class, struct, union).
//
//===----------------------------------------------------------------------===//

static uint16_t leafForAggregateType(const DICompositeType *llvmType) {
  uint16_t leaf;

  switch (static_cast<dwarf::Tag>(llvmType->getTag())) {
#define X(TAG, TYPE) case dwarf::TAG: leaf = TYPE; break
    X(DW_TAG_class_type,     LF_CLASS);
    X(DW_TAG_structure_type, LF_STRUCTURE);
    X(DW_TAG_union_type,     LF_UNION);
#undef  X
  default:
    assert(!"Unknown structure type");
    leaf = LF_INTEL_NONE;
    break;
  }

  return leaf;
}

//===----------------------------------------------------------------------===//
// createTypeBasic(primitive, sizeInBits)
//
// Creates a new STI basic type.
//
//===----------------------------------------------------------------------===//

STITypeBasic *STIDebugImpl::createTypeBasic(
      STITypeBasic::Primitive primitive,
      uint32_t sizeInBits) const {
  STITypeBasic *type;

  type = STITypeBasic::create();
  type->setPrimitive(primitive);
  type->setSizeInBits(sizeInBits);

  return type;
}

//===----------------------------------------------------------------------===//
// createTypeStructure(...)
//
// Creates a single type entry for an aggregate type.
//
//===----------------------------------------------------------------------===//

STITypeStructure *STIDebugImpl::createTypeStructure(
      const uint16_t                leaf,
      std::string                   name,
      STITypeFieldList             *fieldType,
      STITypeVShape                *vshapeType,
      const uint16_t                count,
      STITypeStructure::Properties  properties,
      STINumeric                   *size,
      const uint32_t                sizeInBits) {
  STITypeStructure *type;

  type = STITypeStructure::create();
  type->setLeaf         (leaf);
  type->setCount        (count);
  type->setProperties   (properties);
  type->setFieldType    (fieldType);
  type->setVShapeType   (vshapeType);
  type->setSize         (size);
  type->setName         (name);
  type->setSizeInBits   (sizeInBits);

  return type;
}

//===----------------------------------------------------------------------===//
// lowerTypeStructureDecl(llvmType, classInfo)
//
// Creates a single type entry for an aggregate type declaration.
//
//===----------------------------------------------------------------------===//

STITypeStructure *STIDebugImpl::lowerTypeStructureDecl(
        const DICompositeType *llvmType,
        ClassInfo             &classInfo) {
  typedef STITypeStructure::Properties Properties;

  STITypeStructure *type;
  uint16_t          leaf;
  std::string       name;
  uint16_t          count;
  STINumeric       *size;
  STITypeFieldList *fieldType;
  STITypeVShape    *vshapeType;
  Properties        properties;
  uint32_t          sizeInBits;

  leaf       = leafForAggregateType(llvmType);
  name       = nameForAggregateType(llvmType);
  count      = 0;
  size       = createNumericSignedInt(0);
  fieldType  = nullptr;
  vshapeType = nullptr;
  sizeInBits = llvmType->getSizeInBits();

  properties.set(STI_COMPOSITE_PROPERTY_FWDREF);
  properties.set(STI_COMPOSITE_PROPERTY_REALNAME);

  truncateName(name);

  type = createTypeStructure(
          leaf,
          name,
          fieldType,
          vshapeType,
          count,
          properties,
          size,
          sizeInBits);

  appendType(type);

  return type;
}

//===----------------------------------------------------------------------===//
// lowerTypeVShape(info)
//===----------------------------------------------------------------------===//

STITypeVShape *STIDebugImpl::lowerTypeVShape(unsigned vMethodsCount) {
  STITypeVShape *vShapeType;

  // If the virtual methods count is zero then there isn't going to be a
  // virtual function table and we don't need to describe it's shape.
  // 
  if (!vMethodsCount) {
    return nullptr;
  }

  vShapeType = STITypeVShape::create();
  vShapeType->setCount(vMethodsCount);

  appendType(vShapeType);

  return vShapeType;
}

//===----------------------------------------------------------------------===//
// lowerTypeStructureDefn(llvmType, classInfo)
//
// Creates a single type entry for an aggregate type definition.
//
//===----------------------------------------------------------------------===//

STITypeStructure *STIDebugImpl::lowerTypeStructureDefn(
        const DICompositeType *llvmType,
        ClassInfo             &info) {
  typedef STITypeStructure::Properties Properties;

  STITypeStructure *type;
  uint16_t          leaf;
  std::string       name;
  uint16_t          count;
  STINumeric       *size;
  STITypeFieldList *fieldType;
  STITypeVShape    *vShapeType;
  Properties        properties;
  uint32_t          sizeInBits;

  // Lower the virtual function table shape descriptor.  This is done here
  // because it is referenced by both the LF_VFUNCTAB entry in the field list
  // and by the LF_STRUCT type entry.
  //
  vShapeType = lowerTypeVShape(info.vMethodsCount);

  // The field list needs to be lowered before the structure definition.
  //
  fieldType  = lowerTypeStructureFieldList(llvmType, info, vShapeType);

  leaf       = leafForAggregateType(llvmType);
  name       = nameForAggregateType(llvmType);
  count      = llvmType->getElements().size();
  size       = createNumericSignedInt(llvmType->getSizeInBits() >> 3);
  sizeInBits = llvmType->getSizeInBits();

  properties.set(STI_COMPOSITE_PROPERTY_REALNAME);

  if (info.hasCTOR) {
    properties.set(STI_COMPOSITE_PROPERTY_CTOR);
  }

  truncateName(name);

  type = createTypeStructure(
          leaf,
          name,
          fieldType,
          vShapeType,
          count,
          properties,
          size,
          sizeInBits);

  appendType(type);

  return type;
}

//===----------------------------------------------------------------------===//
// createSymbolUserDefined(type, name)
//
// Creates a S_UDT symbol, mapping the alias name specified to the specified
// type entry.
//
//===----------------------------------------------------------------------===//

STISymbolUserDefined *STIDebugImpl::createSymbolUserDefined(
        STIType  *type,
        StringRef name) {
  STISymbolUserDefined *symbol;

  symbol = STISymbolUserDefined::create();
  symbol->setDefinedType  (type);
  symbol->setName         (name);

  return symbol;
}

//===----------------------------------------------------------------------===//
// toBasicPrimitive(llvmType)
//
// Returns an STI primitive encoding for the specified LLVM basic type.
//
//===----------------------------------------------------------------------===//

STITypeBasic::Primitive STIDebugImpl::toBasicPrimitive(
      const DIBasicType *llvmType) {
  STITypeBasic::Primitive primitive;
  dwarf::TypeKind kind; 
  uint32_t byteSize;

  kind     = static_cast<dwarf::TypeKind>(llvmType->getEncoding());
  byteSize = llvmType->getSizeInBits() >> 3;

  // FIXME: This should use a quicker lookup method.
  //
#define X(KIND, BYTESIZE, PRIMITIVE)                                          \
  if (kind == KIND && byteSize == BYTESIZE) {                                 \
    primitive = PRIMITIVE;                                                    \
  } else
  PRIMITIVE_BASIC_TYPE_MAPPINGS
#undef X
  { primitive = T_NOTYPE; }

  // Map "long INT" types to "LONG" types.
  //
  if (llvmType->getName().count("long")) {
    switch (primitive) {
#define MAP(FROM, TO) case (FROM): primitive = TO; break
    MAP(T_INT4,     T_LONG);
    MAP(T_UINT4,    T_ULONG);
#undef  MAP
    default:
      // For everything else leave the primitive unchanged.
      break;
    }
  }

  return primitive;
}

//===----------------------------------------------------------------------===//
// toPointerPrimitive(llvmType)
//
// Returns a STI primitive encoding for the specified llvmType, which may be
// a pointer to a basic type.  If no primitive matches the specified type
// then T_NOTYPE is returned.
//
//===----------------------------------------------------------------------===//

STITypeBasic::Primitive STIDebugImpl::toPointerPrimitive(
    const DIDerivedType *llvmType) {
  const DIType *pointerTo;
  const DIBasicType *pointerToBasic;
  STITypeBasic::Primitive primitive;
  uint32_t ptrSize;
  dwarf::TypeKind typeKind;
  uint32_t typeSize;
  
  // If the specified llvm type is not a pointer but encoded as a pointer type
  // (such as a reference) then we can't encode it as a pointer to basic type.
  //
  if (llvmType->getTag() != dwarf::DW_TAG_pointer_type) {
    return T_NOTYPE;
  }

  // The type being pointed-to must be a basic type, otherwise we can't encode
  // this a pointer to basic type.
  //
  pointerTo = resolve(llvmType->getBaseType());

  if (pointerTo == nullptr) {
    // LLVM has no attribute encoding for "void", instead a nullptr is used.
    // This maps the nullptr back to the appropriate void pointer type.
    //
    pointerToBasic = nullptr;          // Pointer-To type is void.
    typeSize = 0;                      // Type size of "0" means "void".
    typeKind = dwarf::DW_ATE_address;  // The type kind here isn't used.
  } else {
    // A non-NULL pointer-to type needs to be a basic type or we won't have a
    // primitive type for it.
    //
    pointerToBasic = dyn_cast<DIBasicType>(pointerTo);
    if (!pointerToBasic) {
      return T_NOTYPE;
    }

    typeKind = static_cast<dwarf::TypeKind>(pointerToBasic->getEncoding());
    typeSize = pointerToBasic->getSizeInBits() >> 3;
  }

  ptrSize = llvmType->getSizeInBits() >> 3;

  // FIXME: This should use a quicker lookup method.
  //
#define X(TYPEKIND, TYPESIZE, PTRSIZE, PRIMITIVE)                             \
  if ((typeSize == 0 || typeKind == TYPEKIND) &&                              \
      typeSize == TYPESIZE &&                                                 \
      ptrSize == PTRSIZE) {                                                   \
    primitive = PRIMITIVE;                                                    \
  } else
  PRIMITIVE_POINTER_TYPE_MAPPINGS
#undef  X
  { primitive = T_NOTYPE; }  // No pointer-to-basic encoding matched

  // Map long INT types to LONG types.
  //
  if (pointerToBasic && pointerToBasic->getName().count("long")) {
    switch (primitive) {
#define MAP(FROM, TO) case (FROM): primitive = TO; break
    MAP(T_32PINT4,      T_32PLONG);
    MAP(T_32PUINT4,     T_32PULONG);
    MAP(T_64PINT4,      T_64PLONG);
    MAP(T_64PUINT4,     T_64PULONG);
#undef  MAP
    default:
      // For everything else leave the primitive unchanged.
      break;
    }
  }

  return primitive;
}

//===----------------------------------------------------------------------===//
// nameForAggregateType(llvmType)
//
// Returns a valid name describing the specified aggregate LLVM type.  If
// the specified type is unnamed, then a name unique to this compilation unit
// will be generated for the type.
//
//===----------------------------------------------------------------------===//

std::string STIDebugImpl::nameForAggregateType(
        const DICompositeType *llvmType) {
  StringRef     partialName = llvmType->getName();
  DIScopeRef    scope       = llvmType->getScope();
  std::string   name;

  name = getScopeFullName(resolve(scope), partialName);

  if (name.empty()) {
    STIDebugImpl::StringNameMap *stringMap;

    stringMap = const_cast<STIDebugImpl *>(this)->getStringNameMap();
    if (!stringMap->count(llvmType)) {
      stringMap->insert(std::make_pair(llvmType, getUniqueName()));
    }

    name = stringMap->find(llvmType)->second;
  }

  return name;
}

//===----------------------------------------------------------------------===//
// lowerTypeAlias(llvmType)
//
// Lowers the specified llvmType, which must be a typedef (type alias), to an
// STI intermediate representation.
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerTypeAlias(const DIDerivedType *llvmType) {
  STISymbolUserDefined *symbol;
  STIScope *scope;
  DIType   *llvmBaseType;
  STIType  *baseType;
  StringRef name;

  llvmBaseType = resolve(llvmType->getBaseType());

  // Lower the containing scope.
  //
  scope = getOrCreateScope(resolve(llvmType->getScope()));

  // Lower the underlying base type.  If the underlying type refers to a
  // declaration then refer to the definition instead.
  //
  baseType = lowerType(llvmBaseType);

  // Locate the new name for the type alias.
  //
  name = llvmType->getName();

  if (baseType->getKind() == STI_OBJECT_KIND_TYPE_STRUCTURE) {
    STITypeStructure *pType = static_cast<STITypeStructure *>(baseType);
    auto stringMap = const_cast<STIDebugImpl *>(this)->getStringNameMap();
    if (stringMap->count(llvmBaseType)) {
      (*stringMap)[llvmType] = name;
      pType->setName(name);
    }
  }

  // When creating an alias of an unnamed enumeration, it is expected the
  // enumeration inherits the name of the type alias.
  //
  if (baseType->getKind() == STI_OBJECT_KIND_TYPE_ENUMERATION) {
    STITypeEnumeration *enumeration =
        static_cast<STITypeEnumeration *>(baseType);
    if (enumeration->getName() == _unnamedType) {
      enumeration->setName(name);
    }
  }

  // Create a user-defined symbol in the appropriate scope.
  //
  symbol = createSymbolUserDefined(baseType, name);
  scope->add(symbol);

  // Type references to the UDT should reference the base type.
  //
  mapLLVMTypeToSTIType(llvmType, baseType);

  return baseType;
}

//===----------------------------------------------------------------------===//
// lowerTypeBasic(llvmType)
//
// Lowers the specified llvmType, which must be a basic type, to an STI
// intermediate representation.
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerTypeBasic(const DIBasicType *llvmType) {
  const uint32_t sizeInBits = llvmType->getSizeInBits();
  const STITypeBasic::Primitive primitive = toBasicPrimitive(llvmType);
  STITypeBasic *type;

  type = createTypeBasic(primitive, sizeInBits);

  appendType(type);
  mapLLVMTypeToSTIType(llvmType, type);

  return type;
}

//===----------------------------------------------------------------------===//
// lowerTypePointer(llvmType)
//
// Lowers the specified llvmType, which may be a pointer or reference, to an
// STI intermediate representation.
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerTypePointer(const DIDerivedType *llvmType) {
  const unsigned tag = llvmType->getTag();
  const DIType *baseType;
  STITypePointer *type;
  STIType *pointerTo;
  STIType *classType;
  unsigned int sizeInBits;
  STITypePointer::PTMType ptrToMemberType;
  bool isLValueReference;
  bool isRValueReference;

  // Pointers to basic types can be encoded using basic type encodings.
  //
  if (STIType *pointerToBasic = lowerTypePointerToBasic(llvmType)) {
    return pointerToBasic;
  }

  baseType = resolve(llvmType->getBaseType());

  // Create the class type for member pointers and determine the member
  // pointer type.
  //
  if (tag == dwarf::DW_TAG_ptr_to_member_type) {
    const DIType           *llvmClass;
    const DISubroutineType *llvmMember;

    // Lower the class type containing the member being pointed to.
    //
    llvmClass = resolve(llvmType->getClassType());
    classType = lowerType(llvmClass);

    // Lower the pointed-to member type.
    //
    llvmMember = dyn_cast<DISubroutineType>(baseType);
    if (llvmMember) {
      pointerTo       = lowerTypeMemberFunction(llvmMember, llvmClass);
      ptrToMemberType = STITypePointer::PTM_METHOD;
    } else {
      pointerTo       = lowerType(baseType);
      ptrToMemberType = STITypePointer::PTM_DATA;
    }
  } else {
    // Lower the type being pointed to.
    //
    pointerTo       = lowerType(baseType);
    ptrToMemberType = STITypePointer::PTM_NONE;

    // While processing the type being pointed to it is possible we already
    // created this pointer type.  If so, we check here and return the existing
    // pointer type.
    //
    if (STIType* existingType = getMappedSTIType(llvmType)) {
      return existingType;
    }

    classType = nullptr;
  }

  sizeInBits = llvmType->getSizeInBits();
  if (sizeInBits == 0) {
    sizeInBits = getPointerSizeInBits();
  }

  // References are encoded as pointers with special flags.  If this is a
  // reference then determine the reference type.
  //
  isLValueReference = (tag == dwarf::DW_TAG_reference_type);
  isRValueReference = (tag == dwarf::DW_TAG_rvalue_reference_type);

  type = STITypePointer::create();
  type->setPointerTo        (pointerTo);
  type->setSizeInBits       (sizeInBits);
  type->setContainingClass  (classType);
  type->setIsLValueReference(isLValueReference);
  type->setIsRValueReference(isRValueReference);
  type->setPtrToMemberType  (ptrToMemberType);

  appendType(type);
  mapLLVMTypeToSTIType(llvmType, type);

  return type;
}

//===----------------------------------------------------------------------===//
// lowerTypePointerToBasic(llvmType)
//
// Lowers the specified llvmType, which may be a pointer to a basic type, to
// STI intermediate representation.
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerTypePointerToBasic(const DIDerivedType *llvmType) {
  const uint32_t sizeInBits = llvmType->getSizeInBits();
  STITypeBasic::Primitive primitive;
  STITypeBasic *type;

  // Map the pointer type to a primitive encoding.
  //
  primitive = toPointerPrimitive(llvmType);
  if (primitive == T_NOTYPE) {
    // If the llvmType could not be matched to a pointer primitive then we
    // return nullptr here and the caller is expected to lower the type using
    // some other mechanism.
    //
    return nullptr;
  }

  type = createTypeBasic(primitive, sizeInBits);

  appendType(type);
  mapLLVMTypeToSTIType(llvmType, type);

  return type;
}

//===----------------------------------------------------------------------===//
// lowerTypeModifier(llvmType)
//
// Lowers the specified llvmType, which is a derived type modifier, to STI
// intermediate representation.
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerTypeModifier(const DIDerivedType *llvmType) {
  const unsigned tag = llvmType->getTag();
  STITypeModifier *type;
  STIType *qualifiedType;
  uint32_t sizeInBits;
  bool isConstant;
  bool isVolatile;

  qualifiedType = lowerType(resolve(llvmType->getBaseType()));

  // While processing the type being pointed to, it is possible we already
  // created this modifier type.  If so, we check here and return the existing
  // modifier type.
  //
  if (STIType* existingType = getMappedSTIType(llvmType)) {
    return existingType;
  }

  sizeInBits = qualifiedType->getSizeInBits();
  isConstant = tag == dwarf::DW_TAG_const_type;
  isVolatile = tag == dwarf::DW_TAG_volatile_type;

  type = STITypeModifier::create();
  type->setQualifiedType(qualifiedType);
  type->setIsConstant(isConstant);
  type->setIsVolatile(isVolatile);
  type->setIsUnaligned(false);
  type->setSizeInBits(sizeInBits);

  appendType(type);
  mapLLVMTypeToSTIType(llvmType, type);

  return type;
}

//===----------------------------------------------------------------------===//
// lowerTypeArray(llvmType)
//
// Lowers the specified LLVM array type subranges into STI array type entries.
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerTypeArray(const DICompositeType *llvmType) {
  const StringRef  name         = llvmType->getName();
  const uint32_t   sizeInBits   = llvmType->getSizeInBits();
  STITypeArray    *arrayType;
  STIType         *elementType;
  uint32_t         elementSize;
  bool             undefinedSubrange = false;
  int64_t          lowerBound;
  int64_t          defaultLowerBound;
  int64_t          count;
  STINumeric      *length;

  // Lower the element type.  Refer to the type definition when possible.
  //
  elementType = toTypeDefinition(lowerType(resolve(llvmType->getBaseType())));

  // Calculate the element size.  This size will be recalculated for every
  // subrange.
  //
  elementSize = elementType->getSizeInBits() >> 3;

  // FIXME:
  // There is a bug in the front-end where an array of an incomplete structure
  // declaration ends up not getting a size assigned to it.  This needs to
  // be fixed in the front-end, but in the meantime we don't want to trigger an
  // assertion because of this.
  //
  // For an example look at this test:  multiC/cfe_068@opt_none_debug
  //
  if (llvmType->getSizeInBits() == 0) {
    undefinedSubrange = true;
  }

  // Add subranges to array type.
  //
  DINodeArray elements = llvmType->getElements();
  for (int i = elements.size() - 1; i >= 0; --i) {
    DINode *element = elements[i];

    assert(element->getTag() == dwarf::DW_TAG_subrange_type);

    DISubrange *subrange = cast<DISubrange>(element);
    lowerBound          = subrange->getLowerBound();
    defaultLowerBound   = 0; // FIXME : default bound
    count               = subrange->getCount();

    (void)lowerBound; (void)defaultLowerBound;
    assert(lowerBound == defaultLowerBound); // FIXME

    // FIXME: this is a WA solution until solving dynamic array boundary.
    //
    if (count == -1) {
      count = 1;
      undefinedSubrange = true;
    }

    length = createNumericUnsignedInt(elementSize * count);

    // Create an array type entry representing this subrange.
    //
    arrayType = STITypeArray::create();
    arrayType->setElementType(elementType);
    arrayType->setLength(length);
    appendType(arrayType);

    // Update the element type and element size for subsequent subranges.
    //
    elementType = arrayType;
    elementSize *= count;
  }

  (void)undefinedSubrange;
  assert(undefinedSubrange || elementSize == (llvmType->getSizeInBits() >> 3));

  // The name and size are only added to the outermost subrange.
  //
  arrayType->setName        (name);
  arrayType->setSizeInBits  (sizeInBits);

  mapLLVMTypeToSTIType(llvmType, arrayType);

  return arrayType;
}

//===----------------------------------------------------------------------===//
// lowerTypeStructureFieldList(llvmType, info)
//
// Lowers the specified aggregate LLVM type to a field list, and lowers all
// types referenced by the field list.
//
//===----------------------------------------------------------------------===//

STITypeFieldList* STIDebugImpl::lowerTypeStructureFieldList(
    const DICompositeType *llvmType,
    ClassInfo& info,
    STITypeVShape *vShapeType) {
  STIFieldListBuilder  fieldListBuilder;

  // Create base classes
  ClassInfo::BaseClassList &baseClasses = info.baseClasses;
  for (unsigned i = 0, e = baseClasses.size(); i != e; ++i) {
    const DIDerivedType *inheritance = baseClasses[i];

    STITypeBaseClass *bClass = STITypeBaseClass::create();
    bClass->setAttribute(getTypeAttribute(inheritance, llvmType));
    bClass->setType(lowerType(resolve(inheritance->getBaseType())));
    bClass->setOffset(
            createNumericUnsignedInt(inheritance->getOffsetInBits() >> 3));

    fieldListBuilder.append(bClass);
  }

  // Create virtual base classes
  for (auto &itr : info.vBaseClasses) {
    const DIDerivedType *inheritance =
        dyn_cast<DIDerivedType>(itr.second.llvmInheritance);
    unsigned vbIndex = itr.second.vbIndex;
    bool indirect = itr.second.indirect;

    STITypeVBaseClass *vbClass = STITypeVBaseClass::create(indirect);
    vbClass->setAttribute(getTypeAttribute(inheritance, llvmType));
    vbClass->setType(lowerType(resolve(inheritance->getBaseType())));
    vbClass->setVbpType(getVbpType());
    vbClass->setVbpOffset(createNumericSignedInt(info.vbpOffset));
    vbClass->setVbIndex(createNumericUnsignedInt(vbIndex));

    fieldListBuilder.append(vbClass);
  }

  // Create virtual function table descriptor.
  if (info.vFuncTab) {
    STITypePointer  *vTableType;
    STITypeVFuncTab *vFuncTab;

    vTableType = STITypePointer::create();
    vTableType->setPointerTo(vShapeType);
    vTableType->setSizeInBits(getPointerSizeInBits());
    appendType(vTableType);

    vFuncTab = STITypeVFuncTab::create();
    vFuncTab->setType(vTableType);

    fieldListBuilder.append(vFuncTab);
  }

  // Create members
  ClassInfo::MemberList &members = info.members;
  for (unsigned i = 0, e = members.size(); i != e; ++i) {
    auto itr = members[i];
    const DIDerivedType *llvmMember = dyn_cast<DIDerivedType>(itr.first);

    std::string name = llvmMember->getName();
    truncateName(name);

    STITypeMember *member = STITypeMember::create();
    STIType *memberBaseType = lowerType(resolve(llvmMember->getBaseType()));

    if (llvmMember->isStaticMember()) {
      member->setIsStatic(true);
      member->setAttribute(getTypeAttribute(llvmMember, llvmType));
      member->setType(memberBaseType);
      member->setName(name);

      fieldListBuilder.append(member);
      continue;
    }

    // TODO: move the member size calculation to a helper function.
    uint64_t Size = llvmMember->getSizeInBits();
    uint64_t FieldSize = getBaseTypeSize(llvmMember);
    uint64_t OffsetInBytes = itr.second;

    if (Size != FieldSize) {
      STITypeBitfield *bitfieldType = STITypeBitfield::create();

      uint64_t Offset = llvmMember->getOffsetInBits();
      uint64_t AlignMask = ~(llvmMember->getAlignInBits() - 1);
      uint64_t HiMark = (Offset + FieldSize) & AlignMask;
      uint64_t FieldOffset = (HiMark - FieldSize);
      Offset -= FieldOffset;

      // Maybe we need to work from the other end.
      // if (ASM()->getDataLayout().isLittleEndian())
      //  Offset = FieldSize - (Offset + Size);

      bitfieldType->setOffset(Offset);
      bitfieldType->setSize(Size);
      bitfieldType->setType(memberBaseType);

      appendType(bitfieldType);

      OffsetInBytes += FieldOffset >> 3;
      memberBaseType = bitfieldType;
    } else {
      // This is not a bitfield.
      OffsetInBytes += llvmMember->getOffsetInBits() >> 3;
    }

    member->setAttribute(getTypeAttribute(llvmMember, llvmType));
    member->setType(memberBaseType);
    member->setOffset(createNumericUnsignedInt(OffsetInBytes));
    member->setName(name);

    fieldListBuilder.append(member);
  }

  // Create methods
  for (auto &itr : info.methods) {
    unsigned overloadedCount = itr.second.size();
    assert(overloadedCount > 0 && "Empty methods map entry");
    std::string name = itr.first;
    truncateName(name);
    if (overloadedCount == 1) {
      auto &methodInfo = itr.second[0];
      const DISubprogram *subprogram =
          dyn_cast<DISubprogram>(methodInfo.first);
      bool introduced = methodInfo.second;

      unsigned attribute =
          getFunctionAttribute(subprogram, llvmType, introduced);
      STIType *methodtype =
          lowerTypeMemberFunction(subprogram->getType(), llvmType);
      unsigned virtuality = subprogram->getVirtuality();
      unsigned virtualIndex = subprogram->getVirtualIndex();

      // Create LF_METHOD entry
      STITypeOneMethod *method = STITypeOneMethod::create();

      method->setAttribute(attribute);
      method->setType(methodtype);
      if (introduced) {
        method->setVirtuality(virtuality);
        method->setVirtualIndex(virtualIndex);
      }
      method->setName(name);

      fieldListBuilder.append(method);
    } else {
      // Create LF_METHODLIST entry
      STITypeMethodList *methodList = STITypeMethodList::create();
      for (unsigned i = 0; i < overloadedCount; ++i) {
        auto &methodInfo = itr.second[i];
        const DISubprogram *subprogram =
            dyn_cast<DISubprogram>(methodInfo.first);
        bool introduced = methodInfo.second;

        unsigned attribute =
            getFunctionAttribute(subprogram, llvmType, introduced);
        STIType *methodtype =
            lowerTypeMemberFunction(subprogram->getType(), llvmType);
        unsigned virtuality = subprogram->getVirtuality();
        unsigned virtualIndex = subprogram->getVirtualIndex();

        STITypeMethodListEntry *entry = STITypeMethodListEntry::create();

        entry->setAttribute(attribute);
        entry->setType(methodtype);
        if (introduced) {
          entry->setVirtuality(virtuality);
          entry->setVirtualIndex(virtualIndex);
        }

        methodList->getList().push_back(entry);
      }

      appendType(methodList);

      // Create LF_METHOD entry
      STITypeMethod *method = STITypeMethod::create();

      method->setCount(overloadedCount);
      method->setList(methodList);
      method->setName(name);

      fieldListBuilder.append(method);
    }
  }

  // Append each field list created by the builder to the types table.
  //
  for (STITypeFieldList *fieldList : fieldListBuilder.getFieldLists()) {
    appendType(fieldList);
  }

  // Return the last field list appended, which contains the first field.
  //
  return fieldListBuilder.getFieldLists().back();
}

//===----------------------------------------------------------------------===//
// lowerTypeStructure(llvmType)
//
// Lowers the specified llvmType, which must be an aggregate type, to the
// corresponding STI intermediate representation.
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerTypeStructure(const DICompositeType *llvmType) {
  STIType          *decl;
  STITypeStructure *defn;
  ClassInfo        &classInfo  = collectClassInfo(llvmType);
  bool              isNamed    = !llvmType->getName().empty();
  bool              inProgress = isDefnInProgress(llvmType);

  // If this is a nested type, then we need to set the ISNESTED and CNESTED
  // properties appropriately.  The containing definition may not be created
  // until after we are done here, though, so we need to defer setting these
  // properties until both the containing and nested types have been created.
  //
  if (classInfo.isNested) {
    appendFixup(new STIDebugFixupNested(llvmType));
  }

  // Create a forward declaration for the aggregate type.
  //
  // There are three cases where this should to be done:
  //   1.  The aggregate type is incomplete but referenced in this comp unit.
  //
  //   2.  Named aggregated types always have a forward declaration emitted.
  //
  //   3.  Unnamed aggregate types which contain member functions with
  //       references back to this aggregate type require a forward decl.
  //
  if (llvmType->isForwardDecl() || isNamed || inProgress ) {
    decl = lowerTypeStructureDecl(llvmType, classInfo);

    // Once the declaration has been created the LLVM type will be mapped to
    // the declaration so we no longer need to record the type definition
    // as in-progress.  This needs to be done before the type is mapped.
    //
    if (inProgress) {
      setDefnInProgress(llvmType, false);
    }

    mapLLVMTypeToSTIType(llvmType, decl);
  } else {
    decl = nullptr;
  }

  // If this type is only a forward declaration then we do not have a
  // definition for the type and return the declaration.
  //
  // When a definition is already in progress we don't want to emit another
  // one here.
  //
  // In either case return the declaration we created.
  //
  if (llvmType->isForwardDecl() || inProgress) {
    return decl;
  }

  // Create the aggregate type defintiion.
  //
  if (!decl) {
    // Mark the LLVM type as having a definition in progress.
    //
    setDefnInProgress(llvmType, true);
  }
  defn = lowerTypeStructureDefn(llvmType, classInfo);
  if (!decl) {
    // The definition has been created and is no longer in progress.
    //
    setDefnInProgress(llvmType, false);

    // Processing the definition may force a declaration to be emitted.  If so,
    // look up the declaration here so we can map it to this definition.
    //
    decl = getMappedSTIType(llvmType);
  }

  // Map the type declaration to the definition.
  //
  if (decl && defn) {
    getDefTypeMap()->insert(std::make_pair(decl, defn));
  }

  // Create a S_UDT symbol in the appropriate scope for this aggregate type
  // definition.
  //
  if (isNamed) {
    DIScope              *llvmScope = resolve(llvmType->getScope());
    STIScope             *stiScope  = getOrCreateScope(llvmScope);
    STISymbolUserDefined *symbol;

    symbol = createSymbolUserDefined(defn, defn->getName());
    stiScope->add(symbol);
  }

  // Return the declaration if one was created, otherwise the definition.
  //
  return (decl != nullptr) ? decl : defn;
}

//===----------------------------------------------------------------------===//
// lowerTypeEnumerator(enumerator)
//===----------------------------------------------------------------------===//

STITypeEnumerator* STIDebugImpl::lowerTypeEnumerator(
    const DIEnumerator *llvmType) {
  STITypeEnumerator *type;
  uint16_t attribute;
  STINumeric *value;
  std::string name;

  name      = llvmType->getName();
  value     = createNumericSignedInt(llvmType->getValue());
  attribute = STI_ACCESS_PUBLIC; // FIXME: set correct attributes here!

  truncateName(name);

  type = STITypeEnumerator::create();
  type->setAttribute(attribute);
  type->setValue(value);
  type->setName(name);

  return type;
}

//===----------------------------------------------------------------------===//
// lowerTypeEnumerationFieldList(llvmType)
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerTypeEnumerationFieldList(
    const DICompositeType *llvmType) {
  STIFieldListBuilder  fieldListBuilder;

  // Add enumerators to enumeration type.
  //
  for (DINode* element : llvmType->getElements()) {
    DIEnumerator      *llvmEnumerator;
    STITypeEnumerator *stiEnumerator;

    // If we allow null enumerators here and skip them then our count in the
    // enumeration type would be incorrect.
    //
    llvmEnumerator = dyn_cast_or_null<DIEnumerator>(element);
    assert(llvmEnumerator != nullptr);

    // Lower the enumerator entry.
    //
    stiEnumerator = lowerTypeEnumerator(llvmEnumerator);

    // Append the new entry to the field list.
    //
    fieldListBuilder.append(stiEnumerator);
  }

  // Append each field list to the types table in order.
  //
  for (STITypeFieldList *fieldList : fieldListBuilder.getFieldLists()) {
    appendType(fieldList);
  }

  // Return the last field list appended, which contains the first enumerator.
  //
  return fieldListBuilder.getFieldLists().back();
}

//===----------------------------------------------------------------------===//
// lowerTypeEnumeration(llvmType)
//
// Lowers the specified llvmType, which is an enumeration, to STI intermediate
// representation.
//
// Unnamed enumerations in STI inherit their name from an associated type
// alias.  This is handled later in createSymbolUserDefined().
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerTypeEnumeration(const DICompositeType *llvmType) {
  STITypeEnumeration *type;
  STIType *elementType;
  STIType *fieldType;
  unsigned count;
  STITypeEnumeration::Properties properties;
  std::string name;
  uint32_t sizeInBits;

  // If this enumeration is contained within another type then it needs to be
  // marked as nested (and the parent marked as containing a nested type).
  // We may not have lowered the containing type yet, so create a new fix-up
  // to mark it as nested.
  //
  const DIScope *llvmScope = resolve(llvmType->getScope());
  if (llvmScope && dyn_cast<DIType>(llvmScope)) {
    appendFixup(new STIDebugFixupNested(llvmType));
  }

  name = llvmType->getName();
  if (name.empty()) {
      // When the name is empty (just a null character), this can cause the
      // Microsoft PDB writer to generate a corrupt PDB file.  Instead, we
      // need to emit these as "<unnamed-tag>".  We use a static variable for
      // this in order to test against it if a named alias occurs which
      // we can borrow the name from.
      //
      name = _unnamedType;
  }
  truncateName(name);

  sizeInBits    = llvmType->getSizeInBits();

  if (llvmType->isForwardDecl()) {
    properties.set(STI_COMPOSITE_PROPERTY_FWDREF);

    elementType = nullptr;
    fieldType   = nullptr;
    count       = 0;
  } else {
    elementType = lowerType(resolve(llvmType->getBaseType()));
    fieldType   = lowerTypeEnumerationFieldList(llvmType);
    count       = llvmType->getElements().size();
  }

  type = STITypeEnumeration::create();
  type->setCount        (count);
  type->setProperties   (properties);
  type->setElementType  (elementType);
  type->setFieldType    (fieldType);
  type->setName         (name);
  type->setSizeInBits   (sizeInBits);

  appendType(type);
  mapLLVMTypeToSTIType(llvmType, type);

  return type;
}

//===----------------------------------------------------------------------===//
// lowerTypeSubroutineArgumentList(elements, firstArgIndex)
//
// Lowers all of the argument types in the specified elements array and
// creates a new argument list type entry containing each of the lowered
// argument types.
//
//===----------------------------------------------------------------------===//

STITypeArgumentList *STIDebugImpl::lowerTypeSubroutineArgumentList(
    DITypeRefArray elements,
    uint16_t       firstArgIndex) {
  STITypeArgumentList *argList;
  DIType              *argType;

  // Create the argument list type entry.
  //
  argList = STITypeArgumentList::create();

  // Walk each of the elements, starting with the first argument index, lower
  // the argument type and append it's index to the argument list.
  //
  for (unsigned i = firstArgIndex, size = elements.size(); i < size; ++i) {
    argType = resolve(elements[i]);
    if (argType) {
      argList->append(lowerType(argType));
    } else {
      assert(i == size - 1); // A variadic argument must be the last argument.
      argList->append(nullptr); // FIXME: handle variadic argument
    }
  }

#if 0
  // If there are no parameters then add a single argument of type T_NOTYPE
  // to represent the "void" argument type.
  //
  // Visual Studio 2013 does this, but Visual Studio 2015 omits this additional
  // entry... so it is disabled here to be compatible with 2015.
  //
  if (argList->getArgumentCount() == 0) {
    argList->append(createTypeBasic(T_NOTYPE, 0));
  }
#endif

  appendType(argList);

  return argList;
}

//===----------------------------------------------------------------------===//
// lowerTypeMemberFunction(llvmType, llvmClass)
//
// Lowers the specified subprogram type as a member function of the specified
// class.
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerTypeMemberFunction(
    const DISubroutineType *llvmType,
    const DIType           *llvmClass) {
  STITypeMemberFunction  *type;
  STITypeArgumentList    *argListType;
  STIType                *returnType;
  STIType                *thisType;
  STIType                *classType;
  int                     thisAdjust;
  int                     callingConvention;
  uint16_t                firstArgIndex;
  uint16_t                paramCount;
  DITypeRefArray          arguments;
  const DIType           *llvmThis;

  // Lower the containing class type.
  //
  classType = lowerType(llvmClass);

  // Check to see if a matching member function type exists for this class.
  //
  if (STIType* existingType = getMappedSTIType(llvmType, llvmClass)) {
    return existingType;
  }

  // Lower the return type.
  //
  // The return type is recorded in the argument list at index "0".  If the
  // arguments list is empty then the return type is void and there are no
  // parameters.
  //
  arguments  = llvmType->getTypeArray();
  returnType = lowerType(arguments.size() ? resolve(arguments[0]) : nullptr);

  // Lower the "this" pointer type.
  //
  // Non-static member functions record the object pointer in the elements list
  // at index "1".
  //
  llvmThis = arguments.size() > 1 ? resolve(arguments[1]) : nullptr;
  if (llvmThis && llvmThis->isObjectPointer()) {
    thisType      = lowerType(llvmThis);
    firstArgIndex = 2;
  } else {
    thisType      = nullptr;
    firstArgIndex = 1;
  }
  thisAdjust      = 0; // FIXME

  // Lower the argument list.
  //
  // The index of the first argument follows both the return type and this
  // pointer type if present.
  //
  argListType = lowerTypeSubroutineArgumentList(arguments, firstArgIndex);
  paramCount  = argListType->getArgumentCount();

  // Determine the calling convention this routine uses.
  //
  callingConvention = NEAR_C; // FIXME

  // Create an LF_MFUNCTION type entry.
  //
  type = STITypeMemberFunction::create();
  type->setClassType         (classType);
  type->setThisType          (thisType);
  type->setThisAdjust        (thisAdjust);
  type->setCallingConvention (callingConvention);
  type->setReturnType        (returnType);
  type->setArgumentList      (argListType);
  type->setParamCount        (paramCount);

  appendType(type);
  mapLLVMTypeToSTIType(llvmType, type, llvmClass);

  return type;
}

//===----------------------------------------------------------------------===//
// lowerTypeSubroutine(llvmType)
//
// Lower the specified LLVM subroutine type to STI type entries.
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerTypeSubroutine(const DISubroutineType *llvmType) {
  STITypeProcedure    *type;
  STITypeArgumentList *argListType;
  STIType             *returnType;
  int                  callingConvention;
  uint16_t             paramCount;
  DITypeRefArray       arguments;

  // Lower the return type.
  //
  // The return type is recorded in the argument list at index "0".  If the
  // argument list is empty then the return type is void and there are no
  // parameters.
  //
  arguments  = llvmType->getTypeArray();
  returnType = lowerType(arguments.size() ? resolve(arguments[0]) : nullptr);

  // Lower the argument list.
  //
  // The return type will be index "0", so the first argument is index "1".
  //
  argListType = lowerTypeSubroutineArgumentList(arguments, 1);
  paramCount  = argListType->getArgumentCount();

  // Determine the calling convention this routine uses.
  //
  callingConvention = NEAR_C; // FIXME

  // Create an LF_PROCEDURE type entry.
  //
  type = STITypeProcedure::create();
  type->setReturnType        (returnType);
  type->setArgumentList      (argListType);
  type->setParamCount        (paramCount);
  type->setCallingConvention (callingConvention);

  appendType(type);
  mapLLVMTypeToSTIType(llvmType, type);

  return type;
}

//===----------------------------------------------------------------------===//
// lowerTypeRestrict(llvmType)
//
// Lowers an LLVM restrict type to an STI type representation.
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerTypeRestrict(const DIDerivedType *llvmType) {
  STIType *type;
  DIType  *baseType = resolve(llvmType->getBaseType());

  // There is currently no representation in STI for "restrict", so we ignore
  // it and lower the base type.
  //
  type = lowerType(baseType);

  // Further references to the restrict type should return the existing base
  // type entry.
  //
  mapLLVMTypeToSTIType(llvmType, type);

  return type;
}

//===----------------------------------------------------------------------===//
// lowerTypeUnspecified(llvmType)
//
// Lower an unspecified type (DW_TAG_unspecified_type).
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerTypeUnspecified(const DIType *llvmType) {
  STIType *type;

  // This is currently only used for describing the nullptr base type:
  //   !1855 = !DIBasicType(
  //             tag: DW_TAG_unspecified_type,
  //             name: "decltype(nullptr)")
  //
  assert(isa<DIBasicType>(llvmType));

  // There's no unspecified type on Windows, so create it as a pointer to void.
  //
  type = createTypeBasic(T_PVOID, getPointerSizeInBits());
  appendType(type);
  mapLLVMTypeToSTIType(llvmType, type);

  return type;
}

//===----------------------------------------------------------------------===//
// lowerType(llvmType, info)
//
// Lowers the specified llvmType into one or more STIType entries.
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerType(const DIType *llvmType) {
  STIType *type;
  unsigned int tag;

  if (llvmType == nullptr) {
    return getVoidType();
  }

  if (STIType* existingType = getMappedSTIType(llvmType)) {
    return existingType;
  }

  tag = llvmType->getTag();
  switch (tag) {
#define X(TAG, HANDLER, TYPE)                                                 \
    case dwarf::TAG: type = HANDLER(cast<TYPE>(llvmType)); break
  X(DW_TAG_array_type,            lowerTypeArray,         DICompositeType);
  X(DW_TAG_class_type,            lowerTypeStructure,     DICompositeType);
  X(DW_TAG_structure_type,        lowerTypeStructure,     DICompositeType);
  X(DW_TAG_union_type,            lowerTypeStructure,     DICompositeType);
  X(DW_TAG_enumeration_type,      lowerTypeEnumeration,   DICompositeType);
  X(DW_TAG_base_type,             lowerTypeBasic,         DIBasicType);
  X(DW_TAG_pointer_type,          lowerTypePointer,       DIDerivedType);
  X(DW_TAG_reference_type,        lowerTypePointer,       DIDerivedType);
  X(DW_TAG_rvalue_reference_type, lowerTypePointer,       DIDerivedType);
  X(DW_TAG_unspecified_type,      lowerTypeUnspecified,   DIType);
  X(DW_TAG_ptr_to_member_type,    lowerTypePointer,       DIDerivedType);
  X(DW_TAG_const_type,            lowerTypeModifier,      DIDerivedType);
  X(DW_TAG_volatile_type,         lowerTypeModifier,      DIDerivedType);
  X(DW_TAG_restrict_type,         lowerTypeRestrict,      DIDerivedType);
  X(DW_TAG_subroutine_type,       lowerTypeSubroutine,    DISubroutineType);
  X(DW_TAG_typedef,               lowerTypeAlias,         DIDerivedType);
#undef X

  default:
    assert(tag != tag); // unhandled type tag!
    type = nullptr;
    break;
  }

  return type;
}

//===----------------------------------------------------------------------===//
// lowerSubprogramType(subprogram)
//
// Lowers the subroutine type of the specified subprogram.
//
//===----------------------------------------------------------------------===//

STIType *STIDebugImpl::lowerSubprogramType(const DISubprogram *subprogram) {
  STIType                *type;
  const DISubroutineType *llvmType;
  const DIType           *llvmClass;

  llvmType  = subprogram->getType();
  llvmClass = dyn_cast<DIType>(resolve(subprogram->getScope()));

  if (llvmClass) {
    type = lowerTypeMemberFunction(llvmType, llvmClass);
  } else {
    type = lowerType(llvmType); // Indirectly calls lowerTypeSubroutine().
  }

  return type;
}

STIScope *STIDebugImpl::getOrCreateScope(const DIScope* llvmScope) {
  STIScope* scope = nullptr;
  if (!llvmScope || isa<DIFile>(llvmScope) || isa<DICompileUnit>(llvmScope)) {
    scope = getCompileUnit()->getScope();
  } else if (const DIType* llvmType = dyn_cast<DIType>(llvmScope)) {
    scope = getOrCreateScope(resolve(llvmType->getScope()));
  } else if (const DINamespace* llvmNamespace = dyn_cast<DINamespace>(llvmScope)) {
    // scope = getOrCreateNameSpace(llvmNamespace);
    scope = getOrCreateScope(llvmNamespace->getScope());
  } else if (const DISubprogram* llvmSubprogram = dyn_cast<DISubprogram>(llvmScope)) {
    STISymbolProcedure *proc =
      getOrCreateSymbolProcedure(llvmSubprogram);
    if (proc != nullptr) {
      scope = proc->getScope();
    } else {
      //FIXME: WA to prevent build from crashing!
      scope = getCompileUnit()->getScope();
    }
  } else if (hasScope(llvmScope)) {
    scope = getScope(llvmScope);
  } else if (const DILexicalBlockFile* llvmLexicalBlockFile =
        dyn_cast<DILexicalBlockFile>(llvmScope)) {
    scope = getOrCreateScope(llvmLexicalBlockFile->getScope());
  } else if (const DILexicalBlockBase* llvmLexicalBlock =
        dyn_cast<DILexicalBlockBase>(llvmScope)) {
    STISymbolBlock *block = createSymbolBlock(llvmLexicalBlock);
    if (block == nullptr)
      // Could not create STI symbol block, return null.
      return nullptr;
    scope = block->getScope();
    addScope(llvmScope, scope);
  }

  assert(scope != nullptr);  // Callers assume a valid scope is returned.

  return scope;
}

std::string STIDebugImpl::getScopeFullName(const DIScope* llvmScope,
                                           StringRef name, bool useClassName) {
  if (!llvmScope || isa<DIFile>(llvmScope) || name.empty())
    return name;
  if (const DIType* llvmType = dyn_cast<DIType>(llvmScope)) {
    if (llvmType->getName().empty()) {
      return name;
    }
    std::string scopedName =
        (Twine(llvmType->getName()) + "::" + Twine(name)).str();
    return getScopeFullName(resolve(llvmType->getScope()), scopedName);
  }
  if (const DINamespace* llvmNamespace = dyn_cast<DINamespace>(llvmScope)) {
    StringRef nsName = llvmNamespace->getName();
    if (nsName.empty()) {
      nsName = "`anonymous namespace'";
    }
    std::string scopedName = (Twine(nsName) + "::" + Twine(name)).str();
    return getScopeFullName(llvmNamespace->getScope(), scopedName);
  }
  if (isa<DISubprogram>(llvmScope)) {
    // TODO: should we assert here?
    return name;
  }
  return name;
}

STIType *STIDebugImpl::getClassScope(const DIScope* llvmScope) {
  if (!llvmScope || isa<DIFile>(llvmScope))
    return nullptr;
  if (const DIType* llvmType = dyn_cast<DIType>(llvmScope)) {
    return lowerType(llvmType);
  }
  if (isa<DINamespace>(llvmScope)) {
    return nullptr;
  }
  if (isa<DISubprogram>(llvmScope)) {
    return nullptr;
  }
  return nullptr;
}

STISymbolVariable *STIDebugImpl::createSymbolVariableFromDbgValue(
    const DILocalVariable *DIV,
    const MachineInstr    *DVInsn) {
  STILocation *location = nullptr;

  assert(DVInsn && "Unknown location");
  assert(DVInsn->getNumOperands() == 3 || DVInsn->getNumOperands() == 4);
  // TODO: handle the case DVInsn->getNumOperands() == 4
  if (DVInsn->getOperand(0).isReg()) {
    const MachineOperand RegOp = DVInsn->getOperand(0);
    // If the second operand is an immediate, this is an indirect value.
    if (DVInsn->getOperand(1).isImm()) {
      if (RegOp.getReg() == 0) {
        location = STILocation::createOffset(DVInsn->getOperand(1).getImm());
      } else {
        location = STILocation::createRegisterOffset(
            toSTIRegID(RegOp.getReg()), DVInsn->getOperand(1).getImm());
      }
    } else if (RegOp.getReg()) {
      bool indirect = isIndirectExpression(DVInsn->getDebugExpression());
      if (indirect) {
        location =
            STILocation::createRegisterOffset(toSTIRegID(RegOp.getReg()), 0);
      } else {
        location = STILocation::createRegister(toSTIRegID(RegOp.getReg()));
      }
    }
  } else if (DVInsn->getOperand(0).isImm()) {
    //assert(!"FIXME: support this case");
    // addConstantValue(*VariableDie, DVInsn->getOperand(0), DV.getType());
  } else if (DVInsn->getOperand(0).isFPImm()) {
    //assert(!"FIXME: support this case");
    // addConstantFPValue(*VariableDie, DVInsn->getOperand(0));
  } else if (DVInsn->getOperand(0).isCImm()) {
    //assert(!"FIXME: support this case");
    // addConstantValue(*VariableDie, DVInsn->getOperand(0).getCImm(),
    //                 DV.getType());
  }

  return createSymbolVariable(DIV, location);
}

STISymbolVariable *STIDebugImpl::createSymbolVariableFromFrameIndex(
    const DILocalVariable *DIV,
    int frameIndex) {
  STILocation *location = nullptr;
  unsigned int regnum;
  int          offset;

  const TargetFrameLowering *TFL =
      ASM()->MF->getSubtarget().getFrameLowering();

  regnum = 0;
  offset = TFL->getFrameIndexReference(*ASM()->MF, frameIndex, regnum);

  location = STILocation::createRegisterOffset(toSTIRegID(regnum), offset);

  return createSymbolVariable(DIV, location);
}

STISymbolVariable *STIDebugImpl::createSymbolVariable(
    const DILocalVariable *DIV,
    STILocation *location)
{
  StringRef          name;
  STISymbolVariable *variable;
  STIType           *type;

  // Don't create variables which don't have a valid location.
  if (!location) {
    return nullptr;
  }

  name = DIV->getName();
  type = toTypeDefinition(lowerType(resolve(DIV->getType())));

  // Create variable only if it has a valid location.
  variable = STISymbolVariable::create();
  variable->setName(name);
  variable->setType(type);
  variable->setLocation(location);

  return variable;
}

STISymbolProcedure *
STIDebugImpl::getOrCreateSymbolProcedure(const DISubprogram *SP) {
  DISubprogramMap::iterator Itr = _subprogramMap.find(SP);
  assert(Itr != _subprogramMap.end() && "DISubprogram has no function code!");
  Function *pFunc = Itr->second;

  // Functions with available_externally linkage are not emitted as part of
  // this compilation unit, so we don't emit debug information for them
  // If we did, relocation against these symbols would fail.
  // See (DPD200375706) for more information.
  if (pFunc->hasAvailableExternallyLinkage())
    return nullptr;
  assert(pFunc && "LLVM subprogram has no LLVM function");

  // Check if a symbol has already been created for this subprogram.
  //
  if (_functionMap.count(pFunc)) {
    return _functionMap[pFunc];
  }

  // Thunk routines are emitted as special symbols.
  //
  if (SP->isThunk()) {
    return createSymbolThunk(SP);
  }

  // If this subprogram is a member function of a class then lower the entire
  // class type.
  //
  STIType *procedureType = lowerSubprogramType(SP);

  STISymbolFrameProc *frame = STISymbolFrameProc::create();

  // The subprogram name is now encoded as a fully qualified name, but it
  // isn't formatted the way we like (spaces are missing between template
  // parameters).  So strip the scopes off the name, rebuild them, and
  // then truncate to the maximum allowable size.
  //
  std::string name = stripScopesFromName(SP->getName()).str();
  name = getScopeFullName(resolve(SP->getScope()), name, true);
  truncateName(name);

  STISymbolProcedure *procedure;
  procedure = STISymbolProcedure::create();
  procedure->setName(name);

  if (EmitFunctionIDs) {
    std::string name = SP->getName();
    truncateName(name);
    STIType *classType = getClassScope(resolve(SP->getScope()));
    STITypeFunctionID *funcIDType = STITypeFunctionID::create();
    funcIDType->setType(procedureType);
    funcIDType->setParentScope(nullptr); // FIXME
    funcIDType->setParentClassType(classType);
    funcIDType->setName(name);

    getTypeTable()->push_back(funcIDType);

    procedure->setType(funcIDType);
    procedure->setSymbolID(SP->isLocalToUnit() ? S_LPROC32_ID : S_GPROC32_ID);
  } else {
    procedure->setType(procedureType);
    procedure->setSymbolID(SP->isLocalToUnit() ? S_LPROC32 : S_GPROC32);
  }
  procedure->getLineSlice()->setFunction(pFunc);
  procedure->setScopeLineNumber(SP->getScopeLine());
  procedure->setFrame(frame);

  frame->setProcedure(procedure);

  // In the Microsoft symbols section, all procedure records occur in the
  // compilation unit scope.  Routines which are nested within other routines,
  // such as inlined or Fortran contained routines, are represented using a
  // separate symbol record at the appropriate scoping level.
  //
  getCompileUnit()->getScope()->add(procedure);

  _functionMap.insert(std::make_pair(pFunc, procedure));

  return procedure;
}

STISymbolProcedure *STIDebugImpl::createSymbolThunk(const DISubprogram *SP) {
  STISymbolThunk          *thunk;
  Function                *function;
  unsigned                 line;
  StringRef                name;
  StringRef                linkageName;
  STISymbolThunk::Ordinal  ordinal;

  // Thunk methods are artificial and should not have a source name.
  name = SP->getName();
  assert(name.empty());

  // The LLVM subprogram must have an LLVM function.
  DISubprogramMap::iterator Itr = _subprogramMap.find(SP);
  if (Itr == _subprogramMap.end())
    return nullptr;
  function = Itr->second;
  assert(function != nullptr);

  // Identify the thunk using the linkage name for now.  In the future it may
  // be better to manufacture a name.  We drop the first character because it
  // contains garbage - the same thing is already done for other names in
  // CLANG.
  linkageName = SP->getLinkageName();
  assert(!linkageName.empty());
  name = linkageName.drop_front();

  line = SP->getScopeLine();

  // We haven't seen any cases where Visual Studio actually uses the additional
  // thunk information (this adjustment, virtual table offset, etc). The
  // important part of the S_THUNK32 entry is the address range information so
  // Visual Studio can properly step through the thunk. For now we always emit
  // the ordinal as "NOTYPE".
  // Create and initialize a symbol for the thunk.
  ordinal = STI_THUNK_NOTYPE;

  thunk = STISymbolThunk::create();
  thunk->setScopeLineNumber(line);
  thunk->setName(name);
  thunk->setFunction(function);
  thunk->setOrdinal(ordinal);

  getCompileUnit()->getScope()->add(thunk);

  // Add the thunk procedure to the function map.
  _functionMap.insert(
          std::make_pair(
              function,
              static_cast<STISymbolProcedure*>(thunk)));

  return thunk;
}

STISymbolBlock *STIDebugImpl::createSymbolBlock(const DILexicalBlockBase* LB) {
  STISymbolBlock *block;
  block = STISymbolBlock::create();

  std::string name = LB->getName();
  truncateName(name);

  LexicalScope *Scope = _lexicalScopes.findLexicalScope(LB);
  if (!Scope)
    // Lexical block has no lexical scope created, skip it by returning null.
    return nullptr;

  const SmallVectorImpl<InsnRange> &Ranges = Scope->getRanges();
  assert(!Ranges.empty() && "Handle Block with empty range ");
  // assert(Ranges.size() == 1 && "Handle Block with more than one range");
  // TODO: handle Ranges.size() != 1

  const MachineInstr *BInst = Ranges.front().first;
  const MachineInstr *EInst = Ranges.front().second;

  assert(_labelsBeforeInsn[BInst] && "empty range begin location");
  assert(_labelsAfterInsn[EInst] && "empty range end location");
  // FIXME: emit block labels correctly
  block->setLabelBegin(_labelsBeforeInsn[BInst]);
  block->setLabelEnd(_labelsAfterInsn[EInst]);
  block->setName(name);

  getOrCreateScope(LB->getScope())->add(block);

  DIScope* FuncScope = LB->getScope();
  while (FuncScope && !isa<DISubprogram>(FuncScope)) {
    FuncScope = resolve(FuncScope->getScope());
  }
  assert(isa<DISubprogram>(FuncScope) &&
         "Failed to reach function scope of a lexical block");
  block->setProcedure(
      getOrCreateSymbolProcedure(dyn_cast<DISubprogram>(FuncScope)));

  return block;
}

STIChecksumEntry *STIDebugImpl::getOrCreateChecksum(StringRef path) {
  STIStringEntry *string = _stringTable.find(strdup(path.str().c_str()));
  STIChecksumEntry *checksum = _checksumTable.findEntry(string);

  if (checksum == nullptr) {
    checksum = STIChecksumEntry::create();
    checksum->setStringEntry(string);
    checksum->setType(STIChecksumEntry::STI_FILECHECKSUM_ENTRY_TYPE_NONE);
    checksum->setChecksum(nullptr);
    _checksumTable.append(string, checksum);
  }
  return checksum;
}

//===----------------------------------------------------------------------===//
// getUnqualifiedDIType(ditype)
//
// Returns the specified ditype after stripping the const/volatile qualifiers.
//
//===----------------------------------------------------------------------===//

const DIType *STIDebugImpl::getUnqualifiedDIType(const DIType *ditype) {
  uint16_t tag;

  while (const DIDerivedType *derivedType = dyn_cast<DIDerivedType>(ditype)) {
    tag = derivedType->getTag();
    if (tag != dwarf::DW_TAG_const_type &&
        tag != dwarf::DW_TAG_volatile_type &&
        tag != dwarf::DW_TAG_restrict_type) {
      break;
    }
    ditype = resolve(derivedType->getBaseType());
  }

  return ditype;
}

//===----------------------------------------------------------------------===//
// createNumericUnsignedInt(value)
//
// Creates a numeric leaf representing the specified unsigned integer value.
//
//===----------------------------------------------------------------------===//

STINumeric* STIDebugImpl::createNumericUnsignedInt(const uint64_t value)
{
  STINumeric*           numeric;
  STINumeric::LeafID    leafID;
  size_t                size;
  const char*           data = reinterpret_cast<const char*>(&value);

  if (isUInt<8>(value)) {
    leafID = LF_CHAR;
    size   = 1;
  } else if (isUInt<16>(value)) {
    leafID = LF_USHORT;
    size   = 2;
  } else if (isUInt<32>(value)) {
    leafID = LF_ULONG;
    size   = 4;
  } else {
    leafID = LF_UQUADWORD;
    size   = 8;
  }

  // For small unsigned integers we don't need to encode the leaf identifier.
  //
  if (leafID == LF_CHAR || (leafID == LF_USHORT && value < LF_NUMERIC)) {
    leafID = LF_INTEL_NONE; // No leaf identifier.
  }

  numeric = STINumeric::create(leafID, size, data);

  return numeric;
}

//===----------------------------------------------------------------------===//
// createNumericSignedInt(value)
//
// Creates a numeric leaf representing the specified signed integer value.
//
//===----------------------------------------------------------------------===//

STINumeric* STIDebugImpl::createNumericSignedInt(const int64_t value)
{
  STINumeric*           numeric;
  STINumeric::LeafID    leafID;
  size_t                size;
  const char*           data = reinterpret_cast<const char*>(&value);

  // Non-negative signed values are encoded as unsigned values.
  //
  if (value >= 0) {
    return createNumericUnsignedInt(static_cast<uint64_t>(value));
  }

  // Adjust encoded size based on value.
  //
  if (isInt<8>(value)) {
    leafID = LF_CHAR;
    size   = 1;
  } else if (isInt<16>(value)) {
    leafID = LF_SHORT;
    size   = 2;
  } else if (isInt<32>(value)) {
    leafID = LF_LONG;
    size   = 4;
  } else {
    leafID = LF_QUADWORD;
    size   = 8;
  }

  numeric = STINumeric::create(leafID, size, data);

  return numeric;
}

//===----------------------------------------------------------------------===//
// createNumericAPInt(ditype, value)
//
// Returns a numeric value with one of the following encodings:
//   * LF_USHORT
//   * LF_ULONG
//   * LF_UQUADWORD
//   * LF_CHAR
//   * LF_SHORT
//   * LF_LONG
//   * LF_QUADWORD
//
//===----------------------------------------------------------------------===//

STINumeric* STIDebugImpl::createNumericAPInt(
        const DIType *ditype,
        const APInt& value) {
  STINumeric*   numeric;
  const DIType *unqualifiedDIType;

  // It's not clear how we would encode an arbitrary length integer more
  // than 64-bits long in the STI debug information format, so we ignore
  // them altogether here.
  //
  if (value.getBitWidth() > 64) {
    return nullptr;
  }

  unqualifiedDIType = getUnqualifiedDIType(ditype);

  // We don't currently handle constant values for non-basic types.
  //
  if (!isa<DIBasicType>(unqualifiedDIType)) {
    return nullptr;
  }

  const DIBasicType *dibasic  = dyn_cast<DIBasicType>(unqualifiedDIType);
  unsigned           encoding = dibasic->getEncoding();

  switch (encoding) {
  case dwarf::DW_ATE_boolean:
  case dwarf::DW_ATE_unsigned_char:
  case dwarf::DW_ATE_unsigned:
    numeric = createNumericUnsignedInt(value.getZExtValue());
    break;

  case dwarf::DW_ATE_signed_char:
  case dwarf::DW_ATE_signed:
    numeric = createNumericSignedInt(value.getSExtValue());
    break;

  default:
    numeric = nullptr;
    break;
  }

  return numeric;
}

//===----------------------------------------------------------------------===//
// createNumericAPFloat(ditype, value)
//
// Returns a numeric value with one of the following encodings:
//   * LF_REAL32
//   * LF_REAL48
//   * LF_REAL64
//   * LF_REAL80
//   * LF_REAL128
//
// NOTE: Although cvdump can correctly dump floating point constants, the
//       Microsoft compiler (cl) doesn't produce these for global variables
//       and Visual Studio can't properly display them.
//
//===----------------------------------------------------------------------===//

STINumeric* STIDebugImpl::createNumericAPFloat(
        const DIType *  ditype,
        const APFloat& value) {
  STINumeric*           numeric;
  STINumeric::LeafID    leafID;
  const DIType*         unqualifiedDIType;
  const char*           data;
  size_t                size;                   // size of data in bytes

  unqualifiedDIType = getUnqualifiedDIType(ditype);

  // We don't currently handle constant values for non-basic types.
  //
  if (!isa<DIBasicType>(unqualifiedDIType)) {
    return nullptr;
  }

  // Convert bit size to byte size.  Round up partial bytes (1 bit => 1 byte).
  //
  // NOTE: It looks like the bitcast may be losing some precision, but this is
  //       the same way the rest of the compiler acquires the byte sequence.
  //
  data = reinterpret_cast<const char*>(value.bitcastToAPInt().getRawData());

  // const DIBasicType *dibasic =
  //     dyn_cast<const DIBasicType>(unqualifiedDIType);

  const fltSemantics& semantics = value.getSemantics();

  if (&semantics == &APFloat::IEEEsingle()) {
    leafID = LF_REAL32;
    size   = 4;
  } else if (&semantics == &APFloat::IEEEdouble()) {
    leafID = LF_REAL64;
    size   = 8;
  } else if (&semantics == &APFloat::x87DoubleExtended()) {
    leafID = LF_REAL80;
    size   = 10;
  } else if (&semantics == &APFloat::IEEEquad()) {
    leafID = LF_REAL128;
    size   = 16;
  } else {
    // Not Yet Supported:
    //   * IEEEhalf:
    //   * PPCDoubleDouble:
    //   * Bogus:
    return nullptr;
  }

  // Create the numeric value encoding.
  //
  numeric = STINumeric::create(leafID, size, data);

  return numeric;
}

//===----------------------------------------------------------------------===//
// collectGlobalVariableInfo(CU)
//
// Iterates over all of the global variables in specified compilation unit and
// generates debug information entries for them.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::collectGlobalVariableInfo(const DICompileUnit* CU) {
  DenseMap<const DIGlobalVariableExpression *, const GlobalVariable *> GlobalMap;
  for (const GlobalVariable &GV : MMI()->getModule()->globals()) {
    SmallVector<DIGlobalVariableExpression *, 1> GVEs;
    GV.getDebugInfo(GVEs);
    for (const auto *GVE : GVEs)
      GlobalMap[GVE] = &GV;
  }

  for (auto *GVE : CU->getGlobalVariables()) {
    auto *DIGV = GVE->getVariable();
    if (const GlobalVariable* GV = GlobalMap.lookup(GVE)) {
      STISymbolVariable* variable;
      const DIScope *    scope;

      // Globals with available_externally linkage are not emitted as part of
      // this compilation unit, so we don't emit debug information for them.
      // If we did, relocation against these symbols would fail.
      // See (DPD200375706) for more information.
      if (GV->hasAvailableExternallyLinkage()) {
        continue;
      }

      MCSymbol *label = ASM()->getSymbol(GV);

      STILocation *location = DIGV->isLocalToUnit()
                            ? STILocation::createLocalSegmentedOffset(label)
                            : STILocation::createGlobalSegmentedOffset(label);

      if (DIDerivedType *SDMDecl = DIGV->getStaticDataMemberDeclaration()) {
        scope = resolve(SDMDecl->getScope());
        assert(SDMDecl->isStaticMember() && "Expected static member decl");
        assert(DIGV->isDefinition());
      } else {
        scope = DIGV->getScope();  // Note: "scope" may be null!
      }

      variable = STISymbolVariable::create();
      variable->setName(getScopeFullName(scope, DIGV->getName(), true));
      variable->setType(lowerType(resolve(DIGV->getType())));
      variable->setLocation(location);

      getOrCreateScope(scope)->add(variable);

      std::string path;
      getFullFileName((scope != nullptr ? scope : CU)->getFile(), path);
      (void)getOrCreateChecksum(path);  // FIXME:  Do not check every variable!

    } else if (Constant* constant = nullptr /*DIGV->getVariable()*/) {
      // TODO: This case is disabled due to the change in r281284.
      //       Should use getExpr() to determine the constant values.
      STISymbolConstant* symbol;
      DIScope*           scope = DIGV->getScope();
      DIType *           ditype  = resolve(DIGV->getType());
      STINumeric*        numeric;

      // Translate the different constant types into a STINumeric object.
      //
      if (ConstantInt* CI = dyn_cast<ConstantInt>(constant)) {
        numeric = createNumericAPInt(ditype, CI->getValue());

      } else if (ConstantFP* CFP = dyn_cast<ConstantFP>(constant)) {
        numeric = createNumericAPFloat(ditype, CFP->getValueAPF());

      } else {
        // Possible unsupported numeric encodings:
        //   * LF_COMPLEX32
        //   * LF_COMPLEX64
        //   * LF_COMPLEX80
        //   * LF_COMPLEX128
        //   * LF_VARSTRING
        //   * LF_OCTWORD
        //   * LF_UOCTWORD
        //   * LF_DECIMAL
        //   * LF_UTFSTRING
        //
        numeric = nullptr;
      }

      // If we can't calculate the constant value, then we don't emit anything.
      // Skip to the next entry.
      //
      if (numeric == nullptr) {
          continue;
      }

      // Create a symbolic constant using the type and numeric value.
      //
      symbol = STISymbolConstant::create();
      symbol->setName(getScopeFullName(scope, DIGV->getName(), true));
      symbol->setType(lowerType(ditype));
      symbol->setValue(numeric);

      getOrCreateScope(scope)->add(symbol);
    }
  }
}

void STIDebugImpl::collectModuleInfo() {
  Module *M = const_cast<Module *>(getModule());
  STISymbolModule *module;
  std::string OBJPath = getOBJFullPath();

  // Generate the S_MODULE symbol.
  //
  module = STISymbolModule::create();
  module->setSymbolsSignatureID(STI_SYMBOLS_SIGNATURE_LATEST);
  module->setPath(OBJPath);
  getSymbolTable()->setRoot(module);

  // Generate the S_COMPILE symbol for each compilation unit.
  //
  for (DICompileUnit *CU : M->debug_compile_units()) {
    STISymbolCompileUnit *compileUnit;

    compileUnit = STISymbolCompileUnit::create();
    compileUnit->setProducer(CU->getProducer());
    compileUnit->setMachineID(
        toMachineID(ASM()->TM.getTargetTriple().getArch()));
    module->add(compileUnit);

    // Record the primary source file name in the file checksum table.
    //
    std::string path;
    getFullFileName(CU->getFile(), path);
    (void) getOrCreateChecksum(path);
  }

  // Initialize the DISubprogram to Function map.
  // Initialize the DISubprogram to STISymbolProcedure map.
  //
  for (Module::iterator I = M->begin(), E = M->end(); I != E; ++I) {
    Function *Fn = &*I;
    if (auto *SP = Fn->getSubprogram()) {
      _subprogramMap.insert(std::make_pair(SP, Fn));
      getOrCreateSymbolProcedure(SP);
    }
  }

  // Collect information about global variables.  This needs to be done after
  // the subprograms have been created because this includes static variables.
  //
  for (DICompileUnit *CU : M->debug_compile_units()) {
    collectGlobalVariableInfo(CU);
  }
}

void STIDebugImpl::collectRoutineInfo() {
  typedef MachineFunction::VariableDbgInfo VariableDbgInfo;
  typedef DbgValueHistoryMap::InstrRanges InstrRanges;
  typedef DbgValueHistoryMap::InlinedVariable InlinedVariable;
  typedef std::pair<InlinedVariable, InstrRanges> VariableHistoryInfo;

  STISymbolVariable *variable;

  DenseSet<InlinedVariable> processed;

  for (const VariableDbgInfo &info : ASM()->MF->getVariableDbgInfo()) {
    const DILocalVariable *llvmVar = info.Var;

    if (!llvmVar)
      continue;

    InlinedVariable IV (llvmVar, info.Loc->getInlinedAt());

    if (processed.count(IV))
      continue;
    processed.insert(IV);

    // FIXME: We do not know how to emit inlined variables.
    // Skip inlined variables.
    if (IV.second)
      continue;

    // Ignore this variable if we can't identify the scope it belongs to.
    // This prevents us from crashing later when we try to insert the variable
    // into the scope.
    //
    STIScope *scope = getOrCreateScope(llvmVar->getScope());
    if (!scope)
      continue;

    variable = createSymbolVariableFromFrameIndex(llvmVar, info.Slot);
    if (!variable)
      continue;
    scope->add(variable, llvmVar->getArg());
  }

  for (const VariableHistoryInfo &info : _valueHistory) {
    InlinedVariable                        IV     = info.first;
    const DbgValueHistoryMap::InstrRanges &Ranges = info.second;

    // FIXME: We do not know how to emit inlined variables.
    // Skip inlined variables.
    if (IV.second)
      continue;

    if (processed.count(IV))
      continue;

    if (Ranges.empty())
      continue;

    // Ignore this variable if we can't identify the scope it belongs to.
    // This prevents us from crashing later when we try to insert the variable
    // into the scope.
    //
    STIScope *scope = getOrCreateScope(IV.first->getScope());
    if (!scope)
      continue;

    const MachineInstr *MInsn = Ranges.front().first;
    variable = createSymbolVariableFromDbgValue(IV.first, MInsn);
    if (!variable)
      continue;
    scope->add(variable, IV.first->getArg());

    processed.insert(IV);
  }
}

void STIDebugImpl::fixSymbolUserDefined(STISymbolUserDefined *type) const {
   auto itr = getDefTypeMap()->find(type->getDefinedType());
   if (itr != getDefTypeMap()->end())
     type->setDefinedType(itr->second);
}

//===----------------------------------------------------------------------===//
// setTypeCompositeProperty(type, property)
//
// Sets the specified property on a composite type (class, struct, union, enum).
//
//===----------------------------------------------------------------------===//

static void setTypeCompositeProperty(
        STIType              *type,
        STICompositeProperty  property) {
  switch (type->getKind()) {
  case STI_OBJECT_KIND_TYPE_STRUCTURE:
    static_cast<STITypeStructure*>(type)->setProperty(property);
    break;

  case STI_OBJECT_KIND_TYPE_ENUMERATION:
    static_cast<STITypeEnumeration*>(type)->setProperty(property);
    break;

  default:
    assert(!"Invalid nested type declaration kind!");
    break;
  }
}

//===----------------------------------------------------------------------===//
// fixupNested(fixup)
//
// Fixup properties for nested types (types declared within another type).
// The (nested/contains nested) properties are set accordingly:
//
//     class Outer {            --> CNESTED
//       class Middle{          --> CNESTED, NESTED
//         class Inner {};      --> NESTED
//       };
//       enum Enum {};          --> NESTED
//     };
//
// The "contains nested" flag is not set on declarations.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::fixupNested(const STIDebugFixupNested *fixup) {
  const DIType     *nestedLLVMType;
  const DIType     *parentLLVMType;
  STIType          *nestedTypeDecl;
  STIType          *nestedTypeDefn;
  STIType          *parentTypeDecl;
  STIType          *parentTypeDefn;

  nestedLLVMType = fixup->getNestedType();
  parentLLVMType = dyn_cast<DIType>(resolve(nestedLLVMType->getScope()));

  // Mark the nested STI type declaration as being nested.
  //
  nestedTypeDecl = getMappedSTIType(nestedLLVMType);
  if (nestedTypeDecl) {
    setTypeCompositeProperty(nestedTypeDecl, STI_COMPOSITE_PROPERTY_ISNESTED);
  }

  // Mark the nested STI type definition as being nested.
  //
  nestedTypeDefn = toTypeDefinition(nestedTypeDecl);
  if (nestedTypeDefn && nestedTypeDefn != nestedTypeDecl) {
    setTypeCompositeProperty(nestedTypeDefn, STI_COMPOSITE_PROPERTY_ISNESTED);
  }

  // Mark the parent STI type definition as containing a nested type.
  // The parent declaration is not marked.
  //
  parentTypeDecl = getMappedSTIType(parentLLVMType);
  if (parentTypeDecl) {
    parentTypeDefn = toTypeDefinition(parentTypeDecl);
    if (parentTypeDefn && parentTypeDefn != parentTypeDecl) {
      setTypeCompositeProperty(parentTypeDefn, STI_COMPOSITE_PROPERTY_CNESTED);
    }
  }
}

//===----------------------------------------------------------------------===//
// fixup()
//
// Walk the fix-up table and execute each fix-up of the debug information.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::fixup() {
  for (const STIDebugFixup* fixup : *getFixupTable()) {
    switch (fixup->kind()) {
    case STI_DEBUG_FIXUP_KIND_NESTED:
      fixupNested(static_cast<const STIDebugFixupNested*>(fixup));
      break;

    default:
      break;
    }
  }
}

//===----------------------------------------------------------------------===//
// layout()
//
// This routine performs final layout for the debug information.  After this
// routine is called the debug information should not be modified, and it
// can be safely emitted.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::layout() {
  // Assign unique indexes to each type in the type table.
  //
  // When emitting the type table to an object file, we need to assign the type
  // index to every type here.  When emitting the type table to a PDB file,
  // type indexes are assigned by the PDB writer during emission and are not
  // assigned here.  Basic types are indexed by their primitive kind identifier
  // in either case.
  //
  uint16_t nextTypeIndex = 0x1000;
  for (STIType *type : *getTypeTable()) {
    switch (type->getKind()) {
    case STI_OBJECT_KIND_TYPE_BASIC:
      type->setIndex(static_cast<STITypeBasic *>(type)->getPrimitive());
      break;
    default:
      if (!usePDB()) {
        type->setIndex(nextTypeIndex++);
      }
      break;
    }
  }

  uint32_t nextStringOffset = 0;
  for (STIStringEntry *entry : getStringTable()->getEntries()) {
    entry->setOffset(nextStringOffset);
    nextStringOffset += entry->getString().size() + 1;
  }

  uint32_t nextChecksumOffset = 0;
  for (STIChecksumEntry *entry : getChecksumTable()->getEntries()) {
    entry->setOffset(nextChecksumOffset);
    nextChecksumOffset += 6 + entry->getChecksumSize() + getPaddingSize(entry);
  }
}

//===----------------------------------------------------------------------===//
// emit()
//
// Writes the debug information to it's final destination.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::emit() {
  emitTypes();          // Emits the .debug$S section.
  emitSymbols();        // Emits the .debug$T section.
}

void STIDebugImpl::emitSymbolID(const STISymbolID symbolID) const {
  emitComment(toString(symbolID));
  emitInt16(symbolID);
}

void STIDebugImpl::emitSubsectionBegin(STISubsection *subsection) const {
  STISubsectionID id = subsection->getID();

  // Create the beginning and ending labels for this subsection.
  subsection->setBegin(MMI()->getContext().createTempSymbol());
  subsection->setEnd(MMI()->getContext().createTempSymbol());

  // Subsections are 4-byte aligned.
  emitAlign(4);

  // Each subsection begins with an identifier for the type of subsection.
  emitComment(toString(id));
  emitInt32(id);

  // Followed by the subsection length.  The end label is emitted later.
  emitComment("length");
  emitLabelDiff(subsection->getBegin(), subsection->getEnd());

  // Mark the beginning of the subsection which contributes to the length.
  emitLabel(subsection->getBegin());
}

void STIDebugImpl::emitSubsectionEnd(STISubsection *subsection) const {
  // Mark the end of the subsection which contributes to the length.
  emitLabel(subsection->getEnd());
}

void STIDebugImpl::closeSubsection() const {
  if (_currentSubsection != nullptr) {
    emitSubsectionEnd(_currentSubsection);
    delete _currentSubsection;
    const_cast<STIDebugImpl *>(this)->_currentSubsection = nullptr;
  }
}

void STIDebugImpl::emitSubsection(STISubsectionID id) const {
  // If trying to change subsection to same subsection do nothing.
  if (_currentSubsection != nullptr && _currentSubsection->getID() == id) {
    return;
  }

  closeSubsection();

  const_cast<STIDebugImpl *>(this)->_currentSubsection = new STISubsection(id);
  emitSubsectionBegin(_currentSubsection);
}

void STIDebugImpl::emitAlign(unsigned int byteAlignment) const {
  ASM()->OutStreamer->EmitValueToAlignment(byteAlignment);
}

void STIDebugImpl::idBegin(const STIType* type) const {
  writer()->idBegin(type);
}

void STIDebugImpl::idEnd(const STIType* type) const {
  writer()->idEnd(type);
}

void STIDebugImpl::typeBegin(const STIType* type) const {
  writer()->typeBegin(type);
}

void STIDebugImpl::typeEnd(const STIType* type) const {
  writer()->typeEnd(type);
}

void STIDebugImpl::emitInt8(int value) const {
  writer()->emitInt8(value);
}

void STIDebugImpl::emitInt16(int value) const {
  writer()->emitInt16(value);
}

void STIDebugImpl::emitInt32(int value) const {
  writer()->emitInt32(value);
}

void STIDebugImpl::emitString(StringRef string) const {
  writer()->emitString(string);
}

void STIDebugImpl::emitBytes(const char *data, size_t size) const {
  writer()->emitBytes(size, data);
}

void STIDebugImpl::emitFill(size_t size, const uint8_t byte) const {
  writer()->emitFill(size, byte);
}

void STIDebugImpl::emitComment(StringRef comment) const {
  writer()->emitComment(comment);
}

void STIDebugImpl::emitLabel(MCSymbol *symbol) const {
  writer()->emitLabel(symbol);
}

void STIDebugImpl::emitValue(const MCExpr *value,
                             unsigned int sizeInBytes) const {
  writer()->emitValue(value, sizeInBytes);
}

void STIDebugImpl::emitPadding(unsigned int count) const {
  static const int paddingArray[16] = {
      LF_PAD0,  LF_PAD1,  LF_PAD2,  LF_PAD3,
      LF_PAD4,  LF_PAD5,  LF_PAD6,  LF_PAD7,
      LF_PAD8,  LF_PAD9,  LF_PAD10, LF_PAD11,
      LF_PAD12, LF_PAD13, LF_PAD14, LF_PAD15};

  assert(count < 16);

  for (unsigned int i = count; i > 0; --i) {
    writer()->emitInt8(paddingArray[i]);
  }
}

void STIDebugImpl::emitLabelDiff(const MCSymbol *begin,
                                 const MCSymbol *end,
                                 unsigned sizeInBytes) const {
  MCContext &context = ASM()->OutStreamer->getContext();
  const MCExpr *bExpr;
  const MCExpr *eExpr;
  const MCExpr *delta;

  bExpr = MCSymbolRefExpr::create(begin, MCSymbolRefExpr::VK_None, context);
  eExpr = MCSymbolRefExpr::create(end, MCSymbolRefExpr::VK_None, context);
  delta = MCBinaryExpr::create(MCBinaryExpr::Sub, eExpr, bExpr, context);

  emitValue(delta, sizeInBytes);
}

void STIDebugImpl::emitSecRel32(MCSymbol *symbol) const {
  ASM()->OutStreamer->EmitCOFFSecRel32(symbol, /*Offset=*/0);
}

void STIDebugImpl::emitSectionIndex(MCSymbol *symbol) const {
  ASM()->OutStreamer->EmitCOFFSectionIndex(symbol);
}

void STIDebugImpl::emitNumeric(const uint32_t num) const {
  if (num < LF_NUMERIC) {
    emitInt16(num);
  } else if (num < (LF_NUMERIC << 1)) {
    emitInt16(LF_USHORT);
    emitInt16(num);
  } else {
    emitInt16(LF_ULONG);
    emitInt32(num);
  }
}

bool STIDebugImpl::usePDB() const {
  return _usePDB;
}

char *STIDebugImpl::getCWD() const {
  return GETCWD(nullptr, 0);
}

std::string STIDebugImpl::getPDBFullPath() const {
  char *path = getCWD();
  std::string pdbName = (Twine(path) + Twine("\\") + Twine(_pdbFileName)).str();
  free(path);
  return pdbName;
}

std::string STIDebugImpl::getOBJFullPath() const {
  char *path = getCWD();
  std::string objName = (Twine(path) + Twine("\\") + Twine(_objFileName)).str();
  free(path);
  return objName;
}

StringRef STIDebugImpl::getMDStringValue(StringRef MDName) const {
  StringRef StrVal = "";
  NamedMDNode *MD = getModule()->getNamedMetadata(MDName);
  if (MD) {
    assert(MD->getNumOperands() == 1 && "Expect exactly one operand");
    assert(MD->getOperand(0) &&
      MD->getOperand(0)->getNumOperands() == 1 && "Expect MDNode operand");
    MDString *MDStr = dyn_cast<MDString>(MD->getOperand(0)->getOperand(0));
    assert(MDStr && "Expect MDString operand value");
    if (MDStr) {
      StrVal = MDStr->getString();
    }
  }
  return StrVal;
}

void STIDebugImpl::emitSymbolModule(const STISymbolModule *module) const {
  STISymbolsSignatureID signatureID = module->getSymbolsSignatureID();
  StringRef path = module->getPath();
  const int length = 7 + path.size();

  emitInt16(length);
  emitSymbolID(S_OBJNAME);
  emitInt32(signatureID);
  emitString(path);
}

class STICompile3Flags {
private:
  union FlagsUnion {
    int32_t raw;
    struct {
      uint32_t language : 8;        // Source Language
      uint32_t fEC : 1;             // Edit and Continue
      uint32_t fNoDbgInfo : 1;      // Not compiled with debug
      uint32_t fLTCG : 1;           // Link-time code generation
      uint32_t fNoDataAlign : 1;    // Global data alignment
      uint32_t fManagedPresent : 1; // Managed code/data
      uint32_t fSecurityChecks : 1; // Security Checks (/GS)
      uint32_t fHotPatch : 1;       // Hotpatch Support (/hotpatch)
      uint32_t fCVTCIL : 1;         // CVTCIL
      uint32_t fMSILModule : 1;     // MSIL
      uint32_t padding : 15;        // reserved bits
    } field;
  };
  typedef union FlagsUnion Flags;

  Flags _flags;

public:
  STICompile3Flags() {
    _flags.raw = 0;
    _flags.field.language = STI_C_PLUS_PLUS;
  }

  operator int32_t() const { return _flags.raw; }
};

void STIDebugImpl::emitSymbolCompileUnit(
    const STISymbolCompileUnit *compileUnit) const {
  STISymbolID symbolID = S_COMPILE3;
  STICompile3Flags flags;
  STIMachineID machine = compileUnit->getMachineID();
  int verFEMajor;
  int verFEMinor;
  int verFEBuild;
  int verFEQFE;
  int verMajor;
  int verMinor;
  int verBuild;
  int verQFE;
  StringRef producer;

  verFEMajor = 1;
  verFEMinor = 0;
  verFEBuild = 0;
  verFEQFE   = 0;
  verMajor   = 1;
  verMinor   = 0;
  verBuild   = 0;
  verQFE     = 0;

  producer = compileUnit->getProducer();

  emitInt16(25 + producer.size()); // record length
  emitSymbolID(symbolID);
  emitInt32(flags);
  emitComment(toString(machine));
  emitInt16(machine);
  emitInt16(verFEMajor);
  emitInt16(verFEMinor);
  emitInt16(verFEBuild);
  emitInt16(verFEQFE);
  emitInt16(verMajor);
  emitInt16(verMinor);
  emitInt16(verBuild);
  emitInt16(verQFE);
  emitString(producer);
}

class STIProcedureFlags {
public:
  operator int() { return 0; } // FIXME
};

void
STIDebugImpl::emitSymbolProcedure(const STISymbolProcedure *procedure) const {
  int length;
  STISymbolID symbolID = procedure->getSymbolID();
  int pParent = 0;
  int pEnd = 0;
  int pNext = 0;
  const MCSymbol *labelBegin = procedure->getLabelBegin();
  const MCSymbol *labelEnd = procedure->getLabelEnd();
  const MCSymbol *labelPrologEnd = procedure->getLabelPrologEnd();
  int debugEnd = 0;
  const STIType *procType = procedure->getType();
  STIProcedureFlags flags;
  StringRef name = procedure->getName();

  // Is not this equal to: labelBegin?
  const STILineSlice *slice = procedure->getLineSlice();
  Function *function = slice->getFunction(); // FIXME
  MCSymbol *functionLabel = ASM()->getSymbol(function);

  length = 37 + name.size() + 1;

  emitInt16(length); // record length
  emitSymbolID(symbolID);
  emitInt32(pParent);
  emitInt32(pEnd);
  emitInt32(pNext);
  emitLabelDiff(labelBegin, labelEnd);
  emitLabelDiff(labelBegin, labelPrologEnd);
  emitInt32(debugEnd);
  emitInt32(procType->getIndex());
  emitSecRel32(functionLabel);
  emitSectionIndex(functionLabel);
  emitInt8(flags);
  emitString(name);
}

void
STIDebugImpl::emitSymbolThunk(const STISymbolThunk *thunk) const {
  StringRef                name          = thunk->getName();
  STISymbolThunk::Ordinal  ordinal       = thunk->getOrdinal();
  const MCSymbol          *labelBegin    = thunk->getLabelBegin();
  const MCSymbol          *labelEnd      = thunk->getLabelEnd();
  Function                *function      = thunk->getFunction();
  MCSymbol                *functionLabel = ASM()->getSymbol(function);
  int                      pParent       = 0;
  int                      pEnd          = 0;
  int                      pNext         = 0;
  int                      length;
  unsigned                 ordinalSize;
  StringRef                target;      // STI_THUNK_ADJUSTOR
  int                      adjustor;    // STI_THUNK_ADJUSTOR, STI_THUNK_VCALL
  MCSymbol                *pcodeLabel;  // STI_THUNK_PCODE

  assert(!name.empty()); // The name cannot be an empty string.

  // Determine the size of the ordinal-specific fields.
  switch (ordinal) {
  case (STI_THUNK_NOTYPE):
    ordinalSize = 0;
    break;

  case (STI_THUNK_ADJUSTOR):
    target      = thunk->getTarget();
    adjustor    = thunk->getAdjustor();
    ordinalSize = 2 + target.size() + 1;
    assert(!target.empty()); // The target cannot be an empty string.
    break;

  case (STI_THUNK_VCALL):
    adjustor    = thunk->getAdjustor();
    ordinalSize = 2;
    break;

  case (STI_THUNK_PCODE):
    pcodeLabel  = thunk->getPCODE();
    ordinalSize = 6;
    break;

  default:
    assert(!"Invalid ordinal kind!");
    ordinalSize = 0;
    break;
  }

  // Calculate the length of this S_THUNK32 record.
  length = 23 + name.size() + 1 + ordinalSize;

  // Emit the S_THUNK32 symbol fields which are independent of the ordinal.
  emitInt16         (length);
  emitSymbolID      (S_THUNK32);
  emitInt32         (pParent);
  emitInt32         (pEnd);
  emitInt32         (pNext);
  emitSecRel32      (functionLabel);
  emitSectionIndex  (functionLabel);
  emitLabelDiff     (labelBegin, labelEnd, 2);
  emitInt8          (ordinal);
  emitString        (name);

  // Emit the ordinal-specific fields.
  switch (ordinal) {
  case (STI_THUNK_NOTYPE):
    // Nothing additional is emitted.
    break;

  case (STI_THUNK_ADJUSTOR): {
    emitInt16       (adjustor);     // 'this' pointer adjustment.
    emitString      (target);
    break;
  }

  case (STI_THUNK_VCALL):
    emitInt16       (adjustor);     // virtual table adjustment.
    break;

  case (STI_THUNK_PCODE):
    emitSecRel32    (pcodeLabel);   // PCODE entry point offset.
    emitSectionIndex(pcodeLabel);   // PCODE entry point segment.
    break;

  default:
    assert(!"Invalid ordinal kind!");
    break;
  }
}

void STIDebugImpl::emitSymbolProcedureEnd() const {
  emitInt16(2);
  if (EmitFunctionIDs) {
    emitSymbolID(S_PROC_ID_END);
  } else {
    emitSymbolID(S_END);
  }
}

class STIFrameProcFlags {
public:
  operator int() { return 0; } // FIXME
};

void STIDebugImpl::emitSymbolFrameProc(const STISymbolFrameProc *frame) const {
  int length = 28;
  STISymbolID symbolID = S_FRAMEPROC;

  STIFrameProcFlags flags;


  emitInt16(length); // record length
  emitSymbolID(symbolID);
  emitInt32(0);                    // cbFrame
  emitInt32(0);                    // cbPad
  emitInt32(0);                    // offPad
  emitInt32(0);                    // cbSaveRegs
  emitInt32(0); /* FIXME: */       // offExHdlr
  emitInt16(0); /* FIXME: */       // sectExHdlr
  emitInt32(flags);                // flags
}

void STIDebugImpl::emitSymbolBlock(const STISymbolBlock *block) const {
  int length;
  STISymbolID symbolID;
  int pParent = 0;
  int pEnd = 0;
  MCSymbol *labelBegin = block->getLabelBegin();
  MCSymbol *labelEnd = block->getLabelEnd();
  StringRef name = block->getName();
  STISymbolProcedure *procedure = block->getProcedure();

  const STILineSlice *slice = procedure->getLineSlice();
  Function *function = slice->getFunction();
  MCSymbol *functionLabel = ASM()->getSymbol(function);
  // MCSymbol*           functionLabel   = procedure->getLabelBegin();

  symbolID = S_BLOCK32; // FIXME
  length = 20 + name.size() + 1;

  emitInt16(length); // record length
  emitSymbolID(symbolID);
  emitInt32(pParent);
  emitInt32(pEnd);
  emitLabelDiff(labelBegin, labelEnd);
  emitSecRel32(labelBegin);
  emitSectionIndex(functionLabel);
  emitString(name);
}

void STIDebugImpl::emitSymbolScopeEnd() const {
  emitInt16(2);
  emitSymbolID(S_END);
}

//===----------------------------------------------------------------------===//
// emitNumeric(numeric)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitNumeric(const STINumeric* numeric) const {
  const STINumeric::LeafID leafID = numeric->getLeafID();

  // Emit the leafID if this numeric encoding requires one.  Unsigned values
  // less than LF_NUMERIC (0x8000) do not require one.
  //
  if (leafID != LF_INTEL_NONE) {
    emitInt16(leafID);
  }

  // Emit the numeric value.
  //
  emitBytes(numeric->getData(), numeric->getSize());

  // The minimal field width of a numeric leaf is two bytes.  If the numeric
  // doesn't require a leaf identifier and only requires one byte then we need
  // to pad the value with a zero byte.
  //
  if (leafID == LF_INTEL_NONE && numeric->getSize() == 1) {
    emitInt8(0x00);
  }
}

//===----------------------------------------------------------------------===//
// emitSymbolConstant(symbol)
//
// Emits an entry for a constant symbol.
//
// For example, this source ...
//   +---------------------------------------------------------------------+
//   | const int N = 100;                                                  |
//   +---------------------------------------------------------------------+
//
// ... should create the following debug information symbol:
//   +---------------------------------------------------------------------+
//   | (0001A8) S_CONSTANT: Type:             0x10BC, Value: 100, N        |
//   +---------------------------------------------------------------------+
//
// The format of the S_CONSTANT symbol record is:
//   +----+----+--------+- - - - - - - -+- - - - - - -+
//   |2   |2   |4       |*              |*            |
//   +----+----+--------+- - - - - - - -+- - - - - - -+
//    ^    ^    ^        ^               ^
//    |    |    |        |               `-- name
//    |    |    |        `-- value
//    |    |    `-- typeIndex
//    |    `-- symbolID (S_CONSTANT or S_MANCONSTANT)
//    `-- length
//
// NOTE: The minimum size of the value field is two bytes.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitSymbolConstant(const STISymbolConstant *symbol) const {
  const STISymbolID symbolID    = S_CONSTANT; // S_MANCONSTANT not implemented
  int               length;
  StringRef         name        = symbol->getName();
  const STIType*    type        = symbol->getType();
  const STINumeric* value       = symbol->getValue();
  uint32_t          typeIndex   = type->getIndex();
  
  // Calculate the length in bytes of the symbolic constant, not including the
  // length field itself.
  //
  length = 2 + 4 + numericLength(value) + name.size() + 1;

  // Emit each field in the symbolic constant entry.
  //
  emitInt16     (length);
  emitSymbolID  (symbolID);
  emitInt32     (typeIndex);
  emitNumeric   (value);
  emitString    (name);
}

void STIDebugImpl::emitSymbolVariable(const STISymbolVariable *variable) const {
  assert(variable->getLocation() && "Variable with no location");
    
  STISymbolID symbolID = variable->getLocation()->getSymbolID();
  uint32_t type = variable->getType()->getIndex();
  int reg = variable->getLocation()->getReg();
  int offset = variable->getLocation()->getOffset();
  MCSymbol *label = variable->getLocation()->getLabel();
  StringRef name = variable->getName();
  int length;

  switch (symbolID) {
  case S_REGREL32:
    length = 12 + name.size() + 1;
    emitInt16(length);
    emitSymbolID(symbolID);
    emitInt32(offset);
    emitInt32(type);
    emitInt16(reg);
    emitString(name);
    break;

  case S_REGISTER:
    length = 8 + name.size() + 1;
    emitInt16(length);
    emitSymbolID(symbolID);
    emitInt32(type);
    emitInt16(reg);
    emitString(name);
    break;

  case S_BPREL32:
    length = 10 + name.size() + 1;
    emitInt16(length);
    emitSymbolID(symbolID);
    emitInt32(offset);
    emitInt32(type);
    emitString(name);
    break;

  case S_LDATA32:
  case S_GDATA32:
    length = 12 + name.size() + 1;
    emitInt16(length);
    emitSymbolID(symbolID);
    emitInt32(type);
    emitSecRel32(label);
    emitSectionIndex(label);
    emitString(name);
    break;

  default:
    assert(symbolID != symbolID); // invalid variable symbol id
    break;
  }
}

//===----------------------------------------------------------------------===//
// emitLineEntry(entry)
//
//   +--------+--------+
//   |4       |4       |
//   +--------+--------+
//    ^        ^
//    |        `-- CV_Line
//    `-- offset
//
//
// The bit encoding for the CV_Line entry is:
//
//   32 31             24
//   +--+--------------+--------------------------------------------+
//   |  |              |                                            |
//   +--+--------------+--------------------------------------------+
//    ^  ^              ^
//    |  |              `-- linenumStart
//    |  `-- deltaLineEnd
//    `-- fStatement
//
//===----------------------------------------------------------------------===//

class STILineEntryEncoding {
private:
  union {
    int32_t raw;
    struct {
      uint32_t lineNumStart : 24;
      uint32_t deltaLineEnd : 7;
      uint32_t fStatement : 1;
    } field;
  } _encoding;

public:
  STILineEntryEncoding(const STILineEntry *entry);
  ~STILineEntryEncoding();
  operator int16_t() const;
};

STILineEntryEncoding::STILineEntryEncoding(const STILineEntry *entry) {
  _encoding.raw = 0;
  _encoding.field.lineNumStart = entry->getLineNumStart();
  _encoding.field.deltaLineEnd = entry->getDeltaLineEnd();
  _encoding.field.fStatement = entry->getStatementEnd();
}

STILineEntryEncoding::~STILineEntryEncoding() {}

STILineEntryEncoding::operator int16_t() const { return _encoding.raw; }

void STIDebugImpl::emitLineEntry(const STISymbolProcedure *procedure,
                                 const STILineEntry *entry) const {
  STILineEntryEncoding encodedEntry(entry);

  emitLabelDiff(procedure->getLabelBegin(), entry->getLabel());
  emitInt32(encodedEntry);
}

//===----------------------------------------------------------------------===//
// emitLineBlock(block)
//
//   +--------+--------+--------+- - - - - - - - -
//   |4       |4       |4       |  ... lines
//   +--------+--------+--------+- - - - - - - - -
//    ^        ^        ^
//    |        |        `-- cbFileBlock
//    |        `-- clines
//    `-- offFile
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitLineBlock(const STISymbolProcedure *procedure,
                                 const STILineBlock *block) const {
  MCSymbol *labelBegin = MMI()->getContext().createTempSymbol();
  MCSymbol *labelEnd = MMI()->getContext().createTempSymbol();

  emitLabel(labelBegin);
  emitInt32(block->getChecksumEntry()->getOffset()); // offFile
  emitInt32(block->getLineCount());                  // cLines
  emitLabelDiff(labelBegin, labelEnd);               // cbFileBlock

  for (const STILineEntry *entry : block->getLines()) {
    emitLineEntry(procedure, entry);
  }

  emitLabel(labelEnd);
}

//===----------------------------------------------------------------------===//
// emitLineSlice(slice)
//
// The lines subsection begins with a header identifying the function
// associated with the slice of the line table being emitted:
//
//   +--------+----+----+--------+- - - - - - - -
//   |4       |2   |2   |4       |  ... blocks
//   +--------+----+----+--------+- - - - - - - -
//    ^        ^    ^    ^
//    |        |    |    `-- cbCon
//    |        |    `-- flags
//    |        `-- section
//    `-- secrel
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitLineSlice(const STISymbolProcedure *procedure) const {
  const STILineSlice *slice = procedure->getLineSlice();
  Function *function = slice->getFunction();
  MCSymbol *functionLabel = ASM()->getSymbol(function);

  emitSecRel32(functionLabel);
  emitSectionIndex(functionLabel);
  emitInt16(0); // FIXME: flags values?
  emitLabelDiff(procedure->getLabelBegin(), procedure->getLabelEnd());

  for (const STILineBlock *block : slice->getBlocks()) {
    emitLineBlock(procedure, block);
  }
}

void STIDebugImpl::walkSymbol(const STISymbol *symbol) const {
  STIObjectKind kind;

  // Symbols are emitted into the symbols subsection.
  emitSubsection(STI_SUBSECTION_SYMBOLS);

  kind = symbol->getKind();
  switch (kind) {
  case STI_OBJECT_KIND_SYMBOL_MODULE: {
    const STISymbolModule *module;
    module = static_cast<const STISymbolModule *>(symbol);
    emitSymbolModule(module);
    for (const STISymbolCompileUnit *unit : *module->getCompileUnits()) {
      walkSymbol(unit);
    }
  } break;

  case STI_OBJECT_KIND_SYMBOL_COMPILE_UNIT: {
    const STISymbolCompileUnit *compileUnit;
    compileUnit = static_cast<const STISymbolCompileUnit *>(symbol);
    emitSymbolCompileUnit(compileUnit);
    for (const auto &object : compileUnit->getScope()->getObjects()) {
      walkSymbol(static_cast<const STISymbol *>(object.second)); // FIXME: cast
    }
  } break;

  case STI_OBJECT_KIND_SYMBOL_PROCEDURE: {
    const STISymbolProcedure *procedure;

    procedure = static_cast<const STISymbolProcedure *>(symbol);
    emitSymbolProcedure(procedure);
    emitSymbolFrameProc(procedure->getFrame());
    for (const auto &object : procedure->getScope()->getObjects()) {
      walkSymbol(static_cast<const STISymbol *>(object.second));
    }
    emitSymbolProcedureEnd();
    // This code emits line subsections for each procedure interleaved with
    // the symbols subsections to make it easier to match them up.  Anything
    // emitted after this point will NOT be emitted to the symbols subsection.
    //
    emitSubsection(STI_SUBSECTION_LINES);
    emitLineSlice(procedure);
    // The line slice is emitted into the LINES subsection, so you can't emit
    // anything here and expect it to be emitted into the symbols subsection.
  } break;

  case STI_OBJECT_KIND_SYMBOL_THUNK: {
    const STISymbolThunk *thunk;

    thunk = static_cast<const STISymbolThunk *>(symbol);
    emitSymbolThunk(thunk);
    // For Thunk method no need to emit any additional symbol, like: frame,
    // variable, lines, etc. Just emit the end symbol and break.
    emitSymbolProcedureEnd();
  } break;

  case STI_OBJECT_KIND_SYMBOL_BLOCK: {
    const STISymbolBlock *block;

    block = static_cast<const STISymbolBlock *>(symbol);
    bool emptyBlock = true;
    for (const auto &object : block->getScope()->getObjects()) {
      if (object.second->getKind() != STI_OBJECT_KIND_SYMBOL_BLOCK) {
        emptyBlock = false;
      }
    }
    if (!emptyBlock)
      emitSymbolBlock(block);
    for (const auto &object : block->getScope()->getObjects()) {
      walkSymbol(static_cast<const STISymbol *>(object.second));
    }
    if (!emptyBlock)
      emitSymbolScopeEnd();
  } break;

  case STI_OBJECT_KIND_SYMBOL_VARIABLE: {
    const STISymbolVariable *variable;
    variable = static_cast<const STISymbolVariable *>(symbol);
    emitSymbolVariable(variable);
  } break;

  case STI_OBJECT_KIND_SYMBOL_CONSTANT: {
    const STISymbolConstant *constant;
    constant = static_cast<const STISymbolConstant*>(symbol);
    emitSymbolConstant(constant);
  } break;

  case STI_OBJECT_KIND_SYMBOL_USER_DEFINED: {
    const STISymbolUserDefined *userDefined;
    userDefined = static_cast<const STISymbolUserDefined *>(symbol);
    fixSymbolUserDefined(const_cast<STISymbolUserDefined *>(userDefined));
    emitSymbolUserDefined(userDefined);
  } break;

  default:
    assert(kind != kind); // unrecognized symbol kind!
    break;
  }
}

void STIDebugImpl::emitSectionBegin(MCSection *section) const {
  ASM()->OutStreamer->SwitchSection(section);
}

//===----------------------------------------------------------------------===//
// emitSymbols()
//
// Emits the .debug$S section.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitSymbols() const {
  emitSectionBegin(ASM()->getObjFileLowering().getCOFFDebugSymbolsSection());
  emitComment("Symbols Section Signature");
  emitInt32(STI_SECTION_SIGNATURE_CV7);
  walkSymbol(getSymbolTable()->getRoot());
  emitChecksumTable();
  emitStringTable();
  closeSubsection();
  emitAlign(4);
}

void STIDebugImpl::emitTypeBasic(const STITypeBasic *type) const {
  // Primitive Types Are Predefined And Not Emitted
}

class STITypeModifierAttributes {
private:
  union {
    int16_t raw;
    struct {
      uint16_t _const : 1;
      uint16_t _volatile : 1;
      uint16_t _unaligned_ : 1;
      uint16_t _reserved : 13;
    } field;
  } _attributes;

public:
  STITypeModifierAttributes(const STITypeModifier *const type) {
    _attributes.raw = 0;
    _attributes.field._const = type->isConstant();
    _attributes.field._volatile = type->isVolatile();
    _attributes.field._unaligned_ = type->isUnaligned();
  }

  ~STITypeModifierAttributes() {}

  operator int16_t() const { return _attributes.raw; }
};

void STIDebugImpl::emitTypeModifier(const STITypeModifier *type) const {
  STITypeModifierAttributes attributes(type);
  const STIType *qualifiedType = type->getQualifiedType();
  const uint16_t length = 10;
  const uint16_t padding = paddingBytes(length);

  typeBegin   (type);
  emitInt16   (length + padding - 2);
  emitInt16   (LF_MODIFIER);
  emitInt32   (qualifiedType->getIndex());
  emitInt16   (attributes);
  emitPadding (padding);
  typeEnd     (type);
}

class STITypePointerAttributes {
private:
  union {
    int32_t raw;
    struct {
      uint32_t _ptrtype       : 5;
      uint32_t _ptrmode       : 3;
      uint32_t _isflat32      : 1;
      uint32_t _volatile      : 1;
      uint32_t _const         : 1;
      uint32_t _unaligned_    : 1;
      uint32_t _restrict_     : 1;
      uint32_t _reserved1     : 2;
      uint32_t _unknownField1 : 1;
      uint32_t _unknownField2 : 1;
      uint32_t _reserved2     : 15;
    } field;
  } _attributes;

public:
  STITypePointerAttributes(const STITypePointer *type) {
    _attributes.raw = 0;
    if (type->getSizeInBits() == 64) {
      _attributes.field._ptrtype = ATTR_PTRTYPE_64;
      _attributes.field._unknownField2 = 1; // Necessary to get correct size!
    } else {
      _attributes.field._ptrtype = ATTR_PTRTYPE_NEAR32;
      _attributes.field._unknownField1 = 1; // Necessary to get correct size!
    }

    if (type->isLValueReference()) {
      _attributes.raw |= ATTR_PTRMODE_REFERENCE;
    }
    if (type->isRValueReference()) {
      _attributes.raw |= ATTR_PTRMODE_RVALUE;
    }

    switch (type->getPtrToMemberType()) {
    case STITypePointer::PTM_NONE:
      break;
    case STITypePointer::PTM_DATA:
      _attributes.raw |= ATTR_PTRMODE_DATAMB;
      break;
    case STITypePointer::PTM_METHOD:
      _attributes.raw |= ATTR_PTRMODE_METHOD;
      break;
    }

    if (type->isConstant()) {
      _attributes.field._const = true;
    }
  }

  ~STITypePointerAttributes() {}

  operator int32_t() const { return _attributes.raw; }
};

void STIDebugImpl::emitTypePointer(const STITypePointer *type) const {
  STITypePointerAttributes attributes(type);
  const STIType *pointerTo = type->getPointerTo();
  const STIType *classType = type->getContainingClass();
  const size_t length = 12 + (classType ? 6 : 0);
  const size_t padding = paddingBytes(length);
  int format = 0;

  switch (type->getPtrToMemberType()) {
  case STITypePointer::PTM_NONE:
    break;
  case STITypePointer::PTM_DATA:
    format = FORMAT_16_DATA_NO_VMETHOD_NO_VBASE;
    break;
  case STITypePointer::PTM_METHOD:
    format = FORMAT_16_NEAR_METHOD_NO_VBASE_SADDR;
    break;
  }

  typeBegin     (type);
  emitInt16     (length + padding - 2);
  emitInt16     (LF_POINTER);
  emitInt32     (pointerTo->getIndex());
  emitInt32     (attributes);

  if (classType) {
    emitInt32(classType->getIndex());
    emitInt16(format);
  }

  emitPadding   (padding);
  typeEnd       (type);
}

void STIDebugImpl::emitTypeArray(const STITypeArray *type) const {
  const STIType *elementType = type->getElementType();
  StringRef name = type->getName();
  const STINumeric* arrayLength = type->getLength();
  const size_t length = 12 + numericLength(arrayLength) + name.size() + 1;
  const size_t padding = paddingBytes(length);

  typeBegin     (type);
  emitInt16     (length + padding - 2);
  emitInt16     (LF_ARRAY);
  emitInt32     (elementType->getIndex());
  emitInt32     (T_ULONG);
  emitNumeric   (arrayLength);
  emitString    (name);
  emitPadding   (padding);
  typeEnd       (type);
}

void STIDebugImpl::emitTypeStructure(const STITypeStructure *type) const {
  const uint16_t leaf = type->getLeaf();
  bool isUnion = (leaf == LF_UNION);
  const uint16_t count = type->getCount();
  const uint16_t properties = type->getProperties();
  const STIType *fieldType = type->getFieldType();
  const STIType *derivedType = type->getDerivedType();
  const STIType *vshapeType = type->getVShapeType();
  const STINumeric *size = type->getSize();
  std::string name = type->getName();

  assert(!name.empty() && "empty stucture name!");

  std::string realName = getRealName(name);

  const size_t length = 12
                      + (isUnion ? 0 : 8)
                      + numericLength(size)
                      + name.size() + 1
                      + realName.size() + 1;
  const size_t padding = paddingBytes(length);

  typeBegin     (type);
  emitInt16     (length + padding - 2);
  emitInt16     (leaf);
  emitInt16     (count);
  emitInt16     (properties);
  emitInt32     (fieldType ? fieldType->getIndex() : T_NOTYPE);
  if (!isUnion) {
    emitInt32   (derivedType ? derivedType->getIndex() : T_NOTYPE);
    emitInt32   (vshapeType ? vshapeType->getIndex() : T_NOTYPE);
  }
  emitNumeric   (size);
  emitString    (name);
  emitString    (realName);
  emitPadding   (padding);
  typeEnd       (type);
}

void STIDebugImpl::emitTypeEnumeration(const STITypeEnumeration *type) const {
  const uint16_t count = type->getCount();
  const uint16_t properties = type->getProperties();
  const STIType *elementType = type->getElementType();
  const STIType *fieldType = type->getFieldType();
  StringRef name = type->getName();
  const size_t length = 16 + name.size() + 1;
  const size_t padding = paddingBytes(length);

  // If an empty name is emitted (nothing but a terminating null character),
  // then this can corrupt the PDB writer.  Make sure we never emit an
  // enumeration with a empty name.
  //
  assert(!name.empty());

  typeBegin     (type);
  emitInt16     (length + padding - 2);
  emitInt16     (LF_ENUM);
  emitInt16     (count);
  emitInt16     (properties);
  emitInt32     (elementType ? elementType->getIndex() : T_NOTYPE);
  emitInt32     (fieldType ? fieldType->getIndex() : T_NOTYPE);
  emitString    (name);
  emitPadding   (padding);
  typeEnd       (type);
}

void STIDebugImpl::emitTypeVShape(const STITypeVShape *type) const {
  const uint16_t count = type->getCount();
  const uint16_t byteCount = count / 2;
  const uint16_t tailCount = count % 2;
  const size_t length = 6 + byteCount + tailCount;
  const size_t padding = paddingBytes(length);

  typeBegin     (type);
  emitInt16     (length + padding - 2);
  emitInt16     (LF_VTSHAPE);
  emitInt16     (count);
  for (unsigned i = 0; i < byteCount; ++i) {
    emitInt8    ((CV_VFTS_NEAR32 << 4) | CV_VFTS_NEAR32);
  }
  if (tailCount != 0) {
    emitInt8    (CV_VFTS_NEAR32);
  }
  emitPadding   (padding);
  typeEnd       (type);
}

void STIDebugImpl::emitTypeBitfield(const STITypeBitfield *type) const {
  const uint32_t offset = type->getOffset();
  const uint32_t size = type->getSize();
  const STIType *memberType = type->getType();
  const size_t length = 10;
  const size_t padding = paddingBytes(length);

  typeBegin     (type);
  emitInt16     (length + padding - 2);
  emitInt16     (LF_BITFIELD);
  emitInt32     (memberType ? memberType->getIndex() : T_NOTYPE);
  emitInt8      (size);
  emitInt8      (offset);
  emitPadding   (padding);
  typeEnd       (type);
}

void STIDebugImpl::emitTypeMethodList(const STITypeMethodList *type) const {
  size_t length = 4;
  size_t padding;

  for (STITypeMethodListEntry *method : type->getList()) {
    bool isVirtual = method->getVirtuality();
    length += 8 + (isVirtual ? 4 : 0);
  }

  padding = paddingBytes(length);

  typeBegin     (type);
  emitInt16     (length + padding - 2);
  emitInt16     (LF_MLIST);

  for (STITypeMethodListEntry *method : type->getList()) {
    uint16_t attribute = method->getAttribute();
    const STIType *methodType = method->getType();
    bool isVirtual = method->getVirtuality();
    uint32_t virtualIndex = method->getVirtualIndex();

    emitInt16   (attribute);
    emitInt16   (0); // 0-Padding
    emitInt32   (methodType ? methodType->getIndex() : T_NOTYPE);

    if (isVirtual) {
      emitInt32 (virtualIndex * (getPointerSizeInBits() >> 3));
    }
  }

  emitPadding   (padding);
  typeEnd       (type);
}

//===----------------------------------------------------------------------===//
// emitTypeVBaseClass(vBaseClass)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypeVBaseClass(
      const STITypeVBaseClass *vBaseClass) const {
  STISymbolID symbolID = vBaseClass->getSymbolID();
  uint16_t attribute = vBaseClass->getAttribute();
  const STIType *vBaseClassType = vBaseClass->getType();
  const STIType *vbpType = vBaseClass->getVbpType();
  const STINumeric *offset = vBaseClass->getVbpOffset();
  const STINumeric *index  = vBaseClass->getVbIndex();
  const size_t length = 12 + numericLength(offset) + numericLength(index);
  const size_t padding = paddingBytes(length);

  emitInt16   (symbolID);
  emitInt16   (attribute);
  emitInt32   (vBaseClassType ? vBaseClassType->getIndex() : T_NOTYPE);
  emitInt32   (vbpType ? vbpType->getIndex() : T_NOTYPE);
  emitNumeric (offset);
  emitNumeric (index);
  emitPadding (padding);
}

//===----------------------------------------------------------------------===//
// emitTypeBaseClass(type)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypeBaseClass(const STITypeBaseClass *baseClass) const {
  uint16_t          attribute     = baseClass->getAttribute();
  const STIType    *baseType      = baseClass->getType();
  STIType::Index    baseIndex     = baseType ? baseType->getIndex() : T_NOTYPE;
  const STINumeric *offset        = baseClass->getOffset();
  const size_t      length        = 8 + numericLength(offset);
  const size_t      padding       = paddingBytes(length);

  emitInt16   (LF_BCLASS);
  emitInt16   (attribute);
  emitInt32   (baseIndex);
  emitNumeric (offset);
  emitPadding (padding);
}

//===----------------------------------------------------------------------===//
// emitTypeVFuncTab(vFuncTable)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypeVFuncTab(
      const STITypeVFuncTab *vFuncTab) const {
  const STIType *vptrType = vFuncTab->getType();

  emitInt16   (LF_VFUNCTAB);
  emitInt16   (0); // 0-Padding
  emitInt32   (vptrType ? vptrType->getIndex() : T_NOTYPE);
}

//===----------------------------------------------------------------------===//
// emitTypeMember(member)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypeMember(const STITypeMember *member) const {
  uint16_t attribute = member->getAttribute();
  const STIType *memberType = member->getType();
  const STINumeric *offset = member->getOffset();
  StringRef name = member->getName();
  bool isStatic = member->isStatic();
  const size_t length =
      8 + (isStatic ? 0 : numericLength(offset)) + name.size() + 1;
  const size_t padding = paddingBytes(length);

  emitInt16   (isStatic ? LF_STMEMBER : LF_MEMBER);
  emitInt16   (attribute);
  emitInt32   (memberType ? memberType->getIndex() : T_NOTYPE);
  if (!isStatic) {
    emitNumeric(offset);
  }
  emitString  (name);
  emitPadding (padding);
}

//===----------------------------------------------------------------------===//
// emitTypeMethod(method)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypeMethod(const STITypeMethod *method) const {
  uint16_t count = method->getCount();
  const STIType *methodListType = method->getList();
  StringRef name = method->getName();
  const size_t length = 8 + name.size() + 1;
  const size_t padding = paddingBytes(length);

  emitInt16   (LF_METHOD);
  emitInt16   (count);
  emitInt32   (methodListType ? methodListType->getIndex() : T_NOTYPE);
  emitString  (name);
  emitPadding (padding);
}

//===----------------------------------------------------------------------===//
// emitTypeOneMethod(method)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypeOneMethod(const STITypeOneMethod *method) const {
  uint16_t attribute = method->getAttribute();
  const STIType *methodType = method->getType();
  bool isVirtual = method->getVirtuality();
  uint32_t virtualIndex = method->getVirtualIndex();
  StringRef name = method->getName();
  const size_t length = 8 + (isVirtual ? 4 : 0) + name.size() + 1;
  const size_t padding = paddingBytes(length);

  emitInt16   (LF_ONEMETHOD);
  emitInt16   (attribute);
  emitInt32   (methodType ? methodType->getIndex() : T_NOTYPE);
  if (isVirtual) {
    emitInt32 (virtualIndex * (getPointerSizeInBits() >> 3));
  }
  emitString  (name);
  emitPadding (padding);
}

//===----------------------------------------------------------------------===//
// emitTypeEnumerator(enumerator)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypeEnumerator(
      const STITypeEnumerator *enumerator) const {
  uint16_t attribute = enumerator->getAttribute();
  const STINumeric *value = enumerator->getValue();
  StringRef name = enumerator->getName();
  const size_t length = 4 + numericLength(value) + name.size() + 1;
  const size_t padding = paddingBytes(length);

  emitInt16   (LF_ENUMERATE);
  emitInt16   (attribute);
  emitNumeric (value);
  emitString  (name);
  emitPadding (padding);
}

//===----------------------------------------------------------------------===//
// emitTypeIndex(index)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypeIndex(const STITypeIndex *index) const {
  STIType *indexedType = index->getType();

  emitInt16   (LF_INDEX);
  emitInt16   (0);          // two bytes of zeroes
  emitInt32   (indexedType ? indexedType->getIndex() : T_NOTYPE);
}

//===----------------------------------------------------------------------===//
// emitTypeFieldListItem(field)
//
// Emits the specified field list item by checking the type kind and invoking
// an appropriate handler.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypeFieldListItem(
      const STITypeFieldListItem *field) const {
  // Invoke a field-specific handler based on the field kind.
  switch (field->getKind()) {
#define X(KIND, HANDLER, TYPE)                                                \
    case (STI_OBJECT_KIND_TYPE_##KIND):                                       \
      HANDLER(static_cast<const TYPE*>(field));                               \
      break
  X(VBASECLASS, emitTypeVBaseClass, STITypeVBaseClass);
  X(BASECLASS,  emitTypeBaseClass,  STITypeBaseClass);
  X(VFUNCTAB,   emitTypeVFuncTab,   STITypeVFuncTab);
  X(MEMBER,     emitTypeMember,     STITypeMember);
  X(METHOD,     emitTypeMethod,     STITypeMethod);
  X(ONEMETHOD,  emitTypeOneMethod,  STITypeOneMethod);
  X(ENUMERATOR, emitTypeEnumerator, STITypeEnumerator);
  X(INDEX,      emitTypeIndex,      STITypeIndex);
#undef  X
  default:
    assert(!"unsupported field list item kind!");
    break;
  }
}

//===----------------------------------------------------------------------===//
// emitTypeFieldList(type)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypeFieldList(const STITypeFieldList *fieldList) const {
  size_t       length  = calculateFieldListLength(fieldList);
  const size_t padding = paddingBytes(length);

  assert(length + padding - 2 <= UINT16_MAX);

  typeBegin     (fieldList);
  emitInt16     (length + padding - 2);
  emitInt16     (LF_FIELDLIST);

  for (STITypeFieldListItem *item : fieldList->getFields()) {
    emitTypeFieldListItem(item);
  }

  emitPadding   (padding);
  typeEnd       (fieldList);
}

//===----------------------------------------------------------------------===//
// emitTypeFunctionID(type)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypeFunctionID(const STITypeFunctionID *type) const {
  StringRef name = type->getName();
  const STIType *funcType = type->getType();
  const STIType *parentClassType = type->getParentClassType();
  const STIType *parentScope =
      parentClassType ? parentClassType : type->getParentScope();
  STISymbolID symbolID = parentClassType ? LF_MFUNC_ID : LF_FUNC_ID;
  const size_t length = 12 + name.size() + 1;
  const size_t padding = paddingBytes(length);

  idBegin       (type);
  emitInt16     (length + padding - 2);
  emitInt16     (symbolID);
  emitInt32     (parentScope ? parentScope->getIndex() : T_NOTYPE);
  emitInt32     (funcType ? funcType->getIndex() : T_NOTYPE);
  emitString    (name);
  emitPadding   (padding);
  idEnd         (type);
}

//===----------------------------------------------------------------------===//
// emitTypeProcedure(type)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypeProcedure(const STITypeProcedure *type) const {
  const STIType *returnType = type->getReturnType();
  int callingConvention = type->getCallingConvention();
  uint16_t paramCount = type->getParamCount();
  const STIType *argumentList = type->getArgumentList();
  const size_t length = 16;
  const size_t padding = paddingBytes(length);

  typeBegin     (type);
  emitInt16     (length + padding - 2);
  emitInt16     (LF_PROCEDURE);
  emitInt32     (returnType ? returnType->getIndex() : T_NOTYPE);
  emitInt8      (callingConvention);
  emitInt8      (0); // reserved
  emitInt16     (paramCount);
  emitInt32     (argumentList ? argumentList->getIndex() : T_NOTYPE);
  emitPadding   (padding);
  typeEnd       (type);
}

//===----------------------------------------------------------------------===//
// emitTypeMemberFunction(type)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypeMemberFunction(
      const STITypeMemberFunction *type) const {
  const STIType *returnType = type->getReturnType();
  const STIType *classType = type->getClassType();
  const STIType *thisType = type->getThisType();
  int callingConvention = type->getCallingConvention();
  uint16_t paramCount = type->getParamCount();
  const STIType *argumentList = type->getArgumentList();
  int thisAdjust = type->getThisAdjust();
  const size_t length = 28;
  const size_t padding = paddingBytes(length);

  typeBegin     (type);
  emitInt16     (length + padding - 2);
  emitInt16     (LF_MFUNCTION);
  emitInt32     (returnType ? returnType->getIndex() : T_NOTYPE);
  emitInt32     (classType->getIndex());
  emitInt32     (thisType ? thisType->getIndex() : T_NOTYPE);
  emitInt8      (callingConvention);
  emitInt8      (0); // reserved
  emitInt16     (paramCount);
  emitInt32     (argumentList ? argumentList->getIndex() : T_NOTYPE);
  emitInt32     (thisAdjust);
  emitPadding   (padding);
  typeEnd       (type);
}

void STIDebugImpl::emitTypeArgumentList(const STITypeArgumentList *type) const {
  uint32_t argumentCount = type->getArgumentCount();
  const STITypeTable *argumentList = type->getArgumentList();
  const size_t length = 8 + (4 * argumentCount);
  const size_t padding = paddingBytes(length);

  typeBegin     (type);
  emitInt16     (length + padding - 2);
  emitInt16     (LF_ARGLIST);
  emitInt32     (argumentCount);
  for (const STIType *argumentType : *argumentList) {
    emitInt32   (argumentType ? argumentType->getIndex() : T_NOTYPE);
  }
  emitPadding   (padding);
  typeEnd       (type);
}

void STIDebugImpl::emitTypeServer(const STITypeServer *type) const {
  const size_t MAX_BUFF_LENGTH = 32;
  unsigned char signature[MAX_BUFF_LENGTH];
  unsigned char age[MAX_BUFF_LENGTH];
  StringRef name = type->getPDBFullName();
  size_t signatureLen = pdb_get_signature(signature, MAX_BUFF_LENGTH);
  size_t ageLen = pdb_get_age(age, MAX_BUFF_LENGTH);
  const size_t length = 4 + signatureLen + ageLen + name.size() + 1;
  const size_t padding = paddingBytes(length);

  emitInt16   (length + padding - 2);
  emitInt16   (LF_TYPESERVER2);
  for (size_t i=0; i < signatureLen; ++i) {
    emitInt8(signature[i]);
  }
  for (size_t i=0; i < ageLen; ++i) {
    emitInt8  (age[i]);
  }
  emitString  (name);
  emitPadding (padding);
}

//===----------------------------------------------------------------------===//
// emitType(type)
//
// Emits the specified type record.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitType(const STIType *type) const {
  STIObjectKind kind;

  kind = type->getKind();
  switch (kind) {
#define X(KIND, HANDLER, TYPE)                                                 \
  case STI_OBJECT_KIND_TYPE_##KIND:                                            \
    HANDLER(static_cast<const TYPE *>(type));                                  \
    break
    X(BASIC,            emitTypeBasic,          STITypeBasic);
    X(MODIFIER,         emitTypeModifier,       STITypeModifier);
    X(POINTER,          emitTypePointer,        STITypePointer);
    X(ARRAY,            emitTypeArray,          STITypeArray);
    X(STRUCTURE,        emitTypeStructure,      STITypeStructure);
    X(ENUMERATION,      emitTypeEnumeration,    STITypeEnumeration);
    X(VSHAPE,           emitTypeVShape,         STITypeVShape);
    X(BITFIELD,         emitTypeBitfield,       STITypeBitfield);
    X(METHOD_LIST,      emitTypeMethodList,     STITypeMethodList);
    X(FIELD_LIST,       emitTypeFieldList,      STITypeFieldList);
    X(FUNCTION_ID,      emitTypeFunctionID,     STITypeFunctionID);
    X(PROCEDURE,        emitTypeProcedure,      STITypeProcedure);
    X(MEMBER_FUNCTION,  emitTypeMemberFunction, STITypeMemberFunction);
    X(ARGUMENT_LIST,    emitTypeArgumentList,   STITypeArgumentList);
    X(SERVER,           emitTypeServer,         STITypeServer);
#undef X
  default:
    assert(kind != kind); // invalid type kind
    break;
  }
}

//===----------------------------------------------------------------------===//
// emitTypesSignature()
//
// Emits the type signature at the beginning of the .debug$T section which
// identifies the version number of the types information.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypesSignature() const {
  const STITypesSignatureID signatureID = STI_TYPES_SIGNATURE_LATEST;

  emitComment("Types Section Signature");
  emitInt32(signatureID);
}

//===----------------------------------------------------------------------===//
// emitTypesPDBTypeServer()
//
// When emitting type information to a PDB, this routine emits a LF_TYPESERVER
// record into the object file.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypesPDBTypeServer() const {
  STITypeServer* typeServer;

  // The LF_TYPESERVER entry is only emitted if a PDB is generated.
  if (!usePDB()) {
    return;
  }

  typeServer = STITypeServer::create();
  typeServer->setPDBFullName(getPDBFullPath());
  emitTypeServer(typeServer);
  delete typeServer;
}

//===----------------------------------------------------------------------===//
// emitTypesPDBBegin(savedWriter)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypesPDBBegin(STIWriter** savedWriter) const {
  STIWriter* pdbWriter;

  if (!usePDB()) {
    return;
  }

  pdbWriter = STIPdbWriter::create();
  *savedWriter = writer();
  const_cast<STIDebugImpl*>(this)->setWriter(pdbWriter);
}

//===----------------------------------------------------------------------===//
// emitTypesPDBEnd(savedWriter)
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypesPDBEnd(STIWriter** savedWriter) const {
  STIWriter* pdbWriter;

  if (!usePDB()) {
    return;
  }

  pdbWriter = const_cast<STIDebugImpl*>(this)->writer();
  const_cast<STIDebugImpl*>(this)->setWriter(*savedWriter);
  delete pdbWriter;
}

//===----------------------------------------------------------------------===//
// emitTypesTable()
//
// Emits all of the types from the types table, in order.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypesTable() const {
  for (const STIType *type : *getTypeTable()) {
    emitType(type);
  }
}

//===----------------------------------------------------------------------===//
// emitTypes()
//
// Emits the .debug$T section.
//
//===----------------------------------------------------------------------===//

void STIDebugImpl::emitTypes() const {
  STIWriter* savedWriter = nullptr;

  emitSectionBegin(ASM()->getObjFileLowering().getCOFFDebugTypesSection());
  emitTypesSignature();
  emitTypesPDBTypeServer();
  emitTypesPDBBegin(&savedWriter);
  emitTypesTable();
  emitTypesPDBEnd(&savedWriter);
}

void STIDebugImpl::emitSymbolUserDefined(
    const STISymbolUserDefined *userDefined) const {
  const STIType *definedType = userDefined->getDefinedType();
  StringRef name = userDefined->getName();
  const int16_t length = 6 + name.size() + 1;

  emitInt16(length);
  emitInt16(S_UDT);
  emitInt32(definedType->getIndex());
  emitString(name);
}

void STIDebugImpl::emitStringEntry(const STIStringEntry *entry) const {
  emitString(entry->getString());
}

void STIDebugImpl::emitStringTable() const {
  emitSubsection(STI_SUBSECTION_STRINGTABLE);

  for (const STIStringEntry *entry : getStringTable()->getEntries()) {
    emitStringEntry(entry);
  }
}

void STIDebugImpl::emitChecksumTable() const {
  emitSubsection(STI_SUBSECTION_FILECHKSMS);

  for (const STIChecksumEntry *entry : getChecksumTable()->getEntries()) {
    emitChecksumEntry(entry);
  }
}

size_t STIDebugImpl::getPaddingSize(const STIChecksumEntry *entry) const {
  return 4 - ((6 + entry->getChecksumSize()) % 4);
}

void STIDebugImpl::emitChecksumEntry(const STIChecksumEntry *entry) const {
  emitInt32(entry->getStringEntry()->getOffset());
  emitInt8(entry->getChecksumSize());
  emitInt8(entry->getType());
  emitBytes(entry->getChecksum(), entry->getChecksumSize());
  emitFill(getPaddingSize(entry), 0x00);
}

//===----------------------------------------------------------------------===//
// STIDebug Routines
//===----------------------------------------------------------------------===//

STIDebug::STIDebug() {}

STIDebug::~STIDebug() {}

STIDebug *STIDebug::create(AsmPrinter *Asm) { return new STIDebugImpl(Asm); }
