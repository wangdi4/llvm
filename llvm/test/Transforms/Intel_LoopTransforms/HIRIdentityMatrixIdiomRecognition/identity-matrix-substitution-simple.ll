; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-create-function-level-region -hir-pre-vec-complete-unroll -hir-identity-matrix-substitution -enable-identity-matrix-substitution -print-before=hir-identity-matrix-substitution -print-after=hir-identity-matrix-substitution -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>,hir-identity-matrix-substitution,print<hir>" -hir-create-function-level-region -enable-identity-matrix-substitution -aa-pipeline="basic-aa" -disable-output %s 2>&1 | FileCheck %s

; Verify that we can find the identity matrix definition in first loop, and
; substitute the uses in the next loop with constant values.
; Source of bwaves example, without unrolling:
;
;  // DEFLOOP
;  for (k = 0; k < nz; k++) {
;    for (j = 0; j < ny; j++) {
;      for (i = 0; i < nx; i++) {
;        rhs[k][j][i] = i + k + j;
;        #pragma nounroll
;        for (l = 0; l < 5; l++) {
;          #pragma nounroll
;          for (m = 0; m < 5; m++) {
;            axp[l][m] = mu;
;            ayp[l][m] = gama;
;            ident[l][m] = 0;
;          }
;          ident[l][l] = 1.0;
;        }
;      }
;    }
;  }
;  // USELOOP
;  for (k = 0; k < nz; k++) {
;    for (j = 0; j < ny; j++) {
;      for (i = 0; i < nx; i++) {
;        for (l = 0; l < 5; l++) {
;          #pragma novector
;          for (m = 0; m < 5; m++) {
;            axp[l][m] = mu * 0.5 * ident[l][m];
;            ayp[l][m] = axp[l][m] * ident[l][m];
;          }
;        }
;      }
;    }
;  }

; Check before substitution for reference to ident
;
; CHECK: BEGIN REGION { modified }
; CHECK-COUNT-25: %11 = (%ident)[0][{{[0-9]}}][{{[0-9]}}]
;
; Check after substitution for constants
;
; CHECK: BEGIN REGION { modified }
; CHECK-NOT: %11 = (%ident)
; CHECK-COUNT-1: %11 = 1.000000e+00;
; CHECK-COUNT-5: %11 = 0.000000e+00;
; CHECK-COUNT-1: %11 = 1.000000e+00;
; CHECK-COUNT-5: %11 = 0.000000e+00;
; CHECK-COUNT-1: %11 = 1.000000e+00;
; CHECK-COUNT-5: %11 = 0.000000e+00;
; CHECK-COUNT-1: %11 = 1.000000e+00;
; CHECK-COUNT-5: %11 = 0.000000e+00;
; CHECK-COUNT-1: %11 = 1.000000e+00;
; CHECK-NOT: %11 = (%ident)


;Module Before HIR
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@AX = dso_local local_unnamed_addr global double 0.000000e+00, align 8

; Function Attrs: nounwind uwtable writeonly
define dso_local i32 @mat_times_vec(i32 %ny, i32 %nz, i32 %nx, double* nocapture %rhs, double %mu, double %gama) local_unnamed_addr #0 {
entry:
  %ident = alloca [5 x [5 x double]], align 16
  %axp = alloca [5 x [5 x double]], align 16
  %ayp = alloca [5 x [5 x double]], align 16
  %0 = zext i32 %nx to i64
  %1 = bitcast [5 x [5 x double]]* %ident to i8*
  call void @llvm.lifetime.start.p0i8(i64 200, i8* nonnull %1) #2
  %2 = bitcast [5 x [5 x double]]* %axp to i8*
  call void @llvm.lifetime.start.p0i8(i64 200, i8* nonnull %2) #2
  %3 = bitcast [5 x [5 x double]]* %ayp to i8*
  call void @llvm.lifetime.start.p0i8(i64 200, i8* nonnull %3) #2
  %cmp = icmp slt i32 %nz, 1
  %cmp1 = icmp slt i32 %ny, 1
  %or.cond = or i1 %cmp1, %cmp
  %cmp4 = icmp slt i32 %nx, 1
  %or.cond177 = or i1 %or.cond, %cmp4
  br i1 %or.cond177, label %cleanup, label %for.cond8.preheader.lr.ph

for.cond8.preheader.lr.ph:                        ; preds = %entry
  %4 = zext i32 %ny to i64
  %5 = mul nuw i64 %0, %4
  %wide.trip.count218220 = zext i32 %nz to i64
  %wide.trip.count214 = sext i32 %ny to i64
  %wide.trip.count = sext i32 %nx to i64
  br label %for.cond8.preheader

for.cond8.preheader:                              ; preds = %for.inc51, %for.cond8.preheader.lr.ph
  %indvars.iv216 = phi i64 [ 0, %for.cond8.preheader.lr.ph ], [ %indvars.iv.next217, %for.inc51 ]
  %6 = mul nsw i64 %5, %indvars.iv216
  %arrayidx = getelementptr inbounds double, double* %rhs, i64 %6
  br label %for.cond11.preheader

for.cond54.preheader:                             ; preds = %for.inc51
  %cmp55184 = icmp sgt i32 %nz, 0
  br i1 %cmp55184, label %for.cond58.preheader.lr.ph, label %for.end110

for.cond58.preheader.lr.ph:                       ; preds = %for.cond54.preheader
  %cmp59182 = icmp sgt i32 %ny, 0
  %cmp63180 = icmp sgt i32 %nx, 0
  %mul = fmul double %mu, 5.000000e-01
  br label %for.cond58.preheader

for.cond11.preheader:                             ; preds = %for.inc48, %for.cond8.preheader
  %indvars.iv211 = phi i64 [ 0, %for.cond8.preheader ], [ %indvars.iv.next212, %for.inc48 ]
  %7 = add nuw nsw i64 %indvars.iv211, %indvars.iv216
  %8 = mul nuw nsw i64 %indvars.iv211, %0
  %arrayidx16 = getelementptr inbounds double, double* %arrayidx, i64 %8
  %9 = trunc i64 %7 to i32
  br label %for.body13

for.body13:                                       ; preds = %for.inc45, %for.cond11.preheader
  %indvars.iv208 = phi i64 [ 0, %for.cond11.preheader ], [ %indvars.iv.next209, %for.inc45 ]
  %10 = trunc i64 %indvars.iv208 to i32
  %add14 = add i32 %9, %10
  %conv = sitofp i32 %add14 to double
  %ptridx = getelementptr inbounds double, double* %arrayidx16, i64 %indvars.iv208
  store double %conv, double* %ptridx, align 8, !tbaa !2
  br label %for.cond22.preheader

for.cond22.preheader:                             ; preds = %for.end, %for.body13
  %indvars.iv205 = phi i64 [ 0, %for.body13 ], [ %indvars.iv.next206, %for.end ]
  br label %for.body25

for.body25:                                       ; preds = %for.body25, %for.cond22.preheader
  %indvars.iv202 = phi i64 [ 0, %for.cond22.preheader ], [ %indvars.iv.next203, %for.body25 ]
  %arrayidx29 = getelementptr inbounds [5 x [5 x double]], [5 x [5 x double]]* %axp, i64 0, i64 %indvars.iv205, i64 %indvars.iv202, !intel-tbaa !6
  store double %mu, double* %arrayidx29, align 8, !tbaa !6
  %arrayidx33 = getelementptr inbounds [5 x [5 x double]], [5 x [5 x double]]* %ayp, i64 0, i64 %indvars.iv205, i64 %indvars.iv202, !intel-tbaa !6
  store double %gama, double* %arrayidx33, align 8, !tbaa !6
  %arrayidx37 = getelementptr inbounds [5 x [5 x double]], [5 x [5 x double]]* %ident, i64 0, i64 %indvars.iv205, i64 %indvars.iv202, !intel-tbaa !6
  store double 0.000000e+00, double* %arrayidx37, align 8, !tbaa !6
  %indvars.iv.next203 = add nuw nsw i64 %indvars.iv202, 1
  %exitcond204 = icmp eq i64 %indvars.iv.next203, 5
  br i1 %exitcond204, label %for.end, label %for.body25, !llvm.loop !9

for.end:                                          ; preds = %for.body25
  %arrayidx41 = getelementptr inbounds [5 x [5 x double]], [5 x [5 x double]]* %ident, i64 0, i64 %indvars.iv205, i64 %indvars.iv205, !intel-tbaa !6
  store double 1.000000e+00, double* %arrayidx41, align 8, !tbaa !6
  %indvars.iv.next206 = add nuw nsw i64 %indvars.iv205, 1
  %exitcond207 = icmp eq i64 %indvars.iv.next206, 5
  br i1 %exitcond207, label %for.inc45, label %for.cond22.preheader, !llvm.loop !11

for.inc45:                                        ; preds = %for.end
  %indvars.iv.next209 = add nuw nsw i64 %indvars.iv208, 1
  %exitcond210 = icmp eq i64 %indvars.iv.next209, %wide.trip.count
  br i1 %exitcond210, label %for.inc48, label %for.body13

for.inc48:                                        ; preds = %for.inc45
  %indvars.iv.next212 = add nuw nsw i64 %indvars.iv211, 1
  %exitcond215 = icmp eq i64 %indvars.iv.next212, %wide.trip.count214
  br i1 %exitcond215, label %for.inc51, label %for.cond11.preheader

for.inc51:                                        ; preds = %for.inc48
  %indvars.iv.next217 = add nuw nsw i64 %indvars.iv216, 1
  %exitcond219 = icmp eq i64 %indvars.iv.next217, %wide.trip.count218220
  br i1 %exitcond219, label %for.cond54.preheader, label %for.cond8.preheader

for.cond58.preheader:                             ; preds = %for.inc108, %for.cond58.preheader.lr.ph
  %k.1185 = phi i32 [ 0, %for.cond58.preheader.lr.ph ], [ %inc109, %for.inc108 ]
  br i1 %cmp59182, label %for.cond62.preheader.preheader, label %for.inc108

for.cond62.preheader.preheader:                   ; preds = %for.cond58.preheader
  br label %for.cond62.preheader

for.cond62.preheader:                             ; preds = %for.cond62.preheader.preheader, %for.inc105
  %j.1183 = phi i32 [ %inc106, %for.inc105 ], [ 0, %for.cond62.preheader.preheader ]
  br i1 %cmp63180, label %for.cond66.preheader.preheader, label %for.inc105

for.cond66.preheader.preheader:                   ; preds = %for.cond62.preheader
  br label %for.cond66.preheader

for.cond66.preheader:                             ; preds = %for.cond66.preheader.preheader, %for.inc102
  %i.1181 = phi i32 [ %inc103, %for.inc102 ], [ 0, %for.cond66.preheader.preheader ]
  br label %for.cond70.preheader

for.cond70.preheader:                             ; preds = %for.inc99, %for.cond66.preheader
  %indvars.iv196 = phi i64 [ 0, %for.cond66.preheader ], [ %indvars.iv.next197, %for.inc99 ]
  br label %for.body73

for.body73:                                       ; preds = %for.body73, %for.cond70.preheader
  %indvars.iv = phi i64 [ 0, %for.cond70.preheader ], [ %indvars.iv.next, %for.body73 ]
  %arrayidx77 = getelementptr inbounds [5 x [5 x double]], [5 x [5 x double]]* %ident, i64 0, i64 %indvars.iv196, i64 %indvars.iv, !intel-tbaa !6
  %11 = load double, double* %arrayidx77, align 8, !tbaa !6
  %mul78 = fmul double %mul, %11
  %arrayidx82 = getelementptr inbounds [5 x [5 x double]], [5 x [5 x double]]* %axp, i64 0, i64 %indvars.iv196, i64 %indvars.iv, !intel-tbaa !6
  store double %mul78, double* %arrayidx82, align 8, !tbaa !6
  %mul91 = fmul double %11, %mul78
  %arrayidx95 = getelementptr inbounds [5 x [5 x double]], [5 x [5 x double]]* %ayp, i64 0, i64 %indvars.iv196, i64 %indvars.iv, !intel-tbaa !6
  store double %mul91, double* %arrayidx95, align 8, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.inc99, label %for.body73, !llvm.loop !12

for.inc99:                                        ; preds = %for.body73
  %indvars.iv.next197 = add nuw nsw i64 %indvars.iv196, 1
  %exitcond198 = icmp eq i64 %indvars.iv.next197, 5
  br i1 %exitcond198, label %for.inc102, label %for.cond70.preheader

for.inc102:                                       ; preds = %for.inc99
  %inc103 = add nuw nsw i32 %i.1181, 1
  %exitcond199 = icmp eq i32 %inc103, %nx
  br i1 %exitcond199, label %for.inc105.loopexit, label %for.cond66.preheader

for.inc105.loopexit:                              ; preds = %for.inc102
  br label %for.inc105

for.inc105:                                       ; preds = %for.inc105.loopexit, %for.cond62.preheader
  %inc106 = add nuw nsw i32 %j.1183, 1
  %exitcond200 = icmp eq i32 %inc106, %ny
  br i1 %exitcond200, label %for.inc108.loopexit, label %for.cond62.preheader

for.inc108.loopexit:                              ; preds = %for.inc105
  br label %for.inc108

for.inc108:                                       ; preds = %for.inc108.loopexit, %for.cond58.preheader
  %inc109 = add nuw nsw i32 %k.1185, 1
  %exitcond201 = icmp eq i32 %inc109, %nz
  br i1 %exitcond201, label %for.end110.loopexit, label %for.cond58.preheader

for.end110.loopexit:                              ; preds = %for.inc108
  br label %for.end110

for.end110:                                       ; preds = %for.end110.loopexit, %for.cond54.preheader
  %arrayidx112 = getelementptr inbounds [5 x [5 x double]], [5 x [5 x double]]* %axp, i64 0, i64 4, i64 3, !intel-tbaa !6
  %12 = load double, double* %arrayidx112, align 8, !tbaa !6
  %arrayidx114 = getelementptr inbounds [5 x [5 x double]], [5 x [5 x double]]* %ayp, i64 0, i64 1, i64 3, !intel-tbaa !6
  %13 = load double, double* %arrayidx114, align 8, !tbaa !6
  %add115 = fadd double %12, %13
  store double %add115, double* @AX, align 8, !tbaa !2
  br label %cleanup

cleanup:                                          ; preds = %entry, %for.end110
  %retval.0 = phi i32 [ 0, %for.end110 ], [ 1, %entry ]
  call void @llvm.lifetime.end.p0i8(i64 200, i8* nonnull %3) #2
  call void @llvm.lifetime.end.p0i8(i64 200, i8* nonnull %2) #2
  call void @llvm.lifetime.end.p0i8(i64 200, i8* nonnull %1) #2
  ret i32 %retval.0
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-builtins" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA5_A5_d", !8, i64 0}
!8 = !{!"array@_ZTSA5_d", !3, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.unroll.disable"}
!11 = distinct !{!11, !10}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.vectorize.width", i32 1}
