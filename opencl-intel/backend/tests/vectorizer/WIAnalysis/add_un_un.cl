//
__kernel void 
store_float (__global  int *in,__global int *out)
{
	int gid = get_global_id(0);
	int x=23;
	int y=1;
	int i;
	for (i=0;i<1000;i++)
	{
		if (y>12)
		{
			out[gid]=x+y;
			y=y+1+i;
		}
		else
		{
			out[gid]=x-2*y;
			x=x-4-i;
			y+=3;
		}
	}	
	
	out[gid]+=x+y;
	

}
