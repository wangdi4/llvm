// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "OCLBuilder.h"
#include <cstring>
#include "clang_device_info.h"
#include <ocl_string_exception.h>

using namespace Intel::OpenCL::FECompilerAPI;
using namespace Intel::OpenCL;
using namespace Intel::OpenCL::Utils;

namespace Validation{
#define BE_FE_COMPILER_USE_EXTENSIONS "cl_khr_fp64 cl_khr_icd cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics cl_khr_int64_base_atomics cl_khr_int64_extended_atomics cl_khr_byte_addressable_store cl_intel_printf cl_ext_device_fission cl_intel_exec_by_local_thread cl_intel_vec_len_hint cl_intel_unified_shared_memory"

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
	catch (ocl_string_exception& Error)
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
	catch (ocl_string_exception& Error)
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
	catch (ocl_string_exception& Error)
	{
		throw Validation::Exception::OperationFailed(Error.what());
	}
}

OCLBuilder& OCLBuilder::withFP64Support(bool FP64) {
	try {
		m_CommonBuilder.withFP64Support(FP64);
		return *this;
	}
	catch (ocl_string_exception& Error)
	{
		throw Validation::Exception::OperationFailed(Error.what());
	}
}

OCLBuilder& OCLBuilder::withImageSupport(bool IS) {
	try {
		m_CommonBuilder.withImageSupport(IS);
		return *this;
	}
	catch (ocl_string_exception& Error)
	{
		throw Validation::Exception::OperationFailed(Error.what());
	}
}

//cleanup function
void OCLBuilder::close() {
	try {
		m_CommonBuilder.close();
	}
	catch (ocl_string_exception& Error)
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
	catch (ocl_string_exception& Error) {
		throw Validation::Exception::OperationFailed(Error.what());
	}
}

OCLBuilder::OCLBuilder() : m_CommonBuilder(Intel::OpenCL::Utils::CommonOCLBuilder::instance()) {
	m_CommonBuilder.withExtensions(BE_FE_COMPILER_USE_EXTENSIONS);
}

}
