//===- llvm/Bitcode/CSASaveRawBC.h - CSA Raw BC Interface -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// The CSASaveRawBC pass saves the IR for a module. It is intended to be as
// close to the Bitcode emitted by the -flto option as possible.
//
//===----------------------------------------------------------------------===//


namespace llvm {

// Declare routine to create the pass which will save a copy of the original
// IR. This is called from PassManagerBuilder.cpp to add the pass very early
// in the initialization sequence
ImmutablePass *createCSASaveRawBCPass();


class CSASaveRawBC : public ImmutablePass {
public:
  // The ID used to identify the pass. LLVM will actually use the address,
  // not the value, so it's just initialized to 0 in CSASaveRawBC.cpp
  static char ID;

  // The raw IR serialized as a BC file
  static std::string BcData;

  // Constructor
  CSASaveRawBC();

  // Initialization - Pretty much all that we have to work with. Called before
  // any module or function passes. Save away a copy of the raw (unoptimized)
  // IR so we have it if anybody wants it later
  bool doInitialization(Module &M) override;

  // Get the raw IR serialized as a BC file. Note that this is *NOT* a string!
  // It's a sequence of bytes, some of which may be 0.
  const std::string &getRawBC() const;

private:
  void dumpBC(StringRef modName);

}; // class CSASaveRawBC

}  // namespace llvm
