#define MAX_CHANNEL_SIZE 888
#define MAX_ROW_SIZE 256
__kernel void SampleKernel(const __global float *input, __global float *output,
                           __const int num_channels, __const int num_rows,
                           const __global float *input_kernel,
                           __const int smoothLen) {
  float edge_kernel[MAX_ROW_SIZE];
  bool edgeAxial = false;
  size_t num_local_channels = num_channels / get_num_groups(0);
  size_t num_local_rows = num_rows / get_num_groups(1);
  size_t start_channel = num_local_channels * get_group_id(0);
  size_t start_row = num_local_rows * get_group_id(1);
  size_t end_row = start_row + num_local_rows;
  size_t end_channel = start_channel + num_local_channels;
  for (size_t row = start_row; row < end_row; row++) {
    for (size_t channel = start_channel; channel < end_channel; channel++) {
      output[channel + row * num_channels] = 0.0f;
    }
    int smoothStart = row - smoothLen;
    int smoothFinish = row + smoothLen;
    if (smoothStart < 0 || smoothFinish >= num_rows) {
      float fRes = 0.0f;
      // Calculation of new normalization factor
      for (int i = smoothStart, nWeight = 0; i <= smoothFinish;
           i++, nWeight++) {
        if (i >= 0 && i < num_rows) {
          fRes += input_kernel[nWeight];
        }
      }
      // Re-calculation of the weights
      for (int i = smoothStart, ind = 0, nWeight = 0; i <= smoothFinish;
           i++, nWeight++) {
        if (i >= 0 && i < num_rows) {
          edge_kernel[ind] = input_kernel[nWeight] / fRes;
          ind++;
        }
      }
      // Updating of the kernel support
      if (smoothStart < 0) {
        smoothStart = 0;
      }
      if (smoothFinish >= num_rows) {
        smoothFinish = num_rows - 1;
      }
      edgeAxial = true;
    } else {
      edgeAxial = false;
    }
    // Convolution of the input rows and the smoothing kernel
    for (int rr = smoothStart, w = 0; rr <= smoothFinish; rr++, w++) {
      int input_address = 0;
      if (rr >= num_rows) {
        input_address = (num_rows - 1) * num_channels;
      } else if (rr >= 0) {
        input_address = rr * num_channels;
      }
      float curr_weight = 0.0f;
      if (edgeAxial)
        curr_weight = edge_kernel[w];
      else
        curr_weight = input_kernel[w];
      for (size_t channel = start_channel; channel < end_channel; channel++) {
        output[channel + row * num_channels] +=
            input[channel + input_address] * curr_weight;
      }
    }
  }
}
__kernel void BlankSampleKernel(const __global float *input,
                                __global float *output,
                                __const int num_channels, __const int num_rows,
                                const __global float *input_kernel,
                                __const int smoothLen) {}
