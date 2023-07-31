; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we unroll & jam the i1 loop by 8 using pragma count despite i2 being a big loop.

; HIR-
; + DO i1 = 0, 1019, 1   <DO_LOOP>
; |   + DO i2 = 0, 1015, 1   <DO_LOOP>
; |   |   %mul10 = %0  *  (%input_array)[1024 * i1 + i2 + 4096];
; |   |   %add11 = %mul10  +  0.000000e+00;
; |   |   %mul15 = %1  *  (%input_array)[1024 * i1 + i2 + 4097];
; |   |   %add16 = %add11  +  %mul15;
; |   |   %mul20 = %2  *  (%input_array)[1024 * i1 + i2 + 4098];
; |   |   %add21 = %add16  +  %mul20;
; |   |   %mul25 = %3  *  (%input_array)[1024 * i1 + i2 + 4099];
; |   |   %add26 = %add21  +  %mul25;
; |   |   %mul30 = %4  *  (%input_array)[1024 * i1 + i2 + 4100];
; |   |   %add31 = %add26  +  %mul30;
; |   |   %mul35 = %5  *  (%input_array)[1024 * i1 + i2 + 4101];
; |   |   %add36 = %add31  +  %mul35;
; |   |   %mul40 = %6  *  (%input_array)[1024 * i1 + i2 + 4102];
; |   |   %add41 = %add36  +  %mul40;
; |   |   %mul45 = %7  *  (%input_array)[1024 * i1 + i2 + 4103];
; |   |   %add46 = %add41  +  %mul45;
; |   |   %mul50 = %8  *  (%input_array)[1024 * i1 + i2 + 4104];
; |   |   %add51 = %add46  +  %mul50;
; |   |   %mul55 = %9  *  (%input_array)[1024 * i1 + i2 + 4];
; |   |   %add56 = %add51  +  %mul55;
; |   |   %mul60 = %10  *  (%input_array)[1024 * i1 + i2 + 1028];
; |   |   %add61 = %add56  +  %mul60;
; |   |   %mul65 = %11  *  (%input_array)[1024 * i1 + i2 + 2052];
; |   |   %add66 = %add61  +  %mul65;
; |   |   %mul70 = %12  *  (%input_array)[1024 * i1 + i2 + 3076];
; |   |   %add71 = %add66  +  %mul70;
; |   |   %mul75 = %13  *  (%input_array)[1024 * i1 + i2 + 5124];
; |   |   %add76 = %add71  +  %mul75;
; |   |   %mul80 = %14  *  (%input_array)[1024 * i1 + i2 + 6148];
; |   |   %add81 = %add76  +  %mul80;
; |   |   %mul85 = %15  *  (%input_array)[1024 * i1 + i2 + 7172];
; |   |   %add86 = %add81  +  %mul85;
; |   |   %mul90 = %16  *  (%input_array)[1024 * i1 + i2 + 8196];
; |   |   %add91 = %add86  +  %mul90;
; |   |   (%output_array)[1016 * i1 + i2] = %add91;
; |   + END LOOP
; + END LOOP

; CHECK: modified
; CHECK: (%input_array)[8192 * i1 + i2 + 4096]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @stencil_2D(ptr noalias nocapture readonly %input_array, ptr noalias nocapture %output_array, ptr noalias nocapture readonly %Dx, ptr noalias nocapture readonly %Dy) {
entry:
  %0 = load double, ptr %Dx, align 8
  %arrayidx12 = getelementptr inbounds double, ptr %Dx, i64 1
  %1 = load double, ptr %arrayidx12, align 8
  %arrayidx17 = getelementptr inbounds double, ptr %Dx, i64 2
  %2 = load double, ptr %arrayidx17, align 8
  %arrayidx22 = getelementptr inbounds double, ptr %Dx, i64 3
  %3 = load double, ptr %arrayidx22, align 8
  %arrayidx27 = getelementptr inbounds double, ptr %Dx, i64 4
  %4 = load double, ptr %arrayidx27, align 8
  %arrayidx32 = getelementptr inbounds double, ptr %Dx, i64 5
  %5 = load double, ptr %arrayidx32, align 8
  %arrayidx37 = getelementptr inbounds double, ptr %Dx, i64 6
  %6 = load double, ptr %arrayidx37, align 8
  %arrayidx42 = getelementptr inbounds double, ptr %Dx, i64 7
  %7 = load double, ptr %arrayidx42, align 8
  %arrayidx47 = getelementptr inbounds double, ptr %Dx, i64 8
  %8 = load double, ptr %arrayidx47, align 8
  %9 = load double, ptr %Dy, align 8
  %arrayidx57 = getelementptr inbounds double, ptr %Dy, i64 1
  %10 = load double, ptr %arrayidx57, align 8
  %arrayidx62 = getelementptr inbounds double, ptr %Dy, i64 2
  %11 = load double, ptr %arrayidx62, align 8
  %arrayidx67 = getelementptr inbounds double, ptr %Dy, i64 3
  %12 = load double, ptr %arrayidx67, align 8
  %arrayidx72 = getelementptr inbounds double, ptr %Dy, i64 5
  %13 = load double, ptr %arrayidx72, align 8
  %arrayidx77 = getelementptr inbounds double, ptr %Dy, i64 6
  %14 = load double, ptr %arrayidx77, align 8
  %arrayidx82 = getelementptr inbounds double, ptr %Dy, i64 7
  %15 = load double, ptr %arrayidx82, align 8
  %arrayidx87 = getelementptr inbounds double, ptr %Dy, i64 8
  %16 = load double, ptr %arrayidx87, align 8
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %j.0180 = phi i64 [ 0, %entry ], [ %inc96, %for.cond.cleanup3 ]
  %add5 = shl i64 %j.0180, 10
  %mul92 = mul nuw nsw i64 %j.0180, 1016
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %inc96 = add nuw nsw i64 %j.0180, 1
  %exitcond181 = icmp eq i64 %inc96, 1020
  br i1 %exitcond181, label %for.cond.cleanup, label %for.cond1.preheader, !llvm.loop !0

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %i.0179 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.body4 ]
  %mul6 = add nuw nsw i64 %i.0179, %add5
  %add7 = add nuw nsw i64 %mul6, 4100
  %add8 = add nuw nsw i64 %mul6, 4096
  %arrayidx9 = getelementptr inbounds double, ptr %input_array, i64 %add8
  %17 = load double, ptr %arrayidx9, align 8
  %mul10 = fmul double %0, %17
  %add11 = fadd double %mul10, 0.000000e+00
  %add13 = add nuw nsw i64 %mul6, 4097
  %arrayidx14 = getelementptr inbounds double, ptr %input_array, i64 %add13
  %18 = load double, ptr %arrayidx14, align 8
  %mul15 = fmul double %1, %18
  %add16 = fadd double %add11, %mul15
  %add18 = add nuw nsw i64 %mul6, 4098
  %arrayidx19 = getelementptr inbounds double, ptr %input_array, i64 %add18
  %19 = load double, ptr %arrayidx19, align 8
  %mul20 = fmul double %2, %19
  %add21 = fadd double %add16, %mul20
  %add23 = add nuw nsw i64 %mul6, 4099
  %arrayidx24 = getelementptr inbounds double, ptr %input_array, i64 %add23
  %20 = load double, ptr %arrayidx24, align 8
  %mul25 = fmul double %3, %20
  %add26 = fadd double %add21, %mul25
  %arrayidx29 = getelementptr inbounds double, ptr %input_array, i64 %add7
  %21 = load double, ptr %arrayidx29, align 8
  %mul30 = fmul double %4, %21
  %add31 = fadd double %add26, %mul30
  %add33 = add nuw nsw i64 %mul6, 4101
  %arrayidx34 = getelementptr inbounds double, ptr %input_array, i64 %add33
  %22 = load double, ptr %arrayidx34, align 8
  %mul35 = fmul double %5, %22
  %add36 = fadd double %add31, %mul35
  %add38 = add nuw nsw i64 %mul6, 4102
  %arrayidx39 = getelementptr inbounds double, ptr %input_array, i64 %add38
  %23 = load double, ptr %arrayidx39, align 8
  %mul40 = fmul double %6, %23
  %add41 = fadd double %add36, %mul40
  %add43 = add nuw nsw i64 %mul6, 4103
  %arrayidx44 = getelementptr inbounds double, ptr %input_array, i64 %add43
  %24 = load double, ptr %arrayidx44, align 8
  %mul45 = fmul double %7, %24
  %add46 = fadd double %add41, %mul45
  %add48 = add nuw nsw i64 %mul6, 4104
  %arrayidx49 = getelementptr inbounds double, ptr %input_array, i64 %add48
  %25 = load double, ptr %arrayidx49, align 8
  %mul50 = fmul double %8, %25
  %add51 = fadd double %add46, %mul50
  %add53 = add nuw nsw i64 %mul6, 4
  %arrayidx54 = getelementptr inbounds double, ptr %input_array, i64 %add53
  %26 = load double, ptr %arrayidx54, align 8
  %mul55 = fmul double %9, %26
  %add56 = fadd double %add51, %mul55
  %add58 = add nuw nsw i64 %mul6, 1028
  %arrayidx59 = getelementptr inbounds double, ptr %input_array, i64 %add58
  %27 = load double, ptr %arrayidx59, align 8
  %mul60 = fmul double %10, %27
  %add61 = fadd double %add56, %mul60
  %add63 = add nuw nsw i64 %mul6, 2052
  %arrayidx64 = getelementptr inbounds double, ptr %input_array, i64 %add63
  %28 = load double, ptr %arrayidx64, align 8
  %mul65 = fmul double %11, %28
  %add66 = fadd double %add61, %mul65
  %add68 = add nuw nsw i64 %mul6, 3076
  %arrayidx69 = getelementptr inbounds double, ptr %input_array, i64 %add68
  %29 = load double, ptr %arrayidx69, align 8
  %mul70 = fmul double %12, %29
  %add71 = fadd double %add66, %mul70
  %add73 = add nuw nsw i64 %mul6, 5124
  %arrayidx74 = getelementptr inbounds double, ptr %input_array, i64 %add73
  %30 = load double, ptr %arrayidx74, align 8
  %mul75 = fmul double %13, %30
  %add76 = fadd double %add71, %mul75
  %add78 = add nuw nsw i64 %mul6, 6148
  %arrayidx79 = getelementptr inbounds double, ptr %input_array, i64 %add78
  %31 = load double, ptr %arrayidx79, align 8
  %mul80 = fmul double %14, %31
  %add81 = fadd double %add76, %mul80
  %add83 = add nuw nsw i64 %mul6, 7172
  %arrayidx84 = getelementptr inbounds double, ptr %input_array, i64 %add83
  %32 = load double, ptr %arrayidx84, align 8
  %mul85 = fmul double %15, %32
  %add86 = fadd double %add81, %mul85
  %add88 = add nuw nsw i64 %mul6, 8196
  %arrayidx89 = getelementptr inbounds double, ptr %input_array, i64 %add88
  %33 = load double, ptr %arrayidx89, align 8
  %mul90 = fmul double %16, %33
  %add91 = fadd double %add86, %mul90
  %add93 = add nuw nsw i64 %i.0179, %mul92
  %arrayidx94 = getelementptr inbounds double, ptr %output_array, i64 %add93
  store double %add91, ptr %arrayidx94, align 8
  %inc = add nuw nsw i64 %i.0179, 1
  %exitcond = icmp eq i64 %inc, 1016
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll_and_jam.count", i32 8}
