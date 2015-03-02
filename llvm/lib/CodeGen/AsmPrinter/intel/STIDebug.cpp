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
#include "../DbgValueHistoryCalculator.h"
#include "llvm/ADT/PointerUnion.h" // dyn_cast
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/LexicalScopes.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetSubtargetInfo.h"

using namespace llvm;

//===----------------------------------------------------------------------===//
// Helper Routines
//===----------------------------------------------------------------------===//

static int16_t getNumericSize(const int32_t num) {
  if (num < LF_NUMERIC)
    return 2;
  if (num < (LF_NUMERIC << 1))
    return 4;
  return 6;
}

static int16_t getPaddedSize(const int16_t num) {
  static const int16_t padding = 4;
  static const int16_t paddingInc = padding - 1;
  static const int16_t paddingMask = ~paddingInc;
  return (num + paddingInc) & paddingMask;
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
    X(S_OBJNAME, 0x0000) // FIXME: define these in STI.h
    X(S_COMPILE3, 0x0000)
    X(S_GPROC32_ID, 0x0000)
    X(S_LPROC32_ID, 0x0000)
    X(S_BLOCK32, 0x0000)
    X(S_REGREL32, 0x0000)
    X(S_REGISTER, 0x0000)
    X(S_BPREL32, 0x0000)
    X(S_LDATA32, 0x0000)
    X(S_GDATA32, 0x0000)
    X(S_PROC_ID_END, 0x0000)
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
    MAP(x86, STI_MACHINE_INTEL_PENTIUM_III);
    MAP(x86_64, STI_MACHINE_INTEL64);
#undef MAP
  default:
    assert(!"Architecture cannot be mapped to an STI machine type!");
    break;
  }

  return machineID;
}


//===----------------------------------------------------------------------===//
// STITypeInfo
//===----------------------------------------------------------------------===//

typedef std::vector<STIType *> STITypeTable;

//===----------------------------------------------------------------------===//
// STITypeMap
//===----------------------------------------------------------------------===//
typedef DenseMap<const MDNode *, STIType *> TypeMap;

typedef DenseMap<const MachineInstr *, MCSymbol *> LabelMap;

//===----------------------------------------------------------------------===//
// STIDebugImpl
//===----------------------------------------------------------------------===//

class STIDebugImpl : public STIDebug {
private:
  typedef DenseMap<const Function *, STISymbolProcedure *> FunctionMap;
  typedef DenseMap<const MDNode *, STIScope *> STIScopeMap;

  AsmPrinter *_asmPrinter;
  STISymbolProcedure *_currentProcedure;
  DbgValueHistoryMap _valueHistory;
  FunctionMap _functionMap;
  STISymbolTable _symbolTable;
  STITypeTable _typeTable;
  STIStringTable _stringTable;
  STIChecksumTable _checksumTable;
  STILineTable _lineTable;
  STIScopeMap _ScopeMap;
  TypeMap _typeMap;
  STIType *_voidType;
  unsigned int _blockNumber;
  LexicalScopes _lexicalScopes;
  LabelMap _labelsBeforeInsn;
  LabelMap _labelsAfterInsn;
  const MachineInstr *_curMI;

  // Maps from a type identifier to the actual MDNode.
  DITypeIdentifierMap TypeIdentifierMap;

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
  MachineModuleInfo *MMI();
  const Module *getModule();
  const TargetRegisterInfo *getTargetRegisterInfo();
  STISymbolTable *getSymbolTable();
  const STISymbolTable *getSymbolTable() const;
  STITypeTable *getTypeTable();
  const STITypeTable *getTypeTable() const;
  STIStringTable *getStringTable();
  const STIStringTable *getStringTable() const;
  STIChecksumTable *getChecksumTable();
  const STIChecksumTable *getChecksumTable() const;
  bool hasScope(const MDNode *llvmNode) const;
  STIScope *getScope(const MDNode *llvmNode);
  void addScope(const MDNode *llvmNode, STIScope *object);
  TypeMap *getTypeMap();
  const TypeMap *getTypeMap() const;

  STISymbolCompileUnit *getCompileUnit() { // FIXME:
    STISymbolModule *module =
        static_cast<STISymbolModule *>(getSymbolTable()->getRoot());
    STISymbolCompileUnit *compileUnit = module->getCompileUnits()->back();
    return compileUnit;
  }

  /// \brief Return the TypeIdentifierMap.
  const DITypeIdentifierMap &getTypeIdentifierMap() const;

  STIScope *getOrCreateScope(const DIScope llvmScope);

  STIRegID toSTIRegID(unsigned int regnum) const;
  STISymbolVariable *createSymbolVariable(const DIVariable DIV,
                                          unsigned int frameIndex,
                                          const MachineInstr *DVInsn = nullptr);

  STISymbolProcedure *getCurrentProcedure() const;
  void setCurrentProcedure(STISymbolProcedure *procedure);

  void setSymbolModule(STISymbolModule *module);
  STISymbolModule *getSymbolModule() const;

  size_t getPaddingSize(const STIChecksumEntry *entry) const;

  void clearValueHistory();

  void collectModuleInfo();
  void collectRoutineInfo();

  STISymbolProcedure *getOrCreateSymbolProcedure(const DISubprogram &SP);
  STISymbolBlock *createSymbolBlock(const DILexicalBlock &LB);

  STIType *createType(const DIType llvmType);
  STIType *createTypeBasic(const DIBasicType llvmType);
  STIType *createTypePointer(const DIDerivedType llvmType);
  STIType *createTypeModifier(const DIDerivedType llvmType);
  STIType *createTypeArray(const DICompositeType llvmType);
  STIType *createTypeStructure(const DICompositeType llvmType,
                               bool isDcl = false);
  STIType *createTypeEnumeration(const DICompositeType llvmType);
  STIType *createTypeSubroutine(const DISubroutineType llvmType);

  uint64_t getBaseTypeSize(DIDerivedType Ty) const;

  STIType *getVoidType() const;

  STIType *createSymbolUserDefined(const DIDerivedType llvmType);

  void layout();
  void emit();

  // Used with _typeIdentifierMap for type resolution, not clear why?
  template <typename T> T resolve(DIRef<T> ref) const;

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
  void emitLabelDiff(const MCSymbol *begin, const MCSymbol *end) const;
  void emitSymbolID(const STISymbolID symbolID) const;
  void emitBytes(const char *data, size_t size) const;
  void emitFill(size_t size, const uint8_t byte) const;
  void emitSecRel32(MCSymbol *symbol) const;
  void emitSectionIndex(MCSymbol *symbol) const;

  // Routines for emitting sections.
  void emitSectionBegin(const MCSection *section) const;

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
  void emitSymbolProcedure(const STISymbolProcedure *procedure) const;
  void emitSymbolProcedureEnd() const;
  void emitSymbolBlock(const STISymbolBlock *block) const;
  void emitSymbolScopeEnd() const;
  void emitSymbolVariable(const STISymbolVariable *variable) const;
  void emitSymbolUserDefined(const STISymbolUserDefined *type) const;

  // Routines for emiting the .debug$T section.
  void emitTypes() const;
  void emitType(const STIType *type) const;
  void emitTypeBasic(const STITypeBasic *type) const;
  void emitTypeModifier(const STITypeModifier *type) const;
  void emitTypePointer(const STITypePointer *type) const;
  void emitTypeArray(const STITypeArray *type) const;
  void emitTypeStructure(const STITypeStructure *type) const;
  void emitTypeEnumeration(const STITypeEnumeration *type) const;
  void emitTypeBitfield(const STITypeBitfield *type) const;
  void emitTypeFieldList(const STITypeFieldList *type) const;
  void emitTypeFunctionID(const STITypeFunctionID *type) const;
  void emitTypeProcedure(const STITypeProcedure *type) const;
  void emitTypeArgumentList(const STITypeArgumentList *type) const;

  void emitNumeric(const int32_t num) const;
  // Routines for creating labels.
  MCSymbol *createFuncLabel(const char *name) const;
  MCSymbol *createBlockLabel(const char *name);
};

//===----------------------------------------------------------------------===//
// STIDebugImpl Public Routines
//===----------------------------------------------------------------------===//

STIDebugImpl::STIDebugImpl(AsmPrinter *asmPrinter)
    : STIDebug(), _asmPrinter(asmPrinter), _currentProcedure(nullptr),
      _valueHistory(), _functionMap(), _symbolTable(), _typeTable(),
      _stringTable(), _checksumTable(), _lineTable(), _ScopeMap(), _typeMap(),
      _voidType(nullptr), _blockNumber(0), _lexicalScopes(),
      _labelsBeforeInsn(), _labelsAfterInsn(), _curMI(nullptr) {
  // If module doesn't have named metadata anchors or COFF debug section
  // is not available, skip any debug info related stuff.
  if (!MMI()->getModule()->getNamedMetadata("llvm.dbg.cu") ||
      !ASM()->getObjFileLowering().getCOFFDebugSymbolsSection())
    return;

  beginModule();
}

STIDebugImpl::~STIDebugImpl() {}

void STIDebugImpl::setSymbolSize(const MCSymbol *Symbol, uint64_t size) {}

void STIDebugImpl::beginModule() {
  // Collect all of the initial module information.
  collectModuleInfo();

  // Tell MMI to make the debug information available.
  MMI()->setDebugInfoAvailability(true);
}

void STIDebugImpl::endModule() {
  if (!MMI()->hasDebugInfo())
    return;

  layout();
  emit();
}

void STIDebugImpl::beginFunction(const MachineFunction *MF) {
  if (!MMI()->hasDebugInfo())
    return;

  STISymbolProcedure *procedure;

  _lexicalScopes.initialize(*MF);

  // if (_lexicalScopes.empty())
  //  return;

  // Locate the symbol for this function.
  procedure = _functionMap.find(MF->getFunction())
                  ->second; // FIXME: validate function exist in the map
  procedure->setLabelBegin(createFuncLabel("fbeg"));
  procedure->setLabelEnd(createFuncLabel("fend"));

  // Emit the label marking the beginning of the procedure.
  emitLabel(procedure->getLabelBegin());

  // Record this as the current procedure.
  setCurrentProcedure(procedure);

  calculateDbgValueHistory(MF, getTargetRegisterInfo(), _valueHistory);
}

void STIDebugImpl::endFunction(const MachineFunction *MF) {
  if (!MMI()->hasDebugInfo())
    return;

  STISymbolProcedure *procedure = getCurrentProcedure();

  // Emit the label marking the end of the procedure.
  emitLabel(procedure->getLabelEnd());

  // Collect information about this routine.
  collectRoutineInfo();

  clearValueHistory();
}

void STIDebugImpl::clearValueHistory() { _valueHistory.clear(); }

void STIDebugImpl::beginInstruction(const MachineInstr *MI) {
  DebugLoc location = MI->getDebugLoc();
  STISymbolProcedure *procedure;
  MDNode *node;
  MCSymbol *label;
  std::string path;
  uint32_t line;
  STIStringEntry *string;
  STIChecksumEntry *checksum;
  STILineSlice *slice;
  STILineBlock *block;
  STILineEntry *entry;

  assert(_curMI == nullptr);
  _curMI = MI;

  if (location == DebugLoc()) {
    label = ASM()->MMI->getContext().CreateTempSymbol();
    emitLabel(label);

    // Handle Scope
    _labelsBeforeInsn.insert(std::make_pair(_curMI, label));
    return;
  }

  procedure = getCurrentProcedure();
  slice = procedure->getLineSlice();

  node = location.getScope(ASM()->MF->getFunction()->getContext());
  DIScope scope(node);
  path = (scope.getDirectory() + Twine("\\") + scope.getFilename()).str();
  line = location.getLine();
  std::replace(path.begin(), path.end(), '/', '\\');
  size_t index = 0;
  while ((index = path.find("\\\\", index)) != std::string::npos) {
    path.erase(index, 1);
  }

  label = ASM()->MMI->getContext().CreateTempSymbol();
  emitLabel(label);

  if (slice->getBlocks().size() == 0 ||
      slice->getBlocks().back()->getFilename() != path) {
    string = _stringTable.find(strdup(path.c_str())); // FIXME

    checksum = STIChecksumEntry::create();
    checksum->setStringEntry(string);
    checksum->setType(STIChecksumEntry::STI_FILECHECKSUM_ENTRY_TYPE_NONE);
    checksum->setChecksum(nullptr);
    _checksumTable.append(checksum);

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
      entry->setLineNumStart(line);
      block->appendLine(entry);
    }

    slice->appendBlock(block);
  }

  block = slice->getBlocks().back();

  entry = block->getLines().back();
  if (entry != nullptr && entry->getLineNumStart() != line) {
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

  label = ASM()->MMI->getContext().CreateTempSymbol();
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

MachineModuleInfo *STIDebugImpl::MMI() { return ASM()->MMI; }

const Module *STIDebugImpl::getModule() { return MMI()->getModule(); }

const TargetRegisterInfo *STIDebugImpl::getTargetRegisterInfo() {
  return ASM()->TM.getSubtargetImpl()->getRegisterInfo();
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

bool STIDebugImpl::hasScope(const MDNode *llvmNode) const {
  return _ScopeMap.count(llvmNode);
}

STIScope *STIDebugImpl::getScope(const MDNode *llvmNode) {
  assert(hasScope(llvmNode) && "LLVM node has no STI object mapped yet!");
  return _ScopeMap[llvmNode];
}

void STIDebugImpl::addScope(const MDNode *llvmNode, STIScope *object) {
  assert(!hasScope(llvmNode) && "LLVM node already mapped to STI object!");
  _ScopeMap[llvmNode] = object;
}

TypeMap *STIDebugImpl::getTypeMap() { return &_typeMap; }

const TypeMap *STIDebugImpl::getTypeMap() const { return &_typeMap; }

const DITypeIdentifierMap &STIDebugImpl::getTypeIdentifierMap() const {
  return TypeIdentifierMap;
}

STIRegID STIDebugImpl::toSTIRegID(unsigned int llvmID) const {
  STIRegID stiID;

  switch (llvmID) {
#define MAP(LLVMID, STIID)                                                     \
  case LLVMID:                                                                 \
    stiID = STIID;                                                             \
    break
    // FIXME: register mapping correct?
    MAP(0x13, STI_REGISTER_EAX);
    MAP(0x14, STI_REGISTER_EBP);
    MAP(0x15, STI_REGISTER_EBX);
    MAP(0x16, STI_REGISTER_ECX);
    MAP(0x17, STI_REGISTER_EDI);
    MAP(0x18, STI_REGISTER_EDX);
    // MAP(0x1a,   STI_REGISTER_EIP);
    // MAP(0x1b,   STI_REGISTER_EIZ);
    MAP(0x1d, STI_REGISTER_ESI);
    MAP(0x1e, STI_REGISTER_ESP);

    MAP(0x23, STI_REGISTER_RAX);
    MAP(0x24, STI_REGISTER_RBP);
    MAP(0x25, STI_REGISTER_RBX);
    MAP(0x26, STI_REGISTER_RCX);
    MAP(0x27, STI_REGISTER_RDI);
    MAP(0x28, STI_REGISTER_RDX);
    // MAP(0x29,   STI_REGISTER_RIP);
    // MAP(0x2a,   STI_REGISTER_RIZ);
    MAP(0x2b, STI_REGISTER_RSI);
    MAP(0x2c, STI_REGISTER_RSP);
#undef MAP
  default:
    assert(llvmID != llvmID); // unrecognized llvm register number
    break;
  }

  return stiID;
}

#define PRIMITIVE_TYPE_MAPPINGS                                                \
  X(dwarf::DW_ATE_address, 4, T_32PVOID, T_32PVOID)                            \
  X(dwarf::DW_ATE_boolean, 1, T_BOOL08, T_BOOL08)                              \
  X(dwarf::DW_ATE_boolean, 2, T_BOOL16, T_BOOL16)                              \
  X(dwarf::DW_ATE_boolean, 4, T_BOOL32, T_BOOL32)                              \
  X(dwarf::DW_ATE_boolean, 8, T_BOOL64, T_BOOL64)                              \
  X(dwarf::DW_ATE_complex_float, 4, T_CPLX32, T_CPLX32)                        \
  X(dwarf::DW_ATE_complex_float, 8, T_CPLX64, T_CPLX64)                        \
  X(dwarf::DW_ATE_complex_float, 10, T_CPLX80, T_CPLX80)                       \
  X(dwarf::DW_ATE_complex_float, 16, T_CPLX128, T_CPLX128)                     \
  X(dwarf::DW_ATE_float, 4, T_REAL32, T_REAL32)                                \
  X(dwarf::DW_ATE_float, 6, T_REAL48, T_REAL48)                                \
  X(dwarf::DW_ATE_float, 8, T_REAL64, T_REAL64)                                \
  X(dwarf::DW_ATE_float, 10, T_REAL80, T_REAL80)                               \
  X(dwarf::DW_ATE_float, 16, T_REAL128, T_REAL128)                             \
  X(dwarf::DW_ATE_decimal_float, 4, T_REAL32, T_REAL32)                        \
  X(dwarf::DW_ATE_decimal_float, 6, T_REAL48, T_REAL48)                        \
  X(dwarf::DW_ATE_decimal_float, 8, T_REAL64, T_REAL64)                        \
  X(dwarf::DW_ATE_decimal_float, 10, T_REAL80, T_REAL80)                       \
  X(dwarf::DW_ATE_decimal_float, 16, T_REAL128, T_REAL128)                     \
  X(dwarf::DW_ATE_signed, 1, T_CHAR, T_CHAR)                                   \
  X(dwarf::DW_ATE_signed, 2, T_SHORT, T_SHORT)                                 \
  X(dwarf::DW_ATE_signed, 4, T_INT4, T_LONG)                                   \
  X(dwarf::DW_ATE_signed, 8, T_QUAD, T_QUAD)                                   \
  X(dwarf::DW_ATE_signed_char, 1, T_CHAR, T_CHAR)                              \
  X(dwarf::DW_ATE_unsigned, 1, T_UCHAR, T_UCHAR)                               \
  X(dwarf::DW_ATE_unsigned, 2, T_USHORT, T_USHORT)                             \
  X(dwarf::DW_ATE_unsigned, 4, T_UINT4, T_ULONG)                               \
  X(dwarf::DW_ATE_unsigned, 8, T_UQUAD, T_UQUAD)                               \
  X(dwarf::DW_ATE_unsigned_char, 1, T_UCHAR, T_UCHAR)
// FIXME: dwarf::DW_ATE_imaginary_float
// FIXME: dwarf::DW_ATE_packed_decimal
// FIXME: dwarf::DW_ATE_numeric_string
// FIXME: dwarf::DW_ATE_edited
// FIXME: dwarf::DW_ATE_signed_fixed
// FIXME: dwarf::DW_ATE_unsigned_fixed
// FIXME: dwarf::DW_ATE_UTF

static STITypeBasic::Primitive
toPrimitive(dwarf::TypeKind encoding, uint32_t byteSize,
            bool isLong) // FIXME: improve this implementation
{
  STITypeBasic::Primitive primitive;

// FIXME: Algorithm is not efficient.
#define X(ENCODING, BYTESIZE, PRIMITIVE, PRIMITIVE2)                           \
  if (encoding == ENCODING && byteSize == BYTESIZE) {                          \
    primitive = (isLong) ? PRIMITIVE2 : PRIMITIVE;                             \
  } else
  PRIMITIVE_TYPE_MAPPINGS
#undef X
  { primitive = T_NOTYPE; }

  return primitive;
}

STIType *STIDebugImpl::createTypeBasic(const DIBasicType llvmType) {
  unsigned int encoding = llvmType.getEncoding();
  dwarf::TypeKind typeKind = static_cast<dwarf::TypeKind>(encoding);
  uint64_t sizeInBytes = llvmType.getSizeInBits() >> 3;
  STITypeBasic *type;
  bool isLong = false;

  if (llvmType.getName().count("long")) {
    isLong = true;
  }

  type = STITypeBasic::create();
  type->setPrimitive(toPrimitive(typeKind, sizeInBytes, isLong));
  type->setSizeInBits(llvmType.getSizeInBits());

  return type;
}

template <typename T> T STIDebugImpl::resolve(DIRef<T> ref) const {
  return ref.resolve(getTypeIdentifierMap());
}

STIType *STIDebugImpl::createTypePointer(const DIDerivedType llvmType) {
  STITypePointer *type;

  type = STITypePointer::create();
  if (llvmType.getTypeDerivedFrom() == nullptr) {
    type->setPointerTo(getVoidType());
  } else {
    DIType derivedType = resolve(llvmType.getTypeDerivedFrom());
    type->setPointerTo(createType(derivedType));
  }
  type->setSizeInBits(llvmType.getSizeInBits());

  return type;
}

STIType *STIDebugImpl::createTypeModifier(const DIDerivedType llvmType) {
  STITypeModifier *type;
  STIType *qualifiedType;

  qualifiedType = createType(resolve(llvmType.getTypeDerivedFrom()));

  type = STITypeModifier::create();
  type->setQualifiedType(qualifiedType);
  type->setIsConstant(llvmType.getTag() == dwarf::DW_TAG_const_type);
  type->setIsVolatile(llvmType.getTag() == dwarf::DW_TAG_volatile_type);
  type->setIsUnaligned(false);
  type->setSizeInBits(qualifiedType->getSizeInBits());

  return type;
}

STIType *STIDebugImpl::createSymbolUserDefined(const DIDerivedType llvmType) {
  STISymbolUserDefined *symbol;
  STIType *userDefinedType;

  DIType derivedType = resolve(llvmType.getTypeDerivedFrom());
  userDefinedType = createType(derivedType);

  if (userDefinedType->getKind() == STI_OBJECT_KIND_TYPE_STRUCTURE) {
    STITypeStructure *pType = static_cast<STITypeStructure *>(userDefinedType);
    if (pType->getName().empty()) {
      pType->setName(llvmType.getName());
    }
  }

  if (userDefinedType->getKind() == STI_OBJECT_KIND_TYPE_FUNCTION_ID) {
    STITypeFunctionID *pType =
        static_cast<STITypeFunctionID *>(userDefinedType);
    userDefinedType = pType->getType();
  }

  symbol = STISymbolUserDefined::create();
  symbol->setDefinedType(userDefinedType);
  symbol->setName(llvmType.getName());

  getOrCreateScope(resolve(llvmType.getContext()))->add(symbol);

  return userDefinedType;
}

STIType *STIDebugImpl::createTypeArray(const DICompositeType llvmType) {
  STITypeArray *type = nullptr;
  STIType *elementType;

  elementType = createType(resolve(llvmType.getTypeDerivedFrom()));

  // Add subranges to array type.
  DIArray Elements = llvmType.getElements();
  uint32_t elementLength = elementType->getSizeInBits() >> 3;
  for (int i = Elements.getNumElements() - 1; i >= 0; --i) {
    DIDescriptor Element = Elements.getElement(i);
    if (Element.getTag() != dwarf::DW_TAG_subrange_type) {
      assert(false && "Can array have element that is not of a subrange type?");
      continue;
    }
    DISubrange SR = DISubrange(Element);
    int64_t LowerBound = SR.getLo();
    int64_t DefaultLowerBound = 0; // FIXME : default bound
    int64_t Count = SR.getCount();

    assert(LowerBound == DefaultLowerBound && "TODO: fix default bound check");

    type = STITypeArray::create();
    type->setElementType(elementType);
    type->setLength((uint32_t)(elementLength * Count));

    elementType = type;
    elementLength *= Count;

    if (i != 0) {
      const_cast<STIDebugImpl *>(this)->getTypeTable()->push_back(
          type); // FIXME
    }
  }

  assert(elementLength == (llvmType.getSizeInBits() >> 3) &&
         "mismatch: bad array subrange sizes");

  type->setName(llvmType.getName());
  type->setSizeInBits(llvmType.getSizeInBits());

  return type;
}

/// If this type is derived from a base type then return base type size.
uint64_t STIDebugImpl::getBaseTypeSize(DIDerivedType Ty) const {
  unsigned Tag = Ty.getTag();

  if (Tag != dwarf::DW_TAG_member && Tag != dwarf::DW_TAG_typedef &&
      Tag != dwarf::DW_TAG_const_type && Tag != dwarf::DW_TAG_volatile_type &&
      Tag != dwarf::DW_TAG_restrict_type)
    return Ty.getSizeInBits();

  DIType BaseType = resolve(Ty.getTypeDerivedFrom());

  // If this type is not derived from any type or the type is a declaration then
  // take conservative approach.
  if (!BaseType.isValid() || BaseType.isForwardDecl())
    return Ty.getSizeInBits();

  // If this is a derived type, go ahead and get the base type, unless it's a
  // reference then it's just the size of the field. Pointer types have no need
  // of this since they're a different type of qualification on the type.
  if (BaseType.getTag() == dwarf::DW_TAG_reference_type ||
      BaseType.getTag() == dwarf::DW_TAG_rvalue_reference_type)
    return Ty.getSizeInBits();

  if (BaseType.isDerivedType())
    return getBaseTypeSize(DIDerivedType(BaseType));

  return BaseType.getSizeInBits();
}

STIType *STIDebugImpl::createTypeStructure(const DICompositeType llvmType,
                                           bool isDcl) {
  STITypeStructure *type;
  STITypeFieldList *fieldType = nullptr;
  int16_t prop = 0;
  int32_t size = 0;

  if (llvmType.isForwardDecl()) {
    isDcl = true;
  }

  if (isDcl) {
    prop = prop | PROP_FWDREF;
  } else {
    fieldType = STITypeFieldList::create();

    // Add elements to structure type.
    DIArray Elements = llvmType.getElements();
    for (unsigned i = 0, N = Elements.getNumElements(); i < N; ++i) {
      DIDescriptor Element = Elements.getElement(i);
      if (Element.isSubprogram()) {
        // FIXME: implement this case
        // getOrCreateSubprogramDIE(DISubprogram(Element));
      } else if (Element.isDerivedType()) {
        DIDerivedType DDTy(Element);
        if (DDTy.getTag() == dwarf::DW_TAG_friend) {
          // FIXME: implement this case
          // DIE &ElemDie = createAndAddDIE(dwarf::DW_TAG_friend, Buffer);
          // addType(ElemDie, resolve(DDTy.getTypeDerivedFrom()),
          //        dwarf::DW_AT_friend);
        } else if (DDTy.isStaticMember()) {
          // FIXME: implement this case
          // getOrCreateStaticMemberDIE(DDTy);
        } else {
          // constructMemberDIE(Buffer, DDTy);

          assert(!(DDTy.getTag() == dwarf::DW_TAG_inheritance &&
                   DDTy.isVirtual()) &&
                 "FIXME: implement this case");

          STITypeMember *memberType = STITypeMember::create();

          uint16_t attribute = 0;

          if (DDTy.isProtected())
            attribute = attribute | STI_ACCESS_PRIVATE;
          else if (DDTy.isPrivate())
            attribute = attribute | STI_ACCESS_PROTECT;
          // Otherwise C++ member and base classes are considered public.
          else if (DDTy.isPublic())
            attribute = attribute | STI_ACCESS_PUBLIC;
          else if (llvmType.getTag() == dwarf::DW_TAG_class_type)
            attribute = attribute | STI_ACCESS_PRIVATE;
          else
            attribute = attribute | STI_ACCESS_PUBLIC;

          if (DDTy.isVirtual()) {
            assert(!"FIXME: implement this case");
          }

          if (DDTy.isArtificial()) {
            assert(!"FIXME: implement this case");
          }

          STIType *memberBaseType =
              createType(resolve(DDTy.getTypeDerivedFrom()));

          uint64_t Size = DDTy.getSizeInBits();
          uint64_t FieldSize = getBaseTypeSize(DDTy);
          uint64_t OffsetInBytes;

          if (Size != FieldSize) {
            STITypeBitfield *bitfieldType = STITypeBitfield::create();

            uint64_t Offset = DDTy.getOffsetInBits();
            uint64_t AlignMask = ~(DDTy.getAlignInBits() - 1);
            uint64_t HiMark = (Offset + FieldSize) & AlignMask;
            uint64_t FieldOffset = (HiMark - FieldSize);
            Offset -= FieldOffset;

            // Maybe we need to work from the other end.
            // if (ASM()->getDataLayout().isLittleEndian())
            //  Offset = FieldSize - (Offset + Size);

            bitfieldType->setOffset(Offset);
            bitfieldType->setSize(Size);
            bitfieldType->setType(memberBaseType);

            const_cast<STIDebugImpl *>(this)->getTypeTable()->push_back(
                bitfieldType); // FIXME

            OffsetInBytes = FieldOffset >> 3;
            memberBaseType = bitfieldType;
          } else {
            // This is not a bitfield.
            OffsetInBytes = DDTy.getOffsetInBits() >> 3;
          }

          memberType->setAttribute(attribute);
          memberType->setType(memberBaseType);
          memberType->setOffset(OffsetInBytes);
          memberType->setName(DDTy.getName());

          fieldType->getMembers().push_back(memberType);
        }
      }
    }

    const_cast<STIDebugImpl *>(this)->getTypeTable()->push_back(
        fieldType); // FIXME

    size = (uint32_t)(llvmType.getSizeInBits() >> 3);
  }
#if 0
    DICompositeType ContainingType(resolve(CTy.getContainingType()));
    if (ContainingType)
      addDIEEntry(Buffer, dwarf::DW_AT_containing_type,
                  *getOrCreateTypeDIE(ContainingType));

    // Add template parameters to a class, structure or union types.
    // FIXME: The support isn't in the metadata for this yet.
    if (Tag == dwarf::DW_TAG_class_type ||
        Tag == dwarf::DW_TAG_structure_type || Tag == dwarf::DW_TAG_union_type)
      addTemplateParams(Buffer, CTy.getTemplateParams());
#endif

  type = STITypeStructure::create();

  switch (llvmType.getTag()) {
#define X(TAG, TYPE)                                                           \
  case dwarf::TAG:                                                             \
    type->setLeaf(TYPE);                                                       \
    break
    X(DW_TAG_class_type, LF_CLASS);
    X(DW_TAG_structure_type, LF_STRUCTURE);
    X(DW_TAG_union_type, LF_UNION);
#undef X
  default:
    assert(!"Unknown structure type");
  }

  type->setCount(isDcl ? 0 : llvmType.getElements().getNumElements());

  type->setProperty(prop); //  FIXME: property

  type->setFieldType(fieldType);

  // type->setDerivedType(STIType* derivedType);
  // type->setVShapeType(STIType* vshapeType);

  type->setSize(size);

  type->setName(llvmType.getName());

  type->setSizeInBits(llvmType.getSizeInBits());

  if (!isDcl && !llvmType.getName().empty()) {
    STISymbolUserDefined *symbol = STISymbolUserDefined::create();
    symbol->setDefinedType(type);
    symbol->setName(llvmType.getName());

    getOrCreateScope(resolve(llvmType.getContext()))->add(symbol);
  }

  return type;
}

STIType *STIDebugImpl::createTypeEnumeration(const DICompositeType llvmType) {
  STITypeEnumeration *type;
  STITypeFieldList *fieldType = nullptr;
  int16_t prop = 0;

  DIArray Elements = llvmType.getElements();

  fieldType = STITypeFieldList::create();

  // Add enumerators to enumeration type.
  for (unsigned i = 0, N = Elements.getNumElements(); i < N; ++i) {
    DIEnumerator Enum(Elements.getElement(i));
    if (!Enum.isEnumerator()) {
      assert(!"enumeration element is not an enumerator!");
      continue;
    }
    STITypeEnumerator *enumeratorType = STITypeEnumerator::create();

    uint16_t attribute = 0;

    attribute = attribute | STI_ACCESS_PUBLIC;

    enumeratorType->setAttribute(attribute); // FIXME: attribute
    enumeratorType->setValue(Enum.getEnumValue());
    enumeratorType->setName(Enum.getName());

    fieldType->getEnumerators().push_back(enumeratorType);
  }

  const_cast<STIDebugImpl *>(this)->getTypeTable()->push_back(
      fieldType); // FIXME

  type = STITypeEnumeration::create();

  type->setCount(
      llvmType.getElements().getNumElements()); // TODO: is this right?

  type->setProperty(prop); //  FIXME: property

  type->setElementType(createType(resolve(llvmType.getTypeDerivedFrom())));

  type->setFieldType(fieldType);

  type->setName(llvmType.getName());

  type->setSizeInBits(llvmType.getSizeInBits());

  return type;
}

STIType *STIDebugImpl::createTypeSubroutine(const DISubroutineType llvmType) {
  STITypeFunctionID *funcIDType;
  STITypeProcedure *procedureType;
  STITypeArgumentList *argListType;

  funcIDType = STITypeFunctionID::create();
  procedureType = STITypeProcedure::create();
  argListType = STITypeArgumentList::create();

  funcIDType->setType(procedureType);
  funcIDType->setParentScope(nullptr);
  funcIDType->setName("");

  procedureType->setCallingConvention(NEAR_C); // FIXME:
  procedureType->setArgumentList(argListType);

  // Add return type. A void return won't have a type.
  DITypeArray Elements = llvmType.getTypeArray();
  DIType RTy(resolve(Elements.getElement(0)));
  if (RTy) {
    procedureType->setReturnType(createType(RTy));
  } else {
    procedureType->setReturnType(getVoidType());
  }

  // Function
  procedureType->setParamCount(Elements.getNumElements() - 1);
  for (unsigned i = 1, N = Elements.getNumElements(); i < N; ++i) {
    DIType Ty = resolve(Elements.getElement(i));
    if (!Ty) {
      assert(i == N - 1 && "Unspecified parameter must be the last argument");
      // FIXME: handle variadic function argument
      // createAndAddDIE(dwarf::DW_TAG_unspecified_parameters, Buffer);
      procedureType->setParamCount(Elements.getNumElements() - 2);
      argListType->getArgumentList()->push_back(nullptr);
    } else {
      argListType->getArgumentList()->push_back(createType(Ty));
      // if (DIType(Ty).isArtificial())
      //  addFlag(Arg, dwarf::DW_AT_artificial);
    }
  }
  if (Elements.getNumElements() == 1) {
    // Function with no arguments should have one T_NOTYPE argument.
    argListType->getArgumentList()->push_back(nullptr);
  }

#if 0
    bool isPrototyped = true;
    if (Elements.getNumElements() == 2 &&
        !Elements.getElement(1))
      isPrototyped = false;

    // Add prototype flag if we're dealing with a C language and the
    // function has been prototyped.
    uint16_t Language = getLanguage();
    if (isPrototyped &&
        (Language == dwarf::DW_LANG_C89 || Language == dwarf::DW_LANG_C99 ||
         Language == dwarf::DW_LANG_ObjC))
      addFlag(Buffer, dwarf::DW_AT_prototyped);

    if (CTy.isLValueReference())
      addFlag(Buffer, dwarf::DW_AT_reference);

    if (CTy.isRValueReference())
      addFlag(Buffer, dwarf::DW_AT_rvalue_reference);
#endif

  const_cast<STIDebugImpl *>(this)->getTypeTable()->push_back(
      argListType); // FIXME
  const_cast<STIDebugImpl *>(this)->getTypeTable()->push_back(
      procedureType); // FIXME

  return funcIDType;
}

STIType *STIDebugImpl::getVoidType() const {
  if (_voidType == nullptr) {
    STITypeBasic *voidType = STITypeBasic::create();
    voidType->setPrimitive(T_VOID);
    const_cast<STIDebugImpl *>(this)->getTypeTable()->push_back(
        voidType); // FIXME
    const_cast<STIDebugImpl *>(this)->_voidType = voidType;
  }
  return _voidType;
}

STIType *STIDebugImpl::createType(const DIType llvmType) {
  STIType *type;
  unsigned int tag;

  TypeMap *TM = const_cast<STIDebugImpl *>(this)->getTypeMap(); // FIXME

  TypeMap::iterator itr = TM->find(llvmType);

  if (itr != TM->end()) {
    if (itr->second != nullptr) {
      return itr->second;
    }
    switch (llvmType.getTag()) {
#define X(TAG, HANDLER, TYPE)                                                  \
  case dwarf::TAG:                                                             \
    type = HANDLER(static_cast<TYPE>(llvmType), true);                         \
    const_cast<STIDebugImpl *>(this)->getTypeTable()->push_back(type);         \
    return type
      X(DW_TAG_class_type, createTypeStructure, DICompositeType);
      X(DW_TAG_structure_type, createTypeStructure, DICompositeType);
      X(DW_TAG_union_type, createTypeStructure, DICompositeType);
#undef X
    default:
      break;
    }
  } else {
    TM->insert(std::make_pair(llvmType, nullptr)); // FIXME
  }

  tag = llvmType.getTag();
  switch (tag) {
#define X(TAG, HANDLER, TYPE)                                                  \
  case dwarf::TAG:                                                             \
    type = HANDLER(static_cast<TYPE>(llvmType));                               \
    break
    X(DW_TAG_array_type, createTypeArray, DICompositeType);
    X(DW_TAG_class_type, createTypeStructure, DICompositeType);
    X(DW_TAG_structure_type, createTypeStructure, DICompositeType);
    X(DW_TAG_union_type, createTypeStructure, DICompositeType);
    X(DW_TAG_enumeration_type, createTypeEnumeration, DICompositeType);
    X(DW_TAG_base_type, createTypeBasic, DIBasicType);
    X(DW_TAG_pointer_type, createTypePointer, DIDerivedType);
    X(DW_TAG_const_type, createTypeModifier, DIDerivedType);
    X(DW_TAG_volatile_type, createTypeModifier, DIDerivedType);
    X(DW_TAG_typedef, createSymbolUserDefined, DIDerivedType);
    X(DW_TAG_subroutine_type, createTypeSubroutine, DISubroutineType);
#undef X
  default:
    assert(tag != tag); // unhandled type tag!
    break;
  }

  itr = TM->find(llvmType); // FIXME
  assert(itr != TM->end() && "Type should be in map by now!");
  if (itr->second != nullptr) {
    return itr->second;
  }
  itr->second = type;
  if (tag != dwarf::DW_TAG_typedef) {
    const_cast<STIDebugImpl *>(this)->getTypeTable()->push_back(type); // FIXME
  }

  return type;
}

STIScope *STIDebugImpl::getOrCreateScope(const DIScope llvmScope) {
  if (!llvmScope || llvmScope.isFile())
    return getCompileUnit()->getScope();
  if (llvmScope.isType()) {
    return getOrCreateScope(resolve(DIType(llvmScope).getContext()));
  }
  if (llvmScope.isNameSpace()) {
    // return getOrCreateNameSpace(DINameSpace(llvmScope));
    assert(!"FIXME");
  }
  if (llvmScope.isSubprogram()) {
    return getOrCreateSymbolProcedure(DISubprogram(llvmScope))->getScope();
  }
  if (hasScope(llvmScope)) {
    return getScope(llvmScope);
  }
  assert(llvmScope.isLexicalBlock() && "Expect Lexical scope");
  STISymbolBlock *block = createSymbolBlock(DILexicalBlock(llvmScope));
  addScope(llvmScope, block->getScope());
  return block->getScope();
}

STISymbolVariable *STIDebugImpl::createSymbolVariable(
    const DIVariable DIV, unsigned int frameIndex, const MachineInstr *DVInsn) {
  STISymbolVariable *variable;
  STILocation *location = nullptr;

  variable = STISymbolVariable::create();
  variable->setName(DIV.getName());
  variable->setType(createType(resolve(DIV.getType())));

  if (frameIndex != ~0U) {
    unsigned int regnum;
    int offset;

    const TargetFrameLowering *TFL =
        ASM()->TM.getSubtargetImpl()->getFrameLowering();

    regnum = 0;
    offset = TFL->getFrameIndexReference(*ASM()->MF, frameIndex, regnum);

    location = STILocation::createRegisterOffset(toSTIRegID(regnum), offset);

  } else {
    assert(DVInsn && "Unknown location");
    assert(DVInsn->getNumOperands() == 3);
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
        location = STILocation::createRegister(toSTIRegID(RegOp.getReg()));
      }
    } else if (DVInsn->getOperand(0).isImm()) {
      assert(!"FIXME: support this case");
      // addConstantValue(*VariableDie, DVInsn->getOperand(0), DV.getType());
    } else if (DVInsn->getOperand(0).isFPImm()) {
      assert(!"FIXME: support this case");
      // addConstantFPValue(*VariableDie, DVInsn->getOperand(0));
    } else if (DVInsn->getOperand(0).isCImm()) {
      assert(!"FIXME: support this case");
      // addConstantValue(*VariableDie, DVInsn->getOperand(0).getCImm(),
      //                 DV.getType());
    }
  }

  variable->setLocation(location);

  return variable;
}

STISymbolProcedure *
STIDebugImpl::getOrCreateSymbolProcedure(const DISubprogram &SP) {
  Function *pFunc = SP.getFunction();
  assert(pFunc && "LLVM subprogram has no LLVM function");
  if (_functionMap.count(pFunc)) {
    // Function is already created
    return _functionMap[pFunc];
  }
  STISymbolProcedure *procedure;
  procedure = STISymbolProcedure::create();
  procedure->setName(SP.getName());
  procedure->setType(createType(resolve(SP.getType().operator DITypeRef())));
  procedure->getLineSlice()->setFunction(SP.getFunction());
  procedure->setSymbolID(SP.isLocalToUnit() ? S_LPROC32_ID : S_GPROC32_ID);

  if (procedure->getType()->getKind() == STI_OBJECT_KIND_TYPE_FUNCTION_ID) {
    STITypeFunctionID *pType =
        static_cast<STITypeFunctionID *>(procedure->getType());
    pType->setName(SP.getName());
  }

  getOrCreateScope(resolve(SP.getContext()))
      ->add(procedure); // FIXME: inline function!?

  _functionMap.insert(std::make_pair(pFunc, procedure));
  return procedure;
}

STISymbolBlock *STIDebugImpl::createSymbolBlock(const DILexicalBlock &LB) {
  STISymbolBlock *block;
  block = STISymbolBlock::create();

  LexicalScope *Scope = _lexicalScopes.findLexicalScope(LB);
  const SmallVectorImpl<InsnRange> &Ranges = Scope->getRanges();
  assert(!Ranges.empty() && "Handle Block with empty range ");
  assert(Ranges.size() == 1 && "Handle Block with more than one range");

  const MachineInstr *BInst = Ranges.front().first;
  const MachineInstr *EInst = Ranges.front().second;

  assert(_labelsBeforeInsn[BInst] && "empty range begin location");
  assert(_labelsAfterInsn[EInst] && "empty range end location");
  // FIXME: emit block labels correctly
  block->setLabelBegin(_labelsBeforeInsn[BInst]);
  block->setLabelEnd(_labelsAfterInsn[EInst]);
  block->setName(LB.getName());

  getOrCreateScope(LB.getContext())->add(block);

  DIScope FuncScope = LB.getContext();
  while (FuncScope.isLexicalBlock()) {
    FuncScope = DILexicalBlock(FuncScope).getContext();
  }
  assert(FuncScope.isSubprogram() &&
         "Failed to reach function scope of a lexical block");
  block->setProcedure(getOrCreateSymbolProcedure(DISubprogram(FuncScope)));

  return block;
}

void STIDebugImpl::collectModuleInfo() {
  const Module *M = getModule();
  STISymbolModule *module;

  module = STISymbolModule::create(M);
  getSymbolTable()->setRoot(module);

  NamedMDNode *CU_Nodes = M->getNamedMetadata("llvm.dbg.cu");
  if (!CU_Nodes)
    return;

  TypeIdentifierMap = generateDITypeIdentifierMap(CU_Nodes);

  for (const MDNode *node : CU_Nodes->operands()) {
    DICompileUnit CU(node);
    STISymbolCompileUnit *compileUnit;

    compileUnit = STISymbolCompileUnit::create(CU);
    compileUnit->setProducer(CU.getProducer());
    compileUnit->setMachineID(
        toMachineID(Triple(ASM()->getTargetTriple()).getArch()));
    module->add(compileUnit);

    DIArray GVs = CU.getGlobalVariables();
    for (unsigned int i = 0, e = GVs.getNumElements(); i < e; ++i) {
      // TODO: handled initialized global variable if needed.

      DIGlobalVariable GV(GVs.getElement(i));
      STISymbolVariable *variable;

      MCSymbol *label = ASM()->getSymbol(GV.getGlobal());

      STILocation *location =
          GV.isLocalToUnit() ? STILocation::createLocalSegmentedOffset(label)
                             : STILocation::createGlobalSegmentedOffset(label);

      variable = STISymbolVariable::create();
      variable->setName(GV.getName());
      variable->setType(createType(resolve(GV.getType())));
      variable->setLocation(location);

      getOrCreateScope(GV.getContext())->add(variable);
    }

    DIArray SPs = CU.getSubprograms();
    for (unsigned int i = 0, e = SPs.getNumElements(); i < e; ++i) {
      DISubprogram SP(SPs.getElement(i));

      getOrCreateSymbolProcedure(SP);
    }
  }
}

void STIDebugImpl::collectRoutineInfo() {
  typedef MachineModuleInfo::VariableDbgInfo VariableDbgInfo;
  typedef DbgValueHistoryMap::InstrRanges InstrRanges;
  typedef std::pair<const MDNode *, InstrRanges> VariableHistoryInfo;

  //STISymbolProcedure *procedure = getCurrentProcedure();
  STISymbolVariable *variable;

  std::set<const MDNode *> processed;

  for (const VariableDbgInfo &info : MMI()->getVariableDbgInfo()) {
    DIVariable DIV(info.Var);

    if (processed.count(DIV))
      continue;

    variable = createSymbolVariable(DIV, info.Slot);
    getOrCreateScope(DIV.getContext())->add(variable);

    processed.insert(DIV);
  }

  for (const VariableHistoryInfo &info : _valueHistory) {
    const MDNode *node = info.first;
    const DbgValueHistoryMap::InstrRanges &Ranges = info.second;
    DIVariable DIV(node);

    if (processed.count(DIV))
      continue;

    const MachineInstr *MInsn = Ranges.front().first;
    variable = createSymbolVariable(DIV, ~0, MInsn); // FIXME: params
    getOrCreateScope(DIV.getContext())->add(variable);

    processed.insert(DIV);
  }
}

void STIDebugImpl::layout() {
  uint16_t nextTypeIndex = 0x1000;
  for (STIType *type : *getTypeTable()) {
    switch (type->getKind()) {
    case STI_OBJECT_KIND_TYPE_BASIC:
      type->setIndex(static_cast<STITypeBasic *>(type)->getPrimitive());
      continue;
    case STI_OBJECT_KIND_TYPE_POINTER: {
      STITypePointer *pType = static_cast<STITypePointer *>(type);
      STIType *pPointerTo = pType->getPointerTo();
      if (pPointerTo->getKind() == STI_OBJECT_KIND_TYPE_BASIC) {
        STITypeBasic *pBasicType = static_cast<STITypeBasic *>(pPointerTo);
        switch (pBasicType->getPrimitive()) {
        // TODO: Add more cases!
        case T_CHAR:
          type->setIndex(T_64PRCHAR);
          continue;
        }
      }
    }
    default:
      type->setIndex(nextTypeIndex++);
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

void STIDebugImpl::emit() {
  emitSymbols();
  emitTypes();
}

void STIDebugImpl::emitSymbolID(const STISymbolID symbolID) const {
  emitComment(toString(symbolID));
  emitInt16(symbolID);
}

void STIDebugImpl::emitSubsectionBegin(STISubsection *subsection) const {
  STISubsectionID id = subsection->getID();

  // Create the beginning and ending labels for this subsection.
  subsection->setBegin(ASM()->MMI->getContext().CreateTempSymbol());
  subsection->setEnd(ASM()->MMI->getContext().CreateTempSymbol());

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

static STISubsection *currentSubsection = nullptr;

void STIDebugImpl::closeSubsection() const {
  if (currentSubsection != nullptr) {
    emitSubsectionEnd(currentSubsection);
    delete currentSubsection;
    currentSubsection = nullptr;
  }
}

void STIDebugImpl::emitSubsection(STISubsectionID id) const {
  // If trying to change subsection to same subsection do nothing.
  if (currentSubsection != nullptr && currentSubsection->getID() == id) {
    return;
  }

  closeSubsection();

  currentSubsection = new STISubsection(id);
  emitSubsectionBegin(currentSubsection);
}

MCSymbol *STIDebugImpl::createFuncLabel(const char *name) const {
  return ASM()->GetTempSymbol(name, ASM()->getFunctionNumber());
}

MCSymbol *STIDebugImpl::createBlockLabel(const char *name) {
  return ASM()->GetTempSymbol(name, _blockNumber++);
}

void STIDebugImpl::emitAlign(unsigned int byteAlignment) const {
  ASM()->OutStreamer.EmitValueToAlignment(byteAlignment);
}

void STIDebugImpl::emitPadding(unsigned int padByteCount) const {
  static const int paddingArray[16] = {LF_PAD0,  LF_PAD1,  LF_PAD2,  LF_PAD3,
                                       LF_PAD4,  LF_PAD5,  LF_PAD6,  LF_PAD7,
                                       LF_PAD8,  LF_PAD9,  LF_PAD10, LF_PAD11,
                                       LF_PAD12, LF_PAD13, LF_PAD14, LF_PAD15};

  for (unsigned int i = padByteCount; i > 0; --i) {
    emitInt8(paddingArray[i]);
  }
}

void STIDebugImpl::emitInt8(int value) const { ASM()->EmitInt8(value); }

void STIDebugImpl::emitInt16(int value) const { ASM()->EmitInt16(value); }

void STIDebugImpl::emitInt32(int value) const { ASM()->EmitInt32(value); }

void STIDebugImpl::emitString(StringRef string) const {
  ASM()->OutStreamer.EmitBytes(string);
  ASM()->EmitInt8(0);
}

void STIDebugImpl::emitBytes(const char *data, size_t size) const {
  ASM()->OutStreamer.EmitBytes(StringRef(data, size));
}

void STIDebugImpl::emitFill(size_t size, const uint8_t byte) const {
  ASM()->OutStreamer.EmitFill(size, byte);
}

void STIDebugImpl::emitComment(StringRef comment) const {
  ASM()->OutStreamer.AddComment(comment);
}

void STIDebugImpl::emitLabel(MCSymbol *symbol) const {
  ASM()->OutStreamer.EmitLabel(symbol);
}

void STIDebugImpl::emitValue(const MCExpr *value,
                             unsigned int sizeInBytes) const {
  ASM()->OutStreamer.EmitValue(value, sizeInBytes);
}

void STIDebugImpl::emitLabelDiff(const MCSymbol *begin,
                                 const MCSymbol *end) const {
  MCContext &context = ASM()->OutStreamer.getContext();
  const MCExpr *bExpr;
  const MCExpr *eExpr;
  const MCExpr *delta;

  bExpr = MCSymbolRefExpr::Create(begin, MCSymbolRefExpr::VK_None, context);
  eExpr = MCSymbolRefExpr::Create(end, MCSymbolRefExpr::VK_None, context);
  delta = MCBinaryExpr::Create(MCBinaryExpr::Sub, eExpr, bExpr, context);

  emitValue(delta, 4);
}

void STIDebugImpl::emitSecRel32(MCSymbol *symbol) const {
  ASM()->OutStreamer.EmitCOFFSecRel32(symbol);
}

void STIDebugImpl::emitSectionIndex(MCSymbol *symbol) const {
  ASM()->OutStreamer.EmitCOFFSectionIndex(symbol);
}

void STIDebugImpl::emitSymbolModule(const STISymbolModule *module) const {
  STISignatureID signatureID = module->getSignatureID();
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
  STICompile3Flags() { _flags.raw = 0; }

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

  verFEMajor = 0x0001;
  verFEMinor = 0x0002;
  verFEBuild = 0x0003;
  verFEQFE = 0x0004;
  verMajor = 0x0005;
  verMinor = 0x0006;
  verBuild = 0x0007;
  verQFE = 0x0008;

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
  int procType = procedure->getType()->getIndex();
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
  emitInt32(procType);
  emitSecRel32(functionLabel);
  emitSectionIndex(functionLabel);
  emitInt8(flags);
  emitString(name);
}

void STIDebugImpl::emitSymbolProcedureEnd() const {
  emitInt16(2);
  emitSymbolID(S_PROC_ID_END);
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

void STIDebugImpl::emitSymbolVariable(const STISymbolVariable *variable) const {
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
  MCSymbol *labelBegin = ASM()->MMI->getContext().CreateTempSymbol();
  MCSymbol *labelEnd = ASM()->MMI->getContext().CreateTempSymbol();

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

  emitSubsection(STI_SUBSECTION_LINES);

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

  kind = symbol->getKind();
  switch (kind) {
  case STI_OBJECT_KIND_SYMBOL_MODULE: {
    const STISymbolModule *module;
    module = static_cast<const STISymbolModule *>(symbol);
    emitSubsection(STI_SUBSECTION_SYMBOLS);
    emitSymbolModule(module);
    for (const STISymbolCompileUnit *unit : *module->getCompileUnits()) {
      walkSymbol(unit);
    }
  } break;

  case STI_OBJECT_KIND_SYMBOL_COMPILE_UNIT: {
    const STISymbolCompileUnit *compileUnit;
    compileUnit = static_cast<const STISymbolCompileUnit *>(symbol);
    emitSubsection(STI_SUBSECTION_SYMBOLS);
    emitSymbolCompileUnit(compileUnit);
    for (const STIObject *object : compileUnit->getScope()->getObjects()) {
      walkSymbol(static_cast<const STISymbol *>(object)); // FIXME: cast
    }
  } break;

  case STI_OBJECT_KIND_SYMBOL_PROCEDURE: {
    const STISymbolProcedure *procedure;

    procedure = static_cast<const STISymbolProcedure *>(symbol);
    emitSubsection(STI_SUBSECTION_SYMBOLS);
    emitSymbolProcedure(procedure);
    for (const STIObject *object : procedure->getScope()->getObjects()) {
      walkSymbol(static_cast<const STISymbol *>(object));
    }
    emitSymbolProcedureEnd();
    emitLineSlice(procedure);
  } break;

  case STI_OBJECT_KIND_SYMBOL_BLOCK: {
    const STISymbolBlock *block;

    block = static_cast<const STISymbolBlock *>(symbol);
    emitSubsection(STI_SUBSECTION_SYMBOLS);
    emitSymbolBlock(block);
    for (const STIObject *object : block->getScope()->getObjects()) {
      walkSymbol(static_cast<const STISymbol *>(object));
    }
    emitSymbolScopeEnd();
  } break;

  case STI_OBJECT_KIND_SYMBOL_VARIABLE: {
    const STISymbolVariable *variable;
    variable = static_cast<const STISymbolVariable *>(symbol);
    emitSubsection(STI_SUBSECTION_SYMBOLS);
    emitSymbolVariable(variable);
  } break;

  case STI_OBJECT_KIND_SYMBOL_USER_DEFINED: {
    const STISymbolUserDefined *userDefined;
    userDefined = static_cast<const STISymbolUserDefined *>(symbol);
    emitSubsection(STI_SUBSECTION_SYMBOLS);
    emitSymbolUserDefined(userDefined);
  } break;

  default:
    assert(kind != kind); // unrecognized symbol kind!
    break;
  }
}

void STIDebugImpl::emitSectionBegin(const MCSection *section) const {
  ASM()->OutStreamer.SwitchSection(section);
}

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
  const int16_t length = 8;

  emitInt16(length);
  emitInt16(LF_MODIFIER);
  emitInt32(qualifiedType->getIndex());
  emitInt16(attributes);
}

class STITypePointerAttributes {
private:
  union {
    int32_t raw;
    struct {
      uint32_t _ptrtype : 5;
      uint32_t _ptrmode : 3;
      uint32_t _isflat32 : 1;
      uint32_t _volatile : 1;
      uint32_t _const : 1;
      uint32_t _unaligned_ : 1;
      uint32_t _restrict_ : 1;
      uint32_t _reserved1 : 3;
      uint32_t _unknownField : 1;
      uint32_t _reserved2 : 15;
    } field;
  } _attributes;

public:
  STITypePointerAttributes(const STITypePointer *type) {
    _attributes.raw = 0;
    if (type->getSizeInBits() == 64) {
      _attributes.field._ptrtype = ATTR_PTRTYPE_64;
      _attributes.field._unknownField = 1; // Necessary to get "Size: 8"
    } else {
      _attributes.field._ptrtype = ATTR_PTRTYPE_NEAR32;
    }
  }

  ~STITypePointerAttributes() {}

  operator int32_t() const { return _attributes.raw; }
};

void STIDebugImpl::emitTypePointer(const STITypePointer *type) const {
  STITypePointerAttributes attributes(type);
  const STIType *pointerTo = type->getPointerTo();
  const int16_t length = 10;

  emitInt16(length);
  emitInt16(LF_POINTER);
  emitInt32(pointerTo->getIndex());
  emitInt32(attributes);
  // emit*    (variant);  // emitted based on pointer type
}

void STIDebugImpl::emitTypeArray(const STITypeArray *type) const {
  const STIType *elementType = type->getElementType();
  StringRef name = type->getName();
  const int32_t arrayLength = type->getLength();
  const int16_t length = 10 + getNumericSize(arrayLength) + name.size() + 1;

  emitInt16(length);
  emitInt16(LF_ARRAY);
  emitInt32(elementType->getIndex());
  emitInt32(T_ULONG);
  emitNumeric(arrayLength);
  emitString(name);
}

static std::string getRealName(std::string name) {
  return (Twine(".?AU") + Twine(name) + Twine("@@")).str();
}

void STIDebugImpl::emitTypeStructure(const STITypeStructure *type) const {
  static int s_count = 0;
  const uint16_t leaf = type->getLeaf();
  bool isUnion = (leaf == LF_UNION);
  const uint16_t count = type->getCount();
  const uint16_t prop = type->getProperty();
  const STIType *fieldType = type->getFieldType();
  const STIType *derivedType = type->getDerivedType();
  const STIType *vshapeType = type->getVShapeType();
  const int32_t size = type->getSize();
  std::string name = type->getName();

  if (name.empty()) {
    name = (Twine("<unnamed-tag>") + Twine(s_count++)).str();
  }

  std::string realName = getRealName(name);

  const int16_t length = (isUnion ? 10 : 18) + getNumericSize(size) +
                         name.size() + 1 + realName.size() + 1;

  emitInt16(length);
  emitInt16(leaf);
  emitInt16(count);
  emitInt16(prop | PROP_REALNAME);
  emitInt32(fieldType ? fieldType->getIndex() : 0);
  if (!isUnion) {
    emitInt32(derivedType ? derivedType->getIndex() : 0);
    emitInt32(vshapeType ? vshapeType->getIndex() : 0);
  }
  emitNumeric(size);
  emitString(name);
  emitString(realName);
}

void STIDebugImpl::emitTypeEnumeration(const STITypeEnumeration *type) const {
  const uint16_t count = type->getCount();
  const uint16_t prop = type->getProperty();
  const STIType *elementType = type->getElementType();
  const STIType *fieldType = type->getFieldType();
  StringRef name = type->getName();
  const int16_t length = 14 + name.size() + 1;

  emitInt16(length);
  emitInt16(LF_ENUM);
  emitInt16(count);
  emitInt16(prop);
  emitInt32(elementType ? elementType->getIndex() : 0);
  emitInt32(fieldType ? fieldType->getIndex() : 0);
  emitString(name);
}

void STIDebugImpl::emitTypeBitfield(const STITypeBitfield *type) const {
  const uint32_t offset = type->getOffset();
  const uint32_t size = type->getSize();
  const STIType *memberType = type->getType();
  const int16_t length = 10;

  emitInt16(length);
  emitInt16(LF_BITFIELD);
  emitInt32(memberType ? memberType->getIndex() : 0);
  emitInt8(size);
  emitInt8(offset);
  emitPadding(2);
}

void STIDebugImpl::emitTypeFieldList(const STITypeFieldList *type) const {
  uint16_t length = 2;
  for (STITypeMember *member : type->getMembers()) {
    uint32_t offset = member->getOffset();
    StringRef name = member->getName();
    int16_t memberLength = 8 + getNumericSize(offset) + name.size() + 1;
    length += getPaddedSize(memberLength);
  }

  for (STITypeEnumerator *enumerator : type->getEnumerators()) {
    uint32_t value = enumerator->getValue();
    StringRef name = enumerator->getName();
    int16_t enumeratorLength = 4 + getNumericSize(value) + name.size() + 1;
    length += getPaddedSize(enumeratorLength);
  }

  emitInt16(length);
  emitInt16(LF_FIELDLIST);

  for (STITypeMember *member : type->getMembers()) {
    uint16_t attribute = member->getAttribute();
    const STIType *memberType = member->getType();
    uint32_t offset = member->getOffset();
    StringRef name = member->getName();

    emitInt16(LF_MEMBER);
    emitInt16(attribute);
    emitInt32(memberType ? memberType->getIndex() : 0);
    emitNumeric(offset);
    emitString(name);

    int16_t memberLength = 8 + getNumericSize(offset) + name.size() + 1;
    int16_t paddedSize = getPaddedSize(memberLength);
    emitPadding(paddedSize - memberLength);
  }

  for (STITypeEnumerator *enumerator : type->getEnumerators()) {
    uint16_t attribute = enumerator->getAttribute();
    uint32_t value = enumerator->getValue();
    StringRef name = enumerator->getName();

    emitInt16(LF_ENUMERATE);
    emitInt16(attribute);
    emitNumeric(value);
    emitString(name);

    int16_t enumeratorLength = 4 + getNumericSize(value) + name.size() + 1;
    int16_t paddedSize = getPaddedSize(enumeratorLength);
    emitPadding(paddedSize - enumeratorLength);
  }
}

void STIDebugImpl::emitTypeFunctionID(const STITypeFunctionID *type) const {
  StringRef name = type->getName();
  const STIType *funcType = type->getType();
  const STIType *parentScope = type->getParentScope();

  uint16_t length = 10 + name.size() + 1;
  uint16_t paddedLength = getPaddedSize(length);

  emitInt16(paddedLength);
  emitInt16(LF_FUNC_ID);
  emitInt32(parentScope ? parentScope->getIndex() : 0);
  emitInt32(funcType ? funcType->getIndex() : 0);
  emitString(name);
  emitPadding(paddedLength - length);
}

void STIDebugImpl::emitTypeProcedure(const STITypeProcedure *type) const {
  const STIType *returnType = type->getReturnType();
  int callingConvention = type->getCallingConvention();
  uint16_t paramCount = type->getParamCount();
  const STIType *argumentList = type->getArgumentList();
  uint16_t length = 14;

  emitInt16(length);
  emitInt16(LF_PROCEDURE);
  emitInt32(returnType ? returnType->getIndex() : 0);
  emitInt8(callingConvention);
  emitInt8(0); // reserved
  emitInt16(paramCount);
  emitInt32(argumentList ? argumentList->getIndex() : 0);
}

void STIDebugImpl::emitTypeArgumentList(const STITypeArgumentList *type) const {
  uint32_t argumentCount = type->getArgumentCount();
  const STITypeTable *argumentList = type->getArgumentList();
  uint16_t length = 6 + 4 * argumentCount;

  emitInt16(length);
  emitInt16(LF_ARGLIST);
  emitInt32(argumentCount);
  for (const STIType *argemntType : *argumentList) {
    emitInt32(argemntType ? argemntType->getIndex() : 0);
  }
}

void STIDebugImpl::emitNumeric(const int32_t num) const {
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

void STIDebugImpl::emitType(const STIType *type) const {
  STIObjectKind kind;

  if (type->getIndex() < 0x1000) {
    // TODO: add a comment!
    return;
  }

  kind = type->getKind();
  switch (kind) {
#define X(KIND, HANDLER, TYPE)                                                 \
  case STI_OBJECT_KIND_TYPE_##KIND:                                            \
    HANDLER(static_cast<const TYPE *>(type));                                  \
    break
    X(BASIC, emitTypeBasic, STITypeBasic);
    X(MODIFIER, emitTypeModifier, STITypeModifier);
    X(POINTER, emitTypePointer, STITypePointer);
    X(ARRAY, emitTypeArray, STITypeArray);
    X(STRUCTURE, emitTypeStructure, STITypeStructure);
    X(ENUMERATION, emitTypeEnumeration, STITypeEnumeration);
    X(BITFIELD, emitTypeBitfield, STITypeBitfield);
    X(FIELD_LIST, emitTypeFieldList, STITypeFieldList);
    X(FUNCTION_ID, emitTypeFunctionID, STITypeFunctionID);
    X(PROCEDURE, emitTypeProcedure, STITypeProcedure);
    X(ARGUMENT_LIST, emitTypeArgumentList, STITypeArgumentList);
#undef X
  default:
    assert(kind != kind); // invalid type kind
    break;
  }
}

void STIDebugImpl::emitTypes() const {
  emitSectionBegin(ASM()->getObjFileLowering().getCOFFDebugTypesSection());

  emitComment("Types Section Signature");
  emitInt32(STI_SIGNATURE_LATEST); // FIXME:  record types header data

  for (const STIType *type : *getTypeTable()) {
    emitType(type);
  }
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
