__kernel void main_kernel(__global uchar* buf_in, __global uchar* buf_out)
{
    __local int local_arr[4];
	int checker = 0;
	// Initialize once (to 1024)
	if (get_global_id(0) == 0)
	{
		buf_out[0] = 0;
		buf_out[1] = 4;
		buf_out[2] = 0;
		buf_out[3] = 0;
	}
	barrier(CLK_GLOBAL_MEM_FENCE);

    // Now each WI increments the value atomically
    __global unsigned int* bufuip = &buf_out[0];
    atomic_add(bufuip, 1);
	barrier(CLK_GLOBAL_MEM_FENCE);
	uint result = *((__global uint*) &buf_out[0]);
	int i = 1;
	// some function calls without any purpose except using them
	mem_fence(CLK_GLOBAL_MEM_FENCE);
	mem_fence(CLK_LOCAL_MEM_FENCE);
	read_mem_fence(CLK_GLOBAL_MEM_FENCE);
	read_mem_fence(CLK_LOCAL_MEM_FENCE);
	//write_mem_fence(CLK_GLOBAL_MEM_FENCE); THERE IS A PR0BLEM WITH THESE COMMANDS, SEE FILE testcases\test_barriers.py
	//write_mem_fence(CLK_LOCAL_MEM_FENCE);
	if (get_local_id(0) == 0)
	{
		local_arr[0] = 0;
		local_arr[1] = 1;
		local_arr[2] = 1;
		local_arr[3] = 1;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	__local unsigned int* local_arrp = &local_arr[0];
    atomic_add(local_arrp, 1);
	barrier(CLK_LOCAL_MEM_FENCE);
	i++;
}
