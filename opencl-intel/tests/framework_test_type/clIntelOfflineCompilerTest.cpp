/**************************************************************************************************
* clIntelOfflineCompilerTest 
* -------------------
* Checks the cmd version of IOC
* - checks Binary File Output can be used properly
**************************************************************************************************/

#include "CL/cl.h"
#include "FrameworkTest.h"
#include <sys/stat.h>
#include <gtest/gtest.h>
#include "TestsHelpClasses.h"
#include "cl_cpu_detect.h"
/*------------kernels definitions------------------*/
#define XSTR(s) STR(s)
#define STR(s) #s

/* Instructions:
*	all of this kernels are defines so you can see all the syntax highlight of the kernels
*	in order to transform it into a string str (char[]...) 
*	for a kernel defined KER simply type str = XSTR(KER)
*/

#define KERNEL __kernel void clIntelOfflineCompilerTest(__global int* i)	\
{	\
	int tid = get_global_id(0);	\
	if (tid== (*i))	\
	(*i) ++;	\
}

#define KERNEL_PRINTF __kernel void clIntelOfflineCompilerTest(__global int *mRes){printf("%d\n",2011); (*mRes) ++;}
/*---------------------------------------------------*/
#define FILE_TYPE ir //if the file type will be changed in the future....
#define FILE_NAME kernel_clIntelOfflineCompilerTest
#define EXECUTE -input=FILE_NAME.cl -ir=FILE_NAME.FILE_TYPE

#define FOPEN_OPTIONS rbD  //read,binary,Temporary , remove D so binary file will not be deleted


#define MAX_SOURCE_SIZE 100000//(0x100000)
#define WORK_SIZE 400

#define NullCheckAndExecute(check,command) if ((check)!= NULL ) command

#define ERROR_RESET 1
#define ERR_MESSAGE(message) "while executing: " << (message) ;

class iocOcl
{
public:
	iocOcl(){
		context=NULL;
		cmd_queue=NULL;
		device=NULL;
		program=NULL;
		kernel=NULL;
		err=ERROR_RESET;
		platform=NULL;
		count=NULL;
		output = 0;
	}
	~iocOcl(){
		NullCheckAndExecute(count,clReleaseMemObject(count));
		NullCheckAndExecute(kernel,clReleaseKernel(kernel));
		NullCheckAndExecute(program,clReleaseProgram(program));	
		NullCheckAndExecute(cmd_queue,clReleaseCommandQueue(cmd_queue));
		NullCheckAndExecute(context,clReleaseContext(context));
	}
	void initOclObjects(){
		//init platform
		err=clGetPlatformIDs(1,&platform,NULL);
		ASSERT_EQ(CL_SUCCESS,err) << ERR_MESSAGE("clGetPlatformIDs");

		// init Devices (only one CPU...)
		err=clGetDeviceIDs(platform,CL_DEVICE_TYPE_DEFAULT,1,&device,NULL);
		ASSERT_EQ(CL_SUCCESS,err) << ERR_MESSAGE("clGetDeviceIDs");

		//init context
		context=clCreateContext(NULL,1,&device,NULL,NULL,&err);
		ASSERT_EQ(CL_SUCCESS,err) << ERR_MESSAGE("clCreateContext");

		//init Command Queue
		cmd_queue=clCreateCommandQueue(context,device,0,&err);
		ASSERT_EQ(CL_SUCCESS,err) << ERR_MESSAGE("clCreateCommandQueue");
	}
	void CreateBinaryWithIoc(char * sKernel){
		//Create Program
		FILE* pIRfile = NULL;
#ifdef _WIN32	
		err=FOPEN(pIRfile,XSTR(FILE_NAME.FILE_TYPE), "r"); //just to check that it doesn't exists
#else
		FOPEN(pIRfile,XSTR(FILE_NAME.FILE_TYPE), "r"); //just to check that it doesn't exists
		err = errno;
		if (pIRfile != NULL)
		{
			err = 0;
		}
#endif
		if (err==0){ //file already exists
			printf("ATTENTION: binary file already exists,if \"ioc\" failed old binary will be used \n");
			fclose(pIRfile);
		}
		//USE WITH CAUTION: override a file if already exists 
		//creating the .cl file for ioc use
		FILE* pCLfile = NULL;

		int kernelLength = (int) strlen(sKernel);
#ifdef _WIN32	
		err = FOPEN(pCLfile,XSTR(FILE_NAME.cl),"w"); //writing a new file (override if needed)
#else
		FOPEN(pCLfile,XSTR(FILE_NAME.cl),"w"); //writing a new file (override if needed)
		err = errno;
		if (pCLfile != NULL)
		{
			err = 0;
		}
#endif
		ASSERT_EQ(0,err) << ERR_MESSAGE("creation of file pCLfile");
		err = FPRINTF(pCLfile,sKernel);
		fclose(pCLfile);
		//TODO: decide if we should remove, making problems because of \r\n kind of spaces
		//ASSERT_EQ(kernelLength,err) << ERR_MESSAGE("writing of file pCLfile");

		char exec[]= XSTR(EXECUTE);
		std::string execString;
		std::string archType = (sizeof(void*) == 8) ? "64" : "32";

		execString = "ioc";
		execString += archType + " ";
		execString += exec;


		system(execString.c_str());
		remove(XSTR(FILE_NAME.cl)); //.cl file isn't needed any more

		struct stat IRstatus;
		stat(XSTR(FILE_NAME.FILE_TYPE),&IRstatus);
		size_t binary_length=IRstatus.st_size;

#ifdef _WIN32
		err=FOPEN(pIRfile,XSTR(FILE_NAME.FILE_TYPE), XSTR(FOPEN_OPTIONS));
#else
		FOPEN(pIRfile,XSTR(FILE_NAME.FILE_TYPE), XSTR(FOPEN_OPTIONS));
		err = errno;
		if (pIRfile != NULL)
		{
			err = 0;
		}
#endif
		ASSERT_EQ(0,err) << ERR_MESSAGE("open file pIRfile");

		char* BinaryFile=new char[binary_length]; //dynamic allocation
		fread(BinaryFile,sizeof(char),binary_length,pIRfile);
		fclose(pIRfile);

		cl_int binary_status=ERROR_RESET;	
		program=clCreateProgramWithBinary(context,1,&device,&(binary_length),(const unsigned char**)(&BinaryFile),&binary_status,&err);
		delete[] BinaryFile;
		ASSERT_EQ(CL_SUCCESS,err) << ERR_MESSAGE("clCreateCommandQueue");
	}

	void BuildProgramAndCreateKernel(){
		//Build Program
		err=clBuildProgram(program,0,NULL,NULL,NULL,NULL);
		cl_build_status build_status=ERROR_RESET;
		err|=clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_STATUS,MAX_SOURCE_SIZE,&build_status,NULL);	
		if (err!=CL_SUCCESS || build_status==CL_BUILD_ERROR){
			printf("\n build status is: %d \n",build_status);
			char err_str[MAX_SOURCE_SIZE];	// instead of dynamic allocation
			char* err_str_ptr=err_str;
			err=clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,MAX_SOURCE_SIZE,err_str_ptr,NULL);
			if (err!=CL_SUCCESS)
				printf("Build Info error: %d \n",err);
			printf("%s \n",err_str_ptr);
			FAIL();
		}
		kernel=clCreateKernel(program,"clIntelOfflineCompilerTest",&err);
		ASSERT_EQ(CL_SUCCESS,err) << ERR_MESSAGE("clCreateKernel");

		count=clCreateBuffer(context,CL_MEM_READ_WRITE,sizeof(int),NULL,&err);
		ASSERT_EQ(CL_SUCCESS,err) << ERR_MESSAGE("clCreateBuffer");

		err=clEnqueueWriteBuffer(cmd_queue,count,CL_TRUE,0,sizeof(int),&output,0,NULL,NULL);
		ASSERT_EQ(CL_SUCCESS,err) << ERR_MESSAGE("clEnqueueWriteBuffer");

		err=clSetKernelArg(kernel,0,sizeof(cl_mem),&count);
		ASSERT_EQ(CL_SUCCESS,err) << ERR_MESSAGE("clSetKernelArg");
	}
	void RunKernelAndPrintResult(){
		size_t global_work_size[1];
		global_work_size[0]=WORK_SIZE;
		err=clEnqueueNDRangeKernel(cmd_queue,kernel,1,NULL,global_work_size,NULL,0,NULL,NULL);
		ASSERT_EQ(CL_SUCCESS,err) << ERR_MESSAGE("clEnqueueNDRangeKernel");

		err=clEnqueueReadBuffer(cmd_queue,count,CL_TRUE,0,sizeof(int),&output,0,NULL,NULL);
		ASSERT_EQ(CL_SUCCESS,err) << ERR_MESSAGE("clEnqueueReadBuffer");

		SUCCEED();
		cout << "SUCCESS: the kernel was executed successfully," << endl;
		cout << "it returned " << output << endl;
	}
private:
	cl_context context;
	cl_command_queue cmd_queue;
	cl_device_id device;
	cl_program program;
	cl_kernel kernel;
	cl_int err;
	cl_platform_id platform;
	cl_mem count;
	int output;
};

void clIntelOfflineCompilerTestAux(char sKernel[]) {

	printf("---------------------------------------\n");
	printf("clIntelOfflineCompilerTest with kernel: \n");
	printf("%s \n",sKernel);
	printf("---------------------------------------\n");

	iocOcl IocTest;

	ASSERT_NO_FATAL_FAILURE(IocTest.initOclObjects());

	ASSERT_NO_FATAL_FAILURE(IocTest.CreateBinaryWithIoc(sKernel));

	ASSERT_NO_FATAL_FAILURE(IocTest.BuildProgramAndCreateKernel());

	ASSERT_NO_FATAL_FAILURE(IocTest.RunKernelAndPrintResult());
}

/*-------------------actual tests--------------*/
TEST(IocTests, Threads)
{
	EXPECT_TRUE(clIntelOfflineCompilerThreadsTest());
}

TEST(IocTests, DISABLED_BuildOptions)
{
	EXPECT_TRUE(clIntelOfflineCompilerBuildOptionsTest());
}

TEST(IocTests, SimpleKernel)
{
	clIntelOfflineCompilerTestAux((char*)XSTR(KERNEL));
}
//fails because of a bug
TEST(IocTests, PrintfKernel)
{
	clIntelOfflineCompilerTestAux((char*)XSTR(KERNEL_PRINTF));
}

TEST(IocTests, llvmAndAsmCreation){
	string const kernelName = "kernelPi";
	string const inAsm = "kernelPi:"; //a label that should be there
	string const inLlvm = "target triple"; //a variable that should always exists
	
	runIoc("-input=" + kernelName + ".cl -asm -llvm");
	
	string files[] = {kernelName + ".asm", kernelName + ".ll"};
	ASSERT_NO_FATAL_FAILURE(validateSubstringInFile(files[0], inAsm, true));
	ASSERT_NO_FATAL_FAILURE(validateSubstringInFile(files[1], inLlvm, true));
	
	removeFiles(files, 2);
}


TEST(IocTests, OptionSimd){
	//const argument that you may change in the future
	string const kernelName = "kernelSimd";
	int const simdOptionLength = 3;
	string const simdOption[simdOptionLength] = {"avx", "sse41", "sse42"};
	
	
	//creating all the asm and llvm files
	string fileName[simdOptionLength];
	for (int i = 0 ; i < simdOptionLength; i++ ){
		fileName[i] = kernelName + "_" + simdOption[i] + ".asm";
		runIoc("-input=" + kernelName + ".cl -simd=" + simdOption[i] + " -asm=" + fileName[i]);
	}
	//checking that those file are different
	for (int i = 0; i < simdOptionLength - 1; i++){
		for (int j = i+1 ; j < simdOptionLength; j++){
			ASSERT_NO_FATAL_FAILURE(validateEqualityOfFiles(fileName[i],fileName[j], false,0));
		}
	}
	//remove those file (but only if the test succeeds) 
 	removeFiles(fileName, simdOptionLength);
}

TEST(IocTests, DISABLED_OptionSimdDefault){
	string simdOption = " -simd=";
	if (Intel::OpenCL::Utils::CPUDetect::GetInstance()->IsFeatureSupported(Intel::OpenCL::Utils::CFS_AVX10)){
		simdOption+="avx";
	} else if (Intel::OpenCL::Utils::CPUDetect::GetInstance()->IsFeatureSupported(Intel::OpenCL::Utils::CFS_SSE42)) {
		simdOption+="sse42";
	} else if (Intel::OpenCL::Utils::CPUDetect::GetInstance()->IsFeatureSupported(Intel::OpenCL::Utils::CFS_SSE41)) {
		simdOption+="sse41";
	} else {
		FAIL() << "simd option is not supported for this instructions set";
	}

	string const kernelName = "kernelSimd.cl";
	string files[] = { "OptionSimdDynamic_defualt.bc", "OptionSimdDynamic_defualt.asm", "OptionSimdDynamic_simd.bc", "OptionSimdDynamic_simd.asm" };
	removeFiles(files, 4);
	runIoc("-input=" + kernelName + " -llvm=" + files[0] + " -asm=" + files[1]);
	runIoc("-input=" + kernelName + simdOption + " -llvm=" + files[2] + " -asm=" + files[3]);
	int compareFromLine = 0;
#ifndef _WIN32
	compareFromLine = 1;
#endif
	ASSERT_NO_FATAL_FAILURE(validateEqualityOfFiles(files[0], files[2], true,compareFromLine));
	ASSERT_NO_FATAL_FAILURE(validateEqualityOfFiles(files[1], files[3], true,compareFromLine));
	removeFiles(files, 4);
}

/*----------------------------------------------*/
