__kernel void 
store_float (__global  int *in,__global int *out)
{
	int gid = get_global_id(0);
	int x=gid;
	int y=gid+10;
	int z;
	
	if (y>11)
	{
		z=x+y;
	}
	else
	{
		z=4*y;
	
	}
	
	
	out[gid]=z-1;
	

}
