__kernel void 
store_float (__global  int *in,__global int *out)
{
	int gid = get_global_id(0);
	int x=out[gid];
	int y=gid;
	
	
	out[gid]=x-y;
	

}
