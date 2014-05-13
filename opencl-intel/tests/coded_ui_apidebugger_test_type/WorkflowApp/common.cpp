#include "common.h"
#include <windows.h>
string opencl_error_to_str (cl_int error)
{
#define CASE_CL_CONSTANT(NAME) case NAME: return #NAME;

    // Suppose that no combinations are possible.
    // TODO: Test whether all error codes are listed here
    switch(error)
    {
        CASE_CL_CONSTANT(CL_SUCCESS)
        CASE_CL_CONSTANT(CL_DEVICE_NOT_FOUND)
        CASE_CL_CONSTANT(CL_DEVICE_NOT_AVAILABLE)
        CASE_CL_CONSTANT(CL_COMPILER_NOT_AVAILABLE)
        CASE_CL_CONSTANT(CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CASE_CL_CONSTANT(CL_OUT_OF_RESOURCES)
        CASE_CL_CONSTANT(CL_OUT_OF_HOST_MEMORY)
        CASE_CL_CONSTANT(CL_PROFILING_INFO_NOT_AVAILABLE)
        CASE_CL_CONSTANT(CL_MEM_COPY_OVERLAP)
        CASE_CL_CONSTANT(CL_IMAGE_FORMAT_MISMATCH)
        CASE_CL_CONSTANT(CL_IMAGE_FORMAT_NOT_SUPPORTED)
        CASE_CL_CONSTANT(CL_BUILD_PROGRAM_FAILURE)
        CASE_CL_CONSTANT(CL_MAP_FAILURE)
        CASE_CL_CONSTANT(CL_MISALIGNED_SUB_BUFFER_OFFSET)
        CASE_CL_CONSTANT(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        CASE_CL_CONSTANT(CL_INVALID_VALUE)
        CASE_CL_CONSTANT(CL_INVALID_DEVICE_TYPE)
        CASE_CL_CONSTANT(CL_INVALID_PLATFORM)
        CASE_CL_CONSTANT(CL_INVALID_DEVICE)
        CASE_CL_CONSTANT(CL_INVALID_CONTEXT)
        CASE_CL_CONSTANT(CL_INVALID_QUEUE_PROPERTIES)
        CASE_CL_CONSTANT(CL_INVALID_COMMAND_QUEUE)
        CASE_CL_CONSTANT(CL_INVALID_HOST_PTR)
        CASE_CL_CONSTANT(CL_INVALID_MEM_OBJECT)
        CASE_CL_CONSTANT(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
        CASE_CL_CONSTANT(CL_INVALID_IMAGE_SIZE)
        CASE_CL_CONSTANT(CL_INVALID_SAMPLER)
        CASE_CL_CONSTANT(CL_INVALID_BINARY)
        CASE_CL_CONSTANT(CL_INVALID_BUILD_OPTIONS)
        CASE_CL_CONSTANT(CL_INVALID_PROGRAM)
        CASE_CL_CONSTANT(CL_INVALID_PROGRAM_EXECUTABLE)
        CASE_CL_CONSTANT(CL_INVALID_KERNEL_NAME)
        CASE_CL_CONSTANT(CL_INVALID_KERNEL_DEFINITION)
        CASE_CL_CONSTANT(CL_INVALID_KERNEL)
        CASE_CL_CONSTANT(CL_INVALID_ARG_INDEX)
        CASE_CL_CONSTANT(CL_INVALID_ARG_VALUE)
        CASE_CL_CONSTANT(CL_INVALID_ARG_SIZE)
        CASE_CL_CONSTANT(CL_INVALID_KERNEL_ARGS)
        CASE_CL_CONSTANT(CL_INVALID_WORK_DIMENSION)
        CASE_CL_CONSTANT(CL_INVALID_WORK_GROUP_SIZE)
        CASE_CL_CONSTANT(CL_INVALID_WORK_ITEM_SIZE)
        CASE_CL_CONSTANT(CL_INVALID_GLOBAL_OFFSET)
        CASE_CL_CONSTANT(CL_INVALID_EVENT_WAIT_LIST)
        CASE_CL_CONSTANT(CL_INVALID_EVENT)
        CASE_CL_CONSTANT(CL_INVALID_OPERATION)
        CASE_CL_CONSTANT(CL_INVALID_GL_OBJECT)
        CASE_CL_CONSTANT(CL_INVALID_BUFFER_SIZE)
        CASE_CL_CONSTANT(CL_INVALID_MIP_LEVEL)
        CASE_CL_CONSTANT(CL_INVALID_GLOBAL_WORK_SIZE)
        CASE_CL_CONSTANT(CL_INVALID_PROPERTY)

    default:
        return "UNKNOWN ERROR CODE " + to_str(error);
    }

#undef CASE_CL_CONSTANT
}

vector<char> readFile(string file_name){
	ifstream program_file(file_name.c_str());
	program_file.seekg(0,ifstream::end);
	 
	streamoff file_length = program_file.tellg();
	program_file.seekg(0,ifstream::beg);

    checkHostErrors( (file_length == -1) ,"Cannot determine the length of file " + string(file_name) );
	vector<char> program_text_prepared;

	program_text_prepared.resize(static_cast<size_t>(file_length) + 1);
	program_file.read(&program_text_prepared[0], file_length);
	return program_text_prepared;
}

cl_kernel create_kernel(string file_name, string kernel_name, cl_context context, cl_device_id device, cl_program * prog){
	cl_int err;
	vector <char> program_text_prepared = readFile(file_name);

	const char* raw_text = &program_text_prepared[0];
	*prog = clCreateProgramWithSource(context, 1, &raw_text, 0, &err);
	checkOpenCLErrors(err);
	char cwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, cwd);
	const string build_options("-g -s \"" + string(cwd)+"\\..\\..\\"+file_name+"\"");
	err = clBuildProgram(*prog, 1, &device, build_options.c_str(), 0, 0);
	checkOpenCLErrors(err);
	cl_kernel kernel1 = clCreateKernel(*prog, kernel_name.c_str(), &err);
    checkOpenCLErrors(err);
	return kernel1;
}

cl_mem create_float_buffer(cl_context context, cl_device_id device, size_t size_buffer, cl_mem_flags flags,cl_float**   pp_buf ){
	cl_uint alignment= requiredOpenCLAlignment(device);
	cl_int err;

	//size_t size_buffer=1000;
	size_t size_array=size_buffer*sizeof(cl_float);

	*pp_buf = (cl_float*)aligned_malloc(size_array,alignment);
	checkHostErrors((!(*pp_buf) ), "Failed to allocate array to store buffers");

	for (size_t i=0; i< size_buffer; i++) {
		pp_buf[0][i]=(cl_float)i;
	}
	/*CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR*/
	cl_mem cl_buffer = clCreateBuffer(context, flags, size_array, *pp_buf, &err);
    checkOpenCLErrors(err);
    checkHostErrors((cl_buffer == (cl_mem)0), ("Failed to create input buffer!"));
	return cl_buffer;
}
void delay(){
	Sleep(1000);/*static string tmp("10");
    int cycles=500000;
	for (int i=0;i<cycles;i++){ 
		stringstream ss;
		ss<<(atoi(tmp.c_str())%12345+1);
		tmp=string(ss.str());
	}*/
}

cl_uint requiredOpenCLAlignment (cl_device_id device)
{
    cl_uint result = 0;
    cl_int err = clGetDeviceInfo(
        device,
        CL_DEVICE_MEM_BASE_ADDR_ALIGN,
        sizeof(result),
        &result,
        0
        );
    checkOpenCLErrors(err);
	checkHostErrors(!(result%8 == 0), "Bad alignment");
    return result/8;    // clGetDeviceInfo returns value in bits, convert it to bytes
}
void* aligned_malloc (size_t size, size_t alignment)
{
    // a number of requirements should be met
    checkHostErrors(!((alignment > 0)||((alignment & (alignment - 1)) == 0)),"Passed allignment does not follow expectations");

    if(alignment < sizeof(void*))
    {
        alignment = sizeof(void*);
    }

    checkHostErrors(!(size >= sizeof(void*)), "Incorrect buffer size - first check.");
    checkHostErrors(!(size/sizeof(void*)*sizeof(void*) == size), "Incorrect buffer size - second check.");

    // allocate extra memory and convert to size_t to perform calculations
    char* orig = new char[size + alignment + sizeof(void*)];
    // calculate an aligned position in the allocated region
    // assumption: (size_t)orig does not lose lower bits
    char* aligned =
        orig + (
        (((size_t)orig + alignment + sizeof(void*)) & ~(alignment - 1)) -
        (size_t)orig
        );
    // save the original pointer to use it in aligned_free
    *((char**)aligned - 1) = orig;
    return aligned;
}
