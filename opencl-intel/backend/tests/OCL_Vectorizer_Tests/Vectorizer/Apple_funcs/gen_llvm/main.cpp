#include <iostream>
#include <fstream>
#include <vector>
#include <OpenCL/OpenCL.h>
#include <assert.h>
#include <sys/stat.h>
#include <time.h>

#define CHECK_ERRORS(err) if (err != CL_SUCCESS) {cout << "Error (" << err << ") in line " << __LINE__ << "  In file:" << __FILE__ << endl; exit(-1);}
#define CHECK_VALID(var) if (var == NULL) {cout << "Error! Variable is NULL, in line " << __LINE__ << "  In file:" << __FILE__ << endl; exit(-1);}

using namespace std;

unsigned int size = 32;

cl_device_type use_device = CL_DEVICE_TYPE_CPU;
cl_command_queue commandQueue;
cl_context context;



void createEnv()
{
	cl_device_id dev; 
	cl_int err;
	err = clGetDeviceIDs(NULL, use_device, 1, &dev, NULL);
	CHECK_ERRORS(err);
	context = clCreateContext(0, 1, &dev, NULL, NULL, &err);
	CHECK_VALID(context); 
	CHECK_ERRORS(err);
	
	commandQueue = clCreateCommandQueue(context, dev, 0, &err);
	CHECK_ERRORS(err);
}	


char* loadProgramFromFile(const char* fileName, size_t * clCodeStrSize)
{
	// Load OpenCL source file
	struct stat stbuf;
	if(stat(fileName, &stbuf) == -1) {
		cerr << "Could not stat file '" << fileName << "'\n";
		return NULL;
	}
	*clCodeStrSize = stbuf.st_size;
	char *clCodeStr = new char[*clCodeStrSize + 1];
	assert(clCodeStr);
	
	ifstream inFile(fileName);
	if (!inFile) {
		cerr << "Failed to open '" << fileName << "'\n" << endl;
		return 0;
	}
	inFile.read(clCodeStr, *clCodeStrSize);
	clCodeStr[*clCodeStrSize] = '\0';
	inFile.close();
	
	return clCodeStr;
}




int main (int argc, char * const argv[]) 
{
	size_t clCodeStrSize;
	cl_int err;
	int retval;
	
	createEnv();
		
	// Create the openCL program
	char *program_source = loadProgramFromFile(argv[1], &clCodeStrSize);
	if (program_source == NULL) {
		cerr << "Fail !"<< endl;
		return 0;
	}

	cl_program myProg = clCreateProgramWithSource(context, 1, (const char**)&program_source, &clCodeStrSize, &err);
	CHECK_ERRORS(err);
	err = clBuildProgram(myProg, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
		cout << "clBuildProgramExecutable returned = " << err << endl;
		char buildLog[1024];
		cl_device_id dev;
		err = clGetDeviceIDs(NULL, use_device, 1, &dev, NULL);
		CHECK_ERRORS(err);
		err = clGetProgramBuildInfo(myProg, dev, CL_PROGRAM_BUILD_LOG, 1024, buildLog, NULL);
		CHECK_ERRORS(err);
		cout << "buildLog = \n\t" << buildLog << endl;
		exit (-1);
	} 
		
	return retval;
}