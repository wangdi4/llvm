#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Pass.h"
#include "llvm/Type.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Constants.h"
#include "llvm/LLVMContext.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/DerivedTypes.h"
#include <iostream>
#include <fstream>
using namespace llvm;

#define V_ASSERT(x) if (!(x)) {llvm::cerr << "Asserion in line " << __LINE__ << "\n";  exit(-1);}
#define VECTORIZER_HASH_SIZE 64
#define SCALARIZER_HASH_SIZE VECTORIZER_HASH_SIZE * 4
#define DEBUG_PRINT 1
#define MAX_FUNCS 6


SmallVector<unsigned, 16> scalarizer_hash[SCALARIZER_HASH_SIZE];	
SmallVector<unsigned, 16> vectorizer_hash[VECTORIZER_HASH_SIZE];	
#define cpp_file std::cout
//std::ofstream cpp_file;

class ParseFuncs {
public:
	ParseFuncs();
	~ParseFuncs();
	void runOnFunction(Function &F);	
private:
	unsigned running_index;
	void createEntry(unsigned index, std::string * vec_funcs, std::string * types);	
};


bool firstLine = true;

int main(int argc, char ** argv)
{
	if (argc != 2)
	{
		std::cout << "Input error. Should be:\n./ParseFuncs.a <BitcodeFileName>\n";
		return -1;
	}
	// Load the input LLVM file
	static MemoryBuffer* buf = MemoryBuffer::getFile(argv[1]);	
	if (buf == NULL)
	{
		std::cout << "Input file error. Aborting.\n";
		return -1;
	}
	LLVMContext &context = getGlobalContext();
	Module *mod = ParseBitcodeFile(buf, context);

	// Create parser
	ParseFuncs * parser = new ParseFuncs();

	// Iterate over functions and invoke parser
	for ( Module::iterator I = mod->begin(), E = mod->end(); I != E; ++I)
	{
		Function * nextFunc = &*I;
		if (nextFunc->isDeclaration())
			continue;
		parser->runOnFunction(*nextFunc);
	}

	// Delete parser
	delete parser;
	
	
	return 0;
}




ParseFuncs::~ParseFuncs()
{
//	cpp_file.close();
}

ParseFuncs::ParseFuncs()
{
//	cpp_file.open("auto_funcs.h");		
	running_index = 0;
}





void ParseFuncs::runOnFunction(Function &F) 
{
	unsigned vec_num = 0;
	running_index++;

	// Disassemble function name to create prototype. Func names are defined as: P0_P1_P2_P3_...
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
	std::string vec_funcs[MAX_FUNCS];
	for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
	{
		if (CallInst * CI = dyn_cast<CallInst>(&*I))
		{
			// Add function name to list
			vec_funcs[vec_num++] = CI->getCalledFunction()->getName();
			if (vec_num == num_funcs) break;
		}
	}
	if (vec_num == 5)
	{
		vec_funcs[5] = "_";
	}
	
	// add new funcs to list
	createEntry(running_index, vec_funcs, types);
}


void ParseFuncs::createEntry(unsigned index, std::string * vec_funcs, std::string * types)
{
	if (firstLine)
	{
		firstLine = false;
		cpp_file << " {";
	}
	else
	{
		cpp_file << ",{";
	}
	cpp_file << "\"" << types[0] << "\",";
	cpp_file << "\"" << types[1] << "\",";
	cpp_file << "\"" << types[2] << "\",";
	cpp_file << "\"" << types[3] << "\",";
	
	cpp_file << "\"" << vec_funcs[0] << "\",";
	cpp_file << "\"" << vec_funcs[1] << "\",";
	cpp_file << "\"" << vec_funcs[2] << "\",";
	cpp_file << "\"" << vec_funcs[3] << "\",";
	cpp_file << "\"" << vec_funcs[4] << "\",";
	cpp_file << "\"" << vec_funcs[5] << "\"}\n";
}


	
	
#if 0	
	// Collected num_funcs function values. Create a node for them, and associate to all relevant hashes 
	createNode(running_index, vec_funcs, types);

	// Add new functions list to hash table/s. If the function prototype includes a "STATIC"
	// portion (means one of the inputs remains scalar for all the variations), this function
	// list is an alternative which will not be required by the vectorizer - only the scalarizer

	if ((types[0] != "STATIC") && (types[1] != "STATIC") && (types[2] != "STATIC") && (types[3] != "STATIC"))
	{
		// Adding to vectorizer hash - place in single hash location - according to scalar function name
		unsigned hash_val1 = calculate_hash(vec_funcs[0], false);
		vectorizer_hash[hash_val1].push_back(running_index);
	}
	
	// Adding to scalarizer hash - place in several hash entries, according to all vectorized functions
	unsigned hash_val2 = calculate_hash(vec_funcs[1], true);
	unsigned hash_val4 = calculate_hash(vec_funcs[2], true);
	unsigned hash_val8 = calculate_hash(vec_funcs[3], true);
	unsigned hash_val16 = calculate_hash(vec_funcs[4], true);	
	scalarizer_hash[hash_val2].push_back(running_index);
	if (hash_val4 != hash_val2)
		scalarizer_hash[hash_val4].push_back(running_index);
	if ((hash_val8 != hash_val4) && (hash_val8 != hash_val2))
		scalarizer_hash[hash_val8].push_back(running_index);
	if ((hash_val16 != hash_val8) && (hash_val16 != hash_val4) && (hash_val16 != hash_val2))
		scalarizer_hash[hash_val16].push_back(running_index);
	if (num_funcs == 6)
	{
		unsigned hash_val3 = calculate_hash(vec_funcs[5], true);
		if ((hash_val3 != hash_val16) && (hash_val16 != hash_val8) && (hash_val16 != hash_val4) && (hash_val16 != hash_val2))
			scalarizer_hash[hash_val3].push_back(running_index);
		
	}
	
	
	
}



unsigned ParseFuncs::calculate_hash(std::string funcName, bool isScalarizerHash)
{	
	unsigned hashSize = isScalarizerHash ? SCALARIZER_HASH_SIZE : VECTORIZER_HASH_SIZE;
	size_t lngth = funcName.size();
	
	// Modified Bernstein hash function.
	unsigned Result = 0;
	for (unsigned s = 0; s < lngth; s++)
	{
		Result = Result * 33 ^ funcName.at(s);
	}
	Result = Result + (Result >> 5);
	return ( (Result & ((hashSize-1) << 3)) >> 3);
}



void ParseFuncs::createNodePrefix()
{
	cpp_file << "VFH::hashEntry __f__node[] {\n";
	isFirstNode = true;
}

void ParseFuncs::createNodePostfix()
{
	cpp_file << "};\n";
}

void ParseFuncs::createNode(unsigned index, std::string * vec_funcs, std::string * types)
{
	if (isFirstNode)
	{
		cpp_file << ",";
		isFirstNode = false;
	}
	
	cpp_file << "/* node:" << index << " */\t = {{";
	
	cpp_file << "VFH::T_" << types[0] << ",";
	cpp_file << "VFH::T_" << types[1] << ",";
	cpp_file << "VFH::T_" << types[2] << ",";
	cpp_file << "VFH::T_" << types[3] << "},{";

	cpp_file << "\"" << vec_funcs[0] << "\",";
	cpp_file << "\"" << vec_funcs[1] << "\",";
	cpp_file << "\"" << vec_funcs[2] << "\",";
	cpp_file << "\"" << vec_funcs[3] << "\",";
	cpp_file << "\"" << vec_funcs[4] << "\",";
	cpp_file << "\"" << vec_funcs[5] << "\"}}\n";
}


void ParseFuncs::dumpHashFile()
{		
	unsigned longest = 0;
	// Scalarizer hash
	cpp_file << "\n/*** Scalarizer hash ***/\n";
	//	Dump buckets
	for (unsigned i = 0; i< SCALARIZER_HASH_SIZE; i++)
	{
		cpp_file << "VFH::hashEntry* __f__slist" << i << "[] = {";
		unsigned size = (scalarizer_hash[i]).size();
		if (size > longest) longest = size;
		for (unsigned j = 0; j < size; j++)
			cpp_file << "&__f__node" << (scalarizer_hash[i])[j] << ",";
		cpp_file << "NULL};\n";		
	}		
	//  Dump hash master
	cpp_file << "\nVFH::hashEntry** __f__scalarizer_hash_table[] = {";
	for (unsigned i = 0; i< SCALARIZER_HASH_SIZE; i++)
	{
		cpp_file << "__f__slist" << i;
			if (i < SCALARIZER_HASH_SIZE-1) 
				cpp_file << ",";
	}
	cpp_file << "};\n//longest bucket: " << longest << "\n\n";
	
	// Vectorizer hash
	longest = 0;
	cpp_file << "\n/*** Vectorizer hash ***/\n";
	//	Dump buckets
	for (unsigned i = 0; i< VECTORIZER_HASH_SIZE; i++)
	{
		cpp_file << "VFH::hashEntry* __f__vlist" << i << "[] = {";
		unsigned size = (vectorizer_hash[i]).size();
		if (size > longest) longest = size;
		for (unsigned j = 0; j < size; j++)
			cpp_file << "&__f__node" << (vectorizer_hash[i])[j] << ",";
		cpp_file << "NULL};\n";		
	}		
	//  Dump hash master
	cpp_file << "\nVFH::hashEntry** __f__vectorizer_hash_table[] = {";
	for (unsigned i = 0; i< VECTORIZER_HASH_SIZE; i++)
	{
		cpp_file << "__f__vlist" << i;
		if (i < VECTORIZER_HASH_SIZE-1) 
			cpp_file << ",";
	}
	cpp_file << "};\n//longest bucket: " << longest << "\n\n";
}


void ParseFuncs::createPrefix()
{
	cpp_file 
	<< "/*** GENERATED FILE - DO NOT MODIFY ****/\n"
	<<"\n"
	<<"\n#include \"functions.h\""
	<<"\n"
	<<"\n#define VECTORIZER_HASH_SIZE " << VECTORIZER_HASH_SIZE
	<<"\n#define SCALARIZER_HASH_SIZE " << SCALARIZER_HASH_SIZE
	<<"\n\n";
}



void ParseFuncs::createPostfix()
{
	
	cpp_file
	<<"\nstatic unsigned calculate_hash(std::string funcName, bool isScalarizerHash)"
	<<"\n{"
	<<"\n	// Modified Bernstein hash function."
	<<"\n	unsigned hashSize = isScalarizerHash ? SCALARIZER_HASH_SIZE : VECTORIZER_HASH_SIZE;"
	<<"\n	size_t lngth = funcName.size();"
	<<"\n	unsigned Result = 0;"
	<<"\n	for (unsigned s = 0; s < lngth; s++)"
	<<"\n	{"
	<<"\n		Result = Result * 33 ^ funcName.at(s);"
	<<"\n	}"
	<<"\n	Result = Result + (Result >> 5);"
	<<"\n	return ( (Result & ((hashSize-1) << 3)) >> 3);"
	<<"\n}"
	<<"\n"	
	<<"\n"	
	<<"\nVFH::hashEntry * VFH::findScalarFunctionInHash(std::string &inp_name)"
	<<"\n{"
	<<"\n	unsigned hash_key = calculate_hash(inp_name, false);"
	<<"\n	if (hash_key >= VECTORIZER_HASH_SIZE) return NULL;"
	<<"\n"
	<<"\n	hashEntry ** nodes_list = __f__vectorizer_hash_table[hash_key];"
	<<"\n"
	<<"\n	hashEntry * current = nodes_list[0];"
	<<"\n	unsigned i = 0;"
	<<"\n	while (current != NULL)"
	<<"\n	{"
	<<"\n		// Check if current function fits the requested name"
	<<"\n		if (current->funcs[0] == inp_name)"
	<<"\n			return current;"
	<<"\n		// if not - move along to next function"
	<<"\n		current = nodes_list[++i];"
	<<"\n	}"
	<<"\n	return NULL; // failed to find function"
	<<"\n}"
	<<"\n"
	<<"\nstatic unsigned orderToWidth[] = {1, 2, 4, 8, 16, 3};"
	<<"\nVFH::hashEntry * VFH::findVectorFunctionInHash(std::string &inp_name, unsigned * vecWidth)"
	<<"\n{"
	<<"\n	unsigned hash_key = calculate_hash(inp_name, true);"
	<<"\n	if (hash_key >= SCALARIZER_HASH_SIZE) return NULL;"
	<<"\n"
	<<"\n	hashEntry ** nodes_list = __f__scalarizer_hash_table[hash_key];"
	<<"\n"
	<<"\n	hashEntry * current = nodes_list[0];"
	<<"\n	unsigned i = 0;"
	<<"\n	while (current != NULL)"
	<<"\n	{"
	<<"\n		// Check if current function fits the requested name"
	<<"\n		for (unsigned j = 1; j < 6; j++)"
	<<"\n		{"
	<<"\n			if (current->funcs[j] == inp_name)"
	<<"\n			{"
	<<"\n				*vecWidth = orderToWidth[j];"
	<<"\n				return current;"
	<<"\n			}"
	<<"\n		}"
	<<"\n		// if not - move along to next function"
	<<"\n		current = nodes_list[++i];"
	<<"\n	}"
	<<"\n	return NULL; // failed to find function"
	<<"\n}"
	<<"\n"	
	<<"// amount of entries: " << running_index << "\n";	
}
#endif
