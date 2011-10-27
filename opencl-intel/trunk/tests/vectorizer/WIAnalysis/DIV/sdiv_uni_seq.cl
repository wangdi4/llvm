__kernel void 
store_float (__global  int *in,__global int *out)
{
	int gid = get_global_id(0);
	int x=143;
	int y=gid;
	int z;
	int i;
	
	
	for (i=1;i<1000;i++)
	{
		if (i>12)
		{
				y+=12;
				x-=1;
		}
		y+=i;
	}	
	
	

	
	out[gid]=x/y;
	
	out[gid+10]=y/x;
	
	

}
