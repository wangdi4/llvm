; ModuleID = '<stdin>'

define i1 @allOne(i1 %pred) nounwind readnone {
entry:
  ret i1 %pred
}

define i1 @allOne_i32(i32 %pred) nounwind readnone {
entry:
  %0 = ashr i32 %pred, 31
  %1 = trunc i32 %0 to i1
  ret i1 %1
}

define i1 @allOne_v2(<2 x i1> %pred) nounwind readnone {
entry:
  %elem0 = extractelement <2 x i1> %pred, i32 0
  %elem1 = extractelement <2 x i1> %pred, i32 1
  %res = and i1 %elem0, %elem1
  ret i1 %res
}

define i1 @allOne_v2_i32(<2 x i32> %pred) nounwind readnone {
entry:
  %elem0 = extractelement <2 x i32> %pred, i32 0
  %elem1 = extractelement <2 x i32> %pred, i32 1
  %res = and i32 %elem0, %elem1
  %f = call i1 @allOne_i32(i32 %res)
  ret i1 %f
}

define i1 @allOne_v4_i32(<4 x i32> %pred) nounwind readnone {
entry:
  %a = bitcast <4 x i32> %pred to <4 x float>
  %b = bitcast <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1> to <4 x float>
  %res = call i32 @llvm.x86.sse41.ptestc(<4 x float> %a, <4 x float> %b)
  %one = trunc i32 %res to i1
  ret i1 %one
}

define i1 @allOne_v4(<4 x i1> %pred) nounwind readnone {
entry:
  %elem0 = extractelement <4 x i1> %pred, i32 0
  %elem1 = extractelement <4 x i1> %pred, i32 1
  %elem2 = extractelement <4 x i1> %pred, i32 2
  %elem3 = extractelement <4 x i1> %pred, i32 3
  %res1 = and i1 %elem0, %elem1
  %res2 = and i1 %elem2, %elem3
  %res = and i1 %res1, %res2
  ret i1 %res
}

define i1 @allOne_v8_i32(<8 x i32> %pred) nounwind readnone {
entry:
  %elem0 = extractelement <8 x i32> %pred, i32 0
  %elem1 = extractelement <8 x i32> %pred, i32 1
  %elem2 = extractelement <8 x i32> %pred, i32 2
  %elem3 = extractelement <8 x i32> %pred, i32 3
  %elem4 = extractelement <8 x i32> %pred, i32 4
  %elem5 = extractelement <8 x i32> %pred, i32 5
  %elem6 = extractelement <8 x i32> %pred, i32 6
  %elem7 = extractelement <8 x i32> %pred, i32 7
  %v1_0 = insertelement <4 x i32> undef, i32 %elem0, i32 0
  %v1_1 = insertelement <4 x i32> %v1_0, i32 %elem1, i32 1
  %v1_2 = insertelement <4 x i32> %v1_1, i32 %elem2, i32 2
  %v1_3 = insertelement <4 x i32> %v1_2, i32 %elem3, i32 3
  %v2_0 = insertelement <4 x i32> undef, i32 %elem4, i32 0
  %v2_1 = insertelement <4 x i32> %v2_0, i32 %elem5, i32 1
  %v2_2 = insertelement <4 x i32> %v2_1, i32 %elem6, i32 2
  %v2_3 = insertelement <4 x i32> %v2_2, i32 %elem7, i32 3
  %f0 = call i1 @allOne_v4_i32(<4 x i32> %v1_3)
  %f1 = call i1 @allOne_v4_i32(<4 x i32> %v2_3)
  %f = and i1 %f0, %f1
  ret i1 %f
}

define i1 @allOne_v8(<8 x i1> %pred) nounwind readnone {
entry:
  %elem0 = extractelement <8 x i1> %pred, i32 0
  %elem1 = extractelement <8 x i1> %pred, i32 1
  %elem2 = extractelement <8 x i1> %pred, i32 2
  %elem3 = extractelement <8 x i1> %pred, i32 3
  %elem4 = extractelement <8 x i1> %pred, i32 4
  %elem5 = extractelement <8 x i1> %pred, i32 5
  %elem6 = extractelement <8 x i1> %pred, i32 6
  %elem7 = extractelement <8 x i1> %pred, i32 7
  %v1_0 = insertelement <4 x i1> undef, i1 %elem0, i32 0
  %v1_1 = insertelement <4 x i1> %v1_0, i1 %elem1, i32 1
  %v1_2 = insertelement <4 x i1> %v1_1, i1 %elem2, i32 2
  %v1_3 = insertelement <4 x i1> %v1_2, i1 %elem3, i32 3
  %v2_0 = insertelement <4 x i1> undef, i1 %elem4, i32 0
  %v2_1 = insertelement <4 x i1> %v2_0, i1 %elem5, i32 1
  %v2_2 = insertelement <4 x i1> %v2_1, i1 %elem6, i32 2
  %v2_3 = insertelement <4 x i1> %v2_2, i1 %elem7, i32 3
  %f0 = call i1 @allOne_v4(<4 x i1> %v1_3)
  %f1 = call i1 @allOne_v4(<4 x i1> %v2_3)
  %f = and i1 %f0, %f1
  ret i1 %f
}

define i1 @allOne_v16_i32(<16 x i32> %pred) nounwind readnone {
entry:
  %elem0 = extractelement <16 x i32> %pred, i32 0
  %elem1 = extractelement <16 x i32> %pred, i32 1
  %elem2 = extractelement <16 x i32> %pred, i32 2
  %elem3 = extractelement <16 x i32> %pred, i32 3
  %elem4 = extractelement <16 x i32> %pred, i32 4
  %elem5 = extractelement <16 x i32> %pred, i32 5
  %elem6 = extractelement <16 x i32> %pred, i32 6
  %elem7 = extractelement <16 x i32> %pred, i32 7
  %elem8 = extractelement <16 x i32> %pred, i32 8
  %elem9 = extractelement <16 x i32> %pred, i32 9
  %elem10 = extractelement <16 x i32> %pred, i32 10
  %elem11 = extractelement <16 x i32> %pred, i32 11
  %elem12 = extractelement <16 x i32> %pred, i32 12
  %elem13 = extractelement <16 x i32> %pred, i32 13
  %elem14 = extractelement <16 x i32> %pred, i32 14
  %elem15 = extractelement <16 x i32> %pred, i32 15
  %v1_0 = insertelement <8 x i32> undef, i32 %elem0, i32 0
  %v1_1 = insertelement <8 x i32> %v1_0, i32 %elem1, i32 1
  %v1_2 = insertelement <8 x i32> %v1_1, i32 %elem2, i32 2
  %v1_3 = insertelement <8 x i32> %v1_2, i32 %elem3, i32 3
  %v1_4 = insertelement <8 x i32> %v1_3, i32 %elem4, i32 4
  %v1_5 = insertelement <8 x i32> %v1_4, i32 %elem5, i32 5
  %v1_6 = insertelement <8 x i32> %v1_5, i32 %elem6, i32 6
  %v1_7 = insertelement <8 x i32> %v1_6, i32 %elem7, i32 7
  %v2_0 = insertelement <8 x i32> undef, i32 %elem8, i32 0
  %v2_1 = insertelement <8 x i32> %v2_0, i32 %elem9, i32 1
  %v2_2 = insertelement <8 x i32> %v2_1, i32 %elem10, i32 2
  %v2_3 = insertelement <8 x i32> %v2_2, i32 %elem11, i32 3
  %v2_4 = insertelement <8 x i32> %v2_3, i32 %elem12, i32 4
  %v2_5 = insertelement <8 x i32> %v2_4, i32 %elem13, i32 5
  %v2_6 = insertelement <8 x i32> %v2_5, i32 %elem14, i32 6
  %v2_7 = insertelement <8 x i32> %v2_6, i32 %elem15, i32 7
  %f0 = call i1 @allOne_v8_i32(<8 x i32> %v1_7)
  %f1 = call i1 @allOne_v8_i32(<8 x i32> %v2_7)
  %f = and i1 %f0, %f1
  ret i1 %f
}

define i1 @allOne_v16(<16 x i1> %pred) nounwind readnone {
entry:
  %elem0 = extractelement <16 x i1> %pred, i32 0
  %elem1 = extractelement <16 x i1> %pred, i32 1
  %elem2 = extractelement <16 x i1> %pred, i32 2
  %elem3 = extractelement <16 x i1> %pred, i32 3
  %elem4 = extractelement <16 x i1> %pred, i32 4
  %elem5 = extractelement <16 x i1> %pred, i32 5
  %elem6 = extractelement <16 x i1> %pred, i32 6
  %elem7 = extractelement <16 x i1> %pred, i32 7
  %elem8 = extractelement <16 x i1> %pred, i32 8
  %elem9 = extractelement <16 x i1> %pred, i32 9
  %elem10 = extractelement <16 x i1> %pred, i32 10
  %elem11 = extractelement <16 x i1> %pred, i32 11
  %elem12 = extractelement <16 x i1> %pred, i32 12
  %elem13 = extractelement <16 x i1> %pred, i32 13
  %elem14 = extractelement <16 x i1> %pred, i32 14
  %elem15 = extractelement <16 x i1> %pred, i32 15
  %v1_0 = insertelement <8 x i1> undef, i1 %elem0, i32 0
  %v1_1 = insertelement <8 x i1> %v1_0, i1 %elem1, i32 1
  %v1_2 = insertelement <8 x i1> %v1_1, i1 %elem2, i32 2
  %v1_3 = insertelement <8 x i1> %v1_2, i1 %elem3, i32 3
  %v1_4 = insertelement <8 x i1> %v1_3, i1 %elem4, i32 4
  %v1_5 = insertelement <8 x i1> %v1_4, i1 %elem5, i32 5
  %v1_6 = insertelement <8 x i1> %v1_5, i1 %elem6, i32 6
  %v1_7 = insertelement <8 x i1> %v1_6, i1 %elem7, i32 7
  %v2_0 = insertelement <8 x i1> undef, i1 %elem8, i32 0
  %v2_1 = insertelement <8 x i1> %v2_0, i1 %elem9, i32 1
  %v2_2 = insertelement <8 x i1> %v2_1, i1 %elem10, i32 2
  %v2_3 = insertelement <8 x i1> %v2_2, i1 %elem11, i32 3
  %v2_4 = insertelement <8 x i1> %v2_3, i1 %elem12, i32 4
  %v2_5 = insertelement <8 x i1> %v2_4, i1 %elem13, i32 5
  %v2_6 = insertelement <8 x i1> %v2_5, i1 %elem14, i32 6
  %v2_7 = insertelement <8 x i1> %v2_6, i1 %elem15, i32 7
  %f0 = call i1 @allOne_v8(<8 x i1> %v1_7)
  %f1 = call i1 @allOne_v8(<8 x i1> %v2_7)
  %f = and i1 %f0, %f1
  ret i1 %f
}

define i1 @allZero(i1 %pred) nounwind readnone {
entry:
  %t = xor i1 %pred, true
  ret i1 %t
}

define i1 @allZero_v2(<2 x i1> %pred) nounwind readnone {
entry:
  %t = xor <2 x i1> %pred, <i1 true, i1 true>
  %res = call i1 @allOne_v2(<2 x i1> %t)
  ret i1 %res
}

define i1 @allZero_v4(<4 x i1> %pred) nounwind readnone {
entry:
  %t = xor <4 x i1> %pred, <i1 true, i1 true, i1 true, i1 true>
  %res = call i1 @allOne_v4(<4 x i1> %t)
  ret i1 %res
}

define i1 @allZero_v8(<8 x i1> %pred) nounwind readnone {
entry:
  %t = xor <8 x i1> %pred, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %res = call i1 @allOne_v8(<8 x i1> %t)
  ret i1 %res
}

define i1 @allZero_v16(<16 x i1> %pred) nounwind readnone {
entry:
  %t = xor <16 x i1> %pred, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %res = call i1 @allOne_v16(<16 x i1> %t)
  ret i1 %res
}

define i1 @allZero_i32(i32 %pred) nounwind readnone {
entry:
  %t = xor i32 %pred, -1
  %f = call i1 @allOne_i32(i32 %t)
  ret i1 %f
}

define i1 @allZero_v2_i32(<2 x i32> %pred) nounwind readnone {
entry:
  %t = xor <2 x i32> %pred, <i32 -1, i32 -1>
  %res = call i1 @allOne_v2_i32(<2 x i32> %t)
  ret i1 %res
}

define i1 @allZero_v4_i32(<4 x i32> %pred) nounwind readnone {
entry:
  %a = bitcast <4 x i32> %pred to <4 x float>
  %res = call i32 @llvm.x86.sse41.ptestz(<4 x float> %a, <4 x float> %a)
  %one = trunc i32 %res to i1
  ret i1 %one
}

define i1 @allZero_v8_i32(<8 x i32> %pred) nounwind readnone {
entry:
  %t = xor <8 x i32> %pred, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
  %res = call i1 @allOne_v8_i32(<8 x i32> %t)
  ret i1 %res
}

define i1 @allZero_v16_i32(<16 x i32> %pred) nounwind readnone {
entry:
  %t = xor <16 x i32> %pred, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
  %res = call i1 @allOne_v16_i32(<16 x i32> %t)
  ret i1 %res
}

declare void @barrier(i32)

define void @barrier_v1(i1 %mask, i32 %type) {
  br i1 %mask, label %do, label %dont

do:                                               ; preds = %0
  call void @barrier(i32 %type)
  br label %dont

dont:                                             ; preds = %do, %0
  ret void
}

define void @barrier_v2(<2 x i1> %mask, <2 x i32> %type) {
  %m0 = extractelement <2 x i1> %mask, i32 0
  br i1 %m0, label %do, label %dont

do:                                               ; preds = %0
  %t0 = extractelement <2 x i32> %type, i32 0
  call void @barrier(i32 %t0)
  br label %dont

dont:                                             ; preds = %do, %0
  ret void
}

define void @barrier_v4(<4 x i1> %mask, <4 x i32> %type) {
  %m0 = extractelement <4 x i1> %mask, i32 0
  br i1 %m0, label %do, label %dont

do:                                               ; preds = %0
  %t0 = extractelement <4 x i32> %type, i32 0
  call void @barrier(i32 %t0)
  br label %dont

dont:                                             ; preds = %do, %0
  ret void
}

define void @barrier_v8(<8 x i1> %mask, <8 x i32> %type) {
  %m0 = extractelement <8 x i1> %mask, i32 0
  br i1 %m0, label %do, label %dont

do:                                               ; preds = %0
  %t0 = extractelement <8 x i32> %type, i32 0
  call void @barrier(i32 %t0)
  br label %dont

dont:                                             ; preds = %do, %0
  ret void
}

define void @barrier_v16(<16 x i1> %mask, <16 x i32> %type) {
  %m0 = extractelement <16 x i1> %mask, i32 0
  br i1 %m0, label %do, label %dont

do:                                               ; preds = %0
  %t0 = extractelement <16 x i32> %type, i32 0
  call void @barrier(i32 %t0)
  br label %dont

dont:                                             ; preds = %do, %0
  ret void
}

declare i32 @llvm.x86.sse41.ptestc(<4 x float>, <4 x float>) nounwind readnone

declare i32 @llvm.x86.sse41.ptestz(<4 x float>, <4 x float>) nounwind readnone
