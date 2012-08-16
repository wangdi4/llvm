//RUN: SATest -OCL -VAL -config=%s.cfg -neat=1 --force_ref > %t
// RUN: FileCheck %s <%t
#pragma OPENCL EXTENSION cl_khr_fp64: enable
#define VALTYPE double

__kernel 
void 
testGEP_2D( __global VALTYPE* data, 
            __global VALTYPE* newData)
{
    __local VALTYPE sh[4][4];
        int lidCol = get_global_id(0);
	sh[0][lidCol] = data[0];
	sh[1][lidCol] = data[1];
	sh[2][lidCol] = data[2];
	sh[3][lidCol] = data[3];
	sh[0][lidCol+1] = data[4];
	sh[1][lidCol+1] = data[5];
	sh[2][lidCol+1] = data[6];
	sh[3][lidCol+1] = data[7];

	newData[0] = sh[0][lidCol];
	newData[1] = sh[1][lidCol];
	newData[2] = sh[2][lidCol];
	newData[3] = sh[3][lidCol];
	newData[4] = sh[0][lidCol+1];
	newData[5] = sh[1][lidCol+1];
	newData[6] = sh[2][lidCol+1];
	newData[7] = sh[3][lidCol+1];

        printf("%f\n", newData[0]);
//CHECK: 11.000000
        printf("%f\n", newData[1]);
//CHECK: 22.000000
        printf("%f\n", newData[2]);
//CHECK: 33.000000
        printf("%f\n", newData[3]);
//CHECK: 44.000000
        printf("%f\n", newData[4]);
//CHECK: 55.000000
        printf("%f\n", newData[5]);
//CHECK: 66.000000
        printf("%f\n", newData[6]);
//CHECK: 77.000000
        printf("%f\n", newData[7]);
//CHECK: 88.000000

}

