__kernel void 
store_float (__global  int *in,__global int *out)
{
	int gid = get_global_id(0);
	int x=in[gid];
	int y=x+2;
	
	
	out[gid]=y-10;
	

}
