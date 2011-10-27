

#include "def.h"
__kernel void
check_vectrorazible_f_with_scalar_arg ( __global  ulong *in,
		__global float *out)
{
	int gid = get_global_id(0);
	int i_var1 = gid+4;
	double d_var1 = 3.14;
	double d_var2 = (double) gid  + (double) ( in[0]);
	double d_var3 = d_var1 + d_var2;
	double d_var4 = (d_var3 * gid) + distance(d_var1,d_var2);
	double d_var5 = d_var4 / d_var1 ; 
	
	out[gid] =d_var5 *d_var4 - d_var3;
}
