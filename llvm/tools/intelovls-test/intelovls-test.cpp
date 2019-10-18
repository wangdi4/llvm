//===- intelovls-test.cpp - Provides a test client for OptVLS
//-------------------===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===---------------------------------------------------------------------===//
//
// This file implements a test client of OptVLS to test the implementation of
// OptVLS.
// This gets compiled into an standalone executable called intelovls-test placed
// in to the
// bin directory. It can be run independently just like "opt".
// intelovls-test expects an input text file that contains a list of memrefs.
// Currently, it outputs only OVLSGroups.
// E.g. intelovls-test < inputfile -debug
// -debug will dump debug output.
// Input format:
// Rules:
// - comments start with ; and a space.
// - Input starts with a #, space and a group size
// - Dist: A relative distance from the base.
//   e.g. 1 A 0 i32 4 SLoad C 12
//        2 A 4 i32 4 SLoad C 12
//        3 A 12 i32 4 SLoad C 12, here the distance(12) is the distance from 1
// - VS(vector stride) needs to be set zero even when VSType is U
// - Element Type: {i|f}number of bits : i32, i64, f32, f64
// - Two indexed memrefs are adjacent if they have the matching MemrefId,
// IndexId and IndexType
//   -  <8 D 0 i64 2 ILoad P i64 2> and <10 D 0 i64 2 ILoad C i32 2> are not
//   adjacent
//   -  <8 D 0 i64 2 ILoad P i64 2> and <9 D 4 i64 2 ILoad P i64 2> are adjacent
//   -  <12 E 0 i32 4 ILoad Q i32 4> and <13 E 4 i32 2 ILoad Q i32 2> are not
//   adjacent since they
//      don't have the same index types.
//
// - Two strided memrefs are adjacent if they have the matching MemrefId
//    - For constant vector strides, two memrefs are adjacent if they have the
//    same constant vector-stride value
//      - C as VsId representes constant vector stride, therefore, C can not be
//      used as variable vector stride.
//      - <1 A 0 i32 4 SLoad C 3> and <2 A 4 i32 4 SLoad C 3> are adjacent
//    - For variable vector strides, two memrefs are adjacent if they have the
//    same VSId
//      - <1 A 0 i32 4 SLoad I 0> and <7 A 4 i64 2 SStore J 0> are not adjacent
//      - <1 A 0 i32 4 SLoad I 0> and <2 A 4 i32 4 SLoad I 0> are adjacent
//
// Format:
//
// Strided Memref Specifier:
// Id(int) MemrefId(Char) Dist(in bytes) Type:<ElementType X ElementCount>
// AccessType VsId{C-constant} VS(#bytes)
// e.g. <1 A 0 i32 4 SLoad C 40>
//
// Indexed Memref Specifier:
// Id(int) MemrefId(Char) Dist(in bytes) Type:<ElementType X ElementCount>
// Accesstype IndexId IndexType:<ElementType X ElementCount>
// e.g. <8 D 0 i64 2 ILoad P i64 2>
//
//===---------------------------------------------------------------------===//

#include "intelovls-test.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

// Two memrefs are considered to have same index vectors if they
// have the same index-id and same index-type.
bool ClientMemref::haveSameIndexVector(const ClientMemref &Mrf) {
  if (this->getIndexId() == Mrf.getIndexId() &&
      this->IndexType == Mrf.IndexType) {
    return true;
  }
  return false;
}

bool ClientMemref::haveSameVectorStride(const ClientMemref &Mrf) {
  if (this->hasConstVStride()) {
    if (Mrf.hasConstVStride() &&
        this->getConstVectorStride() == Mrf.getConstVectorStride())
      return true;
    return false;
  } else if (Mrf.hasVarVStride() &&
             this->getVarVectorStride() == Mrf.getVarVectorStride())
    return true;
  return false;
}

// Mrf is at a contant distance from this if
//   - they have the same memref id, and same access type
//   - for indexed accesses, they have to have the same index vectors or,
//   - for strided accesses, they have to have the same vector strides
Optional<int64_t> ClientMemref::getConstDistanceFrom(const OVLSMemref &Mrf) {
  assert(isa<ClientMemref>(&Mrf) && "Expected ClientMemref!!!");
  const ClientMemref *CLMrf = cast<const ClientMemref>(&Mrf);
  if ((MId == CLMrf->getMemrefId()) &&                // have same memref id
      this->getAccessKind() == Mrf.getAccessKind() && // have same access type
      // Indexed accesses have matching index-vectors.
      ((this->getAccessKind().isIndexed() &&
        haveSameIndexVector(*CLMrf)) ||
       // Strided accesses have matching vector strides.
       (this->getAccessKind().isStrided() &&
        haveSameVectorStride(*CLMrf)))) {
    return Dist - (CLMrf->getDistance());
  }
  return None;
}

namespace OVLSTest {
static OVLSContext Context;

static void parseCommandLineOptions(int argc, char **argv,
                                    const char *Banner = nullptr) {
  cl::ParseCommandLineOptions(argc, argv, Banner);
}

OVLSContext &getContext() { return Context; }

static OVLSAccessKind getAccessKind(const std::string &AccKind) {
  if (AccKind.compare("SLoad") == 0)
    return OVLSAccessKind::SLoad;
  else if (AccKind.compare("SStore") == 0)
    return OVLSAccessKind::SStore;
  else if (AccKind.compare("ILoad") == 0)
    return OVLSAccessKind::ILoad;
  else if (AccKind.compare("IStore") == 0)
    return OVLSAccessKind::IStore;
  return OVLSAccessKind::Unknown;
}

static Type *getScalarType(const std::string ST) {
  OVLSContext &Context = getContext();

  if (ST.compare("i8") == 0 || ST.compare("si8") == 0)
    return Type::getInt8Ty(Context);
  else if (ST.compare("i16") == 0 || ST.compare("si16") == 0)
    return Type::getInt16Ty(Context);
  else if (ST.compare("i32") == 0 || ST.compare("si32") == 0)
    return Type::getInt32Ty(Context);
  else if (ST.compare("i64") == 0 || ST.compare("si64") == 0)
    return Type::getInt64Ty(Context);
  else if (ST.compare("f32") == 0)
    return Type::getFloatTy(Context);
  else if (ST.compare("f64") == 0)
    return Type::getDoubleTy(Context);

  return nullptr;
}

static void parseInput(unsigned &GroupSize, OVLSMemrefVector &Mrfs) {

  char MemrefId, InputChar;
  int Dist;
  string SAccKind, SElemType, line;
  unsigned Id, NumElements;
  bool InputStarts = false;

  while (!std::cin.eof()) {
    if (!InputStarts) {
      std::cin >> InputChar;

      if (InputChar == ';') {
        // Ignore comments in the input file.
        getline(std::cin, line);
        continue;
      } else if (InputChar == '#') {
        // Get the Group Size
        std::cin >> GroupSize;
        assert(GroupSize > 0 && GroupSize <= 64 &&
               "Group Size above 64 bytes is not supported currently!!");
        getline(std::cin, line);
        InputStarts = true;
        continue;
      }
    }

    std::cin >> Id >> MemrefId >> Dist >> SElemType >> NumElements >> SAccKind;

    OVLSAccessKind AccKind = getAccessKind(SAccKind);
    assert(AccKind != OVLSAccessKind::Unknown && "Invalid Access Type!!!");

    Type *ElemType = getScalarType(SElemType);

    OVLSMemref *mrf = NULL;

    if (AccKind.isIndexed()) {
      char IndexId;
      string SIdxElemType;
      unsigned IdxNumElems;
      std::cin >> IndexId >> SIdxElemType >> IdxNumElems;
      assert(IdxNumElems == NumElements &&
             "IdxNumElems needs to match the NumElements!");
      VectorType *IdxType =
          VectorType::get(getScalarType(SIdxElemType), IdxNumElems);
      mrf = new ClientMemref(MemrefId, Dist, ElemType, NumElements, AccKind,
                             IndexId, IdxType);
    } else {
      char VsId;
      int VStride;
      std::cin >> VsId;
      std::cin >> VStride;
      if (VsId == 'C') // memref has a constant vector stride
        mrf = new ClientMemref(MemrefId, Dist, ElemType, NumElements, AccKind,
                               true, VStride);
      else
        mrf = new ClientMemref(MemrefId, Dist, ElemType, NumElements, AccKind,
                               false, VsId);
    }
    Mrfs.push_back(mrf);
  }
}

std::unique_ptr<TargetMachine> createTargetMachine() {
  Triple TargetTriple("x86_64-pc-linux");
  std::string Error;
  const Target *T = TargetRegistry::lookupTarget("x86-64", TargetTriple, Error);
  if (!T)
    return nullptr;

  TargetOptions Options;
  return std::unique_ptr<TargetMachine>(T->createTargetMachine(
      TargetTriple.getTriple(), "core-avx-i", "", Options, None,
      None, CodeGenOpt::Aggressive));
}

Function *createFunctionDecl(FunctionType *FTy, StringRef Name, Module *M) {
  return Function::Create(FTy, GlobalValue::ExternalLinkage, Name, M);
}

} // end of namespace OVLSTest

//
// main() for intelovls-test
//
int main(int argc, char **argv) {
  std::unique_ptr<TargetMachine> TM = nullptr;

  // Parse command line options.
  OVLSTest::parseCommandLineOptions(argc, argv, "IntelOptVLS test client\n");

  unsigned GroupSize = 0;

  // parse Input File
  OVLSMemrefVector Mrfs;
  OVLSTest::parseInput(GroupSize, Mrfs);

  OVLSGroupVector Grps;
  OptVLSInterface::getGroups(Mrfs, Grps, GroupSize);

#ifdef OVLSTESTCLIENT
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();

  TM = OVLSTest::createTargetMachine();
#endif

  if (TM) {
    // Create a dummy module.
    Module *M = new Module("OptVLS", OVLSTest::Context);
    // Create a dummy function declaration.
    const Function *BarImpl =
        OVLSTest::createFunctionDecl(
              FunctionType::get(Type::getInt32Ty(OVLSTest::Context), {}, false),
              "client_test", M);
    FunctionAnalysisManager DummyFAM;
    TargetTransformInfo TTI = TM->getTargetIRAnalysis().run(*BarImpl, DummyFAM);
    OVLSCostModel CM(TTI, OVLSTest::getContext());

    // Do something with the grps.
    for (OVLSGroup *Grp : Grps) {
      OVLSInstructionVector InstVec;
      if (OptVLSInterface::getSequence(*Grp, CM, InstVec)) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
        for (OVLSInstruction *I : InstVec)
          OVLSdbgs() << *I << "\n";
#endif
      }
    }
  }

  // Release memory
  for (OVLSMemref *Memref : Mrfs) {
    delete Memref;
  }

  for (OVLSGroup *Grp : Grps) {
    delete Grp;
  }

  return 0;
}
