
; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/apple_only_dcls32.ll -runtime=apple -CLBltnResolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'c:\work\geo_tests32_preVec.ll'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @__crossf3_test
; CHECK-NOT: @_f_v.__vertical_cross3f4
; CHECK: @__vertical_cross3f4
; CHECK-NOT: @_f_v.__vertical_cross3f4
; CHECK: ret void
define void @__crossf3_test(<3 x float>* nocapture, <3 x float>* nocapture, <3 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %3, i32 0
  %extract15 = extractelement <4 x i32> %3, i32 1
  %extract16 = extractelement <4 x i32> %3, i32 2
  %extract17 = extractelement <4 x i32> %3, i32 3
  %4 = getelementptr <3 x float>* %0, i32 %extract
  %5 = getelementptr <3 x float>* %0, i32 %extract15
  %6 = getelementptr <3 x float>* %0, i32 %extract16
  %7 = getelementptr <3 x float>* %0, i32 %extract17
  %8 = load <3 x float>* %4
  %9 = load <3 x float>* %5
  %10 = load <3 x float>* %6
  %11 = load <3 x float>* %7
  %extend_vec = shufflevector <3 x float> %8, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec18 = shufflevector <3 x float> %9, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec19 = shufflevector <3 x float> %10, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec20 = shufflevector <3 x float> %11, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec18, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec19, <4 x float> %extend_vec20, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge27 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle032 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec18, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle133 = shufflevector <4 x float> %extend_vec19, <4 x float> %extend_vec20, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge34 = shufflevector <4 x float> %shuffle032, <4 x float> %shuffle133, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %12 = getelementptr <3 x float>* %1, i32 %extract
  %13 = getelementptr <3 x float>* %1, i32 %extract15
  %14 = getelementptr <3 x float>* %1, i32 %extract16
  %15 = getelementptr <3 x float>* %1, i32 %extract17
  %16 = load <3 x float>* %12
  %17 = load <3 x float>* %13
  %18 = load <3 x float>* %14
  %19 = load <3 x float>* %15
  %extend_vec35 = shufflevector <3 x float> %16, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec36 = shufflevector <3 x float> %17, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec37 = shufflevector <3 x float> %18, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec38 = shufflevector <3 x float> %19, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %shuffle039 = shufflevector <4 x float> %extend_vec35, <4 x float> %extend_vec36, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle140 = shufflevector <4 x float> %extend_vec37, <4 x float> %extend_vec38, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge41 = shufflevector <4 x float> %shuffle039, <4 x float> %shuffle140, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge48 = shufflevector <4 x float> %shuffle039, <4 x float> %shuffle140, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle053 = shufflevector <4 x float> %extend_vec35, <4 x float> %extend_vec36, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle154 = shufflevector <4 x float> %extend_vec37, <4 x float> %extend_vec38, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge55 = shufflevector <4 x float> %shuffle053, <4 x float> %shuffle154, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %store.val = insertvalue [3 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val77 = insertvalue [3 x <4 x float>] %store.val, <4 x float> %shuffleMerge27, 1
  %store.val78 = insertvalue [3 x <4 x float>] %store.val77, <4 x float> %shuffleMerge34, 2
  %store.val79 = insertvalue [3 x <4 x float>] undef, <4 x float> %shuffleMerge41, 0
  %store.val80 = insertvalue [3 x <4 x float>] %store.val79, <4 x float> %shuffleMerge48, 1
  %store.val81 = insertvalue [3 x <4 x float>] %store.val80, <4 x float> %shuffleMerge55, 2
  %20 = tail call [3 x <4 x float>] @_f_v.__vertical_cross3f4([3 x <4 x float>] %store.val78, [3 x <4 x float>] %store.val81)
  %21 = extractvalue [3 x <4 x float>] %20, 0
  %22 = extractvalue [3 x <4 x float>] %20, 1
  %23 = extractvalue [3 x <4 x float>] %20, 2
  %shuf_transpL82 = shufflevector <4 x float> %21, <4 x float> %23, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH84 = shufflevector <4 x float> %21, <4 x float> %23, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH85 = shufflevector <4 x float> %22, <4 x float> undef, <4 x i32> <i32 2, i32 3, i32 undef, i32 undef>
  %shuf_transpL86 = shufflevector <4 x float> %shuf_transpL82, <4 x float> %22, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH87 = shufflevector <4 x float> %shuf_transpL82, <4 x float> %22, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL88 = shufflevector <4 x float> %shuf_transpH84, <4 x float> %shuf_transpH85, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH89 = shufflevector <4 x float> %shuf_transpH84, <4 x float> %shuf_transpH85, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %breakdown90 = shufflevector <4 x float> %shuf_transpL86, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown91 = shufflevector <4 x float> %shuf_transpH87, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown92 = shufflevector <4 x float> %shuf_transpL88, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown93 = shufflevector <4 x float> %shuf_transpH89, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %24 = getelementptr <3 x float>* %2, i32 %extract
  %25 = getelementptr <3 x float>* %2, i32 %extract15
  %26 = getelementptr <3 x float>* %2, i32 %extract16
  %27 = getelementptr <3 x float>* %2, i32 %extract17
  store <3 x float> %breakdown90, <3 x float>* %24
  store <3 x float> %breakdown91, <3 x float>* %25
  store <3 x float> %breakdown92, <3 x float>* %26
  store <3 x float> %breakdown93, <3 x float>* %27
  ret void
}

declare i32 @get_global_id(i32)


; CHECK: @__crossf4_test
; CHECK-NOT: @_f_v.__vertical_cross4f4
; CHECK: @__vertical_cross4f4
; CHECK-NOT: @_f_v.__vertical_cross4f4
; CHECK: ret void
define void @__crossf4_test(<4 x float>* nocapture, <4 x float>* nocapture, <4 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %3, i32 0
  %extract21 = extractelement <4 x i32> %3, i32 1
  %extract22 = extractelement <4 x i32> %3, i32 2
  %extract23 = extractelement <4 x i32> %3, i32 3
  %4 = getelementptr <4 x float>* %0, i32 %extract
  %5 = getelementptr <4 x float>* %0, i32 %extract21
  %6 = getelementptr <4 x float>* %0, i32 %extract22
  %7 = getelementptr <4 x float>* %0, i32 %extract23
  %8 = load <4 x float>* %4
  %9 = load <4 x float>* %5
  %10 = load <4 x float>* %6
  %11 = load <4 x float>* %7
  %shuffle0 = shufflevector <4 x float> %8, <4 x float> %9, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %10, <4 x float> %11, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge26 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle027 = shufflevector <4 x float> %8, <4 x float> %9, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle128 = shufflevector <4 x float> %10, <4 x float> %11, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge29 = shufflevector <4 x float> %shuffle027, <4 x float> %shuffle128, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge32 = shufflevector <4 x float> %shuffle027, <4 x float> %shuffle128, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %12 = getelementptr <4 x float>* %1, i32 %extract
  %13 = getelementptr <4 x float>* %1, i32 %extract21
  %14 = getelementptr <4 x float>* %1, i32 %extract22
  %15 = getelementptr <4 x float>* %1, i32 %extract23
  %16 = load <4 x float>* %12
  %17 = load <4 x float>* %13
  %18 = load <4 x float>* %14
  %19 = load <4 x float>* %15
  %shuffle033 = shufflevector <4 x float> %16, <4 x float> %17, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle134 = shufflevector <4 x float> %18, <4 x float> %19, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge35 = shufflevector <4 x float> %shuffle033, <4 x float> %shuffle134, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge38 = shufflevector <4 x float> %shuffle033, <4 x float> %shuffle134, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle039 = shufflevector <4 x float> %16, <4 x float> %17, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle140 = shufflevector <4 x float> %18, <4 x float> %19, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge41 = shufflevector <4 x float> %shuffle039, <4 x float> %shuffle140, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge44 = shufflevector <4 x float> %shuffle039, <4 x float> %shuffle140, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [4 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val59 = insertvalue [4 x <4 x float>] %store.val, <4 x float> %shuffleMerge26, 1
  %store.val60 = insertvalue [4 x <4 x float>] %store.val59, <4 x float> %shuffleMerge29, 2
  %store.val61 = insertvalue [4 x <4 x float>] %store.val60, <4 x float> %shuffleMerge32, 3
  %store.val62 = insertvalue [4 x <4 x float>] undef, <4 x float> %shuffleMerge35, 0
  %store.val63 = insertvalue [4 x <4 x float>] %store.val62, <4 x float> %shuffleMerge38, 1
  %store.val64 = insertvalue [4 x <4 x float>] %store.val63, <4 x float> %shuffleMerge41, 2
  %store.val65 = insertvalue [4 x <4 x float>] %store.val64, <4 x float> %shuffleMerge44, 3
  %20 = tail call [4 x <4 x float>] @_f_v.__vertical_cross4f4([4 x <4 x float>] %store.val61, [4 x <4 x float>] %store.val65)
  %21 = extractvalue [4 x <4 x float>] %20, 0
  %22 = extractvalue [4 x <4 x float>] %20, 1
  %23 = extractvalue [4 x <4 x float>] %20, 2
  %24 = extractvalue [4 x <4 x float>] %20, 3
  %shuf_transpL66 = shufflevector <4 x float> %21, <4 x float> %23, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpL67 = shufflevector <4 x float> %22, <4 x float> %24, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH68 = shufflevector <4 x float> %21, <4 x float> %23, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH69 = shufflevector <4 x float> %22, <4 x float> %24, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpL70 = shufflevector <4 x float> %shuf_transpL66, <4 x float> %shuf_transpL67, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH71 = shufflevector <4 x float> %shuf_transpL66, <4 x float> %shuf_transpL67, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL72 = shufflevector <4 x float> %shuf_transpH68, <4 x float> %shuf_transpH69, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH73 = shufflevector <4 x float> %shuf_transpH68, <4 x float> %shuf_transpH69, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %25 = getelementptr <4 x float>* %2, i32 %extract
  %26 = getelementptr <4 x float>* %2, i32 %extract21
  %27 = getelementptr <4 x float>* %2, i32 %extract22
  %28 = getelementptr <4 x float>* %2, i32 %extract23
  store <4 x float> %shuf_transpL70, <4 x float>* %25
  store <4 x float> %shuf_transpH71, <4 x float>* %26
  store <4 x float> %shuf_transpL72, <4 x float>* %27
  store <4 x float> %shuf_transpH73, <4 x float>* %28
  ret void
}


; CHECK: @__fast_distancef_test
; CHECK-NOT: @_f_v.__vertical_fast_distance1f4
; CHECK: @__vertical_fast_distance1f4
; CHECK-NOT: @_f_v.__vertical_fast_distance1f4
; CHECK: ret void
define void @__fast_distancef_test(float* nocapture, float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %3 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %3 to <4 x float>*
  %load_arg4 = load <4 x float>* %ptrTypeCast, align 4
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast5 = bitcast float* %4 to <4 x float>*
  %load_arg26 = load <4 x float>* %ptrTypeCast5, align 4
  %5 = tail call <4 x float> @_f_v.__vertical_fast_distance1f4(<4 x float> %load_arg4, <4 x float> %load_arg26)
  %6 = getelementptr float* %2, i32 %gid
  %ptrTypeCast7 = bitcast float* %6 to <4 x float>*
  store <4 x float> %5, <4 x float>* %ptrTypeCast7, align 4
  ret void
}


; CHECK: @__fast_distancef2_test
; CHECK-NOT: @_f_v.__vertical_fast_distance2f4
; CHECK: @__vertical_fast_distance2f4
; CHECK-NOT: @_f_v.__vertical_fast_distance2f4
; CHECK: ret void
define void @__fast_distancef2_test(<2 x float>* nocapture, <2 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %3, i32 0
  %extract7 = extractelement <4 x i32> %3, i32 1
  %extract8 = extractelement <4 x i32> %3, i32 2
  %extract9 = extractelement <4 x i32> %3, i32 3
  %4 = getelementptr <2 x float>* %0, i32 %extract
  %5 = getelementptr <2 x float>* %0, i32 %extract7
  %6 = getelementptr <2 x float>* %0, i32 %extract8
  %7 = getelementptr <2 x float>* %0, i32 %extract9
  %8 = load <2 x float>* %4
  %9 = load <2 x float>* %5
  %10 = load <2 x float>* %6
  %11 = load <2 x float>* %7
  %extend_vec = shufflevector <2 x float> %8, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec10 = shufflevector <2 x float> %9, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec11 = shufflevector <2 x float> %10, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec12 = shufflevector <2 x float> %11, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec10, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec11, <4 x float> %extend_vec12, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge19 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %12 = getelementptr <2 x float>* %1, i32 %extract
  %13 = getelementptr <2 x float>* %1, i32 %extract7
  %14 = getelementptr <2 x float>* %1, i32 %extract8
  %15 = getelementptr <2 x float>* %1, i32 %extract9
  %16 = load <2 x float>* %12
  %17 = load <2 x float>* %13
  %18 = load <2 x float>* %14
  %19 = load <2 x float>* %15
  %extend_vec20 = shufflevector <2 x float> %16, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec21 = shufflevector <2 x float> %17, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec22 = shufflevector <2 x float> %18, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec23 = shufflevector <2 x float> %19, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %shuffle024 = shufflevector <4 x float> %extend_vec20, <4 x float> %extend_vec21, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle125 = shufflevector <4 x float> %extend_vec22, <4 x float> %extend_vec23, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge26 = shufflevector <4 x float> %shuffle024, <4 x float> %shuffle125, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge33 = shufflevector <4 x float> %shuffle024, <4 x float> %shuffle125, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [2 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val43 = insertvalue [2 x <4 x float>] %store.val, <4 x float> %shuffleMerge19, 1
  %store.val44 = insertvalue [2 x <4 x float>] undef, <4 x float> %shuffleMerge26, 0
  %store.val45 = insertvalue [2 x <4 x float>] %store.val44, <4 x float> %shuffleMerge33, 1
  %20 = tail call <4 x float> @_f_v.__vertical_fast_distance2f4([2 x <4 x float>] %store.val43, [2 x <4 x float>] %store.val45)
  %21 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %21 to <4 x float>*
  store <4 x float> %20, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__fast_distancef3_test
; CHECK-NOT: @_f_v.__vertical_fast_distance3f4
; CHECK: @__vertical_fast_distance3f4
; CHECK-NOT: @_f_v.__vertical_fast_distance3f4
; CHECK: ret void
define void @__fast_distancef3_test(<3 x float>* nocapture, <3 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %3, i32 0
  %extract11 = extractelement <4 x i32> %3, i32 1
  %extract12 = extractelement <4 x i32> %3, i32 2
  %extract13 = extractelement <4 x i32> %3, i32 3
  %4 = getelementptr <3 x float>* %0, i32 %extract
  %5 = getelementptr <3 x float>* %0, i32 %extract11
  %6 = getelementptr <3 x float>* %0, i32 %extract12
  %7 = getelementptr <3 x float>* %0, i32 %extract13
  %8 = load <3 x float>* %4
  %9 = load <3 x float>* %5
  %10 = load <3 x float>* %6
  %11 = load <3 x float>* %7
  %extend_vec = shufflevector <3 x float> %8, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec14 = shufflevector <3 x float> %9, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec15 = shufflevector <3 x float> %10, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec16 = shufflevector <3 x float> %11, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec14, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec15, <4 x float> %extend_vec16, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge23 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle028 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec14, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle129 = shufflevector <4 x float> %extend_vec15, <4 x float> %extend_vec16, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge30 = shufflevector <4 x float> %shuffle028, <4 x float> %shuffle129, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %12 = getelementptr <3 x float>* %1, i32 %extract
  %13 = getelementptr <3 x float>* %1, i32 %extract11
  %14 = getelementptr <3 x float>* %1, i32 %extract12
  %15 = getelementptr <3 x float>* %1, i32 %extract13
  %16 = load <3 x float>* %12
  %17 = load <3 x float>* %13
  %18 = load <3 x float>* %14
  %19 = load <3 x float>* %15
  %extend_vec31 = shufflevector <3 x float> %16, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec32 = shufflevector <3 x float> %17, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec33 = shufflevector <3 x float> %18, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec34 = shufflevector <3 x float> %19, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %shuffle035 = shufflevector <4 x float> %extend_vec31, <4 x float> %extend_vec32, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle136 = shufflevector <4 x float> %extend_vec33, <4 x float> %extend_vec34, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge37 = shufflevector <4 x float> %shuffle035, <4 x float> %shuffle136, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge44 = shufflevector <4 x float> %shuffle035, <4 x float> %shuffle136, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle049 = shufflevector <4 x float> %extend_vec31, <4 x float> %extend_vec32, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle150 = shufflevector <4 x float> %extend_vec33, <4 x float> %extend_vec34, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge51 = shufflevector <4 x float> %shuffle049, <4 x float> %shuffle150, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %store.val = insertvalue [3 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val73 = insertvalue [3 x <4 x float>] %store.val, <4 x float> %shuffleMerge23, 1
  %store.val74 = insertvalue [3 x <4 x float>] %store.val73, <4 x float> %shuffleMerge30, 2
  %store.val75 = insertvalue [3 x <4 x float>] undef, <4 x float> %shuffleMerge37, 0
  %store.val76 = insertvalue [3 x <4 x float>] %store.val75, <4 x float> %shuffleMerge44, 1
  %store.val77 = insertvalue [3 x <4 x float>] %store.val76, <4 x float> %shuffleMerge51, 2
  %20 = tail call <4 x float> @_f_v.__vertical_fast_distance3f4([3 x <4 x float>] %store.val74, [3 x <4 x float>] %store.val77)
  %21 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %21 to <4 x float>*
  store <4 x float> %20, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__fast_distancef4_test
; CHECK-NOT: @_f_v.__vertical_fast_distance4f4
; CHECK: @__vertical_fast_distance4f4
; CHECK-NOT: @_f_v.__vertical_fast_distance4f4
; CHECK: ret void
define void @__fast_distancef4_test(<4 x float>* nocapture, <4 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %3, i32 0
  %extract15 = extractelement <4 x i32> %3, i32 1
  %extract16 = extractelement <4 x i32> %3, i32 2
  %extract17 = extractelement <4 x i32> %3, i32 3
  %4 = getelementptr <4 x float>* %0, i32 %extract
  %5 = getelementptr <4 x float>* %0, i32 %extract15
  %6 = getelementptr <4 x float>* %0, i32 %extract16
  %7 = getelementptr <4 x float>* %0, i32 %extract17
  %8 = load <4 x float>* %4
  %9 = load <4 x float>* %5
  %10 = load <4 x float>* %6
  %11 = load <4 x float>* %7
  %shuffle0 = shufflevector <4 x float> %8, <4 x float> %9, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %10, <4 x float> %11, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge20 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle021 = shufflevector <4 x float> %8, <4 x float> %9, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle122 = shufflevector <4 x float> %10, <4 x float> %11, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge23 = shufflevector <4 x float> %shuffle021, <4 x float> %shuffle122, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge26 = shufflevector <4 x float> %shuffle021, <4 x float> %shuffle122, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %12 = getelementptr <4 x float>* %1, i32 %extract
  %13 = getelementptr <4 x float>* %1, i32 %extract15
  %14 = getelementptr <4 x float>* %1, i32 %extract16
  %15 = getelementptr <4 x float>* %1, i32 %extract17
  %16 = load <4 x float>* %12
  %17 = load <4 x float>* %13
  %18 = load <4 x float>* %14
  %19 = load <4 x float>* %15
  %shuffle027 = shufflevector <4 x float> %16, <4 x float> %17, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle128 = shufflevector <4 x float> %18, <4 x float> %19, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge29 = shufflevector <4 x float> %shuffle027, <4 x float> %shuffle128, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge32 = shufflevector <4 x float> %shuffle027, <4 x float> %shuffle128, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle033 = shufflevector <4 x float> %16, <4 x float> %17, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle134 = shufflevector <4 x float> %18, <4 x float> %19, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge35 = shufflevector <4 x float> %shuffle033, <4 x float> %shuffle134, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge38 = shufflevector <4 x float> %shuffle033, <4 x float> %shuffle134, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [4 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val53 = insertvalue [4 x <4 x float>] %store.val, <4 x float> %shuffleMerge20, 1
  %store.val54 = insertvalue [4 x <4 x float>] %store.val53, <4 x float> %shuffleMerge23, 2
  %store.val55 = insertvalue [4 x <4 x float>] %store.val54, <4 x float> %shuffleMerge26, 3
  %store.val56 = insertvalue [4 x <4 x float>] undef, <4 x float> %shuffleMerge29, 0
  %store.val57 = insertvalue [4 x <4 x float>] %store.val56, <4 x float> %shuffleMerge32, 1
  %store.val58 = insertvalue [4 x <4 x float>] %store.val57, <4 x float> %shuffleMerge35, 2
  %store.val59 = insertvalue [4 x <4 x float>] %store.val58, <4 x float> %shuffleMerge38, 3
  %20 = tail call <4 x float> @_f_v.__vertical_fast_distance4f4([4 x <4 x float>] %store.val55, [4 x <4 x float>] %store.val59)
  %21 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %21 to <4 x float>*
  store <4 x float> %20, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__distancef_test
; CHECK-NOT: @_f_v.__vertical_distance1f4
; CHECK: @__vertical_distance1f4
; CHECK-NOT: @_f_v.__vertical_distance1f4
; CHECK: ret void
define void @__distancef_test(float* nocapture, float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %3 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %3 to <4 x float>*
  %load_arg4 = load <4 x float>* %ptrTypeCast, align 4
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast5 = bitcast float* %4 to <4 x float>*
  %load_arg26 = load <4 x float>* %ptrTypeCast5, align 4
  %5 = tail call <4 x float> @_f_v.__vertical_distance1f4(<4 x float> %load_arg4, <4 x float> %load_arg26)
  %6 = getelementptr float* %2, i32 %gid
  %ptrTypeCast7 = bitcast float* %6 to <4 x float>*
  store <4 x float> %5, <4 x float>* %ptrTypeCast7, align 4
  ret void
}


; CHECK: @__lengthf_test
; CHECK-NOT: @_f_v.__vertical_length1f4
; CHECK: @__vertical_length1f4
; CHECK-NOT: @_f_v.__vertical_length1f4
; CHECK: ret void
define void @__lengthf_test(float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %2 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %2 to <4 x float>*
  %load_arg4 = load <4 x float>* %ptrTypeCast, align 4
  %3 = tail call <4 x float> @_f_v.__vertical_length1f4(<4 x float> %load_arg4)
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %ptrTypeCast5, align 4
  ret void
}


; CHECK: @__distancef2_test
; CHECK-NOT: @_f_v.__vertical_distance2f4
; CHECK: @__vertical_distance2f4
; CHECK-NOT: @_f_v.__vertical_distance2f4
; CHECK: ret void
define void @__distancef2_test(<2 x float>* nocapture, <2 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %3, i32 0
  %extract7 = extractelement <4 x i32> %3, i32 1
  %extract8 = extractelement <4 x i32> %3, i32 2
  %extract9 = extractelement <4 x i32> %3, i32 3
  %4 = getelementptr <2 x float>* %0, i32 %extract
  %5 = getelementptr <2 x float>* %0, i32 %extract7
  %6 = getelementptr <2 x float>* %0, i32 %extract8
  %7 = getelementptr <2 x float>* %0, i32 %extract9
  %8 = load <2 x float>* %4
  %9 = load <2 x float>* %5
  %10 = load <2 x float>* %6
  %11 = load <2 x float>* %7
  %extend_vec = shufflevector <2 x float> %8, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec10 = shufflevector <2 x float> %9, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec11 = shufflevector <2 x float> %10, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec12 = shufflevector <2 x float> %11, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec10, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec11, <4 x float> %extend_vec12, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge19 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %12 = getelementptr <2 x float>* %1, i32 %extract
  %13 = getelementptr <2 x float>* %1, i32 %extract7
  %14 = getelementptr <2 x float>* %1, i32 %extract8
  %15 = getelementptr <2 x float>* %1, i32 %extract9
  %16 = load <2 x float>* %12
  %17 = load <2 x float>* %13
  %18 = load <2 x float>* %14
  %19 = load <2 x float>* %15
  %extend_vec20 = shufflevector <2 x float> %16, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec21 = shufflevector <2 x float> %17, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec22 = shufflevector <2 x float> %18, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec23 = shufflevector <2 x float> %19, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %shuffle024 = shufflevector <4 x float> %extend_vec20, <4 x float> %extend_vec21, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle125 = shufflevector <4 x float> %extend_vec22, <4 x float> %extend_vec23, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge26 = shufflevector <4 x float> %shuffle024, <4 x float> %shuffle125, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge33 = shufflevector <4 x float> %shuffle024, <4 x float> %shuffle125, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [2 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val43 = insertvalue [2 x <4 x float>] %store.val, <4 x float> %shuffleMerge19, 1
  %store.val44 = insertvalue [2 x <4 x float>] undef, <4 x float> %shuffleMerge26, 0
  %store.val45 = insertvalue [2 x <4 x float>] %store.val44, <4 x float> %shuffleMerge33, 1
  %20 = tail call <4 x float> @_f_v.__vertical_distance2f4([2 x <4 x float>] %store.val43, [2 x <4 x float>] %store.val45)
  %21 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %21 to <4 x float>*
  store <4 x float> %20, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__lengthf2_test
; CHECK-NOT: @_f_v.__vertical_length2f4
; CHECK: @__vertical_length2f4
; CHECK-NOT: @_f_v.__vertical_length2f4
; CHECK: ret void
define void @__lengthf2_test(<2 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %2 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %2, i32 0
  %extract3 = extractelement <4 x i32> %2, i32 1
  %extract4 = extractelement <4 x i32> %2, i32 2
  %extract5 = extractelement <4 x i32> %2, i32 3
  %3 = getelementptr <2 x float>* %0, i32 %extract
  %4 = getelementptr <2 x float>* %0, i32 %extract3
  %5 = getelementptr <2 x float>* %0, i32 %extract4
  %6 = getelementptr <2 x float>* %0, i32 %extract5
  %7 = load <2 x float>* %3
  %8 = load <2 x float>* %4
  %9 = load <2 x float>* %5
  %10 = load <2 x float>* %6
  %extend_vec = shufflevector <2 x float> %7, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec6 = shufflevector <2 x float> %8, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec7 = shufflevector <2 x float> %9, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec8 = shufflevector <2 x float> %10, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec6, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec7, <4 x float> %extend_vec8, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge15 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [2 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val19 = insertvalue [2 x <4 x float>] %store.val, <4 x float> %shuffleMerge15, 1
  %11 = tail call <4 x float> @_f_v.__vertical_length2f4([2 x <4 x float>] %store.val19)
  %12 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %12 to <4 x float>*
  store <4 x float> %11, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__distancef3_test
; CHECK-NOT: @_f_v.__vertical_distance3f4
; CHECK: @__vertical_distance3f4
; CHECK-NOT: @_f_v.__vertical_distance3f4
; CHECK: ret void
define void @__distancef3_test(<3 x float>* nocapture, <3 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %3, i32 0
  %extract11 = extractelement <4 x i32> %3, i32 1
  %extract12 = extractelement <4 x i32> %3, i32 2
  %extract13 = extractelement <4 x i32> %3, i32 3
  %4 = getelementptr <3 x float>* %0, i32 %extract
  %5 = getelementptr <3 x float>* %0, i32 %extract11
  %6 = getelementptr <3 x float>* %0, i32 %extract12
  %7 = getelementptr <3 x float>* %0, i32 %extract13
  %8 = load <3 x float>* %4
  %9 = load <3 x float>* %5
  %10 = load <3 x float>* %6
  %11 = load <3 x float>* %7
  %extend_vec = shufflevector <3 x float> %8, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec14 = shufflevector <3 x float> %9, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec15 = shufflevector <3 x float> %10, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec16 = shufflevector <3 x float> %11, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec14, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec15, <4 x float> %extend_vec16, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge23 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle028 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec14, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle129 = shufflevector <4 x float> %extend_vec15, <4 x float> %extend_vec16, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge30 = shufflevector <4 x float> %shuffle028, <4 x float> %shuffle129, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %12 = getelementptr <3 x float>* %1, i32 %extract
  %13 = getelementptr <3 x float>* %1, i32 %extract11
  %14 = getelementptr <3 x float>* %1, i32 %extract12
  %15 = getelementptr <3 x float>* %1, i32 %extract13
  %16 = load <3 x float>* %12
  %17 = load <3 x float>* %13
  %18 = load <3 x float>* %14
  %19 = load <3 x float>* %15
  %extend_vec31 = shufflevector <3 x float> %16, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec32 = shufflevector <3 x float> %17, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec33 = shufflevector <3 x float> %18, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec34 = shufflevector <3 x float> %19, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %shuffle035 = shufflevector <4 x float> %extend_vec31, <4 x float> %extend_vec32, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle136 = shufflevector <4 x float> %extend_vec33, <4 x float> %extend_vec34, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge37 = shufflevector <4 x float> %shuffle035, <4 x float> %shuffle136, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge44 = shufflevector <4 x float> %shuffle035, <4 x float> %shuffle136, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle049 = shufflevector <4 x float> %extend_vec31, <4 x float> %extend_vec32, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle150 = shufflevector <4 x float> %extend_vec33, <4 x float> %extend_vec34, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge51 = shufflevector <4 x float> %shuffle049, <4 x float> %shuffle150, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %store.val = insertvalue [3 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val73 = insertvalue [3 x <4 x float>] %store.val, <4 x float> %shuffleMerge23, 1
  %store.val74 = insertvalue [3 x <4 x float>] %store.val73, <4 x float> %shuffleMerge30, 2
  %store.val75 = insertvalue [3 x <4 x float>] undef, <4 x float> %shuffleMerge37, 0
  %store.val76 = insertvalue [3 x <4 x float>] %store.val75, <4 x float> %shuffleMerge44, 1
  %store.val77 = insertvalue [3 x <4 x float>] %store.val76, <4 x float> %shuffleMerge51, 2
  %20 = tail call <4 x float> @_f_v.__vertical_distance3f4([3 x <4 x float>] %store.val74, [3 x <4 x float>] %store.val77)
  %21 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %21 to <4 x float>*
  store <4 x float> %20, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__lengthf3_test
; CHECK-NOT: @_f_v.__vertical_length3f4
; CHECK: @__vertical_length3f4
; CHECK-NOT: @_f_v.__vertical_length3f4
; CHECK: ret void
define void @__lengthf3_test(<3 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %2 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %2, i32 0
  %extract5 = extractelement <4 x i32> %2, i32 1
  %extract6 = extractelement <4 x i32> %2, i32 2
  %extract7 = extractelement <4 x i32> %2, i32 3
  %3 = getelementptr <3 x float>* %0, i32 %extract
  %4 = getelementptr <3 x float>* %0, i32 %extract5
  %5 = getelementptr <3 x float>* %0, i32 %extract6
  %6 = getelementptr <3 x float>* %0, i32 %extract7
  %7 = load <3 x float>* %3
  %8 = load <3 x float>* %4
  %9 = load <3 x float>* %5
  %10 = load <3 x float>* %6
  %extend_vec = shufflevector <3 x float> %7, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec8 = shufflevector <3 x float> %8, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec9 = shufflevector <3 x float> %9, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec10 = shufflevector <3 x float> %10, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec8, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec9, <4 x float> %extend_vec10, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge17 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle022 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec8, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle123 = shufflevector <4 x float> %extend_vec9, <4 x float> %extend_vec10, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge24 = shufflevector <4 x float> %shuffle022, <4 x float> %shuffle123, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %store.val = insertvalue [3 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val34 = insertvalue [3 x <4 x float>] %store.val, <4 x float> %shuffleMerge17, 1
  %store.val35 = insertvalue [3 x <4 x float>] %store.val34, <4 x float> %shuffleMerge24, 2
  %11 = tail call <4 x float> @_f_v.__vertical_length3f4([3 x <4 x float>] %store.val35)
  %12 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %12 to <4 x float>*
  store <4 x float> %11, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__distancef4_test
; CHECK-NOT: @_f_v.__vertical_distance4f4
; CHECK: @__vertical_distance4f4
; CHECK-NOT: @_f_v.__vertical_distance4f4
; CHECK: ret void
define void @__distancef4_test(<4 x float>* nocapture, <4 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %3, i32 0
  %extract15 = extractelement <4 x i32> %3, i32 1
  %extract16 = extractelement <4 x i32> %3, i32 2
  %extract17 = extractelement <4 x i32> %3, i32 3
  %4 = getelementptr <4 x float>* %0, i32 %extract
  %5 = getelementptr <4 x float>* %0, i32 %extract15
  %6 = getelementptr <4 x float>* %0, i32 %extract16
  %7 = getelementptr <4 x float>* %0, i32 %extract17
  %8 = load <4 x float>* %4
  %9 = load <4 x float>* %5
  %10 = load <4 x float>* %6
  %11 = load <4 x float>* %7
  %shuffle0 = shufflevector <4 x float> %8, <4 x float> %9, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %10, <4 x float> %11, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge20 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle021 = shufflevector <4 x float> %8, <4 x float> %9, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle122 = shufflevector <4 x float> %10, <4 x float> %11, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge23 = shufflevector <4 x float> %shuffle021, <4 x float> %shuffle122, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge26 = shufflevector <4 x float> %shuffle021, <4 x float> %shuffle122, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %12 = getelementptr <4 x float>* %1, i32 %extract
  %13 = getelementptr <4 x float>* %1, i32 %extract15
  %14 = getelementptr <4 x float>* %1, i32 %extract16
  %15 = getelementptr <4 x float>* %1, i32 %extract17
  %16 = load <4 x float>* %12
  %17 = load <4 x float>* %13
  %18 = load <4 x float>* %14
  %19 = load <4 x float>* %15
  %shuffle027 = shufflevector <4 x float> %16, <4 x float> %17, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle128 = shufflevector <4 x float> %18, <4 x float> %19, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge29 = shufflevector <4 x float> %shuffle027, <4 x float> %shuffle128, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge32 = shufflevector <4 x float> %shuffle027, <4 x float> %shuffle128, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle033 = shufflevector <4 x float> %16, <4 x float> %17, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle134 = shufflevector <4 x float> %18, <4 x float> %19, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge35 = shufflevector <4 x float> %shuffle033, <4 x float> %shuffle134, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge38 = shufflevector <4 x float> %shuffle033, <4 x float> %shuffle134, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [4 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val53 = insertvalue [4 x <4 x float>] %store.val, <4 x float> %shuffleMerge20, 1
  %store.val54 = insertvalue [4 x <4 x float>] %store.val53, <4 x float> %shuffleMerge23, 2
  %store.val55 = insertvalue [4 x <4 x float>] %store.val54, <4 x float> %shuffleMerge26, 3
  %store.val56 = insertvalue [4 x <4 x float>] undef, <4 x float> %shuffleMerge29, 0
  %store.val57 = insertvalue [4 x <4 x float>] %store.val56, <4 x float> %shuffleMerge32, 1
  %store.val58 = insertvalue [4 x <4 x float>] %store.val57, <4 x float> %shuffleMerge35, 2
  %store.val59 = insertvalue [4 x <4 x float>] %store.val58, <4 x float> %shuffleMerge38, 3
  %20 = tail call <4 x float> @_f_v.__vertical_distance4f4([4 x <4 x float>] %store.val55, [4 x <4 x float>] %store.val59)
  %21 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %21 to <4 x float>*
  store <4 x float> %20, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__lengthf4_test
; CHECK-NOT: @_f_v.__vertical_length4f4
; CHECK: @__vertical_length4f4
; CHECK-NOT: @_f_v.__vertical_length4f4
; CHECK: ret void
define void @__lengthf4_test(<4 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %2 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %2, i32 0
  %extract7 = extractelement <4 x i32> %2, i32 1
  %extract8 = extractelement <4 x i32> %2, i32 2
  %extract9 = extractelement <4 x i32> %2, i32 3
  %3 = getelementptr <4 x float>* %0, i32 %extract
  %4 = getelementptr <4 x float>* %0, i32 %extract7
  %5 = getelementptr <4 x float>* %0, i32 %extract8
  %6 = getelementptr <4 x float>* %0, i32 %extract9
  %7 = load <4 x float>* %3
  %8 = load <4 x float>* %4
  %9 = load <4 x float>* %5
  %10 = load <4 x float>* %6
  %shuffle0 = shufflevector <4 x float> %7, <4 x float> %8, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %9, <4 x float> %10, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge12 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle013 = shufflevector <4 x float> %7, <4 x float> %8, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle114 = shufflevector <4 x float> %9, <4 x float> %10, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge15 = shufflevector <4 x float> %shuffle013, <4 x float> %shuffle114, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge18 = shufflevector <4 x float> %shuffle013, <4 x float> %shuffle114, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [4 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val25 = insertvalue [4 x <4 x float>] %store.val, <4 x float> %shuffleMerge12, 1
  %store.val26 = insertvalue [4 x <4 x float>] %store.val25, <4 x float> %shuffleMerge15, 2
  %store.val27 = insertvalue [4 x <4 x float>] %store.val26, <4 x float> %shuffleMerge18, 3
  %11 = tail call <4 x float> @_f_v.__vertical_length4f4([4 x <4 x float>] %store.val27)
  %12 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %12 to <4 x float>*
  store <4 x float> %11, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__dotf_test
; CHECK-NOT: @_f_v.__vertical_dot1f4
; CHECK: @__vertical_dot1f4
; CHECK-NOT: @_f_v.__vertical_dot1f4
; CHECK: ret void
define void @__dotf_test(float* nocapture, float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %3 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %3 to <4 x float>*
  %load_arg4 = load <4 x float>* %ptrTypeCast, align 4
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast5 = bitcast float* %4 to <4 x float>*
  %load_arg26 = load <4 x float>* %ptrTypeCast5, align 4
  %5 = tail call <4 x float> @_f_v.__vertical_dot1f4(<4 x float> %load_arg4, <4 x float> %load_arg26)
  %6 = getelementptr float* %2, i32 %gid
  %ptrTypeCast7 = bitcast float* %6 to <4 x float>*
  store <4 x float> %5, <4 x float>* %ptrTypeCast7, align 4
  ret void
}


; CHECK: @__dotf2_test
; CHECK-NOT: @_f_v.__vertical_dot2f4
; CHECK: @__vertical_dot2f4
; CHECK-NOT: @_f_v.__vertical_dot2f4
; CHECK: ret void
define void @__dotf2_test(<2 x float>* nocapture, <2 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %3, i32 0
  %extract7 = extractelement <4 x i32> %3, i32 1
  %extract8 = extractelement <4 x i32> %3, i32 2
  %extract9 = extractelement <4 x i32> %3, i32 3
  %4 = getelementptr <2 x float>* %0, i32 %extract
  %5 = getelementptr <2 x float>* %0, i32 %extract7
  %6 = getelementptr <2 x float>* %0, i32 %extract8
  %7 = getelementptr <2 x float>* %0, i32 %extract9
  %8 = load <2 x float>* %4
  %9 = load <2 x float>* %5
  %10 = load <2 x float>* %6
  %11 = load <2 x float>* %7
  %extend_vec = shufflevector <2 x float> %8, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec10 = shufflevector <2 x float> %9, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec11 = shufflevector <2 x float> %10, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec12 = shufflevector <2 x float> %11, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec10, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec11, <4 x float> %extend_vec12, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge19 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %12 = getelementptr <2 x float>* %1, i32 %extract
  %13 = getelementptr <2 x float>* %1, i32 %extract7
  %14 = getelementptr <2 x float>* %1, i32 %extract8
  %15 = getelementptr <2 x float>* %1, i32 %extract9
  %16 = load <2 x float>* %12
  %17 = load <2 x float>* %13
  %18 = load <2 x float>* %14
  %19 = load <2 x float>* %15
  %extend_vec20 = shufflevector <2 x float> %16, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec21 = shufflevector <2 x float> %17, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec22 = shufflevector <2 x float> %18, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec23 = shufflevector <2 x float> %19, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %shuffle024 = shufflevector <4 x float> %extend_vec20, <4 x float> %extend_vec21, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle125 = shufflevector <4 x float> %extend_vec22, <4 x float> %extend_vec23, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge26 = shufflevector <4 x float> %shuffle024, <4 x float> %shuffle125, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge33 = shufflevector <4 x float> %shuffle024, <4 x float> %shuffle125, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [2 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val43 = insertvalue [2 x <4 x float>] %store.val, <4 x float> %shuffleMerge19, 1
  %store.val44 = insertvalue [2 x <4 x float>] undef, <4 x float> %shuffleMerge26, 0
  %store.val45 = insertvalue [2 x <4 x float>] %store.val44, <4 x float> %shuffleMerge33, 1
  %20 = tail call <4 x float> @_f_v.__vertical_dot2f4([2 x <4 x float>] %store.val43, [2 x <4 x float>] %store.val45)
  %21 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %21 to <4 x float>*
  store <4 x float> %20, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__dotf3_test
; CHECK-NOT: @_f_v.__vertical_dot3f4
; CHECK: @__vertical_dot3f4
; CHECK-NOT: @_f_v.__vertical_dot3f4
; CHECK: ret void
define void @__dotf3_test(<3 x float>* nocapture, <3 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %3, i32 0
  %extract11 = extractelement <4 x i32> %3, i32 1
  %extract12 = extractelement <4 x i32> %3, i32 2
  %extract13 = extractelement <4 x i32> %3, i32 3
  %4 = getelementptr <3 x float>* %0, i32 %extract
  %5 = getelementptr <3 x float>* %0, i32 %extract11
  %6 = getelementptr <3 x float>* %0, i32 %extract12
  %7 = getelementptr <3 x float>* %0, i32 %extract13
  %8 = load <3 x float>* %4
  %9 = load <3 x float>* %5
  %10 = load <3 x float>* %6
  %11 = load <3 x float>* %7
  %extend_vec = shufflevector <3 x float> %8, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec14 = shufflevector <3 x float> %9, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec15 = shufflevector <3 x float> %10, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec16 = shufflevector <3 x float> %11, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec14, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec15, <4 x float> %extend_vec16, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge23 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle028 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec14, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle129 = shufflevector <4 x float> %extend_vec15, <4 x float> %extend_vec16, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge30 = shufflevector <4 x float> %shuffle028, <4 x float> %shuffle129, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %12 = getelementptr <3 x float>* %1, i32 %extract
  %13 = getelementptr <3 x float>* %1, i32 %extract11
  %14 = getelementptr <3 x float>* %1, i32 %extract12
  %15 = getelementptr <3 x float>* %1, i32 %extract13
  %16 = load <3 x float>* %12
  %17 = load <3 x float>* %13
  %18 = load <3 x float>* %14
  %19 = load <3 x float>* %15
  %extend_vec31 = shufflevector <3 x float> %16, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec32 = shufflevector <3 x float> %17, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec33 = shufflevector <3 x float> %18, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec34 = shufflevector <3 x float> %19, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %shuffle035 = shufflevector <4 x float> %extend_vec31, <4 x float> %extend_vec32, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle136 = shufflevector <4 x float> %extend_vec33, <4 x float> %extend_vec34, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge37 = shufflevector <4 x float> %shuffle035, <4 x float> %shuffle136, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge44 = shufflevector <4 x float> %shuffle035, <4 x float> %shuffle136, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle049 = shufflevector <4 x float> %extend_vec31, <4 x float> %extend_vec32, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle150 = shufflevector <4 x float> %extend_vec33, <4 x float> %extend_vec34, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge51 = shufflevector <4 x float> %shuffle049, <4 x float> %shuffle150, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %store.val = insertvalue [3 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val73 = insertvalue [3 x <4 x float>] %store.val, <4 x float> %shuffleMerge23, 1
  %store.val74 = insertvalue [3 x <4 x float>] %store.val73, <4 x float> %shuffleMerge30, 2
  %store.val75 = insertvalue [3 x <4 x float>] undef, <4 x float> %shuffleMerge37, 0
  %store.val76 = insertvalue [3 x <4 x float>] %store.val75, <4 x float> %shuffleMerge44, 1
  %store.val77 = insertvalue [3 x <4 x float>] %store.val76, <4 x float> %shuffleMerge51, 2
  %20 = tail call <4 x float> @_f_v.__vertical_dot3f4([3 x <4 x float>] %store.val74, [3 x <4 x float>] %store.val77)
  %21 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %21 to <4 x float>*
  store <4 x float> %20, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__dotf4_test
; CHECK-NOT: @_f_v.__vertical_dot4f4
; CHECK: @__vertical_dot4f4
; CHECK-NOT: @_f_v.__vertical_dot4f4
; CHECK: ret void
define void @__dotf4_test(<4 x float>* nocapture, <4 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %3, i32 0
  %extract15 = extractelement <4 x i32> %3, i32 1
  %extract16 = extractelement <4 x i32> %3, i32 2
  %extract17 = extractelement <4 x i32> %3, i32 3
  %4 = getelementptr <4 x float>* %0, i32 %extract
  %5 = getelementptr <4 x float>* %0, i32 %extract15
  %6 = getelementptr <4 x float>* %0, i32 %extract16
  %7 = getelementptr <4 x float>* %0, i32 %extract17
  %8 = load <4 x float>* %4
  %9 = load <4 x float>* %5
  %10 = load <4 x float>* %6
  %11 = load <4 x float>* %7
  %shuffle0 = shufflevector <4 x float> %8, <4 x float> %9, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %10, <4 x float> %11, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge20 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle021 = shufflevector <4 x float> %8, <4 x float> %9, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle122 = shufflevector <4 x float> %10, <4 x float> %11, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge23 = shufflevector <4 x float> %shuffle021, <4 x float> %shuffle122, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge26 = shufflevector <4 x float> %shuffle021, <4 x float> %shuffle122, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %12 = getelementptr <4 x float>* %1, i32 %extract
  %13 = getelementptr <4 x float>* %1, i32 %extract15
  %14 = getelementptr <4 x float>* %1, i32 %extract16
  %15 = getelementptr <4 x float>* %1, i32 %extract17
  %16 = load <4 x float>* %12
  %17 = load <4 x float>* %13
  %18 = load <4 x float>* %14
  %19 = load <4 x float>* %15
  %shuffle027 = shufflevector <4 x float> %16, <4 x float> %17, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle128 = shufflevector <4 x float> %18, <4 x float> %19, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge29 = shufflevector <4 x float> %shuffle027, <4 x float> %shuffle128, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge32 = shufflevector <4 x float> %shuffle027, <4 x float> %shuffle128, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle033 = shufflevector <4 x float> %16, <4 x float> %17, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle134 = shufflevector <4 x float> %18, <4 x float> %19, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge35 = shufflevector <4 x float> %shuffle033, <4 x float> %shuffle134, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge38 = shufflevector <4 x float> %shuffle033, <4 x float> %shuffle134, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [4 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val53 = insertvalue [4 x <4 x float>] %store.val, <4 x float> %shuffleMerge20, 1
  %store.val54 = insertvalue [4 x <4 x float>] %store.val53, <4 x float> %shuffleMerge23, 2
  %store.val55 = insertvalue [4 x <4 x float>] %store.val54, <4 x float> %shuffleMerge26, 3
  %store.val56 = insertvalue [4 x <4 x float>] undef, <4 x float> %shuffleMerge29, 0
  %store.val57 = insertvalue [4 x <4 x float>] %store.val56, <4 x float> %shuffleMerge32, 1
  %store.val58 = insertvalue [4 x <4 x float>] %store.val57, <4 x float> %shuffleMerge35, 2
  %store.val59 = insertvalue [4 x <4 x float>] %store.val58, <4 x float> %shuffleMerge38, 3
  %20 = tail call <4 x float> @_f_v.__vertical_dot4f4([4 x <4 x float>] %store.val55, [4 x <4 x float>] %store.val59)
  %21 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %21 to <4 x float>*
  store <4 x float> %20, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__fast_lengthf_test
; CHECK-NOT: @_f_v.__vertical_fast_length1f4
; CHECK: @__vertical_fast_length1f4
; CHECK-NOT: @_f_v.__vertical_fast_length1f4
; CHECK: ret void
define void @__fast_lengthf_test(float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %2 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %2 to <4 x float>*
  %load_arg4 = load <4 x float>* %ptrTypeCast, align 4
  %3 = tail call <4 x float> @_f_v.__vertical_fast_length1f4(<4 x float> %load_arg4)
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %ptrTypeCast5, align 4
  ret void
}


; CHECK: @__fast_lengthf2_test
; CHECK-NOT: @_f_v.__vertical_fast_length2f4
; CHECK: @__vertical_fast_length2f4
; CHECK-NOT: @_f_v.__vertical_fast_length2f4
; CHECK: ret void
define void @__fast_lengthf2_test(<2 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %2 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %2, i32 0
  %extract3 = extractelement <4 x i32> %2, i32 1
  %extract4 = extractelement <4 x i32> %2, i32 2
  %extract5 = extractelement <4 x i32> %2, i32 3
  %3 = getelementptr <2 x float>* %0, i32 %extract
  %4 = getelementptr <2 x float>* %0, i32 %extract3
  %5 = getelementptr <2 x float>* %0, i32 %extract4
  %6 = getelementptr <2 x float>* %0, i32 %extract5
  %7 = load <2 x float>* %3
  %8 = load <2 x float>* %4
  %9 = load <2 x float>* %5
  %10 = load <2 x float>* %6
  %extend_vec = shufflevector <2 x float> %7, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec6 = shufflevector <2 x float> %8, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec7 = shufflevector <2 x float> %9, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec8 = shufflevector <2 x float> %10, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec6, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec7, <4 x float> %extend_vec8, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge15 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [2 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val19 = insertvalue [2 x <4 x float>] %store.val, <4 x float> %shuffleMerge15, 1
  %11 = tail call <4 x float> @_f_v.__vertical_fast_length2f4([2 x <4 x float>] %store.val19)
  %12 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %12 to <4 x float>*
  store <4 x float> %11, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__fast_lengthf3_test
; CHECK-NOT: @_f_v.__vertical_fast_length3f4
; CHECK: @__vertical_fast_length3f4
; CHECK-NOT: @_f_v.__vertical_fast_length3f4
; CHECK: ret void
define void @__fast_lengthf3_test(<3 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %2 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %2, i32 0
  %extract5 = extractelement <4 x i32> %2, i32 1
  %extract6 = extractelement <4 x i32> %2, i32 2
  %extract7 = extractelement <4 x i32> %2, i32 3
  %3 = getelementptr <3 x float>* %0, i32 %extract
  %4 = getelementptr <3 x float>* %0, i32 %extract5
  %5 = getelementptr <3 x float>* %0, i32 %extract6
  %6 = getelementptr <3 x float>* %0, i32 %extract7
  %7 = load <3 x float>* %3
  %8 = load <3 x float>* %4
  %9 = load <3 x float>* %5
  %10 = load <3 x float>* %6
  %extend_vec = shufflevector <3 x float> %7, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec8 = shufflevector <3 x float> %8, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec9 = shufflevector <3 x float> %9, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec10 = shufflevector <3 x float> %10, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec8, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec9, <4 x float> %extend_vec10, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge17 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle022 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec8, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle123 = shufflevector <4 x float> %extend_vec9, <4 x float> %extend_vec10, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge24 = shufflevector <4 x float> %shuffle022, <4 x float> %shuffle123, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %store.val = insertvalue [3 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val34 = insertvalue [3 x <4 x float>] %store.val, <4 x float> %shuffleMerge17, 1
  %store.val35 = insertvalue [3 x <4 x float>] %store.val34, <4 x float> %shuffleMerge24, 2
  %11 = tail call <4 x float> @_f_v.__vertical_fast_length3f4([3 x <4 x float>] %store.val35)
  %12 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %12 to <4 x float>*
  store <4 x float> %11, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__fast_lengthf4_test
; CHECK-NOT: @_f_v.__vertical_fast_length4f4
; CHECK: @__vertical_fast_length4f4
; CHECK-NOT: @_f_v.__vertical_fast_length4f4
; CHECK: ret void
define void @__fast_lengthf4_test(<4 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %2 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %2, i32 0
  %extract7 = extractelement <4 x i32> %2, i32 1
  %extract8 = extractelement <4 x i32> %2, i32 2
  %extract9 = extractelement <4 x i32> %2, i32 3
  %3 = getelementptr <4 x float>* %0, i32 %extract
  %4 = getelementptr <4 x float>* %0, i32 %extract7
  %5 = getelementptr <4 x float>* %0, i32 %extract8
  %6 = getelementptr <4 x float>* %0, i32 %extract9
  %7 = load <4 x float>* %3
  %8 = load <4 x float>* %4
  %9 = load <4 x float>* %5
  %10 = load <4 x float>* %6
  %shuffle0 = shufflevector <4 x float> %7, <4 x float> %8, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %9, <4 x float> %10, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge12 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle013 = shufflevector <4 x float> %7, <4 x float> %8, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle114 = shufflevector <4 x float> %9, <4 x float> %10, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge15 = shufflevector <4 x float> %shuffle013, <4 x float> %shuffle114, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge18 = shufflevector <4 x float> %shuffle013, <4 x float> %shuffle114, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [4 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val25 = insertvalue [4 x <4 x float>] %store.val, <4 x float> %shuffleMerge12, 1
  %store.val26 = insertvalue [4 x <4 x float>] %store.val25, <4 x float> %shuffleMerge15, 2
  %store.val27 = insertvalue [4 x <4 x float>] %store.val26, <4 x float> %shuffleMerge18, 3
  %11 = tail call <4 x float> @_f_v.__vertical_fast_length4f4([4 x <4 x float>] %store.val27)
  %12 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %12 to <4 x float>*
  store <4 x float> %11, <4 x float>* %ptrTypeCast, align 4
  ret void
}


; CHECK: @__fast_normalizef_test
; CHECK-NOT: @_f_v.__vertical_fast_normalize1f4
; CHECK: @__vertical_fast_normalize1f4
; CHECK-NOT: @_f_v.__vertical_fast_normalize1f4
; CHECK: ret void
define void @__fast_normalizef_test(float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %2 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %2 to <4 x float>*
  %load_arg4 = load <4 x float>* %ptrTypeCast, align 4
  %3 = tail call <4 x float> @_f_v.__vertical_fast_normalize1f4(<4 x float> %load_arg4)
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %ptrTypeCast5, align 4
  ret void
}


; CHECK: @__fast_normalizef2_test
; CHECK-NOT: @_f_v.__vertical_fast_normalize2f4
; CHECK: @__vertical_fast_normalize2f4
; CHECK-NOT: @_f_v.__vertical_fast_normalize2f4
; CHECK: ret void
define void @__fast_normalizef2_test(<2 x float>* nocapture, <2 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %2 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %2, i32 0
  %extract5 = extractelement <4 x i32> %2, i32 1
  %extract6 = extractelement <4 x i32> %2, i32 2
  %extract7 = extractelement <4 x i32> %2, i32 3
  %3 = getelementptr <2 x float>* %0, i32 %extract
  %4 = getelementptr <2 x float>* %0, i32 %extract5
  %5 = getelementptr <2 x float>* %0, i32 %extract6
  %6 = getelementptr <2 x float>* %0, i32 %extract7
  %7 = load <2 x float>* %3
  %8 = load <2 x float>* %4
  %9 = load <2 x float>* %5
  %10 = load <2 x float>* %6
  %extend_vec = shufflevector <2 x float> %7, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec8 = shufflevector <2 x float> %8, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec9 = shufflevector <2 x float> %9, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec10 = shufflevector <2 x float> %10, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec8, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec9, <4 x float> %extend_vec10, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge17 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [2 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val21 = insertvalue [2 x <4 x float>] %store.val, <4 x float> %shuffleMerge17, 1
  %11 = tail call [2 x <4 x float>] @_f_v.__vertical_fast_normalize2f4([2 x <4 x float>] %store.val21)
  %12 = extractvalue [2 x <4 x float>] %11, 0
  %13 = extractvalue [2 x <4 x float>] %11, 1
  %shuf_transpL22 = shufflevector <4 x float> %12, <4 x float> %13, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH23 = shufflevector <4 x float> %12, <4 x float> %13, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %breakdown24 = shufflevector <4 x float> %shuf_transpL22, <4 x float> undef, <2 x i32> <i32 0, i32 1>
  %breakdown25 = shufflevector <4 x float> %shuf_transpH23, <4 x float> undef, <2 x i32> <i32 0, i32 1>
  %breakdown26 = shufflevector <4 x float> %shuf_transpL22, <4 x float> undef, <2 x i32> <i32 2, i32 3>
  %breakdown27 = shufflevector <4 x float> %shuf_transpH23, <4 x float> undef, <2 x i32> <i32 2, i32 3>
  %14 = getelementptr <2 x float>* %1, i32 %extract
  %15 = getelementptr <2 x float>* %1, i32 %extract5
  %16 = getelementptr <2 x float>* %1, i32 %extract6
  %17 = getelementptr <2 x float>* %1, i32 %extract7
  store <2 x float> %breakdown24, <2 x float>* %14
  store <2 x float> %breakdown25, <2 x float>* %15
  store <2 x float> %breakdown26, <2 x float>* %16
  store <2 x float> %breakdown27, <2 x float>* %17
  ret void
}


; CHECK: @__fast_normalizef3_test
; CHECK-NOT: @_f_v.__vertical_fast_normalize3f4
; CHECK: @__vertical_fast_normalize3f4
; CHECK-NOT: @_f_v.__vertical_fast_normalize3f4
; CHECK: ret void
define void @__fast_normalizef3_test(<3 x float>* nocapture, <3 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %2 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %2, i32 0
  %extract9 = extractelement <4 x i32> %2, i32 1
  %extract10 = extractelement <4 x i32> %2, i32 2
  %extract11 = extractelement <4 x i32> %2, i32 3
  %3 = getelementptr <3 x float>* %0, i32 %extract
  %4 = getelementptr <3 x float>* %0, i32 %extract9
  %5 = getelementptr <3 x float>* %0, i32 %extract10
  %6 = getelementptr <3 x float>* %0, i32 %extract11
  %7 = load <3 x float>* %3
  %8 = load <3 x float>* %4
  %9 = load <3 x float>* %5
  %10 = load <3 x float>* %6
  %extend_vec = shufflevector <3 x float> %7, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec12 = shufflevector <3 x float> %8, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec13 = shufflevector <3 x float> %9, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec14 = shufflevector <3 x float> %10, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec12, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec13, <4 x float> %extend_vec14, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge21 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle026 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec12, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle127 = shufflevector <4 x float> %extend_vec13, <4 x float> %extend_vec14, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge28 = shufflevector <4 x float> %shuffle026, <4 x float> %shuffle127, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %store.val = insertvalue [3 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val38 = insertvalue [3 x <4 x float>] %store.val, <4 x float> %shuffleMerge21, 1
  %store.val39 = insertvalue [3 x <4 x float>] %store.val38, <4 x float> %shuffleMerge28, 2
  %11 = tail call [3 x <4 x float>] @_f_v.__vertical_fast_normalize3f4([3 x <4 x float>] %store.val39)
  %12 = extractvalue [3 x <4 x float>] %11, 0
  %13 = extractvalue [3 x <4 x float>] %11, 1
  %14 = extractvalue [3 x <4 x float>] %11, 2
  %shuf_transpL40 = shufflevector <4 x float> %12, <4 x float> %14, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH42 = shufflevector <4 x float> %12, <4 x float> %14, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH43 = shufflevector <4 x float> %13, <4 x float> undef, <4 x i32> <i32 2, i32 3, i32 undef, i32 undef>
  %shuf_transpL44 = shufflevector <4 x float> %shuf_transpL40, <4 x float> %13, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH45 = shufflevector <4 x float> %shuf_transpL40, <4 x float> %13, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL46 = shufflevector <4 x float> %shuf_transpH42, <4 x float> %shuf_transpH43, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH47 = shufflevector <4 x float> %shuf_transpH42, <4 x float> %shuf_transpH43, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %breakdown48 = shufflevector <4 x float> %shuf_transpL44, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown49 = shufflevector <4 x float> %shuf_transpH45, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown50 = shufflevector <4 x float> %shuf_transpL46, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown51 = shufflevector <4 x float> %shuf_transpH47, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %15 = getelementptr <3 x float>* %1, i32 %extract
  %16 = getelementptr <3 x float>* %1, i32 %extract9
  %17 = getelementptr <3 x float>* %1, i32 %extract10
  %18 = getelementptr <3 x float>* %1, i32 %extract11
  store <3 x float> %breakdown48, <3 x float>* %15
  store <3 x float> %breakdown49, <3 x float>* %16
  store <3 x float> %breakdown50, <3 x float>* %17
  store <3 x float> %breakdown51, <3 x float>* %18
  ret void
}


; CHECK: @__fast_normalizef4_test
; CHECK-NOT: @_f_v.__vertical_fast_normalize4f4
; CHECK: @__vertical_fast_normalize4f4
; CHECK-NOT: @_f_v.__vertical_fast_normalize4f4
; CHECK: ret void
define void @__fast_normalizef4_test(<4 x float>* nocapture, <4 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %2 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %2, i32 0
  %extract13 = extractelement <4 x i32> %2, i32 1
  %extract14 = extractelement <4 x i32> %2, i32 2
  %extract15 = extractelement <4 x i32> %2, i32 3
  %3 = getelementptr <4 x float>* %0, i32 %extract
  %4 = getelementptr <4 x float>* %0, i32 %extract13
  %5 = getelementptr <4 x float>* %0, i32 %extract14
  %6 = getelementptr <4 x float>* %0, i32 %extract15
  %7 = load <4 x float>* %3
  %8 = load <4 x float>* %4
  %9 = load <4 x float>* %5
  %10 = load <4 x float>* %6
  %shuffle0 = shufflevector <4 x float> %7, <4 x float> %8, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %9, <4 x float> %10, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge18 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle019 = shufflevector <4 x float> %7, <4 x float> %8, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle120 = shufflevector <4 x float> %9, <4 x float> %10, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge21 = shufflevector <4 x float> %shuffle019, <4 x float> %shuffle120, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge24 = shufflevector <4 x float> %shuffle019, <4 x float> %shuffle120, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [4 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val31 = insertvalue [4 x <4 x float>] %store.val, <4 x float> %shuffleMerge18, 1
  %store.val32 = insertvalue [4 x <4 x float>] %store.val31, <4 x float> %shuffleMerge21, 2
  %store.val33 = insertvalue [4 x <4 x float>] %store.val32, <4 x float> %shuffleMerge24, 3
  %11 = tail call [4 x <4 x float>] @_f_v.__vertical_fast_normalize4f4([4 x <4 x float>] %store.val33)
  %12 = extractvalue [4 x <4 x float>] %11, 0
  %13 = extractvalue [4 x <4 x float>] %11, 1
  %14 = extractvalue [4 x <4 x float>] %11, 2
  %15 = extractvalue [4 x <4 x float>] %11, 3
  %shuf_transpL34 = shufflevector <4 x float> %12, <4 x float> %14, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpL35 = shufflevector <4 x float> %13, <4 x float> %15, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH36 = shufflevector <4 x float> %12, <4 x float> %14, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH37 = shufflevector <4 x float> %13, <4 x float> %15, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpL38 = shufflevector <4 x float> %shuf_transpL34, <4 x float> %shuf_transpL35, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH39 = shufflevector <4 x float> %shuf_transpL34, <4 x float> %shuf_transpL35, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL40 = shufflevector <4 x float> %shuf_transpH36, <4 x float> %shuf_transpH37, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH41 = shufflevector <4 x float> %shuf_transpH36, <4 x float> %shuf_transpH37, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %16 = getelementptr <4 x float>* %1, i32 %extract
  %17 = getelementptr <4 x float>* %1, i32 %extract13
  %18 = getelementptr <4 x float>* %1, i32 %extract14
  %19 = getelementptr <4 x float>* %1, i32 %extract15
  store <4 x float> %shuf_transpL38, <4 x float>* %16
  store <4 x float> %shuf_transpH39, <4 x float>* %17
  store <4 x float> %shuf_transpL40, <4 x float>* %18
  store <4 x float> %shuf_transpH41, <4 x float>* %19
  ret void
}


; CHECK: @__normalizef_test
; CHECK-NOT: @_f_v.__vertical_normalize1f4
; CHECK: @__vertical_normalize1f4
; CHECK-NOT: @_f_v.__vertical_normalize1f4
; CHECK: ret void
define void @__normalizef_test(float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %2 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %2 to <4 x float>*
  %load_arg4 = load <4 x float>* %ptrTypeCast, align 4
  %3 = tail call <4 x float> @_f_v.__vertical_normalize1f4(<4 x float> %load_arg4)
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %ptrTypeCast5, align 4
  ret void
}


; CHECK: @__normalizef2_test
; CHECK-NOT: @_f_v.__vertical_normalize2f4
; CHECK: @__vertical_normalize2f4
; CHECK-NOT: @_f_v.__vertical_normalize2f4
; CHECK: ret void
define void @__normalizef2_test(<2 x float>* nocapture, <2 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %2 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %2, i32 0
  %extract5 = extractelement <4 x i32> %2, i32 1
  %extract6 = extractelement <4 x i32> %2, i32 2
  %extract7 = extractelement <4 x i32> %2, i32 3
  %3 = getelementptr <2 x float>* %0, i32 %extract
  %4 = getelementptr <2 x float>* %0, i32 %extract5
  %5 = getelementptr <2 x float>* %0, i32 %extract6
  %6 = getelementptr <2 x float>* %0, i32 %extract7
  %7 = load <2 x float>* %3
  %8 = load <2 x float>* %4
  %9 = load <2 x float>* %5
  %10 = load <2 x float>* %6
  %extend_vec = shufflevector <2 x float> %7, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec8 = shufflevector <2 x float> %8, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec9 = shufflevector <2 x float> %9, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %extend_vec10 = shufflevector <2 x float> %10, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec8, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec9, <4 x float> %extend_vec10, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge17 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [2 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val21 = insertvalue [2 x <4 x float>] %store.val, <4 x float> %shuffleMerge17, 1
  %11 = tail call [2 x <4 x float>] @_f_v.__vertical_normalize2f4([2 x <4 x float>] %store.val21)
  %12 = extractvalue [2 x <4 x float>] %11, 0
  %13 = extractvalue [2 x <4 x float>] %11, 1
  %shuf_transpL22 = shufflevector <4 x float> %12, <4 x float> %13, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH23 = shufflevector <4 x float> %12, <4 x float> %13, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %breakdown24 = shufflevector <4 x float> %shuf_transpL22, <4 x float> undef, <2 x i32> <i32 0, i32 1>
  %breakdown25 = shufflevector <4 x float> %shuf_transpH23, <4 x float> undef, <2 x i32> <i32 0, i32 1>
  %breakdown26 = shufflevector <4 x float> %shuf_transpL22, <4 x float> undef, <2 x i32> <i32 2, i32 3>
  %breakdown27 = shufflevector <4 x float> %shuf_transpH23, <4 x float> undef, <2 x i32> <i32 2, i32 3>
  %14 = getelementptr <2 x float>* %1, i32 %extract
  %15 = getelementptr <2 x float>* %1, i32 %extract5
  %16 = getelementptr <2 x float>* %1, i32 %extract6
  %17 = getelementptr <2 x float>* %1, i32 %extract7
  store <2 x float> %breakdown24, <2 x float>* %14
  store <2 x float> %breakdown25, <2 x float>* %15
  store <2 x float> %breakdown26, <2 x float>* %16
  store <2 x float> %breakdown27, <2 x float>* %17
  ret void
}


; CHECK: @__normalizef3_test
; CHECK-NOT: @_f_v.__vertical_normalize3f4
; CHECK: @__vertical_normalize3f4
; CHECK-NOT: @_f_v.__vertical_normalize3f4
; CHECK: ret void
define void @__normalizef3_test(<3 x float>* nocapture, <3 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %2 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %2, i32 0
  %extract9 = extractelement <4 x i32> %2, i32 1
  %extract10 = extractelement <4 x i32> %2, i32 2
  %extract11 = extractelement <4 x i32> %2, i32 3
  %3 = getelementptr <3 x float>* %0, i32 %extract
  %4 = getelementptr <3 x float>* %0, i32 %extract9
  %5 = getelementptr <3 x float>* %0, i32 %extract10
  %6 = getelementptr <3 x float>* %0, i32 %extract11
  %7 = load <3 x float>* %3
  %8 = load <3 x float>* %4
  %9 = load <3 x float>* %5
  %10 = load <3 x float>* %6
  %extend_vec = shufflevector <3 x float> %7, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec12 = shufflevector <3 x float> %8, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec13 = shufflevector <3 x float> %9, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec14 = shufflevector <3 x float> %10, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec12, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec13, <4 x float> %extend_vec14, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge21 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle026 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec12, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle127 = shufflevector <4 x float> %extend_vec13, <4 x float> %extend_vec14, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge28 = shufflevector <4 x float> %shuffle026, <4 x float> %shuffle127, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %store.val = insertvalue [3 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val38 = insertvalue [3 x <4 x float>] %store.val, <4 x float> %shuffleMerge21, 1
  %store.val39 = insertvalue [3 x <4 x float>] %store.val38, <4 x float> %shuffleMerge28, 2
  %11 = tail call [3 x <4 x float>] @_f_v.__vertical_normalize3f4([3 x <4 x float>] %store.val39)
  %12 = extractvalue [3 x <4 x float>] %11, 0
  %13 = extractvalue [3 x <4 x float>] %11, 1
  %14 = extractvalue [3 x <4 x float>] %11, 2
  %shuf_transpL40 = shufflevector <4 x float> %12, <4 x float> %14, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH42 = shufflevector <4 x float> %12, <4 x float> %14, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH43 = shufflevector <4 x float> %13, <4 x float> undef, <4 x i32> <i32 2, i32 3, i32 undef, i32 undef>
  %shuf_transpL44 = shufflevector <4 x float> %shuf_transpL40, <4 x float> %13, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH45 = shufflevector <4 x float> %shuf_transpL40, <4 x float> %13, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL46 = shufflevector <4 x float> %shuf_transpH42, <4 x float> %shuf_transpH43, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH47 = shufflevector <4 x float> %shuf_transpH42, <4 x float> %shuf_transpH43, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %breakdown48 = shufflevector <4 x float> %shuf_transpL44, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown49 = shufflevector <4 x float> %shuf_transpH45, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown50 = shufflevector <4 x float> %shuf_transpL46, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown51 = shufflevector <4 x float> %shuf_transpH47, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %15 = getelementptr <3 x float>* %1, i32 %extract
  %16 = getelementptr <3 x float>* %1, i32 %extract9
  %17 = getelementptr <3 x float>* %1, i32 %extract10
  %18 = getelementptr <3 x float>* %1, i32 %extract11
  store <3 x float> %breakdown48, <3 x float>* %15
  store <3 x float> %breakdown49, <3 x float>* %16
  store <3 x float> %breakdown50, <3 x float>* %17
  store <3 x float> %breakdown51, <3 x float>* %18
  ret void
}


; CHECK: @__normalizef4_test
; CHECK-NOT: @_f_v.__vertical_normalize4f4
; CHECK: @__vertical_normalize4f4
; CHECK-NOT: @_f_v.__vertical_normalize4f4
; CHECK: ret void
define void @__normalizef4_test(<4 x float>* nocapture, <4 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %2 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %2, i32 0
  %extract13 = extractelement <4 x i32> %2, i32 1
  %extract14 = extractelement <4 x i32> %2, i32 2
  %extract15 = extractelement <4 x i32> %2, i32 3
  %3 = getelementptr <4 x float>* %0, i32 %extract
  %4 = getelementptr <4 x float>* %0, i32 %extract13
  %5 = getelementptr <4 x float>* %0, i32 %extract14
  %6 = getelementptr <4 x float>* %0, i32 %extract15
  %7 = load <4 x float>* %3
  %8 = load <4 x float>* %4
  %9 = load <4 x float>* %5
  %10 = load <4 x float>* %6
  %shuffle0 = shufflevector <4 x float> %7, <4 x float> %8, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %9, <4 x float> %10, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge18 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle019 = shufflevector <4 x float> %7, <4 x float> %8, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffle120 = shufflevector <4 x float> %9, <4 x float> %10, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuffleMerge21 = shufflevector <4 x float> %shuffle019, <4 x float> %shuffle120, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge24 = shufflevector <4 x float> %shuffle019, <4 x float> %shuffle120, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %store.val = insertvalue [4 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val31 = insertvalue [4 x <4 x float>] %store.val, <4 x float> %shuffleMerge18, 1
  %store.val32 = insertvalue [4 x <4 x float>] %store.val31, <4 x float> %shuffleMerge21, 2
  %store.val33 = insertvalue [4 x <4 x float>] %store.val32, <4 x float> %shuffleMerge24, 3
  %11 = tail call [4 x <4 x float>] @_f_v.__vertical_normalize4f4([4 x <4 x float>] %store.val33)
  %12 = extractvalue [4 x <4 x float>] %11, 0
  %13 = extractvalue [4 x <4 x float>] %11, 1
  %14 = extractvalue [4 x <4 x float>] %11, 2
  %15 = extractvalue [4 x <4 x float>] %11, 3
  %shuf_transpL34 = shufflevector <4 x float> %12, <4 x float> %14, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpL35 = shufflevector <4 x float> %13, <4 x float> %15, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH36 = shufflevector <4 x float> %12, <4 x float> %14, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH37 = shufflevector <4 x float> %13, <4 x float> %15, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpL38 = shufflevector <4 x float> %shuf_transpL34, <4 x float> %shuf_transpL35, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH39 = shufflevector <4 x float> %shuf_transpL34, <4 x float> %shuf_transpL35, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL40 = shufflevector <4 x float> %shuf_transpH36, <4 x float> %shuf_transpH37, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH41 = shufflevector <4 x float> %shuf_transpH36, <4 x float> %shuf_transpH37, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %16 = getelementptr <4 x float>* %1, i32 %extract
  %17 = getelementptr <4 x float>* %1, i32 %extract13
  %18 = getelementptr <4 x float>* %1, i32 %extract14
  %19 = getelementptr <4 x float>* %1, i32 %extract15
  store <4 x float> %shuf_transpL38, <4 x float>* %16
  store <4 x float> %shuf_transpH39, <4 x float>* %17
  store <4 x float> %shuf_transpL40, <4 x float>* %18
  store <4 x float> %shuf_transpH41, <4 x float>* %19
  ret void
}


; CHECK: @__ci_gamma_scalar_SPI_test
; CHECK-NOT: @_f_v.__ci_gamma_SPI
; CHECK: @__ci_gamma_SPI
; CHECK-NOT: @_f_v.__ci_gamma_SPI
; CHECK: ret void
define void @__ci_gamma_scalar_SPI_test(<3 x float>* nocapture, float* nocapture, <3 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <4 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %3 = add <4 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3>
  %extract = extractelement <4 x i32> %3, i32 0
  %extract9 = extractelement <4 x i32> %3, i32 1
  %extract10 = extractelement <4 x i32> %3, i32 2
  %extract11 = extractelement <4 x i32> %3, i32 3
  %4 = getelementptr <3 x float>* %0, i32 %extract
  %5 = getelementptr <3 x float>* %0, i32 %extract9
  %6 = getelementptr <3 x float>* %0, i32 %extract10
  %7 = getelementptr <3 x float>* %0, i32 %extract11
  %8 = load <3 x float>* %4
  %9 = load <3 x float>* %5
  %10 = load <3 x float>* %6
  %11 = load <3 x float>* %7
  %extend_vec = shufflevector <3 x float> %8, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec12 = shufflevector <3 x float> %9, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec13 = shufflevector <3 x float> %10, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %extend_vec14 = shufflevector <3 x float> %11, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %shuffle0 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec12, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffle1 = shufflevector <4 x float> %extend_vec13, <4 x float> %extend_vec14, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuffleMerge = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffleMerge21 = shufflevector <4 x float> %shuffle0, <4 x float> %shuffle1, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle026 = shufflevector <4 x float> %extend_vec, <4 x float> %extend_vec12, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle127 = shufflevector <4 x float> %extend_vec13, <4 x float> %extend_vec14, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge28 = shufflevector <4 x float> %shuffle026, <4 x float> %shuffle127, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %12 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %12 to <4 x float>*
  %load_arg229 = load <4 x float>* %ptrTypeCast, align 4
  %store.val = insertvalue [3 x <4 x float>] undef, <4 x float> %shuffleMerge, 0
  %store.val39 = insertvalue [3 x <4 x float>] %store.val, <4 x float> %shuffleMerge21, 1
  %store.val40 = insertvalue [3 x <4 x float>] %store.val39, <4 x float> %shuffleMerge28, 2
  %13 = tail call [3 x <4 x float>] @_f_v.__ci_gamma_SPI([3 x <4 x float>] %store.val40, <4 x float> %load_arg229)
  %14 = extractvalue [3 x <4 x float>] %13, 0
  %15 = extractvalue [3 x <4 x float>] %13, 1
  %16 = extractvalue [3 x <4 x float>] %13, 2
  %shuf_transpL41 = shufflevector <4 x float> %14, <4 x float> %16, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH43 = shufflevector <4 x float> %14, <4 x float> %16, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH44 = shufflevector <4 x float> %15, <4 x float> undef, <4 x i32> <i32 2, i32 3, i32 undef, i32 undef>
  %shuf_transpL45 = shufflevector <4 x float> %shuf_transpL41, <4 x float> %15, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH46 = shufflevector <4 x float> %shuf_transpL41, <4 x float> %15, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL47 = shufflevector <4 x float> %shuf_transpH43, <4 x float> %shuf_transpH44, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH48 = shufflevector <4 x float> %shuf_transpH43, <4 x float> %shuf_transpH44, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %breakdown49 = shufflevector <4 x float> %shuf_transpL45, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown50 = shufflevector <4 x float> %shuf_transpH46, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown51 = shufflevector <4 x float> %shuf_transpL47, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown52 = shufflevector <4 x float> %shuf_transpH48, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %17 = getelementptr <3 x float>* %2, i32 %extract
  %18 = getelementptr <3 x float>* %2, i32 %extract9
  %19 = getelementptr <3 x float>* %2, i32 %extract10
  %20 = getelementptr <3 x float>* %2, i32 %extract11
  store <3 x float> %breakdown49, <3 x float>* %17
  store <3 x float> %breakdown50, <3 x float>* %18
  store <3 x float> %breakdown51, <3 x float>* %19
  store <3 x float> %breakdown52, <3 x float>* %20
  ret void
}

declare [3 x <4 x float>] @_f_v.__vertical_cross3f4([3 x <4 x float>], [3 x <4 x float>]) nounwind

declare [4 x <4 x float>] @_f_v.__vertical_cross4f4([4 x <4 x float>], [4 x <4 x float>]) nounwind

declare <4 x float> @_f_v.__vertical_fast_distance1f4(<4 x float>, <4 x float>) nounwind readnone

declare <4 x float> @_f_v.__vertical_fast_distance2f4([2 x <4 x float>], [2 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_fast_distance3f4([3 x <4 x float>], [3 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_fast_distance4f4([4 x <4 x float>], [4 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_distance1f4(<4 x float>, <4 x float>) nounwind readnone

declare <4 x float> @_f_v.__vertical_length1f4(<4 x float>) nounwind readnone

declare <4 x float> @_f_v.__vertical_distance2f4([2 x <4 x float>], [2 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_length2f4([2 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_distance3f4([3 x <4 x float>], [3 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_length3f4([3 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_distance4f4([4 x <4 x float>], [4 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_length4f4([4 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_dot1f4(<4 x float>, <4 x float>) nounwind readnone

declare <4 x float> @_f_v.__vertical_dot2f4([2 x <4 x float>], [2 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_dot3f4([3 x <4 x float>], [3 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_dot4f4([4 x <4 x float>], [4 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_fast_length1f4(<4 x float>) nounwind readnone

declare <4 x float> @_f_v.__vertical_fast_length2f4([2 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_fast_length3f4([3 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_fast_length4f4([4 x <4 x float>]) nounwind readnone

declare <4 x float> @_f_v.__vertical_fast_normalize1f4(<4 x float>) nounwind

declare [2 x <4 x float>] @_f_v.__vertical_fast_normalize2f4([2 x <4 x float>]) nounwind

declare [3 x <4 x float>] @_f_v.__vertical_fast_normalize3f4([3 x <4 x float>]) nounwind

declare [4 x <4 x float>] @_f_v.__vertical_fast_normalize4f4([4 x <4 x float>]) nounwind

declare <4 x float> @_f_v.__vertical_normalize1f4(<4 x float>) nounwind

declare [2 x <4 x float>] @_f_v.__vertical_normalize2f4([2 x <4 x float>]) nounwind

declare [3 x <4 x float>] @_f_v.__vertical_normalize3f4([3 x <4 x float>]) nounwind

declare [4 x <4 x float>] @_f_v.__vertical_normalize4f4([4 x <4 x float>]) nounwind

declare [3 x <4 x float>] @_f_v.__ci_gamma_SPI([3 x <4 x float>], <4 x float>) nounwind
