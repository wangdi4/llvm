#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/LLVMContext.h"
#include <iostream>

using namespace llvm;
using namespace std;

void runOnFunction(Function &F);	
void createNode(std::string * vec_funcs, std::string * types);	




int main(int argc, char ** argv)
{
	if (argc != 2)
	{
		cout << "Input error. Should be:\n./ParseFuncs.a <BitcodeFileName>\n";
		return -1;
	}
	// Load the input LLVM file
	static MemoryBuffer* buf = MemoryBuffer::getFile(argv[1]);	
	if (buf == NULL)
	{
		cout << "Input file error. Aborting.\n";
		return -1;
	}
    Module *mod = ParseBitcodeFile(buf, getGlobalContext());
	
	// Iterate over functions and invoke parser
    cout << "\n\n";
	for ( Module::iterator I = mod->begin(), E = mod->end(); I != E; ++I)
	{
		Function * nextFunc = &*I;
		if (nextFunc->isDeclaration())
			continue;
		runOnFunction(*nextFunc);
	}	
	return 0;
}


void runOnFunction(Function &F) 
{
	unsigned vec_num = 0;
	
	// Disassemble function name to create prototype. 
	std::string funcNameProto = F.getName();
	size_t barrier[5];
	std::string types[4], num_funcs_str;
	barrier[0] = funcNameProto.find_first_of('_', 1);
	barrier[1] = funcNameProto.find_first_of('_', barrier[0]+1);
	barrier[2] = funcNameProto.find_first_of('_', barrier[1]+1);
	barrier[3] = funcNameProto.find_first_of('_', barrier[2]+1);
	barrier[4] = funcNameProto.find_first_of('_', barrier[3]+1);
	types[0] = funcNameProto.substr(1, barrier[0]-1);
	types[1] = funcNameProto.substr(barrier[0]+1, barrier[1]-barrier[0]-1);
	types[2] = funcNameProto.substr(barrier[1]+1, barrier[2]-barrier[1]-1);
	types[3] = funcNameProto.substr(barrier[2]+1, barrier[3]-barrier[2]-1);
	num_funcs_str = funcNameProto.substr(barrier[3]+1, barrier[4]-barrier[3]-1);

	unsigned num_funcs = atoi(num_funcs_str.c_str());
	// Extract names of LLVM funcs
	std::string vec_funcs[10]; // only need 6, but get_global_id is the 7th, and the rest just in case..
	for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
	{
		if (CallInst * CI = dyn_cast<CallInst>(&*I))
		{
			// Add function name to list
			vec_funcs[vec_num++] = CI->getCalledFunction()->getName();
		}
	}
	
	// Collected num_funcs function values. Create a node for them, and associate to all relevant hashes 
	createNode(vec_funcs, types);

}


void createNode(std::string * vec_funcs, std::string * types)
{
    unsigned not_static_flag = 1;
	if ((types[0] == "STATIC") || (types[1] == "STATIC") ||
        (types[2] == "STATIC") || (types[3] == "STATIC")) {
            not_static_flag = 0;
    }

	cout << "  {{";
	cout << "\"" << vec_funcs[0] << "\",";
	cout << "\"" << vec_funcs[1] << "\",";
	cout << "\"" << vec_funcs[2] << "\",";
	cout << "\"" << vec_funcs[3] << "\",";
	cout << "\"" << vec_funcs[4] << "\",";
	cout << "\"" << vec_funcs[5] << "\"},0," << not_static_flag << "},\n";
}


