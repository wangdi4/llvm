#include "llvm/Pass.h"
#include "llvm/Type.h"
#include "llvm/Module.h"
#include "llvm/Instructions.h"
#include "llvm/Constants.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

#include <stdio.h>
#include <string>
#include <sstream>
using namespace std;
FILE * prtFile;
#define V_INIT_PRINT prtFile = fopen("/tmp/vectorizer.txt", "a");
#define V_DESTROY_PRINT	fclose(prtFile);
#define V_DUMP(ptr)				\
{								\
	string tmpStr;				\
	llvm::raw_string_ostream strstr(tmpStr);	\
	ptr->print(strstr, NULL);		\
	fprintf(prtFile, "%s", strstr.str().c_str());\
	fflush(prtFile);\
}

namespace llvm {
  class Function;
}

using namespace llvm;

class Vectorizer;


class Vectorizer : public ModulePass {
public:
	static char ID;
	Vectorizer() : ModulePass((intptr_t)&ID) 
	{
		V_INIT_PRINT;
	}
	~Vectorizer() 
	{
		V_DESTROY_PRINT;
	}
	virtual bool runOnModule(Module &M);
};

// Register the pass
char Vectorizer::ID = 0;
RegisterPass<Vectorizer> X("Vectorizer", "Vectorizer Pass");



bool Vectorizer::runOnModule(Module &M)
{	
	V_DUMP((&M));
	return false;
}	

 

extern "C" Pass *createVectorizerPass()
{	
	return new Vectorizer();	
}

extern "C" int getVectorizerFunctions(Vectorizer *V, SmallVectorImpl<Function*> &Functions)
{
	return 0;
}

extern "C" int getVectorizerWidths(Vectorizer *V, SmallVectorImpl<int> &MaxWidths, SmallVectorImpl<int> &MinWidths)
{
	return 0;
}


