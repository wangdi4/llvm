; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s

; The input C code is inspired by smooth3d. If it's not known that in/out are
; non-aliasing then multiversioning is still helpful to enable interchange and
; unroll+jam. This test checks that we still do this multiversioning even if we
; know "ivdep loop", which does not preclude loop-independent DDG edges between
; in/out references.
;
; #define N 4096
; #define NTIMES 5
; void smooth3d(double in[N][N][N], double out[N][N][N]) {
;     #pragma ivdep loop
;     for(int i=0; i<N; i++)
;         for(int j=1; j<N-1; j++)
;             for(int l=0; l<NTIMES; l++)
;                 for(int k=1; k<N-1; k++)
;                     out[k][j][i] +=
;                     (in[k-1][j][i] + in[k+1][j][i] + in[k][j][i] +
;                      in[k][j+1][i] + in[k][j-1][i] + in[k-1][j-1][i] +
;                      in[k+1][j+1][i] + in[k-1][j+1][i] + in[k+1][j-1][i])/9.0;
; }

; We want to ensure that multiversion occurred:

; CHECK-LABEL: Function: smooth3d
; CHECK-DAG: [[COND1:%[^ ]+]] = &((%in)[4095][4095][4095]) >=u &((%out)[1][1][0]);
; CHECK-DAG: [[COND2:%[^ ]+]] = &((%out)[4094][4094][4095]) >=u &((%in)[0][0][0]);
; CHECK: [[AND:%[^ ]+]] = [[COND1]] & [[COND2]];
; CHECK: if ([[AND]] == 0)
; CHECK: <ivdep>

define void @smooth3d(ptr nocapture readonly %in, ptr nocapture %out) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %indvars.iv154 = phi i64 [ 0, %entry ], [ %indvars.iv.next155, %for.cond.cleanup3 ]
  br label %for.cond5.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond5.preheader:                              ; preds = %for.cond1.preheader, %for.cond.cleanup7
  %indvars.iv150 = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next151, %for.cond.cleanup7 ]
  %indvars.iv.next151 = add nuw nsw i64 %indvars.iv150, 1
  %0 = add nsw i64 %indvars.iv150, -1
  br label %for.cond9.preheader

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %indvars.iv.next155 = add nuw nsw i64 %indvars.iv154, 1
  %exitcond156.not = icmp eq i64 %indvars.iv.next155, 4096
  br i1 %exitcond156.not, label %for.cond.cleanup, label %for.cond1.preheader, !llvm.loop !2

for.cond9.preheader:                              ; preds = %for.cond5.preheader, %for.cond.cleanup11
  %l.0145 = phi i32 [ 0, %for.cond5.preheader ], [ %inc90, %for.cond.cleanup11 ]
  br label %for.body12

for.cond.cleanup7:                                ; preds = %for.cond.cleanup11
  %exitcond153.not = icmp eq i64 %indvars.iv.next151, 4095
  br i1 %exitcond153.not, label %for.cond.cleanup3, label %for.cond5.preheader, !llvm.loop !5

for.cond.cleanup11:                               ; preds = %for.body12
  %inc90 = add nuw nsw i32 %l.0145, 1
  %exitcond149.not = icmp eq i32 %inc90, 5
  br i1 %exitcond149.not, label %for.cond.cleanup7, label %for.cond9.preheader, !llvm.loop !6

for.body12:                                       ; preds = %for.cond9.preheader, %for.body12
  %indvars.iv = phi i64 [ 1, %for.cond9.preheader ], [ %indvars.iv.next, %for.body12 ]
  %1 = add nsw i64 %indvars.iv, -1
  %arrayidx15 = getelementptr inbounds [4096 x [4096 x double]], ptr %in, i64 %1, i64 %indvars.iv150, i64 %indvars.iv154
  %2 = load double, ptr %arrayidx15, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx21 = getelementptr inbounds [4096 x [4096 x double]], ptr %in, i64 %indvars.iv.next, i64 %indvars.iv150, i64 %indvars.iv154
  %3 = load double, ptr %arrayidx21, align 8
  %add22 = fadd fast double %3, %2
  %arrayidx28 = getelementptr inbounds [4096 x [4096 x double]], ptr %in, i64 %indvars.iv, i64 %indvars.iv150, i64 %indvars.iv154
  %4 = load double, ptr %arrayidx28, align 8
  %add29 = fadd fast double %add22, %4
  %arrayidx36 = getelementptr inbounds [4096 x [4096 x double]], ptr %in, i64 %indvars.iv, i64 %indvars.iv.next151, i64 %indvars.iv154
  %5 = load double, ptr %arrayidx36, align 8
  %add37 = fadd fast double %add29, %5
  %arrayidx44 = getelementptr inbounds [4096 x [4096 x double]], ptr %in, i64 %indvars.iv, i64 %0, i64 %indvars.iv154
  %6 = load double, ptr %arrayidx44, align 8
  %add45 = fadd fast double %add37, %6
  %arrayidx53 = getelementptr inbounds [4096 x [4096 x double]], ptr %in, i64 %1, i64 %0, i64 %indvars.iv154
  %7 = load double, ptr %arrayidx53, align 8
  %add54 = fadd fast double %add45, %7
  %arrayidx62 = getelementptr inbounds [4096 x [4096 x double]], ptr %in, i64 %indvars.iv.next, i64 %indvars.iv.next151, i64 %indvars.iv154
  %8 = load double, ptr %arrayidx62, align 8
  %add63 = fadd fast double %add54, %8
  %arrayidx71 = getelementptr inbounds [4096 x [4096 x double]], ptr %in, i64 %1, i64 %indvars.iv.next151, i64 %indvars.iv154
  %9 = load double, ptr %arrayidx71, align 8
  %add72 = fadd fast double %add63, %9
  %arrayidx80 = getelementptr inbounds [4096 x [4096 x double]], ptr %in, i64 %indvars.iv.next, i64 %0, i64 %indvars.iv154
  %10 = load double, ptr %arrayidx80, align 8
  %add81 = fadd fast double %add72, %10
  %div = fmul fast double %add81, 0x3FBC71C71C71C71C
  %arrayidx87 = getelementptr inbounds [4096 x [4096 x double]], ptr %out, i64 %indvars.iv, i64 %indvars.iv150, i64 %indvars.iv154
  %11 = load double, ptr %arrayidx87, align 8
  %add88 = fadd fast double %div, %11
  store double %add88, ptr %arrayidx87, align 8
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4095
  br i1 %exitcond.not, label %for.cond.cleanup11, label %for.body12, !llvm.loop !7
}

!2 = distinct !{!2, !3, !4}
!3 = !{!"llvm.loop.mustprogress"}
!4 = !{!"llvm.loop.vectorize.ivdep_loop"}
!5 = distinct !{!5, !3}
!6 = distinct !{!6, !3}
!7 = distinct !{!7, !3}
