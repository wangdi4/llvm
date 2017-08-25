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

File Name:  main.cpp

\*****************************************************************************/
#include "SATest.h"
#include "SATestException.h"
#include "IRunConfiguration.h"
#include "llvm/Support/DataTypes.h"
#include "exceptions.h"

#include <string>
#include <iostream>
#include <time.h>

// Command line options
#include "llvm/Support/CommandLine.h"

using namespace Validation;
using std::endl;

llvm::cl::opt<std::string>
ConfigFile("config",
           llvm::cl::desc("Test configuration file parameters."),
           llvm::cl::value_desc("filename"));

llvm::cl::opt<std::string>
ObjectFile("inject-object-file",
           llvm::cl::ValueOptional,
           llvm::cl::desc("Execute pre-compiled object file"),
           llvm::cl::value_desc("filename"));

// Enable VTune support in Volcano.
llvm::cl::opt<bool>
StopBeforeJIT("stop-before-jit",
        llvm::cl::desc("Stops compilation after all optimization passes, but before binary generation."),
        llvm::cl::init(false));

llvm::cl::opt<bool>
Verbose("verbose",
        llvm::cl::desc("Print argument's metadata."),
        llvm::cl::init(false));

llvm::cl::opt<std::string>
BaseDirectory("basedir",
           llvm::cl::desc("Base directory to use for configuration and data files lookup. "
                          "Default value - configuration file directory."),
           llvm::cl::value_desc("basedir"));

llvm::cl::opt<ETransposeSize>
TransposeSize("tsize",
         llvm::cl::desc("Transpose size:"),
         llvm::cl::values(
         clEnumValN(TRANSPOSE_SIZE_AUTO, "0",    "Automatic mode"),
         clEnumValN(TRANSPOSE_SIZE_1,    "1",    "Scalar"),
         clEnumValN(TRANSPOSE_SIZE_4,    "4",    "Vector4"),
         clEnumValN(TRANSPOSE_SIZE_8,    "8",    "Vector8"),
         clEnumValN(TRANSPOSE_SIZE_16,   "16",   "Vector16")),
         llvm::cl::init(TRANSPOSE_SIZE_AUTO)
         );

llvm::cl::opt<std::string>
CPUArch("cpuarch",
         llvm::cl::desc("CPU Architecture: auto, core2, corei7, corei7-avx, core-avx2, knc,"
#if defined ENABLE_KNL
         " knl,"
#else
         ""
#endif
         " skx"),
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
SDEEnabled("sde",
      llvm::cl::desc("Enables SDE version of SATest."),
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
         clEnumValN(VALIDATION,  "VAL",     "Validation mode"),
         clEnumValN(REFERENCE,   "REF",     "Reference mode"),
         clEnumValN(PERFORMANCE, "PERF",    "Performance mode"),
         clEnumValN(BUILD,       "BUILD",   "Build only mode")));

llvm::cl::opt<std::string>
PerformanceLog("csv-out",
               llvm::cl::desc("Output the performance measurement to the file <filename>. "
                   "If '-' filename is set the measurements data will be printed to the standard output stream."),
               llvm::cl::value_desc("filename"),
               llvm::cl::init("-"));

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
                   llvm::cl::init(16));

// turn on printing LLVM IR produced by Volcano after all optimization passes applied.
llvm::cl::opt<std::string>
OptimizedLLVMIRDumpFile("dump-llvm-file",
                        llvm::cl::desc("Prints LLVM IR to the file <filename> after all optimization passes applied. "
                            "If '-' filename is set the LLVM IR will be printed to the standard output stream."),
                        llvm::cl::value_desc("filename"),
                        llvm::cl::init(""));

// Enable printing additional information about comparison results.
llvm::cl::opt<bool>
DetailedStat("detailed_stat",
                 llvm::cl::desc("Print detailed statistics."),
                 llvm::cl::init(true));

// Enable VTune support in Volcano.
llvm::cl::opt<bool>
UseVTune("vtune",
        llvm::cl::desc("Enable VTune support in Volcano."),
        llvm::cl::init(false));

// Enable printing Volcano build log.
llvm::cl::opt<bool>
PrintBuildLog("build-log",
        llvm::cl::desc("Enable printing Volcano build log."),
        llvm::cl::init(false));

// turn on printing bytecode instructions after
llvm::cl::list<IRDumpOptions>
PrintIRAfter("dump-IR-after",
         llvm::cl::CommaSeparated,
         llvm::cl::desc("Print IR after specified optimization"),
         llvm::cl::values(
         clEnumValN(DUMP_IR_ALL,            "all",          "Print IR after each optimization"),
         clEnumValN(DUMP_IR_TARGERT_DATA,   "target_data",  "Print IR after target data pass"),
         clEnumValN(DUMP_IR_VECTORIZER,     "vectorizer",   "Print IR after vectorizer pass")));

// turn on printing bytecode instructions before
llvm::cl::list<IRDumpOptions>
PrintIRBefore("dump-IR-before",
         llvm::cl::CommaSeparated,
         llvm::cl::desc("Print IR before specified optimization"),
         llvm::cl::values(
         clEnumValN(DUMP_IR_ALL,            "all",          "Print IR before each optimization"),
         clEnumValN(DUMP_IR_TARGERT_DATA,   "target_data",  "Print IR before target data pass"),
         clEnumValN(DUMP_IR_VECTORIZER,     "vectorizer",   "Print IR before vectorizer pass")));

llvm::cl::opt<std::string>
DumpIRDir("dump-IR-dir",
           llvm::cl::ValueOptional,
           llvm::cl::desc("The directory for dumping IR files (if dump-IR flags are up, this directory will be used). "
                          "If '-' dirname isn't set, the files will be dumped to current directory."),
           llvm::cl::value_desc("dirname"),
           llvm::cl::init("./"));

llvm::cl::opt<bool>
DumpHeuristicIR("dump-heuristic-IR",
           llvm::cl::desc("Dump IR that is passed into the heuristic"),
           llvm::cl::init(false));

// turn on printing JIT
llvm::cl::opt<std::string>
DumpJIT("dump-JIT",
           llvm::cl::ValueOptional,
           llvm::cl::desc("Prints JIT code to the file <filename> after the build is complete. "
                          "The <filename> could be an absolute path or relative to the base directory."),
           llvm::cl::value_desc("filename"));

// Enable -time-passes in Volcano
llvm::cl::opt<std::string>
TimePasses("dump-time-passes",
           llvm::cl::ValueOptional,
           llvm::cl::desc("Generates compilation time detailed report for all the passes and print it to the file <filename>. "
                          "The <filename> could be an absolute path or relative to the base directory."),
           llvm::cl::value_desc("filename"));

// Seed for random input data generator
llvm::cl::opt<uint64_t>
RandomDGSeed("seed",
                   llvm::cl::desc("Seed for random input data generator. Zero seed means generate new one"),
                   llvm::cl::init(time(NULL)));

// Command line example:
// SATest.exe -config=test.cfg
// WARNING! To run OCL_CPU_DEVICE_BACKEND successfully built-in DLLs and RTLs is needed!

int main(int argc, char *argv[])
{
    // Parse received options
    llvm::cl::ParseCommandLineOptions(argc, argv,
        "Stand-alone test application to validate OCL/DX back-end.\n");

    try
    {
        // Run test
        IRunConfiguration* runConfig = RunnerFactory::GetInstance().CreateRunConfiguration();
        runConfig->InitFromCommandLine();

        SATest test(ConfigFile, BaseDirectory, runConfig);
        test.Run(TestMode, runConfig);
        return 0;
    }
    catch (Exception::InvalidEnvironmentException e)
    {
        // Exception of invalid execution environment of SATest
        std::cerr << "InvalidEnvironment exception occurred: " << e.what() << endl;
        return int(e.GetErrorCode());
    }
    catch (Exception::TestFailException e)
    {
        // Test does not match reference
        std::cerr << "Test Failed: " << e.what() << endl;
        return int(e.GetErrorCode());
    }
    catch (Exception::ValidationExceptionBase e)
    {
        // Exception occurred during test run process
        std::cerr << "Validation exception occurred: " << e.what() << endl;
        return int(e.GetErrorCode());
    }
    catch (Exceptions::DeviceBackendExceptionBase e)
    {
        // Exception occurred inside the back-end
        std::cerr << "Back-end exception occurred: "<< e.what() << endl;
        return int(e.GetErrorCode());
    }
}
