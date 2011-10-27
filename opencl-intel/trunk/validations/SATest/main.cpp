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

File Name:  main.cpp

\*****************************************************************************/
#include "SATest.h"
#include "SATestException.h"
#include "IRunConfiguration.h"
#include "llvm/System/DataTypes.h"

#include <string>
#include <iostream>

// Command line options
#include "llvm/Support/CommandLine.h"

using namespace Validation;
using std::endl;

llvm::cl::opt<std::string>
ConfigFile("config",
           llvm::cl::desc("Test configuration file parameters."),
           llvm::cl::value_desc("filename"));

llvm::cl::opt<std::string>
BaseDirectory("basedir",
           llvm::cl::desc("Base directory to use for data file lookup"),
           llvm::cl::value_desc("basedir"));


llvm::cl::opt<RunnerFactory::PROGRAM_TYPE>
ProgramType(llvm::cl::desc("Type of the test program:"),
            llvm::cl::values(
            clEnumValN(RunnerFactory::OPEN_CL, "OCL", "OpenCL program"),
            clEnumValN(RunnerFactory::DIRECT_X, "DX", "DirectX program"),
            clEnumValEnd
            )
            );

llvm::cl::opt<ETransposeSize> 
TransposeSize("tsize",
         llvm::cl::desc("Transpose size:"),
         llvm::cl::values(
         clEnumValN(TRANSPOSE_SIZE_AUTO, "0",    "Automatic mode"),
         clEnumValN(TRANSPOSE_SIZE_1,    "1",    "Scalar"),
         clEnumValN(TRANSPOSE_SIZE_4,    "4",    "Vector4"),
         clEnumValN(TRANSPOSE_SIZE_8,    "8",    "Vector8"),
         clEnumValN(TRANSPOSE_SIZE_16,   "16",   "Vector16"),
         clEnumValEnd),
         llvm::cl::init(TRANSPOSE_SIZE_AUTO)
         );


llvm::cl::opt<std::string> 
CPUArch("cpuarch",
         llvm::cl::desc("CPU Architecture: auto, corei7, sandybridge, haswell, knf"),
         llvm::cl::init("auto")
         );

llvm::cl::opt<std::string> 
CPUFeatures("cpufeatures",
         llvm::cl::desc("CPU Features: -avx, +avx2, ..."),
         llvm::cl::init("")
         );

llvm::cl::opt<bool>
NoRef("noref",
      llvm::cl::desc("Do not run reference, nor neat in validation mode"),
      llvm::cl::init(false));

llvm::cl::opt<bool>
FlagForceRunReference("force_ref",
                   llvm::cl::desc("Force running reference"),
                   llvm::cl::init(false));

llvm::cl::opt<bool>
TraceMarks("trace",
      llvm::cl::desc("Insert trace marks for SDE pinLIT traces generation"),
      llvm::cl::init(false));

llvm::cl::opt<bool>
UseNEAT("neat",
        llvm::cl::desc("Use NEAT"),
        llvm::cl::init(false));

llvm::cl::opt<uint32_t>
BuildIterations("build-iterations",
           llvm::cl::desc("Iterations count to build the kernel in performance mode"),
           llvm::cl::value_desc("Number of iterations"),
           llvm::cl::init(1));

llvm::cl::opt<uint32_t>
ExecuteIterations("execute-iterations",
           llvm::cl::desc("Iterations count to execute the kernel in performance mode"),
           llvm::cl::value_desc("Number of iterations"),
           llvm::cl::init(1));

llvm::cl::opt<TEST_MODE>
TestMode(llvm::cl::desc("Test mode:"),
         llvm::cl::values(
         clEnumValN(VALIDATION, "VAL", "Validation mode"),
         clEnumValN(REFERENCE, "REF", "Reference mode"),
         clEnumValN(PERFORMANCE, "PERF", "Performance mode"),
         clEnumValN(BUILD, "BUILD", "Build only mode"),
         clEnumValEnd));

// turn on running single work group
llvm::cl::opt<bool>
RunSingleWG("single_wg",
            llvm::cl::desc("Run only one work group."),
            llvm::cl::init(false));

// tolerance used in Comparator for comparison of floating point numbers in accurate mode
llvm::cl::opt<double>
ULP_tolerance("ulp_tol",
              llvm::cl::desc("ULP tolerance for comparison"),
              llvm::cl::init(0.0));

// Default Local work group size used in Validation mode
llvm::cl::opt<uint32_t>
DefaultLocalWGSize("default_wg_size",
                   llvm::cl::desc("Default local work group size used in Validation mode"),
                   llvm::cl::init(4));

// turn on printing LLVM IR produced by Volcano after all optimization passes applied.
llvm::cl::opt<std::string>
OptimizedLLVMIRDumpFile("dump-llvm-file",
                        llvm::cl::desc("Prints LLVM IR to the file <filename> after all optimization passes applied. If '-' filename is set the LLVM IR will be printed to the standard output stream."),
                        llvm::cl::value_desc("filename"),
                        llvm::cl::init(""));

// Enable printing additional information about comparison results.
llvm::cl::opt<bool>
DetailedStat("detailed_stat",
                 llvm::cl::desc("Print detailed statistics."),
                 llvm::cl::init(false));

// Enable VTune support in Volcano.
llvm::cl::opt<bool>
UseVTune("vtune",
        llvm::cl::desc("Enable VTune support in Volcano."),
        llvm::cl::init(false));

// Command line example:
// SATest.exe -OCL -config=test.cfg
// WARNING! To run OCL_CPU_DEVICE_BACKEND successfully built-in DLLs and RTLs is needed!

int main(int argc, char *argv[])
{
    // Parse received options
    llvm::cl::ParseCommandLineOptions(argc, argv,
        "Stand-alone test application to validate OCL/DX back-end.\n");

    try
    {
        // Run test
        IRunConfiguration* runConfig = RunnerFactory::GetInstance(ProgramType).CreateRunConfiguration();
        runConfig->InitFromCommandLine();

        SATest test(ProgramType,
                     ConfigFile,
                     BaseDirectory,
                     runConfig);
        test.Run( TestMode, runConfig);
        return 0;
    }
    catch (Exception::TestFailException e)
    {
        // Test does not match reference
        std::cerr << "Test Failed: " << e.what() << endl;
        return -1;
    }
    catch (Exception::ValidationExceptionBase e)
    {
        // Exception occurred during test run process
        std::cerr << "Validation exception occurred: " << e.what() << endl;
        return -1;
    }
}
