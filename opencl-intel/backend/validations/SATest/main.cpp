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

// Enable VTune support.
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

llvm::cl::opt<bool>
EnableSubgroupEmulation("enable-subgroup-emu",
                llvm::cl::desc("Enable subgroup emulation. "
                               "Default value - true."),
                llvm::cl::init(true));

llvm::cl::opt<VectorizerType>
OptVectorizerType("vectorizer-type",
               llvm::cl::desc("Specify vectorizer type. "
                              "Default value - default."),
               llvm::cl::values(
                clEnumValN(VPO_VECTORIZER,     "vpo",     "vplan vectorizer"),
                clEnumValN(DEFAULT_VECTORIZER, "default",  "default vectorizer")
               ),
               llvm::cl::init(DEFAULT_VECTORIZER)
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
         clEnumValN(TRANSPOSE_SIZE_32,   "32",   "Vector32"),
         clEnumValN(TRANSPOSE_SIZE_64,   "64",   "Vector64")),
         llvm::cl::init(TRANSPOSE_SIZE_NOT_SET)
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

// turn on printing LLVM IR produced after all optimization passes applied.
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

// Enable VTune support.
llvm::cl::opt<bool>
UseVTune("vtune",
        llvm::cl::desc("Enable VTune support."),
        llvm::cl::init(false));

// Enable printing build log.
llvm::cl::opt<bool>
PrintBuildLog("build-log",
        llvm::cl::desc("Enable printing build log."),
        llvm::cl::init(false));

// turn on printing bytecode instructions after
llvm::cl::opt<std::string> LLVMOption(
    "llvm-option",
    llvm::cl::desc("A space-separated list of LLVM command line options"));

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

llvm::cl::opt<bool> DumpKernelProperty("dump-kernel-property",
                                       llvm::cl::desc("Dump kernel properties"),
                                       llvm::cl::init(false));

// Enable -time-passes option. Be careful that report is appended to the
// specified filename. It is advised to delete the file before RUN or specify
// "-" as filename.
llvm::cl::opt<std::string> TimePasses(
    "dump-time-passes", llvm::cl::ValueOptional,
    llvm::cl::desc("Generates compilation time detailed report for all the "
                   "passes and append it to the file <filename>. "
                   "The <filename> could be an absolute path or relative to "
                   "the base directory."),
    llvm::cl::value_desc("filename"));

// Debugging pass manager.
llvm::cl::opt<std::string> DebugPassManager(
    "debug-passes",
    llvm::cl::desc("Enable -debug-pass for legacy pass manager"));

// Seed for random input data generator
llvm::cl::opt<uint64_t>
RandomDGSeed("seed",
                   llvm::cl::desc("Seed for random input data generator. Zero seed means generate new one"),
                   llvm::cl::init(time(NULL)));

llvm::cl::opt<unsigned>
ExpensiveMemOpts("enable-expensive-mem-opts",
             llvm::cl::ValueOptional,
             llvm::cl::desc("Enable expensive memory optimization. See cl.cfg for value explanation"),
             llvm::cl::init(0));

// Select pass manager type.
llvm::cl::opt<PassManagerType> OptPassManagerType(
    "pass-manager-type",
    llvm::cl::desc("Specify pass manager type. "
                   "Default value - none."),
    llvm::cl::values(
        clEnumValN(PM_NONE, "none", "pass pipeline is not specified"),
        clEnumValN(PM_OCL_LEGACY, "ocl-legacy", "OpenCL pass pipeline"),
        clEnumValN(PM_OCL, "ocl", "OpenCL new pass manager pipeline"),
        clEnumValN(PM_LTO_LEGACY, "lto-legacy", "llvm legacy pass pipeline"),
        clEnumValN(PM_LTO, "lto", "llvm new pass manager pipeline")),
    llvm::cl::init(PM_NONE));

llvm::cl::opt<bool> SerializeWorkGroups(
    "serialize-work-groups",
    llvm::cl::desc("Serialize workgroups, i.e. they are executed sequentially "
                   "by a single thread"),
    llvm::cl::init(false));

// Command line example:
// SATest.exe -config=test.cfg
// WARNING! To run OCL_CPU_DEVICE_BACKEND successfully built-in DLLs and RTLs is needed!

int main(int argc, char *argv[])
{
    // Parse received options
    llvm::cl::ParseCommandLineOptions(argc, argv,
        "Stand-alone test application to validate OCL back-end.\n");

    try
    {
        if (argc == 1)
          throw Exception::GeneralException("no input files");

        // Run test
        IRunConfiguration* runConfig = RunnerFactory::GetInstance().CreateRunConfiguration();
        runConfig->InitFromCommandLine();

        SATest test(ConfigFile, BaseDirectory, runConfig);
        test.Run(TestMode, runConfig);
        return 0;
    }
    catch (Exception::InvalidEnvironmentException& e)
    {
        // Exception of invalid execution environment of SATest
        std::cerr << "InvalidEnvironment exception occurred: " << e.what() << endl;
        return int(e.GetErrorCode());
    }
    catch (Exception::TestFailException& e)
    {
        // Test does not match reference
        std::cerr << "Test Failed: " << e.what() << endl;
        return int(e.GetErrorCode());
    }
    catch (Exception::ValidationExceptionBase& e)
    {
        // Exception occurred during test run process
        std::cerr << "Validation exception occurred: " << e.what() << endl;
        return int(e.GetErrorCode());
    }
    catch (Exceptions::DeviceBackendExceptionBase& e)
    {
        // Exception occurred inside the back-end
        std::cerr << "Back-end exception occurred: "<< e.what() << endl;
        return int(e.GetErrorCode());
    }
}
