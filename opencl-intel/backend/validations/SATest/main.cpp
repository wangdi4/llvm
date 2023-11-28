// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#include "IRunConfiguration.h"
#include "SATest.h"
#include "SATestException.h"
#include "exceptions.h"
// Command line options
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DataTypes.h"
#include <iostream>
#include <memory>
#include <string>
#include <time.h>

using namespace llvm;
using namespace Validation;
using namespace Intel::OpenCL::DeviceBackend;

cl::opt<std::string> ConfigFile("config",
                                cl::desc("Test configuration file parameters."),
                                cl::value_desc("filename"));

cl::opt<std::string> ObjectFile("inject-object-file", cl::ValueOptional,
                                cl::desc("Execute pre-compiled object file"),
                                cl::value_desc("filename"));

// Enable VTune support.
cl::opt<bool> StopBeforeJIT("stop-before-jit",
                            cl::desc("Stops compilation after all optimization "
                                     "passes, but before binary generation."),
                            cl::init(false));

cl::opt<bool> Verbose("verbose", cl::desc("Print argument's metadata."),
                      cl::init(false));

cl::opt<std::string> BaseDirectory(
    "basedir",
    cl::desc("Base directory to use for configuration and data files lookup. "
             "Default value - configuration file directory."),
    cl::value_desc("basedir"));

cl::opt<bool> EnableSubgroupEmulation("enable-subgroup-emu",
                                      cl::desc("Enable subgroup emulation. "
                                               "Default value - true."),
                                      cl::init(true));

cl::opt<VectorizerType> OptVectorizerType(
    "vectorizer-type",
    cl::desc("Specify vectorizer type. "
             "Default value - default."),
    cl::values(clEnumValN(VPO_VECTORIZER, "vpo", "vplan vectorizer"),
               clEnumValN(DEFAULT_VECTORIZER, "default", "default vectorizer")),
    cl::init(DEFAULT_VECTORIZER));

cl::opt<ETransposeSize> TransposeSize(
    "tsize", cl::desc("Transpose size:"),
    cl::values(clEnumValN(TRANSPOSE_SIZE_AUTO, "0", "Automatic mode"),
               clEnumValN(TRANSPOSE_SIZE_1, "1", "Scalar"),
               clEnumValN(TRANSPOSE_SIZE_4, "4", "Vector4"),
               clEnumValN(TRANSPOSE_SIZE_8, "8", "Vector8"),
               clEnumValN(TRANSPOSE_SIZE_16, "16", "Vector16"),
               clEnumValN(TRANSPOSE_SIZE_32, "32", "Vector32"),
               clEnumValN(TRANSPOSE_SIZE_64, "64", "Vector64")),
    cl::init(TRANSPOSE_SIZE_NOT_SET));

cl::opt<std::string>
    CPUArch("cpuarch",
            cl::desc("CPU Architecture: auto, core2, corei7, corei7-avx, "
                     "core-avx2, knc, skx"),
            cl::init("auto"));

cl::opt<std::string> CPUFeatures("cpufeatures",
                                 cl::desc("CPU Features: -avx, +avx2, ..."),
                                 cl::init(""));

cl::opt<bool>
    NoRef("noref",
          cl::desc("Do not run reference, nor neat in validation mode"),
          cl::init(false));

cl::opt<bool> FlagForceRunReference("force_ref",
                                    cl::desc("Force running reference"),
                                    cl::init(false));

cl::opt<bool> SDEEnabled("sde", cl::desc("Enables SDE version of SATest."),
                         cl::init(false));

cl::opt<bool>
    TraceMarks("trace",
               cl::desc("Insert trace marks for SDE pinLIT traces generation"),
               cl::init(false));

cl::opt<bool> UseNEAT("neat", cl::desc("Use NEAT"), cl::init(false));

cl::opt<uint32_t> BuildIterations(
    "build-iterations",
    cl::desc("Iterations count to build the kernel in performance mode"),
    cl::value_desc("Number of iterations"), cl::init(1));

cl::opt<uint32_t> ExecuteIterations(
    "execute-iterations",
    cl::desc("Iterations count to execute the kernel in performance mode"),
    cl::value_desc("Number of iterations"), cl::init(1));

cl::opt<TEST_MODE>
    TestMode(cl::desc("Test mode:"),
             cl::values(clEnumValN(VALIDATION, "VAL", "Validation mode"),
                        clEnumValN(REFERENCE, "REF", "Reference mode"),
                        clEnumValN(PERFORMANCE, "PERF", "Performance mode"),
                        clEnumValN(BUILD, "BUILD", "Build only mode")));

cl::opt<std::string> PerformanceLog(
    "csv-out",
    cl::desc("Output the performance measurement to the file <filename>. "
             "If '-' filename is set the measurements data will be "
             "printed to the standard output stream."),
    cl::value_desc("filename"), cl::init("-"));

// turn on running single work group
cl::opt<bool> RunSingleWG("single_wg", cl::desc("Run only one work group."),
                          cl::init(false));

// tolerance used in Comparator for comparison of floating point numbers in
// accurate mode
cl::opt<double> ULP_tolerance("ulp_tol",
                              cl::desc("ULP tolerance for comparison"),
                              cl::init(0.0));

// Default Local work group size used in Validation mode
cl::opt<uint32_t> DefaultLocalWGSize(
    "default_wg_size",
    cl::desc("Default local work group size used in Validation mode"),
    cl::init(16));

// turn on printing LLVM IR produced after all optimization passes applied.
cl::opt<std::string> OptimizedLLVMIRDumpFile(
    "dump-llvm-file",
    cl::desc("Prints LLVM IR to the file <filename> after all "
             "optimization passes applied. "
             "If '-' filename is set the LLVM IR will be printed to the "
             "standard output stream."),
    cl::value_desc("filename"), cl::init(""));

// Enable printing additional information about comparison results.
cl::opt<bool> DetailedStat("detailed_stat",
                           cl::desc("Print detailed statistics."),
                           cl::init(true));

// Enable VTune support.
cl::opt<bool> UseVTune("vtune", cl::desc("Enable VTune support."),
                       cl::init(false));

// Enable printing build log.
cl::opt<bool> PrintBuildLog("build-log", cl::desc("Enable printing build log."),
                            cl::init(false));

// turn on printing bytecode instructions after
cl::opt<std::string>
    LLVMOption("llvm-option",
               cl::desc("A space-separated list of LLVM command line options"));

cl::opt<bool>
    DumpHeuristicIR("dump-heuristic-IR",
                    cl::desc("Dump IR that is passed into the heuristic"),
                    cl::init(false));

// turn on printing JIT
cl::opt<std::string> DumpJIT(
    "dump-JIT", cl::ValueOptional,
    cl::desc(
        "Prints JIT code to the file <filename> after the build is complete. "
        "The <filename> could be an absolute path or relative to the base "
        "directory."),
    cl::value_desc("filename"));

cl::opt<bool> DumpKernelProperty("dump-kernel-property",
                                 cl::desc("Dump kernel properties"),
                                 cl::init(false));

// Enable -time-passes option. Be careful that report is appended to the
// specified filename. It is advised to delete the file before RUN or specify
// "-" as filename.
cl::opt<std::string> TimePasses(
    "dump-time-passes", cl::ValueOptional,
    cl::desc("Generates compilation time detailed report for all the "
             "passes and append it to the file <filename>. "
             "The <filename> could be an absolute path or relative to "
             "the base directory."),
    cl::value_desc("filename"));

// Seed for random input data generator
cl::opt<uint64_t>
    RandomDGSeed("seed",
                 cl::desc("Seed for random input data generator. Zero "
                          "seed means generate new one"),
                 cl::init(time(NULL)));

cl::opt<unsigned>
    ExpensiveMemOpts("enable-expensive-mem-opts", cl::ValueOptional,
                     cl::desc("Enable expensive memory optimization. See "
                              "cl.cfg for value explanation"),
                     cl::init(0));

// Select pass manager type.
cl::opt<PassManagerType> OptPassManagerType(
    "pass-manager-type",
    cl::desc("Specify pass manager type. "
             "Default value - none."),
    cl::values(clEnumValN(PM_NONE, "none", "pass pipeline is not specified"),
               clEnumValN(PM_OCL, "ocl", "OpenCL new pass manager pipeline"),
               clEnumValN(PM_LTO, "lto", "llvm new pass manager pipeline")),
    cl::init(PM_NONE));

cl::opt<bool> SerializeWorkGroups(
    "serialize-work-groups",
    cl::desc("Serialize workgroups, i.e. they are executed sequentially "
             "by a single thread"),
    cl::init(false));

// Command line example:
// SATest.exe -config=test.cfg
// WARNING! To run OCL_CPU_DEVICE_BACKEND successfully built-in DLLs and RTLs is
// needed!

int main(int argc, char *argv[]) {
  // Parse received options
  cl::ParseCommandLineOptions(
      argc, argv, "Stand-alone test application to validate OCL back-end.\n");

  llvm_shutdown_obj obj;

  try {
    if (argc == 1)
      throw Exception::GeneralException("no input files");

    // Run test
    std::unique_ptr<IRunConfiguration> runConfig(
        RunnerFactory::GetInstance().CreateRunConfiguration());
    runConfig->InitFromCommandLine();

    SATest test(ConfigFile, BaseDirectory, runConfig.get());
    test.Run(TestMode, runConfig.get());
    return 0;
  } catch (Exception::InvalidEnvironmentException &e) {
    // Exception of invalid execution environment of SATest
    std::cerr << "InvalidEnvironment exception occurred: " << e.what()
              << std::endl;
    return int(e.GetErrorCode());
  } catch (Exception::TestFailException &e) {
    // Test does not match reference
    std::cerr << "Test Failed: " << e.what() << std::endl;
    return int(e.GetErrorCode());
  } catch (Exception::ValidationExceptionBase &e) {
    // Exception occurred during test run process
    std::cerr << "Validation exception occurred: " << e.what() << std::endl;
    return int(e.GetErrorCode());
  } catch (Exceptions::DeviceBackendExceptionBase &e) {
    // Exception occurred inside the back-end
    std::cerr << "Back-end exception occurred: " << e.what() << std::endl;
    return int(e.GetErrorCode());
  }
}
