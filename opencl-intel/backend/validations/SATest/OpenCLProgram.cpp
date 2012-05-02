/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2012).

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
#include "OCLBuilder.h"
#include "Exception.h"

#include "llvm/Module.h"
#include "llvm/Bitcode/BitstreamWriter.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Assembly/Parser.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/IRReader.h"

#include <stdio.h>
#include <memory.h>
#include <fstream>

using namespace Validation;
using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::OpenCL::FECompilerAPI;
using std::string;

static std::string buildLibName (const char* s){
  std::stringstream ret;
#ifdef _WIN32
  ret << s << ".dll";
#else
  ret << "lib" << s << ".so";
#endif
  return ret.str();
}

OpenCLProgram::OpenCLProgram(OpenCLProgramConfiguration * oclProgramConfig,
                             const std::string cpuArch)
{
    std::string programFile = oclProgramConfig->GetProgramFilePath();
    switch (oclProgramConfig->GetProgramFileType())
    {
        case CL:
            {
                OpenCLIncludeDirs* includeDirs(oclProgramConfig->GetIncludeDirs());
                std::stringstream buildOptions;
                if (includeDirs) {
                    for(OpenCLIncludeDirs::IncludeDirsList::const_iterator it = includeDirs->beginIncldueDirs();
                        it != includeDirs->endIncludeDirs();
                        ++it )
                     {
                            buildOptions << " -I "<< *it ;
                     }
                }
                //reading the source file to be compiled
                std::fstream indata(programFile.c_str());
                if(!indata.is_open())// file couldn't be opened
                    throw Exception::IOError(programFile.c_str());
                indata.seekg (0, std::ios::end);
                size_t length = indata.tellg();
                indata.seekg (0, std::ios::beg);
                if (length <= 0)
                    throw Exception::IOError("empty source file");
                char* source = new char[length+1];
                indata.read(source, length);
                size_t read = indata.gcount();
                //in Win-based OS, each line results one-character less read to
                //the buffer, so the actual num of bytes read is lower than the
                //actual file length.
                assert (read <= length);
                source[read] = '\0';
                indata.close();
                //building the CL code:
                std::string clangLib = buildLibName("clang_compiler");
                OCLBuilder& builder = OCLBuilder::instance().
                withSource(source).
                withBuildOptions(buildOptions.str().c_str()).
                withLibrary(clangLib.c_str());
                IOCLFEBinaryResult* result = builder.build();
                delete[] source;
                //
                //Allocating the container
                //
                this->containerSize = result->GetIRSize();
                this->pContainer = (cl_prog_container_header*)(new char[containerSize]);
                memcpy(this->pContainer,
                  result->GetIR(),
                  this->containerSize
                );
                //
                //cleanup
                //
                result->Release();
                builder.close();
                break;
            }
        case LL:
            {
                llvm::SMDiagnostic err;
                llvm::LLVMContext context;
                std::auto_ptr<llvm::Module> M(llvm::ParseAssemblyFile(programFile,
                                                err, context));
                std::vector<unsigned char> buffer;
                llvm::BitstreamWriter stream(buffer);
                llvm::WriteBitcodeToStream( M.get(), stream );
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
  pContainer->description.bin_type = CL_PROG_BIN_EXECUTABLE_LLVM;
  pContainer->description.bin_ver_major = 1;
  pContainer->description.bin_ver_minor = 1;
}
