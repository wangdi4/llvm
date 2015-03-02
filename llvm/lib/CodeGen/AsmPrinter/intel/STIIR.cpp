//===-- STIIR.cpp - Symbol And Type Info -------*- C++ -*--===//
//
//===----------------------------------------------------------------------===//
//
// This file contains data structures for symbol and type information entries.
//
//===----------------------------------------------------------------------===//

#include "STIIR.h"

using namespace llvm;

//===----------------------------------------------------------------------===//
// STIObject
//===----------------------------------------------------------------------===//

uint32_t STIObject::_countUniqueID = 0;

STIObject::STIObject(STIObjectKind kind)
    : _kind(kind), _uniqueID(++_countUniqueID) {}

STIObject::~STIObject() {}

STIObjectKind STIObject::getKind() const { return _kind; }

//===----------------------------------------------------------------------===//
// STILocation
//===----------------------------------------------------------------------===//

STILocation::STILocation(STISymbolID symbolID, STIRegID regnum, int offset,
                         MCSymbol *label)
    : STIObject(STI_OBJECT_KIND_LOCATION), _symbolID(symbolID), _regnum(regnum),
      _offset(offset), _label(label) {}

STILocation::~STILocation() {}

STILocation *STILocation::createRegisterOffset(STIRegID regnum, int offset) {
  return new STILocation(S_REGREL32, regnum, offset, nullptr);
}

STILocation *STILocation::createRegister(STIRegID regnum) {
  return new STILocation(S_REGISTER, regnum, 0, nullptr);
}

STILocation *STILocation::createOffset(int offset) {
  return new STILocation(S_BPREL32, STI_REGISTER_NONE, offset, nullptr);
}

STILocation *STILocation::createGlobalSegmentedOffset(MCSymbol *label) {
  return new STILocation(S_GDATA32, STI_REGISTER_NONE, 0, label);
}

STILocation *STILocation::createLocalSegmentedOffset(MCSymbol *label) {
  return new STILocation(S_LDATA32, STI_REGISTER_NONE, 0, label);
}

STISymbolID STILocation::getSymbolID() const { return _symbolID; }

void STILocation::setSymbolID(STISymbolID symbolID) { _symbolID = symbolID; }

STIRegID STILocation::getReg() const { return _regnum; }

void STILocation::setReg(STIRegID regnum) { _regnum = regnum; }

int STILocation::getOffset() const { return _offset; }

void STILocation::setOffset(int offset) { _offset = offset; }

MCSymbol *STILocation::getLabel() const { return _label; }

void STILocation::setLabel(MCSymbol *label) { _label = label; }

//===----------------------------------------------------------------------===//
// STIStringEntry
//===----------------------------------------------------------------------===//

STIStringEntry::STIStringEntry() : _string(), _offset(0) {}

STIStringEntry::~STIStringEntry() {}

STIStringEntry *STIStringEntry::create() { return new STIStringEntry(); }

StringRef STIStringEntry::getString() const { return _string; }

void STIStringEntry::setString(StringRef string) { _string = string; }

STIStringEntry::Offset STIStringEntry::getOffset() const { return _offset; }

void STIStringEntry::setOffset(Offset offset) { _offset = offset; }

//===----------------------------------------------------------------------===//
// STIStringTable
//===----------------------------------------------------------------------===//

STIStringTable::STIStringTable() : _entries() {
  // The first entry is always the empty string.
  append("");
}

STIStringTable::~STIStringTable() {}

STIStringEntry *STIStringTable::lookup(StringRef string) {
  STIStringEntry *match = nullptr;

  // Inefficient, but the set of strings recorded here is small.
  for (STIStringEntry *entry : getEntries()) {
    if (entry->getString() == string) {
      match = entry;
    }
  }

  return match;
}

STIStringEntry *STIStringTable::append(StringRef string) {
  STIStringEntry *entry;

  entry = STIStringEntry::create();
  entry->setString(string);
  _entries.push_back(entry);

  return entry;
}

STIStringTable::EntryList &STIStringTable::getEntries() { return _entries; }

const STIStringTable::EntryList &STIStringTable::getEntries() const {
  return _entries;
}

STIStringEntry *STIStringTable::find(StringRef string) {
  STIStringEntry *entry;

  entry = lookup(string);
  if (entry == nullptr) {
    entry = append(string);
  }

  return entry;
}

//===----------------------------------------------------------------------===//
// STIChecksumEntry
//===----------------------------------------------------------------------===//

STIChecksumEntry::STIChecksumEntry()
    : _stringEntry(nullptr), _offset(0),
      _type(STI_FILECHECKSUM_ENTRY_TYPE_NONE), _checksum(nullptr) {}

STIChecksumEntry::~STIChecksumEntry() {}

STIChecksumEntry *STIChecksumEntry::create() { return new STIChecksumEntry(); }

StringRef STIChecksumEntry::getFilename() const {
  return _stringEntry->getString();
}

STIStringEntry *STIChecksumEntry::getStringEntry() const {
  return _stringEntry;
}

void STIChecksumEntry::setStringEntry(STIStringEntry *stringEntry) {
  _stringEntry = stringEntry;
}

STIChecksumEntry::Offset STIChecksumEntry::getOffset() const { return _offset; }

void STIChecksumEntry::setOffset(Offset offset) { _offset = offset; }

STIChecksumEntry::Type STIChecksumEntry::getType() const { return _type; }

void STIChecksumEntry::setType(Type type) { _type = type; }

STIChecksumEntry::Checksum STIChecksumEntry::getChecksum() const {
  return _checksum;
}

void STIChecksumEntry::setChecksum(Checksum checksum) { _checksum = checksum; }

size_t STIChecksumEntry::getChecksumSize() const {
  uint32_t size;
  Type type;

  type = getType();
  switch (type) {
#define MAP(TYPE, SIZE)                                                        \
  case TYPE:                                                                   \
    size = SIZE;                                                               \
    break
    MAP(STI_FILECHECKSUM_ENTRY_TYPE_NONE, 0);
    MAP(STI_FILECHECKSUM_ENTRY_TYPE_MD5, 16);
    MAP(STI_FILECHECKSUM_ENTRY_TYPE_SHA1, 20);
#undef MAP
  default:
    assert(type != type); // unrecognized checksum type
    break;
  }

  return size;
}

//===----------------------------------------------------------------------===//
// STIChecksumTable
//===----------------------------------------------------------------------===//

STIChecksumTable::STIChecksumTable() {}

STIChecksumTable::~STIChecksumTable() {}

STIChecksumTable::EntryList &STIChecksumTable::getEntries() { return _entries; }

const STIChecksumTable::EntryList &STIChecksumTable::getEntries() const {
  return _entries;
}

void STIChecksumTable::append(STIChecksumEntry *entry) {
  _entries.push_back(entry);
}

//===----------------------------------------------------------------------===//
// STILineEntry
//
// A line table entry correlating a machine instruction to a source line.
//
//===----------------------------------------------------------------------===//

STILineEntry::STILineEntry() : _label(nullptr), _line() {}

STILineEntry::~STILineEntry() {}

STILineEntry *STILineEntry::create() { return new STILineEntry(); }

const MCSymbol *STILineEntry::getLabel() const { return _label; }

void STILineEntry::setLabel(const MCSymbol *const label) { _label = label; }

uint32_t STILineEntry::getLineNumStart() const { return _line._lineNumStart; }

void STILineEntry::setLineNumStart(const uint32_t lineNumStart) {
  _line._lineNumStart = lineNumStart;
}

uint32_t STILineEntry::getDeltaLineEnd() const { return _line._deltaLineEnd; }

void STILineEntry::setDeltaLineEnd(const uint32_t deltaLineEnd) {
  _line._deltaLineEnd = deltaLineEnd;
}

bool STILineEntry::getStatementEnd() const { return _line._fStatement; }

void STILineEntry::setStatementEnd(const bool statementEnd) {
  _line._fStatement = statementEnd;
}

//===----------------------------------------------------------------------===//
// STILineBlock
//
// A block of line table entries which all have a common source file.
//
//===----------------------------------------------------------------------===//

STILineBlock::STILineBlock() : _checksumEntry(nullptr), _lineEntries() {}

STILineBlock::~STILineBlock() {}

STILineBlock *STILineBlock::create() { return new STILineBlock(); }

StringRef STILineBlock::getFilename() const {
  return _checksumEntry->getFilename();
}

STIChecksumEntry *STILineBlock::getChecksumEntry() const {
  return _checksumEntry;
}

void STILineBlock::setChecksumEntry(STIChecksumEntry *checksumEntry) {
  _checksumEntry = checksumEntry;
}

STILineBlock::LineEntries &STILineBlock::getLines() { return _lineEntries; }

const STILineBlock::LineEntries &STILineBlock::getLines() const {
  return _lineEntries;
}

size_t STILineBlock::getLineCount() const { return _lineEntries.size(); }

void STILineBlock::appendLine(STILineEntry *entry) {
  _lineEntries.push_back(entry);
}

//===----------------------------------------------------------------------===//
// STILineSlice
//
// A segment of the machine instruction to source line correlation specific to
// a single procedure.
//
//===----------------------------------------------------------------------===//

STILineSlice::STILineSlice() : _function(nullptr), _blocks() {}

STILineSlice::~STILineSlice() {}

STILineSlice *STILineSlice::create() { return new STILineSlice(); }

Function *STILineSlice::getFunction() const { return _function; }

void STILineSlice::setFunction(Function *function) { _function = function; }

STILineSlice::BlockList &STILineSlice::getBlocks() { return _blocks; }

const STILineSlice::BlockList &STILineSlice::getBlocks() const {
  return _blocks;
}

void STILineSlice::appendBlock(STILineBlock *block) {
  _blocks.push_back(block);
}

//===----------------------------------------------------------------------===//
// STILineTable
//===----------------------------------------------------------------------===//

STILineTable::STILineTable() : _slices() {}

STILineTable::~STILineTable() {}

STILineTable::SliceList &STILineTable::getSlices() { return _slices; }

const STILineTable::SliceList &STILineTable::getSlices() const {
  return _slices;
}

void STILineTable::appendSlice(STILineSlice *slice) {
  _slices.push_back(slice);
}

//===----------------------------------------------------------------------===//
// STISymbol
//===----------------------------------------------------------------------===//

STISymbol::STISymbol(STIObjectKind kind) : STIObject(kind) {}

STISymbol::~STISymbol() {}

//===----------------------------------------------------------------------===//
// STISymbolModule
//===----------------------------------------------------------------------===//

STISymbolModule *STISymbolModule::create(const Module *module) {
  STISymbolModule *symbol = new STISymbolModule();

  symbol->setSignatureID(STI_SIGNATURE_LATEST);
  // FIXME: set correct path
  symbol->setPath("C:\\Users\\bwyma\\project\\xmain\\helloworld.o");

  return symbol;
}

STISymbolModule::STISymbolModule()
    : STISymbol(STI_OBJECT_KIND_SYMBOL_MODULE), _path(), _compileUnits() {}

STISymbolModule::~STISymbolModule() {}

STISignatureID STISymbolModule::getSignatureID() const { return _signatureID; }

StringRef STISymbolModule::getPath() const { return _path; }

void STISymbolModule::setSignatureID(STISignatureID signatureID) {
  _signatureID = signatureID;
}

void STISymbolModule::setPath(StringRef path) { _path = path; }

void STISymbolModule::add(STISymbolCompileUnit *compileUnit) {
  _compileUnits.push_back(compileUnit);
}

const STISymbolModule::CompileUnitList *
STISymbolModule::getCompileUnits() const {
  return &_compileUnits;
}

//===----------------------------------------------------------------------===//
// STISymbolCompileUnit
//===----------------------------------------------------------------------===//

STISymbolCompileUnit *
STISymbolCompileUnit::create(const DICompileUnit compileUnit) {
  STISymbolCompileUnit *symbol;

  symbol = new STISymbolCompileUnit();

  return symbol;
}

STISymbolCompileUnit::STISymbolCompileUnit()
    : STISymbol(STI_OBJECT_KIND_SYMBOL_COMPILE_UNIT), _machineID(), _producer(),
      _scope(STIScope::create(this)) {}

STISymbolCompileUnit::~STISymbolCompileUnit() {}

STIMachineID STISymbolCompileUnit::getMachineID() const { return _machineID; }

void STISymbolCompileUnit::setMachineID(STIMachineID machineID) {
  _machineID = machineID;
}

StringRef STISymbolCompileUnit::getProducer() const { return _producer; }

void STISymbolCompileUnit::setProducer(StringRef producer) {
  _producer = producer;
}

STIScope *STISymbolCompileUnit::getScope() const { return _scope; }

//===----------------------------------------------------------------------===//
// STISymbolProcedure
//===----------------------------------------------------------------------===//

STISymbolProcedure::STISymbolProcedure()
    : STISymbol(STI_OBJECT_KIND_SYMBOL_PROCEDURE), _symbolID(S_GPROC32_ID),
      _name(), _type(nullptr), _scope(STIScope::create(this)),
      _labelBegin(nullptr), _labelEnd(nullptr), _labelPrologEnd(nullptr),
      _lineSlice(STILineSlice::create()) {}

STISymbolProcedure::~STISymbolProcedure() {}

STISymbolProcedure *STISymbolProcedure::create() {
  return new STISymbolProcedure();
}

STISymbolID STISymbolProcedure::getSymbolID() const { return _symbolID; }

void STISymbolProcedure::setSymbolID(STISymbolID symbolID) {
  _symbolID = symbolID;
}

StringRef STISymbolProcedure::getName() const { return _name; }

void STISymbolProcedure::setName(StringRef name) { _name = name; }

STIType *STISymbolProcedure::getType() const { return _type; }

void STISymbolProcedure::setType(STIType *type) { _type = type; }

STIScope *STISymbolProcedure::getScope() const { return _scope; }

void STISymbolProcedure::setScope(STIScope *scope) { _scope = scope; }

MCSymbol *STISymbolProcedure::getLabelBegin() const { return _labelBegin; }

void STISymbolProcedure::setLabelBegin(MCSymbol *labelBegin) {
  _labelBegin = labelBegin;
}

MCSymbol *STISymbolProcedure::getLabelEnd() const { return _labelEnd; }

void STISymbolProcedure::setLabelEnd(MCSymbol *labelEnd) {
  _labelEnd = labelEnd;
}

MCSymbol *STISymbolProcedure::getLabelPrologEnd() const {
  return _labelPrologEnd;
}

void STISymbolProcedure::setLabelPrologEnd(MCSymbol *labelPrologEnd) {
  _labelPrologEnd = labelPrologEnd;
}

void STISymbolProcedure::add(STIObject *object) { getScope()->add(object); }

STILineSlice *STISymbolProcedure::getLineSlice() { return _lineSlice; }

const STILineSlice *STISymbolProcedure::getLineSlice() const {
  return _lineSlice;
}

//===----------------------------------------------------------------------===//
// STISymbolBlock
//===----------------------------------------------------------------------===//

STISymbolBlock::STISymbolBlock()
    : STISymbol(STI_OBJECT_KIND_SYMBOL_BLOCK), _name(),
      _scope(STIScope::create(this)), _labelBegin(nullptr), _labelEnd(nullptr),
      _procedure(nullptr) {}

STISymbolBlock::~STISymbolBlock() {}

STISymbolBlock *STISymbolBlock::create() { return new STISymbolBlock(); }

StringRef STISymbolBlock::getName() const { return _name; }

void STISymbolBlock::setName(StringRef name) { _name = name; }

STIScope *STISymbolBlock::getScope() const { return _scope; }

void STISymbolBlock::setScope(STIScope *scope) { _scope = scope; }

MCSymbol *STISymbolBlock::getLabelBegin() const { return _labelBegin; }

void STISymbolBlock::setLabelBegin(MCSymbol *labelBegin) {
  _labelBegin = labelBegin;
}

MCSymbol *STISymbolBlock::getLabelEnd() const { return _labelEnd; }

void STISymbolBlock::setLabelEnd(MCSymbol *labelEnd) { _labelEnd = labelEnd; }

STISymbolProcedure *STISymbolBlock::getProcedure() const { return _procedure; }

void STISymbolBlock::setProcedure(STISymbolProcedure *procedure) {
  _procedure = procedure;
}

void STISymbolBlock::add(STIObject *object) { getScope()->add(object); }

//===----------------------------------------------------------------------===//
// STISymbolVariable
//===----------------------------------------------------------------------===//

STISymbolVariable *STISymbolVariable::create() {
  return new STISymbolVariable();
}

STISymbolVariable::STISymbolVariable()
    : STISymbol(STI_OBJECT_KIND_SYMBOL_VARIABLE) {}

STISymbolVariable::~STISymbolVariable() {}

StringRef STISymbolVariable::getName() const { return _name; }

void STISymbolVariable::setName(StringRef name) { _name = name; }

STILocation *STISymbolVariable::getLocation() const { return _location; }

void STISymbolVariable::setLocation(STILocation *location) {
  _location = location;
}

STIType *STISymbolVariable::getType() const { return _type; }

void STISymbolVariable::setType(STIType *type) { _type = type; }

//===----------------------------------------------------------------------===//
// STIType
//===----------------------------------------------------------------------===//

STIType::STIType(STIObjectKind kind)
    : STIObject(kind), _index(T_NOTYPE), _sizeInBits(0) {}

STIType::~STIType() {}

STITypeIndex STIType::getIndex() const { return _index; }

void STIType::setIndex(STITypeIndex index) { _index = index; }

uint32_t STIType::getSizeInBits() const { return _sizeInBits; }

void STIType::setSizeInBits(uint32_t sizeInBits) { _sizeInBits = sizeInBits; }

//===----------------------------------------------------------------------===//
// STITypeBasic
//===----------------------------------------------------------------------===//

STITypeBasic::STITypeBasic()
    : STIType(STI_OBJECT_KIND_TYPE_BASIC), _primitive(T_NOTYPE) {}

STITypeBasic::~STITypeBasic() {}

STITypeBasic *STITypeBasic::create() { return new STITypeBasic(); }

STITypeBasic::Primitive STITypeBasic::getPrimitive() const {
  return _primitive;
}

void STITypeBasic::setPrimitive(Primitive primitive) { _primitive = primitive; }

//===----------------------------------------------------------------------===//
// STITypeModifier
//===----------------------------------------------------------------------===//

STITypeModifier::STITypeModifier()
    : STIType(STI_OBJECT_KIND_TYPE_MODIFIER), _qualifiedType(nullptr),
      _isConstant(false), _isVolatile(false), _isUnaligned(false) {}

STITypeModifier::~STITypeModifier() {}

STITypeModifier *STITypeModifier::create() { return new STITypeModifier(); }

STIType *STITypeModifier::getQualifiedType() const { return _qualifiedType; }

void STITypeModifier::setQualifiedType(STIType *qualifiedType) {
  _qualifiedType = qualifiedType;
}

bool STITypeModifier::isConstant() const { return _isConstant; }

void STITypeModifier::setIsConstant(bool isConstant) {
  _isConstant = isConstant;
}

bool STITypeModifier::isVolatile() const { return _isVolatile; }

void STITypeModifier::setIsVolatile(bool isVolatile) {
  _isVolatile = isVolatile;
}

bool STITypeModifier::isUnaligned() const { return _isUnaligned; }

void STITypeModifier::setIsUnaligned(bool isUnaligned) {
  _isUnaligned = isUnaligned;
}

//===----------------------------------------------------------------------===//
// STITypePointer
//===----------------------------------------------------------------------===//

STITypePointer::STITypePointer()
    : STIType(STI_OBJECT_KIND_TYPE_POINTER), _pointerTo(nullptr) {}

STITypePointer::~STITypePointer() {}

STITypePointer *STITypePointer::create() { return new STITypePointer(); }

STIType *STITypePointer::getPointerTo() const { return _pointerTo; }

void STITypePointer::setPointerTo(STIType *pointerTo) {
  _pointerTo = pointerTo;
}

//===----------------------------------------------------------------------===//
// STITypeArray
//===----------------------------------------------------------------------===//

STITypeArray::STITypeArray()
    : STIType(STI_OBJECT_KIND_TYPE_ARRAY), _elementType(nullptr), _length(0) {}

STITypeArray::~STITypeArray() {}

STITypeArray *STITypeArray::create() { return new STITypeArray(); }

STIType *STITypeArray::getElementType() const { return _elementType; }

void STITypeArray::setElementType(STIType *elementType) {
  _elementType = elementType;
}

StringRef STITypeArray::getName() const { return _name; }

void STITypeArray::setName(StringRef name) { _name = name; }

uint32_t STITypeArray::getLength() const { return _length; }
void STITypeArray::setLength(uint32_t length) { _length = length; }

//===----------------------------------------------------------------------===//
// STITypeBitfield
//===----------------------------------------------------------------------===//

STITypeBitfield::STITypeBitfield()
    : STIType(STI_OBJECT_KIND_TYPE_BITFIELD), _type(nullptr), _offset(~0),
      _size(0) {}

STITypeBitfield::~STITypeBitfield() {}

STITypeBitfield *STITypeBitfield::create() { return new STITypeBitfield(); }

STIType *STITypeBitfield::getType() const { return _type; }

void STITypeBitfield::setType(STIType *type) { _type = type; }

uint32_t STITypeBitfield::getOffset() const { return _offset; }

void STITypeBitfield::setOffset(uint32_t offset) { _offset = offset; }

uint32_t STITypeBitfield::getSize() const { return _size; }

void STITypeBitfield::setSize(uint32_t size) { _size = size; }

//===----------------------------------------------------------------------===//
// STITypeMember
//===----------------------------------------------------------------------===//

STITypeMember::STITypeMember() : _attribute(0), _type(nullptr), _offset(~0) {}

STITypeMember::~STITypeMember() {}

STITypeMember *STITypeMember::create() { return new STITypeMember(); }

uint16_t STITypeMember::getAttribute() const { return _attribute; }

void STITypeMember::setAttribute(uint16_t attr) { _attribute = attr; }

STIType *STITypeMember::getType() const { return _type; }

void STITypeMember::setType(STIType *type) { _type = type; }

uint32_t STITypeMember::getOffset() const { return _offset; }

void STITypeMember::setOffset(uint32_t offset) { _offset = offset; }

StringRef STITypeMember::getName() const { return _name; }

void STITypeMember::setName(StringRef name) { _name = name; }

//===----------------------------------------------------------------------===//
// STITypeEnumerator
//===----------------------------------------------------------------------===//

STITypeEnumerator::STITypeEnumerator() : _attribute(0), _value(~0) {}

STITypeEnumerator::~STITypeEnumerator() {}

STITypeEnumerator *STITypeEnumerator::create() {
  return new STITypeEnumerator();
}

uint16_t STITypeEnumerator::getAttribute() const { return _attribute; }

void STITypeEnumerator::setAttribute(uint16_t attr) { _attribute = attr; }

uint32_t STITypeEnumerator::getValue() const { return _value; }

void STITypeEnumerator::setValue(uint32_t value) { _value = value; }

StringRef STITypeEnumerator::getName() const { return _name; }

void STITypeEnumerator::setName(StringRef name) { _name = name; }

//===----------------------------------------------------------------------===//
// STITypeFieldList
//===----------------------------------------------------------------------===//

STITypeFieldList::STITypeFieldList()
    : STIType(STI_OBJECT_KIND_TYPE_FIELD_LIST) {}

STITypeFieldList::~STITypeFieldList() {
  for (STITypeMember *member : getMembers()) {
    delete member;
  }
}

STITypeFieldList *STITypeFieldList::create() { return new STITypeFieldList(); }

STITypeFieldList::STITypeMemberList &STITypeFieldList::getMembers() {
  return _members;
}

const STITypeFieldList::STITypeMemberList &
STITypeFieldList::getMembers() const {
  return _members;
}

STITypeFieldList::STITypeEnumeratorList &STITypeFieldList::getEnumerators() {
  return _enumerators;
}

const STITypeFieldList::STITypeEnumeratorList &
STITypeFieldList::getEnumerators() const {
  return _enumerators;
}

//===----------------------------------------------------------------------===//
// STITypeStructure
//===----------------------------------------------------------------------===//

STITypeStructure::STITypeStructure()
    : STIType(STI_OBJECT_KIND_TYPE_STRUCTURE), _leaf(0), _count(0),
      _property(0), _fieldType(nullptr), _derivedType(nullptr),
      _vshapeType(nullptr), _size(0) {}

STITypeStructure::~STITypeStructure() {}

STITypeStructure *STITypeStructure::create() { return new STITypeStructure(); }

uint16_t STITypeStructure::getLeaf() const { return _leaf; }

void STITypeStructure::setLeaf(uint16_t leaf) { _leaf = leaf; }

uint16_t STITypeStructure::getCount() const { return _count; }

void STITypeStructure::setCount(uint16_t count) { _count = count; }

uint16_t STITypeStructure::getProperty() const { return _property; }

void STITypeStructure::setProperty(uint16_t prop) { _property = prop; }

STIType *STITypeStructure::getFieldType() const { return _fieldType; }

void STITypeStructure::setFieldType(STIType *fieldType) {
  _fieldType = fieldType;
}

STIType *STITypeStructure::getDerivedType() const { return _derivedType; }

void STITypeStructure::setDerivedType(STIType *derivedType) {
  _derivedType = derivedType;
}

STIType *STITypeStructure::getVShapeType() const { return _vshapeType; }

void STITypeStructure::setVShapeType(STIType *vshapeType) {
  _vshapeType = vshapeType;
}

uint32_t STITypeStructure::getSize() const { return _size; }

void STITypeStructure::setSize(uint32_t size) { _size = size; }

StringRef STITypeStructure::getName() const { return _name; }

void STITypeStructure::setName(StringRef name) { _name = name; }

//===----------------------------------------------------------------------===//
// STITypeEnumeration
//===----------------------------------------------------------------------===//

STITypeEnumeration::STITypeEnumeration()
    : STIType(STI_OBJECT_KIND_TYPE_ENUMERATION), _count(0), _property(0),
      _elementType(nullptr), _fieldType(nullptr) {}

STITypeEnumeration::~STITypeEnumeration() {}

STITypeEnumeration *STITypeEnumeration::create() {
  return new STITypeEnumeration();
}

uint16_t STITypeEnumeration::getCount() const { return _count; }

void STITypeEnumeration::setCount(uint16_t count) { _count = count; }

uint16_t STITypeEnumeration::getProperty() const { return _property; }

void STITypeEnumeration::setProperty(uint16_t prop) { _property = prop; }

STIType *STITypeEnumeration::getElementType() const { return _elementType; }

void STITypeEnumeration::setElementType(STIType *elementType) {
  _elementType = elementType;
}

STIType *STITypeEnumeration::getFieldType() const { return _fieldType; }

void STITypeEnumeration::setFieldType(STIType *fieldType) {
  _fieldType = fieldType;
}

StringRef STITypeEnumeration::getName() const { return _name; }

void STITypeEnumeration::setName(StringRef name) { _name = name; }

//===----------------------------------------------------------------------===//
// STITypeFunctionID
//===----------------------------------------------------------------------===//

STITypeFunctionID::STITypeFunctionID()
    : STIType(STI_OBJECT_KIND_TYPE_FUNCTION_ID), _type(nullptr),
      _parentScope(nullptr) {}

STITypeFunctionID::~STITypeFunctionID() {}

STITypeFunctionID *STITypeFunctionID::create() {
  return new STITypeFunctionID();
}

STIType *STITypeFunctionID::getType() const { return _type; }

void STITypeFunctionID::setType(STIType *type) { _type = type; }

STIType *STITypeFunctionID::getParentScope() const { return _parentScope; }

void STITypeFunctionID::setParentScope(STIType *parentScope) {
  _parentScope = parentScope;
}

StringRef STITypeFunctionID::getName() const { return _name; }

void STITypeFunctionID::setName(StringRef name) { _name = name; }

//===----------------------------------------------------------------------===//
// STITypeProcedure
//===----------------------------------------------------------------------===//

STITypeProcedure::STITypeProcedure()
    : STIType(STI_OBJECT_KIND_TYPE_PROCEDURE), _returnType(nullptr),
      _callingConvention(0), _paramCount(0), _argumentList(nullptr) {}

STITypeProcedure::~STITypeProcedure() {}

STITypeProcedure *STITypeProcedure::create() { return new STITypeProcedure(); }

STIType *STITypeProcedure::getReturnType() const { return _returnType; }

void STITypeProcedure::setReturnType(STIType *returnType) {
  _returnType = returnType;
}

int STITypeProcedure::getCallingConvention() const {
  return _callingConvention;
}

void STITypeProcedure::setCallingConvention(int callingConvention) {
  _callingConvention = callingConvention;
}

uint16_t STITypeProcedure::getParamCount() const { return _paramCount; }

void STITypeProcedure::setParamCount(uint16_t paramCount) {
  _paramCount = paramCount;
}

STIType *STITypeProcedure::getArgumentList() const { return _argumentList; }

void STITypeProcedure::setArgumentList(STIType *argumentList) {
  _argumentList = argumentList;
}

//===----------------------------------------------------------------------===//
// STITypeArgumentList
//===----------------------------------------------------------------------===//

STITypeArgumentList::STITypeArgumentList()
    : STIType(STI_OBJECT_KIND_TYPE_ARGUMENT_LIST), _argumentList() {}

STITypeArgumentList::~STITypeArgumentList() {}

STITypeArgumentList *STITypeArgumentList::create() {
  return new STITypeArgumentList();
}

uint32_t STITypeArgumentList::getArgumentCount() const {
  return _argumentList.size();
}

const STITypeArgumentList::STIArgTypeList *
STITypeArgumentList::getArgumentList() const {
  return &_argumentList;
}

STITypeArgumentList::STIArgTypeList *STITypeArgumentList::getArgumentList() {
  return &_argumentList;
}

//===----------------------------------------------------------------------===//
// STISymbolUserDefined
//===----------------------------------------------------------------------===//

STISymbolUserDefined::STISymbolUserDefined()
    : STISymbol(STI_OBJECT_KIND_SYMBOL_USER_DEFINED), _definedType(nullptr) {}

STISymbolUserDefined::~STISymbolUserDefined() {}

STISymbolUserDefined *STISymbolUserDefined::create() {
  return new STISymbolUserDefined();
}

STIType *STISymbolUserDefined::getDefinedType() const { return _definedType; }

void STISymbolUserDefined::setDefinedType(STIType *definedType) {
  _definedType = definedType;
}

StringRef STISymbolUserDefined::getName() const { return _name; }

void STISymbolUserDefined::setName(StringRef name) { _name = name; }

//===----------------------------------------------------------------------===//
// STIScope
//===----------------------------------------------------------------------===//

STIScope::STIScope(STISymbol *symbol)
    : STIObject(STI_OBJECT_KIND_SCOPE), _parent(nullptr), _symbol(symbol),
      _objects() {}

STIScope::~STIScope() {}

STIScope *STIScope::create(STISymbol *symbol) { return new STIScope(symbol); }

STIScope *STIScope::getParent() const { return _parent; }

void STIScope::setParent(STIScope *parent) { _parent = parent; }

STISymbol *STIScope::getSymbol() const { return _symbol; }

void STIScope::setSymbol(STISymbol *symbol) { _symbol = symbol; }

void STIScope::add(STIObject *object) { _objects.push_back(object); }

const STIScope::ObjectList &STIScope::getObjects() const { return _objects; }

//===----------------------------------------------------------------------===//
// STISubsection
//===----------------------------------------------------------------------===//

STISubsection::STISubsection(STISubsectionID id)
    : _id(id), _begin(nullptr), _end(nullptr) {}

STISubsection::~STISubsection() {}

STISubsectionID STISubsection::getID() const { return _id; }

MCSymbol *STISubsection::getBegin() const { return _begin; }

void STISubsection::setBegin(MCSymbol *begin) { _begin = begin; }

MCSymbol *STISubsection::getEnd() const { return _end; }

void STISubsection::setEnd(MCSymbol *end) { _end = end; }

//===----------------------------------------------------------------------===//
// STISymbolTable
//===----------------------------------------------------------------------===//

STISymbolTable::STISymbolTable() : _root(nullptr) {}

STISymbolTable::~STISymbolTable() {}

STISymbol *STISymbolTable::getRoot() const { return _root; }

void STISymbolTable::setRoot(STISymbol *root) { _root = root; }
