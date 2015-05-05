__kernel void test( __global int* res,int m,int val )
{
    int lid = get_local_id(0);
    int x = m + lid;

    if( x < val )
    {
		atomic_add(res,1<<get_global_id(0));
    }
}
