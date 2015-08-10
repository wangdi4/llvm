// assert with scalar select in Vectorizer

__kernel
void generateHitDataPrim(__global const int* restrict early_exit,
                         __global const float* in_select_arg1,
                         __global const float* in_select_arg2,
                         __global float* out_dets
                         )
{
    const size_t idX = get_global_id(0);
    const size_t idY = get_global_id(1);
    const size_t sizeX = get_global_size(0);
    const size_t sizeY = get_global_size(1);

    const size_t currIndex = idY * sizeX + idX;

    const int early_exit_condition = early_exit[currIndex];

    if( early_exit_condition != INT_MAX )
    {
        int scalar_select_mask = 0;

        float scalar_select_result = select(in_select_arg1[currIndex], in_select_arg2[currIndex], scalar_select_mask == 0);

        out_dets[currIndex] = scalar_select_result * scalar_select_result;
    }
    else {
        out_dets[currIndex] = 0;
    }
}
