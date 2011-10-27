
 #include "def.h"
__kernel void
encountered_not_scal_variable(float *in1,uchar4 *in2,__global uchar *out,float4 fArg)
{
	int gid = get_global_id(0);
	
//float4 temp = (in1[0]+gid,in1[1],in1[2],in1[3]);
	float temp2 = fArg.s0+fArg.s1;
	float temp3 = fArg.s0*fArg.s1;
	
	out[gid]=temp2 +temp3;


}