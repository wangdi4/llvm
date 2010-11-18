// lrb_program.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "lrb_program.h"
#include <stdio.h>

// 
//
LRB_PROGRAM_API int   lrb_program_prototypes_count=2;
LRB_PROGRAM_API char* lrb_program_prototypes[] = 
{
    "dot_product",
    "hallo_world"
};

/************************************************************************
 * 
 ************************************************************************/
float dot_func(const float a[4], const float b[4])
{
    float res = a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3];
    return res;
}

/************************************************************************
 * 
 ************************************************************************/
LRB_PROGRAM_API void dot_product (const float* a, const float* b, float *c, int tid)
{
    c[tid] = dot_func(a+4*tid, b+4*tid);
}

/************************************************************************
 * 
 ************************************************************************/
LRB_PROGRAM_API void hallo_world(const float* a, const float* b, float *c, int tid)
{
    printf("======== hallo_world (global id: %4d) ==========\n", tid);
}
