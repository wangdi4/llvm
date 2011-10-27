__kernel void 
store_float (__global  int *in,__global int *out)
{
	int gid = get_global_id(0);
	int x=gid * 10;
	int y=out[gid]+2;
	
	
	out[gid]=x-y;
	out[gid+gid]=x+y+1;
	

}
