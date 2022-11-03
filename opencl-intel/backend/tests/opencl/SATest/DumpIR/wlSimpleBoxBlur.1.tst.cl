/*
 * wlSimpleBoxBlur
 * simpleBoxBlur: A simple example to demonstrate how to implement a simple
 *                blur filter.  This filter is a good demonstration on how to
 *                access pixels in addition to the one at the current output
 *                coordinate.
 *                Access the nine pixels surrounding the current coordinate
 *location. assuming that the current coordinate is located in the center of the
 *matrix: 1  4  7 2  5  8 3  6  9 If all 9 neighbors are not there for ex.
 *pixels on the corners or edges, access only the ones that are there.
 * @param output output buffer
 * @param input  input buffer
 * @param buffer_size  buffer size
 */

__kernel void wlSimpleBoxBlur_GPU(__global float4 *input,
                                  __global float4 *output, const uint width,
                                  const uint height, const uint buffer_size) {
  // printf("-----------------In wlSimpleBoxBlur KERNEL code------------\n");

  size_t dims = get_work_dim();

  size_t globalIdx = get_global_id(0);
  size_t globalIdy = get_global_id(1);
  size_t localIdx = get_local_id(0);
  size_t localIdy = get_local_id(1);

  const size_t global_szx = get_global_size(0);
  const size_t global_szy = get_global_size(1);
  const size_t local_szx = get_local_size(0);
  const size_t local_szy = get_local_size(1);

  size_t groupIdx = get_group_id(0);
  size_t groupIdy = get_group_id(1);

  // printf("\n Num Dims = %d, Num groups_x = %d, Num groups_y = %d, groupIdx
  // %d, groupIdy %d\n",     dims, get_num_groups(0),
  // get_num_groups(1), groupIdx, groupIdy); printf(" local(%d, %d), global(%d,
  // %d)\n",     localIdx, localIdy, globalIdx, globalIdy); printf("
  // global_szx %d, global_szy %d, local_szx %d, local_szx %d\n",
  // global_szx, global_szy, local_szx, local_szy);

  float denominator = 9.0;
  float4 colorAccumulator = 0.0f; // make_float4(0.0, 0.0, 0.0, 0.0);

  size_t sourceIndex = globalIdy * width + globalIdx;
  size_t tmpIndex1 = (globalIdy - 1) * width + globalIdx;
  size_t tmpIndex2 = (globalIdy + 1) * width + globalIdx;

  colorAccumulator = input[sourceIndex];
  if (globalIdx > 0) {
    colorAccumulator += input[sourceIndex - 1];
  }
  if (globalIdx < (global_szx - 1)) {
    colorAccumulator += input[sourceIndex + 1];
  }

  if (globalIdy > 0) {
    colorAccumulator += input[tmpIndex1];
    if (globalIdx > 0) {
      colorAccumulator += input[tmpIndex1 - 1];
    }
    if (globalIdx < (global_szx - 1)) {
      colorAccumulator += input[tmpIndex1 + 1];
    }
  }

  if (globalIdy < (global_szy - 1)) {
    colorAccumulator += input[tmpIndex2];
    if (globalIdx > 0) {
      colorAccumulator += input[tmpIndex2 - 1];
    }
    if (globalIdx < (global_szx - 1)) {
      colorAccumulator += input[tmpIndex2 + 1];
    }
  }

  // calculate the resultant pixel value which is the accumulated color
  // divided by the total number of neighboring pixels.
  output[sourceIndex] = colorAccumulator / denominator;
}

__kernel void wlSimpleBoxBlur_CPU(__global float4 *input,
                                  __global float4 *output, const uint width,
                                  const uint height, const uint buffer_size) {
  //  printf("-----------------In wlSimpleBoxBlur KERNEL code------------\n");

  size_t dims = get_work_dim();

  size_t globalIdx = get_global_id(0);
  size_t globalIdy = get_global_id(1);
  size_t localIdx = get_local_id(0);
  size_t localIdy = get_local_id(1);

  const size_t global_szx = get_global_size(0);
  const size_t global_szy = get_global_size(1);
  const size_t local_szx = get_local_size(0);
  const size_t local_szy = get_local_size(1);

  size_t groupIdx = get_group_id(0);
  size_t groupIdy = get_group_id(1);

  // printf("\n Num Dims = %d, Num groups_x = %d, Num groups_y = %d, groupIdx
  // %d, groupIdy %d\n",     dims, get_num_groups(0),
  // get_num_groups(1), groupIdx, groupIdy); printf(" local(%d, %d), global(%d,
  // %d)\n",     localIdx, localIdy, globalIdx, globalIdy); printf("
  // global_szx %d, global_szy %d, local_szx %d, local_szx %d\n",
  // global_szx, global_szy, local_szx, local_szy);

  size_t count_x = width / global_szx;
  size_t count_y = height / global_szy;

  uint index_x = width * globalIdx / global_szx;
  uint index_y = height * globalIdy / global_szy;
  uint index_x_orig = index_x;

  //  printf("groupIdx %d, groupIdy %d, count_x %d, count_y %d, index_x %d,
  // index_y %d\n",       groupIdx, groupIdy, count_x, count_y,
  // index_x, index_y);

  float denominator = 9.0;

  for (uint i = 0; i < count_y; i++, index_y++) {
    for (uint j = 0, index_x = index_x_orig; j < count_x; j++, index_x++) {
      float4 colorAccumulator = 0.0f; // make_float4(0.0, 0.0, 0.0, 0.0);

      size_t sourceIndex = index_y * width + index_x;
      size_t tmpIndex1 = (index_y - 1) * width + index_x;
      size_t tmpIndex2 = (index_y + 1) * width + index_x;

      //  printf("index_x, index_y = %d,%d\n", index_x, index_y);

      colorAccumulator = input[sourceIndex];
      if (index_x > 0) {
        colorAccumulator += input[sourceIndex - 1];
      }
      if (index_x < (width - 1)) {
        colorAccumulator += input[sourceIndex + 1];
      }

      if (index_y > 0) {
        colorAccumulator += input[tmpIndex1];
        if (index_x > 0) {
          colorAccumulator += input[tmpIndex1 - 1];
        }
        if (index_x < (width - 1)) {
          colorAccumulator += input[tmpIndex1 + 1];
        }
      }

      if (index_y < (height - 1)) {
        colorAccumulator += input[tmpIndex2];
        if (index_x > 0) {
          colorAccumulator += input[tmpIndex2 - 1];
        }
        if (index_x < (width - 1)) {
          colorAccumulator += input[tmpIndex2 + 1];
        }
      }

      // calculate the resultant pixel value which is the accumulated color
      // divided by the total number of neighboring pixels.
      output[sourceIndex] = colorAccumulator / denominator;
    }
  }
}

////// image2d version of the kernel /////
float4 evaluatePixel(__read_only image2d_t inputImage, int2 outCrd) {
  // sample and set as the output color.  since relativePos
  // is related to the center location, we need to add it back in.
  // We use linear sampling to smooth out some of the pixelation.
  const sampler_t samplerNearest =
      CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_CLAMP;
  float4 outputColor = read_imagef(inputImage, samplerNearest, outCrd);
  return outputColor;
}

__kernel void wlSimpleBoxBlur_image2d(
    __read_only image2d_t
        inputImage, // nVidia requires __read_only on image2d_t
    __global float4 *output, const uint rowCountPerGlobalID) {
  //  printf("-----------------In wlSimpleBoxBlur_image2d KERNEL
  // code------------\n");
  int global_id = get_global_id(0);
  int row = rowCountPerGlobalID * global_id;

#if 0
  printf("global_size = %d\n", get_global_size(0));
  printf("global_id = %d\n", global_id);    
  printf("working on %d rows at row %d\n\n", rowCountPerGlobalID, row);
  printf("output pointer = %p\n", output);
#endif

  int2 imgSize = get_image_dim(inputImage);
  int lastRow = min(row + (int)rowCountPerGlobalID, imgSize.y);
  int index = row * imgSize.x;
  float denominator = 9.0;
  float4 colorAccumulator = 0.0f; // make_float4(0.0, 0.0, 0.0, 0.0);
  int2 curCrd, curLeftCrd, curRightCrd, upCrd, upLeftCrd, upRightCrd, lowCrd,
      lowLeftCrd, lowRightCrd;
  for (; row < lastRow; ++row) {

    curCrd.y = row;
    curLeftCrd.y = row;
    curRightCrd.y = row;
    upCrd.y = row - 1;
    upLeftCrd.y = row - 1;
    upRightCrd.y = row - 1;
    lowCrd.y = row + 1;
    lowLeftCrd.y = row + 1;
    lowRightCrd.y = row + 1;

    for (int col = 0; col < imgSize.x; ++col) {
      curCrd.x = col;
      curLeftCrd.x = col - 1;
      curRightCrd.x = col + 1;
      upCrd.x = col;
      upLeftCrd.x = col - 1;
      upRightCrd.x = col + 1;
      lowCrd.x = col;
      lowLeftCrd.x = col - 1;
      lowRightCrd.x = col + 1;

      colorAccumulator = evaluatePixel(inputImage, curCrd);
      colorAccumulator += evaluatePixel(inputImage, curLeftCrd);
      colorAccumulator += evaluatePixel(inputImage, curRightCrd);
      colorAccumulator += evaluatePixel(inputImage, upCrd);
      colorAccumulator += evaluatePixel(inputImage, upLeftCrd);
      colorAccumulator += evaluatePixel(inputImage, upRightCrd);
      colorAccumulator += evaluatePixel(inputImage, lowCrd);
      colorAccumulator += evaluatePixel(inputImage, lowLeftCrd);
      colorAccumulator += evaluatePixel(inputImage, lowRightCrd);

      output[index++] = colorAccumulator / denominator;
    }
  }
}

__kernel void wlSimpleBoxBlur_Optimized_CPU(__global float4 *input,
                                            __global float4 *output,
                                            const uint width, const uint height,
                                            const uint buffer_size) {
  //  printf("-----------------In wlSimpleBoxBlur KERNEL code------------\n");
  bool topEdge, bottomEdge, leftEdge, rightEdge;
  topEdge = false;
  bottomEdge = false;
  leftEdge = false;
  rightEdge = false;

  size_t dims = get_work_dim();

  size_t globalIdx = get_global_id(0);
  size_t globalIdy = get_global_id(1);

  const size_t global_szx = get_global_size(0);
  const size_t global_szy = get_global_size(1);

  size_t count_y = height / global_szy;
  size_t count_x = width / global_szx;

  uint index_x = width * globalIdx / global_szx;
  uint index_y = height * globalIdy / global_szy;

  // printf("ThreadId %d, global_szx %d, global_szy %d, globalIdx %d, globalIdy
  // %d\n",     iThreadId, global_szx, global_szy, globalIdx,
  // globalIdy);

  // Ignore edges for now ...
  if ((index_y + count_y + 1) >= height) {
    bottomEdge = true;
    count_y = height - index_y - 1;
  }
  if ((index_x + count_x + 1) >= width) {
    rightEdge = true;
    count_x = width - index_x - 1;
  }
  if (index_y < 1) {
    topEdge = true;
    index_y = 1;
    count_y -= 1;
  }
  if (index_x < 1) {
    leftEdge = true;
    index_x = 1;
    count_x -= 1;
  }
  //  printf ("index_x %d, index_y %d, count_x %d, count_y %d \n", index_x,
  // index_y, count_x, count_y);

  // Initialize variables
  float denominator = 9;
  float4 colorAccumulator = 0.0f;      // make_float4(0.0, 0.0, 0.0, 0.0);
  float4 firstBlockAccumulator = 0.0f; // make_float4(0.0, 0.0, 0.0, 0.0);

  // Calculate the first accumulated block value for the whole block.
  // We can then just add / subtract from this value for each subsequent result
  // in either x or y direction.
  size_t leftIndex = (index_y - 1) * width + index_x - 1;

  for (unsigned y = 0; y < 3; y++) {
    firstBlockAccumulator += input[leftIndex];
    firstBlockAccumulator += input[leftIndex + 1];
    firstBlockAccumulator += input[leftIndex + 2];
    leftIndex += width;
  }

  size_t sourceIndex = index_y * width + index_x;
  output[sourceIndex] = firstBlockAccumulator / denominator;
  colorAccumulator = firstBlockAccumulator;
  sourceIndex++;

  size_t leftColumnIndex;
  size_t rightColumnIndex;
  size_t tmpIndex1;
  size_t tmpIndex2;

  size_t topRowIndex = (index_y - 1) * width + index_x - 1;
  size_t bottomRowIndex = (index_y + 1) * width + index_x - 1;
  ;

  for (unsigned int row = 0; row < count_y - 1;) {
    leftColumnIndex = topRowIndex;
    rightColumnIndex = topRowIndex + 2;
    // Calculate the next column in the x direction.
    for (unsigned int column = 1; column < count_x; column++) {
      rightColumnIndex++;
      tmpIndex1 = leftColumnIndex;
      tmpIndex2 = rightColumnIndex;

      colorAccumulator -= input[tmpIndex1];
      tmpIndex1 += width;
      colorAccumulator += input[tmpIndex2];
      tmpIndex2 += width;
      colorAccumulator -= input[tmpIndex1];
      tmpIndex1 += width;
      colorAccumulator += input[tmpIndex2];
      tmpIndex2 += width;
      colorAccumulator -= input[tmpIndex1];
      colorAccumulator += input[tmpIndex2];

      output[sourceIndex] = colorAccumulator / denominator;
      sourceIndex++;
      leftColumnIndex++;
    }
    // Calculate the next row's in y direction
    bottomRowIndex += width;
    row++;
    tmpIndex1 = topRowIndex;
    tmpIndex2 = bottomRowIndex;

    firstBlockAccumulator -= input[tmpIndex1];
    tmpIndex1++;
    firstBlockAccumulator += input[tmpIndex2];
    tmpIndex2++;
    firstBlockAccumulator -= input[tmpIndex1];
    tmpIndex1++;
    firstBlockAccumulator += input[tmpIndex2];
    tmpIndex2++;
    firstBlockAccumulator -= input[tmpIndex1];
    firstBlockAccumulator += input[tmpIndex2];

    sourceIndex = (index_y + row) * width + index_x;
    output[sourceIndex] = firstBlockAccumulator / denominator;
    colorAccumulator = firstBlockAccumulator;
    sourceIndex++;
    topRowIndex += width;
  }
  // Finish up the last row in the center
  leftColumnIndex = topRowIndex;
  rightColumnIndex = topRowIndex + 2;

  for (unsigned int column = 1; column < count_x; column++) {
    rightColumnIndex++;
    tmpIndex1 = leftColumnIndex;
    tmpIndex2 = rightColumnIndex;

    colorAccumulator -= input[tmpIndex1];
    tmpIndex1 += width;
    colorAccumulator += input[tmpIndex2];
    tmpIndex2 += width;
    colorAccumulator -= input[tmpIndex1];
    tmpIndex1 += width;
    colorAccumulator += input[tmpIndex2];
    tmpIndex2 += width;
    colorAccumulator -= input[tmpIndex1];
    colorAccumulator += input[tmpIndex2];

    output[sourceIndex] = colorAccumulator / denominator;
    sourceIndex++;
    leftColumnIndex++;
  }

  if (topEdge && leftEdge) {
    colorAccumulator = 0.0f; // make_float4(0.0, 0.0, 0.0, 0.0);
    colorAccumulator += input[0];
    colorAccumulator += input[1];
    colorAccumulator += input[width];
    colorAccumulator += input[width + 1];
#if CLAMP_TO_EDGE
    colorAccumulator += input[0];
    colorAccumulator += input[0];
    colorAccumulator += input[1];
    colorAccumulator += input[0];
    colorAccumulator += input[width];
#else // consider missing neighbors as black
#endif
    output[0] = colorAccumulator / denominator;
  }

  if (topEdge) {
    for (int column = index_x; column < index_x + count_x; column++) {
      colorAccumulator = 0.0f; // make_float4(0.0, 0.0, 0.0, 0.0);
      colorAccumulator += input[column - 1];
      colorAccumulator += input[column];
      colorAccumulator += input[column + 1];
      colorAccumulator += input[width + column - 1];
      colorAccumulator += input[width + column];
      colorAccumulator += input[width + column + 1];
#if CLAMP_TO_EDGE
      colorAccumulator +=, input[column - 1];
      colorAccumulator +=, input[column];
      colorAccumulator += input[column + 1];
#else // consider missing neighbors as black
#endif
      output[column] = colorAccumulator / denominator;
    }
  }

  if (topEdge && rightEdge) {
    colorAccumulator = 0.0f; // make_float4(0.0, 0.0, 0.0, 0.0);
    colorAccumulator += input[width - 2];
    colorAccumulator += input[width - 1];
    colorAccumulator += input[width + width - 2];
    colorAccumulator += input[width + width - 1];
#if CLAMP_TO_EDGE
    colorAccumulator += input[width - 2];
    colorAccumulator += input[width - 1];
    colorAccumulator += input[width - 1];
    colorAccumulator += input[width - 1];
    colorAccumulator += input[width + width - 1];
#else // consider missing neighbors as black
#endif
    output[width - 1] = colorAccumulator / denominator;
  }

  if (leftEdge) {
    for (int row = index_y; row < index_y + count_y; row++) {
      colorAccumulator = 0.0f; // make_float4(0.0, 0.0, 0.0, 0.0);
      colorAccumulator += input[(row - 1) * width];
      colorAccumulator += input[row * width];
      colorAccumulator += input[(row + 1) * width];
      colorAccumulator += input[(row - 1) * width + 1];
      colorAccumulator += input[row * width + 1];
      colorAccumulator += input[(row + 1) * width + 1];
#if CLAMP_TO_EDGE
      colorAccumulator += input[(row - 1) * width];
      colorAccumulator += input[row * width];
      colorAccumulator += input[(row + 1) * width];
#else // consider missing neighbors as black
#endif
      output[row * width] = colorAccumulator / denominator;
    }
  }

  if (bottomEdge && leftEdge) {
    colorAccumulator = 0.0f; // make_float4(0.0, 0.0, 0.0, 0.0);
    colorAccumulator += input[(height - 2) * width];
    colorAccumulator += input[(height - 2) * width + 1];
    colorAccumulator += input[(height - 1) * width];
    colorAccumulator += input[(height - 1) * width + 1];
#if CLAMP_TO_EDGE
    colorAccumulator += input[(height - 2) * width];
    colorAccumulator += input[(height - 1) * width];
    colorAccumulator += input[(height - 1) * width];
    colorAccumulator += input[(height - 1) * width];
    colorAccumulator += input[(height - 1) * width + 1];
#else // consider missing neighbors as black
#endif
    output[(height - 1) * width] = colorAccumulator / denominator;
  }

  if (bottomEdge) {
    for (int column = index_x; column < index_x + count_x; column++) {
      colorAccumulator = 0.0f; // make_float4(0.0, 0.0, 0.0, 0.0);
      colorAccumulator += input[(height - 2) * width + column - 1];
      colorAccumulator += input[(height - 2) * width + column];
      colorAccumulator += input[(height - 2) * width + column + 1];
      colorAccumulator += input[(height - 1) * width + column - 1];
      colorAccumulator += input[(height - 1) * width + column];
      colorAccumulator += input[(height - 1) * width + column + 1];
#if CLAMP_TO_EDGE
      colorAccumulator += input[(height - 1) * width + column - 1];
      colorAccumulator += input[(height - 1) * width + column];
      colorAccumulator += input[(height - 1) * width + column + 1];
#else // consider missing neighbors as black
#endif
      output[(height - 1) * width + column] = colorAccumulator / denominator;
    }
  }

  if (rightEdge) {
    for (int row = index_y; row < index_y + count_y; row++) {
      colorAccumulator = 0.0f; // make_float4(0.0, 0.0, 0.0, 0.0);
      colorAccumulator += input[(row * width) - 1];
      colorAccumulator += input[(row + 1) * width - 1];
      colorAccumulator += input[(row + 2) * width - 1];
      colorAccumulator += input[(row * width) - 2];
      colorAccumulator += input[(row + 1) * width - 2];
      colorAccumulator += input[(row + 2) * width - 2];
#if CLAMP_TO_EDGE
      colorAccumulator += input[(row * width) - 1];
      colorAccumulator += input[(row + 1) * width - 1];
      colorAccumulator += input[(row + 2) * width - 1];
#else // consider missing neighbors as black
#endif
      output[(row + 1) * width - 1] = colorAccumulator / denominator;
    }
  }

  if (bottomEdge && rightEdge) {
    colorAccumulator = 0.0f; // make_float4(0.0, 0.0, 0.0, 0.0);
    colorAccumulator += input[(height - 1) * width - 2];
    colorAccumulator += input[(height - 1) * width - 1];
    colorAccumulator += input[height * width - 2];
    colorAccumulator += input[height * width - 1];
#if CLAMP_TO_EDGE
    colorAccumulator += input[(height - 1) * width - 1];
    colorAccumulator += input[height * width - 2];
    colorAccumulator += input[height * width - 1];
    colorAccumulator += input[height * width - 1];
    colorAccumulator += input[height * width - 1];
#else // consider missing neighbors as black
#endif
    output[(height * width) - 1] = colorAccumulator / denominator;
  }
}