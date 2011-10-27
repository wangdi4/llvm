
__kernel void 
store_float (__global  int *in,__global int *out, global int *p)
{
	int gid = get_global_id(0);
	p=&in[gid]+gid;
	
	if (p>10)
	{
		p=p+633;
	}
	else
	{
		p=p+111;
	}	

	out[gid]=*p;
}
