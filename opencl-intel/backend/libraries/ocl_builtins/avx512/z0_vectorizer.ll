;; ------------------------------------
;;       GATHER/SCATTER
;; ------------------------------------
declare <16 x float> @llvm.x86.avx512.mask.gather.dps.512 (<16 x float>, i8*, <16 x i32>, <16 x i1>, i32);
declare void @llvm.x86.avx512.mask.scatter.dps.512 (i8*, <16 x i1>, <16 x i32>, <16 x float>, i32)
declare <8 x double> @llvm.x86.avx512.mask.gather.dpd.512 (<8 x double>, i8*, <8 x i32>, <8 x i1>, i32);
declare void @llvm.x86.avx512.mask.scatter.dpd.512 (i8*, <8 x i1>, <8 x i32>, <8 x double>, i32)
declare <8 x float> @llvm.x86.avx512.mask.gather.qps.512 (<8 x float>, i8*, <8 x i64>, <8 x i1>, i32);
declare void @llvm.x86.avx512.mask.scatter.qps.512 (i8*, <8 x i1>, <8 x i64>, <8 x float>, i32)

declare <8 x double> @llvm.x86.avx512.mask.gather.qpd.512(<8 x double>, i8*, <8 x i64>, <8 x i1>, i32);
declare void @llvm.x86.avx512.mask.scatter.qpd.512(i8*, <8 x i1>, <8 x i64>, <8 x double>, i32)

declare <16 x i32> @llvm.x86.avx512.mask.gather.dpi.512 (<16 x i32>, i8*, <16 x i32>, <16 x i1>, i32)
declare void @llvm.x86.avx512.mask.scatter.dpi.512 (i8*, <16 x i1>, <16 x i32>, <16 x i32>, i32)
declare <8 x i64> @llvm.x86.avx512.mask.gather.dpq.512 (<8 x i64>, i8*, <8 x i32>, <8 x i1>, i32)
declare void @llvm.x86.avx512.mask.scatter.dpq.512 (i8*, <8 x i1>, <8 x i32>, <8 x i64>, i32)
declare <8 x i32> @llvm.x86.avx512.mask.gather.qpi.512 (<8 x i32>, i8*, <8 x i64>, <8 x i1>, i32)
declare void @llvm.x86.avx512.mask.scatter.qpi.512 (i8*, <8 x i1>, <8 x i64>, <8 x i32>, i32)

declare <8 x i64> @llvm.x86.avx512.mask.gather.qpq.512 (<8 x i64>, i8*, <8 x i64>, <8 x i1>, i32)
declare void @llvm.x86.avx512.mask.scatter.qpq.512 (i8*, <8 x i1>, <8 x i64>, <8 x i64>, i32)

;; ------------------------------------
;;       Gather for float
;; ------------------------------------
define <16 x float> @gather.v16f32 (float* %addr, <16 x i32>%index) {
  %ptr = bitcast float *%addr to i8*
  %t0 = call <16 x float> @llvm.x86.avx512.mask.gather.dps.512(<16 x float>undef, i8* %ptr, <16 x i32> %index, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 4);
  ret <16 x float> %t0
}

define <16 x float> @gather.v16f32_ind_v16i32 (float* %addr, <16 x i32>%index) {
  %t0 = call <16 x float> @gather.v16f32(float* %addr, <16 x i32>%index);
  ret <16 x float> %t0
}

define <16 x float> @gather.v16f32_ind_v16i64(float* %addr, <16 x i64>%index) {
  %ptr = bitcast float *%addr to i8*
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x float> @llvm.x86.avx512.mask.gather.qps.512(<8 x float> undef, i8* %ptr, <8 x i64> %index0, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 4)
  %t1 = call <8 x float> @llvm.x86.avx512.mask.gather.qps.512(<8 x float> undef, i8* %ptr, <8 x i64> %index1, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 4)
  %t2 = shufflevector <8 x float> %t0, <8 x float> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x float> %t2
}


define <16 x float> @masked_gather.v16f32 (<16 x i1> %mask, float* %addr, <16 x i32>%index) {
  %ptr = bitcast float *%addr to i8*
  %t0 = call <16 x float>  @llvm.x86.avx512.mask.gather.dps.512(<16 x float> zeroinitializer, i8* %ptr, <16 x i32> %index, <16 x i1> %mask, i32 4)
  ret <16 x float> %t0
}

define <16 x float> @masked_gather.v16f32_ind_v16i32(<16 x i1> %mask, float* %addr, <16 x i32>%index) {
  %res = call <16 x float> @masked_gather.v16f32(<16 x i1> %mask, float* %addr, <16 x i32>%index)
  ret <16 x float> %res
}

define <16 x float> @masked_gather.v16f32_ind_v16i64(<16 x i1> %mask, float* %addr, <16 x i64>%index) {
  %ptr = bitcast float *%addr to i8*
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>

  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x float> @llvm.x86.avx512.mask.gather.qps.512(<8 x float> undef, i8* %ptr, <8 x i64> %index0, <8 x i1> %mask0, i32 4)
  %t1 = call <8 x float> @llvm.x86.avx512.mask.gather.qps.512(<8 x float> undef, i8* %ptr, <8 x i64> %index1, <8 x i1> %mask1, i32 4)
  %t2 = shufflevector <8 x float> %t0, <8 x float> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x float> %t2
}


;; ------------------------------------
;;       Gather for double
;; ------------------------------------
define <16 x double> @gather.v16f64 (double* %addr, <16 x i32> %index) {
  %ptr = bitcast double *%addr to i8*
  %index0 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x double> @llvm.x86.avx512.mask.gather.dpd.512(<8 x double> undef, i8* %ptr, <8 x i32> %index0, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 8)
  %t1 = call <8 x double> @llvm.x86.avx512.mask.gather.dpd.512(<8 x double> undef, i8* %ptr, <8 x i32> %index1, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 8)
  %t2 = shufflevector <8 x double> %t0, <8 x double> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x double> %t2
}


define <16 x double> @gather.v16f64_ind_v16i64 (double* %addr, <16 x i64>%index) {
  %ptr = bitcast double *%addr to i8*
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x double> @llvm.x86.avx512.mask.gather.qpd.512(<8 x double> undef, i8* %ptr, <8 x i64> %index0, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 8)
  %t1 = call <8 x double> @llvm.x86.avx512.mask.gather.qpd.512(<8 x double> undef, i8* %ptr, <8 x i64> %index1, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 8)
  %t2 = shufflevector <8 x double> %t0, <8 x double> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x double> %t2
}

define <16 x double> @gather.v16f64_ind_v16i32 (double* %addr, <16 x i32>%index) {
  %t0 = call <16 x double> @gather.v16f64(double* %addr, <16 x i32>%index);
  ret <16 x double> %t0
}

define <8 x double> @gather.v8f64 (double* %addr, <8 x i32> %index) {
  %ptr = bitcast double *%addr to i8*
  %res = call <8 x double> @llvm.x86.avx512.mask.gather.dpd.512(<8 x double> undef, i8* %ptr, <8 x i32> %index, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 8)
  ret <8 x double> %res
}

define <16 x double> @masked_gather.v16f64 (<16 x i1> %mask, double* %addr, 
                                            <16 x i32> %index) {
  %ptr = bitcast double *%addr to i8*
  %index0 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>

  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x double> @llvm.x86.avx512.mask.gather.dpd.512(<8 x double> undef, i8* %ptr, <8 x i32> %index0, <8 x i1> %mask0, i32 8)
  %t1 = call <8 x double> @llvm.x86.avx512.mask.gather.dpd.512(<8 x double> undef, i8* %ptr, <8 x i32> %index1, <8 x i1> %mask1, i32 8)
  %t2 = shufflevector <8 x double> %t0, <8 x double> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x double> %t2
}

define <16 x double> @masked_gather.v16f64_ind_v16i64(<16 x i1> %mask, double* %addr, <16 x i64>%index) {
  %ptr = bitcast double *%addr to i8*
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>

  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x double> @llvm.x86.avx512.mask.gather.qpd.512(<8 x double> undef, i8* %ptr, <8 x i64> %index0, <8 x i1> %mask0, i32 8)
  %t1 = call <8 x double> @llvm.x86.avx512.mask.gather.qpd.512(<8 x double> undef, i8* %ptr, <8 x i64> %index1, <8 x i1> %mask1, i32 8)
  %t2 = shufflevector <8 x double> %t0, <8 x double> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x double> %t2
}

define <16 x double> @masked_gather.v16f64_ind_v16i32(<16 x i1> %mask, double* %addr, <16 x i32>%index) {
  %ptr = bitcast double *%addr to i8*
  %index0 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>

  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x double> @llvm.x86.avx512.mask.gather.dpd.512(<8 x double> undef, i8* %ptr, <8 x i32> %index0, <8 x i1> %mask0, i32 8)
  %t1 = call <8 x double> @llvm.x86.avx512.mask.gather.dpd.512(<8 x double> undef, i8* %ptr, <8 x i32> %index1, <8 x i1> %mask1, i32 8)
  %t2 = shufflevector <8 x double> %t0, <8 x double> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x double> %t2
}

define <8 x double> @masked_gather.v8f64 (<8 x i1> %mask, double* %addr,
                                           <8 x i32> %index) {
  %ptr = bitcast double *%addr to i8*
  %res = call <8 x double> @llvm.x86.avx512.mask.gather.dpd.512(<8 x double> undef, i8* %ptr, <8 x i32> %index, <8 x i1> %mask, i32 8)
  ret <8 x double> %res
}

define <8 x double> @masked_gather.v8f64_ind_v8i32(<8 x i1> %mask, double* %addr, <8 x i32> %index) {
  %t0 = call <8 x double> @masked_gather.v8f64(<8 x i1> %mask, double* %addr, <8 x i32> %index)
  ret <8 x double> %t0
}

;; ------------------------------------
;;       Gather for char
;; ------------------------------------
define <16 x i8> @masked_gather.v16i8 (<16 x i1> %mask, i8* %ptr, <16 x i32>%index) {
  %t0 = call <16 x i32>  @llvm.x86.avx512.mask.gather.dpi.512(<16 x i32> undef, i8* %ptr, <16 x i32> %index, <16 x i1> %mask, i32 1) ; scale 1
  %t1 = trunc <16 x i32> %t0 to <16 x i8>
  ret <16 x i8> %t1
}

;; ------------------------------------
;;       Gather for short
;; ------------------------------------
define <16 x i16> @masked_gather.v16i16 (<16 x i1> %mask, i16* %addr, <16 x i32>%index) {
  %ptr = bitcast i16 *%addr to i8*
  %t0 = call <16 x i32>  @llvm.x86.avx512.mask.gather.dpi.512(<16 x i32> undef, i8* %ptr, <16 x i32> %index, <16 x i1> %mask, i32 2) ; scale 2
  %t1 = trunc <16 x i32> %t0 to <16 x i16>
  ret <16 x i16> %t1
}
;; ------------------------------------
;;       Gather for int
;; ------------------------------------
define <16 x i32> @gather.v16i32 (i32* %addr, <16 x i32>%index) {
  %ptr = bitcast i32 *%addr to i8*
  %t0 = call <16 x i32> @llvm.x86.avx512.mask.gather.dpi.512(<16 x i32> undef, i8* %ptr, <16 x i32> %index, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 4) ; scale 4
  ret <16 x i32> %t0
}

define <16 x i32> @gather.v16i32_ind_v16i32 (i32* %addr, <16 x i32>%index) {
  %t0 = call <16 x i32> @gather.v16i32(i32* %addr, <16 x i32>%index);
  ret <16 x i32> %t0
}

define <16 x i32> @masked_gather.v16i32 (<16 x i1> %mask, i32* %addr, <16 x i32>%index) {
  %ptr = bitcast i32 *%addr to i8*
  %t0 = call <16 x i32>  @llvm.x86.avx512.mask.gather.dpi.512(<16 x i32> undef, i8* %ptr, <16 x i32> %index, <16 x i1> %mask, i32 4) ; scale 4
  ret <16 x i32> %t0
}

define <16 x i32> @masked_gather.v16i32_ind_v16i32 (<16 x i1> %mask, i32* %addr, <16 x i32>%index) {
  %t0 = call <16 x i32>  @masked_gather.v16i32(<16 x i1> %mask, i32* %addr, <16 x i32>%index)
  ret <16 x i32> %t0
}

define <16 x i32> @gather.v16i32_ind_v16i64(i32* %addr, <16 x i64>%index) {
  %ptr = bitcast i32 *%addr to i8*
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x i32> @llvm.x86.avx512.mask.gather.qpi.512(<8 x i32> undef, i8* %ptr, <8 x i64> %index0, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 4)
  %t1 = call <8 x i32> @llvm.x86.avx512.mask.gather.qpi.512(<8 x i32> undef, i8* %ptr, <8 x i64> %index1, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 4)
  %t2 = shufflevector <8 x i32> %t0, <8 x i32> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x i32> %t2
}

define <16 x i32> @masked_gather.v16i32_ind_v16i64(<16 x i1> %mask, i32* %addr, <16 x i64>%index) {
  %ptr = bitcast i32 *%addr to i8*
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>

  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x i32> @llvm.x86.avx512.mask.gather.qpi.512(<8 x i32> undef, i8* %ptr, <8 x i64> %index0, <8 x i1> %mask0, i32 4)
  %t1 = call <8 x i32> @llvm.x86.avx512.mask.gather.qpi.512(<8 x i32> undef, i8* %ptr, <8 x i64> %index1, <8 x i1> %mask1, i32 4)
  %t2 = shufflevector <8 x i32> %t0, <8 x i32> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x i32> %t2
}

define <16 x i64> @masked_gather.v16i64_ind_v16i32(<16 x i1> %mask, i64* %addr, <16 x i32>%index) {
  %ptr = bitcast i64 *%addr to i8*
  %index0 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>

  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x i64> @llvm.x86.avx512.mask.gather.dpq.512(<8 x i64> undef, i8* %ptr, <8 x i32> %index0, <8 x i1> %mask0, i32 8)
  %t1 = call <8 x i64> @llvm.x86.avx512.mask.gather.dpq.512(<8 x i64> undef, i8* %ptr, <8 x i32> %index1, <8 x i1> %mask1, i32 8)
  %t2 = shufflevector <8 x i64> %t0, <8 x i64> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x i64> %t2
}

;; ------------------------------------
;;       Gather for long
;; ------------------------------------
define <16 x i64> @gather.v16i64 (i64* %addr, <16 x i32> %index) {
  %ptr = bitcast i64 *%addr to i8*
  %index0 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x i64> @llvm.x86.avx512.mask.gather.dpq.512(<8 x i64>undef, i8* %ptr, <8 x i32> %index0, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 8) ; scale 8
  %t1 = call <8 x i64> @llvm.x86.avx512.mask.gather.dpq.512(<8 x i64>undef, i8* %ptr, <8 x i32> %index1, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 8) ; scale 8
  %t2 = shufflevector <8 x i64> %t0, <8 x i64> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x i64> %t2
}

define <16 x i64> @gather.v16i64_ind_v16i32 (i64* %addr, <16 x i32>%index) {
  %t0 = call <16 x i64> @gather.v16i64(i64* %addr, <16 x i32>%index);
  ret <16 x i64> %t0
}

define <16 x i64> @gather.v16i64_ind_v16i64 (i64* %addr, <16 x i64>%index) {
  %ptr = bitcast i64 *%addr to i8*
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x i64> @llvm.x86.avx512.mask.gather.qpq.512(<8 x i64> undef, i8* %ptr, <8 x i64> %index0, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 8)
  %t1 = call <8 x i64> @llvm.x86.avx512.mask.gather.qpq.512(<8 x i64> undef, i8* %ptr, <8 x i64> %index1, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, i32 8)
  %t2 = shufflevector <8 x i64> %t0, <8 x i64> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x i64> %t2
}

define <16 x i64> @masked_gather.v16i64 (<16 x i1> %mask, i64* %addr,
                                            <16 x i32> %index) {
  %ptr = bitcast i64 *%addr to i8*
  %index0 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>

  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x i64> @llvm.x86.avx512.mask.gather.dpq.512(<8 x i64> undef, i8* %ptr,
                                               <8 x i32> %index0, <8 x i1> %mask0,
                                               i32 8) ; scale 8
  %t1 = call <8 x i64> @llvm.x86.avx512.mask.gather.dpq.512(<8 x i64> undef, i8* %ptr,
                                               <8 x i32> %index1, <8 x i1> %mask1,
                                               i32 8) ; scale 8
  %t2 = shufflevector <8 x i64> %t0, <8 x i64> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x i64> %t2
}

define <16 x i64> @masked_gather.v16i64_ind_v16i64(<16 x i1> %mask, i64* %addr, <16 x i64>%index) {
  %ptr = bitcast i64 *%addr to i8*
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>

  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x i64> @llvm.x86.avx512.mask.gather.qpq.512(<8 x i64> undef, i8* %ptr, <8 x i64> %index0, <8 x i1> %mask0, i32 8)
  %t1 = call <8 x i64> @llvm.x86.avx512.mask.gather.qpq.512(<8 x i64> undef, i8* %ptr, <8 x i64> %index1, <8 x i1> %mask1, i32 8)
  %t2 = shufflevector <8 x i64> %t0, <8 x i64> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x i64> %t2
}


;; ------------------------------------
;;       Scatter for float
;; ------------------------------------
define void @scatter.v16f32 (float* %addr, <16 x i32>%index, <16 x float> %data) {
  %ptr = bitcast float *%addr to i8*
  call void @llvm.x86.avx512.mask.scatter.dps.512(i8* %ptr, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <16 x i32> %index,
                                           <16 x float> %data, i32 4) ; scale 4
  ret void
}

define void @scatter.v16f32_ind_v16i32 (float* %addr, <16 x i32>%index, <16 x float> %data) {
  call void @scatter.v16f32(float* %addr, <16 x i32>%index, <16 x float> %data)
  ret void
}

define void @scatter.v16f32_ind_v16i64 (float* %addr,
                                               <16 x i64> %index,
                                               <16 x float> %data) {
  %ptr = bitcast float *%addr to i8*
  %data0 = shufflevector <16 x float> %data, <16 x float> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %data1 = shufflevector <16 x float> %data, <16 x float> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.avx512.mask.scatter.qps.512(i8* %ptr, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <8 x i64> %index0,
                                    <8 x float> %data0,
                                    i32 4) ; scale 4
  call void @llvm.x86.avx512.mask.scatter.qps.512(i8* %ptr, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <8 x i64> %index1,
                                    <8 x float> %data1,
                                    i32 4) ; scale 4
  ret void
}

define void @scatter.v16i32_ind_v16i64 (i32* %addr,
                                               <16 x i64> %index,
                                               <16 x i32> %data) {
  %ptr = bitcast i32 *%addr to i8*
  %data0 = shufflevector <16 x i32> %data, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %data1 = shufflevector <16 x i32> %data, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.avx512.mask.scatter.qpi.512(i8* %ptr, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <8 x i64> %index0,
                                    <8 x i32> %data0,
                                    i32 4) ; scale 4
  call void @llvm.x86.avx512.mask.scatter.qpi.512(i8* %ptr, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <8 x i64> %index1,
                                    <8 x i32> %data1,
                                    i32 4) ; scale 4
  ret void
}


define void @masked_scatter.v16f32 (<16 x i1> %mask, float* %addr, <16 x i32>%index,
                                    <16 x float> %data) {
  %ptr = bitcast float *%addr to i8*
  call void @llvm.x86.avx512.mask.scatter.dps.512(i8* %ptr, <16 x i1> %mask,
                                          <16 x i32> %index,  <16 x float> %data,
                                          i32 4) ; scale 4
  ret void
}

define void @masked_scatter.v16f32_ind_v16i32 (<16 x i1> %mask, float* %addr,
                                               <16 x i32> %index,
                                               <16 x float> %data) {
  call void @masked_scatter.v16f32(<16 x i1> %mask, float* %addr,
                                   <16 x i32>%index, <16 x float> %data)
  ret void
}

define void @masked_scatter.v16f32_ind_v16i64 (<16 x i1> %mask, float* %addr,
                                               <16 x i64> %index,
                                               <16 x float> %data) {
  %ptr = bitcast float *%addr to i8*
  %data0 = shufflevector <16 x float> %data, <16 x float> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %data1 = shufflevector <16 x float> %data, <16 x float> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.avx512.mask.scatter.qps.512(i8* %ptr, <8 x i1> %mask0, <8 x i64> %index0,
                                    <8 x float> %data0,
                                    i32 4) ; scale 4
  call void @llvm.x86.avx512.mask.scatter.qps.512(i8* %ptr, <8 x i1> %mask1, <8 x i64> %index1,
                                    <8 x float> %data1,
                                    i32 4) ; scale 4
  ret void
}

;; ------------------------------------
;;       Scatter for double
;; ------------------------------------
define void @scatter.v16f64 (double* %addr, <16 x i32>%index, <16 x double> %data) {
  %ptr = bitcast double *%addr to i8*
  %data0 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index0 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %data1 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.avx512.mask.scatter.dpd.512(i8* %ptr, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <8 x i32> %index0,
                                    <8 x double> %data0,
                                    i32 8) ; scale 8
  call void @llvm.x86.avx512.mask.scatter.dpd.512(i8* %ptr, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <8 x i32> %index1,
                                    <8 x double> %data1,
                                    i32 8) ; scale 8
  ret void
}


define void @scatter.v16f64_ind_v16i64 (double* %addr, <16 x i64>%index, <16 x double> %data) {
  %ptr = bitcast double *%addr to i8*
  %data0 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %data1 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.avx512.mask.scatter.qpd.512(i8* %ptr, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <8 x i64> %index0,
                                    <8 x double> %data0,
                                    i32 8) ; scale 8
  call void @llvm.x86.avx512.mask.scatter.qpd.512(i8* %ptr, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <8 x i64> %index1,
                                    <8 x double> %data1,
                                    i32 8) ; scale 8
  ret void
}


define void @scatter.v16f64_ind_v16i32 (double* %addr, <16 x i32>%index, <16 x double> %data) {
  call void @scatter.v16f64 (double* %addr, <16 x i32>%index, <16 x double> %data)
  ret void
}

define void @scatter.v16i64_ind_v16i32 (i64* %addr, <16 x i32>%index, <16 x i64> %data) {
  call void @scatter.v16i64 (i64* %addr, <16 x i32>%index, <16 x  i64> %data)
  ret void
}

define void @scatter.v8f64 (double* %addr, <8 x i32>%index, <8 x double> %data) {
  %ptr = bitcast double *%addr to i8*
  call void @llvm.x86.avx512.mask.scatter.dpd.512(i8* %ptr, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <8 x i32> %index,
                                    <8 x double> %data,
                                    i32 8) ; scale 8
  ret void
}

define void @masked_scatter.v16f64 (<16 x i1> %mask, double* %addr,
                                    <16 x i32>%index, <16 x double> %data) {
  %ptr = bitcast double *%addr to i8*
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index0 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %data0 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %data1 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.avx512.mask.scatter.dpd.512(i8* %ptr, <8 x i1> %mask0,
                                          <8 x i32> %index0,
                                          <8 x double> %data0,
                                          i32 8) ; scale 8
  call void @llvm.x86.avx512.mask.scatter.dpd.512(i8* %ptr, <8 x i1> %mask1,
                                          <8 x i32> %index1,
                                          <8 x double> %data1,
                                          i32 8) ; scale 8
  ret void
}

define void @masked_scatter.v16f64_ind_v16i64 (<16 x i1> %mask, double* %addr,
                                               <16 x i64> %index,
                                               <16 x double> %data) {
  %ptr = bitcast double *%addr to i8*
  %data0 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %data1 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.avx512.mask.scatter.qpd.512(i8* %ptr, <8 x i1> %mask0, <8 x i64> %index0,
                                    <8 x double> %data0,
                                    i32 8) ; scale 4
  call void @llvm.x86.avx512.mask.scatter.qpd.512(i8* %ptr, <8 x i1> %mask1, <8 x i64> %index1,
                                    <8 x double> %data1,
                                    i32 8) ; scale 4
  ret void
}

define void @masked_scatter.v16f64_ind_v16i32 (<16 x i1> %mask, double* %addr,
                                               <16 x i32> %index,
                                               <16 x double> %data) {
  %ptr = bitcast double *%addr to i8*
  %data0 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index0 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %data1 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.avx512.mask.scatter.dpd.512(i8* %ptr, <8 x i1> %mask0, <8 x i32> %index0,
                                    <8 x double> %data0,
                                    i32 8) ; scale 4
  call void @llvm.x86.avx512.mask.scatter.dpd.512(i8* %ptr, <8 x i1> %mask1, <8 x i32> %index1,
                                    <8 x double> %data1,
                                    i32 8) ; scale 4
  ret void
}

define void @masked_scatter.v8f64 (<8 x i1> %mask, double* %addr,
                                   <8 x i32>%index, <8 x double> %data) {
  %ptr = bitcast double *%addr to i8*
  call void @llvm.x86.avx512.mask.scatter.dpd.512(i8* %ptr, <8 x i1> %mask,
                                          <8 x i32> %index,
                                          <8 x double> %data,
                                          i32 8) ; scale 8
  ret void
}


;; ------------------------------------
;;       Scatter for int
;; ------------------------------------
define void @scatter.v16i32 (i32* %addr, <16 x i32>%index, <16 x i32> %data) {
  %ptr = bitcast i32 *%addr to i8*
  call void @llvm.x86.avx512.mask.scatter.dpi.512(i8* %ptr, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
                                    <16 x i32> %index,
                                    <16 x i32> %data,
                                    i32 4) ; scale 4
  ret void
}

define void @scatter.v16i32_ind_v16i32 (i32* %addr, <16 x i32>%index, <16 x i32> %data) {
  call void @scatter.v16i32(i32* %addr, <16 x i32>%index, <16 x i32> %data)
  ret void
}


define void @masked_scatter.v16i32 (<16 x i1> %mask, i32* %addr, <16 x i32>%index,
                                <16 x i32> %data) {
  %ptr = bitcast i32 *%addr to i8*
  call void @llvm.x86.avx512.mask.scatter.dpi.512(i8* %ptr,
                                          <16 x i1> %mask,
                                          <16 x i32> %index,
                                          <16 x i32> %data,
                                          i32 4) ; scale 4
  ret void
}

define void @masked_scatter.v16i32_ind_v16i32 (<16 x i1> %mask, i32* %addr, <16 x i32>%index,
                                <16 x i32> %data) {
  call void @masked_scatter.v16i32(<16 x i1> %mask, i32* %addr, <16 x i32>%index,
                                  <16 x i32> %data)
  ret void
}

define void @masked_scatter.v16i32_ind_v16i64 (<16 x i1> %mask, i32* %addr,
                                               <16 x i64> %index,
                                               <16 x i32> %data) {
  %ptr = bitcast i32 *%addr to i8*
  %data0 = shufflevector <16 x i32> %data, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %data1 = shufflevector <16 x i32> %data, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.avx512.mask.scatter.qpi.512(i8* %ptr, <8 x i1> %mask0, <8 x i64> %index0,
                                    <8 x i32> %data0,
                                    i32 4) ; scale 4
  call void @llvm.x86.avx512.mask.scatter.qpi.512(i8* %ptr, <8 x i1> %mask1, <8 x i64> %index1,
                                    <8 x i32> %data1,
                                    i32 4) ; scale 4
  ret void
}

;; ------------------------------------
;;       Scatter for long
;; ------------------------------------
define void @scatter.v16i64 (i64* %addr, <16 x i32>%index, <16 x i64> %data) {
  %ptr = bitcast i64 *%addr to i8*
  %data0 = shufflevector <16 x i64> %data, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index0 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %data1 = shufflevector <16 x i64> %data, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.avx512.mask.scatter.dpq.512(i8* %ptr, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
                                    <8 x i32> %index0,
                                    <8 x i64> %data0,
                                    i32 8) ; scale 8
  call void @llvm.x86.avx512.mask.scatter.dpq.512(i8* %ptr, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
                                    <8 x i32> %index1,
                                    <8 x i64> %data1,
                                    i32 8) ; scale 8
  ret void
}

define void @scatter.v16i64_ind_v16i64 (i64* %addr, <16 x i64>%index, <16 x i64> %data) {
  %ptr = bitcast i64 *%addr to i8*
  %data0 = shufflevector <16 x i64> %data, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %data1 = shufflevector <16 x i64> %data, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.avx512.mask.scatter.qpq.512(i8* %ptr, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <8 x i64> %index0,
                                    <8 x i64> %data0,
                                    i32 8) ; scale 8
  call void @llvm.x86.avx512.mask.scatter.qpq.512(i8* %ptr, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <8 x i64> %index1,
                                    <8 x i64> %data1,
                                    i32 8) ; scale 8
  ret void
}

define void @masked_scatter.v16i64 (<16 x i1> %mask, i64* %addr,
                                    <16 x i32>%index, <16 x i64> %data) {
  %ptr = bitcast i64 *%addr to i8*
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index0 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %data0 = shufflevector <16 x i64> %data, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %data1 = shufflevector <16 x i64> %data, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.avx512.mask.scatter.dpq.512(i8* %ptr,
                                          <8 x i1> %mask0,
                                          <8 x i32> %index0,
                                          <8 x i64> %data0,
                                          i32 8) ; scale 8
  call void @llvm.x86.avx512.mask.scatter.dpq.512(i8* %ptr,
                                          <8 x i1> %mask1,
                                          <8 x i32> %index1,
                                          <8 x i64> %data1,
                                          i32 8) ; scale 8
  ret void
}

define void @masked_scatter.v16i64_ind_v16i64 (<16 x i1> %mask, i64* %addr,
                                               <16 x i64> %index,
                                               <16 x i64> %data) {
  %ptr = bitcast i64 *%addr to i8*
  %data0 = shufflevector <16 x i64> %data, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index0 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %data1 = shufflevector <16 x i64> %data, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %index1 = shufflevector <16 x i64> %index, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.avx512.mask.scatter.qpq.512(i8* %ptr, <8 x i1> %mask0, <8 x i64> %index0,
                                    <8 x i64> %data0,
                                    i32 8) ; scale 4
  call void @llvm.x86.avx512.mask.scatter.qpq.512(i8* %ptr, <8 x i1> %mask1, <8 x i64> %index1,
                                    <8 x i64> %data1,
                                    i32 8) ; scale 4
  ret void
}

define void @masked_scatter.v16i64_ind_v16i32 (<16 x i1> %mask, i64* %addr,
                                    <16 x i32>%index, <16 x i64> %data) {
  call void @masked_scatter.v16i64 (<16 x i1> %mask, i64* %addr,
                               <16 x i32>%index, <16 x i64> %data)
  ret void
}

;; ------------------------------------
;;       MASK Operations
;; ------------------------------------

define i1 @__ocl_allOne_v16(<16 x i1> %pred) nounwind readnone {
entry:
  %ipred = bitcast <16 x i1> %pred to i16
  %res = icmp eq i16 %ipred, -1
  ret i1 %res
}

define i1 @__ocl_allZero_v16(<16 x i1> %pred) nounwind readnone {
entry:
  %ipred = bitcast <16 x i1> %pred to i16
  %res = icmp eq i16 %ipred, 0
  ret i1 %res
}

define i1 @__ocl_allZero_v8(<8 x i1> %pred) nounwind readnone {
entry:
  %ipred = bitcast <8 x i1> %pred to i8
  %res = icmp eq i8 %ipred, 0
  ret i1 %res
}

define i1 @__ocl_allZero_v4(<4 x i1> %pred) nounwind readnone {
entry:
  %ipred = sext <4 x i1> %pred to <4 x i8>
  %bpred = bitcast <4 x i8> %ipred to i32
  %res = icmp eq i32 %bpred, 0
  ret i1 %res
}

define i1 @__ocl_allOne_v8(<8 x i1> %pred) nounwind readnone {
entry:
  %ipred = bitcast <8 x i1> %pred to i8
  %res = icmp eq i8 %ipred, -1
  ret i1 %res
}

define i1 @__ocl_allOne_v4(<4 x i1> %pred) nounwind readnone {
entry:
  %t1 = sext <4 x i1> %pred to <4 x i8>
  %t2 = bitcast <4 x i8> %t1 to i32
  %res = icmp eq i32 %t2, -1
  ret i1 %res
}

define i1 @__ocl_allOne(i1 %pred) nounwind readnone {
entry:
  ret i1 %pred
}

define i1 @__ocl_allZero(i1 %t) nounwind readnone {
entry:
  %pred = xor i1 %t, true
  ret i1 %pred
}

