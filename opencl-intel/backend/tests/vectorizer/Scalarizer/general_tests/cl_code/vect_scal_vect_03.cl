/*
 * Part of the tests that check different LLVM instructions on different argument
 * types and different argument sources.
 *
 * Tests the OpenCL
 * This function is vectorizable.
 *
 * @param in
 * 		The input array.
 * @param out
 * 		The output array.
 *
 */

#include "def.h"
__kernel void
func_vect_scan ( const __global int *in,
		__global int2 *out)
{
	int gid = get_global_id(0);
	int2 v1_1= (int2) (in[1], gid);
	
	out[gid] =v1_1 + (int2) ( 14, 257) ;
}
