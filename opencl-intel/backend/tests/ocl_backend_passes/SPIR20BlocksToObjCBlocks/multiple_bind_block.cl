void block_fn(int arg, __global int* res)
{
  *res = arg;
}

kernel void multiple_bind_block(__global int* res)
{
    void (^kernelBlock)(void) = ^{ block_fn(2, res); };
    uint globalSize = get_kernel_work_group_size(kernelBlock);
    uint multiple   = get_kernel_preferred_work_group_size_multiple(kernelBlock);
    uint localSize  = globalSize / multiple;

    queue_t q1 = get_default_queue();
    ndrange_t ndrange = ndrange_1D(localSize, globalSize);
    enqueue_kernel(q1, CLK_ENQUEUE_FLAGS_WAIT_KERNEL, ndrange, kernelBlock);
}
