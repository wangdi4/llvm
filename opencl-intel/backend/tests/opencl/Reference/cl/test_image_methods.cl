typedef struct {
    int width;
    int height;
    int depth;
    int widthDim;
    int heightDim;
    int depthDim;
    int channelType;
    int channelOrder;
    int expectedChannelType;
    int expectedChannelOrder;
 } image_kernel_data;
__kernel void sample_kernel( read_only image3d_t input, __global image_kernel_data *outData )
{
   outData->width = get_image_width( input );
   outData->height = get_image_height( input );
   outData->depth = get_image_depth( input );

   int4 dim = get_image_dim( input );
   outData->widthDim = dim.x;
   outData->heightDim = dim.y;
   outData->depthDim = dim.z;

   outData->channelType = get_image_channel_data_type( input );
   outData->channelOrder = get_image_channel_order( input );

   outData->expectedChannelType = CLK_UNORM_INT16;
   outData->expectedChannelOrder = CLK_R;
}
