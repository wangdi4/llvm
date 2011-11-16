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

File Name:  OpenCLProgram.cpp

\*****************************************************************************/
#include "OpenCLProgram.h"
#include "cl_types.h"
#include "exceptions.h"
#include "SATestException.h"

#include "llvm/Module.h"
#include "llvm/Bitcode/BitstreamWriter.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Assembly/Parser.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"

#include <stdio.h>
#include <memory.h>
#include <fstream>

using namespace Validation;
using namespace Intel::OpenCL::DeviceBackend;
using std::string;

OpenCLProgram::OpenCLProgram(OpenCLProgramConfiguration * oclProgramConfig,
                             const std::string cpuArch)
{
    std::stringstream fileName;
    fileName << oclProgramConfig->GetProgramName().c_str() << 
        ".llvm_ir" << std::ends;

    //Checks to see if the file already exist, and if so, gives a warning
    FILE* pIRfile = NULL;
    #ifdef _WIN32
        fopen_s(&pIRfile, fileName.str().c_str(), "rb");
    #else
        pIRfile = fopen(fileName.str().c_str(), "rb");
    #endif

    if ( NULL != pIRfile )
    {
      fclose(pIRfile);
      std::cout << "Warning! SATest will overwrite "<< fileName.str().c_str() 
          << std::endl;
    }
    
    std::string programFile = oclProgramConfig->GetProgramFilePath();
    switch (oclProgramConfig->GetProgramFileType())
    {
        case CL:
            {
                OpenCLIncludeDirs* includeDirs(oclProgramConfig->GetIncludeDirs());
                std::stringstream cmd;
                cmd << "clang -cc1 -x cl -S -emit-llvm-bc -target-cpu "
                    << cpuArch ;
                    for(OpenCLIncludeDirs::IncludeDirsList::const_iterator it = includeDirs->beginIncldueDirs();
                        it != includeDirs->endIncludeDirs();
                        ++it )
                    {
                    cmd << " -I "<< *it ;
                    }
                    cmd << " -include opencl_.h -D __OPENCL_VERSION__=110 -D \
                    CL_VERSION_1_0=100 -D CL_VERSION_1_1=110 -D __ENDIAN_LITTLE__=1 \
                    -D __ROUNDING_MODE__=rte -D __IMAGE_SUPPORT__=1 -D cl_khr_fp64 \
                    -target-feature +cl_khr_fp64 -D cl_khr_global_int32_base_atomics \
                    -target-feature +cl_khr_global_int32_base_atomics -D \
                    cl_khr_global_int32_extended_atomic -target-feature \
                    +cl_khr_global_int32_extended_atomics -D cl_khr_local_int32_base_atomics \
                    -target-feature +cl_khr_local_int32_base_atomics -D \
                    cl_khr_local_int32_extended_atomics -target-feature \
                    +cl_khr_local_int32_extended_atomics -D cl_khr_gl_sharing \
                    -target-feature +cl_khr_gl_sharing -D cl_khr_byte_addressable_store \
                    -target-feature +cl_khr_byte_addressable_store -D cl_intel_printf \
                    -target-feature +cl_intel_printf -D cl_intel_overloading \
                    -target-feature +cl_intel_overloading -O0 ";
                #ifndef _WIN32
                    cmd << "-triple x86_64-unknown-linux-gnu ";
                #endif
                cmd << programFile.c_str() << " -o " << fileName.str().c_str() << std::ends;
                system(cmd.str().c_str());
                BCOpenCLProgram(fileName.str());
         }
            break;
        case LL:
            {
                llvm::SMDiagnostic err;
                llvm::LLVMContext context;
                std::auto_ptr<llvm::Module> M(llvm::ParseAssemblyFile(programFile,
                                                err, context));
                std::vector<unsigned char> buffer;
                llvm::BitstreamWriter stream(buffer);
                llvm::WriteBitcodeToStream( M.get(), stream );

                // Create the output file for testing reason.
                std::string ErrorInfo;
                llvm::raw_fd_ostream FDTemp(fileName.str().c_str(), ErrorInfo,
                            llvm::raw_fd_ostream::F_Binary);
                if (ErrorInfo.empty()) {
                    llvm::WriteBitcodeToFile( M.get(), FDTemp );
                }

                std::vector<unsigned char> byteCodeBuffer(stream.getBuffer());

                int bufferSize=byteCodeBuffer.size();
                containerSize = sizeof(cl_prog_container_header) +
                                                sizeof(cl_llvm_prog_header);
                containerSize += (unsigned int)bufferSize;
                setContainer();

                pContainer->container_size = (unsigned int)bufferSize +
                                                sizeof(cl_llvm_prog_header);
                char* pContainerPosition = ((char*)pContainer) +
                    sizeof(cl_prog_container_header)+ sizeof(cl_llvm_prog_header);
                for (unsigned i = 0, e = bufferSize; i != e; ++i) {
                    pContainerPosition[i] = byteCodeBuffer[i];
                }
            }
            break;
        case BC:
            BCOpenCLProgram(programFile);
            break;
        default:
            throw Exception::InvalidArgument("Unsupported byte code type.");
    }

}

OpenCLProgram::~OpenCLProgram(void)
{
  delete [] pContainer;
}

cl_prog_container_header* OpenCLProgram::GetProgramContainer() const
{
  return pContainer;
}

unsigned int OpenCLProgram::GetProgramContainerSize() const
{
  return containerSize;
}

void OpenCLProgram::BCOpenCLProgram(const string& programFile)
{
  const char* szFileName = programFile.c_str(); 

  containerSize = sizeof(cl_prog_container_header) + sizeof(cl_llvm_prog_header);

  FILE* pIRfile = NULL;

#ifdef _WIN32
  fopen_s(&pIRfile, szFileName, "rb");
#else
  pIRfile = fopen(szFileName, "rb");
#endif

  if ( NULL == pIRfile )
  {
      throw Exception::IOError("Unable to open file with LLVM program");
  }
  fseek(pIRfile, 0, SEEK_END);
  unsigned int fileSizeUint = (unsigned int)(ftell (pIRfile));
  fseek(pIRfile, 0, SEEK_SET);

  containerSize += fileSizeUint;

  // Construct program container
  setContainer();

  pContainer->container_size = fileSizeUint + sizeof(cl_llvm_prog_header);
  fread(((char*)pContainer) + sizeof(cl_prog_container_header) +
                sizeof(cl_llvm_prog_header), 1, (size_t)fileSizeUint, pIRfile);
  fclose(pIRfile);
}

void OpenCLProgram::setContainer()
{
  pContainer = (cl_prog_container_header*)(new char[containerSize]);
  memset(pContainer, 0, sizeof(cl_prog_container_header) +
                                        sizeof(cl_llvm_prog_header));

  // Container mask
  memcpy((void*)pContainer->mask, _CL_CONTAINER_MASK_, sizeof(pContainer->mask));

  pContainer->container_type = CL_PROG_CNT_PRIVATE;
  pContainer->description.bin_type = CL_PROG_BIN_LLVM;
  pContainer->description.bin_ver_major = 1;
  pContainer->description.bin_ver_minor = 1;
}
