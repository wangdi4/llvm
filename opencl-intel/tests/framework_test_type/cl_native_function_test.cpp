/**************************************************************************************************
 **************************************************************************************************/
// Check only for scalar float type.
// TODO: uncomment some tests once bug are fixed/ functions are implemented
//       


#include "CL/cl.h"
#include "cl_types.h"
#include "Logger.h"
#include "cl_objects_map.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "FrameworkTest.h"
#include <emmintrin.h>


using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;


#define BUFFER_SIZE 128	// number of iterations the test will do 
#define MAX_BUFFS 3			//max number of arguments to a kernel

//define the max ULP of each native function (sorted by definition on spec)
//ULP=0 probably mean that the implementation 
//of the native functions is the same as the regular one
//other ULPs based on try and error (for now...)
#define COS_ULP 4096
#define DIVIDE_ULP 4700
#define EXP_ULP 2500
#define EXP2_ULP 2500
#define EXP10_ULP 2500
#define LOG_ULP 3000
#define LOG2_ULP 1000
#define LOG10_ULP 1000
#define POWR_ULP 2500
#define RECIP_ULP 4500
#define RSQRT_ULP 4500
#define SIN_ULP 2300
#define SQRT_ULP 1 
#define TAN_ULP 4000

#define D_COS_ULP 60000000
#define D_DIVIDE_ULP 10
#define D_EXP_ULP 60000000
#define D_EXP2_ULP 20000000
#define D_EXP10_ULP 20000000
#define D_LOG_ULP 3000
#define D_LOG2_ULP 2000
#define D_LOG10_ULP 2000
#define D_POWR_ULP 65000000
#define D_RECIP_ULP 10
#define D_RSQRT_ULP 10
#define D_SIN_ULP 60000000
#define D_SQRT_ULP 10 
#define D_TAN_ULP 5000000

#define PXSTR(p, s) p STR(s)
#define XSTR(s) STR(s)
#define STR(s) #s
#define NAME_STR(name) #name
#define NAME_VEC(name,typ,vec) NAME_STR(name typ##vec)
//#define MAX(one,two) ((one) > (two)) ? (one) : (two) ; 
 
#define RELEASE_KERNEL 1
#define RELEASE_PROGRAM 2
#define RELEASE_QUEUE 3
#define RELEASE_CONTEXT 4
#define RELEASE_END 5
#define FINAL_ROUTINE 6
#define RELEASE_MEM 7
#define RELEASE_IMAGES 8



//kernel test for native functions with one argument

#define NATIVE_TEST_FLOAT(func,vec) __kernel void native_test(__global float##vec* pBuff,__global int##vec* pULP )  \
{\
	size_t index = get_global_id(0); \
	float##vec fres=(func(pBuff[index])); \
	float##vec native_fres=(native_##func(pBuff[index]));	\
	float##vec two=2; \
	pBuff[index] = fabs(fres - native_fres); \
	int##vec res=as_int##vec(fres);  \
	int##vec native_res=as_int##vec(native_fres); \
	pULP[index] = (-as_int##vec(abs(isgreater(pBuff[index],fabs(fres/two)))))  | as_int##vec(abs(res - native_res)); \
}

#define NATIVE_TEST_DOUBLE(func,vec) \
__kernel void native_test(__global double##vec* pBuff,__global int##vec* pULP)  \
{\
	size_t index = get_global_id(0); \
	double##vec dres=(func(pBuff[index])); \
	double##vec native_dres=(native_##func(pBuff[index]));	\
	pBuff[index] = fabs(dres - native_dres); \
	long##vec res=as_long##vec(dres);  \
	long##vec native_res=as_long##vec(native_dres); \
	double##vec two=2; \
	pULP[index] = (-convert_int##vec(abs(isgreater(pBuff[index],fabs(dres/two))))) | convert_int##vec(abs(res - native_res)); \
}	

#define NATIVE_RECIP(vec) __kernel void native_recip_test(__global float##vec* pBuff,__global int##vec* pULP)  \
{\
	size_t index = get_global_id(0); \
	float##vec one=1.0;	\
	float##vec fres=(one/(pBuff[index])); \
	float##vec native_fres=(native_recip(pBuff[index]));	\
	pBuff[index] = fabs(fres-native_fres); \
	int##vec res=as_int##vec(fres);  \
	int##vec native_res=as_int##vec(native_fres); \
	float##vec two=2; \
	pULP[index] = (-as_int##vec(abs(isgreater(pBuff[index],fabs(fres/two)))))  | as_int##vec(abs(res-native_res)); \
}

#define NATIVE_RECIP_DOUBLE(vec) __kernel void native_recip_test(__global double##vec* pBuff,__global int##vec* pULP)  \
{\
	size_t index = get_global_id(0); \
	double##vec one=1.0;	\
	double##vec fres=(one/(pBuff[index])); \
	double##vec native_fres=(native_recip(pBuff[index]));	\
	pBuff[index] = fabs(fres-native_fres); \
	long##vec res=as_long##vec(fres);  \
	long##vec native_res=as_long##vec(native_fres); \
	double##vec two=2; \
	pULP[index] = (-convert_int##vec(abs(isgreater(pBuff[index],fabs(fres/two)))))  | convert_int##vec(abs(res-native_res)); \
}

#define NATIVE_DIVIDE(vec) __kernel void native_divide_test(__global float##vec* pBuffX,__global float##vec* pBuffY,__global int##vec* pULP)  \
{\
	size_t index = get_global_id(0); \
	float##vec fres=(pBuffX[index]/(pBuffY[index])); \
	float##vec native_fres=(native_divide(pBuffX[index],pBuffY[index]));	\
	pBuffX[index] = fabs(fres-native_fres); \
	int##vec res=as_int##vec(fres);  \
	int##vec native_res=as_int##vec(native_fres); \
	float##vec two=2; \
	pULP[index] =(-as_int##vec(abs(isgreater(pBuffX[index],fabs(fres/two)))))  | as_int##vec(abs(res-native_res)); \
}

#define NATIVE_DIVIDE_DOUBLE(vec) __kernel void native_divide_test(__global double##vec* pBuffX,__global double##vec* pBuffY,__global int##vec* pULP)  \
{\
	size_t index = get_global_id(0); \
	double##vec fres=(pBuffX[index]/(pBuffY[index])); \
	double##vec native_fres=(native_divide(pBuffX[index],pBuffY[index]));	\
	pBuffX[index] = fabs(fres-native_fres); \
	long##vec res=as_long##vec(fres);  \
	long##vec native_res=as_long##vec(native_fres); \
	double##vec two=2; \
	pULP[index] =(-convert_int##vec(abs(isgreater(pBuffX[index],fabs(fres/two)))))  | convert_int##vec(abs(res-native_res)); \
}

#define NATIVE_POWR(vec) __kernel void native_powr_test(__global float##vec* pBuffX,__global float##vec* pBuffY,__global int##vec* pULP)  \
{\
	size_t index = get_global_id(0); \
	float##vec fres=powr(pBuffX[index],pBuffY[index]); \
	float##vec native_fres=(native_powr(pBuffX[index],pBuffY[index]));	\
	pBuffX[index] = fabs(fres-native_fres); \
	int##vec res=as_int##vec(fres);  \
	int##vec native_res=as_int##vec(native_fres); \
	float##vec two=2; \
	pULP[index] =(-as_int##vec(abs(isgreater(pBuffX[index],fabs(fres/two)))))  | as_int##vec(abs(res-native_res)); \
}

#define NATIVE_POWR_DOUBLE(vec) __kernel void native_powr_test(__global double##vec* pBuffX,__global double##vec* pBuffY,__global int##vec* pULP)  \
{\
	size_t index = get_global_id(0); \
	double##vec fres=powr(pBuffX[index],pBuffY[index]); \
	double##vec native_fres=(native_powr(pBuffX[index],pBuffY[index]));	\
	pBuffX[index] = fabs(fres-native_fres); \
	long##vec res=as_long##vec(fres);  \
	long##vec native_res=as_long##vec(native_fres); \
	double##vec two=2; \
	pULP[index] =(-convert_int##vec(abs(isgreater(pBuffX[index],fabs(fres/two)))))  |  convert_int##vec(abs(res-native_res)); \
}

#ifndef _WIN32
// I don't know where this is defined, but apparently it is defined somewhere as "__builtin_isgreater".
#undef isgreater
#endif

#define RUNTESTS_FLOATS(name,func,buff,buffnum)	\
	RunFunctionTest(NAME_VEC(name,float,),XSTR(NATIVE_TEST_FLOAT(func,)),1,context,queue,buff,buffnum,stBuffSize,bResult,name##_ULP); \
	RunFunctionTest(NAME_VEC(name,float,2),XSTR(NATIVE_TEST_FLOAT(func,2)),2,context,queue,buff,buffnum,stBuffSize,bResult,name##_ULP); \
	RunFunctionTest(NAME_VEC(name,float,3),XSTR(NATIVE_TEST_FLOAT(func,3)),3,context,queue,buff,buffnum,stBuffSize,bResult,name##_ULP); \
	RunFunctionTest(NAME_VEC(name,float,4),XSTR(NATIVE_TEST_FLOAT(func,4)),4,context,queue,buff,buffnum,stBuffSize,bResult,name##_ULP); \
	RunFunctionTest(NAME_VEC(name,float,8),XSTR(NATIVE_TEST_FLOAT(func,8)),8,context,queue,buff,buffnum,stBuffSize,bResult,name##_ULP); \
	RunFunctionTest(NAME_VEC(name,float,16),XSTR(NATIVE_TEST_FLOAT(func,16)),16,context,queue,buff,buffnum,stBuffSize,bResult,name##_ULP); \

#define RUNTESTS_DOUBLES(name,func,buff,buffnum)	\
	RunFunctionTest(NAME_VEC(name,double,), XSTR(NATIVE_TEST_DOUBLE(func,)),1,context,queue,buff,buffnum,stBuffSize,bResult,D_##name##_ULP); \
	RunFunctionTest(NAME_VEC(name,double,2),XSTR(NATIVE_TEST_DOUBLE(func,2)),2,context,queue,buff,buffnum,stBuffSize,bResult,D_##name##_ULP); \
	RunFunctionTest(NAME_VEC(name,double,3),XSTR(NATIVE_TEST_DOUBLE(func,3)),3,context,queue,buff,buffnum,stBuffSize,bResult,D_##name##_ULP); \
	RunFunctionTest(NAME_VEC(name,double,4),XSTR(NATIVE_TEST_DOUBLE(func,4)),4,context,queue,buff,buffnum,stBuffSize,bResult,D_##name##_ULP); \
	RunFunctionTest(NAME_VEC(name,double,8),XSTR(NATIVE_TEST_DOUBLE(func,8)),8,context,queue,buff,buffnum,stBuffSize,bResult,D_##name##_ULP); \
	RunFunctionTest(NAME_VEC(name,double,16),XSTR(NATIVE_TEST_DOUBLE(func,16)),16,context,queue,buff,buffnum,stBuffSize,bResult,D_##name##_ULP); \


#define RUNTESTS_SPECIAL_FLOATS(name,func,buff,buffnum)	\
	RunFunctionTest(NAME_VEC(name,float,),XSTR(NATIVE_##name()),1,context,queue,buff,buffnum,stBuffSize,bResult,name##_ULP); \
	RunFunctionTest(NAME_VEC(name,float,2),XSTR(NATIVE_##name(2)),2,context,queue,buff,buffnum,stBuffSize,bResult,name##_ULP); \
	RunFunctionTest(NAME_VEC(name,float,3),XSTR(NATIVE_##name(3)),3,context,queue,buff,buffnum,stBuffSize,bResult,name##_ULP); \
	RunFunctionTest(NAME_VEC(name,float,4),XSTR(NATIVE_##name(4)),4,context,queue,buff,buffnum,stBuffSize,bResult,name##_ULP); \
	RunFunctionTest(NAME_VEC(name,float,8),XSTR(NATIVE_##name(8)),8,context,queue,buff,buffnum,stBuffSize,bResult,name##_ULP); \
	RunFunctionTest(NAME_VEC(name,float,16),XSTR(NATIVE_##name(16)),16,context,queue,buff,buffnum,stBuffSize,bResult,name##_ULP); \

#define RUNTESTS_SPECIAL_DOUBLES(name,func,buff,buffnum)	\
	RunFunctionTest(NAME_VEC(name,double,),XSTR(NATIVE_##name##_DOUBLE()),1,context,queue,buff,buffnum,stBuffSize,bResult,D_##name##_ULP); \
	RunFunctionTest(NAME_VEC(name,double,2),XSTR(NATIVE_##name##_DOUBLE(2)),2,context,queue,buff,buffnum,stBuffSize,bResult,D_##name##_ULP); \
	RunFunctionTest(NAME_VEC(name,double,3),XSTR(NATIVE_##name##_DOUBLE(3)),3,context,queue,buff,buffnum,stBuffSize,bResult,D_##name##_ULP); \
	RunFunctionTest(NAME_VEC(name,double,4),XSTR(NATIVE_##name##_DOUBLE(4)),4,context,queue,buff,buffnum,stBuffSize,bResult,D_##name##_ULP); \
	RunFunctionTest(NAME_VEC(name,double,8),XSTR(NATIVE_##name##_DOUBLE(8)),8,context,queue,buff,buffnum,stBuffSize,bResult,D_##name##_ULP); \
  RunFunctionTest(NAME_VEC(name,double,16),XSTR(NATIVE_##name##_DOUBLE(16)),16,context,queue,buff,buffnum,stBuffSize,bResult,D_##name##_ULP); \
	//TODO: once convert_int3(long3) implemented uncomment vec 3 and delete the temporary one

const char Pragma[] = "#pragma OPENCL EXTENSION cl_khr_fp64 : enable";

//runs a kernel and check it's results vs targetULP
template <typename T>
void RunFunctionTest (const char* FuncName,const char* ocl_test_program,int vec,cl_context& context,cl_command_queue& queue,T** pBuff,int numBuffs,size_t& stBuffSize,bool& bResult,int target_ulp){
	if ((numBuffs>MAX_BUFFS) || (numBuffs<1)){
		printf("IILEGAL NUMBER OF BUFFERS(ARGUMENT TO KERNEL) \n");
		throw RELEASE_QUEUE;
	} 
	int vector3=-1; //means false (-1 for aritmetic calculation)
	if (vec==3){
		vec=4; //because vector3 refer an array like vector4
		vector3=0;	//means true (0 for aritmetic calculation)	
	}
	cl_int iRet = CL_SUCCESS;

	cl_program program;
	cl_kernel kernel;
	cl_mem clBuff[MAX_BUFFS+1]; //+1 for clULP which is clBuff[numBuffs]
  char *kernelCode = new char[strlen(ocl_test_program) + strlen(Pragma)+10];
  sprintf(kernelCode, "%s\n%s\n", Pragma,  ocl_test_program);
	try{
	// create program with source
	if ( !BuildProgramSynch(context, 1, (const char**)&kernelCode, NULL, NULL, &program) ){
		bResult=false;
		throw RELEASE_QUEUE;
	}

	iRet=clCreateKernelsInProgram(program,sizeof(kernel),&kernel,NULL);
	bResult &= SilentCheck(L"clCreateKernelsInProgram", CL_SUCCESS, iRet);
	if (!bResult) throw RELEASE_PROGRAM;
	

	cl_int pULP[BUFFER_SIZE];

	for( unsigned ui = 0; ui < BUFFER_SIZE; ui++ )
	{
		pULP[ui] = -1 ;
	}


	for (int i=0 ; i< (numBuffs) ; i++){
	clBuff[i] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(T)*stBuffSize, pBuff[i], &iRet);
	bResult &= SilentCheck(L"clCreateBuffer", CL_SUCCESS, iRet);
	if (!bResult) {
		numBuffs=i-1;
		throw RELEASE_IMAGES;
	}
	}
	clBuff[numBuffs] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(pULP), pULP, &iRet);
	bResult &= SilentCheck(L"clCreateBuffer", CL_SUCCESS, iRet);
	if (!bResult) {
		numBuffs--;
		throw RELEASE_IMAGES;
	}

	// Set Kernel Arguments
	for (int i=0 ; i< (numBuffs+1) ; i++){
	iRet |= clSetKernelArg(kernel, i, sizeof(cl_mem), &clBuff[i]);
	bResult &= SilentCheck(L"clSetKernelArg", CL_SUCCESS, iRet);
	if (!bResult) throw RELEASE_IMAGES;
	}

	size_t global_work_size[1] = { stBuffSize / vec };

	// Execute kernel
	iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
	bResult &= SilentCheck(L"clEnqueueNDRangeKernel", CL_SUCCESS, iRet);    
	if (!bResult) throw RELEASE_IMAGES;
	//
	// Verification phase
	//
	T pDstBuff[BUFFER_SIZE];

	iRet = clEnqueueReadBuffer( queue, clBuff[0], CL_TRUE, 0, sizeof(pDstBuff), pDstBuff, 0, NULL, NULL );
	iRet |= clEnqueueReadBuffer( queue, clBuff[numBuffs], CL_TRUE, 0, sizeof(pULP), pULP, 0, NULL, NULL );
	bResult &= SilentCheck(L"clEnqueueReadBuffer ", CL_SUCCESS, iRet);
	if (!bResult) throw RELEASE_IMAGES;
	
	int maxULP=0;
	T maxEpsilon=0;
	for( unsigned y=0; (y < stBuffSize) && bResult; ++y )
	{
		if ( ((pULP[y] > target_ulp) || (pULP[y]<0) ) && ( (y+1) % 4 != vector3 ) ) 
		{
			printf("KERNEL TEST FAILED: the function %s failed \n \n",FuncName);
			printf("difference between regular function and relaxed function is: %d ULP\n",pULP[y]);
			if (pULP[y]==-1) printf (" -1 means the difference between native function and regular one is over half of the outcome \n ");
			printf("and should have been: %d ULP \n", target_ulp);
			printf("which is an epsilon of: %.38f \n",pDstBuff[y]);
			printf("for the numbers: \n");
			for (int i=0 ; i< (numBuffs) ; i++){
				printf( " %.38f  \n",pBuff[i][y]);
			}
			bResult = false;
			throw RELEASE_IMAGES;
		
		}
		maxULP=MAX(maxULP,pULP[y]);
		maxEpsilon=MAX(maxEpsilon,pDstBuff[y]);
	}
	printf ("***function %s test completed successfully, ",FuncName);
	printf ("with max ULP of %d and max Epsilon of %.38f *** \n",maxULP,maxEpsilon);
	}
	catch (int error)
	{
		bResult=false;
		switch (error){
			case RELEASE_IMAGES:
				for (int i=0 ; i< (numBuffs+1) ;i++){
				clReleaseMemObject(clBuff[i]);		
				}
		   case RELEASE_KERNEL:
			   clReleaseKernel(kernel);
		   case RELEASE_PROGRAM:
			   clReleaseProgram(program);
			   throw RELEASE_QUEUE;
		   default:
			   throw error;
		}
	}
	//finished successfully 
	for (int i=0 ; i< (numBuffs+1) ;i++){
		clReleaseMemObject(clBuff[i]);		
	}
	clReleaseKernel(kernel);
	clReleaseProgram(program);
  delete [] kernelCode;

}

bool clNativeFunctionTest()
{
    bool         bResult = true;
    cl_int       iRet = CL_SUCCESS;
    cl_device_id clDefaultDeviceId;

	printf("=============================================================\n");
	printf("clNativeFunctionTest\n");
	printf("=============================================================\n");


	cl_platform_id platform = 0;

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= SilentCheck(L"clGetPlatformIDs", CL_SUCCESS, iRet);
	if (!bResult)	return bResult;

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

    //
    // Initiate test infrastructure:
    // Create context, Queue
    //

    size_t	stBuffSize = BUFFER_SIZE;
    cl_float		pBuff[BUFFER_SIZE];
    cl_float		pBuff2[BUFFER_SIZE];
    cl_double		pBuffD[BUFFER_SIZE];
    cl_double		pBuff2D[BUFFER_SIZE];
    cl_double		PospBuffD[BUFFER_SIZE];
    cl_float		PospBuff[BUFFER_SIZE];

    cl_float* OneBuffer[]={pBuff};
    cl_double* OneBufferD[]={pBuffD};
    cl_float* PostiveOneBuffer[]={PospBuff};
    cl_double* PostiveOneBufferD[]={PospBuffD};
    cl_float* TwoBuffers[]={PospBuff,pBuff2};
    cl_double* TwoBuffersD[]={PospBuffD,pBuff2D};

    cl_command_queue queue;


    cl_context context = clCreateContextFromType(prop, CL_DEVICE_TYPE_CPU, NULL, NULL, &iRet);
    bResult &= SilentCheck(L"clCreateContextFromType", CL_SUCCESS, iRet);    
    if (!bResult) goto release_end;

	iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &clDefaultDeviceId, NULL);
    bResult &= SilentCheck(L"clGetDeviceIDs", CL_SUCCESS, iRet);    
    if (!bResult) goto release_context;

    queue = clCreateCommandQueue (context, clDefaultDeviceId, 0 /*no properties*/, &iRet);
	bResult &= SilentCheck(L"clCreateCommandQueue - queue", CL_SUCCESS, iRet);
	if (!bResult) goto release_context;

	srand( 0 );

    // fill with random bits no matter what
	for( unsigned ui = 0; ui < BUFFER_SIZE; ui++ )
    {
        pBuff[ui] = (cl_float)(rand() - RAND_MAX/2) / (cl_float)RAND_MAX ;
		pBuffD[ui] = (cl_double)(rand() - RAND_MAX/2) / (cl_double)RAND_MAX ;
		PospBuff[ui]=fabs(pBuff[ui]);
		PospBuffD[ui]=fabs(pBuffD[ui]);
		pBuff2[ui] = (cl_float)(rand() - RAND_MAX/2) / (cl_float)RAND_MAX ;
		pBuff2D[ui] = (cl_double)(rand() - RAND_MAX/2) / (cl_double)RAND_MAX ;
    }

	try
	{
		RUNTESTS_FLOATS(COS,cos,OneBuffer,1);
		RUNTESTS_DOUBLES(COS,cos,OneBufferD,1);
		RUNTESTS_SPECIAL_FLOATS(DIVIDE,divide,TwoBuffers,2);	
		RUNTESTS_SPECIAL_DOUBLES(DIVIDE,divide,TwoBuffersD,2); 
		RUNTESTS_FLOATS(EXP,exp,OneBuffer,1);
		RUNTESTS_DOUBLES(EXP,exp,OneBufferD,1);
		RUNTESTS_FLOATS(EXP2,exp2,OneBuffer,1);
		RUNTESTS_DOUBLES(EXP2,exp2,OneBufferD,1);
		RUNTESTS_FLOATS(EXP10,exp10,OneBuffer,1);
		RUNTESTS_DOUBLES(EXP10,exp10,OneBufferD,1);
		RUNTESTS_FLOATS(LOG,log,PostiveOneBuffer,1);
		RUNTESTS_DOUBLES(LOG,log,PostiveOneBufferD,1);
		RUNTESTS_FLOATS(LOG2,log2,PostiveOneBuffer,1);
		RUNTESTS_DOUBLES(LOG2,log2,PostiveOneBufferD,1);
		RUNTESTS_FLOATS(LOG10,log10,PostiveOneBuffer,1);
		RUNTESTS_DOUBLES(LOG10,log10,PostiveOneBufferD,1);
#ifndef _M_X64
		RUNTESTS_SPECIAL_FLOATS(POWR,powr,TwoBuffers,2);
		RUNTESTS_SPECIAL_DOUBLES(POWR,powr,TwoBuffersD,2);
#endif
		RUNTESTS_SPECIAL_FLOATS(RECIP,recip,OneBuffer,1);
		RUNTESTS_SPECIAL_DOUBLES(RECIP,recip,OneBufferD,1); 
		RUNTESTS_FLOATS(RSQRT,rsqrt,PostiveOneBuffer,1);
		RUNTESTS_DOUBLES(RSQRT,rsqrt,PostiveOneBufferD,1);
		RUNTESTS_FLOATS(SIN,sin,OneBuffer,1);
		RUNTESTS_DOUBLES(SIN,sin,OneBufferD,1);
		RUNTESTS_FLOATS(SQRT,sqrt,PostiveOneBuffer,1);
		RUNTESTS_DOUBLES(SQRT,sqrt,PostiveOneBufferD,1);
		RUNTESTS_FLOATS(TAN,tan,OneBuffer,1);
		RUNTESTS_DOUBLES(TAN,tan,OneBufferD,1);		
		
	}
	catch (int error)
	{
		switch (error){
	case RELEASE_QUEUE:
		goto release_queue;
	case RELEASE_CONTEXT:
		goto release_context;
	case RELEASE_END:
		goto release_end;
	default:
		printf("no such error \n");
		return false;
		}
	}
	
release_queue:
    clFinish(queue);
    clReleaseCommandQueue(queue);
release_context:
    clReleaseContext(context);
release_end:
	if ( bResult )
	{
		printf ("*************************************************** \n");
		printf ("*** clNativeFunctionTest verification succeeded *** \n");
		printf ("*************************************************** \n");
	}
	else
	{
		printf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
		printf ("!!!!!! clNativeFunctionTest verification failed !!!!! \n");
		printf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
	}
    return bResult;
}

