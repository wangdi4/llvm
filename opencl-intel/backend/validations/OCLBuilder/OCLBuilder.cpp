/*****************************************************************************\

Copyright (c) Intel Corporation (2011,2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: OCLBuilder.cpp

\*****************************************************************************/

#include "OCLBuilder.h"
#include <cstring>
#include "clang_device_info.h"
#include <ocl_string_exception.h>

using namespace Intel::OpenCL::FECompilerAPI;
using namespace Intel::OpenCL;
using namespace Intel::OpenCL::Utils;

namespace Validation{
#define BE_FE_COMPILER_USE_EXTENSIONS "cl_khr_fp64 cl_khr_icd cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_byte_addressable_store cl_intel_printf cl_ext_device_fission cl_intel_exec_by_local_thread cl_intel_vec_len_hint"

OCLBuilder& OCLBuilder::Instance() {
  //Statically initialized instance of the builder
  static OCLBuilder instance;
  return instance;
}


//Sets the name of the library, from which the compiler will be loaded
OCLBuilder& OCLBuilder::withLibrary(const char* lib)
{
	try {
		m_CommonBuilder.withLibrary(lib);
		return *this;
	}
	catch (ocl_string_exception Error)
	{
		throw Validation::Exception::OperationFailed(Error.what());
	}
}

//Sets the build options
OCLBuilder& OCLBuilder::withBuildOptions(const char* options) {
	try {
		m_CommonBuilder.withBuildOptions(options);
		return *this;
	}
	catch (ocl_string_exception Error)
	{
		throw Validation::Exception::OperationFailed(Error.what());
	}
}

//sets the OCL source to be compiled
OCLBuilder& OCLBuilder::withSource(const char* src) {
	try {
		m_CommonBuilder.withSource(src);
		return *this;
	}
	catch (ocl_string_exception Error)
	{
		throw Validation::Exception::OperationFailed(Error.what());
	}
}

OCLBuilder& OCLBuilder::withFP64Support(bool FP64) {
	try {
		m_CommonBuilder.withFP64Support(FP64);
		return *this;
	}
	catch (ocl_string_exception Error)
	{
		throw Validation::Exception::OperationFailed(Error.what());
	}
}

OCLBuilder& OCLBuilder::withImageSupport(bool IS) {
	try {
		m_CommonBuilder.withImageSupport(IS);
		return *this;
	}
	catch (ocl_string_exception Error)
	{
		throw Validation::Exception::OperationFailed(Error.what());
	}
}

//cleanup function
void OCLBuilder::close() {
	try {
		m_CommonBuilder.close();
	}
	catch (ocl_string_exception Error)
	{
		throw Validation::Exception::OperationFailed(Error.what());
	}
}

//Compiles the (previously given) source file with the compiler loaded from
//(the previously given) library.
//Returns value: IOCLFEBinaryResult, which contains the binary result in its
//bytecode form, with some metadeta on it (size in bytes etc.)
Intel::OpenCL::ClangFE::IOCLFEBinaryResult* OCLBuilder::build() {
	try {
		return m_CommonBuilder.build();
	}
	catch (ocl_string_exception Error) {
		throw Validation::Exception::OperationFailed(Error.what());
	}
}

OCLBuilder::OCLBuilder() : m_CommonBuilder(Intel::OpenCL::Utils::CommonOCLBuilder::instance()) {
	m_CommonBuilder.withExtensions(BE_FE_COMPILER_USE_EXTENSIONS);
}

}
