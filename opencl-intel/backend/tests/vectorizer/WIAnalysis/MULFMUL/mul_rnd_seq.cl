__kernel void 
store_float (__global  int *in,__global int *out)
{
	int gid = get_global_id(0);
	int x=in[gid];
	int y=(gid+11);
	int i=0;
	int z;
	
	
	for (i=0;i<2000;i++)
	{
		if (i>300)
		{
			x+=x;
		}
		
	}		
	
	z=x*y;
	
	out[gid]=z;
	
	z=z*gid;
	out[gid+10]=z;
	
	

}
