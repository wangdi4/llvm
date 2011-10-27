//===--- Builtins.cpp - Builtin function implementation -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements various things for builtin functions.
//
//===----------------------------------------------------------------------===//

#include "stdafx.h"

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Constants.h"
#include "llvm/Module.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/CallingConv.h"
#include "llvm/TypeSymbolTable.h"

#include "clang/ast/Builtins.h"

using namespace llvm;

static const clang::Builtin::Info myBuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS) { #ID, TYPE, ATTRS, 0, false },
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER) { #ID, TYPE, ATTRS, 0, false },
#include "clang/AST/Builtins.def"
#define BUILTIN(ID, TYPE, ATTRS) { #ID, TYPE, ATTRS, 0, false },
#include "clang/AST/X86Builtins.def"
	{NULL, NULL, NULL, NULL, false}
};

/// DecodeTypeFromStr - This decodes one type descriptor from Str, advancing the
/// pointer over the consumed characters.  This returns the resultant type.
static const llvm::Type *DecodeTypeFromStr(const char *&Str, bool AllowTypeModifiers = true)
{
	// Modifiers.
	int HowLong = 0;
	bool Signed = false, Unsigned = false;

	// Read the modifiers first.
	bool Done = false;
	while (!Done)
	{
		switch (*Str++) 
		{
		default: Done = true; --Str; break; 
		case 'S':
			assert(!Unsigned && "Can't use both 'S' and 'U' modifiers!");
			assert(!Signed && "Can't use 'S' modifier multiple times!");
			Signed = true;
			break;
		case 'U':
			assert(!Signed && "Can't use both 'S' and 'U' modifiers!");
			assert(!Unsigned && "Can't use 'S' modifier multiple times!");
			Unsigned = true;
			break;
		case 'L':
			assert(HowLong <= 2 && "Can't have LLLL modifier");
			++HowLong;
			break;
		}
	}

	const llvm::Type *retType;

	// Read the base type.
	switch (*Str++)
	{
	default: assert(0 && "Unknown builtin type letter!");
	case 'v':
		assert(HowLong == 0 && !Signed && !Unsigned &&
			"Bad modifiers used with 'v'!");
		retType = Type::VoidTy;
		break;
	case 'f':
		assert(HowLong == 0 && !Signed && !Unsigned &&
			"Bad modifiers used with 'f'!");
		retType = Type::FloatTy;
		break;
	case 'd':
		assert(HowLong < 2 && !Signed && !Unsigned &&
			"Bad modifiers used with 'd'!");
		if (HowLong)
			retType = Type::DoubleTy;
		else
			retType = Type::DoubleTy;
		break;
	case 's':
		assert(HowLong == 0 && "Bad modifiers used with 's'!");
		if (Unsigned)
			// (GUY: unsigned
			retType = Type::Int16Ty;
		else
			retType = Type::Int16Ty;
		break;
	case 'i':
		if (HowLong == 3)
			// (GUY: unsigned
			retType = Unsigned ? Type::Int64Ty : Type::Int64Ty;
		else if (HowLong == 2)
			// (GUY: unsigned
			retType = Unsigned ? Type::Int64Ty : Type::Int64Ty;
		else if (HowLong == 1)
			// (GUY: unsigned
			retType = Unsigned ? Type::Int64Ty : Type::Int64Ty;
		else
			// (GUY: unsigned
			retType = Unsigned ? Type::Int32Ty : Type::Int32Ty;
		break;
	case 'c':
		assert(HowLong == 0 && "Bad modifiers used with 'c'!");
		if (Signed)
			// (GUY: signed
			retType = Type::Int8Ty;
		else if (Unsigned)
			// (GUY: unsigned
			retType = Type::Int8Ty;
		else
			retType = Type::Int8Ty;
		break;
	case 'b': // boolean
		assert(HowLong == 0 && !Signed && !Unsigned && "Bad modifiers for 'b'!");
		retType = Type::Int1Ty;
		break;
	case 'z':  // size_t.
		assert(HowLong == 0 && !Signed && !Unsigned && "Bad modifiers for 'z'!");
		// GUY: unsigned
		retType = Type::Int32Ty;
		break;
	case 'F':
		// GUY: const string
		retType = PointerType::get(Type::Int8Ty,0);
		break;
	case 'a':
		// GUY: va list
		retType = PointerType::get(Type::Int8Ty, 0);
		break;
	case 'A':
		// This is a "reference" to a va_list; however, what exactly
		// this means depends on how va_list is defined. There are two
		// different kinds of va_list: ones passed by value, and ones
		// passed by reference.  An example of a by-value va_list is
		// x86, where va_list is a char*. An example of by-ref va_list
		// is x86-64, where va_list is a __va_list_tag[1]. For x86,
		// we want this argument to be a char*&; for x86-64, we want
		// it to be a __va_list_tag*.
		// GUY: va list reference
		retType = PointerType::get(Type::Int8Ty,0);;
		break;
		// OpenCL: Support 'X' for ExtVector, will remove once we're using PCH.
	case 'X': {
			char *End;

			unsigned NumElements = strtoul(Str, &End, 10);
			assert(End != Str && "Missing vector size");

			Str = End;

			const llvm::Type *ElementType = DecodeTypeFromStr(Str, false);
			retType = VectorType::get(ElementType, NumElements);
			break;
		}
	case 'V': {
			char *End;

			unsigned NumElements = strtoul(Str, &End, 10);
			assert(End != Str && "Missing vector size");

			Str = End;

			const llvm::Type *ElementType = DecodeTypeFromStr(Str, false);
			retType = VectorType::get(ElementType, NumElements);
			break;
		}
	case 'P': {
			// GUY: FILE
			retType = PointerType::get(Type::Int8Ty,0);;
			break;
		}
	}

	if (!AllowTypeModifiers)
		return retType;

	Done = false;
	while (!Done) {
		switch (*Str++) {
		default: Done = true; --Str; break;
		case '*': {
			char *End;
			unsigned AddrSpaceNum = strtoul(Str, &End, 10);
			if (Str != End) {
				if(retType == llvm::Type::VoidTy) retType = llvm::Type::Int8Ty;
				retType = PointerType::get(retType, AddrSpaceNum);
				Str = End;
			}
			if(retType == llvm::Type::VoidTy) retType = llvm::Type::Int8Ty;
			retType = PointerType::get(retType, 0);
			break;
		}
		case '&':
			// GUY: Reference
			retType = PointerType::get(Type::Int8Ty, 0);
			break;
		case 'C':
			// (GUY: const
			retType = Type::Int64Ty;
			break;
		}
	}

	return retType;
}

llvm::Module *CreateBuiltinsModule()
{
	llvm::Module *retModule = new llvm::Module("builtins");

	for(int i = 0; myBuiltinInfo[i].Name != NULL; i++)
	{
		const llvm::Type *rettype;
		std::vector<const llvm::Type *> Params;

		const char *Str   = myBuiltinInfo[i].Type;

		rettype = DecodeTypeFromStr(Str);

		// Change width of vectors with less than 128 bits to 128 bits (like clang - Guy)
		if(VectorType::classof(rettype))
		{
			 const llvm::VectorType *vectType = dyn_cast<llvm::VectorType>(rettype);

			 const llvm::Type *baseType = vectType->getElementType();
			 int numElements = vectType->getNumElements();

			 if(baseType == llvm::Type::Int8Ty && numElements > 1)
				 rettype = VectorType::get(baseType, 16);
			 else if(baseType == llvm::Type::Int16Ty && numElements < 8 && numElements > 1)
				 rettype = VectorType::get(baseType, 8);

		}

		while (Str[0] && Str[0] != '.')
		{
			const llvm::Type *Ty = DecodeTypeFromStr(Str);

			//// Do array -> pointer decay.  The builtin should use the decayed type.
			//if (Ty->isArrayType())
			//	Ty = Context.getArrayDecayedType(Ty);

			Params.push_back(Ty);
		}

		llvm::FunctionType *fType = llvm::FunctionType::get(rettype, Params, false);

		retModule->getOrInsertFunction(myBuiltinInfo[i].Name, fType);
	}

	return retModule;
}

