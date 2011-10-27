__kernel void 
store_float (__global  int *in,__global int *out)
{
	int gid = get_global_id(0);
	int x=gid*11+100;
	int y=gid*7-12;
	int z;
	int i;
	
	
	
	
	

	
	out[gid]=x/y;
	
	out[gid+10]=y/x;
	
	

}
