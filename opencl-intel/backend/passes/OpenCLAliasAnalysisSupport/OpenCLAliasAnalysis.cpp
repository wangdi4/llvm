#include "OpenCLAliasAnalysis.h"

using namespace llvm;

// Checks whether O was an actual parameter for a formal restrict parameter
bool OpenCLAliasAnalysis::isNoAliasArgument(const Value *O) {
  if (noAliasArgs.count((Value *)O)) 
    return true;
  return false;
}

// Check aliasing using the information from noAliasArgs and the fact that in openCL pointers from different memory addresses do not alias
// This code is based on rules taken from BasicAliasAnalysis
AliasAnalysis::AliasResult OpenCLAliasAnalysis::alias(const Location &LocA, const Location &LocB) {
  const Value *V1 = LocA.Ptr;
  const Value *V2 = LocB.Ptr;

  // Remove casts if exists
  V1 = V1->stripPointerCasts();
  V2 = V2->stripPointerCasts();

  if (V1->getType()->isPointerTy() && V2->getType()->isPointerTy()) {
    PointerType *V1P = cast<PointerType>(V1->getType());
    PointerType *V2P = cast<PointerType>(V2->getType());

    // If V1 and V2 are pointers to different address spaces then they do not alias.
    // This is not true in general, however, in openCL the memory addresses for private, local, and global are disjoint.
    if (V1P->getAddressSpace() != V2P->getAddressSpace())
      return NoAlias;
  }
  // try to identify to which object these pointers are pointing to
  const Value *O1 = GetUnderlyingObject(V1, TD);
  const Value *O2 = GetUnderlyingObject(V2, TD);

  if (O1 != O2) {
    // Check whether these object identified using several rules
    // isIdentifiedObject checks several rules that identifies a given object.
    // one of the rules checks whether the object is an argument and in case it is then return true 
    // if this argument is also marked as no alias.
    // The inlining after the wrapper remove the arguments together with their
    // corresponding no alias information. Therefore, we use function isNoAliasArgument
    // that returns true whether before the inlining the object was an actual parameter of an 
    // no alias formal parameter.
    bool isIdentifiedO1 = isIdentifiedObject(O1) || isNoAliasArgument(O1);
    bool isIdentifiedO2 = isIdentifiedObject(O2) || isNoAliasArgument(O2);

    // If both objects were identified then they do not alias
    if (isIdentifiedO1 && isIdentifiedO2)
      return NoAlias;
 
   // Constant pointers can't alias with non-const identified objects.
    if ((isa<Constant>(O1) && isIdentifiedO2 && !isa<Constant>(O2)) ||
        (isa<Constant>(O2) && isIdentifiedO1 && !isa<Constant>(O1)))
      return NoAlias;
  }

  return AliasAnalysis::alias(LocA, LocB);
}

// Extracts from the instruction's metadata the actual parameter for a NoAlias formal parameter that appeared at the wrapper's inlined function
// The metadata was inserted at the wrapper.
bool OpenCLAliasAnalysis::runOnModule(Module &M) {
  InitializeAliasAnalysis(this);
  for (Module::iterator mi = M.begin(), me = M.end(); mi != me; ++mi) {
    Function *F = mi;
    for (Function::iterator i = F->begin(), e = F->end(); i != e; ++i) {
      BasicBlock *blk = i;
      for (BasicBlock::iterator j = blk->begin(), e = blk->end(); j != e; ++j) {
        MDNode * mdNode = j->getMetadata("restrict");
        if (mdNode != 0) {
          Value * O = GetUnderlyingObject(j->stripPointerCasts(), TD);
          noAliasArgs.insert(O);
        }
      }
    }
  }
  return false;
}

char OpenCLAliasAnalysis::ID = 0;

 INITIALIZE_AG_PASS(OpenCLAliasAnalysis, AliasAnalysis, "opencl-aa",
                   "Alias Analysis for OpenCL",
                   false, true, false)

