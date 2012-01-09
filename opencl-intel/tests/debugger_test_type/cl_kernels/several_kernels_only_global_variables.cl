__constant uchar c = 6;
__constant int b = 5;
__constant int d = 4;
__kernel void fourth_kernel()
{
	d++;
	d++;
	d++;
	return;
}

__kernel void third_kernel()
{
	d++;
	d++;
	fourth_kernel();
	return;
}

__kernel void second_kernel()
{
	d++;
	d++;
	third_kernel();
	return;
}

__kernel void main_kernel(__global uchar* buf_in, __global uchar* buf_out)
{
	d++;
	d++;
	second_kernel();
	return;
}
