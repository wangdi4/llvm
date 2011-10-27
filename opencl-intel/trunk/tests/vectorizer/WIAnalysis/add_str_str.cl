__kernel void 
store_float (__global  int *in,__global int *out)
{
	int gid = get_global_id(0);
	int x=gid*2+123;
	int y=gid*11;
	
	
	
	y=y-gid;
	
	
	
	out[gid]=x+y;
	

}
