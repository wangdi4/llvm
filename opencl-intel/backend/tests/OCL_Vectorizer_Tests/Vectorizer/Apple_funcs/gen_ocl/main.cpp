#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include "ocl_funcs.h"
using namespace std;

#define outfile cout
#define stringize(x) #x

int g_index = 0; // Global index used for naming new variables

int g_isInt3;
int g_types;


#define AMOUNT_OF_BASIC_TYPES	5
#define LOC_VEC3_TYPE			5
#define LOC_TYPE_PREFIX			6
#define LOC_TYPE_NAME			7
#define LOC_DUMMY_VAL			8

string _float[] =     {"float",  "float2",  "float4",  "float8",  "float16",  "float3",  "__F_", "FLOAT", "0.0f"};
//string _double[] =     {"double",  "double2", "double4",  "double8",  "double16",  "double3",  "__D_", "DOUBLE", "0.0f"};
string _int[] =       {"int",    "int2",  "int4",    "int8",    "int16",  "int3",    "__I_", "I32", "0"};
string _uint[] =      {"uint",   "uint2", "uint4",   "uint8",   "uint16",   "uint3",   "__UI_", "I32", "0"};
string _char[] =      {"char",   "char2", "char4",   "char8",   "char16",   "char3",   "__C_", "I8", "0"};
string _uchar[] =     {"uchar",  "uchar2", "uchar4",  "uchar8",  "uchar16",  "uchar3",  "__UC_", "I8", "0"};
string _short[] =     {"short",  "short2", "short4",  "short8",  "short16",  "short3",  "__S_", "I16", "0"};
string _ushort[] =    {"ushort", "ushort2", "ushort4", "ushort8", "ushort16", "ushort3", "__US_", "I16", "0"};
string _long[] =      {"long",   "long2", "long4",   "long8",   "long16",   "long3",   "__L_", "I64", "0"};
string _ulong[] =     {"ulong",  "ulong2", "ulong4",  "ulong8",  "ulong16",  "ulong3",  "__UL_", "I64", "0"};



void emit_single_global(const string &typeName,const string &varName,const string &dummyVal, unsigned width)
{
	outfile << "__constant " << typeName << " " << varName << " = " << dummyVal << ";\n";
}

unsigned widths[] = {1, 2, 4, 8, 16, 3};
string numstrs[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};

void emit_globals()
{
	for (int i = 0; i< AMOUNT_OF_BASIC_TYPES + 1; i++)
	{
		emit_single_global(_float[i], _float[LOC_TYPE_PREFIX] + numstrs[i], _float[LOC_DUMMY_VAL], widths[i]);
//		emit_single_global(_double[i], _double[LOC_TYPE_PREFIX] + numstrs[i], _double[LOC_DUMMY_VAL], widths[i]);
		emit_single_global(_char[i], _char[LOC_TYPE_PREFIX] + numstrs[i], _char[LOC_DUMMY_VAL], widths[i]);
		emit_single_global(_uchar[i], _uchar[LOC_TYPE_PREFIX] + numstrs[i], _uchar[LOC_DUMMY_VAL], widths[i]);
		emit_single_global(_short[i], _short[LOC_TYPE_PREFIX] + numstrs[i], _short[LOC_DUMMY_VAL], widths[i]);
		emit_single_global(_ushort[i], _ushort[LOC_TYPE_PREFIX] + numstrs[i], _ushort[LOC_DUMMY_VAL], widths[i]);
		emit_single_global(_int[i], _int[LOC_TYPE_PREFIX] + numstrs[i], _int[LOC_DUMMY_VAL], widths[i]);
		emit_single_global(_uint[i], _uint[LOC_TYPE_PREFIX] + numstrs[i], _uint[LOC_DUMMY_VAL], widths[i]);
		emit_single_global(_long[i], _long[LOC_TYPE_PREFIX] + numstrs[i], _long[LOC_DUMMY_VAL], widths[i]);
		emit_single_global(_ulong[i], _ulong[LOC_TYPE_PREFIX] + numstrs[i], _ulong[LOC_DUMMY_VAL], widths[i]);
	}
	outfile << "\n\n";
}


void emit_func_calls_x(string name, string* ret_type, string* var_type, int num_vars)
{
	outfile << "\n__kernel void _";
	outfile << ret_type[LOC_TYPE_NAME] << "_";
	for (int j = 0; j < num_vars; j++)
		outfile << var_type[LOC_TYPE_NAME] << "_";
	for (int j = num_vars + 1; j < 4; j++)
		outfile << "NONE_";
	outfile << g_types << "_" << name << g_index++ << "(__global " << ret_type[0] << " * output)\n{\n";
	
	for (int i = 0; i < g_types; i++)
	{
		outfile << "\t" << ret_type[i] << " _out" << i << " = " << name << "(";
		for (int j = 0; j < num_vars; j++)
		{
			outfile << var_type[LOC_TYPE_PREFIX] << i;
			if (j+1 < num_vars) outfile << ",";
		}
		outfile << ");\n";			
	}
	outfile << "\tint tid = get_global_id(0);\n";
	outfile << "\toutput[tid] = ";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "_out" << i;
		if (i > 0) outfile << ".s0";
		if (i+1 < g_types) outfile << " + ";
	}
	outfile << ";\n";
	outfile << "}\n";
}

void emit_func_calls_x_xx(string name, string* ret_type, string* var_type1, string* var_type2)
{
	outfile << "\n__kernel void _";
	outfile << ret_type[LOC_TYPE_NAME] << "_" << var_type1[LOC_TYPE_NAME] << "_" << var_type2[LOC_TYPE_NAME] << "_NONE_";
	outfile << g_types << "_" << name << g_index++ << "(__global " << ret_type[0] << " * output)\n{\n";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "\t" << ret_type[i] << " _out" << i << " = " << name << "(" << var_type1[LOC_TYPE_PREFIX] << i << "," << var_type2[LOC_TYPE_PREFIX] << i << ");\n";
	}
	outfile << "\tint tid = get_global_id(0);\n";
	outfile << "\toutput[tid] = ";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "_out" << i;
		if (i > 0) outfile << ".s0";
		if (i+1 < g_types) outfile << " + ";
	}
	outfile << ";\n";
	outfile << "}\n";
}

void emit_func_calls_x_xxx(string name, string* ret_type, string* var_type1, string* var_type2, string* var_type3)
{
	outfile << "\n__kernel void _";
	outfile << ret_type[LOC_TYPE_NAME] << "_" << var_type1[LOC_TYPE_NAME] << "_" << var_type2[LOC_TYPE_NAME] << "_" << var_type3[LOC_TYPE_NAME] << "_";
	outfile << g_types << "_" << name << g_index++ << "(__global " << ret_type[0] << " * output)\n{\n";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "\t" << ret_type[i] << " _out" << i << " = " << name << "(" 
			<< var_type1[LOC_TYPE_PREFIX] << i << "," 
			<< var_type2[LOC_TYPE_PREFIX] << i << "," 
			<< var_type3[LOC_TYPE_PREFIX] << i << ");\n";
	}
	outfile << "\tint tid = get_global_id(0);\n";
	outfile << "\toutput[tid] = ";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "_out" << i;
		if (i > 0) outfile << ".s0";
		if (i+1 < g_types) outfile << " + ";
	}
	outfile << ";\n";
	outfile << "}\n";
}

void emit_func_calls_x_xs(string name, string* ret_type, string* var_type, string* scalar_type)
{
	outfile << "\n__kernel void _";
	outfile << ret_type[LOC_TYPE_NAME] << "_" << var_type[LOC_TYPE_NAME] << "_STATIC_NONE_";
	outfile << g_types << "_" << name << g_index++ << "(__global " << ret_type[0] << " * output)\n{\n";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "\t" << ret_type[i] << " _out" << i << " = " << name << "(" << var_type[LOC_TYPE_PREFIX] << i << "," << scalar_type[LOC_TYPE_PREFIX] << "0);\n";
	}
	outfile << "\tint tid = get_global_id(0);\n";
	outfile << "\toutput[tid] = ";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "_out" << i;
		if (i > 0) outfile << ".s0";
		if (i+1 < g_types) outfile << " + ";
	}
	outfile << ";\n";
	outfile << "}\n";
}

void emit_func_calls_x_xss(string name, string* ret_type, string* var_type, string* scalar_type)
{
	outfile << "\n__kernel void _";
	outfile << ret_type[LOC_TYPE_NAME] << "_" << var_type[LOC_TYPE_NAME] << "_STATIC_STATIC_";
	outfile << g_types << "_" << name << g_index++ << "(__global " << ret_type[0] << " * output)\n{\n";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "\t" << ret_type[i] << " _out" << i << " = " << name << "(" 
			<< var_type[LOC_TYPE_PREFIX] << i << ","
			<< scalar_type[LOC_TYPE_PREFIX] << "0,"
			<< scalar_type[LOC_TYPE_PREFIX] << "0);\n";
	}
	outfile << "\tint tid = get_global_id(0);\n";
	outfile << "\toutput[tid] = ";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "_out" << i;
		if (i > 0) outfile << ".s0";
		if (i+1 < g_types) outfile << " + ";
	}
	outfile << ";\n";
	outfile << "}\n";
}

void emit_func_calls_x_xxs(string name, string* ret_type, string* var_type, string* scalar_type)
{
	outfile << "\n__kernel void _";
	outfile << ret_type[LOC_TYPE_NAME] << "_" << var_type[LOC_TYPE_NAME] << "_" << var_type[LOC_TYPE_NAME] << "_STATIC_";
	outfile << g_types << "_" << name << g_index++ << "(__global " << ret_type[0] << " * output)\n{\n";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "\t" << ret_type[i] << " _out" << i << " = " << name << "(" 
		<< var_type[LOC_TYPE_PREFIX] << i << ","
		<< var_type[LOC_TYPE_PREFIX] << i << ","
		<< scalar_type[LOC_TYPE_PREFIX] << "0);\n";
	}
	outfile << "\tint tid = get_global_id(0);\n";
	outfile << "\toutput[tid] = ";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "_out" << i;
		if (i > 0) outfile << ".s0";
		if (i+1 < g_types) outfile << " + ";
	}
	outfile << ";\n";
	outfile << "}\n";
}

void emit_func_calls_x_ssx(string name, string* ret_type, string* var_type, string* scalar_type)
{
	outfile << "\n__kernel void _";
	outfile << ret_type[LOC_TYPE_NAME] << "_STATIC_STATIC_" << var_type[LOC_TYPE_NAME] << "_";
	outfile << g_types << "_" << name << g_index++ << "(__global " << ret_type[0] << " * output)\n{\n";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "\t" << ret_type[i] << " _out" << i << " = " << name << "(" 
		<< scalar_type[LOC_TYPE_PREFIX] << "0,"
		<< scalar_type[LOC_TYPE_PREFIX] << "0,"
		<< var_type[LOC_TYPE_PREFIX] << i << ");\n";
	}
	outfile << "\tint tid = get_global_id(0);\n";
	outfile << "\toutput[tid] = ";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "_out" << i;
		if (i > 0) outfile << ".s0";
		if (i+1 < g_types) outfile << " + ";
	}
	outfile << ";\n";
	outfile << "}\n";
}

void emit_func_calls_x_sx(string name, string* ret_type, string* var_type, string* scalar_type)
{
	outfile << "\n__kernel void _";
	outfile << ret_type[LOC_TYPE_NAME] << "_STATIC_" << var_type[LOC_TYPE_NAME] << "_NONE_";
	outfile << g_types << "_" << name << g_index++ << "(__global " << ret_type[0] << " * output)\n{\n";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "\t" << ret_type[i] << " _out" << i << " = " << name << "(" 
		<< scalar_type[LOC_TYPE_PREFIX] << "0,"
		<< var_type[LOC_TYPE_PREFIX] << i << ");\n";
	}
	outfile << "\tint tid = get_global_id(0);\n";
	outfile << "\toutput[tid] = ";
	for (int i = 0; i < g_types; i++)
	{
		outfile << "_out" << i;
		if (i > 0) outfile << ".s0";
		if (i+1 < g_types) outfile << " + ";
	}
	outfile << ";\n";
	outfile << "}\n";
}








// Generate a function which invokes (in order) - "name" for float, float2, float4, float8, float16
void gen_func(string name, string proto_name, func_proto proto, unsigned is_support_vec3)
{
	g_isInt3 = is_support_vec3;
	g_types = AMOUNT_OF_BASIC_TYPES + g_isInt3;
	
	switch (proto) {	
		case PROTO_F_F1:
			emit_func_calls_x(name, _float, _float, 1);
			break;
		case PROTO_F_F2:
			emit_func_calls_x(name, _float, _float, 2);
			break;
		case PROTO_F_F3:
			emit_func_calls_x(name, _float, _float, 3);
			break;
		case PROTO_I_I1:
			emit_func_calls_x(name, _char, _char, 1);
			emit_func_calls_x(name, _uchar, _uchar, 1);
			emit_func_calls_x(name, _short, _short, 1);
			emit_func_calls_x(name, _ushort, _ushort, 1);
			emit_func_calls_x(name, _int, _int, 1);
			emit_func_calls_x(name, _uint, _uint, 1);
			emit_func_calls_x(name, _long, _long, 1);
			emit_func_calls_x(name, _ulong, _ulong, 1);
			break;
		case PROTO_I_I2:
			emit_func_calls_x(name, _char, _char, 2);
			emit_func_calls_x(name, _uchar, _uchar, 2);
			emit_func_calls_x(name, _short, _short, 2);
			emit_func_calls_x(name, _ushort, _ushort, 2);
			emit_func_calls_x(name, _int, _int, 2);
			emit_func_calls_x(name, _uint, _uint, 2);
			emit_func_calls_x(name, _long, _long, 2);
			emit_func_calls_x(name, _ulong, _ulong, 2);
			break;
		case PROTO_I_I3:
			emit_func_calls_x(name, _char, _char, 3);
			emit_func_calls_x(name, _uchar, _uchar, 3);
			emit_func_calls_x(name, _short, _short, 3);
			emit_func_calls_x(name, _ushort, _ushort, 3);
			emit_func_calls_x(name, _int, _int, 3);
			emit_func_calls_x(name, _uint, _uint, 3);
			emit_func_calls_x(name, _long, _long, 3);
			emit_func_calls_x(name, _ulong, _ulong, 3);		
			break;
		case PROTO_FastInt_2:
			emit_func_calls_x(name, _int, _int, 2);
			emit_func_calls_x(name, _uint, _uint, 2);
			break;
		case PROTO_FastInt_3:
			emit_func_calls_x(name, _int, _int, 3);
			emit_func_calls_x(name, _uint, _uint, 3);
			break;
		case PROTO_F_F2_or_Ff:
			emit_func_calls_x(name, _float, _float, 2);
			emit_func_calls_x_xs(name, _float, _float, _float);
			break;
		case PROTO_I_F:
			emit_func_calls_x(name, _int, _float, 1);
			break;
		case PROTO_F_FI_or_Fi:
			emit_func_calls_x_xx(name, _float, _float, _int);
			emit_func_calls_x_xs(name, _float, _float, _int);			
			break;
		case PROTO_F_FI:
			emit_func_calls_x_xx(name, _float, _float, _int);
			break;
		case PROTO_UI_I:
			emit_func_calls_x(name, _uchar, _char, 1);
			emit_func_calls_x(name, _uchar, _uchar, 1);
			emit_func_calls_x(name, _ushort, _short, 1);
			emit_func_calls_x(name, _ushort, _ushort, 1);
			emit_func_calls_x(name, _uint, _int, 1);
			emit_func_calls_x(name, _uint, _uint, 1);
			emit_func_calls_x(name, _ulong, _long, 1);
			emit_func_calls_x(name, _ulong, _ulong, 1);
			break;
		case PROTO_UI_II:
			emit_func_calls_x(name, _uchar, _char, 2);
			emit_func_calls_x(name, _uchar, _uchar, 2);
			emit_func_calls_x(name, _ushort, _short, 2);
			emit_func_calls_x(name, _ushort, _ushort, 2);
			emit_func_calls_x(name, _uint, _int, 2);
			emit_func_calls_x(name, _uint, _uint, 2);
			emit_func_calls_x(name, _ulong, _long, 2);
			emit_func_calls_x(name, _ulong, _ulong, 2);
			break;
		case PROTO_F_F3_or_Fff:
			emit_func_calls_x(name, _float, _float, 3);
			emit_func_calls_x_xss(name, _float, _float, _float);
			break;
		case PROTO_F_F3_or_FFf:
			emit_func_calls_x(name, _float, _float, 3);
			emit_func_calls_x_xxs(name, _float, _float, _float);
			break;
		case PROTO_F_F3_or_ffF:
			emit_func_calls_x(name, _float, _float, 3);
			emit_func_calls_x_ssx(name, _float, _float, _float);
			break;
		case PROTO_F_F2_or_fF:
			emit_func_calls_x(name, _float, _float, 2);
			emit_func_calls_x_sx(name, _float, _float, _float);
			break;
		case PROTO_ALL_or_Ff:
			emit_func_calls_x(name, _char, _char, 2);
			emit_func_calls_x(name, _uchar, _uchar, 2);
			emit_func_calls_x(name, _short, _short, 2);
			emit_func_calls_x(name, _ushort, _ushort, 2);
			emit_func_calls_x(name, _int, _int, 2);
			emit_func_calls_x(name, _uint, _uint, 2);
			emit_func_calls_x(name, _long, _long, 2);
			emit_func_calls_x(name, _ulong, _ulong, 2);			
			emit_func_calls_x(name, _float, _float, 2);
			emit_func_calls_x_xs(name, _float, _float, _float);			
			break;
		case PROTO_A_AAU_or_AAI:
			emit_func_calls_x_xxx(name, _char, _char, _char, _char);
			emit_func_calls_x_xxx(name, _uchar, _uchar, _uchar, _char);
			emit_func_calls_x_xxx(name, _short, _short, _short, _short);
			emit_func_calls_x_xxx(name, _ushort, _ushort, _ushort, _short);
			emit_func_calls_x_xxx(name, _int, _int, _int, _int);
			emit_func_calls_x_xxx(name, _uint, _uint, _uint, _int);
			emit_func_calls_x_xxx(name, _long, _long, _long, _long);
			emit_func_calls_x_xxx(name, _ulong, _ulong, _ulong, _long);			
			emit_func_calls_x_xxx(name, _float, _float, _float, _int);			
			emit_func_calls_x_xxx(name, _char, _char, _char, _uchar);
			emit_func_calls_x_xxx(name, _uchar, _uchar, _uchar, _uchar);
			emit_func_calls_x_xxx(name, _short, _short, _short, _ushort);
			emit_func_calls_x_xxx(name, _ushort, _ushort, _ushort, _ushort);
			emit_func_calls_x_xxx(name, _int, _int, _int, _uint);
			emit_func_calls_x_xxx(name, _uint, _uint, _uint, _uint);
			emit_func_calls_x_xxx(name, _long, _long, _long, _ulong);
			emit_func_calls_x_xxx(name, _ulong, _ulong, _ulong, _ulong);			
			emit_func_calls_x_xxx(name, _float, _float, _float, _uint);
			break;
		case PROTO_Iup_IUI:
			emit_func_calls_x_xx(name, _short, _char, _uchar);
			emit_func_calls_x_xx(name, _ushort, _uchar, _uchar);
			emit_func_calls_x_xx(name, _int, _short, _ushort);
			emit_func_calls_x_xx(name, _uint, _ushort, _ushort);
			emit_func_calls_x_xx(name, _long, _int, _uint);
			emit_func_calls_x_xx(name, _ulong, _uint, _uint);
			break;
		default:
			cerr << "\nError! Unsupported function type!\n";
			exit(-1);
			break;
	}


}


int main (int argc, char * const argv[]) 
{

	outfile << "/* AUTO GENERATED FILE - DO NOT EDIT */ \n\n";
	outfile << "\n\n\n";
	
	emit_globals();

#define ACTION(a,b,c) gen_func(stringize(a),stringize(b),b,c);
	GEN_ALL_FUNCS(ACTION);
#undef ACTION	
	
    return 0;
}
 

