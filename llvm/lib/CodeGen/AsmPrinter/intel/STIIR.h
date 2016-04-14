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
class STINumeric;
class STISymbol;
class STISymbolModule;
class STISymbolCompileUnit;
class STISymbolConstant;
class STISymbolProcedure;
class STISymbolFrameProc;
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
class STITypeBaseClass;
class STITypeVBaseClass;
class STITypeVFuncTab;
class STITypeFieldList;
class STITypeStructure;
class STITypeEnumeration;
class STITypeVShape;
class STITypeMethodList;
class STITypeFunctionID;
class STITypeProcedure;
class STITypeMemberFunction;
class STITypeArgumentList;
class STIStringEntry;
class STIStringTable;
class STIChecksumEntry;
class STIChecksumTable;
class STILineEntry;
class STILineBlock;
class STILineSlice;

typedef uint32_t STISymbolID; // FIXME: Make enum and move to STI.h

//===----------------------------------------------------------------------===//
// STIObjectKind
//===----------------------------------------------------------------------===//

#define STI_OBJECT_KIND_LIST                                                  \
    X(STI_OBJECT_KIND_NONE,                 0)                                \
    X(STI_OBJECT_KIND_LOCATION,             1)   /* STILocation           */  \
    X(STI_OBJECT_KIND_SCOPE,                2)   /* STIScope              */  \
    X(STI_OBJECT_KIND_SYMBOL_MODULE,        3)   /* STISymbolModule       */  \
    X(STI_OBJECT_KIND_SYMBOL_COMPILE_UNIT,  4)   /* STISymbolCompileUnit  */  \
    X(STI_OBJECT_KIND_SYMBOL_CONSTANT,      5)   /* STISymbolConstant     */  \
    X(STI_OBJECT_KIND_SYMBOL_PROCEDURE,     6)   /* STISymbolProcedure    */  \
    X(STI_OBJECT_KIND_SYMBOL_THUNK,         7)   /* STISymbolThunk        */  \
    X(STI_OBJECT_KIND_SYMBOL_FRAMEPROC,     8)   /* STISymbolFrameProc    */  \
    X(STI_OBJECT_KIND_SYMBOL_BLOCK,         9)   /* STISymbolBlock        */  \
    X(STI_OBJECT_KIND_SYMBOL_VARIABLE,      10)  /* STISymbolVariable     */  \
    X(STI_OBJECT_KIND_SYMBOL_USER_DEFINED,  11)  /* STISymbolUserDefined  */  \
    X(STI_OBJECT_KIND_TYPE_BASIC,           12)  /* STITypeBasic          */  \
    X(STI_OBJECT_KIND_TYPE_INDEX,           13)  /* STITypeIndex          */  \
    X(STI_OBJECT_KIND_TYPE_MODIFIER,        14)  /* STITypeModifier       */  \
    X(STI_OBJECT_KIND_TYPE_POINTER,         15)  /* STITypePointer        */  \
    X(STI_OBJECT_KIND_TYPE_ARRAY,           16)  /* STITypeArray          */  \
    X(STI_OBJECT_KIND_TYPE_STRUCTURE,       17)  /* STITypeStructure      */  \
    X(STI_OBJECT_KIND_TYPE_ENUMERATION,     18)  /* STITypeEnumeration    */  \
    X(STI_OBJECT_KIND_TYPE_VSHAPE,          19)  /* STITypeVShape         */  \
    X(STI_OBJECT_KIND_TYPE_BITFIELD,        20)  /* STITypeBitfield       */  \
    X(STI_OBJECT_KIND_TYPE_METHOD_LIST,     21)  /* STITypeMethodList     */  \
    X(STI_OBJECT_KIND_TYPE_FIELD_LIST,      22)  /* STITypeFieldList      */  \
    X(STI_OBJECT_KIND_TYPE_FUNCTION_ID,     23)  /* STITypeFunctionID     */  \
    X(STI_OBJECT_KIND_TYPE_PROCEDURE,       24)  /* STITypeProcedure      */  \
    X(STI_OBJECT_KIND_TYPE_MEMBER_FUNCTION, 25)  /* STITypeMemberFunction */  \
    X(STI_OBJECT_KIND_TYPE_ARGUMENT_LIST,   26)  /* STITypeArgumentList   */  \
    X(STI_OBJECT_KIND_TYPE_VBASECLASS,      27)  /* STITypeVBaseClass     */  \
    X(STI_OBJECT_KIND_TYPE_BASECLASS,       28)  /* STITypeBaseClass      */  \
    X(STI_OBJECT_KIND_TYPE_VFUNCTAB,        29)  /* STITypeVFuncTab       */  \
    X(STI_OBJECT_KIND_TYPE_MEMBER,          30)  /* STITypeMember         */  \
    X(STI_OBJECT_KIND_TYPE_METHOD,          31)  /* STITypeMethod         */  \
    X(STI_OBJECT_KIND_TYPE_ONEMETHOD,       32)  /* STITypeOneMethod      */  \
    X(STI_OBJECT_KIND_TYPE_ENUMERATOR,      33)  /* STITypeEnumerator     */  \
    X(STI_OBJECT_KIND_TYPE_SERVER,          34)  /* STITypeServer         */

enum STIObjectKindEnum {
#define X(KIND, VALUE) KIND = VALUE,
  STI_OBJECT_KIND_LIST
#undef  X
};
typedef enum STIObjectKindEnum STIObjectKind;

//===----------------------------------------------------------------------===//
// toObjectKindString(kind)
//
// Returns a string describing the specified object kind, to be used for
// debugging/dumping purposes only.
//
//===----------------------------------------------------------------------===//

const char* toObjectKindString(STIObjectKind kind);

//===----------------------------------------------------------------------===//
// STIObject
//
// Root object type for all STI IR objects.
//
//===----------------------------------------------------------------------===//

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
  typedef DenseMap<const STIStringEntry *, STIChecksumEntry *> EntryMap;

private:
  EntryList _entries;
  EntryMap _map;

public:
  STIChecksumTable();
  ~STIChecksumTable();

  EntryList &getEntries();
  const EntryList &getEntries() const;

  STIChecksumEntry *findEntry(const STIStringEntry *string) const;

  void append(STIStringEntry *string, STIChecksumEntry *entry);
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
// STINumeric
//===----------------------------------------------------------------------===//

class STINumeric {
public:
  typedef uint16_t LeafID;

  static STINumeric* create(LeafID leafID, size_t size, const char* data);

  ~STINumeric();

  LeafID        getLeafID () const;
  size_t        getSize   () const;
  const char*   getData   () const;

protected:
  STINumeric(LeafID leafID, size_t size, const char* data);

private:
  LeafID    _leafID;
  size_t    _size;
  char*     _data;
};

//===----------------------------------------------------------------------===//
// STICompositeProperties
//
// A collection of STICompositeProperty values which are used for class,
// struct, union, and enumeration types.
//
//===----------------------------------------------------------------------===//

class STICompositeProperties {
private:
  uint16_t _value;

public:
  typedef STICompositeProperty Property;

  STICompositeProperties() : _value (STI_COMPOSITE_PROPERTY_NONE) {}

  STICompositeProperties(const STICompositeProperties& properties) :
      _value (properties._value) {}

  ~STICompositeProperties() {}

  void set(Property property)   { _value |= property;  }
  void unset(Property property) { _value &= ~property; }

  STICompositeProperties& operator = (const STICompositeProperties properties) {
    _value = properties._value;
    return *this;
  }

  operator uint16_t () const { return _value; }
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
  STISymbolsSignatureID _signatureID;
  std::string _path;
  CompileUnitList _compileUnits;

protected:
  STISymbolModule();

public:
  static STISymbolModule *create();
  ~STISymbolModule();

  STISymbolsSignatureID getSymbolsSignatureID() const;
  void setSymbolsSignatureID(STISymbolsSignatureID signatureID);
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
  std::string _producer;
  STIScope *_scope;

protected:
  STISymbolCompileUnit();

public:
  static STISymbolCompileUnit *create();

  ~STISymbolCompileUnit();

  STIMachineID getMachineID() const;
  void setMachineID(STIMachineID machineID);
  StringRef getProducer() const;
  void setProducer(StringRef producer);
  STIScope *getScope() const;
};

//===----------------------------------------------------------------------===//
// STISymbolConstant
//
// Represents the following debug information symbols:
//   * S_CONSTANT
//   * S_MANCONSTANT
//
//===----------------------------------------------------------------------===//

class STISymbolConstant : public STISymbol {
public:
  static STISymbolConstant *create();

  ~STISymbolConstant();

  StringRef getName() const;
  void setName(StringRef name);
  const STINumeric *getValue() const;
  void setValue(const STINumeric *value);
  STIType *getType() const;
  void setType(STIType *type);

protected:
  STISymbolConstant();

private:
  std::string           _name;
  STIType*              _type;
  const STINumeric*     _value;
};

//===----------------------------------------------------------------------===//
// STISymbolProcedure
//===----------------------------------------------------------------------===//

class STISymbolProcedure : public STISymbol {
private:
  STISymbolID _symbolID;
  std::string _name;
  STIType *_type;
  STIScope *_scope;
  MCSymbol *_labelBegin;
  MCSymbol *_labelEnd;
  MCSymbol *_labelPrologEnd;
  STILineSlice *_lineSlice;
  unsigned _scopeLineNumber;
  STISymbolFrameProc *_frame;

protected:
  STISymbolProcedure();
  STISymbolProcedure(STIObjectKind kind, STISymbolID _symbolID);

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

  unsigned getScopeLineNumber() const;
  void setScopeLineNumber(unsigned line);

  STISymbolFrameProc *getFrame() const;
  void setFrame(STISymbolFrameProc *frame);

  Function *getFunction() const;
  void setFunction(Function *function);
};

//===----------------------------------------------------------------------===//
// STISymbolThunk
//===----------------------------------------------------------------------===//

class STISymbolThunk : public STISymbolProcedure {
public:
  typedef uint32_t Ordinal;

private:
  Ordinal         _ordinal;
  int             _adjustor;
  std::string     _target;
  MCSymbol       *_pcode;

protected:
  STISymbolThunk();

public:
  static STISymbolThunk *create();

  ~STISymbolThunk();

  Ordinal getOrdinal() const;
  void setOrdinal(Ordinal ordinal);
  int getAdjustor() const;
  void setAdjustor(int adjustor);
  StringRef getTarget() const;
  void setTarget(StringRef name);
  MCSymbol *getPCODE() const;
  void setPCODE(MCSymbol *pcode);
};

//===----------------------------------------------------------------------===//
// STISymbolFrameProc
//===----------------------------------------------------------------------===//

class STISymbolFrameProc : public STISymbol {
private:
  STISymbolProcedure *_procedure;

protected:
  STISymbolFrameProc();

public:
  static STISymbolFrameProc *create();

  ~STISymbolFrameProc();

  STISymbolProcedure *getProcedure() const;
  void setProcedure(STISymbolProcedure *procedure);
};

//===----------------------------------------------------------------------===//
// STISymbolBlock
//===----------------------------------------------------------------------===//

class STISymbolBlock : public STISymbol {
private:
  std::string _name;
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
};

//===----------------------------------------------------------------------===//
// STISymbolVariable
//===----------------------------------------------------------------------===//

class STISymbolVariable : public STISymbol {
private:
  std::string _name;
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

class STIType : public STIObject {
public:
  typedef uint32_t Index;

private:
  Index    _index;
  uint32_t _sizeInBits;

protected:
  STIType(STIObjectKind kind);

public:
  ~STIType();

  STIType::Index getIndex() const;
  void setIndex(STIType::Index index);
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
  bool _isConstant  : 1;
  bool _isVolatile  : 1;
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
public:
  enum PTMType { PTM_NONE, PTM_DATA, PTM_METHOD };

private:
  STIType *_pointerTo;
  STIType *_containingClass;
  PTMType  _ptrToMemberType;
  bool     _isLValueReference;
  bool     _isRValueReference;
  bool     _isConstant;

protected:
  STITypePointer();

public:
  ~STITypePointer();

  static STITypePointer *create();

  STIType *getPointerTo() const;
  void setPointerTo(STIType *pointerTo);

  STIType *getContainingClass() const;
  void setContainingClass(STIType *classType);

  PTMType getPtrToMemberType() const;
  void setPtrToMemberType(PTMType ptrToMemberType);

  bool isLValueReference() const;
  void setIsLValueReference(bool isLValueReference);

  bool isRValueReference() const;
  void setIsRValueReference(bool isRValueReference);

  bool isConstant() const;
  void setIsConstant(bool isConst);
};

//===----------------------------------------------------------------------===//
// STITypeArray
//===----------------------------------------------------------------------===//

class STITypeArray : public STIType {
private:
  STIType*          _elementType;
  std::string       _name;
  const STINumeric* _length;

protected:
  STITypeArray();

public:
  ~STITypeArray();

  static STITypeArray *create();

  STIType *getElementType() const;
  void setElementType(STIType *elementType);

  StringRef getName() const;
  void setName(StringRef name);

  const STINumeric* getLength() const;
  void setLength(const STINumeric* length);
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
// STITypeFieldListItem
//
// Base class for types which are emitted in field lists.
//
//===----------------------------------------------------------------------===//

class STITypeFieldListItem : public STIType {
protected:
  STITypeFieldListItem(STIObjectKind kind);

public:
  ~STITypeFieldListItem();
};

//===----------------------------------------------------------------------===//
// STITypeFieldList
//===----------------------------------------------------------------------===//

class STITypeFieldList : public STIType {
public:
  typedef std::vector<STITypeFieldListItem*> Fields;

private:
  Fields _fields;

protected:
  STITypeFieldList();

public:
  ~STITypeFieldList();

  static STITypeFieldList *create();

  const Fields &getFields() const;

  void append(STITypeFieldListItem *field);
};

//===----------------------------------------------------------------------===//
// STITypeIndex
//===----------------------------------------------------------------------===//

class STITypeIndex : public STITypeFieldListItem {
private:
  STIType *_type;

protected:
  STITypeIndex();

public:
  ~STITypeIndex();

  static STITypeIndex *create();

  STIType *getType() const { return _type; }
  void setType(STIType *type) { _type = type; }
};

//===----------------------------------------------------------------------===//
// STITypeMember
//===----------------------------------------------------------------------===//

class STITypeMember : public STITypeFieldListItem {
private:
  uint16_t _attribute;
  STIType *_type;
  const STINumeric *_offset;
  std::string _name;
  bool _isStatic;

protected:
  STITypeMember();

public:
  ~STITypeMember();

  static STITypeMember *create();

  uint16_t getAttribute() const;
  void setAttribute(uint16_t attr);

  STIType *getType() const;
  void setType(STIType *type);

  const STINumeric* getOffset() const;
  void setOffset(const STINumeric *offset);

  StringRef getName() const;
  void setName(StringRef name);

  bool isStatic() const;
  void setIsStatic(bool isStatic);
};

//===----------------------------------------------------------------------===//
// STITypeVBaseClass
//===----------------------------------------------------------------------===//

class STITypeVBaseClass : public STITypeFieldListItem {
private:
  STISymbolID _symbolID;
  uint16_t _attribute;
  STIType *_type;
  STIType *_vbpType;
  const STINumeric *_vbpOffset;
  const STINumeric *_vbIndex;

protected:
  STITypeVBaseClass(bool indirect);

public:
  ~STITypeVBaseClass();

  static STITypeVBaseClass *create(bool indirect);

  STISymbolID getSymbolID() const;

  uint16_t getAttribute() const;
  void setAttribute(uint16_t attr);

  STIType *getType() const;
  void setType(STIType *type);

  STIType *getVbpType() const;
  void setVbpType(STIType *type);

  const STINumeric* getVbpOffset() const;
  void setVbpOffset(const STINumeric* offset);

  const STINumeric* getVbIndex() const;
  void setVbIndex(const STINumeric* index);
};

//===----------------------------------------------------------------------===//
// STITypeMethod
//===----------------------------------------------------------------------===//

class STITypeMethod : public STITypeFieldListItem {
private:
  int _count;
  STIType *_methodList;
  std::string _name;

protected:
  STITypeMethod();

public:
  ~STITypeMethod();

  static STITypeMethod *create();

  int getCount() const;
  void setCount(int count);

  STIType *getList() const;
  void setList(STIType *methodList);

  StringRef getName() const;
  void setName(StringRef name);
};

//===----------------------------------------------------------------------===//
// STITypeOneMethod
//===----------------------------------------------------------------------===//

class STITypeOneMethod : public STITypeFieldListItem {
private:
  uint16_t _attribute;
  STIType *_type;
  int _virtuality;
  int _virtualIndex;
  std::string _name;

protected:
  STITypeOneMethod();

public:
  ~STITypeOneMethod();

  static STITypeOneMethod *create();

  uint16_t getAttribute() const;
  void setAttribute(uint16_t attr);

  STIType *getType() const;
  void setType(STIType *type);

  int getVirtuality() const;
  void setVirtuality(int virtuality);

  int getVirtualIndex() const;
  void setVirtualIndex(int virtualIndex);

  StringRef getName() const;
  void setName(StringRef name);
};

//===----------------------------------------------------------------------===//
// STITypeEnumerator
//===----------------------------------------------------------------------===//

class STITypeEnumerator : public STITypeFieldListItem {
private:
  uint16_t _attribute;
  const STINumeric *_value;
  std::string _name;

protected:
  STITypeEnumerator();

public:
  ~STITypeEnumerator();

  static STITypeEnumerator *create();

  uint16_t getAttribute() const;
  void setAttribute(uint16_t attr);

  const STINumeric* getValue() const;
  void setValue(const STINumeric *value);

  StringRef getName() const;
  void setName(StringRef name);
};

//===----------------------------------------------------------------------===//
// STITypeBaseClass
//===----------------------------------------------------------------------===//

class STITypeBaseClass : public STITypeFieldListItem {
private:
  uint16_t _attribute;
  STIType *_type;
  const STINumeric *_offset;

protected:
  STITypeBaseClass();

public:
  ~STITypeBaseClass();

  static STITypeBaseClass *create();

  uint16_t getAttribute() const;
  void setAttribute(uint16_t attr);

  STIType *getType() const;
  void setType(STIType *type);

  const STINumeric* getOffset() const;
  void setOffset(const STINumeric *offset);
};

//===----------------------------------------------------------------------===//
// STITypeVFuncTab
//===----------------------------------------------------------------------===//

class STITypeVFuncTab : public STITypeFieldListItem {
private:
  STIType *_type;

protected:
  STITypeVFuncTab();

public:
  ~STITypeVFuncTab();

  static STITypeVFuncTab *create();

  STIType *getType() const;
  void setType(STIType *type);
};

//===----------------------------------------------------------------------===//
// STITypeMethodListEntry
//===----------------------------------------------------------------------===//

class STITypeMethodListEntry {
private:
  uint16_t _attribute;
  STIType *_type;
  int _virtuality;
  int _virtualIndex;

protected:
  STITypeMethodListEntry();

public:
  ~STITypeMethodListEntry();

  static STITypeMethodListEntry *create();

  uint16_t getAttribute() const;
  void setAttribute(uint16_t _attribute);

  STIType *getType() const;
  void setType(STIType *type);

  int getVirtuality() const;
  void setVirtuality(int virtuality);

  int getVirtualIndex() const;
  void setVirtualIndex(int virtualIndex);
};

//===----------------------------------------------------------------------===//
// STITypeMethodList
//===----------------------------------------------------------------------===//

class STITypeMethodList : public STIType {
public:
  typedef std::vector<STITypeMethodListEntry *> STIMethodTypeList;

private:
  STIMethodTypeList _methodList;

protected:
  STITypeMethodList();

public:
  ~STITypeMethodList();

  static STITypeMethodList *create();

  uint32_t getMethodsCount() const;

  const STIMethodTypeList &getList() const;
  STIMethodTypeList &getList();
};

//===----------------------------------------------------------------------===//
// STITypeStructure
//===----------------------------------------------------------------------===//

class STITypeStructure : public STIType {
public:
  typedef STICompositeProperties Properties;

private:
  uint16_t _leaf; // FIXME: STISymbolID?
  uint16_t _count;
  Properties _properties;
  STIType *_fieldType;
  STIType *_derivedType;
  STIType *_vshapeType;
  const STINumeric *_size;
  std::string _name;

protected:
  STITypeStructure();

public:
  ~STITypeStructure();

  static STITypeStructure *create();

  uint16_t getLeaf() const;
  void setLeaf(uint16_t leaf);

  uint16_t getCount() const;
  void setCount(uint16_t count);

  Properties getProperties() const;
  void setProperties(Properties properties);
  void setProperty(STICompositeProperty property);

  STIType *getFieldType() const;
  void setFieldType(STIType *fieldType);

  STIType *getDerivedType() const;
  void setDerivedType(STIType *derivedType);

  STIType *getVShapeType() const;
  void setVShapeType(STIType *vshapeType);

  const STINumeric* getSize() const;
  void setSize(const STINumeric* size);

  StringRef getName() const;
  void setName(StringRef name);
};

//===----------------------------------------------------------------------===//
// STITypeEnumeration
//===----------------------------------------------------------------------===//

class STITypeEnumeration : public STIType {
public:
  typedef STICompositeProperties Properties;

private:
  uint16_t _count;
  Properties _properties;
  STIType *_elementType;
  STIType *_fieldType;
  std::string _name;

protected:
  STITypeEnumeration();

public:
  ~STITypeEnumeration();

  static STITypeEnumeration *create();

  uint16_t getCount() const;
  void setCount(uint16_t count);

  Properties getProperties() const;
  void setProperties(Properties properties);
  void setProperty(STICompositeProperty property);

  STIType *getElementType() const;
  void setElementType(STIType *elementType);

  STIType *getFieldType() const;
  void setFieldType(STIType *fieldType);

  StringRef getName() const;
  void setName(StringRef name);
};

//===----------------------------------------------------------------------===//
// STITypeVShape
//===----------------------------------------------------------------------===//

class STITypeVShape : public STIType {
private:
  uint16_t _count;

protected:
  STITypeVShape();

public:
  ~STITypeVShape();

  static STITypeVShape *create();

  uint16_t getCount() const;
  void setCount(uint16_t count);
};

//===----------------------------------------------------------------------===//
// STITypeFunctionID
//===----------------------------------------------------------------------===//

class STITypeFunctionID : public STIType {
private:
  STIType *_type;
  STIType *_parentScope;
  STIType *_parentClassType;
  std::string _name;

protected:
  STITypeFunctionID();

public:
  ~STITypeFunctionID();

  static STITypeFunctionID *create();

  STIType *getType() const;
  void setType(STIType *type);

  STIType *getParentScope() const;
  void setParentScope(STIType *parentScope);

  STIType *getParentClassType() const;
  void setParentClassType(STIType *parentClassType);

  StringRef getName() const;
  void setName(StringRef name);
};

//===----------------------------------------------------------------------===//
// STITypeProcedure
//===----------------------------------------------------------------------===//

class STITypeProcedure : public STIType {
private:
  STIType *_returnType;
  STIType *_argumentList;
  int _callingConvention;
  uint16_t _paramCount;

protected:
  STITypeProcedure();
  STITypeProcedure(STIObjectKind kind);

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
// STITypeMemberFunction
//===----------------------------------------------------------------------===//

class STITypeMemberFunction : public STITypeProcedure {
private:
  STIType *_classType;
  STIType *_thisType;
  int      _thisAdjust;

protected:
  STITypeMemberFunction();

public:
  ~STITypeMemberFunction();

  static STITypeMemberFunction *create();

  STIType *getClassType() const;
  void setClassType(STIType *classType);

  STIType *getThisType() const;
  void setThisType(STIType *thisType);

  int getThisAdjust() const;
  void setThisAdjust(int thisAdjust);
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

  void append(STIType *argument);
};

//===----------------------------------------------------------------------===//
// STITypeServer
//===----------------------------------------------------------------------===//

class STITypeServer : public STIType {
private:
  std::string _pdbFullName;

protected:
  STITypeServer();

public:
  ~STITypeServer();

  static STITypeServer *create();

  StringRef getPDBFullName() const;
  void setPDBFullName(StringRef name);
};

//===----------------------------------------------------------------------===//
// STISymbolUserDefined
//===----------------------------------------------------------------------===//

class STISymbolUserDefined : public STISymbol {
private:
  STIType *_definedType;
  std::string _name;

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
  typedef std::vector<std::pair<unsigned, STIObject *> > ObjectList;

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

  void add(STIObject *object, unsigned ArgNum = 0);
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
