; ModuleID = '/home/zrackove/my_MIC/src/backend/libraries/ocl_builtins/clbltfnb1_vectorizer.bc'
target datalayout = "e-p:64:64"

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

declare i16 @llvm.x86.mic.knot(i16) nounwind readnone

define i1 @__ocl_allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @__ocl_allOne_v2(<2 x i1> %pred) {
entry:
  %0 = shufflevector <2 x i1> %pred, <2 x i1> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %1 = shufflevector <4 x i1> %0, <4 x i1> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %2 = shufflevector <8 x i1> %1, <8 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %ipred = bitcast <16 x i1> %2 to i16
  %lpred = zext i16 %ipred to i32
  %mask = and i32 %lpred, 3
  %res = icmp eq i32 %mask, 3
  ret i1 %res
}

define i1 @__ocl_allOne_v4(<4 x i1> %pred) {
entry:
  %0 = shufflevector <4 x i1> %pred, <4 x i1> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %1 = shufflevector <8 x i1> %0, <8 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %ipred = bitcast <16 x i1> %1 to i16
  %lpred = zext i16 %ipred to i32
  %mask = and i32 %lpred, 15
  %res = icmp eq i32 %mask, 15
  ret i1 %res
}

define i1 @__ocl_allOne_v8(<8 x i1> %pred) {
entry:
  %0 = shufflevector <8 x i1> %pred, <8 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %ipred = bitcast <16 x i1> %0 to i16
  %lpred = zext i16 %ipred to i32
  %mask = and i32 %lpred, 255
  %res = icmp eq i32 %mask, 255
  ret i1 %res
}

define i1 @__ocl_allOne_v16(<16 x i1> %pred) {
entry:
  %ipred = bitcast <16 x i1> %pred to i16
  %val = call i32 @llvm.x86.mic.kortestc(i16 %ipred, i16 %ipred)
  %res = trunc i32 %val to i1
  ret i1 %res
}

define i1 @__ocl_allZero(i1 %t) {
entry:
  %pred = xor i1 %t, true
  ret i1 %pred
}

define i1 @__ocl_allZero_v2(<2 x i1> %t) {
entry:
  %0 = shufflevector <2 x i1> %t, <2 x i1> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %1 = shufflevector <4 x i1> %0, <4 x i1> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %2 = shufflevector <8 x i1> %1, <8 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %ipred = bitcast <16 x i1> %2 to i16
  %lpred = zext i16 %ipred to i32
  %mask = and i32 %lpred, 3
  %res = icmp eq i32 %mask, 0
  ret i1 %res
}

define i1 @__ocl_allZero_v4(<4 x i1> %t) {
entry:
  %0 = shufflevector <4 x i1> %t, <4 x i1> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %1 = shufflevector <8 x i1> %0, <8 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %ipred = bitcast <16 x i1> %1 to i16
  %lpred = zext i16 %ipred to i32
  %mask = and i32 %lpred, 15
  %res = icmp eq i32 %mask, 0
  ret i1 %res
}

define i1 @__ocl_allZero_v8(<8 x i1> %t) {
entry:
  %0 = shufflevector <8 x i1> %t, <8 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %ipred = bitcast <16 x i1> %0 to i16
  %lpred = zext i16 %ipred to i32
  %mask = and i32 %lpred, 255
  %res = icmp eq i32 %mask, 0
  ret i1 %res
}

define i1 @__ocl_allZero_v16(<16 x i1> %t) {
entry:
  %ipred = bitcast <16 x i1> %t to i16
  %val = call i32 @llvm.x86.mic.kortestz(i16 %ipred, i16 %ipred)
  %res = trunc i32 %val to i1
  ret i1 %res
}



declare <16 x i32> @llvm.x86.mic.gather.pi(<16 x i32>, i8*, i32, i32, i32)
declare <16 x i32> @llvm.x86.mic.mask.gather.pi(<16 x i32>, i16,
                                            <16 x i32>, i8*, i32, i32, i32)
declare <8 x i64> @llvm.x86.mic.gather.pq(<16 x i32>, i8*, i32, i32, i32)
declare <8 x i64> @llvm.x86.mic.mask.gather.pq(<8 x i64>, i8,
                                          <16 x i32>, i8*, i32, i32, i32)
declare <16 x float> @llvm.x86.mic.gather.ps(<16 x i32>, i8*, i32, i32, i32)
declare <16 x float> @llvm.x86.mic.mask.gather.ps(<16 x float>, i16,
                                            <16 x i32>, i8*, i32, i32, i32)
declare <8 x double> @llvm.x86.mic.gather.pd(<16 x i32>, i8*, i32, i32, i32)
declare <8 x double> @llvm.x86.mic.mask.gather.pd(<8 x double>, i8,
                                            <16 x i32>, i8*, i32, i32, i32)
declare void @llvm.x86.mic.scatter.pi(i8*, <16 x i32>, <16 x i32>,
                                            i32, i32, i32)
declare void @llvm.x86.mic.mask.scatter.pi(i8*, i16, <16 x i32>,
                                            <16 x i32>, i32, i32, i32)
declare void @llvm.x86.mic.scatter.pq(i8*, <16 x i32>, <8 x i64>,
                                            i32, i32, i32)
declare void @llvm.x86.mic.mask.scatter.pq(i8*, i8, <16 x i32>,
                                            <8 x i64>, i32, i32, i32)
declare void @llvm.x86.mic.scatter.ps(i8*, <16 x i32>, <16 x float>,
                                            i32, i32, i32)
declare void @llvm.x86.mic.mask.scatter.ps(i8*, i16, <16 x i32>,
                                            <16 x float>, i32, i32, i32)
declare void @llvm.x86.mic.scatter.pd(i8*, <16 x i32>, <8 x double>,
                                            i32, i32, i32)
declare void @llvm.x86.mic.mask.scatter.pd(i8*, i8, <16 x i32>,
                                            <8 x double>, i32, i32, i32)

declare void @llvm.x86.mic.gatherpf.ps(<16 x i32>, i8*,
                                            i32, i32, i32)
declare void @llvm.x86.mic.mask.gatherpf.ps(<16 x i32>, i16, 
                                            i8*, i32, i32, i32)

declare void @llvm.x86.mic.scatterpf.ps(i8*, <16 x i32>, 
                                            i32, i32, i32)
declare void @llvm.x86.mic.mask.scatterpf.ps(i8*, i16, <16 x i32>, 
                                            i32, i32, i32)


define void @gatherpf.32(i8* %addr, <16 x i32> %index) {
  call void @llvm.x86.mic.gatherpf.ps(<16 x i32> %index, i8* %addr,
                                               i32 0, ; no up conversion
                                               i32 4, ; scale 4
                                               i32 2) ; prefetch to L2 cache, non exclusive
  ret void
}

define void @masked_gatherpf.32(<16 x i1> %mask, i8* %addr, <16 x i32>%index) {
  %imask = bitcast <16 x i1> %mask to i16

  call void @llvm.x86.mic.mask.gatherpf.ps(<16 x i32> %index, i16 %imask, i8* %addr,
                                               i32 0, ; no up conversion
                                               i32 4, ; scale 4
                                               i32 2) ; prefetch to L2 cache, non exclusive
  ret void
}

define void @gatherpf.64(i8* %addr, <16 x i32> %index) {
  call void @llvm.x86.mic.gatherpf.ps(<16 x i32> %index, i8* %addr,
                                               i32 0, ; no up conversion
                                               i32 8, ; scale 8
                                               i32 2) ; prefetch to L2 cache, non exclusive
  ret void
}

define void @masked_gatherpf.64(<16 x i1> %mask, i8* %addr, <16 x i32>%index) {
  %imask = bitcast <16 x i1> %mask to i16

  call void @llvm.x86.mic.mask.gatherpf.ps(<16 x i32> %index, i16 %imask, i8* %addr,
                                               i32 0, ; no up conversion
                                               i32 8, ; scale 8
                                               i32 2) ; prefetch to L2 cache, non exclusive
  ret void
}

define void @gatherpf.v16f32(float* %addr, <16 x i32>%index) {
  %ptr = bitcast float *%addr to i8*
  call void @gatherpf.32(i8* %ptr, <16 x i32> %index)
  ret void
}

define void @gatherpf.v16i8(i8* %ptr, <16 x i32>%index) {
  call void @llvm.x86.mic.gatherpf.ps(<16 x i32> %index, i8* %ptr,
                                               i32 1, ; u32 -> u8
                                               i32 1, ; scale 1
                                               i32 2) ; prefetch to L2 cache, non exclusive
  ret void
}

define void @gatherpf.v16i16(i16* %addr, <16 x i32>%index) {
  %ptr = bitcast i16 *%addr to i8*
  call void @llvm.x86.mic.gatherpf.ps(<16 x i32> %index, i8* %ptr,
                                               i32 3, ; u32 -> u16
                                               i32 2, ; scale 2
                                               i32 2) ; prefetch to L2 cache, non exclusive
  ret void
}

define void @gatherpf.v16i32(i32* %addr, <16 x i32>%index) {
  %ptr = bitcast i32 *%addr to i8*
  call void @gatherpf.32(i8* %ptr, <16 x i32> %index)
  ret void
}

define void @gatherpf.v16f64(double* %addr, <16 x i32>%index) {
  %ptr = bitcast double *%addr to i8*
  call void @gatherpf.64(i8* %ptr, <16 x i32> %index)
  ret void
}

define void @gatherpf.v16i64(i64* %addr, <16 x i32>%index) {
  %ptr = bitcast i64 *%addr to i8*
  call void @gatherpf.64(i8* %ptr, <16 x i32> %index)
  ret void
}

define void @masked_gatherpf.v16f32(<16 x i1> %mask, float* %addr, <16 x i32>%index) {
  %ptr = bitcast float *%addr to i8*
  call void @masked_gatherpf.32(<16 x i1> %mask, i8* %ptr, <16 x i32> %index)
  ret void
}

define void @masked_gatherpf.v16i8(<16 x i1> %mask, i8* %ptr, <16 x i32>%index) {
  %imask = bitcast <16 x i1> %mask to i16
  call void @llvm.x86.mic.mask.gatherpf.ps(<16 x i32> %index, i16 %imask, i8* %ptr,
                                               i32 1, ; u32 -> u8
                                               i32 1, ; scale 1
                                               i32 2) ; prefetch to L2 cache, non exclusive
  ret void
}

define void @masked_gatherpf.v16i16(<16 x i1> %mask, i16* %addr, <16 x i32>%index) {
  %ptr = bitcast i16 *%addr to i8*
  %imask = bitcast <16 x i1> %mask to i16
  call void @llvm.x86.mic.mask.gatherpf.ps(<16 x i32> %index, i16 %imask, i8* %ptr,
                                               i32 3, ; u32 -> u16
                                               i32 2, ; scale 2
                                               i32 2) ; prefetch to L2 cache, non exclusive
  ret void
}

define void @masked_gatherpf.v16i32(<16 x i1> %mask, i32* %addr, <16 x i32>%index) {
  %ptr = bitcast i32 *%addr to i8*
  call void @masked_gatherpf.32(<16 x i1> %mask, i8* %ptr, <16 x i32> %index)
  ret void
}

define void @masked_gatherpf.v16f64(<16 x i1> %mask, double* %addr, <16 x i32>%index) {
  %ptr = bitcast double *%addr to i8*
  call void @masked_gatherpf.64(<16 x i1> %mask, i8* %ptr, <16 x i32> %index)
  ret void
}

define void @masked_gatherpf.v16i64(<16 x i1> %mask, i64* %addr, <16 x i32>%index) {
  %ptr = bitcast i64 *%addr to i8*
  call void @masked_gatherpf.64(<16 x i1> %mask, i8* %ptr, <16 x i32> %index)
  ret void
}

define void @scatterpf.32(i8* %addr, <16 x i32>%index) {
 call void @llvm.x86.mic.scatterpf.ps(i8* %addr, <16 x i32> %index,
                                               i32 0, ; no up conversion
                                               i32 4, ; scale 4
                                               i32 6) ; prefetch to L2 cache, exclusive
 ret void
}

define void @masked_scatterpf.32(<16 x i1> %mask, i8* %addr, <16 x i32>%index) {
 %imask = bitcast <16 x i1> %mask to i16
 call void @llvm.x86.mic.mask.scatterpf.ps(i8* %addr, i16 %imask, <16 x i32> %index,
                                               i32 0, ; no up conversion
                                               i32 4, ; scale 4
                                               i32 6) ; prefetch to L2 cache, exclusive
 ret void
}

define void @scatterpf.64(i8* %addr, <16 x i32>%index) {
 call void @llvm.x86.mic.scatterpf.ps(i8* %addr, <16 x i32> %index,
                                               i32 0, ; no up conversion
                                               i32 8, ; scale 8
                                               i32 6) ; prefetch to L2 cache, exclusive
 ret void
}

define void @masked_scatterpf.64(<16 x i1> %mask, i8* %addr, <16 x i32>%index) {
 %imask = bitcast <16 x i1> %mask to i16
 call void @llvm.x86.mic.mask.scatterpf.ps(i8* %addr, i16 %imask, <16 x i32> %index,
                                               i32 0, ; no up conversion
                                               i32 8, ; scale 8
                                               i32 6) ; prefetch to L2 cache, exclusive
 ret void
}

define void @scatterpf.v16i8 (i8* %ptr, <16 x i32>%index) {
 call void @llvm.x86.mic.scatterpf.ps(i8* %ptr, <16 x i32> %index,
                                               i32 1, ; u8 - > u32
                                               i32 1, ; scale 1
                                               i32 6) ; prefetch to L2 cache, exclusive
 ret void
}

define void @scatterpf.v16i16 (i16* %addr, <16 x i32>%index) {
 %ptr = bitcast i16 *%addr to i8*
 call void @llvm.x86.mic.scatterpf.ps(i8* %ptr, <16 x i32> %index,
                                               i32 3, ; u16 - > u32
                                               i32 2, ; scale 2
                                               i32 6) ; prefetch to L2 cache, exclusive
 ret void
}

define void @scatterpf.v16i32 (i32* %addr, <16 x i32>%index) {
 %ptr = bitcast i32 *%addr to i8*
 call void @scatterpf.32(i8* %ptr, <16 x i32> %index)
 ret void
}

define void @scatterpf.v16f32 (float* %addr, <16 x i32>%index) {
 %ptr = bitcast float *%addr to i8*
 call void @scatterpf.32(i8* %ptr, <16 x i32> %index)
 ret void
}

define void @scatterpf.v16i64 (i64* %addr, <16 x i32>%index) {
 %ptr = bitcast i64 *%addr to i8*
 call void @scatterpf.64(i8* %ptr, <16 x i32> %index)
 ret void
}

define void @scatterpf.v16f64 (double* %addr, <16 x i32>%index) {
 %ptr = bitcast double *%addr to i8*
 call void @scatterpf.64(i8* %ptr, <16 x i32> %index)
 ret void
}

define void @masked_scatterpf.v16i8 (<16 x i1> %mask, i8* %ptr, <16 x i32>%index) {
 %imask = bitcast <16 x i1> %mask to i16
 call void @llvm.x86.mic.mask.scatterpf.ps(i8* %ptr, i16 %imask, <16 x i32> %index,
                                               i32 1, ; u8 -> u32
                                               i32 1, ; scale 2
                                               i32 6) ; prefetch to L2 cache, exclusive
 ret void
}

define void @masked_scatterpf.v16i16 (<16 x i1> %mask, i16* %addr, <16 x i32>%index) {
 %ptr = bitcast i16 *%addr to i8*
 %imask = bitcast <16 x i1> %mask to i16
 call void @llvm.x86.mic.mask.scatterpf.ps(i8* %ptr, i16 %imask, <16 x i32> %index,
                                               i32 3, ; u16 -> u32
                                               i32 2, ; scale 2
                                               i32 6) ; prefetch to L2 cache, exclusive
 ret void
}

define void @masked_scatterpf.v16i32 (<16 x i1> %mask, i32* %addr, <16 x i32>%index) {
 %ptr = bitcast i32 *%addr to i8*
 call void @masked_scatterpf.32(<16 x i1> %mask, i8* %ptr, <16 x i32> %index)
 ret void
}

define void @masked_scatterpf.v16f32 (<16 x i1> %mask, float* %addr, <16 x i32>%index) {
 %ptr = bitcast float *%addr to i8*
 call void @masked_scatterpf.32(<16 x i1> %mask, i8* %ptr, <16 x i32> %index)
 ret void
}

define void @masked_scatterpf.v16i64 (<16 x i1> %mask, i64* %addr, <16 x i32>%index) {
 %ptr = bitcast i64 *%addr to i8*
 call void @masked_scatterpf.64(<16 x i1> %mask, i8* %ptr, <16 x i32> %index)
 ret void
}

define void @masked_scatterpf.v16f64 (<16 x i1> %mask, double* %addr, <16 x i32>%index) {
 %ptr = bitcast double *%addr to i8*
 call void @masked_scatterpf.64(<16 x i1> %mask, i8* %ptr, <16 x i32> %index)
 ret void
}

define <16 x i8> @gather.v16i8 (i8* %ptr, <16 x i32>%index) {
  %t0 = call <16 x i32> @llvm.x86.mic.gather.pi(<16 x i32> %index, i8* %ptr,
                                               i32 1, ; u8 -> u32 (zext)
                                               i32 1, ; scale 1
                                               i32 0) ; gather to L1 cache
  %t1 = trunc <16 x i32> %t0 to <16 x i8>
  ret <16 x i8> %t1
}

define <16 x i8> @masked_gather.v16i8 (<16 x i1> %mask, i8* %ptr, <16 x i32>%index) {
  %imask = bitcast <16 x i1> %mask to i16
  %t0 = call <16 x i32>  @llvm.x86.mic.mask.gather.pi(<16 x i32> undef,
                                               i16 %imask,
                                               <16 x i32> %index, i8* %ptr,
                                               i32 1, ; u8 -> u32
                                               i32 1, ; scale 1
                                               i32 0) ; gather to L1 cache
  %t1 = trunc <16 x i32> %t0 to <16 x i8>
  ret <16 x i8> %t1
}

define <16 x i16> @gather.v16i16 (i16* %addr, <16 x i32>%index) {
  %ptr = bitcast i16 *%addr to i8*
  %t0 = call <16 x i32> @llvm.x86.mic.gather.pi(<16 x i32> %index, i8* %ptr,
                                               i32 3, ; u16 -> u32
                                               i32 2, ; scale 2
                                               i32 0) ; gather to L1 cache
  %t1 = trunc <16 x i32> %t0 to <16 x i16>
  ret <16 x i16> %t1
}

define <16 x i16> @masked_gather.v16i16 (<16 x i1> %mask, i16* %addr, <16 x i32>%index) {
  %ptr = bitcast i16 *%addr to i8*
  %imask = bitcast <16 x i1> %mask to i16
  %t0 = call <16 x i32>  @llvm.x86.mic.mask.gather.pi(<16 x i32> undef,
                                               i16 %imask,
                                               <16 x i32> %index, i8* %ptr,
                                               i32 3, ; u16 -> u32
                                               i32 2, ; scale 2
                                               i32 0) ; gather to L1 cache
  %t1 = trunc <16 x i32> %t0 to <16 x i16>
  ret <16 x i16> %t1
}

define <16 x i32> @gather.v16i32 (i32* %addr, <16 x i32>%index) {
  %ptr = bitcast i32 *%addr to i8*
  %t0 = call <16 x i32> @llvm.x86.mic.gather.pi(<16 x i32> %index, i8* %ptr,
                                               i32 0, ; no up conversion
                                               i32 4, ; scale 4
                                               i32 0) ; gather to L1 cache
  ret <16 x i32> %t0
}

define <16 x i32> @masked_gather.v16i32 (<16 x i1> %mask, i32* %addr, <16 x i32>%index) {
  %ptr = bitcast i32 *%addr to i8*
  %imask = bitcast <16 x i1> %mask to i16
  %t0 = call <16 x i32>  @llvm.x86.mic.mask.gather.pi(<16 x i32> undef,
                                               i16 %imask,
                                               <16 x i32> %index, i8* %ptr,
                                               i32 0, ; no up conversion
                                               i32 4, ; scale 4
                                               i32 0) ; gather to L1 cache
  ret <16 x i32> %t0
}

define <16 x i64> @gather.v16i64 (i64* %addr, <16 x i32>%index) {
  %ptr = bitcast i64 *%addr to i8*
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x i64> @llvm.x86.mic.gather.pq(<16 x i32> %index, i8* %ptr,
                                               i32 0, ; no up conversion
                                               i32 8, ; scale 8
                                               i32 0) ; gather to L1 cache
  %t1 = call <8 x i64> @llvm.x86.mic.gather.pq(<16 x i32> %index1, i8* %ptr,
                                               i32 0, ; no up conversion
                                               i32 8, ; scale 8
                                               i32 0) ; gather to L1 cache
  %t2 = shufflevector <8 x i64> %t0, <8 x i64> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x i64> %t2
}

define <16 x i64> @masked_gather.v16i64 (<16 x i1> %mask, i64* %addr, <16 x i32>%index) {
  %ptr = bitcast i64 *%addr to i8*
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %imask0 = bitcast <8 x i1> %mask0 to i8
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15,
            i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %imask1 = bitcast <8 x i1> %mask1 to i8
  %t0 = call <8 x i64> @llvm.x86.mic.mask.gather.pq(<8 x i64> undef,
                                               i8 %imask0,
                                               <16 x i32> %index, i8* %ptr,
                                               i32 0, ; no up conversion
                                               i32 8, ; scale 8
                                               i32 0) ; gather to L1 cache
  %t1 = call <8 x i64> @llvm.x86.mic.mask.gather.pq(<8 x i64> undef,
                                               i8 %imask1,
                                               <16 x i32> %index1, i8* %ptr,
                                               i32 0, ; no up conversion
                                               i32 8, ; scale 8
                                               i32 0) ; gather to L1 cache
  %t2 = shufflevector <8 x i64> %t0, <8 x i64> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x i64> %t2
}

define <16 x float> @gather.v16f32 (float* %addr, <16 x i32>%index) {
  %ptr = bitcast float *%addr to i8*
  %t0 = call <16 x float> @llvm.x86.mic.gather.ps(<16 x i32> %index, i8* %ptr,
                                               i32 0, ; no up conversion
                                               i32 4, ; scale 4
                                               i32 0) ; gather to L1 cache
  ret <16 x float> %t0
}

define <16 x float> @masked_gather.v16f32 (<16 x i1> %mask, float* %addr, <16 x i32>%index) {
  %ptr = bitcast float *%addr to i8*
  %imask = bitcast <16 x i1> %mask to i16
  %t0 = call <16 x float>  @llvm.x86.mic.mask.gather.ps(<16 x float> undef,
                                               i16 %imask,
                                               <16 x i32> %index, i8* %ptr,
                                               i32 0, ; no up conversion
                                               i32 4, ; scale 4
                                               i32 0) ; gather to L1 cache
  ret <16 x float> %t0
}

define <16 x double> @gather.v16f64 (double* %addr, <16 x i32> %index) {
  %ptr = bitcast double *%addr to i8*
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %t0 = call <8 x double> @llvm.x86.mic.gather.pd(<16 x i32> %index, i8* %ptr,
                                               i32 0, ; no up conversion
                                               i32 8, ; scale 8
                                               i32 0) ; gather to L1 cache
  %t1 = call <8 x double> @llvm.x86.mic.gather.pd(<16 x i32> %index1, i8* %ptr,
                                               i32 0, ; no up conversion
                                               i32 8, ; scale 8
                                               i32 0) ; gather to L1 cache
  %t2 = shufflevector <8 x double> %t0, <8 x double> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x double> %t2
}

define <16 x double> @masked_gather.v16f64 (<16 x i1> %mask, double* %addr, <16 x i32> %index) {
  %ptr = bitcast double *%addr to i8*
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %imask0 = bitcast <8 x i1> %mask0 to i8
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %imask1 = bitcast <8 x i1> %mask1 to i8
  %t0 = call <8 x double> @llvm.x86.mic.mask.gather.pd(<8 x double> undef,
                                               i8 %imask0,
                                               <16 x i32> %index, i8* %ptr,
                                               i32 0, ; no up conversion
                                               i32 8, ; scale 8
                                               i32 0) ; gather to L1 cache
  %t1 = call <8 x double> @llvm.x86.mic.mask.gather.pd(<8 x double> undef,
                                               i8 %imask1,
                                               <16 x i32> %index1, i8* %ptr,
                                               i32 0, ; no up conversion
                                               i32 8, ; scale 8
                                               i32 0) ; gather to L1 cache
  %t2 = shufflevector <8 x double> %t0, <8 x double> %t1,
            <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x double> %t2
}

define void @scatter.v16i8 (i8* %ptr, <16 x i32>%index, <16 x i8> %data) {
  %data1 = zext <16 x i8> %data to <16 x i32>
  call void @llvm.x86.mic.scatter.pi(i8* %ptr,
                                    <16 x i32> %index,
                                    <16 x i32> %data1,
                                    i32 1, ; u32 -> u16
                                    i32 1, ; scale 1
                                    i32 0) ; scatter to L1 cache
  ret void
}

define void @masked_scatter.v16i8 (<16 x i1> %mask, i8* %ptr, <16 x i32>%index,
                                <16 x i8> %data) {
  %imask = bitcast <16 x i1> %mask to i16
  %data1 = zext <16 x i8> %data to <16 x i32>
  call void @llvm.x86.mic.mask.scatter.pi(i8* %ptr,
                                          i16 %imask,
                                          <16 x i32> %index,
                                          <16 x i32> %data1,
                                          i32 1, ; u32 -> u8
                                          i32 1, ; scale 1
                                          i32 0) ; scatter to L1 cache
  ret void
}

define void @scatter.v16i16 (i16* %addr, <16 x i32>%index, <16 x i16> %data) {
  %ptr = bitcast i16 *%addr to i8*
  %data1 = zext <16 x i16> %data to <16 x i32>
  call void @llvm.x86.mic.scatter.pi(i8* %ptr,
                                    <16 x i32> %index,
                                    <16 x i32> %data1,
                                    i32 3, ; u32 -> u16
                                    i32 2, ; scale 2
                                    i32 0) ; scatter to L1 cache
  ret void
}

define void @masked_scatter.v16i16 (<16 x i1> %mask, i16* %addr, <16 x i32>%index,
                                <16 x i16> %data) {
  %ptr = bitcast i16 *%addr to i8*
  %imask = bitcast <16 x i1> %mask to i16
  %data1 = zext <16 x i16> %data to <16 x i32>
  call void @llvm.x86.mic.mask.scatter.pi(i8* %ptr,
                                          i16 %imask,
                                          <16 x i32> %index,
                                          <16 x i32> %data1,
                                          i32 3, ; u32 -> u16
                                          i32 2, ; scale 2
                                          i32 0) ; scatter to L1 cache
  ret void
}

define void @scatter.v16i32 (i32* %addr, <16 x i32>%index, <16 x i32> %data) {
  %ptr = bitcast i32 *%addr to i8*
  call void @llvm.x86.mic.scatter.pi(i8* %ptr,
                                    <16 x i32> %index,
                                    <16 x i32> %data,
                                    i32 0, ; no down conversion
                                    i32 4, ; scale 4
                                    i32 0) ; scatter to L1 cache
  ret void
}

define void @masked_scatter.v16i32 (<16 x i1> %mask, i32* %addr, <16 x i32>%index,
                                <16 x i32> %data) {
  %ptr = bitcast i32 *%addr to i8*
  %imask = bitcast <16 x i1> %mask to i16
  call void @llvm.x86.mic.mask.scatter.pi(i8* %ptr,
                                          i16 %imask,
                                          <16 x i32> %index,
                                          <16 x i32> %data,
                                          i32 0, ; no down conversion
                                          i32 4, ; scale 4
                                          i32 0) ; scatter to L1 cache
  ret void
}

define void @scatter.v16i64 (i64* %addr, <16 x i32>%index, <16 x i64> %data) {
  %ptr = bitcast i64 *%addr to i8*
  %data0 = shufflevector <16 x i64> %data, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %data1 = shufflevector <16 x i64> %data, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.mic.scatter.pq(i8* %ptr,
                                    <16 x i32> %index,
                                    <8 x i64> %data0,
                                    i32 0, ; no down conversion
                                    i32 8, ; scale 8
                                    i32 0) ; scatter to L1 cache
  call void @llvm.x86.mic.scatter.pq(i8* %ptr,
                                    <16 x i32> %index1,
                                    <8 x i64> %data1,
                                    i32 0, ; no down conversion
                                    i32 8, ; scale 8
                                    i32 0) ; scatter to L1 cache
  ret void
}

define void @masked_scatter.v16i64 (<16 x i1> %mask, i64* %addr, <16 x i32>%index,
                                <16 x i64> %data) {
  %ptr = bitcast i64 *%addr to i8*
  %data0 = shufflevector <16 x i64> %data, <16 x i64> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %imask0 = bitcast <8 x i1> %mask0 to i8
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %data1 = shufflevector <16 x i64> %data, <16 x i64> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %imask1 = bitcast <8 x i1> %mask1 to i8
  call void @llvm.x86.mic.mask.scatter.pq(i8* %ptr,
                                          i8 %imask0,
                                          <16 x i32> %index,
                                          <8 x i64> %data0,
                                          i32 0, ; no down conversion
                                          i32 8, ; scale 8
                                          i32 0) ; scatter to L1 cache
  call void @llvm.x86.mic.mask.scatter.pq(i8* %ptr,
                                          i8 %imask1,
                                          <16 x i32> %index1,
                                          <8 x i64> %data1,
                                          i32 0, ; no down conversion
                                          i32 8, ; scale 8
                                          i32 0) ; scatter to L1 cache
  ret void
}

define void @scatter.v16f32 (float* %addr, <16 x i32>%index, <16 x float> %data) {
  %ptr = bitcast float *%addr to i8*
  call void @llvm.x86.mic.scatter.ps(i8* %ptr,
                                    <16 x i32> %index,
                                    <16 x float> %data,
                                    i32 0, ; no down conversion
                                    i32 4, ; scale 4
                                    i32 0) ; scatter to L1 cache
  ret void
}

define void @masked_scatter.v16f32 (<16 x i1> %mask, float* %addr, <16 x i32>%index,
                                <16 x float> %data) {
  %ptr = bitcast float *%addr to i8*
  %imask = bitcast <16 x i1> %mask to i16
  call void @llvm.x86.mic.mask.scatter.ps(i8* %ptr,
                                          i16 %imask,
                                          <16 x i32> %index,
                                          <16 x float> %data,
                                          i32 0, ; no down conversion
                                          i32 4, ; scale 4
                                          i32 0) ; scatter to L1 cache
  ret void
}

define void @scatter.v16f64 (double* %addr, <16 x i32>%index, <16 x double> %data) {
  %ptr = bitcast double *%addr to i8*
  %data0 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %data1 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  call void @llvm.x86.mic.scatter.pd(i8* %ptr,
                                    <16 x i32> %index,
                                    <8 x double> %data0,
                                    i32 0, ; no down conversion
                                    i32 8, ; scale 8
                                    i32 0) ; scatter to L1 cache
  call void @llvm.x86.mic.scatter.pd(i8* %ptr,
                                    <16 x i32> %index1,
                                    <8 x double> %data1,
                                    i32 0, ; no down conversion
                                    i32 8, ; scale 8
                                    i32 0) ; scatter to L1 cache
  ret void
}

define void @masked_scatter.v16f64 (<16 x i1> %mask, double* %addr, <16 x i32>%index,
                                <16 x double> %data) {
  %ptr = bitcast double *%addr to i8*
  %data0 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %mask0 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %imask0 = bitcast <8 x i1> %mask0 to i8
  %index1 = shufflevector <16 x i32> %index, <16 x i32> undef,
            <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15,
             i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %data1 = shufflevector <16 x double> %data, <16 x double> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %mask1 = shufflevector <16 x i1> %mask, <16 x i1> undef,
            <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %imask1 = bitcast <8 x i1> %mask1 to i8
  call void @llvm.x86.mic.mask.scatter.pd(i8* %ptr,
                                          i8 %imask0,
                                          <16 x i32> %index,
                                          <8 x double> %data0,
                                          i32 0, ; no down conversion
                                          i32 8, ; scale 8
                                          i32 0) ; scatter to L1 cache
  call void @llvm.x86.mic.mask.scatter.pd(i8* %ptr,
                                          i8 %imask1,
                                          <16 x i32> %index1,
                                          <8 x double> %data1,
                                          i32 0, ; no down conversion
                                          i32 8, ; scale 8
                                          i32 0) ; scatter to L1 cache
  ret void
}
