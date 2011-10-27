__kernel void 
store_float (__global  int *in,__global int *out)
{
	int gid = get_global_id(0);
	int x=gid+123;  //seq
	int y=gid*12;   //str
	
	
	out[gid]=x+y;
	

}
