#include "host_program_common.h"
#include "test_utils.h"
#include "cl_utils.h"
#include "test_pipe_thread.h"
#include "cl_user_logger.h"
#include <iostream>
#include <cstdio>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>

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

namespace Intel { namespace OpenCL { namespace Utils {

FrameworkUserLogger* g_pUserLogger = NULL;

}}}

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
        for (vector<string>::const_iterator flag_i = flags.begin();
                flag_i != flags.end(); ++flag_i) {
            // Split flag to key=value pair. Ignore invalidly structured flags
            vector<string> keyval = tokenize(*flag_i, "=");
            if (keyval.size() != 2)
                continue;
            m_options[keyval[0]] = keyval[1];
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
    throw runtime_error("Unknown host program: '" + name + "'");
}


int main(int argc, char** argv)
{
    int rc;
    DTT_LOG("<---------------------------------");
    DTT_LOG("Starting debug_test_type");

#ifdef _WIN32
    // Create a thread that will simulate client configuration write
    NamedPipeThread *thread = new NamedPipeThread();
    thread->Start();

    // Allow thread to run (write the data)
    Sleep(1000);
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

        cl::Context context(CL_DEVICE_TYPE_CPU, properties);
        vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
        if (devices.size() == 0)
            throw runtime_error("0 devices found");

        cl::Program::Sources sources(1,
            make_pair(cl_code.c_str(), cl_code.size()));
        cl::Program prog = cl::Program(context, sources);

        // Build the program with debug arguments enabled.
        // In case of a build error, we can provide more information on the
        // failure from the build log.
        //
        try {
            string build_flags = "";
            if (options.get("debug_build") != "off") {
                build_flags += "-g ";
            }
            build_flags += string("-s \"") + cl_file_name + "\"";
            prog.build(devices, build_flags.c_str());
        }
        catch (cl::Error err) {
            string buildlog;
            prog.getBuildInfo(devices[0], CL_PROGRAM_BUILD_LOG, &buildlog);
            cerr << "Build error, log:\n" << buildlog << endl;
            throw;
        }

        cl::Kernel kernel(prog, "main_kernel");

        DTT_LOG("Running host program...");

        // Invoke the host program with the built kernel
        //
        host_program(context, devices[0], kernel, args);

        DTT_LOG("Host program finished");
        rc = 0;
    }
    catch (const cl::Error& err) {
        cerr << "Error: " << err.what() << "(" << err.err() << ")\n";
#ifdef _WIN32
        fprintf(stderr, "ClErrTxt error: %ws\n", ClErrTxt(err.err()));
#else
        fprintf(stderr, "ClErrTxt error: %ls\n", ClErrTxt(err.err()));
#endif // _WIN32
        rc = 1;
    }
    catch (const runtime_error& err) {
        cerr << "Error: " << err.what() << endl;
        rc = 1;
    }

#ifdef _WIN32
    // Wait for thread to finish
    thread->Join();
#endif _WIN32

    DTT_LOG("debug_test_type done.");
    DTT_LOG("--------------------------------->");
    return rc;
}
