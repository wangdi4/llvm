; ModuleID = '/home/zrackove/my_MIC/src/backend/libraries/ocl_builtins/clbltfnb1_vectorizer.bc'
target datalayout = "e-p:64:64"

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

declare i16 @llvm.x86.mic.knot(i16) nounwind readnone

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allOne_v2(<2 x i1> %pred) {
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

define i1 @allOne_v4(<4 x i1> %pred) {
entry:
  %0 = shufflevector <4 x i1> %pred, <4 x i1> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %1 = shufflevector <8 x i1> %0, <8 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %ipred = bitcast <16 x i1> %1 to i16
  %lpred = zext i16 %ipred to i32
  %mask = and i32 %lpred, 15
  %res = icmp eq i32 %mask, 15
  ret i1 %res
}

define i1 @allOne_v8(<8 x i1> %pred) {
entry:
  %0 = shufflevector <8 x i1> %pred, <8 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %ipred = bitcast <16 x i1> %0 to i16
  %lpred = zext i16 %ipred to i32
  %mask = and i32 %lpred, 255
  %res = icmp eq i32 %mask, 255
  ret i1 %res
}

define i1 @allOne_v16(<16 x i1> %pred) {
entry:
  %ipred = bitcast <16 x i1> %pred to i16
  %val = call i32 @llvm.x86.mic.kortestc(i16 %ipred, i16 %ipred)
  %res = trunc i32 %val to i1
  ret i1 %res
}

define i1 @allZero(i1 %t) {
entry:
  %pred = xor i1 %t, true
  ret i1 %pred
}

define i1 @allZero_v2(<2 x i1> %t) {
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

define i1 @allZero_v4(<4 x i1> %t) {
entry:
  %0 = shufflevector <4 x i1> %t, <4 x i1> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %1 = shufflevector <8 x i1> %0, <8 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %ipred = bitcast <16 x i1> %1 to i16
  %lpred = zext i16 %ipred to i32
  %mask = and i32 %lpred, 15
  %res = icmp eq i32 %mask, 0
  ret i1 %res
}

define i1 @allZero_v8(<8 x i1> %t) {
entry:
  %0 = shufflevector <8 x i1> %t, <8 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %ipred = bitcast <16 x i1> %0 to i16
  %lpred = zext i16 %ipred to i32
  %mask = and i32 %lpred, 255
  %res = icmp eq i32 %mask, 0
  ret i1 %res
}

define i1 @allZero_v16(<16 x i1> %t) {
entry:
  %ipred = bitcast <16 x i1> %t to i16
  %val = call i32 @llvm.x86.mic.kortestz(i16 %ipred, i16 %ipred)
  %res = trunc i32 %val to i1
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
