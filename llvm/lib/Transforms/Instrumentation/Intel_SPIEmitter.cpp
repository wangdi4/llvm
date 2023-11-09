//===- Transforms/Instrumentation/Intel_SPIEmitter.cpp ---------*- C++ -*-===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Instrumentation/Intel_SPIEmitter.h"

#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/LEB128.h"

using namespace llvm;

#define DEBUG_TYPE "spi-emitter"

static cl::opt<bool>
    SPIGenerate("spi-generate", cl::init(false), cl::Hidden,
                cl::desc("Enable writing coverage mapping data to SPI file"));

// This option is used during testing to specify the temporary filename to save
// code coverage data needed to run the llvm-cov tool without using the
// application's binary image.
static cl::opt<std::string> SPITempFilename(
    "spi-temp-filename", cl::ReallyHidden,
    cl::desc("Name to use for temporary coverage mapping file"));

namespace {
class SPIEmitterImpl {
public:
  void run(Module &M);

private:
  // Returns true if the compiler generated symbol for the code coverage mapping
  // variable is present.
  bool containsCoverageSymbols(Module &M) const;

  GlobalVariable *getNamedSymbol(Module &M, StringRef Name) const;

  // Emit the code coverage data structures of the Module to the stream.
  bool writeTempSPI(Module &M, raw_ostream &OS);

  // Helpers used during writeTempSPI for writing various types.
  void writeInitializer(const DataLayout &DL, Constant *Init, raw_ostream &OS);
  void writeStructInitializer(const DataLayout &DL, ConstantStruct *CS,
                              raw_ostream &OS);
  void writeArrayInitializer(const DataLayout &DL, ConstantDataArray *CA,
                             raw_ostream &OS);
  void writeIntegerInitializer(const DataLayout &DL, ConstantInt *CI,
                               raw_ostream &OS);
};

void SPIEmitterImpl::run(Module &M) {
  if (!containsCoverageSymbols(M)) {
    LLVM_DEBUG(dbgs() << "spi-emitter: No code coverage symbols found\n");
    return;
  }

  std::string OutputName = SPITempFilename;
  if (OutputName.empty()) {
    // TODO: Generate a temporary file name for the data.
    llvm_unreachable("NYI");
  }

  std::error_code EC;
  raw_fd_ostream OS(OutputName, EC);
  if (EC) {
    errs() << "Failed to open '" << OutputName
           << "' for writing SPI information\n";
    return;
  }

  bool Written = writeTempSPI(M, OS);
  OS.close();

  if (Written) {
    // TODO: Append temp file into main pgopti.spi file.
  }

  if (SPITempFilename.empty()) {
    // TODO: Remove the generated temp file.
  }
}

bool SPIEmitterImpl::containsCoverageSymbols(Module &M) const {
  return getNamedSymbol(M, getCoverageMappingVarName()) != nullptr;
}

GlobalVariable *SPIEmitterImpl::getNamedSymbol(Module &M,
                                               StringRef Name) const {
  return M.getGlobalVariable(Name,
                             /*AllowInternal=*/true);
}

// Write the code coverage data structures into the testing file format.
//
// Header:
//   uint64_t Magic = 'llvmcovm';
//   uint64_t Version = 1;
// Names section: __llvm_prf_nm variable
//   LEB128 NamesLen;
//   LEB128 Address = 0;
//   char NamesData[NamesLen];
// Coverage mapping section: __llvm_coverage_mapping
//   LEB128 CoverageMappingLen;
//   (optional) Padding to 64-bit address
//   struct CoverageMapping initializer { { i32, i32, i32, i32 }, [n x i8] }
// (optional) Padding to 64-bit address
// Coverage records:
//   For each variable in the "__llvm_covfun" section
//     struct CoverageRec: <{ i64, i32, i64, i64, [m x i8] }>
//
bool SPIEmitterImpl::writeTempSPI(Module &M, raw_ostream &OS) {
  auto ByteSwap = [](uint64_t N) {
    return support::endian::byte_swap<uint64_t, endianness::little>(N);
  };

  // Verify all the variables and initializers to be written exist.
  GlobalVariable *ProfileNamesData =
      getNamedSymbol(M, getInstrProfNamesVarName());
  GlobalVariable *CovMappingData =
      getNamedSymbol(M, getCoverageMappingVarName());
  if (!ProfileNamesData || !CovMappingData) {
    LLVM_DEBUG(dbgs() << "Coverage variables not found\n");
    return false;
  }

  // Magic number for header that corresponds to the string "llvmcovm", in
  // little-endian representation.
  constexpr uint64_t TestingFormatMagic = 0x6d766f636d766c6c;
  constexpr uint64_t Version = 1;

  auto MagicLE = ByteSwap(TestingFormatMagic);
  OS.write(reinterpret_cast<char *>(&MagicLE), sizeof(MagicLE));

  // Output version field.
  auto VersionLE = ByteSwap(uint64_t(Version));
  OS.write(reinterpret_cast<char *>(&VersionLE), sizeof(VersionLE));

  // Output the Names section.
  const DataLayout &DL = M.getDataLayout();
  Type *NamesType = ProfileNamesData->getValueType();
  assert(ProfileNamesData->hasInitializer() && "Expected initialized variable");
  assert(NamesType->isArrayTy() && "Expecting array type");
  assert(DL.getTypeSizeInBits(NamesType->getArrayElementType()) == 8 &&
         "Expected i8 type");
  encodeULEB128(cast<ArrayType>(NamesType)->getArrayNumElements(), OS);
  encodeULEB128(0, OS);
  writeInitializer(DL, ProfileNamesData->getInitializer(), OS);

  // Output the Coverage mapping section.
  Type *CovMappingType = CovMappingData->getValueType();
  assert(CovMappingData->hasInitializer() && "Expected initialized variable");
  assert(CovMappingType->isStructTy() && "Expecting structure type");
  uint64_t CoverageMappingDataSize = DL.getTypeAllocSize(CovMappingType);
  encodeULEB128(CoverageMappingDataSize, OS);

  // Coverage mapping data is expected to have an alignment of 8.
  for (unsigned Pad = offsetToAlignment(OS.tell(), Align(8)); Pad; --Pad)
    OS.write(uint8_t(0));

  writeInitializer(DL, CovMappingData->getInitializer(), OS);

  // Coverage records data is expected to start with an alignment of 8.
  for (unsigned Pad = offsetToAlignment(OS.tell(), Align(8)); Pad; --Pad)
    OS.write(uint8_t(0));

  // Output each of the function records.
  for (auto &GV : M.globals()) {
    if (!GV.getSection().equals("__llvm_covfun"))
      continue;

    for (unsigned Pad = offsetToAlignment(OS.tell(), Align(GV.getAlignment()));
         Pad; --Pad)
      OS.write(uint8_t(0));
    writeInitializer(DL, GV.getInitializer(), OS);
  }

  return true;
}

void SPIEmitterImpl::writeInitializer(const DataLayout &DL, Constant *Init,
                                      raw_ostream &OS) {
  if (auto *CS = dyn_cast<ConstantStruct>(Init))
    writeStructInitializer(DL, CS, OS);
  else if (auto *CA = dyn_cast<ConstantDataArray>(Init))
    writeArrayInitializer(DL, CA, OS);
  else if (auto *CI = dyn_cast<ConstantInt>(Init))
    writeIntegerInitializer(DL, CI, OS);
  else
    llvm_unreachable("Unsupported initializer type encountered");
}

void SPIEmitterImpl::writeStructInitializer(const DataLayout &DL,
                                            ConstantStruct *CS,
                                            raw_ostream &OS) {
  unsigned Size = DL.getTypeAllocSize(CS->getType());
  const StructLayout *Layout = DL.getStructLayout(CS->getType());

  for (unsigned I = 0, E = CS->getNumOperands(); I != E; ++I) {
    Constant *Field = CS->getOperand(I);
    writeInitializer(DL, Field, OS);

    // Check if padding is needed and insert one or more 0s.
    uint64_t FieldSize = DL.getTypeAllocSize(Field->getType());
    uint64_t PadSize = ((I == E - 1 ? Size : Layout->getElementOffset(I + 1)) -
                        Layout->getElementOffset(I)) -
                       FieldSize;
    for (uint64_t PI = 0; PI < PadSize; ++PI) {
      OS.write('\0');
    }
  }
}

void SPIEmitterImpl::writeIntegerInitializer(const DataLayout &DL,
                                             ConstantInt *CI, raw_ostream &OS) {
  TypeSize ByteCount = DL.getTypeStoreSize(CI->getType());
  assert(ByteCount <= 8 && "Only up to i64 support");
  uint64_t Value = CI->getZExtValue();
  OS.write(reinterpret_cast<char *>(&Value), ByteCount);
}

void SPIEmitterImpl::writeArrayInitializer(const DataLayout &DL,
                                           ConstantDataArray *CA,
                                           raw_ostream &OS) {
  ArrayType *ArTy = CA->getType();
  uint64_t Length = ArTy->getNumElements();
  OS.write(CA->getRawDataValues().str().c_str(), Length);
}

} // namespace

PreservedAnalyses SPIEmitter::run(Module &M, ModuleAnalysisManager &AM) {
  if (!SPIGenerate)
    return PreservedAnalyses::all();

  SPIEmitterImpl Emitter;
  Emitter.run(M);
  return PreservedAnalyses::all();
}
