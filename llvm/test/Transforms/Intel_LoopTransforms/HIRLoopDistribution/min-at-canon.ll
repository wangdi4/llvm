;RUN: opt -hir-ssa-deconstruction -hir-details -print-after=hir-loop-distribute-memrec -hir-loop-distribute-memrec < %s 2>&1 | FileCheck %s

;+ DO i64 i1 = 0, (%storemerge966)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;| <RVAL-REG> LINEAR i64 (%storemerge966)/u64 {sb:2}
;|    <BLOB> LINEAR i64 %storemerge966 {sb:21}
;|
;|   %min = (-64 * i1 + %storemerge966 <= 63) ? -64 * i1 + %storemerge966 : 63;
;|   <LVAL-REG> NON-LINEAR i64 %min {sb:222}
;|   <RVAL-REG> LINEAR i64 -64 * i1 + %storemerge966 {sb:2}
;|      <BLOB> LINEAR i64 %storemerge966 {sb:21}
;|   <RVAL-REG> LINEAR i64 -64 * i1 + %storemerge966 {sb:2}
;|      <BLOB> LINEAR i64 %storemerge966 {sb:21}
;|   ...
;|   + DO i64 i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 1>
;|   | <RVAL-REG> LINEAR i64 %min{def@1} {sb:222}
;    ...
;|   + END LOOP
;
;|   + DO i64 i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 1>
;|   | <RVAL-REG> LINEAR i64 %min{def@1} {sb:222}
;    ...
;|   + END LOOP
;+ END LOOP

; Check that Loop UB ref symbase is updated to a generic value (2).

; CHECK: + DO i64 i1 = 0, (%storemerge966)/u64
; CHECK: <RVAL-REG> LINEAR i64 (%storemerge966)/u64 {sb:2}

; Check if min is non-linear at level 1 and linear at level 2 with def@1

; CHECK: %min = (-64 * i1 + %storemerge966 <= 63) ? -64 * i1 + %storemerge966 : 63;
; CHECK: <LVAL-REG> NON-LINEAR i64 %min
; CHECK: DO i64 i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>
; CHECK: <RVAL-REG> LINEAR i64 %min{def@1}
; CHECK: DO i64 i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>
; CHECK: <RVAL-REG> LINEAR i64 %min{def@1}

; ModuleID = 'min-at-canon.ll'
source_filename = "mathcC/opt_speed/g3d4c.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@MAIN__.f11 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.f32 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.f33 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.f34 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.f41 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.f42 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.f43 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.f44 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.a11 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.a12 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.a13 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.f12 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.a14 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.a21 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.a22 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.a23 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.a24 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.a31 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.a32 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.a33 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.a34 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.a41 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.f13 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.a42 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.a43 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.a44 = external hidden unnamed_addr constant [100 x float], align 16
@MAIN__.f14 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.f21 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.f22 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.f23 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.f24 = external hidden unnamed_addr global [100 x float], align 16
@MAIN__.f31 = external hidden unnamed_addr global [100 x float], align 16

; Function Attrs: nounwind uwtable
define dso_local void @MAIN___for.body4(i64 %storemerge966) #0 {
newFuncRoot:
  br label %for.body4

for.end.exitStub:                                 ; preds = %for.body4
  ret void

for.body4:                                        ; preds = %newFuncRoot, %for.body4
  %inc939 = phi i64 [ 0, %newFuncRoot ], [ %inc, %for.body4 ]
  %storemerge903938 = phi i64 [ 1, %newFuncRoot ], [ %inc322, %for.body4 ]
  %inc = add nuw nsw i64 %inc939, 1
  %sub = add nsw i64 %storemerge903938, -1
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f11, i64 0, i64 %sub
  store float 2.000000e+00, float* %arrayidx
  %0 = load float, float* %arrayidx, align 4, !tbaa !2
  %arrayidx6 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a11, i64 0, i64 %inc939
  %1 = load float, float* %arrayidx6, align 4, !tbaa !2
  %sub7 = fsub float %0, %1
  %arrayidx9 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a21, i64 0, i64 %inc939
  %2 = load float, float* %arrayidx9, align 4, !tbaa !2
  %mul10 = fmul float %2, 2.000000e+00
  %sub11 = fsub float %sub7, %mul10
  %arrayidx13 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a31, i64 0, i64 %inc939
  %3 = load float, float* %arrayidx13, align 4, !tbaa !2
  %mul14 = fmul float %3, 3.000000e+00
  %sub15 = fsub float %sub11, %mul14
  %arrayidx17 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a41, i64 0, i64 %inc939
  %4 = load float, float* %arrayidx17, align 4, !tbaa !2
  %mul18 = fmul float %4, 4.000000e+00
  %sub19 = fsub float %sub15, %mul18
  store float %sub19, float* %arrayidx, align 4, !tbaa !2
  %arrayidx23 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f21, i64 0, i64 %sub
  %5 = load float, float* %arrayidx23, align 4, !tbaa !2
  %sub27 = fsub float %5, %1
  %sub31 = fsub float %sub27, %mul10
  %sub35 = fsub float %sub31, %mul14
  %sub39 = fsub float %sub35, %mul18
  store float %sub39, float* %arrayidx23, align 4, !tbaa !2
  %arrayidx43 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f31, i64 0, i64 %sub
  %6 = load float, float* %arrayidx43, align 4, !tbaa !2
  %sub47 = fsub float %6, %1
  %sub51 = fsub float %sub47, %mul10
  %sub55 = fsub float %sub51, %mul14
  %sub59 = fsub float %sub55, %mul18
  store float %sub59, float* %arrayidx43, align 4, !tbaa !2
  %arrayidx63 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f41, i64 0, i64 %sub
  %7 = load float, float* %arrayidx63, align 4, !tbaa !2
  %sub67 = fsub float %7, %1
  %sub71 = fsub float %sub67, %mul10
  %sub75 = fsub float %sub71, %mul14
  %sub79 = fsub float %sub75, %mul18
  store float %sub79, float* %arrayidx63, align 4, !tbaa !2
  %arrayidx83 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f12, i64 0, i64 %sub
  %8 = load float, float* %arrayidx83, align 4, !tbaa !2
  %arrayidx85 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a12, i64 0, i64 %inc939
  %9 = load float, float* %arrayidx85, align 4, !tbaa !2
  %sub87 = fsub float %8, %9
  %arrayidx89 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a22, i64 0, i64 %inc939
  %10 = load float, float* %arrayidx89, align 4, !tbaa !2
  %mul90 = fmul float %10, 2.000000e+00
  %sub91 = fsub float %sub87, %mul90
  %arrayidx93 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a32, i64 0, i64 %inc939
  %11 = load float, float* %arrayidx93, align 4, !tbaa !2
  %mul94 = fmul float %11, 3.000000e+00
  %sub95 = fsub float %sub91, %mul94
  %arrayidx97 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a42, i64 0, i64 %inc939
  %12 = load float, float* %arrayidx97, align 4, !tbaa !2
  %mul98 = fmul float %12, 4.000000e+00
  %sub99 = fsub float %sub95, %mul98
  store float %sub99, float* %arrayidx83, align 4, !tbaa !2
  %arrayidx103 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f22, i64 0, i64 %sub
  %13 = load float, float* %arrayidx103, align 4, !tbaa !2
  %sub107 = fsub float %13, %9
  %sub111 = fsub float %sub107, %mul90
  %sub115 = fsub float %sub111, %mul94
  %sub119 = fsub float %sub115, %mul98
  store float %sub119, float* %arrayidx103, align 4, !tbaa !2
  %arrayidx123 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f32, i64 0, i64 %sub
  %14 = load float, float* %arrayidx123, align 4, !tbaa !2
  %sub127 = fsub float %14, %9
  %sub131 = fsub float %sub127, %mul90
  %sub135 = fsub float %sub131, %mul94
  %sub139 = fsub float %sub135, %mul98
  store float %sub139, float* %arrayidx123, align 4, !tbaa !2
  %arrayidx143 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f42, i64 0, i64 %sub
  %15 = load float, float* %arrayidx143, align 4, !tbaa !2
  %sub147 = fsub float %15, %9
  %sub151 = fsub float %sub147, %mul90
  %sub155 = fsub float %sub151, %mul94
  %sub159 = fsub float %sub155, %mul98
  store float %sub159, float* %arrayidx143, align 4, !tbaa !2
  %arrayidx163 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f13, i64 0, i64 %sub
  %16 = load float, float* %arrayidx163, align 4, !tbaa !2
  %arrayidx165 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a13, i64 0, i64 %inc939
  %17 = load float, float* %arrayidx165, align 4, !tbaa !2
  %sub167 = fsub float %16, %17
  %arrayidx169 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a23, i64 0, i64 %inc939
  %18 = load float, float* %arrayidx169, align 4, !tbaa !2
  %mul170 = fmul float %18, 2.000000e+00
  %sub171 = fsub float %sub167, %mul170
  %arrayidx173 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a33, i64 0, i64 %inc939
  %19 = load float, float* %arrayidx173, align 4, !tbaa !2
  %mul174 = fmul float %19, 3.000000e+00
  %sub175 = fsub float %sub171, %mul174
  %arrayidx177 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a43, i64 0, i64 %inc939
  %20 = load float, float* %arrayidx177, align 4, !tbaa !2
  %mul178 = fmul float %20, 4.000000e+00
  %sub179 = fsub float %sub175, %mul178
  store float %sub179, float* %arrayidx163, align 4, !tbaa !2
  %arrayidx183 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f23, i64 0, i64 %sub
  %21 = load float, float* %arrayidx183, align 4, !tbaa !2
  %sub187 = fsub float %21, %17
  %sub191 = fsub float %sub187, %mul170
  %sub195 = fsub float %sub191, %mul174
  %sub199 = fsub float %sub195, %mul178
  store float %sub199, float* %arrayidx183, align 4, !tbaa !2
  %arrayidx203 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f33, i64 0, i64 %sub
  %22 = load float, float* %arrayidx203, align 4, !tbaa !2
  %sub207 = fsub float %22, %17
  %sub211 = fsub float %sub207, %mul170
  %sub215 = fsub float %sub211, %mul174
  %sub219 = fsub float %sub215, %mul178
  store float %sub219, float* %arrayidx203, align 4, !tbaa !2
  %arrayidx223 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f43, i64 0, i64 %sub
  %23 = load float, float* %arrayidx223, align 4, !tbaa !2
  %sub227 = fsub float %23, %17
  %sub231 = fsub float %sub227, %mul170
  %sub235 = fsub float %sub231, %mul174
  %sub239 = fsub float %sub235, %mul178
  store float %sub239, float* %arrayidx223, align 4, !tbaa !2
  %arrayidx243 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f14, i64 0, i64 %sub
  %24 = load float, float* %arrayidx243, align 4, !tbaa !2
  %arrayidx245 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a14, i64 0, i64 %inc939
  %25 = load float, float* %arrayidx245, align 4, !tbaa !2
  %sub247 = fsub float %24, %25
  %arrayidx249 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a24, i64 0, i64 %inc939
  %26 = load float, float* %arrayidx249, align 4, !tbaa !2
  %mul250 = fmul float %26, 2.000000e+00
  %sub251 = fsub float %sub247, %mul250
  %arrayidx253 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a34, i64 0, i64 %inc939
  %27 = load float, float* %arrayidx253, align 4, !tbaa !2
  %mul254 = fmul float %27, 3.000000e+00
  %sub255 = fsub float %sub251, %mul254
  %arrayidx257 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.a44, i64 0, i64 %inc939
  %28 = load float, float* %arrayidx257, align 4, !tbaa !2
  %mul258 = fmul float %28, 4.000000e+00
  %sub259 = fsub float %sub255, %mul258
  store float %sub259, float* %arrayidx243, align 4, !tbaa !2
  %arrayidx263 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f24, i64 0, i64 %sub
  %29 = load float, float* %arrayidx263, align 4, !tbaa !2
  %sub267 = fsub float %29, %25
  %sub271 = fsub float %sub267, %mul250
  %sub275 = fsub float %sub271, %mul254
  %sub279 = fsub float %sub275, %mul258
  store float %sub279, float* %arrayidx263, align 4, !tbaa !2
  %arrayidx283 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f34, i64 0, i64 %sub
  %30 = load float, float* %arrayidx283, align 4, !tbaa !2
  %sub287 = fsub float %30, %25
  %sub291 = fsub float %sub287, %mul250
  %sub295 = fsub float %sub291, %mul254
  %sub299 = fsub float %sub295, %mul258
  store float %sub299, float* %arrayidx283, align 4, !tbaa !2
  %arrayidx303 = getelementptr inbounds [100 x float], [100 x float]* @MAIN__.f44, i64 0, i64 %sub
  %31 = load float, float* %arrayidx303, align 4, !tbaa !2
  %sub307 = fsub float %31, %25
  %sub311 = fsub float %sub307, %mul250
  %sub315 = fsub float %sub311, %mul254
  %sub319 = fsub float %sub315, %mul258
  store float %sub319, float* %arrayidx303, align 4, !tbaa !2
  %inc322 = add nuw nsw i64 %storemerge903938, 1
  %exitcond = icmp ugt i64 %storemerge903938, %storemerge966
  br i1 %exitcond, label %for.end.exitStub, label %for.body4
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c95d67b22c5ea6ea67afdc54154ea9648f91208c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm f5b26d034efdfb010744f56a126a4ce818ead37b)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
