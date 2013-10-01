;; ------------------------------------
;;       GATHER/SCATTER 
;; ------------------------------------
declare <16 x float> @llvm.x86.avx512.gather.dps.512 (<16 x i32>, i8*, i32);
declare void @llvm.x86.avx512.scatter.dps.512 (i8*, <16 x i32>, <16 x float>, i32)

declare <16 x float> @llvm.x86.avx512.gather.dps.mask.512 (<16 x float>, i16, <16 x i32>, i8*, i32)
declare void @llvm.x86.avx512.scatter.dps.mask.512 (i8*, i16, <16 x i32>, <16 x float>, i32)

declare <8 x double> @llvm.x86.avx512.gather.dpd.512 (<8 x i32>, i8*, i32);
declare void @llvm.x86.avx512.scatter.dpd.512 (i8*, <8 x i32>, <8 x double>, i32)

declare <8 x double> @llvm.x86.avx512.gather.dpd.mask.512 (<8 x double>, i8, <8 x i32>, i8*, i32)
declare void @llvm.x86.avx512.scatter.dpd.mask.512 (i8*, i8, <8 x i32>, <8 x double>, i32)

declare <16 x i32> @llvm.x86.avx512.gather.dpi.mask.512 (<16 x i32>, i16, <16 x i32>, i8*, i32)
declare void @llvm.x86.avx512.scatter.dpi.mask.512 (i8*, i16, <16 x i32>, <16 x i32>, i32)
declare <16 x i32> @llvm.x86.avx512.gather.dpi.512 (<16 x i32>, i8*, i32)
declare void @llvm.x86.avx512.scatter.dpi.512 (i8*, <16 x i32>, <16 x i32>, i32)

declare <8 x i64> @llvm.x86.avx512.gather.dpq.mask.512 (<8 x i64>, i8, <8 x i32>, i8*, i32)
declare void @llvm.x86.avx512.scatter.dpq.mask.512 (i8*, i8, <8 x i32>, <8 x i64>, i32)
declare <8 x i64> @llvm.x86.avx512.gather.dpq.512 (<8 x i64>, <8 x i32>, i8*, i32)
declare void @llvm.x86.avx512.scatter.dpq.512 (i8*, <8 x i32>, <8 x i64>, i32)

declare <8 x i32> @llvm.x86.avx512.gather.qpi.mask.512 (<8 x i32>, i8, <8 x i64>, i8*, i32)
declare void @llvm.x86.avx512.scatter.qpi.mask.512 (i8*, i8, <8 x i64>, <8 x i32>, i32)
declare <8 x i64> @llvm.x86.avx512.gather.qpq.mask.512 (<8 x i64>, i8, <8 x i64>, i8*, i32)
declare void @llvm.x86.avx512.scatter.qpq.mask.512 (i8*, i8, <8 x i64>, <8 x i64>, i32)

define <16 x float> @gather.v16f32 (float* %addr, <16 x i32>%index) {
  %ptr = bitcast float *%addr to i8*
  %t0 = call <16 x float> @llvm.x86.avx512.gather.dps.512(<16 x i32> %index, i8* %ptr,
                                               i32 4); scale 4
  ret <16 x float> %t0
}

define <16 x float> @masked_gather.v16f32 (<16 x i1> %mask, float* %addr, <16 x i32>%index) {
  %ptr = bitcast float *%addr to i8*
  %imask = bitcast <16 x i1> %mask to i16
  %t0 = call <16 x float>  @llvm.x86.avx512.gather.dps.mask.512(<16 x float> undef,
                                               i16 %imask,
                                               <16 x i32> %index, i8* %ptr,
                                               i32 4) ; scale 4
  ret <16 x float> %t0
}

define <16 x i32> @masked_gather.v16i32 (<16 x i1> %mask, i32* %addr, <16 x i32>%index) {
  %ptr = bitcast i32 *%addr to i8*
  %imask = bitcast <16 x i1> %mask to i16
  %t0 = call <16 x i32>  @llvm.x86.avx512.gather.dpi.mask.512(<16 x i32> undef,
                                               i16 %imask,
                                               <16 x i32> %index, i8* %ptr,
                                               i32 4) ; scale 4
  ret <16 x i32> %t0
}


define void @scatter.v16f32 (float* %addr, <16 x i32>%index, <16 x float> %data) {
  %ptr = bitcast float *%addr to i8*
  call void @llvm.x86.avx512.scatter.dps.512(i8* %ptr, <16 x i32> %index,
                                           <16 x float> %data, i32 4) ; scale 4
  ret void
}

define void @masked_scatter.v16f32 (<16 x i1> %mask, float* %addr, <16 x i32>%index,
                                    <16 x float> %data) {
  %ptr = bitcast float *%addr to i8*
  %imask = bitcast <16 x i1> %mask to i16
  call void @llvm.x86.avx512.scatter.dps.mask.512(i8* %ptr,
                                          i16 %imask,
                                          <16 x i32> %index,
                                          <16 x float> %data,
                                          i32 4) ; scale 4
  ret void
}

define void @masked_scatter.v16i32 (<16 x i1> %mask, i32* %addr, <16 x i32>%index,
                                <16 x i32> %data) {
  %ptr = bitcast i32 *%addr to i8*
  %imask = bitcast <16 x i1> %mask to i16
  call void @llvm.x86.avx512.scatter.dpi.mask.512(i8* %ptr,
                                          i16 %imask,
                                          <16 x i32> %index,
                                          <16 x i32> %data,
                                          i32 4) ; scale 4
  ret void
}

define <16 x double> @gather.v16f64 (double* %addr, <16 x i32> %index) {
  %ptr = bitcast double *%addr to i8*
  %index0 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x double> @llvm.x86.avx512.gather.dpd.512(<8 x i32> %index0, i8* %ptr,
                                               i32 8) ; scale 8
  %t1 = call <8 x double> @llvm.x86.avx512.gather.dpd.512(<8 x i32> %index1, i8* %ptr,
                                               i32 8) ; scale 8
  %t2 = shufflevector <8 x double> %t0, <8 x double> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x double> %t2
}

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
  call void @llvm.x86.avx512.scatter.dpd.512(i8* %ptr,
                                    <8 x i32> %index0,
                                    <8 x double> %data0,
                                    i32 8) ; scale 8
  call void @llvm.x86.avx512.scatter.dpd.512(i8* %ptr,
                                    <8 x i32> %index1,
                                    <8 x double> %data1,
                                    i32 8) ; scale 8
  ret void
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
  %imask0 = bitcast <8 x i1> %mask0 to i8

  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %imask1 = bitcast <8 x i1> %mask1 to i8
  %t0 = call <8 x double> @llvm.x86.avx512.gather.dpd.mask.512(<8 x double> undef,
                                               i8 %imask0,
                                               <8 x i32> %index0, i8* %ptr,
                                               i32 8) ; scale 8
  %t1 = call <8 x double> @llvm.x86.avx512.gather.dpd.mask.512(<8 x double> undef,
                                               i8 %imask1,
                                               <8 x i32> %index1, i8* %ptr,
                                               i32 8) ; scale 8
  %t2 = shufflevector <8 x double> %t0, <8 x double> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x double> %t2
}

define void @masked_scatter.v16f64 (<16 x i1> %mask, double* %addr, 
                                    <16 x i32>%index, <16 x double> %data) {
  %ptr = bitcast double *%addr to i8*
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %imask0 = bitcast <8 x i1> %mask0 to i8
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
  %imask1 = bitcast <8 x i1> %mask1 to i8
  call void @llvm.x86.avx512.scatter.dpd.mask.512(i8* %ptr,
                                          i8 %imask0,
                                          <8 x i32> %index0,
                                          <8 x double> %data0,
                                          i32 8) ; scale 8
  call void @llvm.x86.avx512.scatter.dpd.mask.512(i8* %ptr,
                                          i8 %imask1,
                                          <8 x i32> %index1,
                                          <8 x double> %data1,
                                          i32 8) ; scale 8
  ret void
}

define <16 x i32> @gather.v16i32 (i32* %addr, <16 x i32>%index) {
  %ptr = bitcast i32 *%addr to i8*
  %t0 = call <16 x i32> @llvm.x86.avx512.gather.dpi.512(<16 x i32> %index, 
                                                      i8* %ptr,
                                                      i32 4) ; scale 4
  ret <16 x i32> %t0
}
define void @scatter.v16i32 (i32* %addr, <16 x i32>%index, <16 x i32> %data) {
  %ptr = bitcast i32 *%addr to i8*
  call void @llvm.x86.avx512.scatter.dpi.512(i8* %ptr,
                                    <16 x i32> %index,
                                    <16 x i32> %data,
                                    i32 4) ; scale 4
  ret void
}


;; ------------------------------------
;;       MASK Operations 
;; ------------------------------------
declare i32 @llvm.x86.avx512.kortestc(i16, i16) nounwind readnone
declare i32 @llvm.x86.avx512.kortestz(i16, i16) nounwind readnone

define i1 @__ocl_allOne_v16(<16 x i1> %pred) {
entry:
  %ipred = bitcast <16 x i1> %pred to i16
  %val = call i32 @llvm.x86.avx512.kortestc(i16 %ipred, i16 %ipred)
  %res = trunc i32 %val to i1
  ret i1 %res
}

define i1 @__ocl_allZero_v16(<16 x i1> %t) {
entry:
  %ipred = bitcast <16 x i1> %t to i16
  %val = call i32 @llvm.x86.avx512.kortestz(i16 %ipred, i16 %ipred)
  %res = trunc i32 %val to i1
  ret i1 %res
}
