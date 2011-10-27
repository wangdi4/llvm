
; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/apple_only_dcls32.ll -runtime=apple -CLBltnResolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; ModuleID = 'c:\work\geo_tests32_preVec.ll'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @__crossf3_test
; CHECK-NOT: @_f_v.__vertical_cross3f8
; CHECK: @__vertical_cross3f8
; CHECK-NOT: @_f_v.__vertical_cross3f8
; CHECK: ret void
define void @__crossf3_test(<3 x float>* nocapture, <3 x float>* nocapture, <3 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %3 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %3, i32 0
  %extract15 = extractelement <8 x i32> %3, i32 1
  %extract16 = extractelement <8 x i32> %3, i32 2
  %extract17 = extractelement <8 x i32> %3, i32 3
  %extract18 = extractelement <8 x i32> %3, i32 4
  %extract19 = extractelement <8 x i32> %3, i32 5
  %extract20 = extractelement <8 x i32> %3, i32 6
  %extract21 = extractelement <8 x i32> %3, i32 7
  %4 = getelementptr <3 x float>* %0, i32 %extract
  %5 = getelementptr <3 x float>* %0, i32 %extract15
  %6 = getelementptr <3 x float>* %0, i32 %extract16
  %7 = getelementptr <3 x float>* %0, i32 %extract17
  %8 = getelementptr <3 x float>* %0, i32 %extract18
  %9 = getelementptr <3 x float>* %0, i32 %extract19
  %10 = getelementptr <3 x float>* %0, i32 %extract20
  %11 = getelementptr <3 x float>* %0, i32 %extract21
  %12 = load <3 x float>* %4
  %13 = load <3 x float>* %5
  %14 = load <3 x float>* %6
  %15 = load <3 x float>* %7
  %16 = load <3 x float>* %8
  %17 = load <3 x float>* %9
  %18 = load <3 x float>* %10
  %19 = load <3 x float>* %11
  %20 = extractelement <3 x float> %12, i32 0
  %21 = extractelement <3 x float> %13, i32 0
  %22 = extractelement <3 x float> %14, i32 0
  %23 = extractelement <3 x float> %15, i32 0
  %24 = extractelement <3 x float> %16, i32 0
  %25 = extractelement <3 x float> %17, i32 0
  %26 = extractelement <3 x float> %18, i32 0
  %27 = extractelement <3 x float> %19, i32 0
  %temp.vect85 = insertelement <8 x float> undef, float %20, i32 0
  %temp.vect86 = insertelement <8 x float> %temp.vect85, float %21, i32 1
  %temp.vect87 = insertelement <8 x float> %temp.vect86, float %22, i32 2
  %temp.vect88 = insertelement <8 x float> %temp.vect87, float %23, i32 3
  %temp.vect89 = insertelement <8 x float> %temp.vect88, float %24, i32 4
  %temp.vect90 = insertelement <8 x float> %temp.vect89, float %25, i32 5
  %temp.vect91 = insertelement <8 x float> %temp.vect90, float %26, i32 6
  %temp.vect92 = insertelement <8 x float> %temp.vect91, float %27, i32 7
  %28 = extractelement <3 x float> %12, i32 1
  %29 = extractelement <3 x float> %13, i32 1
  %30 = extractelement <3 x float> %14, i32 1
  %31 = extractelement <3 x float> %15, i32 1
  %32 = extractelement <3 x float> %16, i32 1
  %33 = extractelement <3 x float> %17, i32 1
  %34 = extractelement <3 x float> %18, i32 1
  %35 = extractelement <3 x float> %19, i32 1
  %temp.vect77 = insertelement <8 x float> undef, float %28, i32 0
  %temp.vect78 = insertelement <8 x float> %temp.vect77, float %29, i32 1
  %temp.vect79 = insertelement <8 x float> %temp.vect78, float %30, i32 2
  %temp.vect80 = insertelement <8 x float> %temp.vect79, float %31, i32 3
  %temp.vect81 = insertelement <8 x float> %temp.vect80, float %32, i32 4
  %temp.vect82 = insertelement <8 x float> %temp.vect81, float %33, i32 5
  %temp.vect83 = insertelement <8 x float> %temp.vect82, float %34, i32 6
  %temp.vect84 = insertelement <8 x float> %temp.vect83, float %35, i32 7
  %36 = extractelement <3 x float> %12, i32 2
  %37 = extractelement <3 x float> %13, i32 2
  %38 = extractelement <3 x float> %14, i32 2
  %39 = extractelement <3 x float> %15, i32 2
  %40 = extractelement <3 x float> %16, i32 2
  %41 = extractelement <3 x float> %17, i32 2
  %42 = extractelement <3 x float> %18, i32 2
  %43 = extractelement <3 x float> %19, i32 2
  %temp.vect69 = insertelement <8 x float> undef, float %36, i32 0
  %temp.vect70 = insertelement <8 x float> %temp.vect69, float %37, i32 1
  %temp.vect71 = insertelement <8 x float> %temp.vect70, float %38, i32 2
  %temp.vect72 = insertelement <8 x float> %temp.vect71, float %39, i32 3
  %temp.vect73 = insertelement <8 x float> %temp.vect72, float %40, i32 4
  %temp.vect74 = insertelement <8 x float> %temp.vect73, float %41, i32 5
  %temp.vect75 = insertelement <8 x float> %temp.vect74, float %42, i32 6
  %temp.vect76 = insertelement <8 x float> %temp.vect75, float %43, i32 7
  %44 = getelementptr <3 x float>* %1, i32 %extract
  %45 = getelementptr <3 x float>* %1, i32 %extract15
  %46 = getelementptr <3 x float>* %1, i32 %extract16
  %47 = getelementptr <3 x float>* %1, i32 %extract17
  %48 = getelementptr <3 x float>* %1, i32 %extract18
  %49 = getelementptr <3 x float>* %1, i32 %extract19
  %50 = getelementptr <3 x float>* %1, i32 %extract20
  %51 = getelementptr <3 x float>* %1, i32 %extract21
  %52 = load <3 x float>* %44
  %53 = load <3 x float>* %45
  %54 = load <3 x float>* %46
  %55 = load <3 x float>* %47
  %56 = load <3 x float>* %48
  %57 = load <3 x float>* %49
  %58 = load <3 x float>* %50
  %59 = load <3 x float>* %51
  %60 = extractelement <3 x float> %52, i32 0
  %61 = extractelement <3 x float> %53, i32 0
  %62 = extractelement <3 x float> %54, i32 0
  %63 = extractelement <3 x float> %55, i32 0
  %64 = extractelement <3 x float> %56, i32 0
  %65 = extractelement <3 x float> %57, i32 0
  %66 = extractelement <3 x float> %58, i32 0
  %67 = extractelement <3 x float> %59, i32 0
  %temp.vect111 = insertelement <8 x float> undef, float %60, i32 0
  %temp.vect112 = insertelement <8 x float> %temp.vect111, float %61, i32 1
  %temp.vect113 = insertelement <8 x float> %temp.vect112, float %62, i32 2
  %temp.vect114 = insertelement <8 x float> %temp.vect113, float %63, i32 3
  %temp.vect115 = insertelement <8 x float> %temp.vect114, float %64, i32 4
  %temp.vect116 = insertelement <8 x float> %temp.vect115, float %65, i32 5
  %temp.vect117 = insertelement <8 x float> %temp.vect116, float %66, i32 6
  %temp.vect118 = insertelement <8 x float> %temp.vect117, float %67, i32 7
  %68 = extractelement <3 x float> %52, i32 1
  %69 = extractelement <3 x float> %53, i32 1
  %70 = extractelement <3 x float> %54, i32 1
  %71 = extractelement <3 x float> %55, i32 1
  %72 = extractelement <3 x float> %56, i32 1
  %73 = extractelement <3 x float> %57, i32 1
  %74 = extractelement <3 x float> %58, i32 1
  %75 = extractelement <3 x float> %59, i32 1
  %temp.vect103 = insertelement <8 x float> undef, float %68, i32 0
  %temp.vect104 = insertelement <8 x float> %temp.vect103, float %69, i32 1
  %temp.vect105 = insertelement <8 x float> %temp.vect104, float %70, i32 2
  %temp.vect106 = insertelement <8 x float> %temp.vect105, float %71, i32 3
  %temp.vect107 = insertelement <8 x float> %temp.vect106, float %72, i32 4
  %temp.vect108 = insertelement <8 x float> %temp.vect107, float %73, i32 5
  %temp.vect109 = insertelement <8 x float> %temp.vect108, float %74, i32 6
  %temp.vect110 = insertelement <8 x float> %temp.vect109, float %75, i32 7
  %76 = extractelement <3 x float> %52, i32 2
  %77 = extractelement <3 x float> %53, i32 2
  %78 = extractelement <3 x float> %54, i32 2
  %79 = extractelement <3 x float> %55, i32 2
  %80 = extractelement <3 x float> %56, i32 2
  %81 = extractelement <3 x float> %57, i32 2
  %82 = extractelement <3 x float> %58, i32 2
  %83 = extractelement <3 x float> %59, i32 2
  %temp.vect95 = insertelement <8 x float> undef, float %76, i32 0
  %temp.vect96 = insertelement <8 x float> %temp.vect95, float %77, i32 1
  %temp.vect97 = insertelement <8 x float> %temp.vect96, float %78, i32 2
  %temp.vect98 = insertelement <8 x float> %temp.vect97, float %79, i32 3
  %temp.vect99 = insertelement <8 x float> %temp.vect98, float %80, i32 4
  %temp.vect100 = insertelement <8 x float> %temp.vect99, float %81, i32 5
  %temp.vect101 = insertelement <8 x float> %temp.vect100, float %82, i32 6
  %temp.vect102 = insertelement <8 x float> %temp.vect101, float %83, i32 7
  %store.val = insertvalue [3 x <8 x float>] undef, <8 x float> %temp.vect92, 0
  %store.val93 = insertvalue [3 x <8 x float>] %store.val, <8 x float> %temp.vect84, 1
  %store.val94 = insertvalue [3 x <8 x float>] %store.val93, <8 x float> %temp.vect76, 2
  %store.val119 = insertvalue [3 x <8 x float>] undef, <8 x float> %temp.vect118, 0
  %store.val120 = insertvalue [3 x <8 x float>] %store.val119, <8 x float> %temp.vect110, 1
  %store.val121 = insertvalue [3 x <8 x float>] %store.val120, <8 x float> %temp.vect102, 2
  %84 = tail call [3 x <8 x float>] @_f_v.__vertical_cross3f8([3 x <8 x float>] %store.val94, [3 x <8 x float>] %store.val121)
  %85 = extractvalue [3 x <8 x float>] %84, 0
  %86 = extractvalue [3 x <8 x float>] %84, 1
  %87 = extractvalue [3 x <8 x float>] %84, 2
  %shuf_transpL = shufflevector <8 x float> %85, <8 x float> %87, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %shuf_transpH = shufflevector <8 x float> %85, <8 x float> %87, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %shuf_transpH123 = shufflevector <8 x float> %86, <8 x float> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 6, i32 7, i32 undef, i32 undef>
  %shuf_transpL124 = shufflevector <8 x float> %shuf_transpL, <8 x float> %86, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH125 = shufflevector <8 x float> %shuf_transpL, <8 x float> %86, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %shuf_transpL126 = shufflevector <8 x float> %shuf_transpH, <8 x float> %shuf_transpH123, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH127 = shufflevector <8 x float> %shuf_transpH, <8 x float> %shuf_transpH123, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %breakdown = shufflevector <8 x float> %shuf_transpL124, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown128 = shufflevector <8 x float> %shuf_transpH125, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown129 = shufflevector <8 x float> %shuf_transpL126, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown130 = shufflevector <8 x float> %shuf_transpH127, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown131 = shufflevector <8 x float> %shuf_transpL124, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %breakdown132 = shufflevector <8 x float> %shuf_transpH125, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %breakdown133 = shufflevector <8 x float> %shuf_transpL126, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %breakdown134 = shufflevector <8 x float> %shuf_transpH127, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %88 = getelementptr <3 x float>* %2, i32 %extract
  %89 = getelementptr <3 x float>* %2, i32 %extract15
  %90 = getelementptr <3 x float>* %2, i32 %extract16
  %91 = getelementptr <3 x float>* %2, i32 %extract17
  %92 = getelementptr <3 x float>* %2, i32 %extract18
  %93 = getelementptr <3 x float>* %2, i32 %extract19
  %94 = getelementptr <3 x float>* %2, i32 %extract20
  %95 = getelementptr <3 x float>* %2, i32 %extract21
  store <3 x float> %breakdown, <3 x float>* %88
  store <3 x float> %breakdown128, <3 x float>* %89
  store <3 x float> %breakdown129, <3 x float>* %90
  store <3 x float> %breakdown130, <3 x float>* %91
  store <3 x float> %breakdown131, <3 x float>* %92
  store <3 x float> %breakdown132, <3 x float>* %93
  store <3 x float> %breakdown133, <3 x float>* %94
  store <3 x float> %breakdown134, <3 x float>* %95
  ret void
}

declare i32 @get_global_id(i32)



; CHECK: @__crossf4_test
; CHECK-NOT: @_f_v.__vertical_cross4f8
; CHECK: @__vertical_cross4f8
; CHECK-NOT: @_f_v.__vertical_cross4f8
; CHECK: ret void
define void @__crossf4_test(<4 x float>* nocapture, <4 x float>* nocapture, <4 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %3 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %3, i32 0
  %extract21 = extractelement <8 x i32> %3, i32 1
  %extract22 = extractelement <8 x i32> %3, i32 2
  %extract23 = extractelement <8 x i32> %3, i32 3
  %extract24 = extractelement <8 x i32> %3, i32 4
  %extract25 = extractelement <8 x i32> %3, i32 5
  %extract26 = extractelement <8 x i32> %3, i32 6
  %extract27 = extractelement <8 x i32> %3, i32 7
  %4 = getelementptr <4 x float>* %0, i32 %extract
  %5 = getelementptr <4 x float>* %0, i32 %extract21
  %6 = getelementptr <4 x float>* %0, i32 %extract22
  %7 = getelementptr <4 x float>* %0, i32 %extract23
  %8 = getelementptr <4 x float>* %0, i32 %extract24
  %9 = getelementptr <4 x float>* %0, i32 %extract25
  %10 = getelementptr <4 x float>* %0, i32 %extract26
  %11 = getelementptr <4 x float>* %0, i32 %extract27
  %12 = load <4 x float>* %4
  %13 = load <4 x float>* %5
  %14 = load <4 x float>* %6
  %15 = load <4 x float>* %7
  %16 = load <4 x float>* %8
  %17 = load <4 x float>* %9
  %18 = load <4 x float>* %10
  %19 = load <4 x float>* %11
  %extend_vec = shufflevector <4 x float> %12, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec28 = shufflevector <4 x float> %13, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec29 = shufflevector <4 x float> %14, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec30 = shufflevector <4 x float> %15, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec31 = shufflevector <4 x float> %16, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec32 = shufflevector <4 x float> %17, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec33 = shufflevector <4 x float> %18, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec34 = shufflevector <4 x float> %19, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %Seq_128_0 = shufflevector <8 x float> %extend_vec, <8 x float> %extend_vec31, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_1 = shufflevector <8 x float> %extend_vec28, <8 x float> %extend_vec32, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_2 = shufflevector <8 x float> %extend_vec29, <8 x float> %extend_vec33, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_3 = shufflevector <8 x float> %extend_vec30, <8 x float> %extend_vec34, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_64_0 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_64_1 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_32_0 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_160 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %Seq_64_285 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_64_386 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_32_293 = shufflevector <8 x float> %Seq_64_285, <8 x float> %Seq_64_386, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_3126 = shufflevector <8 x float> %Seq_64_285, <8 x float> %Seq_64_386, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %20 = getelementptr <4 x float>* %1, i32 %extract
  %21 = getelementptr <4 x float>* %1, i32 %extract21
  %22 = getelementptr <4 x float>* %1, i32 %extract22
  %23 = getelementptr <4 x float>* %1, i32 %extract23
  %24 = getelementptr <4 x float>* %1, i32 %extract24
  %25 = getelementptr <4 x float>* %1, i32 %extract25
  %26 = getelementptr <4 x float>* %1, i32 %extract26
  %27 = getelementptr <4 x float>* %1, i32 %extract27
  %28 = load <4 x float>* %20
  %29 = load <4 x float>* %21
  %30 = load <4 x float>* %22
  %31 = load <4 x float>* %23
  %32 = load <4 x float>* %24
  %33 = load <4 x float>* %25
  %34 = load <4 x float>* %26
  %35 = load <4 x float>* %27
  %extend_vec131 = shufflevector <4 x float> %28, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec132 = shufflevector <4 x float> %29, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec133 = shufflevector <4 x float> %30, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec134 = shufflevector <4 x float> %31, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec135 = shufflevector <4 x float> %32, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec136 = shufflevector <4 x float> %33, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec137 = shufflevector <4 x float> %34, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec138 = shufflevector <4 x float> %35, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %Seq_128_0139 = shufflevector <8 x float> %extend_vec131, <8 x float> %extend_vec135, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_1140 = shufflevector <8 x float> %extend_vec132, <8 x float> %extend_vec136, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_2141 = shufflevector <8 x float> %extend_vec133, <8 x float> %extend_vec137, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_3142 = shufflevector <8 x float> %extend_vec134, <8 x float> %extend_vec138, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_64_0147 = shufflevector <8 x float> %Seq_128_0139, <8 x float> %Seq_128_2141, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_64_1148 = shufflevector <8 x float> %Seq_128_1140, <8 x float> %Seq_128_3142, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_32_0155 = shufflevector <8 x float> %Seq_64_0147, <8 x float> %Seq_64_1148, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_1188 = shufflevector <8 x float> %Seq_64_0147, <8 x float> %Seq_64_1148, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %Seq_64_2213 = shufflevector <8 x float> %Seq_128_0139, <8 x float> %Seq_128_2141, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_64_3214 = shufflevector <8 x float> %Seq_128_1140, <8 x float> %Seq_128_3142, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_32_2221 = shufflevector <8 x float> %Seq_64_2213, <8 x float> %Seq_64_3214, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_3254 = shufflevector <8 x float> %Seq_64_2213, <8 x float> %Seq_64_3214, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %store.val = insertvalue [4 x <8 x float>] undef, <8 x float> %Seq_32_0, 0
  %store.val288 = insertvalue [4 x <8 x float>] %store.val, <8 x float> %Seq_32_160, 1
  %store.val289 = insertvalue [4 x <8 x float>] %store.val288, <8 x float> %Seq_32_293, 2
  %store.val290 = insertvalue [4 x <8 x float>] %store.val289, <8 x float> %Seq_32_3126, 3
  %store.val291 = insertvalue [4 x <8 x float>] undef, <8 x float> %Seq_32_0155, 0
  %store.val292 = insertvalue [4 x <8 x float>] %store.val291, <8 x float> %Seq_32_1188, 1
  %store.val293 = insertvalue [4 x <8 x float>] %store.val292, <8 x float> %Seq_32_2221, 2
  %store.val294 = insertvalue [4 x <8 x float>] %store.val293, <8 x float> %Seq_32_3254, 3
  %36 = tail call [4 x <8 x float>] @_f_v.__vertical_cross4f8([4 x <8 x float>] %store.val290, [4 x <8 x float>] %store.val294)
  %37 = extractvalue [4 x <8 x float>] %36, 0
  %38 = extractvalue [4 x <8 x float>] %36, 1
  %39 = extractvalue [4 x <8 x float>] %36, 2
  %40 = extractvalue [4 x <8 x float>] %36, 3
  %shuf_transpL295 = shufflevector <8 x float> %37, <8 x float> %39, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %shuf_transpL296 = shufflevector <8 x float> %38, <8 x float> %40, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %shuf_transpH297 = shufflevector <8 x float> %37, <8 x float> %39, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %shuf_transpH298 = shufflevector <8 x float> %38, <8 x float> %40, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %shuf_transpL299 = shufflevector <8 x float> %shuf_transpL295, <8 x float> %shuf_transpL296, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH300 = shufflevector <8 x float> %shuf_transpL295, <8 x float> %shuf_transpL296, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %shuf_transpL301 = shufflevector <8 x float> %shuf_transpH297, <8 x float> %shuf_transpH298, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH302 = shufflevector <8 x float> %shuf_transpH297, <8 x float> %shuf_transpH298, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %breakdown303 = shufflevector <8 x float> %shuf_transpL299, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %breakdown304 = shufflevector <8 x float> %shuf_transpH300, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %breakdown305 = shufflevector <8 x float> %shuf_transpL301, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %breakdown306 = shufflevector <8 x float> %shuf_transpH302, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %breakdown307 = shufflevector <8 x float> %shuf_transpL299, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %breakdown308 = shufflevector <8 x float> %shuf_transpH300, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %breakdown309 = shufflevector <8 x float> %shuf_transpL301, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %breakdown310 = shufflevector <8 x float> %shuf_transpH302, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %41 = getelementptr <4 x float>* %2, i32 %extract
  %42 = getelementptr <4 x float>* %2, i32 %extract21
  %43 = getelementptr <4 x float>* %2, i32 %extract22
  %44 = getelementptr <4 x float>* %2, i32 %extract23
  %45 = getelementptr <4 x float>* %2, i32 %extract24
  %46 = getelementptr <4 x float>* %2, i32 %extract25
  %47 = getelementptr <4 x float>* %2, i32 %extract26
  %48 = getelementptr <4 x float>* %2, i32 %extract27
  store <4 x float> %breakdown303, <4 x float>* %41
  store <4 x float> %breakdown304, <4 x float>* %42
  store <4 x float> %breakdown305, <4 x float>* %43
  store <4 x float> %breakdown306, <4 x float>* %44
  store <4 x float> %breakdown307, <4 x float>* %45
  store <4 x float> %breakdown308, <4 x float>* %46
  store <4 x float> %breakdown309, <4 x float>* %47
  store <4 x float> %breakdown310, <4 x float>* %48
  ret void
}



; CHECK: @__fast_distancef_test
; CHECK-NOT: @_f_v.__vertical_fast_distance1f8
; CHECK: @__vertical_fast_distance1f8
; CHECK-NOT: @_f_v.__vertical_fast_distance1f8
; CHECK: ret void
define void @__fast_distancef_test(float* nocapture, float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %3 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %3 to <8 x float>*
  %load_arg8 = load <8 x float>* %ptrTypeCast, align 4
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast9 = bitcast float* %4 to <8 x float>*
  %load_arg210 = load <8 x float>* %ptrTypeCast9, align 4
  %5 = tail call <8 x float> @_f_v.__vertical_fast_distance1f8(<8 x float> %load_arg8, <8 x float> %load_arg210)
  %6 = getelementptr float* %2, i32 %gid
  %ptrTypeCast11 = bitcast float* %6 to <8 x float>*
  store <8 x float> %5, <8 x float>* %ptrTypeCast11, align 4
  ret void
}



; CHECK: @__fast_distancef2_test
; CHECK-NOT: @_f_v.__vertical_fast_distance2f8
; CHECK: @__vertical_fast_distance2f8
; CHECK-NOT: @_f_v.__vertical_fast_distance2f8
; CHECK: ret void
define void @__fast_distancef2_test(<2 x float>* nocapture, <2 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %3 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %3, i32 0
  %extract7 = extractelement <8 x i32> %3, i32 1
  %extract8 = extractelement <8 x i32> %3, i32 2
  %extract9 = extractelement <8 x i32> %3, i32 3
  %extract10 = extractelement <8 x i32> %3, i32 4
  %extract11 = extractelement <8 x i32> %3, i32 5
  %extract12 = extractelement <8 x i32> %3, i32 6
  %extract13 = extractelement <8 x i32> %3, i32 7
  %4 = getelementptr <2 x float>* %0, i32 %extract
  %5 = getelementptr <2 x float>* %0, i32 %extract7
  %6 = getelementptr <2 x float>* %0, i32 %extract8
  %7 = getelementptr <2 x float>* %0, i32 %extract9
  %8 = getelementptr <2 x float>* %0, i32 %extract10
  %9 = getelementptr <2 x float>* %0, i32 %extract11
  %10 = getelementptr <2 x float>* %0, i32 %extract12
  %11 = getelementptr <2 x float>* %0, i32 %extract13
  %12 = load <2 x float>* %4
  %13 = load <2 x float>* %5
  %14 = load <2 x float>* %6
  %15 = load <2 x float>* %7
  %16 = load <2 x float>* %8
  %17 = load <2 x float>* %9
  %18 = load <2 x float>* %10
  %19 = load <2 x float>* %11
  %20 = extractelement <2 x float> %12, i32 0
  %21 = extractelement <2 x float> %13, i32 0
  %22 = extractelement <2 x float> %14, i32 0
  %23 = extractelement <2 x float> %15, i32 0
  %24 = extractelement <2 x float> %16, i32 0
  %25 = extractelement <2 x float> %17, i32 0
  %26 = extractelement <2 x float> %18, i32 0
  %27 = extractelement <2 x float> %19, i32 0
  %temp.vect52 = insertelement <8 x float> undef, float %20, i32 0
  %temp.vect53 = insertelement <8 x float> %temp.vect52, float %21, i32 1
  %temp.vect54 = insertelement <8 x float> %temp.vect53, float %22, i32 2
  %temp.vect55 = insertelement <8 x float> %temp.vect54, float %23, i32 3
  %temp.vect56 = insertelement <8 x float> %temp.vect55, float %24, i32 4
  %temp.vect57 = insertelement <8 x float> %temp.vect56, float %25, i32 5
  %temp.vect58 = insertelement <8 x float> %temp.vect57, float %26, i32 6
  %temp.vect59 = insertelement <8 x float> %temp.vect58, float %27, i32 7
  %28 = extractelement <2 x float> %12, i32 1
  %29 = extractelement <2 x float> %13, i32 1
  %30 = extractelement <2 x float> %14, i32 1
  %31 = extractelement <2 x float> %15, i32 1
  %32 = extractelement <2 x float> %16, i32 1
  %33 = extractelement <2 x float> %17, i32 1
  %34 = extractelement <2 x float> %18, i32 1
  %35 = extractelement <2 x float> %19, i32 1
  %temp.vect = insertelement <8 x float> undef, float %28, i32 0
  %temp.vect45 = insertelement <8 x float> %temp.vect, float %29, i32 1
  %temp.vect46 = insertelement <8 x float> %temp.vect45, float %30, i32 2
  %temp.vect47 = insertelement <8 x float> %temp.vect46, float %31, i32 3
  %temp.vect48 = insertelement <8 x float> %temp.vect47, float %32, i32 4
  %temp.vect49 = insertelement <8 x float> %temp.vect48, float %33, i32 5
  %temp.vect50 = insertelement <8 x float> %temp.vect49, float %34, i32 6
  %temp.vect51 = insertelement <8 x float> %temp.vect50, float %35, i32 7
  %36 = getelementptr <2 x float>* %1, i32 %extract
  %37 = getelementptr <2 x float>* %1, i32 %extract7
  %38 = getelementptr <2 x float>* %1, i32 %extract8
  %39 = getelementptr <2 x float>* %1, i32 %extract9
  %40 = getelementptr <2 x float>* %1, i32 %extract10
  %41 = getelementptr <2 x float>* %1, i32 %extract11
  %42 = getelementptr <2 x float>* %1, i32 %extract12
  %43 = getelementptr <2 x float>* %1, i32 %extract13
  %44 = load <2 x float>* %36
  %45 = load <2 x float>* %37
  %46 = load <2 x float>* %38
  %47 = load <2 x float>* %39
  %48 = load <2 x float>* %40
  %49 = load <2 x float>* %41
  %50 = load <2 x float>* %42
  %51 = load <2 x float>* %43
  %52 = extractelement <2 x float> %44, i32 0
  %53 = extractelement <2 x float> %45, i32 0
  %54 = extractelement <2 x float> %46, i32 0
  %55 = extractelement <2 x float> %47, i32 0
  %56 = extractelement <2 x float> %48, i32 0
  %57 = extractelement <2 x float> %49, i32 0
  %58 = extractelement <2 x float> %50, i32 0
  %59 = extractelement <2 x float> %51, i32 0
  %temp.vect69 = insertelement <8 x float> undef, float %52, i32 0
  %temp.vect70 = insertelement <8 x float> %temp.vect69, float %53, i32 1
  %temp.vect71 = insertelement <8 x float> %temp.vect70, float %54, i32 2
  %temp.vect72 = insertelement <8 x float> %temp.vect71, float %55, i32 3
  %temp.vect73 = insertelement <8 x float> %temp.vect72, float %56, i32 4
  %temp.vect74 = insertelement <8 x float> %temp.vect73, float %57, i32 5
  %temp.vect75 = insertelement <8 x float> %temp.vect74, float %58, i32 6
  %temp.vect76 = insertelement <8 x float> %temp.vect75, float %59, i32 7
  %60 = extractelement <2 x float> %44, i32 1
  %61 = extractelement <2 x float> %45, i32 1
  %62 = extractelement <2 x float> %46, i32 1
  %63 = extractelement <2 x float> %47, i32 1
  %64 = extractelement <2 x float> %48, i32 1
  %65 = extractelement <2 x float> %49, i32 1
  %66 = extractelement <2 x float> %50, i32 1
  %67 = extractelement <2 x float> %51, i32 1
  %temp.vect61 = insertelement <8 x float> undef, float %60, i32 0
  %temp.vect62 = insertelement <8 x float> %temp.vect61, float %61, i32 1
  %temp.vect63 = insertelement <8 x float> %temp.vect62, float %62, i32 2
  %temp.vect64 = insertelement <8 x float> %temp.vect63, float %63, i32 3
  %temp.vect65 = insertelement <8 x float> %temp.vect64, float %64, i32 4
  %temp.vect66 = insertelement <8 x float> %temp.vect65, float %65, i32 5
  %temp.vect67 = insertelement <8 x float> %temp.vect66, float %66, i32 6
  %temp.vect68 = insertelement <8 x float> %temp.vect67, float %67, i32 7
  %store.val = insertvalue [2 x <8 x float>] undef, <8 x float> %temp.vect59, 0
  %store.val60 = insertvalue [2 x <8 x float>] %store.val, <8 x float> %temp.vect51, 1
  %store.val77 = insertvalue [2 x <8 x float>] undef, <8 x float> %temp.vect76, 0
  %store.val78 = insertvalue [2 x <8 x float>] %store.val77, <8 x float> %temp.vect68, 1
  %68 = tail call <8 x float> @_f_v.__vertical_fast_distance2f8([2 x <8 x float>] %store.val60, [2 x <8 x float>] %store.val78)
  %69 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %69 to <8 x float>*
  store <8 x float> %68, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__fast_distancef3_test
; CHECK-NOT: @_f_v.__vertical_fast_distance3f8
; CHECK: @__vertical_fast_distance3f8
; CHECK-NOT: @_f_v.__vertical_fast_distance3f8
; CHECK: ret void
define void @__fast_distancef3_test(<3 x float>* nocapture, <3 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %3 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %3, i32 0
  %extract11 = extractelement <8 x i32> %3, i32 1
  %extract12 = extractelement <8 x i32> %3, i32 2
  %extract13 = extractelement <8 x i32> %3, i32 3
  %extract14 = extractelement <8 x i32> %3, i32 4
  %extract15 = extractelement <8 x i32> %3, i32 5
  %extract16 = extractelement <8 x i32> %3, i32 6
  %extract17 = extractelement <8 x i32> %3, i32 7
  %4 = getelementptr <3 x float>* %0, i32 %extract
  %5 = getelementptr <3 x float>* %0, i32 %extract11
  %6 = getelementptr <3 x float>* %0, i32 %extract12
  %7 = getelementptr <3 x float>* %0, i32 %extract13
  %8 = getelementptr <3 x float>* %0, i32 %extract14
  %9 = getelementptr <3 x float>* %0, i32 %extract15
  %10 = getelementptr <3 x float>* %0, i32 %extract16
  %11 = getelementptr <3 x float>* %0, i32 %extract17
  %12 = load <3 x float>* %4
  %13 = load <3 x float>* %5
  %14 = load <3 x float>* %6
  %15 = load <3 x float>* %7
  %16 = load <3 x float>* %8
  %17 = load <3 x float>* %9
  %18 = load <3 x float>* %10
  %19 = load <3 x float>* %11
  %20 = extractelement <3 x float> %12, i32 0
  %21 = extractelement <3 x float> %13, i32 0
  %22 = extractelement <3 x float> %14, i32 0
  %23 = extractelement <3 x float> %15, i32 0
  %24 = extractelement <3 x float> %16, i32 0
  %25 = extractelement <3 x float> %17, i32 0
  %26 = extractelement <3 x float> %18, i32 0
  %27 = extractelement <3 x float> %19, i32 0
  %temp.vect80 = insertelement <8 x float> undef, float %20, i32 0
  %temp.vect81 = insertelement <8 x float> %temp.vect80, float %21, i32 1
  %temp.vect82 = insertelement <8 x float> %temp.vect81, float %22, i32 2
  %temp.vect83 = insertelement <8 x float> %temp.vect82, float %23, i32 3
  %temp.vect84 = insertelement <8 x float> %temp.vect83, float %24, i32 4
  %temp.vect85 = insertelement <8 x float> %temp.vect84, float %25, i32 5
  %temp.vect86 = insertelement <8 x float> %temp.vect85, float %26, i32 6
  %temp.vect87 = insertelement <8 x float> %temp.vect86, float %27, i32 7
  %28 = extractelement <3 x float> %12, i32 1
  %29 = extractelement <3 x float> %13, i32 1
  %30 = extractelement <3 x float> %14, i32 1
  %31 = extractelement <3 x float> %15, i32 1
  %32 = extractelement <3 x float> %16, i32 1
  %33 = extractelement <3 x float> %17, i32 1
  %34 = extractelement <3 x float> %18, i32 1
  %35 = extractelement <3 x float> %19, i32 1
  %temp.vect72 = insertelement <8 x float> undef, float %28, i32 0
  %temp.vect73 = insertelement <8 x float> %temp.vect72, float %29, i32 1
  %temp.vect74 = insertelement <8 x float> %temp.vect73, float %30, i32 2
  %temp.vect75 = insertelement <8 x float> %temp.vect74, float %31, i32 3
  %temp.vect76 = insertelement <8 x float> %temp.vect75, float %32, i32 4
  %temp.vect77 = insertelement <8 x float> %temp.vect76, float %33, i32 5
  %temp.vect78 = insertelement <8 x float> %temp.vect77, float %34, i32 6
  %temp.vect79 = insertelement <8 x float> %temp.vect78, float %35, i32 7
  %36 = extractelement <3 x float> %12, i32 2
  %37 = extractelement <3 x float> %13, i32 2
  %38 = extractelement <3 x float> %14, i32 2
  %39 = extractelement <3 x float> %15, i32 2
  %40 = extractelement <3 x float> %16, i32 2
  %41 = extractelement <3 x float> %17, i32 2
  %42 = extractelement <3 x float> %18, i32 2
  %43 = extractelement <3 x float> %19, i32 2
  %temp.vect = insertelement <8 x float> undef, float %36, i32 0
  %temp.vect65 = insertelement <8 x float> %temp.vect, float %37, i32 1
  %temp.vect66 = insertelement <8 x float> %temp.vect65, float %38, i32 2
  %temp.vect67 = insertelement <8 x float> %temp.vect66, float %39, i32 3
  %temp.vect68 = insertelement <8 x float> %temp.vect67, float %40, i32 4
  %temp.vect69 = insertelement <8 x float> %temp.vect68, float %41, i32 5
  %temp.vect70 = insertelement <8 x float> %temp.vect69, float %42, i32 6
  %temp.vect71 = insertelement <8 x float> %temp.vect70, float %43, i32 7
  %44 = getelementptr <3 x float>* %1, i32 %extract
  %45 = getelementptr <3 x float>* %1, i32 %extract11
  %46 = getelementptr <3 x float>* %1, i32 %extract12
  %47 = getelementptr <3 x float>* %1, i32 %extract13
  %48 = getelementptr <3 x float>* %1, i32 %extract14
  %49 = getelementptr <3 x float>* %1, i32 %extract15
  %50 = getelementptr <3 x float>* %1, i32 %extract16
  %51 = getelementptr <3 x float>* %1, i32 %extract17
  %52 = load <3 x float>* %44
  %53 = load <3 x float>* %45
  %54 = load <3 x float>* %46
  %55 = load <3 x float>* %47
  %56 = load <3 x float>* %48
  %57 = load <3 x float>* %49
  %58 = load <3 x float>* %50
  %59 = load <3 x float>* %51
  %60 = extractelement <3 x float> %52, i32 0
  %61 = extractelement <3 x float> %53, i32 0
  %62 = extractelement <3 x float> %54, i32 0
  %63 = extractelement <3 x float> %55, i32 0
  %64 = extractelement <3 x float> %56, i32 0
  %65 = extractelement <3 x float> %57, i32 0
  %66 = extractelement <3 x float> %58, i32 0
  %67 = extractelement <3 x float> %59, i32 0
  %temp.vect106 = insertelement <8 x float> undef, float %60, i32 0
  %temp.vect107 = insertelement <8 x float> %temp.vect106, float %61, i32 1
  %temp.vect108 = insertelement <8 x float> %temp.vect107, float %62, i32 2
  %temp.vect109 = insertelement <8 x float> %temp.vect108, float %63, i32 3
  %temp.vect110 = insertelement <8 x float> %temp.vect109, float %64, i32 4
  %temp.vect111 = insertelement <8 x float> %temp.vect110, float %65, i32 5
  %temp.vect112 = insertelement <8 x float> %temp.vect111, float %66, i32 6
  %temp.vect113 = insertelement <8 x float> %temp.vect112, float %67, i32 7
  %68 = extractelement <3 x float> %52, i32 1
  %69 = extractelement <3 x float> %53, i32 1
  %70 = extractelement <3 x float> %54, i32 1
  %71 = extractelement <3 x float> %55, i32 1
  %72 = extractelement <3 x float> %56, i32 1
  %73 = extractelement <3 x float> %57, i32 1
  %74 = extractelement <3 x float> %58, i32 1
  %75 = extractelement <3 x float> %59, i32 1
  %temp.vect98 = insertelement <8 x float> undef, float %68, i32 0
  %temp.vect99 = insertelement <8 x float> %temp.vect98, float %69, i32 1
  %temp.vect100 = insertelement <8 x float> %temp.vect99, float %70, i32 2
  %temp.vect101 = insertelement <8 x float> %temp.vect100, float %71, i32 3
  %temp.vect102 = insertelement <8 x float> %temp.vect101, float %72, i32 4
  %temp.vect103 = insertelement <8 x float> %temp.vect102, float %73, i32 5
  %temp.vect104 = insertelement <8 x float> %temp.vect103, float %74, i32 6
  %temp.vect105 = insertelement <8 x float> %temp.vect104, float %75, i32 7
  %76 = extractelement <3 x float> %52, i32 2
  %77 = extractelement <3 x float> %53, i32 2
  %78 = extractelement <3 x float> %54, i32 2
  %79 = extractelement <3 x float> %55, i32 2
  %80 = extractelement <3 x float> %56, i32 2
  %81 = extractelement <3 x float> %57, i32 2
  %82 = extractelement <3 x float> %58, i32 2
  %83 = extractelement <3 x float> %59, i32 2
  %temp.vect90 = insertelement <8 x float> undef, float %76, i32 0
  %temp.vect91 = insertelement <8 x float> %temp.vect90, float %77, i32 1
  %temp.vect92 = insertelement <8 x float> %temp.vect91, float %78, i32 2
  %temp.vect93 = insertelement <8 x float> %temp.vect92, float %79, i32 3
  %temp.vect94 = insertelement <8 x float> %temp.vect93, float %80, i32 4
  %temp.vect95 = insertelement <8 x float> %temp.vect94, float %81, i32 5
  %temp.vect96 = insertelement <8 x float> %temp.vect95, float %82, i32 6
  %temp.vect97 = insertelement <8 x float> %temp.vect96, float %83, i32 7
  %store.val = insertvalue [3 x <8 x float>] undef, <8 x float> %temp.vect87, 0
  %store.val88 = insertvalue [3 x <8 x float>] %store.val, <8 x float> %temp.vect79, 1
  %store.val89 = insertvalue [3 x <8 x float>] %store.val88, <8 x float> %temp.vect71, 2
  %store.val114 = insertvalue [3 x <8 x float>] undef, <8 x float> %temp.vect113, 0
  %store.val115 = insertvalue [3 x <8 x float>] %store.val114, <8 x float> %temp.vect105, 1
  %store.val116 = insertvalue [3 x <8 x float>] %store.val115, <8 x float> %temp.vect97, 2
  %84 = tail call <8 x float> @_f_v.__vertical_fast_distance3f8([3 x <8 x float>] %store.val89, [3 x <8 x float>] %store.val116)
  %85 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %85 to <8 x float>*
  store <8 x float> %84, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__fast_distancef4_test
; CHECK-NOT: @_f_v.__vertical_fast_distance4f8
; CHECK: @__vertical_fast_distance4f8
; CHECK-NOT: @_f_v.__vertical_fast_distance4f8
; CHECK: ret void
define void @__fast_distancef4_test(<4 x float>* nocapture, <4 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %3 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %3, i32 0
  %extract15 = extractelement <8 x i32> %3, i32 1
  %extract16 = extractelement <8 x i32> %3, i32 2
  %extract17 = extractelement <8 x i32> %3, i32 3
  %extract18 = extractelement <8 x i32> %3, i32 4
  %extract19 = extractelement <8 x i32> %3, i32 5
  %extract20 = extractelement <8 x i32> %3, i32 6
  %extract21 = extractelement <8 x i32> %3, i32 7
  %4 = getelementptr <4 x float>* %0, i32 %extract
  %5 = getelementptr <4 x float>* %0, i32 %extract15
  %6 = getelementptr <4 x float>* %0, i32 %extract16
  %7 = getelementptr <4 x float>* %0, i32 %extract17
  %8 = getelementptr <4 x float>* %0, i32 %extract18
  %9 = getelementptr <4 x float>* %0, i32 %extract19
  %10 = getelementptr <4 x float>* %0, i32 %extract20
  %11 = getelementptr <4 x float>* %0, i32 %extract21
  %12 = load <4 x float>* %4
  %13 = load <4 x float>* %5
  %14 = load <4 x float>* %6
  %15 = load <4 x float>* %7
  %16 = load <4 x float>* %8
  %17 = load <4 x float>* %9
  %18 = load <4 x float>* %10
  %19 = load <4 x float>* %11
  %extend_vec = shufflevector <4 x float> %12, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec22 = shufflevector <4 x float> %13, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec23 = shufflevector <4 x float> %14, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec24 = shufflevector <4 x float> %15, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec25 = shufflevector <4 x float> %16, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec26 = shufflevector <4 x float> %17, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec27 = shufflevector <4 x float> %18, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec28 = shufflevector <4 x float> %19, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %Seq_128_0 = shufflevector <8 x float> %extend_vec, <8 x float> %extend_vec25, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_1 = shufflevector <8 x float> %extend_vec22, <8 x float> %extend_vec26, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_2 = shufflevector <8 x float> %extend_vec23, <8 x float> %extend_vec27, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_3 = shufflevector <8 x float> %extend_vec24, <8 x float> %extend_vec28, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_64_0 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_64_1 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_32_0 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_154 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %Seq_64_279 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_64_380 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_32_287 = shufflevector <8 x float> %Seq_64_279, <8 x float> %Seq_64_380, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_3120 = shufflevector <8 x float> %Seq_64_279, <8 x float> %Seq_64_380, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %20 = getelementptr <4 x float>* %1, i32 %extract
  %21 = getelementptr <4 x float>* %1, i32 %extract15
  %22 = getelementptr <4 x float>* %1, i32 %extract16
  %23 = getelementptr <4 x float>* %1, i32 %extract17
  %24 = getelementptr <4 x float>* %1, i32 %extract18
  %25 = getelementptr <4 x float>* %1, i32 %extract19
  %26 = getelementptr <4 x float>* %1, i32 %extract20
  %27 = getelementptr <4 x float>* %1, i32 %extract21
  %28 = load <4 x float>* %20
  %29 = load <4 x float>* %21
  %30 = load <4 x float>* %22
  %31 = load <4 x float>* %23
  %32 = load <4 x float>* %24
  %33 = load <4 x float>* %25
  %34 = load <4 x float>* %26
  %35 = load <4 x float>* %27
  %extend_vec125 = shufflevector <4 x float> %28, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec126 = shufflevector <4 x float> %29, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec127 = shufflevector <4 x float> %30, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec128 = shufflevector <4 x float> %31, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec129 = shufflevector <4 x float> %32, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec130 = shufflevector <4 x float> %33, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec131 = shufflevector <4 x float> %34, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec132 = shufflevector <4 x float> %35, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %Seq_128_0133 = shufflevector <8 x float> %extend_vec125, <8 x float> %extend_vec129, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_1134 = shufflevector <8 x float> %extend_vec126, <8 x float> %extend_vec130, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_2135 = shufflevector <8 x float> %extend_vec127, <8 x float> %extend_vec131, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_3136 = shufflevector <8 x float> %extend_vec128, <8 x float> %extend_vec132, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_64_0141 = shufflevector <8 x float> %Seq_128_0133, <8 x float> %Seq_128_2135, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_64_1142 = shufflevector <8 x float> %Seq_128_1134, <8 x float> %Seq_128_3136, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_32_0149 = shufflevector <8 x float> %Seq_64_0141, <8 x float> %Seq_64_1142, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_1182 = shufflevector <8 x float> %Seq_64_0141, <8 x float> %Seq_64_1142, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %Seq_64_2207 = shufflevector <8 x float> %Seq_128_0133, <8 x float> %Seq_128_2135, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_64_3208 = shufflevector <8 x float> %Seq_128_1134, <8 x float> %Seq_128_3136, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_32_2215 = shufflevector <8 x float> %Seq_64_2207, <8 x float> %Seq_64_3208, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_3248 = shufflevector <8 x float> %Seq_64_2207, <8 x float> %Seq_64_3208, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %store.val = insertvalue [4 x <8 x float>] undef, <8 x float> %Seq_32_0, 0
  %store.val282 = insertvalue [4 x <8 x float>] %store.val, <8 x float> %Seq_32_154, 1
  %store.val283 = insertvalue [4 x <8 x float>] %store.val282, <8 x float> %Seq_32_287, 2
  %store.val284 = insertvalue [4 x <8 x float>] %store.val283, <8 x float> %Seq_32_3120, 3
  %store.val285 = insertvalue [4 x <8 x float>] undef, <8 x float> %Seq_32_0149, 0
  %store.val286 = insertvalue [4 x <8 x float>] %store.val285, <8 x float> %Seq_32_1182, 1
  %store.val287 = insertvalue [4 x <8 x float>] %store.val286, <8 x float> %Seq_32_2215, 2
  %store.val288 = insertvalue [4 x <8 x float>] %store.val287, <8 x float> %Seq_32_3248, 3
  %36 = tail call <8 x float> @_f_v.__vertical_fast_distance4f8([4 x <8 x float>] %store.val284, [4 x <8 x float>] %store.val288)
  %37 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %37 to <8 x float>*
  store <8 x float> %36, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__distancef_test
; CHECK-NOT: @_f_v.__vertical_distance1f8
; CHECK: @__vertical_distance1f8
; CHECK-NOT: @_f_v.__vertical_distance1f8
; CHECK: ret void
define void @__distancef_test(float* nocapture, float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %3 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %3 to <8 x float>*
  %load_arg8 = load <8 x float>* %ptrTypeCast, align 4
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast9 = bitcast float* %4 to <8 x float>*
  %load_arg210 = load <8 x float>* %ptrTypeCast9, align 4
  %5 = tail call <8 x float> @_f_v.__vertical_distance1f8(<8 x float> %load_arg8, <8 x float> %load_arg210)
  %6 = getelementptr float* %2, i32 %gid
  %ptrTypeCast11 = bitcast float* %6 to <8 x float>*
  store <8 x float> %5, <8 x float>* %ptrTypeCast11, align 4
  ret void
}



; CHECK: @__lengthf_test
; CHECK-NOT: @_f_v.__vertical_length1f8
; CHECK: @__vertical_length1f8
; CHECK-NOT: @_f_v.__vertical_length1f8
; CHECK: ret void
define void @__lengthf_test(float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %2 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %2 to <8 x float>*
  %load_arg8 = load <8 x float>* %ptrTypeCast, align 4
  %3 = tail call <8 x float> @_f_v.__vertical_length1f8(<8 x float> %load_arg8)
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast9 = bitcast float* %4 to <8 x float>*
  store <8 x float> %3, <8 x float>* %ptrTypeCast9, align 4
  ret void
}



; CHECK: @__distancef2_test
; CHECK-NOT: @_f_v.__vertical_distance2f8
; CHECK: @__vertical_distance2f8
; CHECK-NOT: @_f_v.__vertical_distance2f8
; CHECK: ret void
define void @__distancef2_test(<2 x float>* nocapture, <2 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %3 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %3, i32 0
  %extract7 = extractelement <8 x i32> %3, i32 1
  %extract8 = extractelement <8 x i32> %3, i32 2
  %extract9 = extractelement <8 x i32> %3, i32 3
  %extract10 = extractelement <8 x i32> %3, i32 4
  %extract11 = extractelement <8 x i32> %3, i32 5
  %extract12 = extractelement <8 x i32> %3, i32 6
  %extract13 = extractelement <8 x i32> %3, i32 7
  %4 = getelementptr <2 x float>* %0, i32 %extract
  %5 = getelementptr <2 x float>* %0, i32 %extract7
  %6 = getelementptr <2 x float>* %0, i32 %extract8
  %7 = getelementptr <2 x float>* %0, i32 %extract9
  %8 = getelementptr <2 x float>* %0, i32 %extract10
  %9 = getelementptr <2 x float>* %0, i32 %extract11
  %10 = getelementptr <2 x float>* %0, i32 %extract12
  %11 = getelementptr <2 x float>* %0, i32 %extract13
  %12 = load <2 x float>* %4
  %13 = load <2 x float>* %5
  %14 = load <2 x float>* %6
  %15 = load <2 x float>* %7
  %16 = load <2 x float>* %8
  %17 = load <2 x float>* %9
  %18 = load <2 x float>* %10
  %19 = load <2 x float>* %11
  %20 = extractelement <2 x float> %12, i32 0
  %21 = extractelement <2 x float> %13, i32 0
  %22 = extractelement <2 x float> %14, i32 0
  %23 = extractelement <2 x float> %15, i32 0
  %24 = extractelement <2 x float> %16, i32 0
  %25 = extractelement <2 x float> %17, i32 0
  %26 = extractelement <2 x float> %18, i32 0
  %27 = extractelement <2 x float> %19, i32 0
  %temp.vect52 = insertelement <8 x float> undef, float %20, i32 0
  %temp.vect53 = insertelement <8 x float> %temp.vect52, float %21, i32 1
  %temp.vect54 = insertelement <8 x float> %temp.vect53, float %22, i32 2
  %temp.vect55 = insertelement <8 x float> %temp.vect54, float %23, i32 3
  %temp.vect56 = insertelement <8 x float> %temp.vect55, float %24, i32 4
  %temp.vect57 = insertelement <8 x float> %temp.vect56, float %25, i32 5
  %temp.vect58 = insertelement <8 x float> %temp.vect57, float %26, i32 6
  %temp.vect59 = insertelement <8 x float> %temp.vect58, float %27, i32 7
  %28 = extractelement <2 x float> %12, i32 1
  %29 = extractelement <2 x float> %13, i32 1
  %30 = extractelement <2 x float> %14, i32 1
  %31 = extractelement <2 x float> %15, i32 1
  %32 = extractelement <2 x float> %16, i32 1
  %33 = extractelement <2 x float> %17, i32 1
  %34 = extractelement <2 x float> %18, i32 1
  %35 = extractelement <2 x float> %19, i32 1
  %temp.vect = insertelement <8 x float> undef, float %28, i32 0
  %temp.vect45 = insertelement <8 x float> %temp.vect, float %29, i32 1
  %temp.vect46 = insertelement <8 x float> %temp.vect45, float %30, i32 2
  %temp.vect47 = insertelement <8 x float> %temp.vect46, float %31, i32 3
  %temp.vect48 = insertelement <8 x float> %temp.vect47, float %32, i32 4
  %temp.vect49 = insertelement <8 x float> %temp.vect48, float %33, i32 5
  %temp.vect50 = insertelement <8 x float> %temp.vect49, float %34, i32 6
  %temp.vect51 = insertelement <8 x float> %temp.vect50, float %35, i32 7
  %36 = getelementptr <2 x float>* %1, i32 %extract
  %37 = getelementptr <2 x float>* %1, i32 %extract7
  %38 = getelementptr <2 x float>* %1, i32 %extract8
  %39 = getelementptr <2 x float>* %1, i32 %extract9
  %40 = getelementptr <2 x float>* %1, i32 %extract10
  %41 = getelementptr <2 x float>* %1, i32 %extract11
  %42 = getelementptr <2 x float>* %1, i32 %extract12
  %43 = getelementptr <2 x float>* %1, i32 %extract13
  %44 = load <2 x float>* %36
  %45 = load <2 x float>* %37
  %46 = load <2 x float>* %38
  %47 = load <2 x float>* %39
  %48 = load <2 x float>* %40
  %49 = load <2 x float>* %41
  %50 = load <2 x float>* %42
  %51 = load <2 x float>* %43
  %52 = extractelement <2 x float> %44, i32 0
  %53 = extractelement <2 x float> %45, i32 0
  %54 = extractelement <2 x float> %46, i32 0
  %55 = extractelement <2 x float> %47, i32 0
  %56 = extractelement <2 x float> %48, i32 0
  %57 = extractelement <2 x float> %49, i32 0
  %58 = extractelement <2 x float> %50, i32 0
  %59 = extractelement <2 x float> %51, i32 0
  %temp.vect69 = insertelement <8 x float> undef, float %52, i32 0
  %temp.vect70 = insertelement <8 x float> %temp.vect69, float %53, i32 1
  %temp.vect71 = insertelement <8 x float> %temp.vect70, float %54, i32 2
  %temp.vect72 = insertelement <8 x float> %temp.vect71, float %55, i32 3
  %temp.vect73 = insertelement <8 x float> %temp.vect72, float %56, i32 4
  %temp.vect74 = insertelement <8 x float> %temp.vect73, float %57, i32 5
  %temp.vect75 = insertelement <8 x float> %temp.vect74, float %58, i32 6
  %temp.vect76 = insertelement <8 x float> %temp.vect75, float %59, i32 7
  %60 = extractelement <2 x float> %44, i32 1
  %61 = extractelement <2 x float> %45, i32 1
  %62 = extractelement <2 x float> %46, i32 1
  %63 = extractelement <2 x float> %47, i32 1
  %64 = extractelement <2 x float> %48, i32 1
  %65 = extractelement <2 x float> %49, i32 1
  %66 = extractelement <2 x float> %50, i32 1
  %67 = extractelement <2 x float> %51, i32 1
  %temp.vect61 = insertelement <8 x float> undef, float %60, i32 0
  %temp.vect62 = insertelement <8 x float> %temp.vect61, float %61, i32 1
  %temp.vect63 = insertelement <8 x float> %temp.vect62, float %62, i32 2
  %temp.vect64 = insertelement <8 x float> %temp.vect63, float %63, i32 3
  %temp.vect65 = insertelement <8 x float> %temp.vect64, float %64, i32 4
  %temp.vect66 = insertelement <8 x float> %temp.vect65, float %65, i32 5
  %temp.vect67 = insertelement <8 x float> %temp.vect66, float %66, i32 6
  %temp.vect68 = insertelement <8 x float> %temp.vect67, float %67, i32 7
  %store.val = insertvalue [2 x <8 x float>] undef, <8 x float> %temp.vect59, 0
  %store.val60 = insertvalue [2 x <8 x float>] %store.val, <8 x float> %temp.vect51, 1
  %store.val77 = insertvalue [2 x <8 x float>] undef, <8 x float> %temp.vect76, 0
  %store.val78 = insertvalue [2 x <8 x float>] %store.val77, <8 x float> %temp.vect68, 1
  %68 = tail call <8 x float> @_f_v.__vertical_distance2f8([2 x <8 x float>] %store.val60, [2 x <8 x float>] %store.val78)
  %69 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %69 to <8 x float>*
  store <8 x float> %68, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__lengthf2_test
; CHECK-NOT: @_f_v.__vertical_length2f8
; CHECK: @__vertical_length2f8
; CHECK-NOT: @_f_v.__vertical_length2f8
; CHECK: ret void
define void @__lengthf2_test(<2 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %2 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %2, i32 0
  %extract3 = extractelement <8 x i32> %2, i32 1
  %extract4 = extractelement <8 x i32> %2, i32 2
  %extract5 = extractelement <8 x i32> %2, i32 3
  %extract6 = extractelement <8 x i32> %2, i32 4
  %extract7 = extractelement <8 x i32> %2, i32 5
  %extract8 = extractelement <8 x i32> %2, i32 6
  %extract9 = extractelement <8 x i32> %2, i32 7
  %3 = getelementptr <2 x float>* %0, i32 %extract
  %4 = getelementptr <2 x float>* %0, i32 %extract3
  %5 = getelementptr <2 x float>* %0, i32 %extract4
  %6 = getelementptr <2 x float>* %0, i32 %extract5
  %7 = getelementptr <2 x float>* %0, i32 %extract6
  %8 = getelementptr <2 x float>* %0, i32 %extract7
  %9 = getelementptr <2 x float>* %0, i32 %extract8
  %10 = getelementptr <2 x float>* %0, i32 %extract9
  %11 = load <2 x float>* %3
  %12 = load <2 x float>* %4
  %13 = load <2 x float>* %5
  %14 = load <2 x float>* %6
  %15 = load <2 x float>* %7
  %16 = load <2 x float>* %8
  %17 = load <2 x float>* %9
  %18 = load <2 x float>* %10
  %19 = extractelement <2 x float> %11, i32 0
  %20 = extractelement <2 x float> %12, i32 0
  %21 = extractelement <2 x float> %13, i32 0
  %22 = extractelement <2 x float> %14, i32 0
  %23 = extractelement <2 x float> %15, i32 0
  %24 = extractelement <2 x float> %16, i32 0
  %25 = extractelement <2 x float> %17, i32 0
  %26 = extractelement <2 x float> %18, i32 0
  %temp.vect32 = insertelement <8 x float> undef, float %19, i32 0
  %temp.vect33 = insertelement <8 x float> %temp.vect32, float %20, i32 1
  %temp.vect34 = insertelement <8 x float> %temp.vect33, float %21, i32 2
  %temp.vect35 = insertelement <8 x float> %temp.vect34, float %22, i32 3
  %temp.vect36 = insertelement <8 x float> %temp.vect35, float %23, i32 4
  %temp.vect37 = insertelement <8 x float> %temp.vect36, float %24, i32 5
  %temp.vect38 = insertelement <8 x float> %temp.vect37, float %25, i32 6
  %temp.vect39 = insertelement <8 x float> %temp.vect38, float %26, i32 7
  %27 = extractelement <2 x float> %11, i32 1
  %28 = extractelement <2 x float> %12, i32 1
  %29 = extractelement <2 x float> %13, i32 1
  %30 = extractelement <2 x float> %14, i32 1
  %31 = extractelement <2 x float> %15, i32 1
  %32 = extractelement <2 x float> %16, i32 1
  %33 = extractelement <2 x float> %17, i32 1
  %34 = extractelement <2 x float> %18, i32 1
  %temp.vect = insertelement <8 x float> undef, float %27, i32 0
  %temp.vect25 = insertelement <8 x float> %temp.vect, float %28, i32 1
  %temp.vect26 = insertelement <8 x float> %temp.vect25, float %29, i32 2
  %temp.vect27 = insertelement <8 x float> %temp.vect26, float %30, i32 3
  %temp.vect28 = insertelement <8 x float> %temp.vect27, float %31, i32 4
  %temp.vect29 = insertelement <8 x float> %temp.vect28, float %32, i32 5
  %temp.vect30 = insertelement <8 x float> %temp.vect29, float %33, i32 6
  %temp.vect31 = insertelement <8 x float> %temp.vect30, float %34, i32 7
  %store.val = insertvalue [2 x <8 x float>] undef, <8 x float> %temp.vect39, 0
  %store.val40 = insertvalue [2 x <8 x float>] %store.val, <8 x float> %temp.vect31, 1
  %35 = tail call <8 x float> @_f_v.__vertical_length2f8([2 x <8 x float>] %store.val40)
  %36 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %36 to <8 x float>*
  store <8 x float> %35, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__distancef3_test
; CHECK-NOT: @_f_v.__vertical_distance3f8
; CHECK: @__vertical_distance3f8
; CHECK-NOT: @_f_v.__vertical_distance3f8
; CHECK: ret void
define void @__distancef3_test(<3 x float>* nocapture, <3 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %3 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %3, i32 0
  %extract11 = extractelement <8 x i32> %3, i32 1
  %extract12 = extractelement <8 x i32> %3, i32 2
  %extract13 = extractelement <8 x i32> %3, i32 3
  %extract14 = extractelement <8 x i32> %3, i32 4
  %extract15 = extractelement <8 x i32> %3, i32 5
  %extract16 = extractelement <8 x i32> %3, i32 6
  %extract17 = extractelement <8 x i32> %3, i32 7
  %4 = getelementptr <3 x float>* %0, i32 %extract
  %5 = getelementptr <3 x float>* %0, i32 %extract11
  %6 = getelementptr <3 x float>* %0, i32 %extract12
  %7 = getelementptr <3 x float>* %0, i32 %extract13
  %8 = getelementptr <3 x float>* %0, i32 %extract14
  %9 = getelementptr <3 x float>* %0, i32 %extract15
  %10 = getelementptr <3 x float>* %0, i32 %extract16
  %11 = getelementptr <3 x float>* %0, i32 %extract17
  %12 = load <3 x float>* %4
  %13 = load <3 x float>* %5
  %14 = load <3 x float>* %6
  %15 = load <3 x float>* %7
  %16 = load <3 x float>* %8
  %17 = load <3 x float>* %9
  %18 = load <3 x float>* %10
  %19 = load <3 x float>* %11
  %20 = extractelement <3 x float> %12, i32 0
  %21 = extractelement <3 x float> %13, i32 0
  %22 = extractelement <3 x float> %14, i32 0
  %23 = extractelement <3 x float> %15, i32 0
  %24 = extractelement <3 x float> %16, i32 0
  %25 = extractelement <3 x float> %17, i32 0
  %26 = extractelement <3 x float> %18, i32 0
  %27 = extractelement <3 x float> %19, i32 0
  %temp.vect80 = insertelement <8 x float> undef, float %20, i32 0
  %temp.vect81 = insertelement <8 x float> %temp.vect80, float %21, i32 1
  %temp.vect82 = insertelement <8 x float> %temp.vect81, float %22, i32 2
  %temp.vect83 = insertelement <8 x float> %temp.vect82, float %23, i32 3
  %temp.vect84 = insertelement <8 x float> %temp.vect83, float %24, i32 4
  %temp.vect85 = insertelement <8 x float> %temp.vect84, float %25, i32 5
  %temp.vect86 = insertelement <8 x float> %temp.vect85, float %26, i32 6
  %temp.vect87 = insertelement <8 x float> %temp.vect86, float %27, i32 7
  %28 = extractelement <3 x float> %12, i32 1
  %29 = extractelement <3 x float> %13, i32 1
  %30 = extractelement <3 x float> %14, i32 1
  %31 = extractelement <3 x float> %15, i32 1
  %32 = extractelement <3 x float> %16, i32 1
  %33 = extractelement <3 x float> %17, i32 1
  %34 = extractelement <3 x float> %18, i32 1
  %35 = extractelement <3 x float> %19, i32 1
  %temp.vect72 = insertelement <8 x float> undef, float %28, i32 0
  %temp.vect73 = insertelement <8 x float> %temp.vect72, float %29, i32 1
  %temp.vect74 = insertelement <8 x float> %temp.vect73, float %30, i32 2
  %temp.vect75 = insertelement <8 x float> %temp.vect74, float %31, i32 3
  %temp.vect76 = insertelement <8 x float> %temp.vect75, float %32, i32 4
  %temp.vect77 = insertelement <8 x float> %temp.vect76, float %33, i32 5
  %temp.vect78 = insertelement <8 x float> %temp.vect77, float %34, i32 6
  %temp.vect79 = insertelement <8 x float> %temp.vect78, float %35, i32 7
  %36 = extractelement <3 x float> %12, i32 2
  %37 = extractelement <3 x float> %13, i32 2
  %38 = extractelement <3 x float> %14, i32 2
  %39 = extractelement <3 x float> %15, i32 2
  %40 = extractelement <3 x float> %16, i32 2
  %41 = extractelement <3 x float> %17, i32 2
  %42 = extractelement <3 x float> %18, i32 2
  %43 = extractelement <3 x float> %19, i32 2
  %temp.vect = insertelement <8 x float> undef, float %36, i32 0
  %temp.vect65 = insertelement <8 x float> %temp.vect, float %37, i32 1
  %temp.vect66 = insertelement <8 x float> %temp.vect65, float %38, i32 2
  %temp.vect67 = insertelement <8 x float> %temp.vect66, float %39, i32 3
  %temp.vect68 = insertelement <8 x float> %temp.vect67, float %40, i32 4
  %temp.vect69 = insertelement <8 x float> %temp.vect68, float %41, i32 5
  %temp.vect70 = insertelement <8 x float> %temp.vect69, float %42, i32 6
  %temp.vect71 = insertelement <8 x float> %temp.vect70, float %43, i32 7
  %44 = getelementptr <3 x float>* %1, i32 %extract
  %45 = getelementptr <3 x float>* %1, i32 %extract11
  %46 = getelementptr <3 x float>* %1, i32 %extract12
  %47 = getelementptr <3 x float>* %1, i32 %extract13
  %48 = getelementptr <3 x float>* %1, i32 %extract14
  %49 = getelementptr <3 x float>* %1, i32 %extract15
  %50 = getelementptr <3 x float>* %1, i32 %extract16
  %51 = getelementptr <3 x float>* %1, i32 %extract17
  %52 = load <3 x float>* %44
  %53 = load <3 x float>* %45
  %54 = load <3 x float>* %46
  %55 = load <3 x float>* %47
  %56 = load <3 x float>* %48
  %57 = load <3 x float>* %49
  %58 = load <3 x float>* %50
  %59 = load <3 x float>* %51
  %60 = extractelement <3 x float> %52, i32 0
  %61 = extractelement <3 x float> %53, i32 0
  %62 = extractelement <3 x float> %54, i32 0
  %63 = extractelement <3 x float> %55, i32 0
  %64 = extractelement <3 x float> %56, i32 0
  %65 = extractelement <3 x float> %57, i32 0
  %66 = extractelement <3 x float> %58, i32 0
  %67 = extractelement <3 x float> %59, i32 0
  %temp.vect106 = insertelement <8 x float> undef, float %60, i32 0
  %temp.vect107 = insertelement <8 x float> %temp.vect106, float %61, i32 1
  %temp.vect108 = insertelement <8 x float> %temp.vect107, float %62, i32 2
  %temp.vect109 = insertelement <8 x float> %temp.vect108, float %63, i32 3
  %temp.vect110 = insertelement <8 x float> %temp.vect109, float %64, i32 4
  %temp.vect111 = insertelement <8 x float> %temp.vect110, float %65, i32 5
  %temp.vect112 = insertelement <8 x float> %temp.vect111, float %66, i32 6
  %temp.vect113 = insertelement <8 x float> %temp.vect112, float %67, i32 7
  %68 = extractelement <3 x float> %52, i32 1
  %69 = extractelement <3 x float> %53, i32 1
  %70 = extractelement <3 x float> %54, i32 1
  %71 = extractelement <3 x float> %55, i32 1
  %72 = extractelement <3 x float> %56, i32 1
  %73 = extractelement <3 x float> %57, i32 1
  %74 = extractelement <3 x float> %58, i32 1
  %75 = extractelement <3 x float> %59, i32 1
  %temp.vect98 = insertelement <8 x float> undef, float %68, i32 0
  %temp.vect99 = insertelement <8 x float> %temp.vect98, float %69, i32 1
  %temp.vect100 = insertelement <8 x float> %temp.vect99, float %70, i32 2
  %temp.vect101 = insertelement <8 x float> %temp.vect100, float %71, i32 3
  %temp.vect102 = insertelement <8 x float> %temp.vect101, float %72, i32 4
  %temp.vect103 = insertelement <8 x float> %temp.vect102, float %73, i32 5
  %temp.vect104 = insertelement <8 x float> %temp.vect103, float %74, i32 6
  %temp.vect105 = insertelement <8 x float> %temp.vect104, float %75, i32 7
  %76 = extractelement <3 x float> %52, i32 2
  %77 = extractelement <3 x float> %53, i32 2
  %78 = extractelement <3 x float> %54, i32 2
  %79 = extractelement <3 x float> %55, i32 2
  %80 = extractelement <3 x float> %56, i32 2
  %81 = extractelement <3 x float> %57, i32 2
  %82 = extractelement <3 x float> %58, i32 2
  %83 = extractelement <3 x float> %59, i32 2
  %temp.vect90 = insertelement <8 x float> undef, float %76, i32 0
  %temp.vect91 = insertelement <8 x float> %temp.vect90, float %77, i32 1
  %temp.vect92 = insertelement <8 x float> %temp.vect91, float %78, i32 2
  %temp.vect93 = insertelement <8 x float> %temp.vect92, float %79, i32 3
  %temp.vect94 = insertelement <8 x float> %temp.vect93, float %80, i32 4
  %temp.vect95 = insertelement <8 x float> %temp.vect94, float %81, i32 5
  %temp.vect96 = insertelement <8 x float> %temp.vect95, float %82, i32 6
  %temp.vect97 = insertelement <8 x float> %temp.vect96, float %83, i32 7
  %store.val = insertvalue [3 x <8 x float>] undef, <8 x float> %temp.vect87, 0
  %store.val88 = insertvalue [3 x <8 x float>] %store.val, <8 x float> %temp.vect79, 1
  %store.val89 = insertvalue [3 x <8 x float>] %store.val88, <8 x float> %temp.vect71, 2
  %store.val114 = insertvalue [3 x <8 x float>] undef, <8 x float> %temp.vect113, 0
  %store.val115 = insertvalue [3 x <8 x float>] %store.val114, <8 x float> %temp.vect105, 1
  %store.val116 = insertvalue [3 x <8 x float>] %store.val115, <8 x float> %temp.vect97, 2
  %84 = tail call <8 x float> @_f_v.__vertical_distance3f8([3 x <8 x float>] %store.val89, [3 x <8 x float>] %store.val116)
  %85 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %85 to <8 x float>*
  store <8 x float> %84, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__lengthf3_test
; CHECK-NOT: @_f_v.__vertical_length3f8
; CHECK: @__vertical_length3f8
; CHECK-NOT: @_f_v.__vertical_length3f8
; CHECK: ret void
define void @__lengthf3_test(<3 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %2 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %2, i32 0
  %extract5 = extractelement <8 x i32> %2, i32 1
  %extract6 = extractelement <8 x i32> %2, i32 2
  %extract7 = extractelement <8 x i32> %2, i32 3
  %extract8 = extractelement <8 x i32> %2, i32 4
  %extract9 = extractelement <8 x i32> %2, i32 5
  %extract10 = extractelement <8 x i32> %2, i32 6
  %extract11 = extractelement <8 x i32> %2, i32 7
  %3 = getelementptr <3 x float>* %0, i32 %extract
  %4 = getelementptr <3 x float>* %0, i32 %extract5
  %5 = getelementptr <3 x float>* %0, i32 %extract6
  %6 = getelementptr <3 x float>* %0, i32 %extract7
  %7 = getelementptr <3 x float>* %0, i32 %extract8
  %8 = getelementptr <3 x float>* %0, i32 %extract9
  %9 = getelementptr <3 x float>* %0, i32 %extract10
  %10 = getelementptr <3 x float>* %0, i32 %extract11
  %11 = load <3 x float>* %3
  %12 = load <3 x float>* %4
  %13 = load <3 x float>* %5
  %14 = load <3 x float>* %6
  %15 = load <3 x float>* %7
  %16 = load <3 x float>* %8
  %17 = load <3 x float>* %9
  %18 = load <3 x float>* %10
  %19 = extractelement <3 x float> %11, i32 0
  %20 = extractelement <3 x float> %12, i32 0
  %21 = extractelement <3 x float> %13, i32 0
  %22 = extractelement <3 x float> %14, i32 0
  %23 = extractelement <3 x float> %15, i32 0
  %24 = extractelement <3 x float> %16, i32 0
  %25 = extractelement <3 x float> %17, i32 0
  %26 = extractelement <3 x float> %18, i32 0
  %temp.vect50 = insertelement <8 x float> undef, float %19, i32 0
  %temp.vect51 = insertelement <8 x float> %temp.vect50, float %20, i32 1
  %temp.vect52 = insertelement <8 x float> %temp.vect51, float %21, i32 2
  %temp.vect53 = insertelement <8 x float> %temp.vect52, float %22, i32 3
  %temp.vect54 = insertelement <8 x float> %temp.vect53, float %23, i32 4
  %temp.vect55 = insertelement <8 x float> %temp.vect54, float %24, i32 5
  %temp.vect56 = insertelement <8 x float> %temp.vect55, float %25, i32 6
  %temp.vect57 = insertelement <8 x float> %temp.vect56, float %26, i32 7
  %27 = extractelement <3 x float> %11, i32 1
  %28 = extractelement <3 x float> %12, i32 1
  %29 = extractelement <3 x float> %13, i32 1
  %30 = extractelement <3 x float> %14, i32 1
  %31 = extractelement <3 x float> %15, i32 1
  %32 = extractelement <3 x float> %16, i32 1
  %33 = extractelement <3 x float> %17, i32 1
  %34 = extractelement <3 x float> %18, i32 1
  %temp.vect42 = insertelement <8 x float> undef, float %27, i32 0
  %temp.vect43 = insertelement <8 x float> %temp.vect42, float %28, i32 1
  %temp.vect44 = insertelement <8 x float> %temp.vect43, float %29, i32 2
  %temp.vect45 = insertelement <8 x float> %temp.vect44, float %30, i32 3
  %temp.vect46 = insertelement <8 x float> %temp.vect45, float %31, i32 4
  %temp.vect47 = insertelement <8 x float> %temp.vect46, float %32, i32 5
  %temp.vect48 = insertelement <8 x float> %temp.vect47, float %33, i32 6
  %temp.vect49 = insertelement <8 x float> %temp.vect48, float %34, i32 7
  %35 = extractelement <3 x float> %11, i32 2
  %36 = extractelement <3 x float> %12, i32 2
  %37 = extractelement <3 x float> %13, i32 2
  %38 = extractelement <3 x float> %14, i32 2
  %39 = extractelement <3 x float> %15, i32 2
  %40 = extractelement <3 x float> %16, i32 2
  %41 = extractelement <3 x float> %17, i32 2
  %42 = extractelement <3 x float> %18, i32 2
  %temp.vect = insertelement <8 x float> undef, float %35, i32 0
  %temp.vect35 = insertelement <8 x float> %temp.vect, float %36, i32 1
  %temp.vect36 = insertelement <8 x float> %temp.vect35, float %37, i32 2
  %temp.vect37 = insertelement <8 x float> %temp.vect36, float %38, i32 3
  %temp.vect38 = insertelement <8 x float> %temp.vect37, float %39, i32 4
  %temp.vect39 = insertelement <8 x float> %temp.vect38, float %40, i32 5
  %temp.vect40 = insertelement <8 x float> %temp.vect39, float %41, i32 6
  %temp.vect41 = insertelement <8 x float> %temp.vect40, float %42, i32 7
  %store.val = insertvalue [3 x <8 x float>] undef, <8 x float> %temp.vect57, 0
  %store.val58 = insertvalue [3 x <8 x float>] %store.val, <8 x float> %temp.vect49, 1
  %store.val59 = insertvalue [3 x <8 x float>] %store.val58, <8 x float> %temp.vect41, 2
  %43 = tail call <8 x float> @_f_v.__vertical_length3f8([3 x <8 x float>] %store.val59)
  %44 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %44 to <8 x float>*
  store <8 x float> %43, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__distancef4_test
; CHECK-NOT: @_f_v.__vertical_distance4f8
; CHECK: @__vertical_distance4f8
; CHECK-NOT: @_f_v.__vertical_distance4f8
; CHECK: ret void
define void @__distancef4_test(<4 x float>* nocapture, <4 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %3 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %3, i32 0
  %extract15 = extractelement <8 x i32> %3, i32 1
  %extract16 = extractelement <8 x i32> %3, i32 2
  %extract17 = extractelement <8 x i32> %3, i32 3
  %extract18 = extractelement <8 x i32> %3, i32 4
  %extract19 = extractelement <8 x i32> %3, i32 5
  %extract20 = extractelement <8 x i32> %3, i32 6
  %extract21 = extractelement <8 x i32> %3, i32 7
  %4 = getelementptr <4 x float>* %0, i32 %extract
  %5 = getelementptr <4 x float>* %0, i32 %extract15
  %6 = getelementptr <4 x float>* %0, i32 %extract16
  %7 = getelementptr <4 x float>* %0, i32 %extract17
  %8 = getelementptr <4 x float>* %0, i32 %extract18
  %9 = getelementptr <4 x float>* %0, i32 %extract19
  %10 = getelementptr <4 x float>* %0, i32 %extract20
  %11 = getelementptr <4 x float>* %0, i32 %extract21
  %12 = load <4 x float>* %4
  %13 = load <4 x float>* %5
  %14 = load <4 x float>* %6
  %15 = load <4 x float>* %7
  %16 = load <4 x float>* %8
  %17 = load <4 x float>* %9
  %18 = load <4 x float>* %10
  %19 = load <4 x float>* %11
  %extend_vec = shufflevector <4 x float> %12, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec22 = shufflevector <4 x float> %13, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec23 = shufflevector <4 x float> %14, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec24 = shufflevector <4 x float> %15, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec25 = shufflevector <4 x float> %16, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec26 = shufflevector <4 x float> %17, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec27 = shufflevector <4 x float> %18, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec28 = shufflevector <4 x float> %19, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %Seq_128_0 = shufflevector <8 x float> %extend_vec, <8 x float> %extend_vec25, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_1 = shufflevector <8 x float> %extend_vec22, <8 x float> %extend_vec26, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_2 = shufflevector <8 x float> %extend_vec23, <8 x float> %extend_vec27, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_3 = shufflevector <8 x float> %extend_vec24, <8 x float> %extend_vec28, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_64_0 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_64_1 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_32_0 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_154 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %Seq_64_279 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_64_380 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_32_287 = shufflevector <8 x float> %Seq_64_279, <8 x float> %Seq_64_380, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_3120 = shufflevector <8 x float> %Seq_64_279, <8 x float> %Seq_64_380, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %20 = getelementptr <4 x float>* %1, i32 %extract
  %21 = getelementptr <4 x float>* %1, i32 %extract15
  %22 = getelementptr <4 x float>* %1, i32 %extract16
  %23 = getelementptr <4 x float>* %1, i32 %extract17
  %24 = getelementptr <4 x float>* %1, i32 %extract18
  %25 = getelementptr <4 x float>* %1, i32 %extract19
  %26 = getelementptr <4 x float>* %1, i32 %extract20
  %27 = getelementptr <4 x float>* %1, i32 %extract21
  %28 = load <4 x float>* %20
  %29 = load <4 x float>* %21
  %30 = load <4 x float>* %22
  %31 = load <4 x float>* %23
  %32 = load <4 x float>* %24
  %33 = load <4 x float>* %25
  %34 = load <4 x float>* %26
  %35 = load <4 x float>* %27
  %extend_vec125 = shufflevector <4 x float> %28, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec126 = shufflevector <4 x float> %29, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec127 = shufflevector <4 x float> %30, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec128 = shufflevector <4 x float> %31, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec129 = shufflevector <4 x float> %32, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec130 = shufflevector <4 x float> %33, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec131 = shufflevector <4 x float> %34, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec132 = shufflevector <4 x float> %35, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %Seq_128_0133 = shufflevector <8 x float> %extend_vec125, <8 x float> %extend_vec129, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_1134 = shufflevector <8 x float> %extend_vec126, <8 x float> %extend_vec130, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_2135 = shufflevector <8 x float> %extend_vec127, <8 x float> %extend_vec131, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_3136 = shufflevector <8 x float> %extend_vec128, <8 x float> %extend_vec132, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_64_0141 = shufflevector <8 x float> %Seq_128_0133, <8 x float> %Seq_128_2135, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_64_1142 = shufflevector <8 x float> %Seq_128_1134, <8 x float> %Seq_128_3136, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_32_0149 = shufflevector <8 x float> %Seq_64_0141, <8 x float> %Seq_64_1142, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_1182 = shufflevector <8 x float> %Seq_64_0141, <8 x float> %Seq_64_1142, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %Seq_64_2207 = shufflevector <8 x float> %Seq_128_0133, <8 x float> %Seq_128_2135, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_64_3208 = shufflevector <8 x float> %Seq_128_1134, <8 x float> %Seq_128_3136, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_32_2215 = shufflevector <8 x float> %Seq_64_2207, <8 x float> %Seq_64_3208, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_3248 = shufflevector <8 x float> %Seq_64_2207, <8 x float> %Seq_64_3208, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %store.val = insertvalue [4 x <8 x float>] undef, <8 x float> %Seq_32_0, 0
  %store.val282 = insertvalue [4 x <8 x float>] %store.val, <8 x float> %Seq_32_154, 1
  %store.val283 = insertvalue [4 x <8 x float>] %store.val282, <8 x float> %Seq_32_287, 2
  %store.val284 = insertvalue [4 x <8 x float>] %store.val283, <8 x float> %Seq_32_3120, 3
  %store.val285 = insertvalue [4 x <8 x float>] undef, <8 x float> %Seq_32_0149, 0
  %store.val286 = insertvalue [4 x <8 x float>] %store.val285, <8 x float> %Seq_32_1182, 1
  %store.val287 = insertvalue [4 x <8 x float>] %store.val286, <8 x float> %Seq_32_2215, 2
  %store.val288 = insertvalue [4 x <8 x float>] %store.val287, <8 x float> %Seq_32_3248, 3
  %36 = tail call <8 x float> @_f_v.__vertical_distance4f8([4 x <8 x float>] %store.val284, [4 x <8 x float>] %store.val288)
  %37 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %37 to <8 x float>*
  store <8 x float> %36, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__lengthf4_test
; CHECK-NOT: @_f_v.__vertical_length4f8
; CHECK: @__vertical_length4f8
; CHECK-NOT: @_f_v.__vertical_length4f8
; CHECK: ret void
define void @__lengthf4_test(<4 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %2 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %2, i32 0
  %extract7 = extractelement <8 x i32> %2, i32 1
  %extract8 = extractelement <8 x i32> %2, i32 2
  %extract9 = extractelement <8 x i32> %2, i32 3
  %extract10 = extractelement <8 x i32> %2, i32 4
  %extract11 = extractelement <8 x i32> %2, i32 5
  %extract12 = extractelement <8 x i32> %2, i32 6
  %extract13 = extractelement <8 x i32> %2, i32 7
  %3 = getelementptr <4 x float>* %0, i32 %extract
  %4 = getelementptr <4 x float>* %0, i32 %extract7
  %5 = getelementptr <4 x float>* %0, i32 %extract8
  %6 = getelementptr <4 x float>* %0, i32 %extract9
  %7 = getelementptr <4 x float>* %0, i32 %extract10
  %8 = getelementptr <4 x float>* %0, i32 %extract11
  %9 = getelementptr <4 x float>* %0, i32 %extract12
  %10 = getelementptr <4 x float>* %0, i32 %extract13
  %11 = load <4 x float>* %3
  %12 = load <4 x float>* %4
  %13 = load <4 x float>* %5
  %14 = load <4 x float>* %6
  %15 = load <4 x float>* %7
  %16 = load <4 x float>* %8
  %17 = load <4 x float>* %9
  %18 = load <4 x float>* %10
  %extend_vec = shufflevector <4 x float> %11, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec14 = shufflevector <4 x float> %12, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec15 = shufflevector <4 x float> %13, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec16 = shufflevector <4 x float> %14, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec17 = shufflevector <4 x float> %15, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec18 = shufflevector <4 x float> %16, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec19 = shufflevector <4 x float> %17, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec20 = shufflevector <4 x float> %18, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %Seq_128_0 = shufflevector <8 x float> %extend_vec, <8 x float> %extend_vec17, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_1 = shufflevector <8 x float> %extend_vec14, <8 x float> %extend_vec18, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_2 = shufflevector <8 x float> %extend_vec15, <8 x float> %extend_vec19, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_3 = shufflevector <8 x float> %extend_vec16, <8 x float> %extend_vec20, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_64_0 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_64_1 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_32_0 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_146 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %Seq_64_271 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_64_372 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_32_279 = shufflevector <8 x float> %Seq_64_271, <8 x float> %Seq_64_372, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_3112 = shufflevector <8 x float> %Seq_64_271, <8 x float> %Seq_64_372, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %store.val = insertvalue [4 x <8 x float>] undef, <8 x float> %Seq_32_0, 0
  %store.val130 = insertvalue [4 x <8 x float>] %store.val, <8 x float> %Seq_32_146, 1
  %store.val131 = insertvalue [4 x <8 x float>] %store.val130, <8 x float> %Seq_32_279, 2
  %store.val132 = insertvalue [4 x <8 x float>] %store.val131, <8 x float> %Seq_32_3112, 3
  %19 = tail call <8 x float> @_f_v.__vertical_length4f8([4 x <8 x float>] %store.val132)
  %20 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %20 to <8 x float>*
  store <8 x float> %19, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__dotf_test
; CHECK-NOT: @_f_v.__vertical_dot1f8
; CHECK: @__vertical_dot1f8
; CHECK-NOT: @_f_v.__vertical_dot1f8
; CHECK: ret void
define void @__dotf_test(float* nocapture, float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %3 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %3 to <8 x float>*
  %load_arg8 = load <8 x float>* %ptrTypeCast, align 4
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast9 = bitcast float* %4 to <8 x float>*
  %load_arg210 = load <8 x float>* %ptrTypeCast9, align 4
  %5 = tail call <8 x float> @_f_v.__vertical_dot1f8(<8 x float> %load_arg8, <8 x float> %load_arg210)
  %6 = getelementptr float* %2, i32 %gid
  %ptrTypeCast11 = bitcast float* %6 to <8 x float>*
  store <8 x float> %5, <8 x float>* %ptrTypeCast11, align 4
  ret void
}



; CHECK: @__dotf2_test
; CHECK-NOT: @_f_v.__vertical_dot2f8
; CHECK: @__vertical_dot2f8
; CHECK-NOT: @_f_v.__vertical_dot2f8
; CHECK: ret void
define void @__dotf2_test(<2 x float>* nocapture, <2 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %3 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %3, i32 0
  %extract7 = extractelement <8 x i32> %3, i32 1
  %extract8 = extractelement <8 x i32> %3, i32 2
  %extract9 = extractelement <8 x i32> %3, i32 3
  %extract10 = extractelement <8 x i32> %3, i32 4
  %extract11 = extractelement <8 x i32> %3, i32 5
  %extract12 = extractelement <8 x i32> %3, i32 6
  %extract13 = extractelement <8 x i32> %3, i32 7
  %4 = getelementptr <2 x float>* %0, i32 %extract
  %5 = getelementptr <2 x float>* %0, i32 %extract7
  %6 = getelementptr <2 x float>* %0, i32 %extract8
  %7 = getelementptr <2 x float>* %0, i32 %extract9
  %8 = getelementptr <2 x float>* %0, i32 %extract10
  %9 = getelementptr <2 x float>* %0, i32 %extract11
  %10 = getelementptr <2 x float>* %0, i32 %extract12
  %11 = getelementptr <2 x float>* %0, i32 %extract13
  %12 = load <2 x float>* %4
  %13 = load <2 x float>* %5
  %14 = load <2 x float>* %6
  %15 = load <2 x float>* %7
  %16 = load <2 x float>* %8
  %17 = load <2 x float>* %9
  %18 = load <2 x float>* %10
  %19 = load <2 x float>* %11
  %20 = extractelement <2 x float> %12, i32 0
  %21 = extractelement <2 x float> %13, i32 0
  %22 = extractelement <2 x float> %14, i32 0
  %23 = extractelement <2 x float> %15, i32 0
  %24 = extractelement <2 x float> %16, i32 0
  %25 = extractelement <2 x float> %17, i32 0
  %26 = extractelement <2 x float> %18, i32 0
  %27 = extractelement <2 x float> %19, i32 0
  %temp.vect52 = insertelement <8 x float> undef, float %20, i32 0
  %temp.vect53 = insertelement <8 x float> %temp.vect52, float %21, i32 1
  %temp.vect54 = insertelement <8 x float> %temp.vect53, float %22, i32 2
  %temp.vect55 = insertelement <8 x float> %temp.vect54, float %23, i32 3
  %temp.vect56 = insertelement <8 x float> %temp.vect55, float %24, i32 4
  %temp.vect57 = insertelement <8 x float> %temp.vect56, float %25, i32 5
  %temp.vect58 = insertelement <8 x float> %temp.vect57, float %26, i32 6
  %temp.vect59 = insertelement <8 x float> %temp.vect58, float %27, i32 7
  %28 = extractelement <2 x float> %12, i32 1
  %29 = extractelement <2 x float> %13, i32 1
  %30 = extractelement <2 x float> %14, i32 1
  %31 = extractelement <2 x float> %15, i32 1
  %32 = extractelement <2 x float> %16, i32 1
  %33 = extractelement <2 x float> %17, i32 1
  %34 = extractelement <2 x float> %18, i32 1
  %35 = extractelement <2 x float> %19, i32 1
  %temp.vect = insertelement <8 x float> undef, float %28, i32 0
  %temp.vect45 = insertelement <8 x float> %temp.vect, float %29, i32 1
  %temp.vect46 = insertelement <8 x float> %temp.vect45, float %30, i32 2
  %temp.vect47 = insertelement <8 x float> %temp.vect46, float %31, i32 3
  %temp.vect48 = insertelement <8 x float> %temp.vect47, float %32, i32 4
  %temp.vect49 = insertelement <8 x float> %temp.vect48, float %33, i32 5
  %temp.vect50 = insertelement <8 x float> %temp.vect49, float %34, i32 6
  %temp.vect51 = insertelement <8 x float> %temp.vect50, float %35, i32 7
  %36 = getelementptr <2 x float>* %1, i32 %extract
  %37 = getelementptr <2 x float>* %1, i32 %extract7
  %38 = getelementptr <2 x float>* %1, i32 %extract8
  %39 = getelementptr <2 x float>* %1, i32 %extract9
  %40 = getelementptr <2 x float>* %1, i32 %extract10
  %41 = getelementptr <2 x float>* %1, i32 %extract11
  %42 = getelementptr <2 x float>* %1, i32 %extract12
  %43 = getelementptr <2 x float>* %1, i32 %extract13
  %44 = load <2 x float>* %36
  %45 = load <2 x float>* %37
  %46 = load <2 x float>* %38
  %47 = load <2 x float>* %39
  %48 = load <2 x float>* %40
  %49 = load <2 x float>* %41
  %50 = load <2 x float>* %42
  %51 = load <2 x float>* %43
  %52 = extractelement <2 x float> %44, i32 0
  %53 = extractelement <2 x float> %45, i32 0
  %54 = extractelement <2 x float> %46, i32 0
  %55 = extractelement <2 x float> %47, i32 0
  %56 = extractelement <2 x float> %48, i32 0
  %57 = extractelement <2 x float> %49, i32 0
  %58 = extractelement <2 x float> %50, i32 0
  %59 = extractelement <2 x float> %51, i32 0
  %temp.vect69 = insertelement <8 x float> undef, float %52, i32 0
  %temp.vect70 = insertelement <8 x float> %temp.vect69, float %53, i32 1
  %temp.vect71 = insertelement <8 x float> %temp.vect70, float %54, i32 2
  %temp.vect72 = insertelement <8 x float> %temp.vect71, float %55, i32 3
  %temp.vect73 = insertelement <8 x float> %temp.vect72, float %56, i32 4
  %temp.vect74 = insertelement <8 x float> %temp.vect73, float %57, i32 5
  %temp.vect75 = insertelement <8 x float> %temp.vect74, float %58, i32 6
  %temp.vect76 = insertelement <8 x float> %temp.vect75, float %59, i32 7
  %60 = extractelement <2 x float> %44, i32 1
  %61 = extractelement <2 x float> %45, i32 1
  %62 = extractelement <2 x float> %46, i32 1
  %63 = extractelement <2 x float> %47, i32 1
  %64 = extractelement <2 x float> %48, i32 1
  %65 = extractelement <2 x float> %49, i32 1
  %66 = extractelement <2 x float> %50, i32 1
  %67 = extractelement <2 x float> %51, i32 1
  %temp.vect61 = insertelement <8 x float> undef, float %60, i32 0
  %temp.vect62 = insertelement <8 x float> %temp.vect61, float %61, i32 1
  %temp.vect63 = insertelement <8 x float> %temp.vect62, float %62, i32 2
  %temp.vect64 = insertelement <8 x float> %temp.vect63, float %63, i32 3
  %temp.vect65 = insertelement <8 x float> %temp.vect64, float %64, i32 4
  %temp.vect66 = insertelement <8 x float> %temp.vect65, float %65, i32 5
  %temp.vect67 = insertelement <8 x float> %temp.vect66, float %66, i32 6
  %temp.vect68 = insertelement <8 x float> %temp.vect67, float %67, i32 7
  %store.val = insertvalue [2 x <8 x float>] undef, <8 x float> %temp.vect59, 0
  %store.val60 = insertvalue [2 x <8 x float>] %store.val, <8 x float> %temp.vect51, 1
  %store.val77 = insertvalue [2 x <8 x float>] undef, <8 x float> %temp.vect76, 0
  %store.val78 = insertvalue [2 x <8 x float>] %store.val77, <8 x float> %temp.vect68, 1
  %68 = tail call <8 x float> @_f_v.__vertical_dot2f8([2 x <8 x float>] %store.val60, [2 x <8 x float>] %store.val78)
  %69 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %69 to <8 x float>*
  store <8 x float> %68, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__dotf3_test
; CHECK-NOT: @_f_v.__vertical_dot3f8
; CHECK: @__vertical_dot3f8
; CHECK-NOT: @_f_v.__vertical_dot3f8
; CHECK: ret void
define void @__dotf3_test(<3 x float>* nocapture, <3 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %3 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %3, i32 0
  %extract11 = extractelement <8 x i32> %3, i32 1
  %extract12 = extractelement <8 x i32> %3, i32 2
  %extract13 = extractelement <8 x i32> %3, i32 3
  %extract14 = extractelement <8 x i32> %3, i32 4
  %extract15 = extractelement <8 x i32> %3, i32 5
  %extract16 = extractelement <8 x i32> %3, i32 6
  %extract17 = extractelement <8 x i32> %3, i32 7
  %4 = getelementptr <3 x float>* %0, i32 %extract
  %5 = getelementptr <3 x float>* %0, i32 %extract11
  %6 = getelementptr <3 x float>* %0, i32 %extract12
  %7 = getelementptr <3 x float>* %0, i32 %extract13
  %8 = getelementptr <3 x float>* %0, i32 %extract14
  %9 = getelementptr <3 x float>* %0, i32 %extract15
  %10 = getelementptr <3 x float>* %0, i32 %extract16
  %11 = getelementptr <3 x float>* %0, i32 %extract17
  %12 = load <3 x float>* %4
  %13 = load <3 x float>* %5
  %14 = load <3 x float>* %6
  %15 = load <3 x float>* %7
  %16 = load <3 x float>* %8
  %17 = load <3 x float>* %9
  %18 = load <3 x float>* %10
  %19 = load <3 x float>* %11
  %20 = extractelement <3 x float> %12, i32 0
  %21 = extractelement <3 x float> %13, i32 0
  %22 = extractelement <3 x float> %14, i32 0
  %23 = extractelement <3 x float> %15, i32 0
  %24 = extractelement <3 x float> %16, i32 0
  %25 = extractelement <3 x float> %17, i32 0
  %26 = extractelement <3 x float> %18, i32 0
  %27 = extractelement <3 x float> %19, i32 0
  %temp.vect80 = insertelement <8 x float> undef, float %20, i32 0
  %temp.vect81 = insertelement <8 x float> %temp.vect80, float %21, i32 1
  %temp.vect82 = insertelement <8 x float> %temp.vect81, float %22, i32 2
  %temp.vect83 = insertelement <8 x float> %temp.vect82, float %23, i32 3
  %temp.vect84 = insertelement <8 x float> %temp.vect83, float %24, i32 4
  %temp.vect85 = insertelement <8 x float> %temp.vect84, float %25, i32 5
  %temp.vect86 = insertelement <8 x float> %temp.vect85, float %26, i32 6
  %temp.vect87 = insertelement <8 x float> %temp.vect86, float %27, i32 7
  %28 = extractelement <3 x float> %12, i32 1
  %29 = extractelement <3 x float> %13, i32 1
  %30 = extractelement <3 x float> %14, i32 1
  %31 = extractelement <3 x float> %15, i32 1
  %32 = extractelement <3 x float> %16, i32 1
  %33 = extractelement <3 x float> %17, i32 1
  %34 = extractelement <3 x float> %18, i32 1
  %35 = extractelement <3 x float> %19, i32 1
  %temp.vect72 = insertelement <8 x float> undef, float %28, i32 0
  %temp.vect73 = insertelement <8 x float> %temp.vect72, float %29, i32 1
  %temp.vect74 = insertelement <8 x float> %temp.vect73, float %30, i32 2
  %temp.vect75 = insertelement <8 x float> %temp.vect74, float %31, i32 3
  %temp.vect76 = insertelement <8 x float> %temp.vect75, float %32, i32 4
  %temp.vect77 = insertelement <8 x float> %temp.vect76, float %33, i32 5
  %temp.vect78 = insertelement <8 x float> %temp.vect77, float %34, i32 6
  %temp.vect79 = insertelement <8 x float> %temp.vect78, float %35, i32 7
  %36 = extractelement <3 x float> %12, i32 2
  %37 = extractelement <3 x float> %13, i32 2
  %38 = extractelement <3 x float> %14, i32 2
  %39 = extractelement <3 x float> %15, i32 2
  %40 = extractelement <3 x float> %16, i32 2
  %41 = extractelement <3 x float> %17, i32 2
  %42 = extractelement <3 x float> %18, i32 2
  %43 = extractelement <3 x float> %19, i32 2
  %temp.vect = insertelement <8 x float> undef, float %36, i32 0
  %temp.vect65 = insertelement <8 x float> %temp.vect, float %37, i32 1
  %temp.vect66 = insertelement <8 x float> %temp.vect65, float %38, i32 2
  %temp.vect67 = insertelement <8 x float> %temp.vect66, float %39, i32 3
  %temp.vect68 = insertelement <8 x float> %temp.vect67, float %40, i32 4
  %temp.vect69 = insertelement <8 x float> %temp.vect68, float %41, i32 5
  %temp.vect70 = insertelement <8 x float> %temp.vect69, float %42, i32 6
  %temp.vect71 = insertelement <8 x float> %temp.vect70, float %43, i32 7
  %44 = getelementptr <3 x float>* %1, i32 %extract
  %45 = getelementptr <3 x float>* %1, i32 %extract11
  %46 = getelementptr <3 x float>* %1, i32 %extract12
  %47 = getelementptr <3 x float>* %1, i32 %extract13
  %48 = getelementptr <3 x float>* %1, i32 %extract14
  %49 = getelementptr <3 x float>* %1, i32 %extract15
  %50 = getelementptr <3 x float>* %1, i32 %extract16
  %51 = getelementptr <3 x float>* %1, i32 %extract17
  %52 = load <3 x float>* %44
  %53 = load <3 x float>* %45
  %54 = load <3 x float>* %46
  %55 = load <3 x float>* %47
  %56 = load <3 x float>* %48
  %57 = load <3 x float>* %49
  %58 = load <3 x float>* %50
  %59 = load <3 x float>* %51
  %60 = extractelement <3 x float> %52, i32 0
  %61 = extractelement <3 x float> %53, i32 0
  %62 = extractelement <3 x float> %54, i32 0
  %63 = extractelement <3 x float> %55, i32 0
  %64 = extractelement <3 x float> %56, i32 0
  %65 = extractelement <3 x float> %57, i32 0
  %66 = extractelement <3 x float> %58, i32 0
  %67 = extractelement <3 x float> %59, i32 0
  %temp.vect106 = insertelement <8 x float> undef, float %60, i32 0
  %temp.vect107 = insertelement <8 x float> %temp.vect106, float %61, i32 1
  %temp.vect108 = insertelement <8 x float> %temp.vect107, float %62, i32 2
  %temp.vect109 = insertelement <8 x float> %temp.vect108, float %63, i32 3
  %temp.vect110 = insertelement <8 x float> %temp.vect109, float %64, i32 4
  %temp.vect111 = insertelement <8 x float> %temp.vect110, float %65, i32 5
  %temp.vect112 = insertelement <8 x float> %temp.vect111, float %66, i32 6
  %temp.vect113 = insertelement <8 x float> %temp.vect112, float %67, i32 7
  %68 = extractelement <3 x float> %52, i32 1
  %69 = extractelement <3 x float> %53, i32 1
  %70 = extractelement <3 x float> %54, i32 1
  %71 = extractelement <3 x float> %55, i32 1
  %72 = extractelement <3 x float> %56, i32 1
  %73 = extractelement <3 x float> %57, i32 1
  %74 = extractelement <3 x float> %58, i32 1
  %75 = extractelement <3 x float> %59, i32 1
  %temp.vect98 = insertelement <8 x float> undef, float %68, i32 0
  %temp.vect99 = insertelement <8 x float> %temp.vect98, float %69, i32 1
  %temp.vect100 = insertelement <8 x float> %temp.vect99, float %70, i32 2
  %temp.vect101 = insertelement <8 x float> %temp.vect100, float %71, i32 3
  %temp.vect102 = insertelement <8 x float> %temp.vect101, float %72, i32 4
  %temp.vect103 = insertelement <8 x float> %temp.vect102, float %73, i32 5
  %temp.vect104 = insertelement <8 x float> %temp.vect103, float %74, i32 6
  %temp.vect105 = insertelement <8 x float> %temp.vect104, float %75, i32 7
  %76 = extractelement <3 x float> %52, i32 2
  %77 = extractelement <3 x float> %53, i32 2
  %78 = extractelement <3 x float> %54, i32 2
  %79 = extractelement <3 x float> %55, i32 2
  %80 = extractelement <3 x float> %56, i32 2
  %81 = extractelement <3 x float> %57, i32 2
  %82 = extractelement <3 x float> %58, i32 2
  %83 = extractelement <3 x float> %59, i32 2
  %temp.vect90 = insertelement <8 x float> undef, float %76, i32 0
  %temp.vect91 = insertelement <8 x float> %temp.vect90, float %77, i32 1
  %temp.vect92 = insertelement <8 x float> %temp.vect91, float %78, i32 2
  %temp.vect93 = insertelement <8 x float> %temp.vect92, float %79, i32 3
  %temp.vect94 = insertelement <8 x float> %temp.vect93, float %80, i32 4
  %temp.vect95 = insertelement <8 x float> %temp.vect94, float %81, i32 5
  %temp.vect96 = insertelement <8 x float> %temp.vect95, float %82, i32 6
  %temp.vect97 = insertelement <8 x float> %temp.vect96, float %83, i32 7
  %store.val = insertvalue [3 x <8 x float>] undef, <8 x float> %temp.vect87, 0
  %store.val88 = insertvalue [3 x <8 x float>] %store.val, <8 x float> %temp.vect79, 1
  %store.val89 = insertvalue [3 x <8 x float>] %store.val88, <8 x float> %temp.vect71, 2
  %store.val114 = insertvalue [3 x <8 x float>] undef, <8 x float> %temp.vect113, 0
  %store.val115 = insertvalue [3 x <8 x float>] %store.val114, <8 x float> %temp.vect105, 1
  %store.val116 = insertvalue [3 x <8 x float>] %store.val115, <8 x float> %temp.vect97, 2
  %84 = tail call <8 x float> @_f_v.__vertical_dot3f8([3 x <8 x float>] %store.val89, [3 x <8 x float>] %store.val116)
  %85 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %85 to <8 x float>*
  store <8 x float> %84, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__dotf4_test
; CHECK-NOT: @_f_v.__vertical_dot4f8
; CHECK: @__vertical_dot4f8
; CHECK-NOT: @_f_v.__vertical_dot4f8
; CHECK: ret void
define void @__dotf4_test(<4 x float>* nocapture, <4 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %3 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %3, i32 0
  %extract15 = extractelement <8 x i32> %3, i32 1
  %extract16 = extractelement <8 x i32> %3, i32 2
  %extract17 = extractelement <8 x i32> %3, i32 3
  %extract18 = extractelement <8 x i32> %3, i32 4
  %extract19 = extractelement <8 x i32> %3, i32 5
  %extract20 = extractelement <8 x i32> %3, i32 6
  %extract21 = extractelement <8 x i32> %3, i32 7
  %4 = getelementptr <4 x float>* %0, i32 %extract
  %5 = getelementptr <4 x float>* %0, i32 %extract15
  %6 = getelementptr <4 x float>* %0, i32 %extract16
  %7 = getelementptr <4 x float>* %0, i32 %extract17
  %8 = getelementptr <4 x float>* %0, i32 %extract18
  %9 = getelementptr <4 x float>* %0, i32 %extract19
  %10 = getelementptr <4 x float>* %0, i32 %extract20
  %11 = getelementptr <4 x float>* %0, i32 %extract21
  %12 = load <4 x float>* %4
  %13 = load <4 x float>* %5
  %14 = load <4 x float>* %6
  %15 = load <4 x float>* %7
  %16 = load <4 x float>* %8
  %17 = load <4 x float>* %9
  %18 = load <4 x float>* %10
  %19 = load <4 x float>* %11
  %extend_vec = shufflevector <4 x float> %12, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec22 = shufflevector <4 x float> %13, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec23 = shufflevector <4 x float> %14, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec24 = shufflevector <4 x float> %15, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec25 = shufflevector <4 x float> %16, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec26 = shufflevector <4 x float> %17, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec27 = shufflevector <4 x float> %18, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec28 = shufflevector <4 x float> %19, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %Seq_128_0 = shufflevector <8 x float> %extend_vec, <8 x float> %extend_vec25, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_1 = shufflevector <8 x float> %extend_vec22, <8 x float> %extend_vec26, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_2 = shufflevector <8 x float> %extend_vec23, <8 x float> %extend_vec27, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_3 = shufflevector <8 x float> %extend_vec24, <8 x float> %extend_vec28, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_64_0 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_64_1 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_32_0 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_154 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %Seq_64_279 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_64_380 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_32_287 = shufflevector <8 x float> %Seq_64_279, <8 x float> %Seq_64_380, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_3120 = shufflevector <8 x float> %Seq_64_279, <8 x float> %Seq_64_380, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %20 = getelementptr <4 x float>* %1, i32 %extract
  %21 = getelementptr <4 x float>* %1, i32 %extract15
  %22 = getelementptr <4 x float>* %1, i32 %extract16
  %23 = getelementptr <4 x float>* %1, i32 %extract17
  %24 = getelementptr <4 x float>* %1, i32 %extract18
  %25 = getelementptr <4 x float>* %1, i32 %extract19
  %26 = getelementptr <4 x float>* %1, i32 %extract20
  %27 = getelementptr <4 x float>* %1, i32 %extract21
  %28 = load <4 x float>* %20
  %29 = load <4 x float>* %21
  %30 = load <4 x float>* %22
  %31 = load <4 x float>* %23
  %32 = load <4 x float>* %24
  %33 = load <4 x float>* %25
  %34 = load <4 x float>* %26
  %35 = load <4 x float>* %27
  %extend_vec125 = shufflevector <4 x float> %28, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec126 = shufflevector <4 x float> %29, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec127 = shufflevector <4 x float> %30, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec128 = shufflevector <4 x float> %31, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec129 = shufflevector <4 x float> %32, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec130 = shufflevector <4 x float> %33, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec131 = shufflevector <4 x float> %34, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec132 = shufflevector <4 x float> %35, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %Seq_128_0133 = shufflevector <8 x float> %extend_vec125, <8 x float> %extend_vec129, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_1134 = shufflevector <8 x float> %extend_vec126, <8 x float> %extend_vec130, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_2135 = shufflevector <8 x float> %extend_vec127, <8 x float> %extend_vec131, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_3136 = shufflevector <8 x float> %extend_vec128, <8 x float> %extend_vec132, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_64_0141 = shufflevector <8 x float> %Seq_128_0133, <8 x float> %Seq_128_2135, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_64_1142 = shufflevector <8 x float> %Seq_128_1134, <8 x float> %Seq_128_3136, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_32_0149 = shufflevector <8 x float> %Seq_64_0141, <8 x float> %Seq_64_1142, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_1182 = shufflevector <8 x float> %Seq_64_0141, <8 x float> %Seq_64_1142, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %Seq_64_2207 = shufflevector <8 x float> %Seq_128_0133, <8 x float> %Seq_128_2135, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_64_3208 = shufflevector <8 x float> %Seq_128_1134, <8 x float> %Seq_128_3136, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_32_2215 = shufflevector <8 x float> %Seq_64_2207, <8 x float> %Seq_64_3208, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_3248 = shufflevector <8 x float> %Seq_64_2207, <8 x float> %Seq_64_3208, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %store.val = insertvalue [4 x <8 x float>] undef, <8 x float> %Seq_32_0, 0
  %store.val282 = insertvalue [4 x <8 x float>] %store.val, <8 x float> %Seq_32_154, 1
  %store.val283 = insertvalue [4 x <8 x float>] %store.val282, <8 x float> %Seq_32_287, 2
  %store.val284 = insertvalue [4 x <8 x float>] %store.val283, <8 x float> %Seq_32_3120, 3
  %store.val285 = insertvalue [4 x <8 x float>] undef, <8 x float> %Seq_32_0149, 0
  %store.val286 = insertvalue [4 x <8 x float>] %store.val285, <8 x float> %Seq_32_1182, 1
  %store.val287 = insertvalue [4 x <8 x float>] %store.val286, <8 x float> %Seq_32_2215, 2
  %store.val288 = insertvalue [4 x <8 x float>] %store.val287, <8 x float> %Seq_32_3248, 3
  %36 = tail call <8 x float> @_f_v.__vertical_dot4f8([4 x <8 x float>] %store.val284, [4 x <8 x float>] %store.val288)
  %37 = getelementptr float* %2, i32 %extract
  %ptrTypeCast = bitcast float* %37 to <8 x float>*
  store <8 x float> %36, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__fast_lengthf_test
; CHECK-NOT: @_f_v.__vertical_fast_length1f8
; CHECK: @__vertical_fast_length1f8
; CHECK-NOT: @_f_v.__vertical_fast_length1f8
; CHECK: ret void
define void @__fast_lengthf_test(float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %2 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %2 to <8 x float>*
  %load_arg8 = load <8 x float>* %ptrTypeCast, align 4
  %3 = tail call <8 x float> @_f_v.__vertical_fast_length1f8(<8 x float> %load_arg8)
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast9 = bitcast float* %4 to <8 x float>*
  store <8 x float> %3, <8 x float>* %ptrTypeCast9, align 4
  ret void
}



; CHECK: @__fast_lengthf2_test
; CHECK-NOT: @_f_v.__vertical_fast_length2f8
; CHECK: @__vertical_fast_length2f8
; CHECK-NOT: @_f_v.__vertical_fast_length2f8
; CHECK: ret void
define void @__fast_lengthf2_test(<2 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %2 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %2, i32 0
  %extract3 = extractelement <8 x i32> %2, i32 1
  %extract4 = extractelement <8 x i32> %2, i32 2
  %extract5 = extractelement <8 x i32> %2, i32 3
  %extract6 = extractelement <8 x i32> %2, i32 4
  %extract7 = extractelement <8 x i32> %2, i32 5
  %extract8 = extractelement <8 x i32> %2, i32 6
  %extract9 = extractelement <8 x i32> %2, i32 7
  %3 = getelementptr <2 x float>* %0, i32 %extract
  %4 = getelementptr <2 x float>* %0, i32 %extract3
  %5 = getelementptr <2 x float>* %0, i32 %extract4
  %6 = getelementptr <2 x float>* %0, i32 %extract5
  %7 = getelementptr <2 x float>* %0, i32 %extract6
  %8 = getelementptr <2 x float>* %0, i32 %extract7
  %9 = getelementptr <2 x float>* %0, i32 %extract8
  %10 = getelementptr <2 x float>* %0, i32 %extract9
  %11 = load <2 x float>* %3
  %12 = load <2 x float>* %4
  %13 = load <2 x float>* %5
  %14 = load <2 x float>* %6
  %15 = load <2 x float>* %7
  %16 = load <2 x float>* %8
  %17 = load <2 x float>* %9
  %18 = load <2 x float>* %10
  %19 = extractelement <2 x float> %11, i32 0
  %20 = extractelement <2 x float> %12, i32 0
  %21 = extractelement <2 x float> %13, i32 0
  %22 = extractelement <2 x float> %14, i32 0
  %23 = extractelement <2 x float> %15, i32 0
  %24 = extractelement <2 x float> %16, i32 0
  %25 = extractelement <2 x float> %17, i32 0
  %26 = extractelement <2 x float> %18, i32 0
  %temp.vect32 = insertelement <8 x float> undef, float %19, i32 0
  %temp.vect33 = insertelement <8 x float> %temp.vect32, float %20, i32 1
  %temp.vect34 = insertelement <8 x float> %temp.vect33, float %21, i32 2
  %temp.vect35 = insertelement <8 x float> %temp.vect34, float %22, i32 3
  %temp.vect36 = insertelement <8 x float> %temp.vect35, float %23, i32 4
  %temp.vect37 = insertelement <8 x float> %temp.vect36, float %24, i32 5
  %temp.vect38 = insertelement <8 x float> %temp.vect37, float %25, i32 6
  %temp.vect39 = insertelement <8 x float> %temp.vect38, float %26, i32 7
  %27 = extractelement <2 x float> %11, i32 1
  %28 = extractelement <2 x float> %12, i32 1
  %29 = extractelement <2 x float> %13, i32 1
  %30 = extractelement <2 x float> %14, i32 1
  %31 = extractelement <2 x float> %15, i32 1
  %32 = extractelement <2 x float> %16, i32 1
  %33 = extractelement <2 x float> %17, i32 1
  %34 = extractelement <2 x float> %18, i32 1
  %temp.vect = insertelement <8 x float> undef, float %27, i32 0
  %temp.vect25 = insertelement <8 x float> %temp.vect, float %28, i32 1
  %temp.vect26 = insertelement <8 x float> %temp.vect25, float %29, i32 2
  %temp.vect27 = insertelement <8 x float> %temp.vect26, float %30, i32 3
  %temp.vect28 = insertelement <8 x float> %temp.vect27, float %31, i32 4
  %temp.vect29 = insertelement <8 x float> %temp.vect28, float %32, i32 5
  %temp.vect30 = insertelement <8 x float> %temp.vect29, float %33, i32 6
  %temp.vect31 = insertelement <8 x float> %temp.vect30, float %34, i32 7
  %store.val = insertvalue [2 x <8 x float>] undef, <8 x float> %temp.vect39, 0
  %store.val40 = insertvalue [2 x <8 x float>] %store.val, <8 x float> %temp.vect31, 1
  %35 = tail call <8 x float> @_f_v.__vertical_fast_length2f8([2 x <8 x float>] %store.val40)
  %36 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %36 to <8 x float>*
  store <8 x float> %35, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__fast_lengthf3_test
; CHECK-NOT: @_f_v.__vertical_fast_length3f8
; CHECK: @__vertical_fast_length3f8
; CHECK-NOT: @_f_v.__vertical_fast_length3f8
; CHECK: ret void
define void @__fast_lengthf3_test(<3 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %2 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %2, i32 0
  %extract5 = extractelement <8 x i32> %2, i32 1
  %extract6 = extractelement <8 x i32> %2, i32 2
  %extract7 = extractelement <8 x i32> %2, i32 3
  %extract8 = extractelement <8 x i32> %2, i32 4
  %extract9 = extractelement <8 x i32> %2, i32 5
  %extract10 = extractelement <8 x i32> %2, i32 6
  %extract11 = extractelement <8 x i32> %2, i32 7
  %3 = getelementptr <3 x float>* %0, i32 %extract
  %4 = getelementptr <3 x float>* %0, i32 %extract5
  %5 = getelementptr <3 x float>* %0, i32 %extract6
  %6 = getelementptr <3 x float>* %0, i32 %extract7
  %7 = getelementptr <3 x float>* %0, i32 %extract8
  %8 = getelementptr <3 x float>* %0, i32 %extract9
  %9 = getelementptr <3 x float>* %0, i32 %extract10
  %10 = getelementptr <3 x float>* %0, i32 %extract11
  %11 = load <3 x float>* %3
  %12 = load <3 x float>* %4
  %13 = load <3 x float>* %5
  %14 = load <3 x float>* %6
  %15 = load <3 x float>* %7
  %16 = load <3 x float>* %8
  %17 = load <3 x float>* %9
  %18 = load <3 x float>* %10
  %19 = extractelement <3 x float> %11, i32 0
  %20 = extractelement <3 x float> %12, i32 0
  %21 = extractelement <3 x float> %13, i32 0
  %22 = extractelement <3 x float> %14, i32 0
  %23 = extractelement <3 x float> %15, i32 0
  %24 = extractelement <3 x float> %16, i32 0
  %25 = extractelement <3 x float> %17, i32 0
  %26 = extractelement <3 x float> %18, i32 0
  %temp.vect50 = insertelement <8 x float> undef, float %19, i32 0
  %temp.vect51 = insertelement <8 x float> %temp.vect50, float %20, i32 1
  %temp.vect52 = insertelement <8 x float> %temp.vect51, float %21, i32 2
  %temp.vect53 = insertelement <8 x float> %temp.vect52, float %22, i32 3
  %temp.vect54 = insertelement <8 x float> %temp.vect53, float %23, i32 4
  %temp.vect55 = insertelement <8 x float> %temp.vect54, float %24, i32 5
  %temp.vect56 = insertelement <8 x float> %temp.vect55, float %25, i32 6
  %temp.vect57 = insertelement <8 x float> %temp.vect56, float %26, i32 7
  %27 = extractelement <3 x float> %11, i32 1
  %28 = extractelement <3 x float> %12, i32 1
  %29 = extractelement <3 x float> %13, i32 1
  %30 = extractelement <3 x float> %14, i32 1
  %31 = extractelement <3 x float> %15, i32 1
  %32 = extractelement <3 x float> %16, i32 1
  %33 = extractelement <3 x float> %17, i32 1
  %34 = extractelement <3 x float> %18, i32 1
  %temp.vect42 = insertelement <8 x float> undef, float %27, i32 0
  %temp.vect43 = insertelement <8 x float> %temp.vect42, float %28, i32 1
  %temp.vect44 = insertelement <8 x float> %temp.vect43, float %29, i32 2
  %temp.vect45 = insertelement <8 x float> %temp.vect44, float %30, i32 3
  %temp.vect46 = insertelement <8 x float> %temp.vect45, float %31, i32 4
  %temp.vect47 = insertelement <8 x float> %temp.vect46, float %32, i32 5
  %temp.vect48 = insertelement <8 x float> %temp.vect47, float %33, i32 6
  %temp.vect49 = insertelement <8 x float> %temp.vect48, float %34, i32 7
  %35 = extractelement <3 x float> %11, i32 2
  %36 = extractelement <3 x float> %12, i32 2
  %37 = extractelement <3 x float> %13, i32 2
  %38 = extractelement <3 x float> %14, i32 2
  %39 = extractelement <3 x float> %15, i32 2
  %40 = extractelement <3 x float> %16, i32 2
  %41 = extractelement <3 x float> %17, i32 2
  %42 = extractelement <3 x float> %18, i32 2
  %temp.vect = insertelement <8 x float> undef, float %35, i32 0
  %temp.vect35 = insertelement <8 x float> %temp.vect, float %36, i32 1
  %temp.vect36 = insertelement <8 x float> %temp.vect35, float %37, i32 2
  %temp.vect37 = insertelement <8 x float> %temp.vect36, float %38, i32 3
  %temp.vect38 = insertelement <8 x float> %temp.vect37, float %39, i32 4
  %temp.vect39 = insertelement <8 x float> %temp.vect38, float %40, i32 5
  %temp.vect40 = insertelement <8 x float> %temp.vect39, float %41, i32 6
  %temp.vect41 = insertelement <8 x float> %temp.vect40, float %42, i32 7
  %store.val = insertvalue [3 x <8 x float>] undef, <8 x float> %temp.vect57, 0
  %store.val58 = insertvalue [3 x <8 x float>] %store.val, <8 x float> %temp.vect49, 1
  %store.val59 = insertvalue [3 x <8 x float>] %store.val58, <8 x float> %temp.vect41, 2
  %43 = tail call <8 x float> @_f_v.__vertical_fast_length3f8([3 x <8 x float>] %store.val59)
  %44 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %44 to <8 x float>*
  store <8 x float> %43, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__fast_lengthf4_test
; CHECK-NOT: @_f_v.__vertical_fast_length4f8
; CHECK: @__vertical_fast_length4f8
; CHECK-NOT: @_f_v.__vertical_fast_length4f8
; CHECK: ret void
define void @__fast_lengthf4_test(<4 x float>* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %2 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %2, i32 0
  %extract7 = extractelement <8 x i32> %2, i32 1
  %extract8 = extractelement <8 x i32> %2, i32 2
  %extract9 = extractelement <8 x i32> %2, i32 3
  %extract10 = extractelement <8 x i32> %2, i32 4
  %extract11 = extractelement <8 x i32> %2, i32 5
  %extract12 = extractelement <8 x i32> %2, i32 6
  %extract13 = extractelement <8 x i32> %2, i32 7
  %3 = getelementptr <4 x float>* %0, i32 %extract
  %4 = getelementptr <4 x float>* %0, i32 %extract7
  %5 = getelementptr <4 x float>* %0, i32 %extract8
  %6 = getelementptr <4 x float>* %0, i32 %extract9
  %7 = getelementptr <4 x float>* %0, i32 %extract10
  %8 = getelementptr <4 x float>* %0, i32 %extract11
  %9 = getelementptr <4 x float>* %0, i32 %extract12
  %10 = getelementptr <4 x float>* %0, i32 %extract13
  %11 = load <4 x float>* %3
  %12 = load <4 x float>* %4
  %13 = load <4 x float>* %5
  %14 = load <4 x float>* %6
  %15 = load <4 x float>* %7
  %16 = load <4 x float>* %8
  %17 = load <4 x float>* %9
  %18 = load <4 x float>* %10
  %extend_vec = shufflevector <4 x float> %11, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec14 = shufflevector <4 x float> %12, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec15 = shufflevector <4 x float> %13, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec16 = shufflevector <4 x float> %14, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec17 = shufflevector <4 x float> %15, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec18 = shufflevector <4 x float> %16, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec19 = shufflevector <4 x float> %17, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec20 = shufflevector <4 x float> %18, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %Seq_128_0 = shufflevector <8 x float> %extend_vec, <8 x float> %extend_vec17, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_1 = shufflevector <8 x float> %extend_vec14, <8 x float> %extend_vec18, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_2 = shufflevector <8 x float> %extend_vec15, <8 x float> %extend_vec19, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_3 = shufflevector <8 x float> %extend_vec16, <8 x float> %extend_vec20, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_64_0 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_64_1 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_32_0 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_146 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %Seq_64_271 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_64_372 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_32_279 = shufflevector <8 x float> %Seq_64_271, <8 x float> %Seq_64_372, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_3112 = shufflevector <8 x float> %Seq_64_271, <8 x float> %Seq_64_372, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %store.val = insertvalue [4 x <8 x float>] undef, <8 x float> %Seq_32_0, 0
  %store.val130 = insertvalue [4 x <8 x float>] %store.val, <8 x float> %Seq_32_146, 1
  %store.val131 = insertvalue [4 x <8 x float>] %store.val130, <8 x float> %Seq_32_279, 2
  %store.val132 = insertvalue [4 x <8 x float>] %store.val131, <8 x float> %Seq_32_3112, 3
  %19 = tail call <8 x float> @_f_v.__vertical_fast_length4f8([4 x <8 x float>] %store.val132)
  %20 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %20 to <8 x float>*
  store <8 x float> %19, <8 x float>* %ptrTypeCast, align 4
  ret void
}



; CHECK: @__fast_normalizef_test
; CHECK-NOT: @_f_v.__vertical_fast_normalize1f8
; CHECK: @__vertical_fast_normalize1f8
; CHECK-NOT: @_f_v.__vertical_fast_normalize1f8
; CHECK: ret void
define void @__fast_normalizef_test(float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %2 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %2 to <8 x float>*
  %load_arg8 = load <8 x float>* %ptrTypeCast, align 4
  %3 = tail call <8 x float> @_f_v.__vertical_fast_normalize1f8(<8 x float> %load_arg8)
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast9 = bitcast float* %4 to <8 x float>*
  store <8 x float> %3, <8 x float>* %ptrTypeCast9, align 4
  ret void
}



; CHECK: @__fast_normalizef2_test
; CHECK-NOT: @_f_v.__vertical_fast_normalize2f8
; CHECK: @__vertical_fast_normalize2f8
; CHECK-NOT: @_f_v.__vertical_fast_normalize2f8
; CHECK: ret void
define void @__fast_normalizef2_test(<2 x float>* nocapture, <2 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %2 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %2, i32 0
  %extract5 = extractelement <8 x i32> %2, i32 1
  %extract6 = extractelement <8 x i32> %2, i32 2
  %extract7 = extractelement <8 x i32> %2, i32 3
  %extract8 = extractelement <8 x i32> %2, i32 4
  %extract9 = extractelement <8 x i32> %2, i32 5
  %extract10 = extractelement <8 x i32> %2, i32 6
  %extract11 = extractelement <8 x i32> %2, i32 7
  %3 = getelementptr <2 x float>* %0, i32 %extract
  %4 = getelementptr <2 x float>* %0, i32 %extract5
  %5 = getelementptr <2 x float>* %0, i32 %extract6
  %6 = getelementptr <2 x float>* %0, i32 %extract7
  %7 = getelementptr <2 x float>* %0, i32 %extract8
  %8 = getelementptr <2 x float>* %0, i32 %extract9
  %9 = getelementptr <2 x float>* %0, i32 %extract10
  %10 = getelementptr <2 x float>* %0, i32 %extract11
  %11 = load <2 x float>* %3
  %12 = load <2 x float>* %4
  %13 = load <2 x float>* %5
  %14 = load <2 x float>* %6
  %15 = load <2 x float>* %7
  %16 = load <2 x float>* %8
  %17 = load <2 x float>* %9
  %18 = load <2 x float>* %10
  %19 = extractelement <2 x float> %11, i32 0
  %20 = extractelement <2 x float> %12, i32 0
  %21 = extractelement <2 x float> %13, i32 0
  %22 = extractelement <2 x float> %14, i32 0
  %23 = extractelement <2 x float> %15, i32 0
  %24 = extractelement <2 x float> %16, i32 0
  %25 = extractelement <2 x float> %17, i32 0
  %26 = extractelement <2 x float> %18, i32 0
  %temp.vect35 = insertelement <8 x float> undef, float %19, i32 0
  %temp.vect36 = insertelement <8 x float> %temp.vect35, float %20, i32 1
  %temp.vect37 = insertelement <8 x float> %temp.vect36, float %21, i32 2
  %temp.vect38 = insertelement <8 x float> %temp.vect37, float %22, i32 3
  %temp.vect39 = insertelement <8 x float> %temp.vect38, float %23, i32 4
  %temp.vect40 = insertelement <8 x float> %temp.vect39, float %24, i32 5
  %temp.vect41 = insertelement <8 x float> %temp.vect40, float %25, i32 6
  %temp.vect42 = insertelement <8 x float> %temp.vect41, float %26, i32 7
  %27 = extractelement <2 x float> %11, i32 1
  %28 = extractelement <2 x float> %12, i32 1
  %29 = extractelement <2 x float> %13, i32 1
  %30 = extractelement <2 x float> %14, i32 1
  %31 = extractelement <2 x float> %15, i32 1
  %32 = extractelement <2 x float> %16, i32 1
  %33 = extractelement <2 x float> %17, i32 1
  %34 = extractelement <2 x float> %18, i32 1
  %temp.vect27 = insertelement <8 x float> undef, float %27, i32 0
  %temp.vect28 = insertelement <8 x float> %temp.vect27, float %28, i32 1
  %temp.vect29 = insertelement <8 x float> %temp.vect28, float %29, i32 2
  %temp.vect30 = insertelement <8 x float> %temp.vect29, float %30, i32 3
  %temp.vect31 = insertelement <8 x float> %temp.vect30, float %31, i32 4
  %temp.vect32 = insertelement <8 x float> %temp.vect31, float %32, i32 5
  %temp.vect33 = insertelement <8 x float> %temp.vect32, float %33, i32 6
  %temp.vect34 = insertelement <8 x float> %temp.vect33, float %34, i32 7
  %store.val = insertvalue [2 x <8 x float>] undef, <8 x float> %temp.vect42, 0
  %store.val43 = insertvalue [2 x <8 x float>] %store.val, <8 x float> %temp.vect34, 1
  %35 = tail call [2 x <8 x float>] @_f_v.__vertical_fast_normalize2f8([2 x <8 x float>] %store.val43)
  %36 = extractvalue [2 x <8 x float>] %35, 0
  %37 = extractvalue [2 x <8 x float>] %35, 1
  %shuf_transpL = shufflevector <8 x float> %36, <8 x float> %37, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH = shufflevector <8 x float> %36, <8 x float> %37, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %breakdown = shufflevector <8 x float> %shuf_transpL, <8 x float> undef, <2 x i32> <i32 0, i32 1>
  %breakdown44 = shufflevector <8 x float> %shuf_transpH, <8 x float> undef, <2 x i32> <i32 0, i32 1>
  %breakdown45 = shufflevector <8 x float> %shuf_transpL, <8 x float> undef, <2 x i32> <i32 2, i32 3>
  %breakdown46 = shufflevector <8 x float> %shuf_transpH, <8 x float> undef, <2 x i32> <i32 2, i32 3>
  %breakdown47 = shufflevector <8 x float> %shuf_transpL, <8 x float> undef, <2 x i32> <i32 4, i32 5>
  %breakdown48 = shufflevector <8 x float> %shuf_transpH, <8 x float> undef, <2 x i32> <i32 4, i32 5>
  %breakdown49 = shufflevector <8 x float> %shuf_transpL, <8 x float> undef, <2 x i32> <i32 6, i32 7>
  %breakdown50 = shufflevector <8 x float> %shuf_transpH, <8 x float> undef, <2 x i32> <i32 6, i32 7>
  %38 = getelementptr <2 x float>* %1, i32 %extract
  %39 = getelementptr <2 x float>* %1, i32 %extract5
  %40 = getelementptr <2 x float>* %1, i32 %extract6
  %41 = getelementptr <2 x float>* %1, i32 %extract7
  %42 = getelementptr <2 x float>* %1, i32 %extract8
  %43 = getelementptr <2 x float>* %1, i32 %extract9
  %44 = getelementptr <2 x float>* %1, i32 %extract10
  %45 = getelementptr <2 x float>* %1, i32 %extract11
  store <2 x float> %breakdown, <2 x float>* %38
  store <2 x float> %breakdown44, <2 x float>* %39
  store <2 x float> %breakdown45, <2 x float>* %40
  store <2 x float> %breakdown46, <2 x float>* %41
  store <2 x float> %breakdown47, <2 x float>* %42
  store <2 x float> %breakdown48, <2 x float>* %43
  store <2 x float> %breakdown49, <2 x float>* %44
  store <2 x float> %breakdown50, <2 x float>* %45
  ret void
}



; CHECK: @__fast_normalizef3_test
; CHECK-NOT: @_f_v.__vertical_fast_normalize3f8
; CHECK: @__vertical_fast_normalize3f8
; CHECK-NOT: @_f_v.__vertical_fast_normalize3f8
; CHECK: ret void
define void @__fast_normalizef3_test(<3 x float>* nocapture, <3 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %2 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %2, i32 0
  %extract9 = extractelement <8 x i32> %2, i32 1
  %extract10 = extractelement <8 x i32> %2, i32 2
  %extract11 = extractelement <8 x i32> %2, i32 3
  %extract12 = extractelement <8 x i32> %2, i32 4
  %extract13 = extractelement <8 x i32> %2, i32 5
  %extract14 = extractelement <8 x i32> %2, i32 6
  %extract15 = extractelement <8 x i32> %2, i32 7
  %3 = getelementptr <3 x float>* %0, i32 %extract
  %4 = getelementptr <3 x float>* %0, i32 %extract9
  %5 = getelementptr <3 x float>* %0, i32 %extract10
  %6 = getelementptr <3 x float>* %0, i32 %extract11
  %7 = getelementptr <3 x float>* %0, i32 %extract12
  %8 = getelementptr <3 x float>* %0, i32 %extract13
  %9 = getelementptr <3 x float>* %0, i32 %extract14
  %10 = getelementptr <3 x float>* %0, i32 %extract15
  %11 = load <3 x float>* %3
  %12 = load <3 x float>* %4
  %13 = load <3 x float>* %5
  %14 = load <3 x float>* %6
  %15 = load <3 x float>* %7
  %16 = load <3 x float>* %8
  %17 = load <3 x float>* %9
  %18 = load <3 x float>* %10
  %19 = extractelement <3 x float> %11, i32 0
  %20 = extractelement <3 x float> %12, i32 0
  %21 = extractelement <3 x float> %13, i32 0
  %22 = extractelement <3 x float> %14, i32 0
  %23 = extractelement <3 x float> %15, i32 0
  %24 = extractelement <3 x float> %16, i32 0
  %25 = extractelement <3 x float> %17, i32 0
  %26 = extractelement <3 x float> %18, i32 0
  %temp.vect55 = insertelement <8 x float> undef, float %19, i32 0
  %temp.vect56 = insertelement <8 x float> %temp.vect55, float %20, i32 1
  %temp.vect57 = insertelement <8 x float> %temp.vect56, float %21, i32 2
  %temp.vect58 = insertelement <8 x float> %temp.vect57, float %22, i32 3
  %temp.vect59 = insertelement <8 x float> %temp.vect58, float %23, i32 4
  %temp.vect60 = insertelement <8 x float> %temp.vect59, float %24, i32 5
  %temp.vect61 = insertelement <8 x float> %temp.vect60, float %25, i32 6
  %temp.vect62 = insertelement <8 x float> %temp.vect61, float %26, i32 7
  %27 = extractelement <3 x float> %11, i32 1
  %28 = extractelement <3 x float> %12, i32 1
  %29 = extractelement <3 x float> %13, i32 1
  %30 = extractelement <3 x float> %14, i32 1
  %31 = extractelement <3 x float> %15, i32 1
  %32 = extractelement <3 x float> %16, i32 1
  %33 = extractelement <3 x float> %17, i32 1
  %34 = extractelement <3 x float> %18, i32 1
  %temp.vect47 = insertelement <8 x float> undef, float %27, i32 0
  %temp.vect48 = insertelement <8 x float> %temp.vect47, float %28, i32 1
  %temp.vect49 = insertelement <8 x float> %temp.vect48, float %29, i32 2
  %temp.vect50 = insertelement <8 x float> %temp.vect49, float %30, i32 3
  %temp.vect51 = insertelement <8 x float> %temp.vect50, float %31, i32 4
  %temp.vect52 = insertelement <8 x float> %temp.vect51, float %32, i32 5
  %temp.vect53 = insertelement <8 x float> %temp.vect52, float %33, i32 6
  %temp.vect54 = insertelement <8 x float> %temp.vect53, float %34, i32 7
  %35 = extractelement <3 x float> %11, i32 2
  %36 = extractelement <3 x float> %12, i32 2
  %37 = extractelement <3 x float> %13, i32 2
  %38 = extractelement <3 x float> %14, i32 2
  %39 = extractelement <3 x float> %15, i32 2
  %40 = extractelement <3 x float> %16, i32 2
  %41 = extractelement <3 x float> %17, i32 2
  %42 = extractelement <3 x float> %18, i32 2
  %temp.vect39 = insertelement <8 x float> undef, float %35, i32 0
  %temp.vect40 = insertelement <8 x float> %temp.vect39, float %36, i32 1
  %temp.vect41 = insertelement <8 x float> %temp.vect40, float %37, i32 2
  %temp.vect42 = insertelement <8 x float> %temp.vect41, float %38, i32 3
  %temp.vect43 = insertelement <8 x float> %temp.vect42, float %39, i32 4
  %temp.vect44 = insertelement <8 x float> %temp.vect43, float %40, i32 5
  %temp.vect45 = insertelement <8 x float> %temp.vect44, float %41, i32 6
  %temp.vect46 = insertelement <8 x float> %temp.vect45, float %42, i32 7
  %store.val = insertvalue [3 x <8 x float>] undef, <8 x float> %temp.vect62, 0
  %store.val63 = insertvalue [3 x <8 x float>] %store.val, <8 x float> %temp.vect54, 1
  %store.val64 = insertvalue [3 x <8 x float>] %store.val63, <8 x float> %temp.vect46, 2
  %43 = tail call [3 x <8 x float>] @_f_v.__vertical_fast_normalize3f8([3 x <8 x float>] %store.val64)
  %44 = extractvalue [3 x <8 x float>] %43, 0
  %45 = extractvalue [3 x <8 x float>] %43, 1
  %46 = extractvalue [3 x <8 x float>] %43, 2
  %shuf_transpL = shufflevector <8 x float> %44, <8 x float> %46, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %shuf_transpH = shufflevector <8 x float> %44, <8 x float> %46, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %shuf_transpH66 = shufflevector <8 x float> %45, <8 x float> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 6, i32 7, i32 undef, i32 undef>
  %shuf_transpL67 = shufflevector <8 x float> %shuf_transpL, <8 x float> %45, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH68 = shufflevector <8 x float> %shuf_transpL, <8 x float> %45, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %shuf_transpL69 = shufflevector <8 x float> %shuf_transpH, <8 x float> %shuf_transpH66, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH70 = shufflevector <8 x float> %shuf_transpH, <8 x float> %shuf_transpH66, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %breakdown = shufflevector <8 x float> %shuf_transpL67, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown71 = shufflevector <8 x float> %shuf_transpH68, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown72 = shufflevector <8 x float> %shuf_transpL69, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown73 = shufflevector <8 x float> %shuf_transpH70, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown74 = shufflevector <8 x float> %shuf_transpL67, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %breakdown75 = shufflevector <8 x float> %shuf_transpH68, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %breakdown76 = shufflevector <8 x float> %shuf_transpL69, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %breakdown77 = shufflevector <8 x float> %shuf_transpH70, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %47 = getelementptr <3 x float>* %1, i32 %extract
  %48 = getelementptr <3 x float>* %1, i32 %extract9
  %49 = getelementptr <3 x float>* %1, i32 %extract10
  %50 = getelementptr <3 x float>* %1, i32 %extract11
  %51 = getelementptr <3 x float>* %1, i32 %extract12
  %52 = getelementptr <3 x float>* %1, i32 %extract13
  %53 = getelementptr <3 x float>* %1, i32 %extract14
  %54 = getelementptr <3 x float>* %1, i32 %extract15
  store <3 x float> %breakdown, <3 x float>* %47
  store <3 x float> %breakdown71, <3 x float>* %48
  store <3 x float> %breakdown72, <3 x float>* %49
  store <3 x float> %breakdown73, <3 x float>* %50
  store <3 x float> %breakdown74, <3 x float>* %51
  store <3 x float> %breakdown75, <3 x float>* %52
  store <3 x float> %breakdown76, <3 x float>* %53
  store <3 x float> %breakdown77, <3 x float>* %54
  ret void
}



; CHECK: @__fast_normalizef4_test
; CHECK-NOT: @_f_v.__vertical_fast_normalize4f8
; CHECK: @__vertical_fast_normalize4f8
; CHECK-NOT: @_f_v.__vertical_fast_normalize4f8
; CHECK: ret void
define void @__fast_normalizef4_test(<4 x float>* nocapture, <4 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %2 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %2, i32 0
  %extract13 = extractelement <8 x i32> %2, i32 1
  %extract14 = extractelement <8 x i32> %2, i32 2
  %extract15 = extractelement <8 x i32> %2, i32 3
  %extract16 = extractelement <8 x i32> %2, i32 4
  %extract17 = extractelement <8 x i32> %2, i32 5
  %extract18 = extractelement <8 x i32> %2, i32 6
  %extract19 = extractelement <8 x i32> %2, i32 7
  %3 = getelementptr <4 x float>* %0, i32 %extract
  %4 = getelementptr <4 x float>* %0, i32 %extract13
  %5 = getelementptr <4 x float>* %0, i32 %extract14
  %6 = getelementptr <4 x float>* %0, i32 %extract15
  %7 = getelementptr <4 x float>* %0, i32 %extract16
  %8 = getelementptr <4 x float>* %0, i32 %extract17
  %9 = getelementptr <4 x float>* %0, i32 %extract18
  %10 = getelementptr <4 x float>* %0, i32 %extract19
  %11 = load <4 x float>* %3
  %12 = load <4 x float>* %4
  %13 = load <4 x float>* %5
  %14 = load <4 x float>* %6
  %15 = load <4 x float>* %7
  %16 = load <4 x float>* %8
  %17 = load <4 x float>* %9
  %18 = load <4 x float>* %10
  %extend_vec = shufflevector <4 x float> %11, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec20 = shufflevector <4 x float> %12, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec21 = shufflevector <4 x float> %13, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec22 = shufflevector <4 x float> %14, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec23 = shufflevector <4 x float> %15, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec24 = shufflevector <4 x float> %16, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec25 = shufflevector <4 x float> %17, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec26 = shufflevector <4 x float> %18, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %Seq_128_0 = shufflevector <8 x float> %extend_vec, <8 x float> %extend_vec23, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_1 = shufflevector <8 x float> %extend_vec20, <8 x float> %extend_vec24, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_2 = shufflevector <8 x float> %extend_vec21, <8 x float> %extend_vec25, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_3 = shufflevector <8 x float> %extend_vec22, <8 x float> %extend_vec26, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_64_0 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_64_1 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_32_0 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_152 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %Seq_64_277 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_64_378 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_32_285 = shufflevector <8 x float> %Seq_64_277, <8 x float> %Seq_64_378, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_3118 = shufflevector <8 x float> %Seq_64_277, <8 x float> %Seq_64_378, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %store.val = insertvalue [4 x <8 x float>] undef, <8 x float> %Seq_32_0, 0
  %store.val136 = insertvalue [4 x <8 x float>] %store.val, <8 x float> %Seq_32_152, 1
  %store.val137 = insertvalue [4 x <8 x float>] %store.val136, <8 x float> %Seq_32_285, 2
  %store.val138 = insertvalue [4 x <8 x float>] %store.val137, <8 x float> %Seq_32_3118, 3
  %19 = tail call [4 x <8 x float>] @_f_v.__vertical_fast_normalize4f8([4 x <8 x float>] %store.val138)
  %20 = extractvalue [4 x <8 x float>] %19, 0
  %21 = extractvalue [4 x <8 x float>] %19, 1
  %22 = extractvalue [4 x <8 x float>] %19, 2
  %23 = extractvalue [4 x <8 x float>] %19, 3
  %shuf_transpL139 = shufflevector <8 x float> %20, <8 x float> %22, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %shuf_transpL140 = shufflevector <8 x float> %21, <8 x float> %23, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %shuf_transpH141 = shufflevector <8 x float> %20, <8 x float> %22, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %shuf_transpH142 = shufflevector <8 x float> %21, <8 x float> %23, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %shuf_transpL143 = shufflevector <8 x float> %shuf_transpL139, <8 x float> %shuf_transpL140, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH144 = shufflevector <8 x float> %shuf_transpL139, <8 x float> %shuf_transpL140, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %shuf_transpL145 = shufflevector <8 x float> %shuf_transpH141, <8 x float> %shuf_transpH142, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH146 = shufflevector <8 x float> %shuf_transpH141, <8 x float> %shuf_transpH142, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %breakdown147 = shufflevector <8 x float> %shuf_transpL143, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %breakdown148 = shufflevector <8 x float> %shuf_transpH144, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %breakdown149 = shufflevector <8 x float> %shuf_transpL145, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %breakdown150 = shufflevector <8 x float> %shuf_transpH146, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %breakdown151 = shufflevector <8 x float> %shuf_transpL143, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %breakdown152 = shufflevector <8 x float> %shuf_transpH144, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %breakdown153 = shufflevector <8 x float> %shuf_transpL145, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %breakdown154 = shufflevector <8 x float> %shuf_transpH146, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %24 = getelementptr <4 x float>* %1, i32 %extract
  %25 = getelementptr <4 x float>* %1, i32 %extract13
  %26 = getelementptr <4 x float>* %1, i32 %extract14
  %27 = getelementptr <4 x float>* %1, i32 %extract15
  %28 = getelementptr <4 x float>* %1, i32 %extract16
  %29 = getelementptr <4 x float>* %1, i32 %extract17
  %30 = getelementptr <4 x float>* %1, i32 %extract18
  %31 = getelementptr <4 x float>* %1, i32 %extract19
  store <4 x float> %breakdown147, <4 x float>* %24
  store <4 x float> %breakdown148, <4 x float>* %25
  store <4 x float> %breakdown149, <4 x float>* %26
  store <4 x float> %breakdown150, <4 x float>* %27
  store <4 x float> %breakdown151, <4 x float>* %28
  store <4 x float> %breakdown152, <4 x float>* %29
  store <4 x float> %breakdown153, <4 x float>* %30
  store <4 x float> %breakdown154, <4 x float>* %31
  ret void
}



; CHECK: @__normalizef_test
; CHECK-NOT: @_f_v.__vertical_normalize1f8
; CHECK: @__vertical_normalize1f8
; CHECK-NOT: @_f_v.__vertical_normalize1f8
; CHECK: ret void
define void @__normalizef_test(float* nocapture, float* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %2 = getelementptr float* %0, i32 %gid
  %ptrTypeCast = bitcast float* %2 to <8 x float>*
  %load_arg8 = load <8 x float>* %ptrTypeCast, align 4
  %3 = tail call <8 x float> @_f_v.__vertical_normalize1f8(<8 x float> %load_arg8)
  %4 = getelementptr float* %1, i32 %gid
  %ptrTypeCast9 = bitcast float* %4 to <8 x float>*
  store <8 x float> %3, <8 x float>* %ptrTypeCast9, align 4
  ret void
}



; CHECK: @__normalizef2_test
; CHECK-NOT: @_f_v.__vertical_normalize2f8
; CHECK: @__vertical_normalize2f8
; CHECK-NOT: @_f_v.__vertical_normalize2f8
; CHECK: ret void
define void @__normalizef2_test(<2 x float>* nocapture, <2 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %2 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %2, i32 0
  %extract5 = extractelement <8 x i32> %2, i32 1
  %extract6 = extractelement <8 x i32> %2, i32 2
  %extract7 = extractelement <8 x i32> %2, i32 3
  %extract8 = extractelement <8 x i32> %2, i32 4
  %extract9 = extractelement <8 x i32> %2, i32 5
  %extract10 = extractelement <8 x i32> %2, i32 6
  %extract11 = extractelement <8 x i32> %2, i32 7
  %3 = getelementptr <2 x float>* %0, i32 %extract
  %4 = getelementptr <2 x float>* %0, i32 %extract5
  %5 = getelementptr <2 x float>* %0, i32 %extract6
  %6 = getelementptr <2 x float>* %0, i32 %extract7
  %7 = getelementptr <2 x float>* %0, i32 %extract8
  %8 = getelementptr <2 x float>* %0, i32 %extract9
  %9 = getelementptr <2 x float>* %0, i32 %extract10
  %10 = getelementptr <2 x float>* %0, i32 %extract11
  %11 = load <2 x float>* %3
  %12 = load <2 x float>* %4
  %13 = load <2 x float>* %5
  %14 = load <2 x float>* %6
  %15 = load <2 x float>* %7
  %16 = load <2 x float>* %8
  %17 = load <2 x float>* %9
  %18 = load <2 x float>* %10
  %19 = extractelement <2 x float> %11, i32 0
  %20 = extractelement <2 x float> %12, i32 0
  %21 = extractelement <2 x float> %13, i32 0
  %22 = extractelement <2 x float> %14, i32 0
  %23 = extractelement <2 x float> %15, i32 0
  %24 = extractelement <2 x float> %16, i32 0
  %25 = extractelement <2 x float> %17, i32 0
  %26 = extractelement <2 x float> %18, i32 0
  %temp.vect35 = insertelement <8 x float> undef, float %19, i32 0
  %temp.vect36 = insertelement <8 x float> %temp.vect35, float %20, i32 1
  %temp.vect37 = insertelement <8 x float> %temp.vect36, float %21, i32 2
  %temp.vect38 = insertelement <8 x float> %temp.vect37, float %22, i32 3
  %temp.vect39 = insertelement <8 x float> %temp.vect38, float %23, i32 4
  %temp.vect40 = insertelement <8 x float> %temp.vect39, float %24, i32 5
  %temp.vect41 = insertelement <8 x float> %temp.vect40, float %25, i32 6
  %temp.vect42 = insertelement <8 x float> %temp.vect41, float %26, i32 7
  %27 = extractelement <2 x float> %11, i32 1
  %28 = extractelement <2 x float> %12, i32 1
  %29 = extractelement <2 x float> %13, i32 1
  %30 = extractelement <2 x float> %14, i32 1
  %31 = extractelement <2 x float> %15, i32 1
  %32 = extractelement <2 x float> %16, i32 1
  %33 = extractelement <2 x float> %17, i32 1
  %34 = extractelement <2 x float> %18, i32 1
  %temp.vect27 = insertelement <8 x float> undef, float %27, i32 0
  %temp.vect28 = insertelement <8 x float> %temp.vect27, float %28, i32 1
  %temp.vect29 = insertelement <8 x float> %temp.vect28, float %29, i32 2
  %temp.vect30 = insertelement <8 x float> %temp.vect29, float %30, i32 3
  %temp.vect31 = insertelement <8 x float> %temp.vect30, float %31, i32 4
  %temp.vect32 = insertelement <8 x float> %temp.vect31, float %32, i32 5
  %temp.vect33 = insertelement <8 x float> %temp.vect32, float %33, i32 6
  %temp.vect34 = insertelement <8 x float> %temp.vect33, float %34, i32 7
  %store.val = insertvalue [2 x <8 x float>] undef, <8 x float> %temp.vect42, 0
  %store.val43 = insertvalue [2 x <8 x float>] %store.val, <8 x float> %temp.vect34, 1
  %35 = tail call [2 x <8 x float>] @_f_v.__vertical_normalize2f8([2 x <8 x float>] %store.val43)
  %36 = extractvalue [2 x <8 x float>] %35, 0
  %37 = extractvalue [2 x <8 x float>] %35, 1
  %shuf_transpL = shufflevector <8 x float> %36, <8 x float> %37, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH = shufflevector <8 x float> %36, <8 x float> %37, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %breakdown = shufflevector <8 x float> %shuf_transpL, <8 x float> undef, <2 x i32> <i32 0, i32 1>
  %breakdown44 = shufflevector <8 x float> %shuf_transpH, <8 x float> undef, <2 x i32> <i32 0, i32 1>
  %breakdown45 = shufflevector <8 x float> %shuf_transpL, <8 x float> undef, <2 x i32> <i32 2, i32 3>
  %breakdown46 = shufflevector <8 x float> %shuf_transpH, <8 x float> undef, <2 x i32> <i32 2, i32 3>
  %breakdown47 = shufflevector <8 x float> %shuf_transpL, <8 x float> undef, <2 x i32> <i32 4, i32 5>
  %breakdown48 = shufflevector <8 x float> %shuf_transpH, <8 x float> undef, <2 x i32> <i32 4, i32 5>
  %breakdown49 = shufflevector <8 x float> %shuf_transpL, <8 x float> undef, <2 x i32> <i32 6, i32 7>
  %breakdown50 = shufflevector <8 x float> %shuf_transpH, <8 x float> undef, <2 x i32> <i32 6, i32 7>
  %38 = getelementptr <2 x float>* %1, i32 %extract
  %39 = getelementptr <2 x float>* %1, i32 %extract5
  %40 = getelementptr <2 x float>* %1, i32 %extract6
  %41 = getelementptr <2 x float>* %1, i32 %extract7
  %42 = getelementptr <2 x float>* %1, i32 %extract8
  %43 = getelementptr <2 x float>* %1, i32 %extract9
  %44 = getelementptr <2 x float>* %1, i32 %extract10
  %45 = getelementptr <2 x float>* %1, i32 %extract11
  store <2 x float> %breakdown, <2 x float>* %38
  store <2 x float> %breakdown44, <2 x float>* %39
  store <2 x float> %breakdown45, <2 x float>* %40
  store <2 x float> %breakdown46, <2 x float>* %41
  store <2 x float> %breakdown47, <2 x float>* %42
  store <2 x float> %breakdown48, <2 x float>* %43
  store <2 x float> %breakdown49, <2 x float>* %44
  store <2 x float> %breakdown50, <2 x float>* %45
  ret void
}



; CHECK: @__normalizef3_test
; CHECK-NOT: @_f_v.__vertical_normalize3f8
; CHECK: @__vertical_normalize3f8
; CHECK-NOT: @_f_v.__vertical_normalize3f8
; CHECK: ret void
define void @__normalizef3_test(<3 x float>* nocapture, <3 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %2 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %2, i32 0
  %extract9 = extractelement <8 x i32> %2, i32 1
  %extract10 = extractelement <8 x i32> %2, i32 2
  %extract11 = extractelement <8 x i32> %2, i32 3
  %extract12 = extractelement <8 x i32> %2, i32 4
  %extract13 = extractelement <8 x i32> %2, i32 5
  %extract14 = extractelement <8 x i32> %2, i32 6
  %extract15 = extractelement <8 x i32> %2, i32 7
  %3 = getelementptr <3 x float>* %0, i32 %extract
  %4 = getelementptr <3 x float>* %0, i32 %extract9
  %5 = getelementptr <3 x float>* %0, i32 %extract10
  %6 = getelementptr <3 x float>* %0, i32 %extract11
  %7 = getelementptr <3 x float>* %0, i32 %extract12
  %8 = getelementptr <3 x float>* %0, i32 %extract13
  %9 = getelementptr <3 x float>* %0, i32 %extract14
  %10 = getelementptr <3 x float>* %0, i32 %extract15
  %11 = load <3 x float>* %3
  %12 = load <3 x float>* %4
  %13 = load <3 x float>* %5
  %14 = load <3 x float>* %6
  %15 = load <3 x float>* %7
  %16 = load <3 x float>* %8
  %17 = load <3 x float>* %9
  %18 = load <3 x float>* %10
  %19 = extractelement <3 x float> %11, i32 0
  %20 = extractelement <3 x float> %12, i32 0
  %21 = extractelement <3 x float> %13, i32 0
  %22 = extractelement <3 x float> %14, i32 0
  %23 = extractelement <3 x float> %15, i32 0
  %24 = extractelement <3 x float> %16, i32 0
  %25 = extractelement <3 x float> %17, i32 0
  %26 = extractelement <3 x float> %18, i32 0
  %temp.vect55 = insertelement <8 x float> undef, float %19, i32 0
  %temp.vect56 = insertelement <8 x float> %temp.vect55, float %20, i32 1
  %temp.vect57 = insertelement <8 x float> %temp.vect56, float %21, i32 2
  %temp.vect58 = insertelement <8 x float> %temp.vect57, float %22, i32 3
  %temp.vect59 = insertelement <8 x float> %temp.vect58, float %23, i32 4
  %temp.vect60 = insertelement <8 x float> %temp.vect59, float %24, i32 5
  %temp.vect61 = insertelement <8 x float> %temp.vect60, float %25, i32 6
  %temp.vect62 = insertelement <8 x float> %temp.vect61, float %26, i32 7
  %27 = extractelement <3 x float> %11, i32 1
  %28 = extractelement <3 x float> %12, i32 1
  %29 = extractelement <3 x float> %13, i32 1
  %30 = extractelement <3 x float> %14, i32 1
  %31 = extractelement <3 x float> %15, i32 1
  %32 = extractelement <3 x float> %16, i32 1
  %33 = extractelement <3 x float> %17, i32 1
  %34 = extractelement <3 x float> %18, i32 1
  %temp.vect47 = insertelement <8 x float> undef, float %27, i32 0
  %temp.vect48 = insertelement <8 x float> %temp.vect47, float %28, i32 1
  %temp.vect49 = insertelement <8 x float> %temp.vect48, float %29, i32 2
  %temp.vect50 = insertelement <8 x float> %temp.vect49, float %30, i32 3
  %temp.vect51 = insertelement <8 x float> %temp.vect50, float %31, i32 4
  %temp.vect52 = insertelement <8 x float> %temp.vect51, float %32, i32 5
  %temp.vect53 = insertelement <8 x float> %temp.vect52, float %33, i32 6
  %temp.vect54 = insertelement <8 x float> %temp.vect53, float %34, i32 7
  %35 = extractelement <3 x float> %11, i32 2
  %36 = extractelement <3 x float> %12, i32 2
  %37 = extractelement <3 x float> %13, i32 2
  %38 = extractelement <3 x float> %14, i32 2
  %39 = extractelement <3 x float> %15, i32 2
  %40 = extractelement <3 x float> %16, i32 2
  %41 = extractelement <3 x float> %17, i32 2
  %42 = extractelement <3 x float> %18, i32 2
  %temp.vect39 = insertelement <8 x float> undef, float %35, i32 0
  %temp.vect40 = insertelement <8 x float> %temp.vect39, float %36, i32 1
  %temp.vect41 = insertelement <8 x float> %temp.vect40, float %37, i32 2
  %temp.vect42 = insertelement <8 x float> %temp.vect41, float %38, i32 3
  %temp.vect43 = insertelement <8 x float> %temp.vect42, float %39, i32 4
  %temp.vect44 = insertelement <8 x float> %temp.vect43, float %40, i32 5
  %temp.vect45 = insertelement <8 x float> %temp.vect44, float %41, i32 6
  %temp.vect46 = insertelement <8 x float> %temp.vect45, float %42, i32 7
  %store.val = insertvalue [3 x <8 x float>] undef, <8 x float> %temp.vect62, 0
  %store.val63 = insertvalue [3 x <8 x float>] %store.val, <8 x float> %temp.vect54, 1
  %store.val64 = insertvalue [3 x <8 x float>] %store.val63, <8 x float> %temp.vect46, 2
  %43 = tail call [3 x <8 x float>] @_f_v.__vertical_normalize3f8([3 x <8 x float>] %store.val64)
  %44 = extractvalue [3 x <8 x float>] %43, 0
  %45 = extractvalue [3 x <8 x float>] %43, 1
  %46 = extractvalue [3 x <8 x float>] %43, 2
  %shuf_transpL = shufflevector <8 x float> %44, <8 x float> %46, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %shuf_transpH = shufflevector <8 x float> %44, <8 x float> %46, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %shuf_transpH66 = shufflevector <8 x float> %45, <8 x float> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 6, i32 7, i32 undef, i32 undef>
  %shuf_transpL67 = shufflevector <8 x float> %shuf_transpL, <8 x float> %45, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH68 = shufflevector <8 x float> %shuf_transpL, <8 x float> %45, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %shuf_transpL69 = shufflevector <8 x float> %shuf_transpH, <8 x float> %shuf_transpH66, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH70 = shufflevector <8 x float> %shuf_transpH, <8 x float> %shuf_transpH66, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %breakdown = shufflevector <8 x float> %shuf_transpL67, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown71 = shufflevector <8 x float> %shuf_transpH68, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown72 = shufflevector <8 x float> %shuf_transpL69, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown73 = shufflevector <8 x float> %shuf_transpH70, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown74 = shufflevector <8 x float> %shuf_transpL67, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %breakdown75 = shufflevector <8 x float> %shuf_transpH68, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %breakdown76 = shufflevector <8 x float> %shuf_transpL69, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %breakdown77 = shufflevector <8 x float> %shuf_transpH70, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %47 = getelementptr <3 x float>* %1, i32 %extract
  %48 = getelementptr <3 x float>* %1, i32 %extract9
  %49 = getelementptr <3 x float>* %1, i32 %extract10
  %50 = getelementptr <3 x float>* %1, i32 %extract11
  %51 = getelementptr <3 x float>* %1, i32 %extract12
  %52 = getelementptr <3 x float>* %1, i32 %extract13
  %53 = getelementptr <3 x float>* %1, i32 %extract14
  %54 = getelementptr <3 x float>* %1, i32 %extract15
  store <3 x float> %breakdown, <3 x float>* %47
  store <3 x float> %breakdown71, <3 x float>* %48
  store <3 x float> %breakdown72, <3 x float>* %49
  store <3 x float> %breakdown73, <3 x float>* %50
  store <3 x float> %breakdown74, <3 x float>* %51
  store <3 x float> %breakdown75, <3 x float>* %52
  store <3 x float> %breakdown76, <3 x float>* %53
  store <3 x float> %breakdown77, <3 x float>* %54
  ret void
}



; CHECK: @__normalizef4_test
; CHECK-NOT: @_f_v.__vertical_normalize4f8
; CHECK: @__vertical_normalize4f8
; CHECK-NOT: @_f_v.__vertical_normalize4f8
; CHECK: ret void
define void @__normalizef4_test(<4 x float>* nocapture, <4 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %2 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %2, i32 0
  %extract13 = extractelement <8 x i32> %2, i32 1
  %extract14 = extractelement <8 x i32> %2, i32 2
  %extract15 = extractelement <8 x i32> %2, i32 3
  %extract16 = extractelement <8 x i32> %2, i32 4
  %extract17 = extractelement <8 x i32> %2, i32 5
  %extract18 = extractelement <8 x i32> %2, i32 6
  %extract19 = extractelement <8 x i32> %2, i32 7
  %3 = getelementptr <4 x float>* %0, i32 %extract
  %4 = getelementptr <4 x float>* %0, i32 %extract13
  %5 = getelementptr <4 x float>* %0, i32 %extract14
  %6 = getelementptr <4 x float>* %0, i32 %extract15
  %7 = getelementptr <4 x float>* %0, i32 %extract16
  %8 = getelementptr <4 x float>* %0, i32 %extract17
  %9 = getelementptr <4 x float>* %0, i32 %extract18
  %10 = getelementptr <4 x float>* %0, i32 %extract19
  %11 = load <4 x float>* %3
  %12 = load <4 x float>* %4
  %13 = load <4 x float>* %5
  %14 = load <4 x float>* %6
  %15 = load <4 x float>* %7
  %16 = load <4 x float>* %8
  %17 = load <4 x float>* %9
  %18 = load <4 x float>* %10
  %extend_vec = shufflevector <4 x float> %11, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec20 = shufflevector <4 x float> %12, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec21 = shufflevector <4 x float> %13, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec22 = shufflevector <4 x float> %14, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec23 = shufflevector <4 x float> %15, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec24 = shufflevector <4 x float> %16, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec25 = shufflevector <4 x float> %17, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %extend_vec26 = shufflevector <4 x float> %18, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef>
  %Seq_128_0 = shufflevector <8 x float> %extend_vec, <8 x float> %extend_vec23, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_1 = shufflevector <8 x float> %extend_vec20, <8 x float> %extend_vec24, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_2 = shufflevector <8 x float> %extend_vec21, <8 x float> %extend_vec25, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_128_3 = shufflevector <8 x float> %extend_vec22, <8 x float> %extend_vec26, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  %Seq_64_0 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_64_1 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %Seq_32_0 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_152 = shufflevector <8 x float> %Seq_64_0, <8 x float> %Seq_64_1, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %Seq_64_277 = shufflevector <8 x float> %Seq_128_0, <8 x float> %Seq_128_2, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_64_378 = shufflevector <8 x float> %Seq_128_1, <8 x float> %Seq_128_3, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %Seq_32_285 = shufflevector <8 x float> %Seq_64_277, <8 x float> %Seq_64_378, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %Seq_32_3118 = shufflevector <8 x float> %Seq_64_277, <8 x float> %Seq_64_378, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %store.val = insertvalue [4 x <8 x float>] undef, <8 x float> %Seq_32_0, 0
  %store.val136 = insertvalue [4 x <8 x float>] %store.val, <8 x float> %Seq_32_152, 1
  %store.val137 = insertvalue [4 x <8 x float>] %store.val136, <8 x float> %Seq_32_285, 2
  %store.val138 = insertvalue [4 x <8 x float>] %store.val137, <8 x float> %Seq_32_3118, 3
  %19 = tail call [4 x <8 x float>] @_f_v.__vertical_normalize4f8([4 x <8 x float>] %store.val138)
  %20 = extractvalue [4 x <8 x float>] %19, 0
  %21 = extractvalue [4 x <8 x float>] %19, 1
  %22 = extractvalue [4 x <8 x float>] %19, 2
  %23 = extractvalue [4 x <8 x float>] %19, 3
  %shuf_transpL139 = shufflevector <8 x float> %20, <8 x float> %22, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %shuf_transpL140 = shufflevector <8 x float> %21, <8 x float> %23, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %shuf_transpH141 = shufflevector <8 x float> %20, <8 x float> %22, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %shuf_transpH142 = shufflevector <8 x float> %21, <8 x float> %23, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %shuf_transpL143 = shufflevector <8 x float> %shuf_transpL139, <8 x float> %shuf_transpL140, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH144 = shufflevector <8 x float> %shuf_transpL139, <8 x float> %shuf_transpL140, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %shuf_transpL145 = shufflevector <8 x float> %shuf_transpH141, <8 x float> %shuf_transpH142, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH146 = shufflevector <8 x float> %shuf_transpH141, <8 x float> %shuf_transpH142, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %breakdown147 = shufflevector <8 x float> %shuf_transpL143, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %breakdown148 = shufflevector <8 x float> %shuf_transpH144, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %breakdown149 = shufflevector <8 x float> %shuf_transpL145, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %breakdown150 = shufflevector <8 x float> %shuf_transpH146, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %breakdown151 = shufflevector <8 x float> %shuf_transpL143, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %breakdown152 = shufflevector <8 x float> %shuf_transpH144, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %breakdown153 = shufflevector <8 x float> %shuf_transpL145, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %breakdown154 = shufflevector <8 x float> %shuf_transpH146, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %24 = getelementptr <4 x float>* %1, i32 %extract
  %25 = getelementptr <4 x float>* %1, i32 %extract13
  %26 = getelementptr <4 x float>* %1, i32 %extract14
  %27 = getelementptr <4 x float>* %1, i32 %extract15
  %28 = getelementptr <4 x float>* %1, i32 %extract16
  %29 = getelementptr <4 x float>* %1, i32 %extract17
  %30 = getelementptr <4 x float>* %1, i32 %extract18
  %31 = getelementptr <4 x float>* %1, i32 %extract19
  store <4 x float> %breakdown147, <4 x float>* %24
  store <4 x float> %breakdown148, <4 x float>* %25
  store <4 x float> %breakdown149, <4 x float>* %26
  store <4 x float> %breakdown150, <4 x float>* %27
  store <4 x float> %breakdown151, <4 x float>* %28
  store <4 x float> %breakdown152, <4 x float>* %29
  store <4 x float> %breakdown153, <4 x float>* %30
  store <4 x float> %breakdown154, <4 x float>* %31
  ret void
}



; CHECK: @__ci_gamma_scalar_SPI_test
; CHECK-NOT: @_f_v.__ci_gamma_SPI_8
; CHECK: @__ci_gamma_SPI_8
; CHECK-NOT: @_f_v.__ci_gamma_SPI_8
; CHECK: ret void
define void @__ci_gamma_scalar_SPI_test(<3 x float>* nocapture, float* nocapture, <3 x float>* nocapture) {
entry:
  %gid = tail call i32 @get_global_id(i32 0)
  %broadcast1 = insertelement <8 x i32> undef, i32 %gid, i32 0
  %broadcast2 = shufflevector <8 x i32> %broadcast1, <8 x i32> undef, <8 x i32> zeroinitializer
  %3 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %extract = extractelement <8 x i32> %3, i32 0
  %extract9 = extractelement <8 x i32> %3, i32 1
  %extract10 = extractelement <8 x i32> %3, i32 2
  %extract11 = extractelement <8 x i32> %3, i32 3
  %extract12 = extractelement <8 x i32> %3, i32 4
  %extract13 = extractelement <8 x i32> %3, i32 5
  %extract14 = extractelement <8 x i32> %3, i32 6
  %extract15 = extractelement <8 x i32> %3, i32 7
  %4 = getelementptr <3 x float>* %0, i32 %extract
  %5 = getelementptr <3 x float>* %0, i32 %extract9
  %6 = getelementptr <3 x float>* %0, i32 %extract10
  %7 = getelementptr <3 x float>* %0, i32 %extract11
  %8 = getelementptr <3 x float>* %0, i32 %extract12
  %9 = getelementptr <3 x float>* %0, i32 %extract13
  %10 = getelementptr <3 x float>* %0, i32 %extract14
  %11 = getelementptr <3 x float>* %0, i32 %extract15
  %12 = load <3 x float>* %4
  %13 = load <3 x float>* %5
  %14 = load <3 x float>* %6
  %15 = load <3 x float>* %7
  %16 = load <3 x float>* %8
  %17 = load <3 x float>* %9
  %18 = load <3 x float>* %10
  %19 = load <3 x float>* %11
  %20 = extractelement <3 x float> %12, i32 0
  %21 = extractelement <3 x float> %13, i32 0
  %22 = extractelement <3 x float> %14, i32 0
  %23 = extractelement <3 x float> %15, i32 0
  %24 = extractelement <3 x float> %16, i32 0
  %25 = extractelement <3 x float> %17, i32 0
  %26 = extractelement <3 x float> %18, i32 0
  %27 = extractelement <3 x float> %19, i32 0
  %temp.vect56 = insertelement <8 x float> undef, float %20, i32 0
  %temp.vect57 = insertelement <8 x float> %temp.vect56, float %21, i32 1
  %temp.vect58 = insertelement <8 x float> %temp.vect57, float %22, i32 2
  %temp.vect59 = insertelement <8 x float> %temp.vect58, float %23, i32 3
  %temp.vect60 = insertelement <8 x float> %temp.vect59, float %24, i32 4
  %temp.vect61 = insertelement <8 x float> %temp.vect60, float %25, i32 5
  %temp.vect62 = insertelement <8 x float> %temp.vect61, float %26, i32 6
  %temp.vect63 = insertelement <8 x float> %temp.vect62, float %27, i32 7
  %28 = extractelement <3 x float> %12, i32 1
  %29 = extractelement <3 x float> %13, i32 1
  %30 = extractelement <3 x float> %14, i32 1
  %31 = extractelement <3 x float> %15, i32 1
  %32 = extractelement <3 x float> %16, i32 1
  %33 = extractelement <3 x float> %17, i32 1
  %34 = extractelement <3 x float> %18, i32 1
  %35 = extractelement <3 x float> %19, i32 1
  %temp.vect48 = insertelement <8 x float> undef, float %28, i32 0
  %temp.vect49 = insertelement <8 x float> %temp.vect48, float %29, i32 1
  %temp.vect50 = insertelement <8 x float> %temp.vect49, float %30, i32 2
  %temp.vect51 = insertelement <8 x float> %temp.vect50, float %31, i32 3
  %temp.vect52 = insertelement <8 x float> %temp.vect51, float %32, i32 4
  %temp.vect53 = insertelement <8 x float> %temp.vect52, float %33, i32 5
  %temp.vect54 = insertelement <8 x float> %temp.vect53, float %34, i32 6
  %temp.vect55 = insertelement <8 x float> %temp.vect54, float %35, i32 7
  %36 = extractelement <3 x float> %12, i32 2
  %37 = extractelement <3 x float> %13, i32 2
  %38 = extractelement <3 x float> %14, i32 2
  %39 = extractelement <3 x float> %15, i32 2
  %40 = extractelement <3 x float> %16, i32 2
  %41 = extractelement <3 x float> %17, i32 2
  %42 = extractelement <3 x float> %18, i32 2
  %43 = extractelement <3 x float> %19, i32 2
  %temp.vect40 = insertelement <8 x float> undef, float %36, i32 0
  %temp.vect41 = insertelement <8 x float> %temp.vect40, float %37, i32 1
  %temp.vect42 = insertelement <8 x float> %temp.vect41, float %38, i32 2
  %temp.vect43 = insertelement <8 x float> %temp.vect42, float %39, i32 3
  %temp.vect44 = insertelement <8 x float> %temp.vect43, float %40, i32 4
  %temp.vect45 = insertelement <8 x float> %temp.vect44, float %41, i32 5
  %temp.vect46 = insertelement <8 x float> %temp.vect45, float %42, i32 6
  %temp.vect47 = insertelement <8 x float> %temp.vect46, float %43, i32 7
  %44 = getelementptr float* %1, i32 %extract
  %ptrTypeCast = bitcast float* %44 to <8 x float>*
  %load_arg239 = load <8 x float>* %ptrTypeCast, align 4
  %store.val = insertvalue [3 x <8 x float>] undef, <8 x float> %temp.vect63, 0
  %store.val64 = insertvalue [3 x <8 x float>] %store.val, <8 x float> %temp.vect55, 1
  %store.val65 = insertvalue [3 x <8 x float>] %store.val64, <8 x float> %temp.vect47, 2
  %45 = tail call [3 x <8 x float>] @_f_v.__ci_gamma_SPI_8([3 x <8 x float>] %store.val65, <8 x float> %load_arg239)
  %46 = extractvalue [3 x <8 x float>] %45, 0
  %47 = extractvalue [3 x <8 x float>] %45, 1
  %48 = extractvalue [3 x <8 x float>] %45, 2
  %shuf_transpL = shufflevector <8 x float> %46, <8 x float> %48, <8 x i32> <i32 0, i32 1, i32 8, i32 9, i32 4, i32 5, i32 12, i32 13>
  %shuf_transpH = shufflevector <8 x float> %46, <8 x float> %48, <8 x i32> <i32 2, i32 3, i32 10, i32 11, i32 6, i32 7, i32 14, i32 15>
  %shuf_transpH67 = shufflevector <8 x float> %47, <8 x float> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 6, i32 7, i32 undef, i32 undef>
  %shuf_transpL68 = shufflevector <8 x float> %shuf_transpL, <8 x float> %47, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH69 = shufflevector <8 x float> %shuf_transpL, <8 x float> %47, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %shuf_transpL70 = shufflevector <8 x float> %shuf_transpH, <8 x float> %shuf_transpH67, <8 x i32> <i32 0, i32 8, i32 2, i32 10, i32 4, i32 12, i32 6, i32 14>
  %shuf_transpH71 = shufflevector <8 x float> %shuf_transpH, <8 x float> %shuf_transpH67, <8 x i32> <i32 1, i32 9, i32 3, i32 11, i32 5, i32 13, i32 7, i32 15>
  %breakdown = shufflevector <8 x float> %shuf_transpL68, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown72 = shufflevector <8 x float> %shuf_transpH69, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown73 = shufflevector <8 x float> %shuf_transpL70, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown74 = shufflevector <8 x float> %shuf_transpH71, <8 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  %breakdown75 = shufflevector <8 x float> %shuf_transpL68, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %breakdown76 = shufflevector <8 x float> %shuf_transpH69, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %breakdown77 = shufflevector <8 x float> %shuf_transpL70, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %breakdown78 = shufflevector <8 x float> %shuf_transpH71, <8 x float> undef, <3 x i32> <i32 4, i32 5, i32 6>
  %49 = getelementptr <3 x float>* %2, i32 %extract
  %50 = getelementptr <3 x float>* %2, i32 %extract9
  %51 = getelementptr <3 x float>* %2, i32 %extract10
  %52 = getelementptr <3 x float>* %2, i32 %extract11
  %53 = getelementptr <3 x float>* %2, i32 %extract12
  %54 = getelementptr <3 x float>* %2, i32 %extract13
  %55 = getelementptr <3 x float>* %2, i32 %extract14
  %56 = getelementptr <3 x float>* %2, i32 %extract15
  store <3 x float> %breakdown, <3 x float>* %49
  store <3 x float> %breakdown72, <3 x float>* %50
  store <3 x float> %breakdown73, <3 x float>* %51
  store <3 x float> %breakdown74, <3 x float>* %52
  store <3 x float> %breakdown75, <3 x float>* %53
  store <3 x float> %breakdown76, <3 x float>* %54
  store <3 x float> %breakdown77, <3 x float>* %55
  store <3 x float> %breakdown78, <3 x float>* %56
  ret void
}

declare [3 x <8 x float>] @_f_v.__vertical_cross3f8([3 x <8 x float>], [3 x <8 x float>]) nounwind

declare [4 x <8 x float>] @_f_v.__vertical_cross4f8([4 x <8 x float>], [4 x <8 x float>]) nounwind

declare <8 x float> @_f_v.__vertical_fast_distance1f8(<8 x float>, <8 x float>) nounwind readnone

declare <8 x float> @_f_v.__vertical_fast_distance2f8([2 x <8 x float>], [2 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_fast_distance3f8([3 x <8 x float>], [3 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_fast_distance4f8([4 x <8 x float>], [4 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_distance1f8(<8 x float>, <8 x float>) nounwind readnone

declare <8 x float> @_f_v.__vertical_length1f8(<8 x float>) nounwind readnone

declare <8 x float> @_f_v.__vertical_distance2f8([2 x <8 x float>], [2 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_length2f8([2 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_distance3f8([3 x <8 x float>], [3 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_length3f8([3 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_distance4f8([4 x <8 x float>], [4 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_length4f8([4 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_dot1f8(<8 x float>, <8 x float>) nounwind readnone

declare <8 x float> @_f_v.__vertical_dot2f8([2 x <8 x float>], [2 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_dot3f8([3 x <8 x float>], [3 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_dot4f8([4 x <8 x float>], [4 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_fast_length1f8(<8 x float>) nounwind readnone

declare <8 x float> @_f_v.__vertical_fast_length2f8([2 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_fast_length3f8([3 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_fast_length4f8([4 x <8 x float>]) nounwind readnone

declare <8 x float> @_f_v.__vertical_fast_normalize1f8(<8 x float>) nounwind

declare [2 x <8 x float>] @_f_v.__vertical_fast_normalize2f8([2 x <8 x float>]) nounwind

declare [3 x <8 x float>] @_f_v.__vertical_fast_normalize3f8([3 x <8 x float>]) nounwind

declare [4 x <8 x float>] @_f_v.__vertical_fast_normalize4f8([4 x <8 x float>]) nounwind

declare <8 x float> @_f_v.__vertical_normalize1f8(<8 x float>) nounwind

declare [2 x <8 x float>] @_f_v.__vertical_normalize2f8([2 x <8 x float>]) nounwind

declare [3 x <8 x float>] @_f_v.__vertical_normalize3f8([3 x <8 x float>]) nounwind

declare [4 x <8 x float>] @_f_v.__vertical_normalize4f8([4 x <8 x float>]) nounwind

declare [3 x <8 x float>] @_f_v.__ci_gamma_SPI_8([3 x <8 x float>], <8 x float>) nounwind
