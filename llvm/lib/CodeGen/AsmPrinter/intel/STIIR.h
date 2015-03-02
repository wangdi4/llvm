//===-- STIIR.h - Symbol And Type Info -------*- C++ -*--===//
//
//===----------------------------------------------------------------------===//
//
// This file contains data structures for symbol and type information entries.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_ASMPRINTER_STIIR_H
#define LLVM_LIB_CODEGEN_ASMPRINTER_STIIR_H

#include "STI.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/DebugInfo.h"

namespace llvm {

class Function;
class MCSymbol;
class Module;

//===----------------------------------------------------------------------===//
// Forward References
//===----------------------------------------------------------------------===//

class STILocation;
class STIScope;
class STISymbol;
class STISymbolModule;
class STISymbolCompileUnit;
class STISymbolProcedure;
class STISymbolBlock;
class STISymbolVariable;
class STISymbolUserDefined;
class STIType;
class STITypeModifier;
class STITypePointer;
class STITypeArray;
class STITypeBitfield;
class STITypeMember;
class STITypeEnumerator;
class STITypeFieldList;
class STITypeStructure;
class STITypeEnumeration;
class STITypeFunctionID;
class STITypeProcedure;
class STITypeArgumentList;
class STIStringEntry;
class STIStringTable;
class STIChecksumEntry;
class STIChecksumTable;
class STILineEntry;
class STILineBlock;
class STILineSlice;
class STILineTable;

typedef uint32_t STISymbolID; // FIXME: Make enum and move to STI.h

//===----------------------------------------------------------------------===//
// STIObject
//===----------------------------------------------------------------------===//

enum STIObjectKindEnum {
  STI_OBJECT_KIND_NONE = 0,
#if 0
  STI_OBJECT_KIND_INSTRUCTION,         // STIInstruction
#endif
  STI_OBJECT_KIND_LOCATION,            // STILocation
  STI_OBJECT_KIND_SCOPE,               // STIScope
  STI_OBJECT_KIND_SYMBOL_MODULE,       // STISymbolModule
  STI_OBJECT_KIND_SYMBOL_COMPILE_UNIT, // STISymbolCompileUnit
  STI_OBJECT_KIND_SYMBOL_PROCEDURE,    // STISymbolProcedure
  STI_OBJECT_KIND_SYMBOL_BLOCK,        // STISymbolBlock
  STI_OBJECT_KIND_SYMBOL_VARIABLE,     // STISymbolVariable
  STI_OBJECT_KIND_SYMBOL_USER_DEFINED, // STISymbolUserDefined
  STI_OBJECT_KIND_TYPE_BASIC,          // STITypeBasic
  STI_OBJECT_KIND_TYPE_MODIFIER,       // STITypeModifier
  STI_OBJECT_KIND_TYPE_POINTER,        // STITypePointer
  STI_OBJECT_KIND_TYPE_ARRAY,          // STITypeArray
  STI_OBJECT_KIND_TYPE_STRUCTURE,      // STITypeStructure
  STI_OBJECT_KIND_TYPE_ENUMERATION,    // STITypeEnumeration
  STI_OBJECT_KIND_TYPE_BITFIELD,       // STITypeBitfield
  STI_OBJECT_KIND_TYPE_FIELD_LIST,     // STITypeFieldList
  STI_OBJECT_KIND_TYPE_FUNCTION_ID,    // STITypeFunctionID
  STI_OBJECT_KIND_TYPE_PROCEDURE,      // STITypeProcedure
  STI_OBJECT_KIND_TYPE_ARGUMENT_LIST,  // STITypeArgumentList
};
typedef enum STIObjectKindEnum STIObjectKind;

class STIObject {
private:
  STIObjectKind _kind;
  uint32_t _uniqueID;
  static uint32_t _countUniqueID;

public:
  STIObject(STIObjectKind kind);
  ~STIObject();

  STIObjectKind getKind() const;
};

//===----------------------------------------------------------------------===//
// STILocation
//===----------------------------------------------------------------------===//

class STILocation : public STIObject {
private:
  STISymbolID _symbolID;
  STIRegID _regnum;
  int _offset;
  MCSymbol *_label;

protected:
  STILocation(STISymbolID symbolID, STIRegID regnum, int offset,
              MCSymbol *label);

public:
  ~STILocation();

  static STILocation *createRegisterOffset(STIRegID reg, int offset);
  static STILocation *createRegister(STIRegID reg);
  static STILocation *createOffset(int offset);
  static STILocation *createGlobalSegmentedOffset(MCSymbol *label);
  static STILocation *createLocalSegmentedOffset(MCSymbol *label);

  STISymbolID getSymbolID() const;
  void setSymbolID(STISymbolID symbolID);
  STIRegID getReg() const;
  void setReg(STIRegID reg);
  int getOffset() const;
  void setOffset(int offset);
  MCSymbol *getLabel() const;
  void setLabel(MCSymbol *label);
};

//===----------------------------------------------------------------------===//
// STIStringEntry
//===----------------------------------------------------------------------===//

class STIStringEntry {
public:
  typedef uint32_t Offset;

private:
  StringRef _string;
  Offset _offset;

protected:
  STIStringEntry();

public:
  ~STIStringEntry();

  static STIStringEntry *create();

  StringRef getString() const;
  void setString(StringRef string);
  Offset getOffset() const;
  void setOffset(Offset offset);
};

//===----------------------------------------------------------------------===//
// STIStringTable
//===----------------------------------------------------------------------===//

class STIStringTable {
public:
  typedef std::vector<STIStringEntry *> EntryList;

private:
  EntryList _entries;

  STIStringEntry *lookup(StringRef string);
  STIStringEntry *append(StringRef string);

public:
  STIStringTable();
  ~STIStringTable();

  EntryList &getEntries();
  const EntryList &getEntries() const;
  STIStringEntry *find(StringRef string);
};

//===----------------------------------------------------------------------===//
// STIChecksumEntry
//===----------------------------------------------------------------------===//

class STIChecksumEntry {
public:
  typedef uint32_t Offset;

  enum TypeEnum {
    STI_FILECHECKSUM_ENTRY_TYPE_NONE = 0,
    STI_FILECHECKSUM_ENTRY_TYPE_MD5 = 1,
    STI_FILECHECKSUM_ENTRY_TYPE_SHA1 = 2
  };
  typedef enum TypeEnum Type;

  typedef const char *Checksum;

private:
  STIStringEntry *_stringEntry;
  Offset _offset;
  Type _type;
  Checksum _checksum;

public:
  STIChecksumEntry();
  ~STIChecksumEntry();

  static STIChecksumEntry *create();

  StringRef getFilename() const;
  STIStringEntry *getStringEntry() const;
  void setStringEntry(STIStringEntry *entry);
  Offset getOffset() const;
  void setOffset(Offset offset);
  Type getType() const;
  void setType(Type type);
  Checksum getChecksum() const;
  void setChecksum(Checksum checksum);

  size_t getChecksumSize() const;
};

//===----------------------------------------------------------------------===//
// STIChecksumTable
//===----------------------------------------------------------------------===//

class STIChecksumTable {
public:
  typedef std::vector<STIChecksumEntry *> EntryList;

private:
  EntryList _entries;

public:
  STIChecksumTable();
  ~STIChecksumTable();

  EntryList &getEntries();
  const EntryList &getEntries() const;

  void append(STIChecksumEntry *entry);
};

//===----------------------------------------------------------------------===//
// STILineEntry
//
// A line table entry correlating a machine instruction to a source line.
//
//===----------------------------------------------------------------------===//

class STILineEntry {
private:
  class LineNumber {
  public:
    uint32_t _lineNumStart : 24;
    uint32_t _deltaLineEnd : 7;
    uint32_t _fStatement : 1;

    LineNumber() : _lineNumStart(0), _deltaLineEnd(0), _fStatement(0) {}
  };

  const MCSymbol *_label;
  LineNumber _line;
  // Column       _column;

protected:
  STILineEntry();

public:
  ~STILineEntry();

  static STILineEntry *create();

  const MCSymbol *getLabel() const;
  void setLabel(const MCSymbol *const symbol);
  uint32_t getLineNumStart() const;
  void setLineNumStart(const uint32_t lineNumStart);
  uint32_t getDeltaLineEnd() const;
  void setDeltaLineEnd(const uint32_t deltaLineEnd);
  bool getStatementEnd() const;
  void setStatementEnd(const bool statementEnd);
};

//===----------------------------------------------------------------------===//
// STILineBlock
//
// A block of line table entries which all have a common source file.
//
//===----------------------------------------------------------------------===//

class STILineBlock {
public:
  typedef std::vector<STILineEntry *> LineEntries;

private:
  STIChecksumEntry *_checksumEntry;
  LineEntries _lineEntries;

protected:
  STILineBlock();

public:
  ~STILineBlock();

  static STILineBlock *create();

  StringRef getFilename() const;
  STIChecksumEntry *getChecksumEntry() const;
  void setChecksumEntry(STIChecksumEntry *entry);
  LineEntries &getLines();
  const LineEntries &getLines() const;
  size_t getLineCount() const;
  void appendLine(STILineEntry *entry);
};

//===----------------------------------------------------------------------===//
// STILineSlice
//
// A segment of the machine instruction to source line correlation specific to
// a single procedure.
//
//===----------------------------------------------------------------------===//

class STILineSlice {
public:
  typedef std::vector<STILineBlock *> BlockList;

private:
  Function *_function;
  BlockList _blocks;

public:
  STILineSlice();
  ~STILineSlice();

  static STILineSlice *create();

  Function *getFunction() const;
  void setFunction(Function *function);
  BlockList &getBlocks();
  const BlockList &getBlocks() const;
  void appendBlock(STILineBlock *block);
};

//===----------------------------------------------------------------------===//
// STILineTable
//===----------------------------------------------------------------------===//

class STILineTable {
public:
  typedef std::vector<STILineSlice *> SliceList;

private:
  SliceList _slices;

public:
  STILineTable();
  ~STILineTable();

  SliceList &getSlices();
  const SliceList &getSlices() const;
  void appendSlice(STILineSlice *slice);
};

//===----------------------------------------------------------------------===//
// STISymbol
//===----------------------------------------------------------------------===//

class STISymbol : public STIObject {
public:
  STISymbol(STIObjectKind kind);
  ~STISymbol();
};

//===----------------------------------------------------------------------===//
// STISymbolModule
//===----------------------------------------------------------------------===//

class STISymbolModule : public STISymbol {
public:
  typedef std::vector<STISymbolCompileUnit *> CompileUnitList;

private:
  STISignatureID _signatureID;
  StringRef _path;
  CompileUnitList _compileUnits;

protected:
  STISymbolModule();

public:
  static STISymbolModule *create(const Module *module);
  ~STISymbolModule();

  STISignatureID getSignatureID() const;
  void setSignatureID(STISignatureID signatureID);
  StringRef getPath() const;
  void setPath(StringRef path);
  const CompileUnitList *getCompileUnits() const;

  void add(STISymbolCompileUnit *compileUnit);
};

//===----------------------------------------------------------------------===//
// STISymbolCompileUnit
//===----------------------------------------------------------------------===//

class STISymbolCompileUnit : public STISymbol {
private:
  STIMachineID _machineID;
  StringRef _producer;
  STIScope *_scope;

protected:
  STISymbolCompileUnit();

public:
  static STISymbolCompileUnit *create(const DICompileUnit compileUnit);

  ~STISymbolCompileUnit();

  STIMachineID getMachineID() const;
  void setMachineID(STIMachineID machineID);
  StringRef getProducer() const;
  void setProducer(StringRef producer);
  STIScope *getScope() const;
};

//===----------------------------------------------------------------------===//
// STISymbolProcedure
//===----------------------------------------------------------------------===//

class STISymbolProcedure : public STISymbol {
private:
  STISymbolID _symbolID;
  StringRef _name;
  STIType *_type;
  STIScope *_scope;
  MCSymbol *_labelBegin;
  MCSymbol *_labelEnd;
  MCSymbol *_labelPrologEnd;
  STILineSlice *_lineSlice;

protected:
  STISymbolProcedure();

public:
  static STISymbolProcedure *create();

  ~STISymbolProcedure();

  STISymbolID getSymbolID() const;
  void setSymbolID(STISymbolID symbolID);
  StringRef getName() const;
  void setName(StringRef name);
  STIType *getType() const;
  void setType(STIType *type);
  STIScope *getScope() const;
  void setScope(STIScope *scope);
  MCSymbol *getLabelBegin() const;
  void setLabelBegin(MCSymbol *labelBegin);
  MCSymbol *getLabelEnd() const;
  void setLabelEnd(MCSymbol *labelEnd);
  MCSymbol *getLabelPrologEnd() const;
  void setLabelPrologEnd(MCSymbol *labelPrologEnd);

  STILineSlice *getLineSlice();
  const STILineSlice *getLineSlice() const;

  void add(STIObject *object);
};

//===----------------------------------------------------------------------===//
// STISymbolBlock
//===----------------------------------------------------------------------===//

class STISymbolBlock : public STISymbol {
private:
  StringRef _name;
  STIScope *_scope;
  MCSymbol *_labelBegin;
  MCSymbol *_labelEnd;
  STISymbolProcedure *_procedure;

protected:
  STISymbolBlock();

public:
  static STISymbolBlock *create();

  ~STISymbolBlock();

  StringRef getName() const;
  void setName(StringRef name);
  STIScope *getScope() const;
  void setScope(STIScope *scope);
  MCSymbol *getLabelBegin() const;
  void setLabelBegin(MCSymbol *labelBegin);
  MCSymbol *getLabelEnd() const;
  void setLabelEnd(MCSymbol *labelEnd);
  STISymbolProcedure *getProcedure() const;
  void setProcedure(STISymbolProcedure *procedure);

  void add(STIObject *object);
};

//===----------------------------------------------------------------------===//
// STISymbolVariable
//===----------------------------------------------------------------------===//

class STISymbolVariable : public STISymbol {
private:
  StringRef _name;
  STILocation *_location;
  STIType *_type;

protected:
  STISymbolVariable();

public:
  static STISymbolVariable *create();

  ~STISymbolVariable();

  StringRef getName() const;
  void setName(StringRef name);
  STILocation *getLocation() const;
  void setLocation(STILocation *location);
  STIType *getType() const;
  void setType(STIType *type);
};

//===----------------------------------------------------------------------===//
// STIType
//===----------------------------------------------------------------------===//

typedef uint32_t STITypeIndex;

class STIType : public STIObject {
private:
  STITypeIndex _index;
  uint32_t _sizeInBits;

protected:
  STIType(STIObjectKind kind);

public:
  ~STIType();

  STITypeIndex getIndex() const;
  void setIndex(STITypeIndex index);
  uint32_t getSizeInBits() const;
  void setSizeInBits(uint32_t sizeInBits);
};

//===----------------------------------------------------------------------===//
// STITypeBasic
//===----------------------------------------------------------------------===//

class STITypeBasic : public STIType {
public:
  typedef uint16_t Primitive;

private:
  Primitive _primitive;

protected:
  STITypeBasic();

public:
  ~STITypeBasic();

  static STITypeBasic *create();

  Primitive getPrimitive() const;
  void setPrimitive(Primitive primitive);
};

//===----------------------------------------------------------------------===//
// STITypeModifier
//===----------------------------------------------------------------------===//

class STITypeModifier : public STIType {
private:
  STIType *_qualifiedType;
  bool _isConstant : 1;
  bool _isVolatile : 1;
  bool _isUnaligned : 1;

protected:
  STITypeModifier();

public:
  ~STITypeModifier();

  static STITypeModifier *create();

  STIType *getQualifiedType() const;
  void setQualifiedType(STIType *qualifiedType);

  bool isConstant() const;
  void setIsConstant(bool isConstant);
  bool isVolatile() const;
  void setIsVolatile(bool isVolatile);
  bool isUnaligned() const;
  void setIsUnaligned(bool isUnaligned);
};

//===----------------------------------------------------------------------===//
// STITypePointer
//===----------------------------------------------------------------------===//

class STITypePointer : public STIType {
private:
  STIType *_pointerTo;

protected:
  STITypePointer();

public:
  ~STITypePointer();

  static STITypePointer *create();

  STIType *getPointerTo() const;
  void setPointerTo(STIType *pointerTo);
};

//===----------------------------------------------------------------------===//
// STITypeArray
//===----------------------------------------------------------------------===//

class STITypeArray : public STIType {
private:
  STIType *_elementType;
  StringRef _name;
  uint32_t _length;

protected:
  STITypeArray();

public:
  ~STITypeArray();

  static STITypeArray *create();

  STIType *getElementType() const;
  void setElementType(STIType *elementType);

  StringRef getName() const;
  void setName(StringRef name);

  uint32_t getLength() const;
  void setLength(uint32_t length);
};

//===----------------------------------------------------------------------===//
// STITypeBitfield
//===----------------------------------------------------------------------===//

class STITypeBitfield : public STIType {
private:
  STIType *_type;
  uint32_t _offset;
  uint32_t _size;

protected:
  STITypeBitfield();

public:
  ~STITypeBitfield();

  static STITypeBitfield *create();

  STIType *getType() const;
  void setType(STIType *type);

  uint32_t getOffset() const;
  void setOffset(uint32_t offset);

  uint32_t getSize() const;
  void setSize(uint32_t size);
};

//===----------------------------------------------------------------------===//
// STITypeMember
//===----------------------------------------------------------------------===//

class STITypeMember {
private:
  uint16_t _attribute;
  STIType *_type;
  uint32_t _offset;
  StringRef _name;

protected:
  STITypeMember();

public:
  ~STITypeMember();

  static STITypeMember *create();

  uint16_t getAttribute() const;
  void setAttribute(uint16_t attr);

  STIType *getType() const;
  void setType(STIType *type);

  uint32_t getOffset() const;
  void setOffset(uint32_t offset);

  StringRef getName() const;
  void setName(StringRef name);
};

//===----------------------------------------------------------------------===//
// STITypeEnumerator
//===----------------------------------------------------------------------===//

class STITypeEnumerator {
private:
  uint16_t _attribute;
  uint32_t _value;
  StringRef _name;

protected:
  STITypeEnumerator();

public:
  ~STITypeEnumerator();

  static STITypeEnumerator *create();

  uint16_t getAttribute() const;
  void setAttribute(uint16_t attr);

  uint32_t getValue() const;
  void setValue(uint32_t value);

  StringRef getName() const;
  void setName(StringRef name);
};

//===----------------------------------------------------------------------===//
// STITypeFieldList
//===----------------------------------------------------------------------===//

class STITypeFieldList : public STIType {
public:
  typedef std::vector<STITypeMember *> STITypeMemberList;
  typedef std::vector<STITypeEnumerator *> STITypeEnumeratorList;

private:
  STITypeMemberList _members;
  STITypeEnumeratorList _enumerators;

protected:
  STITypeFieldList();

public:
  ~STITypeFieldList();

  static STITypeFieldList *create();

  STITypeMemberList &getMembers();
  const STITypeMemberList &getMembers() const;

  STITypeEnumeratorList &getEnumerators();
  const STITypeEnumeratorList &getEnumerators() const;
};

//===----------------------------------------------------------------------===//
// STITypeStructure
//===----------------------------------------------------------------------===//

class STITypeStructure : public STIType {
private:
  uint16_t _leaf;
  uint16_t _count;
  uint16_t _property;
  STIType *_fieldType;
  STIType *_derivedType;
  STIType *_vshapeType;
  uint32_t _size;
  StringRef _name;

protected:
  STITypeStructure();

public:
  ~STITypeStructure();

  static STITypeStructure *create();

  uint16_t getLeaf() const;
  void setLeaf(uint16_t leaf);

  uint16_t getCount() const;
  void setCount(uint16_t count);

  uint16_t getProperty() const;
  void setProperty(uint16_t prop);

  STIType *getFieldType() const;
  void setFieldType(STIType *fieldType);

  STIType *getDerivedType() const;
  void setDerivedType(STIType *derivedType);

  STIType *getVShapeType() const;
  void setVShapeType(STIType *vshapeType);

  uint32_t getSize() const;
  void setSize(uint32_t size);

  StringRef getName() const;
  void setName(StringRef name);
};

//===----------------------------------------------------------------------===//
// STITypeEnumeration
//===----------------------------------------------------------------------===//

class STITypeEnumeration : public STIType {
private:
  uint16_t _leaf;
  uint16_t _count;
  uint16_t _property;
  STIType *_elementType;
  STIType *_fieldType;
  StringRef _name;

protected:
  STITypeEnumeration();

public:
  ~STITypeEnumeration();

  static STITypeEnumeration *create();

  uint16_t getCount() const;
  void setCount(uint16_t count);

  uint16_t getProperty() const;
  void setProperty(uint16_t prop);

  STIType *getElementType() const;
  void setElementType(STIType *elementType);

  STIType *getFieldType() const;
  void setFieldType(STIType *fieldType);

  StringRef getName() const;
  void setName(StringRef name);
};

//===----------------------------------------------------------------------===//
// STITypeFunctionID
//===----------------------------------------------------------------------===//

class STITypeFunctionID : public STIType {
private:
  STIType *_type;
  STIType *_parentScope;
  StringRef _name;

protected:
  STITypeFunctionID();

public:
  ~STITypeFunctionID();

  static STITypeFunctionID *create();

  STIType *getType() const;
  void setType(STIType *type);

  STIType *getParentScope() const;
  void setParentScope(STIType *parentScope);

  StringRef getName() const;
  void setName(StringRef name);
};

//===----------------------------------------------------------------------===//
// STITypeProcedure
//===----------------------------------------------------------------------===//

class STITypeProcedure : public STIType {
private:
  STIType *_returnType;
  int _callingConvention;
  uint16_t _paramCount;
  STIType *_argumentList;

protected:
  STITypeProcedure();

public:
  ~STITypeProcedure();

  static STITypeProcedure *create();

  STIType *getReturnType() const;
  void setReturnType(STIType *returnType);

  int getCallingConvention() const;
  void setCallingConvention(int callingConvention);

  uint16_t getParamCount() const;
  void setParamCount(uint16_t paramCount);

  STIType *getArgumentList() const;
  void setArgumentList(STIType *argumentList);
};

//===----------------------------------------------------------------------===//
// STITypeArgumentList
//===----------------------------------------------------------------------===//

class STITypeArgumentList : public STIType {
public:
  typedef std::vector<STIType *> STIArgTypeList;

private:
  STIArgTypeList _argumentList;

protected:
  STITypeArgumentList();

public:
  ~STITypeArgumentList();

  static STITypeArgumentList *create();

  uint32_t getArgumentCount() const;

  const STIArgTypeList *getArgumentList() const;
  STIArgTypeList *getArgumentList();
};

//===----------------------------------------------------------------------===//
// STISymbolUserDefined
//===----------------------------------------------------------------------===//

class STISymbolUserDefined : public STISymbol {
private:
  STIType *_definedType;
  StringRef _name;

protected:
  STISymbolUserDefined();

public:
  ~STISymbolUserDefined();

  static STISymbolUserDefined *create();

  STIType *getDefinedType() const;
  void setDefinedType(STIType *definedType);

  StringRef getName() const;
  void setName(StringRef name);
};

//===----------------------------------------------------------------------===//
// STIScope
//===----------------------------------------------------------------------===//

class STIScope : public STIObject {
public:
  typedef std::vector<STIObject *> ObjectList;

private:
  STIScope *_parent;
  STISymbol *_symbol;
  ObjectList _objects;

public:
  STIScope(STISymbol *symbol);
  ~STIScope();

  static STIScope *create(STISymbol *symbol);

  STIScope *getParent() const;
  void setParent(STIScope *parent);
  STISymbol *getSymbol() const;
  void setSymbol(STISymbol *symbol);
  const ObjectList &getObjects() const;

  void add(STIObject *object);
};

//===----------------------------------------------------------------------===//
// STISubsection
//===----------------------------------------------------------------------===//

class STISubsection {
private:
  STISubsectionID _id;
  MCSymbol *_begin;
  MCSymbol *_end;

public:
  STISubsection(STISubsectionID id);

  ~STISubsection();

  STISubsectionID getID() const;

  MCSymbol *getBegin() const;
  void setBegin(MCSymbol *begin);

  MCSymbol *getEnd() const;
  void setEnd(MCSymbol *end);
};

//===----------------------------------------------------------------------===//
// STISymbolTable
//===----------------------------------------------------------------------===//

class STISymbolTable {
private:
  STISymbol *_root;

public:
  STISymbolTable();
  ~STISymbolTable();

  STISymbol *getRoot() const;
  void setRoot(STISymbol *symbol);
};

} // end namespace llvm

#endif
