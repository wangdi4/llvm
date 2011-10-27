
__kernel void 
store_float (__global  int *in,__global int *out)
{
	int gid = get_global_id(0);
	int x=2;
	int z=out[gid+1]+2;
	
	if (z>10)
	{
		out[gid]=z-1;
	}
	else
	{
		out[gid]=2;
	}	

}
