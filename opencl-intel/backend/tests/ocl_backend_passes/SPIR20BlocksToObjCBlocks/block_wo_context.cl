void block_fn(int arg, __global int* res)
{
  *res = arg;
}

__global int glbRes = 0;
void (^kernelBlockNoCtx)(void) = ^{ block_fn(1, &glbRes); };

kernel void enqueue_block_wo_context(__global int* res)
{
    queue_t q1 = get_default_queue();
    ndrange_t ndrange = ndrange_1D(1, 1);
    // Enqueue kernel w\o captured context
    enqueue_kernel(q1, CLK_ENQUEUE_FLAGS_WAIT_KERNEL, ndrange, kernelBlockNoCtx);
}
