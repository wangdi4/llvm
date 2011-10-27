#include <iostream>
#include <fstream>
#include <vector>

#define VECTORIZER_HASH_SIZE 64
#define SCALARIZER_HASH_SIZE VECTORIZER_HASH_SIZE * 4
#define DEBUG_PRINT 1
using namespace std;


vector<unsigned> scalarizer_hash[SCALARIZER_HASH_SIZE];	
vector<unsigned> vectorizer_hash[VECTORIZER_HASH_SIZE];	


typedef struct inputNode
{
	string type0;
	string type1;
	string type2;
	string type3;
	string func_v1;
	string func_v2;
	string func_v4;
	string func_v8;
	string func_v16;
	string func_v3;
} inputNode;

inputNode allFuncs[] = {
#include "../tmp/funcs.list"
	,{"","","","","","","","","",""}
};

unsigned calculateHash(string &funcName, bool isScalarizerHash)
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

// Returns amount of functions
unsigned gatherHashes()
{
	unsigned i = 0;
	while (allFuncs[i].type0 != "")
	{

		// Add new functions list to hash table/s. If the function prototype includes a "STATIC"
		// portion (means one of the inputs remains scalar for all the variations), this function
		// list is an alternative which will not be required by the vectorizer - only the scalarizer
		
		if ((allFuncs[i].type0 != "STATIC") && (allFuncs[i].type1 != "STATIC") && 
			(allFuncs[i].type2 != "STATIC") && (allFuncs[i].type3 != "STATIC"))
		{
			// Adding to vectorizer hash - place in single hash location - according to scalar function name
			unsigned hash_val1 = calculateHash(allFuncs[i].func_v1, false);
			vectorizer_hash[hash_val1].push_back(i);
		}
		
		// Adding to scalarizer hash - place in several hash entries, according to all vectorized functions
		unsigned hash_val2 = calculateHash(allFuncs[i].func_v2, true);
		unsigned hash_val4 = calculateHash(allFuncs[i].func_v4, true);
		unsigned hash_val8 = calculateHash(allFuncs[i].func_v8, true);
		unsigned hash_val16 = calculateHash(allFuncs[i].func_v16, true);	
		scalarizer_hash[hash_val2].push_back(i);
		if (hash_val4 != hash_val2)
			scalarizer_hash[hash_val4].push_back(i);
		if ((hash_val8 != hash_val4) && (hash_val8 != hash_val2))
			scalarizer_hash[hash_val8].push_back(i);
		if ((hash_val16 != hash_val8) && (hash_val16 != hash_val4) && (hash_val16 != hash_val2))
			scalarizer_hash[hash_val16].push_back(i);
		if (allFuncs[i].func_v3 != "_")
		{
			unsigned hash_val3 = calculateHash(allFuncs[i].func_v3, true);
			if ((hash_val3 != hash_val16) && (hash_val3 != hash_val8) && (hash_val3 != hash_val4) && (hash_val3 != hash_val2))
				scalarizer_hash[hash_val3].push_back(i);
		}
		
		i++;
	}
	return i;
}


void dumpPrefix()
{
	cout 
	<<"/*********************************************************************************************\n"
	<<" * Copyright © 2010, Intel Corporation\n"
	<<" * Subject to the terms and conditions of the Master Development License\n"
	<<" * Agreement between Intel and Apple dated August 26, 2005; under the Intel\n"
	<<" * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744\n"
	<<" *********************************************************************************************/\n"
	<<"\n"
	<<"\n#include \"functions.h\""
	<<"\n"
	<<"\n#define VECTORIZER_HASH_SIZE " << VECTORIZER_HASH_SIZE
	<<"\n#define SCALARIZER_HASH_SIZE " << SCALARIZER_HASH_SIZE
	<<"\n\n";
}

void dumpNodes(unsigned num_funcs)
{
	cout << "// List of hash entries...\n";
	cout << "VFH::hashEntry __f__node[" << num_funcs << "] = {\n";
	for (unsigned i = 0; i < num_funcs; ++i)
	{
		cout << "/*[" << i << "]*/ {";
		cout << "VFH::T_" << allFuncs[i].type0 << ",";
		cout << "VFH::T_" << allFuncs[i].type1 << ",";
		cout << "VFH::T_" << allFuncs[i].type2 << ",";
		cout << "VFH::T_" << allFuncs[i].type3 << ",";
		cout << "\"" << allFuncs[i].func_v1 << "\",";
		cout << "\"" << allFuncs[i].func_v2 << "\",";
		cout << "\"" << allFuncs[i].func_v4 << "\",";
		cout << "\"" << allFuncs[i].func_v8 << "\",";
		cout << "\"" << allFuncs[i].func_v16 << "\",";
		cout << "\"" << allFuncs[i].func_v3 << "\"}";
		if (i+1 < num_funcs) cout << ",";
		cout << "\n";
	}
	cout << "};\n\n";
}


void dumpHashes()
{		
	unsigned longest = 0;
	// Scalarizer hash
	cout << "\n/*** Scalarizer hash ***/\n";
	//	Dump buckets
	for (unsigned i = 0; i< SCALARIZER_HASH_SIZE; i++)
	{
		cout << "VFH::hashEntry* __f__slist" << i << "[] = {";
		unsigned size = (scalarizer_hash[i]).size();
		if (size > longest) longest = size;
		for (unsigned j = 0; j < size; j++)
			cout << "&__f__node[" << (scalarizer_hash[i])[j] << "],";
		cout << "NULL};\n";		
	}		
	//  Dump hash master
	cout << "\nVFH::hashEntry** __f__scalarizer_hash_table[] = {";
	for (unsigned i = 0; i< SCALARIZER_HASH_SIZE; i++)
	{
		cout << "__f__slist" << i;
		if (i < SCALARIZER_HASH_SIZE-1) 
			cout << ",";
	}
	cout << "};\n//longest bucket: " << longest << "\n\n";
	
	// Vectorizer hash
	longest = 0;
	cout << "\n/*** Vectorizer hash ***/\n";
	//	Dump buckets
	for (unsigned i = 0; i< VECTORIZER_HASH_SIZE; i++)
	{
		cout << "VFH::hashEntry* __f__vlist" << i << "[] = {";
		unsigned size = (vectorizer_hash[i]).size();
		if (size > longest) longest = size;
		for (unsigned j = 0; j < size; j++)
			cout << "&__f__node[" << (vectorizer_hash[i])[j] << "],";
		cout << "NULL};\n";		
	}		
	//  Dump hash master
	cout << "\nVFH::hashEntry** __f__vectorizer_hash_table[] = {";
	for (unsigned i = 0; i< VECTORIZER_HASH_SIZE; i++)
	{
		cout << "__f__vlist" << i;
		if (i < VECTORIZER_HASH_SIZE-1) 
			cout << ",";
	}
	cout << "};\n//longest bucket: " << longest << "\n\n";
}



void createPostfix(unsigned num_funcs)
{
	
	cout
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
	<<"\nunsigned VFH::debugGetNumEntries()"
	<<"\n{"
	<<"\n	return " << num_funcs << ";"
	<<"\n}"
	<<"\n"
	<<"\nVFH::hashEntry * VFH::debugGetEntry(unsigned num)"
	<<"\n{"
	<<"\n	return &__f__node[num];"
	<<"\n}\n\n";
}



int main(int argc, char ** argv)
{
	unsigned num_funcs;
	
	// Gather functions into hash tables
	num_funcs = gatherHashes();

	// Dump prefix
	dumpPrefix();

	// Dump funcs list
	dumpNodes(num_funcs);
	
	// Dump hash tables
	dumpHashes();
	
	// Dump postfix
	createPostfix(num_funcs);

	return 0;
}


