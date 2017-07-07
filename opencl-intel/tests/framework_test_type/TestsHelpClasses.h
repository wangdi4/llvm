#pragma once
/*
* This file holds all of the functions/classes and macros that can help you make a better,easier tests.
*
*/

#include <gtest/gtest.h>
#include "CL/cl.h"
#include "FrameworkTest.h"
#include <sys/stat.h>
#include <fstream>
#include <iostream>

// used to make a macro definition into a string simply call XSTR with you defined macro "s"
#define XSTR(s) STR(s)
#define STR(s) #s



#define MAX_SOURCE_SIZE 100000//(0x100000)
#define WORK_SIZE 400

// just to help eliminate some DERY boilerplate code
#define NullCheckAndExecute(check,command) if ((check)!= NULL ) command

//================ helper functions ===================================
/*
* opens a file using fopen like function, works on windows an linux
* filename - the name of the file
* optns - options (see fopen)
* file - the pointer returned with the file
* return an int with the error type of the problem, 0 means success
*/
int crossPlatformFOpen(char* fileName, const char* optns, FILE* file);

#define FOPEN_OPTIONS rbD  //read,binary,Temporary , remove D so binary file will not be deleted

/*
* return true if the file exists or false otherwise
*/
inline bool checkFileExistence(char* fileName);
/*
* check if 2 text files are equals or not, if so google test will fail in this function 
* and will produce the 2 different lines (in case you asked for equal
*	fileName1,fileName2 - the files names to be checked
*	isEqual - true if you want to check they are equal and false if you want to check they are different
*/
void validateEqualityOfFiles(string fileName1, string fileName2, bool isEqual, int linesToSkip);

void validateSubstringInFile(string fileName, string subString, bool doesExist);
/*
* run Ioc from the command prompt
* arguments - all the commands and options for Ioc
*/
void runIoc(string arguments);
/*
* just delete all the files you give him, if they exists
* files[] - all the file names to be removed
* num - the length of the array.
*/
void removeFiles(string files[], int num);



/*
* google test base class for OpenCL framework tests, just inherit from him and use TEST_F
* this class does all the boilerplate code of openCL include creating all of the base object
*/
class baseOcl : public testing::Test 
{
public:
	baseOcl();
	~baseOcl();
	/* 
	* default SetUpTestCase Procedure, Override for other purposes 
	* the default one initialize the basic objects, and they are shared for all the test suite
	* for time saving
	*/
	static void SetUpTestCase();
	static void TearDownTestCase();
	void TearDown();

	/*
	* initialize the Platform object, used by the default set up
	*/
	static void initPlatform();
	/*
	* initialize the Device object, used by the default set up
	*/
	static void initDevices();
	/*
	* initialize the Context object, used by the default set up
	*/
	static void initContext();
	/*
	* initialize the Command Queue object, used by the default set up
	*/
	static void initCommandQueue();
	//static field because they are shared for all the test suite
	static cl_platform_id platform;
	static cl_device_id device;
	static cl_context context;
	static cl_command_queue cmd_queue;
protected:	
	//Create a program from the kernelCode
	void simpleProgramCreation(const char **kernelCode);
	//just build that program with all possibles devices
	void simpleProgramBuild();
	//Create a kernel from the program, assumes there is only one __kernel
	void simpleKernelCreation();
	cl_kernel kernel;
	cl_program program;

};

//================ Google test helpers ===================================
//googleTest specialized assertions 
#define EXPECT_OCL_EQ(expected, actual, function_name) EXPECT_EQ(oclErr(expected),oclErr(actual)) << ERR_FUNCTION(#function_name)
#define ASSERT_OCL_EQ(expected, actual, function_name) ASSERT_EQ(oclErr(expected),oclErr(actual)) << ERR_FUNCTION(#function_name)
#define EXPECT_OCL_SUCCESS(actual, function_name) EXPECT_OCL_EQ(CL_SUCCESS, actual, function_name)
#define ASSERT_OCL_SUCCESS(actual, function_name) ASSERT_OCL_EQ(CL_SUCCESS, actual, function_name)
/*#define ASSERT_EQ(rowPitch , getRowPitch()) << ERR_FUNCTION("clEnqueueMapImage") << ERR_IN_LOOP(index);*/

#define ERROR_RESET 1
// for better error messages
#define ERR_FUNCTION(message) "while executing: " << (message) 
#define ERR_IN_LOOP(i) " in loop with index: "<< (i);
/*
* this class help produce better error messages, output the acutal names of the error instead of thier serial number
*/
class oclErr 
{
public:
	oclErr(cl_err_code errCode);
	
	string gerErrString() const;
	bool operator==(const oclErr& other) const;
    bool operator!=(const oclErr& other) const { return !operator==(other); }

private:
	cl_err_code errCode;
	string err;

};

::std::ostream& operator<<(::std::ostream& os, const oclErr& OclErr);

class Ocl2DImage 
{
public:
	Ocl2DImage(size_t size_, bool Random = false);
	Ocl2DImage(const Ocl2DImage &copy);
	//just wrap the data 
	Ocl2DImage(cl_uchar* pImage_, size_t Size_) : pImage(pImage_) , Size(Size_) {}

	Ocl2DImage(int numOfImages, const Ocl2DImage &copy);

	void init( size_t size_, bool Random);
	~Ocl2DImage(void);
	Ocl2DImage& operator=(const Ocl2DImage &other);
	bool operator==(const Ocl2DImage &other) const;
	bool operator!=(const Ocl2DImage &other) const { return !(*this == other); }
	inline size_t size() const { return Size; }

	operator cl_uchar*() const { return pImage; }
	operator void*() const { return pImage; }

private:
	cl_uchar* pImage;
	size_t Size;
};

//================ C++ wrappers for all clObjects ===================================
// this are wrappers taken from typeWrappers.h in conformance ,
// they are copied because I don't won't a dependency to this files.
// so this wrappers might defer slightly from the original ones

/* cl_mem (stream) wrapper */
class clMemWrapper
{
public:
	clMemWrapper() { mMem = NULL; }
	clMemWrapper( cl_mem mem ) { mMem = mem; }
	~clMemWrapper() { reset(); }
	
	void reset() {
		if( mMem != NULL ) {
			clReleaseMemObject( mMem );
		}
		mMem = NULL;
	}
	clMemWrapper & operator=( const cl_mem &rhs ) { mMem = rhs; return *this; }
	operator cl_mem() { return mMem; }
	cl_mem* operator&() { return &mMem; }

	bool operator==( const cl_mem &rhs ) const { return mMem == rhs; }

	bool operator==( const int &num ) const {
		if ( num != (int) NULL){
			return false;
		}
		// TODO: why mMem == num doesn't work?
		return mMem == NULL; }
	
	ostream& print(::std::ostream& os) const { return os << mMem;}
	
protected:

	cl_mem mMem;
};

std::ostream& operator<<(std::ostream& os, const clMemWrapper& mem);

bool operator==(const int num, const clMemWrapper& mem);

/* cl_program wrapper */

class clProgramWrapper
{
public:
	clProgramWrapper() { mProgram = NULL; }
	clProgramWrapper( cl_program program ) { mProgram = program; }
	~clProgramWrapper() { if( mProgram != NULL ) clReleaseProgram( mProgram ); }

	clProgramWrapper & operator=( const cl_program &rhs ) { mProgram = rhs; return *this; }
	operator cl_program() { return mProgram; }

	cl_program * operator&() { return &mProgram; }

	bool operator==( const cl_program &rhs ) { return mProgram == rhs; }

protected:

	cl_program mProgram;
};

/* cl_kernel wrapper */

class clKernelWrapper
{
public:
	clKernelWrapper() { mKernel = NULL; }
	clKernelWrapper( cl_kernel kernel ) { mKernel = kernel; }
	~clKernelWrapper() { if( mKernel != NULL ) clReleaseKernel( mKernel ); }

	clKernelWrapper & operator=( const cl_kernel &rhs ) { mKernel = rhs; return *this; }
	operator cl_kernel() { return mKernel; }

	cl_kernel * operator&() { return &mKernel; }

	bool operator==( const cl_kernel &rhs ) { return mKernel == rhs; }

protected:

	cl_kernel mKernel;
};

