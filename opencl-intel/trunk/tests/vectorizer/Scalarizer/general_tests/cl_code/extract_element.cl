
 #include "def.h"
__kernel void
encountered_not_scal_variable(uchar4 *in1,uchar4 *in2,__global uchar *out)
{
	int gid = get_global_id(0);
	uchar4 temp = in1[0]* in2[0];
	uchar4 temp_rez = rhadd(  temp, in1[0]);
	out[gid]=temp_rez.s0 + temp_rez.s1+ temp.s3;

}