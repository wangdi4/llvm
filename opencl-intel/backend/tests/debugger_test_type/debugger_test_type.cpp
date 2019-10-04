// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "host_program_common.h"
#include "test_utils.h"
#include "cl_utils.h"
#include "test_pipe_thread.h"
#include "cl_user_logger.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

// External declarations of host programs defined elsewhere.
//
extern HostProgramFunc host_1d_inout;
extern HostProgramFunc host_ndrange_inout;
extern HostProgramFunc host_multi_ndrange;
extern HostProgramFunc host_printf_tester;
extern HostProgramFunc host_1d_float4_with_size;
extern HostProgramFunc host_struct_kernel_arg;
extern HostProgramFunc host_images_and_struct;
extern HostProgramFunc host_local_global;
extern HostProgramFunc host_task;
extern HostProgramFunc host_fpga_host_side_pipes;
extern HostProgramFunc host_fpga_channels;
extern HostProgramFunc host_fpga_autorun;
extern HostProgramFunc host_fpga_fp16;
extern HostProgramFunc host_compile_link;

namespace Intel { namespace OpenCL { namespace Utils {

FrameworkUserLogger* g_pUserLogger = NULL;

}}}

// List of tests which should be launched only on FPGA Emulator
vector<string> FPGATests = {
    "fpga_host_side_pipes",
    "fpga_channels",
    "fpga_autorun",
    "fpga_fp16"
};

// List of tests which should not be launched on FPGA Emulator
vector<string> NonFPGATests = {
    "images_and_struct" // FPGA Emulator does not support image
};

// Encapsulates options parsed from a flag string.
// Initialize with a flag string, then use the 'get' method to obtain values
// of flags by name.
//
class DTTOptions
{
public:
    DTTOptions(const string& flags_str) {
        if (flags_str == "none")
            return;
        vector<string> flags = tokenize(flags_str, ",");
        for (auto &flag : flags) {
            // Split flag to key=value pair. Ignore invalidly structured flags
            const size_t i = flag.find_first_of("=");
            if (i != string::npos)
              m_options[flag.substr(0, i)] =
                flag.substr(i + 1, flag.size() - i - 1);
        }
    }

    string get(const string& key, const string& dflt="")
    {
        map<string, string>::const_iterator val_i = m_options.find(key);
        if (val_i == m_options.end())
            return dflt;
        else
            return val_i->second;
    }

private:
    map<string, string> m_options;
};


// Convenience utility for placing the command-line arguments into a vector of
// strings. Only the command-line arguments are returned (the program name
// is not).
//
vector<string> read_commandline(int argc, char** argv)
{
    vector<string> args;
    for (int i = 1; i < argc; ++i)
        args.push_back(argv[i]);
    return args;
}


// Get the host program function from a name. Throw runtime_error in case of
// errors.
//
HostProgramFunc get_host_program_by_name(string name)
{
    if (name == "1d_inout")
        return host_1d_inout;
    else if (name == "ndrange_inout")
        return host_ndrange_inout;
    else if (name == "multi_ndrange")
        return host_multi_ndrange;
    else if (name == "printf_tester")
        return host_printf_tester;
    else if (name == "1d_float4_with_size")
        return host_1d_float4_with_size;
    else if (name == "struct_kernel_arg")
        return host_struct_kernel_arg;
    else if (name == "images_and_struct")
        return host_images_and_struct;
    else if (name == "local_global")
        return host_local_global;
    else if (name == "task")
        return host_task;
    else if (name == "fpga_host_side_pipes")
        return host_fpga_host_side_pipes;
    else if (name == "fpga_channels")
        return host_fpga_channels;
    else if (name == "fpga_autorun")
        return host_fpga_autorun;
    else if (name == "fpga_fp16")
        return host_fpga_fp16;
    else if (name == "host_compile_link")
        return host_compile_link;

    throw runtime_error("Unknown host program: '" + name + "'");
}
#ifdef _WIN32
volatile char cdbGWorkitemInjectionBuffer[128] = { 0 };

// This is required, because CDB doesn't provide a facility
// to change the environment variables, like gdb.
// This function will return the injected string from cdbGWorkitemInjectionBuffer.
const char* cdbInjectWorkitemFocus()
{
    string gworkitem;
    stringstream filename;
    filename << "workitemfocus_inject_" << GetCurrentProcessId() << ".tmp";
    ifstream injectionfile(filename.str());
    if (injectionfile.is_open())
    {
        getline(injectionfile, gworkitem);
        strcpy((char*)cdbGWorkitemInjectionBuffer, gworkitem.c_str());
    }

    if (cdbGWorkitemInjectionBuffer[0] == '(')
        return (const char*)cdbGWorkitemInjectionBuffer;
    else
        return "";
}
#endif
const char* getWorkitemFocus()
{
#ifdef _WIN32
    const char* envValCdb = cdbInjectWorkitemFocus();
    if (envValCdb[0] != 0)
        return envValCdb;
#endif
    return getenv("GWORKITEM");
}

int main(int argc, char** argv)
{
    int rc;
    DTT_LOG("<---------------------------------");
    DTT_LOG("Starting debug_test_type");

#ifdef _WIN32
    char* envVar = getenv("CL_CONFIG_USE_NATIVE_DEBUGGER");
    bool useNativeDebugger = (envVar && *envVar == '1');
    NamedPipeThread *thread = nullptr;
    if (!useNativeDebugger)
    {
        // Create a thread that will simulate client configuration write
        thread = new NamedPipeThread();
        thread->Start();

        // Allow thread to run (write the data)
        Sleep(1000);
    }
#endif
    vector<string> args = read_commandline(argc, argv);

    if (args.size() < 3) {
        cerr << "Error: expected arguments: ";
        cerr << "<host program name> <options> <cl file name> [extra arguments]\n";
        cerr << "\n\toptions: comma-separated list of key=value pairs\n";
        return 1;
    }

    try {
        string host_program_name = args[0];
        HostProgramFunc host_program = get_host_program_by_name(host_program_name);

        string flags = args[1];
        DTTOptions options = DTTOptions(flags);

        string cl_file_name = args[2];
        string cl_code = ReadFileContents(cl_file_name);
        if (cl_code.size() == 0)
            throw runtime_error("Can't read kernel from file: " + cl_file_name);
        args.erase(args.begin(), args.begin() + 3);

        DTT_LOG("Host program name: " + host_program_name);
        DTT_LOG("Options: " + flags);
        DTT_LOG("CL file name: " + cl_file_name);

        string s = "Extra arguments: ";
        if (args.size() == 0)
            s += "<none>";
        else {
            vector<string>::const_iterator i = args.begin(), end = args.end();
            for (; i != args.end(); ++i)
                s += *i + " ";
        }
        DTT_LOG(s);

        DTT_LOG("Preparing OpenCL execution...");
        vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platforms.size() == 0)
            throw runtime_error("0 platforms found");

        cl_context_properties properties[] = {
            CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0
        };

        cl_device_type deviceType = CL_DEVICE_TYPE_DEFAULT;

        if (options.get("device") == "fpga_fast_emu")
            deviceType = CL_DEVICE_TYPE_ACCELERATOR;
        else if (options.get("device") == "cpu")
            deviceType = CL_DEVICE_TYPE_CPU;
        else if (!options.get("device").empty())
          throw runtime_error("Wrong device type specified."
                              "Please, specify 'fpga_fast_emu' or 'cpu'");

        cl::Context context(deviceType, properties);

        vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

        // Ensure that tests will be run on CPU or ACCELERATOR
        devices[0].getInfo(CL_DEVICE_TYPE, &deviceType);
        if (deviceType != CL_DEVICE_TYPE_CPU &&
            deviceType != CL_DEVICE_TYPE_ACCELERATOR)
            throw runtime_error("Wrong device type."
                                "Only CPU or ACCELERATOR types supported");

        // Skip fpga-specific tests on CPU
        if (deviceType != CL_DEVICE_TYPE_ACCELERATOR &&
            find(FPGATests.begin(),
                 FPGATests.end(), host_program_name) != FPGATests.end()) {
            DTT_LOG("Not a FGPA emulator device. Skip the fpga-spicific test");
            return 0;
        }

        // Skip non-fpga tests on FPGA Emulator
        if (deviceType == CL_DEVICE_TYPE_ACCELERATOR &&
            find(NonFPGATests.begin(), NonFPGATests.end(),
                 host_program_name) != NonFPGATests.end()) {
            DTT_LOG("FGPA emulator device. Skip the non-fpga test");
            return 0;
        }

        cl::Program::Sources sources(1,
            make_pair(cl_code.c_str(), cl_code.size()));
        cl::Program prog = cl::Program(context, sources);

        // Build the program with debug arguments enabled.
        // In case of a build error, we can provide more information on the
        // failure from the build log.
        //
        try {
            // Make possible to pass any build options to tests
            string build_flags = options.get("build_opts") + " ";
            const char* gworkitem = getWorkitemFocus();
            if (gworkitem) {
                build_flags += "-gworkitem=";
                build_flags += gworkitem;
                build_flags += " ";
            }
            if (options.get("debug_build") != "off") {
                build_flags += "-g ";
            }
            build_flags += string("-s \"") + cl_file_name + "\"";
            DTT_LOG("Build flags: " + build_flags);
            prog.build(devices, build_flags.c_str());
        }
        catch (cl::Error err) {
            string buildlog;
            prog.getBuildInfo(devices[0], CL_PROGRAM_BUILD_LOG, &buildlog);
            cerr << "Build error, log:\n" << buildlog << endl;
            throw;
        }


        DTT_LOG("Running host program...");

        // Invoke the host with the built program
        host_program(context, devices[0], prog, args);

        DTT_LOG("Host program finished");
        rc = 0;
    }
    catch (const cl::Error& err) {
        cerr << "Error: " << err.what() << "(" << err.err() << ")\n";
#ifdef _WIN32
        fprintf(stderr, "ClErrTxt error: %ws\n", ClErrTxt(err.err()));
#else
        fprintf(stderr, "ClErrTxt error: %s\n", ClErrTxt(err.err()));
#endif // _WIN32
        rc = 1;
    }
    catch (const runtime_error& err) {
        cerr << "Error: " << err.what() << endl;
        rc = 1;
    }

#ifdef _WIN32
    if (!useNativeDebugger)
    {
        // Wait for thread to finish
        thread->Join();
    }
#endif

    DTT_LOG("debug_test_type done.");
    DTT_LOG("--------------------------------->");
    return rc;
}
