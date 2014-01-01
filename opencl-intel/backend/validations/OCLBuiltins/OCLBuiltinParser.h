/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  OCLBuiltinParser.h

Parser of OpenCL builtin names compiled by Intel Clang to LLVM code. 
Example of buitlin name format "_Z10native_sinDv4_f"
Works with clang 2.8 name mangling convention

Attention(!): Apple clang has different naming convention of OpenCL builtin function
This parser cannot be used for parsing OpenCL builtin names produced by Apple clang

\*****************************************************************************/

#ifndef OCL_BUILTIN_PARSER_H
#define OCL_BUILTIN_PARSER_H

#include <vector>
#include <string>
#include "llvm/Support/DataTypes.h"

namespace llvm {

    /// Parser of OCL built-ins
    /// !!! Works with clang 2.8 name mangling convention
    class OCLBuiltinParser
    {
    public:
        // fwd declaration
        struct ARG;

        /// general type of argument 
        enum GenArgType
        {
            BASIC = 0,
            VECTOR,
            ARRAY,
            POINTER,
            IMAGE, // images in OpenCL1.2 should have this type
            SAMPLER,
            NA = 255
        };

#undef VOID // winnt.h defines VOID to be void, so now that this file gets included, we need this to avoid a compilation error.

        /// basic types
        enum BasicArgType
        {
            VOID = 0,
            BOOL,
            CHAR,
            UCHAR,
            SHORT,
            USHORT,
            INT,
            UINT,
            LONG,
            ULONG,
            LONGLONG,
            ULONGLONG,
            INT128,
            UINT128,
            FLOAT,
            DOUBLE,
            LONGDOUBLE,
            IMAGE_1D_T,
            IMAGE_2D_T,
            IMAGE_3D_T,
            IMAGE_1D_BUFFER_T,
            IMAGE_1D_ARRAY_T,
            IMAGE_2D_ARRAY_T,
            SAMPLER_T,
            INVALID = 255
        };

        /// vector type
        struct VecType
        {
            /// type of element. should be basic type
            BasicArgType elType;
            /// number of elements in vector
            uint32_t elNum;
        };

        /// array type
        struct ArrType
        {
            /// type of array's element. 
            /// vector should have size()== 1
            std::vector<ARG> elType;
            /// number of elements in array
            int elNum;
        };

        enum AddressSpace
        {
            PRIVATE = 0,
            GLOBAL,
            CONSTANT,
            LOCAL,
            GENERIC,
            INVALID_AS = 255
        };

        /// pointer type
        struct PointerType
        {
            /// default ctor
            PointerType() :
                isAddrSpace(false), isPointsToConst(false), AddrSpace(PRIVATE), ptrToStr("")
            {}
            // TODO: arbitrary type of pointer
            //std::vector<ARG> ptrType;
            /// is pointer specifies address space
            bool isAddrSpace;
            /// Is pointer points to constant type?
            bool isPointsToConst;
            /// number of address space
            AddressSpace AddrSpace;
            /// string with pointer type
            std::string ptrToStr;
            /// type of ptr
            /// vector should have size()== 1
            std::vector<ARG> ptrType;
        };

        /// image type
        struct ImageType
        {
            std::string imgStr;
        };

        /// argument of function
        struct ARG
        {
            // general desc
            GenArgType genType;

            union{
                /// basic 
                BasicArgType basicType;
                /// vector 
                VecType vecType;
            };
            /// array
            ArrType arrType;
            /// pointer
            PointerType ptrType;
            /// image
            ImageType imgType;

        };

        /// vector of arguments
        typedef std::vector<ARG> ArgVector;

        /// This function detects NEAT supported OCL builtin name from string 
        /// Name mangling is from clang 2.8
        /// string is usually obtained from llvm::Function F->getNameStr()
        /// @param in_str - string with LLVM function F->getNameStr()
        /// @param out_FuncStr - resulting string with exctracted OCL builtin name
        /// for instance "sin", "cos", "tan", etc
        /// @param out_Args - contains parsed builtin arguments
        /// @return - true if OCL builtin name is found, false if not
        static bool ParseOCLBuiltin(const std::string& in_str, 
            std::string& out_FuncStr, ArgVector& out_Args );

        static bool GetOCLMangledName(const std::string& in_funcName,
            const ArgVector& in_args, std::string& out_str);

    };

} // llvm
#endif // OCL_BUILTIN_PARSER_H

