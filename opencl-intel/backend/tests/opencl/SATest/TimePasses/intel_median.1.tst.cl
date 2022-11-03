
// 3x3 median filter kernel based on bitonic and binary search
// http://en.wikipedia.org/wiki/Binary_search_algorithm
//

__kernel void intel_median(__global uchar4 *pSrc, __global unsigned int *pDst,
                           int iImageWidth, int iImageHeight) {
  /// int x = get_global_id(0);
  /// int y = get_global_id(1);
  int y = get_global_id(0);
  for (int x = 0; x < iImageWidth; x++) // columns loop
  {

    // Reset accumulators
    int4 iYes = {128, 128, 128, 128};
    int4 iMin = {0, 0, 0, 0};
    int4 iMax = {255, 255, 255, 255};

    // Local registers
    uchar4 ucRGBA;
    const uint uiZerro = 0;
    int4 iZerro = {0, 0, 0, 0};
    int4 iFour = {4, 4, 4, 4};
    int4 iMask;
    int4 iPixels[9];
    int iPixelCount = 0;

    // Load necessary data
    for (int iRow = -1; iRow <= 1; iRow++) // 3 rows
    {
      /// int iLocalOffset = mul24(iRow, iImageWidth) + x;
      int iLocalOffset = (y + iRow + 2) * iImageWidth + x;

      // Left Pix (RGB)
      // Read left pixel
      ucRGBA = pSrc[iLocalOffset - 1];
      iPixels[iPixelCount].x = ucRGBA.x;
      iPixels[iPixelCount].y = ucRGBA.y;
      iPixels[iPixelCount].z = ucRGBA.z;
      iPixelCount++;

      // Middle Pix (RGB)
      // Read middle pixel
      ucRGBA = pSrc[iLocalOffset];
      iPixels[iPixelCount].x = ucRGBA.x;
      iPixels[iPixelCount].y = ucRGBA.y;
      iPixels[iPixelCount].z = ucRGBA.z;
      iPixelCount++;

      // Right Pix (RGB)
      // Read right pixel
      ucRGBA = pSrc[iLocalOffset + 1];
      iPixels[iPixelCount].x = ucRGBA.x;
      iPixels[iPixelCount].y = ucRGBA.y;
      iPixels[iPixelCount].z = ucRGBA.z;
      iPixelCount++;
    }

    // Median binary search
    // For 8-bit colors iSearch_max = 8 (256 levels = 2^8).
    // For 16-bit colors iSearch_max = 16 (65536 levels = 2^16).
    int iSearch_max = 8;
    for (int iSearch = 0; iSearch < iSearch_max; iSearch++) {
      int4 iHighCount = {0, 0, 0, 0};
      iPixelCount = 0;
      for (int iRow = -1; iRow <= 1; iRow++) // 3 rows
      {

        // Left Pix (RGB)
        /// iHighCount += isless(iYes, iPixels[iPixelCount]);
        iHighCount += iYes < iPixels[iPixelCount];
        iPixelCount++;

        // Middle Pix (RGB)
        /// iHighCount += isless(iYes, iPixels[iPixelCount]);
        iHighCount += iYes < iPixels[iPixelCount];
        iPixelCount++;

        // Right Pix (RGB)
        /// iHighCount += isless(iYes, iPixels[iPixelCount]);
        iHighCount += iYes < iPixels[iPixelCount];
        iPixelCount++;
      }

      // 0 if false, -1 if true; negative value accumulated
      //  so we just change sign
      iHighCount = iZerro - iHighCount;

      /// printf("%d %d %d\n", iHighCount.x, iHighCount.y, iHighCount.z);
      //********************************
      // Update Min and Max values
      /// iMask = isgreater(iHighCount, iFour);
      iMask = iHighCount > iFour;
      iMin = (iYes & iMask) | (iMin & (~iMask));
      iMax = (iYes & (~iMask)) | (iMax & iMask);
      // Update median estimate
      /// iYes = (iMax + iMin)/2;
      iYes = (iMax + iMin) >> 1;
      /// iYes = (iMax + iMin)*0.5;
    }

    // Pack in uint output value
    unsigned int uiPackedPixel = 0x000000FF & (unsigned int)iYes.x;
    uiPackedPixel |= 0x0000FF00 & (((unsigned int)iYes.y) << 8);
    uiPackedPixel |= 0x00FF0000 & (((unsigned int)iYes.z) << 16);

    // Convert and copy to output
    pDst[(y + 2) * iImageWidth + x] = uiPackedPixel;
  }

} //__kernel

__kernel void intel_median_scalar(__global uchar *pSrc,
                                  __global unsigned int *pDst, int iImageWidth,
                                  int iImageHeight) {

  /// int x = get_global_id(0);
  /// int y = get_global_id(1);
  int y = get_global_id(0);
  for (int x = 0; x < iImageWidth; x++) // columns loop
  {
    // Local registers
    /// uchar* pRGBA;
    const uint uiZerro = 0;
    int iZerro = 0;
    int iFour = 4;
    int iResult[4];
    for (int ch = 0; ch < 3; ch++) {
      // Reset accumulators
      int iYes = 128;
      int iMin = 0;
      int iMax = 255;
      int iMask;
      int iPixels[9];
      int iPixelCount = 0;

      // Load necessary data
      /// for (int iRow = y - 1; iRow <= y + 1; iRow++) (vectorizer fails)
      for (int iRow = -1; iRow <= 1; iRow++) // 3 rows  (enables vectorizer)
      {
        /// int iLocalOffset = mul24(iRow, iImageWidth) + x;
        /// int iLocalOffset = (iRow+2)*iImageWidth + x;
        int iLocalOffset = (y + iRow + 2) * iImageWidth + x;

        // Left Pix (RGB)
        // Read left pixel
        // pRGBA = pSrc[(iLocalOffset - 1)*4+ch];
        iPixels[iPixelCount] = pSrc[(iLocalOffset - 1) * 4 + ch]; // pRGBA[ch];
        iPixelCount++;

        // Middle Pix (RGB)
        // Read middle pixel
        // pRGBA = (uchar*)&pSrc [iLocalOffset];
        iPixels[iPixelCount] = pSrc[(iLocalOffset)*4 + ch]; // pRGBA[ch];
        iPixelCount++;

        // Right Pix (RGB)
        // Read in next pixel value to a local register:  if boundary pixel, use
        // zero
        // pRGBA = (uchar*)&pSrc [iLocalOffset + 1];
        iPixels[iPixelCount] = pSrc[(iLocalOffset + 1) * 4 + ch]; // pRGBA[ch];
        iPixelCount++;
      }

      // Median binary search
      // For 8-bit colors iSearch_max = 8 (256 levels = 2^8).
      // For 16-bit colors iSearch_max = 16 (65536 levels = 2^16).
      int iSearch_max = 8;
      for (int iSearch = 0; iSearch < iSearch_max; iSearch++) {
        int iHighCount = 0;
        iPixelCount = 0;
        /// for (int iRow = y - 1; iRow <= y + 1; iRow++)
        for (int iRow = -1; iRow <= 1; iRow++) // 3 rows
        {

          // Left Pix (RGB)
          iHighCount += iYes < iPixels[iPixelCount];
          iPixelCount++;

          // Middle Pix (RGB)
          iHighCount += iYes < iPixels[iPixelCount];
          iPixelCount++;

          // Right Pix (RGB)
          iHighCount += iYes < iPixels[iPixelCount];
          iPixelCount++;
        }

        // printf("%d\n", iHighCount );

        //********************************
        // Update Min and Max values
        iMin = iHighCount > iFour ? iYes : iMin;
        iMax = iHighCount <= iFour ? iYes : iMax;
        // Update median estimate
        /// iYes = (iMax + iMin)/2;
        iYes = (iMax + iMin) >> 1;
      }

      // copy to output
      ///((uchar*)(&pDst[(y+2) * iImageWidth + x]))[ch] = iYes;
      /// pDst[((y+2) * iImageWidth + x)*4 + ch] = iYes;
      iResult[ch] = iYes;
    } // ch
    // Pack in uint output value
    unsigned int uiPackedPixel = 0x000000FF & (unsigned int)iResult[0];
    uiPackedPixel |= 0x0000FF00 & (((unsigned int)iResult[1]) << 8);
    uiPackedPixel |= 0x00FF0000 & (((unsigned int)iResult[2]) << 16);

    // Convert and copy to output
    pDst[(y + 2) * iImageWidth + x] = uiPackedPixel;
  }

} //__kernel
