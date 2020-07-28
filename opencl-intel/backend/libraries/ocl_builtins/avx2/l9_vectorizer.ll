; ModuleID = '<stdin>'

define i1 @__ocl_allOne(i1 %pred) nounwind readnone {
entry:
  ret i1 %pred
}

define i1 @__ocl_allOne_i32(i32 %pred) nounwind readnone {
entry:
  %0 = ashr i32 %pred, 31
  %1 = trunc i32 %0 to i1
  ret i1 %1
}

define i1 @__ocl_allOne_v2(<2 x i1> %pred) nounwind readnone {
entry:
  %elem0 = extractelement <2 x i1> %pred, i32 0
  %elem1 = extractelement <2 x i1> %pred, i32 1
  %res = and i1 %elem0, %elem1
  ret i1 %res
}

define i1 @__ocl_allOne_v2_i32(<2 x i32> %pred) nounwind readnone {
entry:
  %elem0 = extractelement <2 x i32> %pred, i32 0
  %elem1 = extractelement <2 x i32> %pred, i32 1
  %res = and i32 %elem0, %elem1
  %f = call i1 @__ocl_allOne_i32(i32 %res)
  ret i1 %f
}

define i1 @__ocl_allOne_v4_i32(<4 x i32> %pred) nounwind readnone {
entry:
  %a = bitcast <4 x i32> %pred to <4 x float>
  %b = bitcast <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1> to <4 x float>
  %res = call i32 @llvm.x86.sse41.ptestc(<4 x float> %a, <4 x float> %b)
  %one = trunc i32 %res to i1
  ret i1 %one
}

define i1 @__ocl_allOne_v4(<4 x i1> %pred) nounwind readnone {
entry:
  %t = sext <4 x i1> %pred to <4 x i32>
  %res = call i1 @__ocl_allOne_v4_i32(<4 x i32> %t)
  ret i1 %res
}

define i1 @__ocl_allOne_v8_i32(<8 x i32> %pred) nounwind readnone {
entry:
  %a = bitcast <8 x i32> %pred to <4 x i64>
  %res = call i32 @llvm.x86.avx.ptestc.256(<4 x i64> %a, <4 x i64> <i64 -1, i64 -1, i64 -1, i64 -1>)
  %one = trunc i32 %res to i1
  ret i1 %one
}

define i1 @__ocl_allOne_v8(<8 x i1> %pred) nounwind readnone {
entry:
  %t = sext <8 x i1> %pred to <8 x i32>
  %res = call i1 @__ocl_allOne_v8_i32(<8 x i32> %t)
  ret i1 %res
}

define i1 @__ocl_allOne_v16_i32(<16 x i32> %pred) nounwind readnone {
entry:
  %v0 = shufflevector <16 x i32> %pred, <16 x i32> undef,
                      <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %v1 = shufflevector <16 x i32> %pred, <16 x i32> undef,
                      <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %f0 = call i1 @__ocl_allOne_v8_i32(<8 x i32> %v0)
  %f1 = call i1 @__ocl_allOne_v8_i32(<8 x i32> %v1)
  %f = and i1 %f0, %f1
  ret i1 %f
}

define i1 @__ocl_allOne_v16(<16 x i1> %pred) nounwind readnone {
entry:
  %t = sext <16 x i1> %pred to <16 x i32>
  %res = call i1 @__ocl_allOne_v16_i32(<16 x i32> %t)
  ret i1 %res
}

define i1 @__ocl_allZero(i1 %pred) nounwind readnone {
entry:
  %t = xor i1 %pred, true
  ret i1 %t
}

define i1 @__ocl_allZero_v2(<2 x i1> %pred) nounwind readnone {
entry:
  %elem0 = extractelement <2 x i1> %pred, i32 0
  %elem1 = extractelement <2 x i1> %pred, i32 1
  %res = or i1 %elem0, %elem1
  %f = call i1 @__ocl_allZero(i1 %res)
  ret i1 %f
}

define i1 @__ocl_allZero_v4(<4 x i1> %pred) nounwind readnone {
entry:
  %t = sext <4 x i1> %pred to <4 x i32>
  %res = call i1 @__ocl_allZero_v4_i32(<4 x i32> %t)
  ret i1 %res
}

define i1 @__ocl_allZero_v8(<8 x i1> %pred) nounwind readnone {
entry:
  %t = sext <8 x i1> %pred to <8 x i32>
  %res = call i1 @__ocl_allZero_v8_i32(<8 x i32> %t)
  ret i1 %res
}

define i1 @__ocl_allZero_v16(<16 x i1> %pred) nounwind readnone {
entry:
  %t = sext <16 x i1> %pred to <16 x i32>
  %res = call i1 @__ocl_allZero_v16_i32(<16 x i32> %t)
  ret i1 %res
}

define i1 @__ocl_allZero_i32(i32 %pred) nounwind readnone {
entry:
  %0 = and i32 %pred, 2147483647
  %1 = icmp eq i32 %0, 0
  ret i1 %1
}

define i1 @__ocl_allZero_v2_i32(<2 x i32> %pred) nounwind readnone {
entry:
  %elem0 = extractelement <2 x i32> %pred, i32 0
  %elem1 = extractelement <2 x i32> %pred, i32 1
  %res = or i32 %elem0, %elem1
  %f = call i1 @__ocl_allZero_i32(i32 %res)
  ret i1 %f
}

define i1 @__ocl_allZero_v4_i32(<4 x i32> %pred) nounwind readnone {
entry:
  %a = bitcast <4 x i32> %pred to <4 x float>
  %res = call i32 @llvm.x86.sse41.ptestz(<4 x float> %a, <4 x float> %a)
  %zero = trunc i32 %res to i1
  ret i1 %zero
}

define i1 @__ocl_allZero_v8_i32(<8 x i32> %pred) nounwind readnone {
entry:
  %a = bitcast <8 x i32> %pred to <4 x i64>
  %res = call i32 @llvm.x86.avx.ptestz.256(<4 x i64> %a, <4 x i64> %a)
  %zero = trunc i32 %res to i1
  ret i1 %zero
}

define i1 @__ocl_allZero_v16_i32(<16 x i32> %pred) nounwind readnone {
entry:
  %v0 = shufflevector <16 x i32> %pred, <16 x i32> undef,
                      <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %v1 = shufflevector <16 x i32> %pred, <16 x i32> undef,
                      <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %f0 = call i1 @__ocl_allZero_v8_i32(<8 x i32> %v0)
  %f1 = call i1 @__ocl_allZero_v8_i32(<8 x i32> %v1)
  %f = and i1 %f0, %f1
  ret i1 %f
}

declare i32 @llvm.x86.sse41.ptestc(<4 x float>, <4 x float>) nounwind readnone

declare i32 @llvm.x86.sse41.ptestz(<4 x float>, <4 x float>) nounwind readnone

declare i32 @llvm.x86.avx.ptestc.256(<4 x i64>, <4 x i64>) nounwind readnone

declare i32 @llvm.x86.avx.ptestz.256(<4 x i64>, <4 x i64>) nounwind readnone
