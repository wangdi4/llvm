/**************************************************************************************************
 * clIntelOfflineCompilerBuildOptionsTest 
 * -------------------
 * Checks the cmd version of IOC
 * - checks Binary File Output can be used properly
 **************************************************************************************************/

//|
//| TESTSUITE: IOC
//|
//| test the Intel Offline Compiler tool, check that it is working properly.
//|
#include <string>
#include "CL/cl.h"
#include "FrameworkTest.h"
#include <sys/stat.h>

#define XSTR(s) STR(s)
#define STR(s) #s

#define FILE_TYPE ir //if the file type will be changed in the future....
#define CL_FILE_NAME kernel_clIntelOfflineCompilerTest
#define OUTPUT_FILE_NAME out.txt
#define EXECUTE -input=FILE_NAME.cl -ir=FILE_NAME.FILE_TYPE -output=OUTPUT_FILE_NAME -bo="-D CONST=3"
//#if defined _WIN32
//#define EXECUTE32 ioc32 EXECUTE
//#define EXECUTE64 ioc64 EXECUTE
//#else
//#define EXECUTE64 ../../ioc64 EXECUTE
//#endif

#define KERNEL __kernel void clIntelOfflineCompilerTest(__global int* i)	\
{	\
	int tid = get_global_id(0);	\
	if (tid== (*i))	\
		(*i) ++;	\
}

#define FOPEN_OPTIONS rbD  //read,binary,Temporary , remove D so binary file will not be deleted
#define ERROR_RESET 1
enum Error {	NO_INPUT,
				NO_OUTPUT,
				FINAL_ROUTINE};

//|
//| TEST: IOC.clIntelOfflineCompilerBuildOptionsTest
//|
//| Purpose 
//| -------
//|
//| Runs IOC cmd version, and check the build options feature from the command line.
//|
//| Method
//| ------
//|
//| 1. Create a .cl file from a given kernel.
//| 2. Run IOC cmd version with a line that compiles the .cl file and outputs .ir and log files.
//| 3. Read the log file and make sure the build options given were used.
//|
//| Pass criteria
//| -------------
//|
//| The log file stated the expected build options were used.
//|

bool clIntelOfflineCompilerBuildOptionsTest() {

printf("---------------------------------------\n");
printf("clIntelOfflineCompilerBuildOptionsTest \n");
printf("---------------------------------------\n");
bool bResult = true;
cl_int err=ERROR_RESET;

try
{
	//Create cl file
	FILE* pOutputFile = NULL;
#ifdef _WIN32	
	err=FOPEN(pOutputFile,XSTR(FILE_NAME.FILE_TYPE), "r"); //just to check that it doesn't exists
#else
	FOPEN(pOutputFile,XSTR(FILE_NAME.FILE_TYPE), "r"); //just to check that it doesn't exists
	err = errno;
	if (pOutputFile != NULL)
	{
		err = 0;
	}
#endif
	if (err==0){ //file already exists
		printf("ATTENTION: binary file already exists,if \"ioc\" failed old binary will be used \n");
		fclose(pOutputFile);
	}
	//USE WITH CAUTION: override a file if already exists 
	//creating the .cl file for ioc use
	FILE* pCLfile = NULL;
	char sKernel[] = XSTR(KERNEL);
	size_t kernelLength = strlen(sKernel);
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
	bResult&=SilentCheck(L"creation of file pCLfile",CL_SUCCESS,err);
	if (!bResult){
		throw NO_INPUT;
	}
	err = FPRINTF(pCLfile,sKernel);
	bResult&=SilentCheckSize(L"writing of file pCLfile",kernelLength,err);
	if (!bResult){
		fclose(pCLfile);
		throw NO_INPUT;
	}
	fclose(pCLfile);
	
	// Run ioc
	char exec[]= XSTR(EXECUTE);
	std::string execString;
	std::string archType = (sizeof(void*) == 8) ? "64" : "32";
#if defined _WIN32
	execString = "ioc";
#else
	execString = "../../ioc";
#endif
	execString += archType + " ";
	execString += exec;

	system(execString.c_str());
	remove(XSTR(FILE_NAME.cl)); //.cl file isn't needed any more
	
	struct stat outputStatus;
	stat(XSTR(OUTPUT_FILE_NAME),&outputStatus);
	size_t output_length=outputStatus.st_size;

#ifdef _WIN32
	err=FOPEN(pOutputFile,XSTR(OUTPUT_FILE_NAME), XSTR(FOPEN_OPTIONS));
#else
	FOPEN(pOutputFile,XSTR(OUTPUT_FILE_NAME), XSTR(FOPEN_OPTIONS));
	err = errno;
	if (pOutputFile != NULL)
	{
		err = 0;
	}
#endif
	bResult&=SilentCheck(L"open file pOutputFile",CL_SUCCESS,err);
	if (!bResult){
		throw NO_OUTPUT;
	}
	
	char* OutputFile=new char[output_length]; //dynamic allocation
	fread(OutputFile,sizeof(char),output_length,pOutputFile);
	fclose(pOutputFile);

	if (strstr(OutputFile, "Using build options: -D CONST=3"))
	{
		printf("Found build options in build log file.\n");
	}
	else
	{
		printf("Didn't find build options in build log file.\n");
		bResult = false;
	}		
	delete[] OutputFile;
	
	throw FINAL_ROUTINE;
}
catch (enum Error error){ 
	switch (error){
case NO_INPUT:
	printf("Failed to create the input file.\n");
case NO_OUTPUT:
	printf("Failed to create the output file.\n");
case FINAL_ROUTINE:
	if ( bResult )
	{
		printf ("********************************************************* \n");
		printf ("*** clIntelOfflineCompilerBuildOptionsTest succeeded  *** \n");
		printf ("********************************************************* \n");
	}
	else
	{
		printf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
		printf ("!!!!!! clIntelOfflineCompilerBuildOptionsTest failed !!!!! \n");
		printf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
	}
	return bResult;
default:
	printf("no such error: %d \n",error);
	printf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
	printf ("!!!!!! clIntelOfflineCompilerBuildOptionsTest failed !!!!! \n");
	printf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
	return false;
	}
}
}