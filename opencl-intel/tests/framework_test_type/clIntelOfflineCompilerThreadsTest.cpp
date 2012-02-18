/**************************************************************************************************
 * clIntelOfflineCompilerThreadsTest 
 * -------------------
 * Checks the cmd version of IOC
 * - checks MultiThreading of ioc.exe + outputing a binary file (without using it furtuer)
 **************************************************************************************************/
#include "cl_thread.h"
#include "FrameworkTest.h"
#include <stdio.h>
#include "cl_sys_defines.h"

#ifdef _WIN32
	#include <windows.h> 
#else
	#include <unistd.h>
#endif


#define CBUFF_SIZE 256
#define NUM_THREADS 10   // change it if you want more threads
#define	REMOVE_RETRIES 5
#define SLEEP_DURATION 1

#define SUCCESS 0
#define FAIL 1

#define XSTR(s) STR(s)
#define STR(s) #s

#define FILE_TYPE ir //if the file type will be changed in the future....
#define FILE_NAME kernel_clIntelOfflineCompilerTest


#define KERNEL __kernel void clIntelOfflineCompilerTest(__global int* i)	\
{	\
	int tid = get_global_id(0);	\
	if (tid== (*i))	\
	(*i) ++;	\
}

inline void CrossPlatformSleep (int seconds)
{
	#ifdef _WIN32
		Sleep(seconds);
	#else
		sleep(seconds);
	#endif
}

int makeClFile(char* sKernel,char* fileName)
{
	FILE* pCLfile = NULL;
	int kernelLength = (int)strlen(sKernel);
	int res;
#ifdef _WIN32
	int err = FOPEN(pCLfile,fileName,"w"); //writing a new file (override if needed)
#else
	FOPEN(pCLfile,fileName,"w"); //writing a new file (override if needed)
	int err = errno;
	if (pCLfile != NULL)
	{
		err = 0;
	}
#endif
	res=SilentCheck(L"creation of file pCLfile",0,err);
	if (!res){
		return err;
	}
	err = FPRINTF(pCLfile,sKernel);
	res=SilentCheck(L"writing of file pCLfile",kernelLength,err);
	if (!res){
		fclose(pCLfile);
		return err;
	}
	fclose(pCLfile);
	return 0;
}
using namespace Intel::OpenCL::Utils;
using namespace std;

class OfflineCompilerBuilderThread :
	public OclThread
{

public:
	OfflineCompilerBuilderThread(bool bAutoDelete = false)
	{
		bResult = true;
	};
	virtual ~OfflineCompilerBuilderThread(void)
	{
	};
	bool bResult;
protected:
	virtual int Run()
	{
		int res=SUCCESS;
		printf("@@@ start of running Thread: %d @@@ \n",m_threadId);
		
		char buf[CBUFF_SIZE] = {0};
		SPRINTF_S(buf,CBUFF_SIZE,"%d",m_threadId);
		string ThreadNum(buf);
		string fileName(XSTR(FILE_NAME));
		string OutFile(fileName + "_Thread_" + ThreadNum + ".ir" );
		char OutFileC[CBUFF_SIZE]={0};
		OutFile.copy(OutFileC,OutFile.length());
		string archType = (sizeof(void*) == 8) ? "64" : "32";

#if defined _WIN32
		string exec("ioc" + archType + " -input=" + fileName + ".cl -ir=" + OutFile);
#else
		string exec("../../ioc" + archType + " -input=" + fileName + ".cl -ir=" + OutFile);
#endif
		char execute[CBUFF_SIZE]={0};
		exec.copy(execute,exec.length());
		
		system(execute);
		int ret;
		for (int i = 0 ; i < REMOVE_RETRIES; i++){
			ret=remove(OutFileC);
			if (ret == 0){
				printf("\nSUCCESS: output file: %s deleted successfully  \n",OutFileC);
				break;
			}
			CrossPlatformSleep(SLEEP_DURATION);
		}
		if (ret != 0) {
			printf("WARNING: File %s was not deleted successfully  \n",OutFileC);
			perror("while trying to delete the following error occurred");
			printf("ioc might have failed \n ");
			res=FAIL;
			bResult = false;
		}

		printf("@@@ end of running Thread: %d  @@@ \n",m_threadId);
		return res; 
	};
	
};

bool clIntelOfflineCompilerThreadsTest()
{
	printf("---------------------------------------\n");
	printf("clIntelOfflineCompilerThreadsTest \n");
	printf("---------------------------------------\n");
	bool bResult = true;

	makeClFile(XSTR(KERNEL),XSTR(FILE_NAME.cl));

	OfflineCompilerBuilderThread* threads[NUM_THREADS];
	for (int i=0 ; i<NUM_THREADS ; i++)
	{
		threads[i]=new OfflineCompilerBuilderThread();
	}

	for (int i=0 ; i<NUM_THREADS ; i++)
	{
		threads[i]->Start();
	}
	for (int i=0 ; i<NUM_THREADS ; i++)
	{
		threads[i]->Join();
		bResult&=threads[i]->bResult;
		delete threads[i];

	}

	remove(XSTR(FILE_NAME.cl)); //.cl file isn't needed any more

	if ( bResult )
	{
		printf ("*************************************************** \n");
		printf ("*** clIntelOfflineCompilerThreadsTest succeeded *** \n");
		printf ("*************************************************** \n");
	}
	else
	{
		printf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
		printf ("!!!!!! clIntelOfflineCompilerThreadsTest failed !!!!! \n");
		printf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
	}
	return bResult;
}




