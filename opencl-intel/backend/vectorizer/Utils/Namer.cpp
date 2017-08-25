/*********************************************************************************************
 * Copyright ? 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "Namer.h"
#include "OCLPassSupport.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"

#include <string>
#include <sstream>
#include <algorithm>
#include <map>

using namespace llvm;

static cl::opt<unsigned>
NameInstDepth("name-depth", cl::init(0), cl::Hidden,
  cl::desc("depth for naming according to ops"));

namespace intel {

char nameByInstType::ID = 0;

OCL_INITIALIZE_PASS(nameByInstType, "nameByInstType", "add names for Values according to instruction and operands type", false, false)

//@brief Small Pass that remove names from all values in the function.
class nameRemove : public FunctionPass {
public:

  static char ID;
  /// @brief C'tor
  nameRemove(): FunctionPass(ID){}
  /// @brief D'tor
  ~nameRemove(){}
  /// @brief Provides name of pass
  virtual llvm::StringRef getPassName() const {
    return "nameRemove";
  }

  virtual bool runOnFunction(Function &F) {
    for (Function::arg_iterator argIt = F.arg_begin(), argE = F.arg_end();
         argIt != argE; ++argIt) {
      argIt->setName("");
    }
    for (Function::iterator bbit = F.begin(), bbe = F.end(); bbit != bbe; ++bbit) {
      bbit->setName("");
      for (BasicBlock::iterator I = bbit->begin(), E = bbit->end(); I!=E; ++I){
        I->setName("");
      }
    }
    return true;
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
  };
};



std::string toString(std::string str, unsigned n) {
  std::stringstream  mystream;
  mystream << str << n;
  return mystream.str();
}


void concatTypesRec(std::stringstream& sstr, unsigned depth, unsigned dest_depth, Value *v) {
  if (depth >= dest_depth) return;
  sstr << "." << v->getType()->getTypeID();
  Instruction *I = dyn_cast<Instruction>(v);
  if (!I) return;
  std::string name (I->getOpcodeName());
  sstr << "." << name;
  for (unsigned i=0; i<I->getNumOperands(); ++i) {
    concatTypesRec(sstr, depth +1, dest_depth, I->getOperand(i));
  }
}


std::string getInstructionName(Instruction *I) {
  std::stringstream sstr;
  sstr << I->getOpcodeName();
  concatTypesRec(sstr, 0, NameInstDepth, I);
  sstr << ".";
  std::string str = sstr.str();
  replace(str.begin(), str.end(), ' ', '_');
  return str;
}

bool nameByInstType::runOnFunction(Function &F) {
  RenameValues(F);
  return true;
}

void nameByInstType::RenameValues(Function &F) {
  // Incase name all flag is set first remove all names from the Function,
  // this will makes the following procedures rename all values.
  if (m_nameAll) {
    nameRemove remPass;
    remPass.runOnFunction(F);
  }

  unsigned argInd = 0;
  for (Function::arg_iterator argIt = F.arg_begin(), argE = F.arg_end(); argIt != argE; ++argIt) {
    if (argIt->getName() == "") {
      argIt->setName(toString("arg", argInd));
    }
    ++argInd;
  }

  std::map<std::string, unsigned> name_map;
  unsigned bbInd = 0;
  for (Function::iterator bbit = F.begin(), bbe = F.end(); bbit != bbe; ++bbit) {
    if(bbit->getName() == "") bbit->setName(toString("BB",bbInd++));
    for (BasicBlock::iterator IIt = bbit->begin(), E = bbit->end(); IIt!=E; ++IIt){
      Instruction* I = &*IIt;
      if (I->getType()->isVoidTy()) continue;
      if (I->getName() != "") continue;

      std::string str = getInstructionName(I);
      unsigned index = 0;
      if (name_map.count(str)) {
        index = name_map[str] + 1;
      }
      name_map[str] = index;
      I->setName(toString(str ,index));
    }
  }
}


} // namespace intel


char intel::nameRemove::ID = 0;
extern "C" {
  FunctionPass* createNameRemovePass() {
    return new intel::nameRemove();
  }
}
//static RegisterPass<intel::nameRemove> nameRemove("nameRemove", "remove names from Values");


extern "C" {
  FunctionPass* createNameByInstTypePass() {
    return new intel::nameByInstType();
  }
}

