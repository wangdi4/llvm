#include <cassert>
#include <string>
#include <sstream>
#include "OptionParser.h"
#include "ResultDatabase.h"
#include "S3D.h"
#include "Timer.h"
#include "gr_base.h"
#include "ratt.h"
#include "ratt2.h"
#include "ratx.h"
#include "qssa.h"
#include "qssa2.h"
#include "rdwdot.h"

using namespace std;

// Forward declaration
template <class real>
void RunTest(string testName, ResultDatabase &resultDB, OptionParser &op);

// ********************************************************
// Function: toString
//
// Purpose:
//   Simple templated function to convert objects into
//   strings using stringstream
//
// Arguments:
//   t: the object to convert to a string
//
// Returns:  a string representation
//
// Modifications:
//
// ********************************************************
template<class T> inline string toString(const T& t)
{
    stringstream ss;
    ss << t;
    return ss.str();
}

// ****************************************************************************
// Function: addBenchmarkSpecOptions
//
// Purpose:
//   Add benchmark specific options parsing
//
// Arguments:
//   op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Kyle Spafford
// Creation: March 13, 2010
//
// Modifications:
//
// ****************************************************************************
void
addBenchmarkSpecOptions(OptionParser &op)
{
    ; // No S3D specific options
}

// ****************************************************************************
// Function: RunBenchmark
//
// Purpose:
//   Executes the S3D benchmark
//
// Arguments:
//   resultDB: results from the benchmark are stored in this db
//   op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Kyle Spafford
// Creation: March 13, 2010
//
// Modifications:
//
// ****************************************************************************
void RunBenchmark(OptionParser &op, ResultDatabase &resultDB)
{
    // Always run the single precision test
    RunTest<float>("S3D-SP", resultDB, op);
    RunTest<double>("S3D-DP", resultDB, op);
}

template <class real>
void RunTest(string testName, ResultDatabase &resultDB, OptionParser &op)
{
    // Number of grid points (specified in header file)
    const int probSizes[4] = { 16, 32, 48, 64 };
    int sizeClass = op.getOptionInt("size") - 1;
    assert(sizeClass >= 0 && sizeClass < 4);
    sizeClass = probSizes[sizeClass];
    int n = sizeClass * sizeClass * sizeClass;

    // Host variables
    static real* host_t;
    static real* host_p;
    static real* host_y;
    static real* host_wdot;
    static real* host_molwt;

    static real *host_rf;
    static real *host_rb;
    static real *host_rklow;
    static real *host_c;
    static real *host_a;
    static real *host_eg;

    // Malloc host memory

    host_t=(real*)_mm_malloc(n*sizeof(real),ALIGN);
    host_p=(real*)_mm_malloc(n*sizeof(real),ALIGN);;
    host_y=(real*)_mm_malloc(Y_SIZE*n*sizeof(real),ALIGN);;
    host_wdot=(real*)_mm_malloc(WDOT_SIZE*n*sizeof(real),ALIGN);
    host_molwt=(real*)_mm_malloc(WDOT_SIZE*n*sizeof(real),ALIGN);

    host_rf=(real*)_mm_malloc(n*RF_SIZE*sizeof(real), ALIGN);
    host_rb=(real*)_mm_malloc(n*RB_SIZE*sizeof(real), ALIGN);
    host_rklow=(real*)_mm_malloc(n*RKLOW_SIZE*sizeof(real), ALIGN);
    host_c=(real*)_mm_malloc(n*C_SIZE*sizeof(real), ALIGN);
    host_a=(real*)_mm_malloc(n*A_SIZE*sizeof(real), ALIGN);
    host_eg=(real*)_mm_malloc(n*EG_SIZE*sizeof(real), ALIGN);

    // Initialize Test Problem

    // For now these are just 1, to compare results between cpu & host
    real rateconv = 1.0;
    real tconv = 1.0;
    real pconv = 1.0;

    // Initialize temp and pressure
    for (int i=0; i<n; i++)
    {
        host_p[i] = 1.0132e6;
        host_t[i] = 1000.0;
    }

    // Init molwt: for now these are just 1, to compare results betw. cpu & host
    for (int i=0; i<WDOT_SIZE; i++)
    {
        host_molwt[i] = 1;
    }

    // Initialize mass fractions
    for (int j=0; j<Y_SIZE; j++)
    {
        for (int i=0; i<n; i++)
        {
            host_y[(j*n)+i]= 0.0;
            if (j==14)
                host_y[(j*n)+i] = 0.064;
            if (j==3)
                host_y[(j*n)+i] = 0.218;
            if (j==21)
                host_y[(j*n)+i] = 0.718;
        }
    }

    unsigned int passes = op.getOptionInt("passes");
    double start;

    #pragma offload target(mic)\
	in(host_t:length(n) free_if(0))\
	in(host_p:length(n) free_if(0))\
	in(host_y:length(n*Y_SIZE) free_if(0))\
	in(host_molwt:length(n*WDOT_SIZE) free_if(0))\
        out(host_wdot:length(n*WDOT_SIZE) free_if(0))\
	in(host_rf:length(n*RF_SIZE) free_if(0))\
	in(host_rb:length(n*RB_SIZE) free_if(0))\
	in(host_rklow:length(n*RKLOW_SIZE) free_if(0))\
	in(host_c:length(n*C_SIZE) free_if(0))\
	in(host_a:length(n*A_SIZE) free_if(0))\
	in(host_eg:length(n*EG_SIZE) free_if(0))
	{
	}

    start=curr_second();
    #pragma offload target(mic)\
        in(host_t:length(n) alloc_if(0) free_if(0))\
        in(host_p:length(n) alloc_if(0) free_if(0))\
	in(host_y:length(n*Y_SIZE) alloc_if(0) free_if(0))\
	in(host_molwt:length(n*WDOT_SIZE) alloc_if(0) free_if(0))\
        out(host_wdot:length(n*WDOT_SIZE) alloc_if(0) free_if(0))
        {
        }
     static double transferTime=curr_second()-start;
     static double kernelTime;

    for (unsigned int i = 0; i < passes; i++)
    {

    start=curr_second();
    
    #pragma offload target(mic)\
        nocopy(host_t:length(n) alloc_if(0) free_if(0))\
        nocopy(host_p:length(n) alloc_if(0) free_if(0))\
        nocopy(host_y:length(n*Y_SIZE) alloc_if(0) free_if(0))\
        nocopy(host_molwt:length(n*WDOT_SIZE) alloc_if(0) free_if(0))\
        out(host_wdot:length(n*WDOT_SIZE) alloc_if(0) free_if(0))\
	nocopy(host_rf:length(n*RF_SIZE) alloc_if(0) free_if(0))\
	nocopy(host_rb:length(n*RB_SIZE) alloc_if(0) free_if(0))\
	nocopy(host_rklow:length(n*RKLOW_SIZE) alloc_if(0) free_if(0))\
	nocopy(host_c:length(n*C_SIZE) alloc_if(0) free_if(0))\
	nocopy(host_a:length(n*A_SIZE) alloc_if(0) free_if(0))\
	nocopy(host_eg:length(n*EG_SIZE) alloc_if(0) free_if(0))\

    {
        /*real *host_rf=(real*)malloc(RF_SIZE*n*sizeof(real));
        real *host_rb=(real*)malloc(RB_SIZE*n*sizeof(real));
        real *host_rklow=(real*)malloc(RKLOW_SIZE*n*sizeof(real));
        real *host_c=(real*)malloc(C_SIZE*n*sizeof(real));
        real *host_a=(real*)malloc(A_SIZE*n*sizeof(real));
        real *host_eg=(real*)malloc(EG_SIZE*n*sizeof(real));*/

        ratt_kernel (host_t, host_rf, tconv,n);

        rdsmh_kernel(host_t, host_eg, tconv,n);

        gr_base(host_p, host_t, host_y,host_c, tconv, pconv,n);

        ratt2_kernel(host_t, host_rf, host_rb,host_eg, tconv,n);
        ratt3_kernel(host_t, host_rf, host_rb,host_eg, tconv,n);
        ratt4_kernel(host_t, host_rf, host_rb,host_eg, tconv,n);
        ratt5_kernel(host_t, host_rf, host_rb,host_eg, tconv,n);
        ratt6_kernel(host_t, host_rf, host_rb,host_eg, tconv,n);
        ratt7_kernel(host_t, host_rf, host_rb,host_eg, tconv,n);
        ratt8_kernel(host_t, host_rf, host_rb,host_eg, tconv,n);
        ratt9_kernel(host_t, host_rf, host_rb,host_eg, tconv,n);
        ratt10_kernel(host_t, host_rklow, tconv,n);
	ratx_kernel(host_t, host_c, host_rf, host_rb,host_rklow, tconv,n);
        ratxb_kernel   (host_t, host_c, host_rf, host_rb,host_rklow, tconv,n);
        ratx2_kernel(host_c, host_rf, host_rb,n);
        ratx4_kernel(host_c, host_rf, host_rb,n);
        qssa_kernel (host_rf, host_rb, host_a,n);
        qssab_kernel   (host_rf, host_rb, host_a,n);
        qssa2_kernel(host_rf, host_rb, host_a,n);
        rdwdot_kernel(host_rf, host_rb, host_wdot,rateconv, host_molwt,n);
        rdwdot2_kernel (host_rf, host_rb, host_wdot,rateconv, host_molwt,n);
        rdwdot3_kernel (host_rf, host_rb, host_wdot,rateconv, host_molwt,n);
        rdwdot6_kernel (host_rf, host_rb, host_wdot,rateconv, host_molwt, n);
        rdwdot7_kernel (host_rf, host_rb, host_wdot,rateconv, host_molwt, n);
        rdwdot8_kernel (host_rf, host_rb, host_wdot,rateconv, host_molwt, n);
        rdwdot9_kernel (host_rf, host_rb, host_wdot,rateconv, host_molwt, n);
        rdwdot10_kernel(host_rf, host_rb, host_wdot,rateconv, host_molwt, n);	
    }

	kernelTime=curr_second()-start;
	
	double gflops = ((n*10000.) / 1.e9);

        resultDB.AddResult(testName, toString(n) + "_gridPoints", "GFLOPS",
                gflops / kernelTime);
        resultDB.AddResult(testName + "_PCIe", toString(n) + "_gridPoints", "GFLOPS",
                        gflops / (kernelTime + transferTime));
        resultDB.AddResult(testName + "_Parity", toString(n) + "_gridPoints", "N",
                (transferTime) / kernelTime);
    }

	_mm_free(host_rf);
	_mm_free(host_rb);
	_mm_free(host_rklow);
	_mm_free(host_c);
	_mm_free(host_a);
	_mm_free(host_eg);

    #pragma offload target(mic)\
        in(host_t:length(n) alloc_if(0))\
        in(host_p:length(n) alloc_if(0))\
	in(host_y:length(n*Y_SIZE) alloc_if(0))\
	in(host_molwt:length(n*WDOT_SIZE) alloc_if(0))\
        out(host_wdot:length(n*WDOT_SIZE) alloc_if(0))\
	in(host_rf:length(n*RF_SIZE) alloc_if(0))\
        in(host_rb:length(n*RB_SIZE) alloc_if(0))\
        in(host_rklow:length(n*RKLOW_SIZE) alloc_if(0))\
        in(host_c:length(n*C_SIZE) alloc_if(0))\
        in(host_a:length(n*A_SIZE) alloc_if(0))\
        in(host_eg:length(n*EG_SIZE) alloc_if(0))
        {
        }


//    // Print out answers to compare with CPU
    for (int i=0; i<WDOT_SIZE; i++) {
        printf("% 23.16E ", host_wdot[i*n]);
        if (i % 3 == 2)
            printf("\n");
    }
    printf("\n");


	//Free memory;
    _mm_free(host_t);
    _mm_free(host_p);
    _mm_free(host_y);
    _mm_free(host_wdot);
    _mm_free(host_molwt);

}
