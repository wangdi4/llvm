#include <CL/cl.h>
#include <string>
#include <Windows.h>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <windows.h>
using std::string;
using namespace std;

#define CPU 0
#define GPU 1
#define MIC 2
#define SUB1 3
#define SUB2 4



// Report about an OpenCL problem.
// Macro is used instead of a function here
// to report source file name and line number.
// Message box is displayed to get error on screenshot
#define checkOpenCLErrors(ERR)														\
    if(ERR != CL_SUCCESS)														\
    {																			\
		string ttt="OpenCL error "	+ opencl_error_to_str(ERR) +				\
            " happened in file " + to_str(__FILE__) +							\
            " at line " + to_str(__LINE__) + "." ;								\
		MessageBoxA(	NULL,(LPCSTR)ttt.c_str(),									\
					(LPCSTR)"Error during OpenCL code execution", MB_ICONERROR|MB_OK);	\
    }

// Report about an host problem.
// Macro is used instead of a function here
// to report source file name and line number.
// Message box is displayed to get error on screenshot
#define checkHostErrors(FAIL,MSG)														\
    if(FAIL)														\
    {																			\
		string ttt="Host error "	+ to_str(MSG) +				\
            " happened in file " + to_str(__FILE__) +							\
            " at line " + to_str(__LINE__) + "." ;								\
		MessageBoxA(	NULL,(LPCSTR)ttt.c_str(),									\
					(LPCSTR)"Error during host code execution", MB_ICONERROR|MB_OK);	\
    }

// Convert from a value of a given type to string with optional formatting.
// T should have operator<< defined to be written to stream.
template <typename T> string to_str (const T x, std::streamsize width = 0, char fill = ' ')
{
    ostringstream os;
    os << setw(width) << setfill(fill) << x;
    if(!os)
    {
		checkHostErrors(true,"Cannot represent object as a string");
		throw std::runtime_error("Cannot represent object as a string");
    }
    return os.str();
}

string opencl_error_to_str (cl_int error);
void delay();
cl_uint requiredOpenCLAlignment (cl_device_id device);
void* aligned_malloc (size_t size, size_t alignment);
vector<char> readFile(string file_name);
cl_kernel create_kernel(string file_name, string kernel_name, cl_context context, cl_device_id device, cl_program *);
cl_mem create_float_buffer(cl_context context, cl_device_id device, size_t size_buffer, cl_mem_flags flags,cl_float**   pp_buf );