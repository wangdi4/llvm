; RUN:  opt < %s  -loop-simplify  -hir-ssa-deconstruction | opt  -dda  -dda-verify=Region  -analyze  | FileCheck %s 
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [100 x [100 x float]] zeroinitializer, align 16
@a = common global [10 x [10 x [10 x [10 x [10 x float]]]]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub1(float* nocapture %p, float* nocapture readonly %q) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %q, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %add = fadd float %0, 1.000000e+00
  %arrayidx2 = getelementptr inbounds float, float* %p, i64 %indvars.iv
  store float %add, float* %arrayidx2, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 35
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

;;    for (i=0; i< 35; i++) {
;;			p[i] = q[i] +1; }

; CHECK: 'Data Dependence Analysis' for function 'sub1'
; CHECK-DAG: {al:4}(%q)[i1] --> {al:4}(%p)[i1] ANTI (*)
; CHECK-DAG: {al:4}(%p)[i1] --> {al:4}(%q)[i1] FLOW (*)				

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define void @sub2(float* nocapture %p, i64 %n) #0 {

 
;;     for (i=0; i<n; i++) {
;;        p[i] = p[i+1] +1;
;;     }

; CHECK: 'Data Dependence Analysis' for function 'sub2'
; CHECK-DAG: {al:4}(%p)[i1 + 1] --> {al:4}(%p)[i1] ANTI (<)

entry:
  %cmp.8 = icmp sgt i64 %n, 0
  br i1 %cmp.8, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %i.09 = phi i64 [ %add, %for.body ], [ 0, %entry ]
  %add = add nuw nsw i64 %i.09, 1
  %arrayidx = getelementptr inbounds float, float* %p, i64 %add
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %add1 = fadd float %0, 1.000000e+00
  %arrayidx2 = getelementptr inbounds float, float* %p, i64 %i.09
  store float %add1, float* %arrayidx2, align 4, !tbaa !1
  %exitcond = icmp eq i64 %add, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

; Function Attrs: nounwind uwtable
define void @sub3(float* nocapture %p, i32 %n) #0 {


;;    for (i=0; i< 45; i++) {
;;        p[i] = p[i+1] +1;
;;       p[i-1] = p[i-1] + p[i];
;;    }

; CHECK: 'Data Dependence Analysis' for function 'sub3'
; CHECK-DAG: {al:4}(%p)[i1 + 1] --> {al:4}(%p)[i1] ANTI (<)
; CHECK-DAG: {al:4}(%p)[i1 + 1] --> {al:4}(%p)[i1 + -1] ANTI (<)


entry:
  %arrayidx5.phi.trans.insert = getelementptr inbounds float, float* %p, i64 -1
  %.pre = load float, float* %arrayidx5.phi.trans.insert, align 4, !tbaa !1
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %0 = phi float [ %.pre, %entry ], [ %add1, %for.body ]
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds float, float* %p, i64 %indvars.iv.next
  %1 = load float, float* %arrayidx, align 4, !tbaa !1
  %add1 = fadd float %1, 1.000000e+00
  %arrayidx3 = getelementptr inbounds float, float* %p, i64 %indvars.iv
  store float %add1, float* %arrayidx3, align 4, !tbaa !1
  %2 = add nsw i64 %indvars.iv, -1
  %arrayidx5 = getelementptr inbounds float, float* %p, i64 %2
  %add8 = fadd float %0, %add1
  store float %add8, float* %arrayidx5, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 45
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

; Function Attrs: nounwind uwtable
define void @sub4(float* nocapture %p, float* nocapture %q, i32 %n) #0 {
		
;;    for (i=1; i <= N; i++) {
;;        for (j=1; j <= N; j++) {
;;            p[N*i + j] = i;
;;            q[i] =  p[N*i + j -1] ;
;;       }
;;    }
  
; CHECK: 'Data Dependence Analysis' for function 'sub4'
; CHECK-DAG: {al:4}(%p)[100 * i1 + i2 + 101] --> {al:4}(i32*)(%p)[100 * i1 + i2 + 100] FLOW (<= <>)

entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.8, %entry
  %i.022 = phi i64 [ 1, %entry ], [ %inc9, %for.inc.8 ]
  %conv = sitofp i64 %i.022 to float
  %mul = mul nuw nsw i64 %i.022, 100
  %arrayidx7 = getelementptr inbounds float, float* %q, i64 %i.022
  %0 = bitcast float* %arrayidx7 to i32*
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.cond.1.preheader
  %j.021 = phi i64 [ 1, %for.cond.1.preheader ], [ %inc, %for.body.3 ]
  %add = add nuw nsw i64 %j.021, %mul
  %arrayidx = getelementptr inbounds float, float* %p, i64 %add
  store float %conv, float* %arrayidx, align 4, !tbaa !1
  %sub = add nsw i64 %add, -1
  %arrayidx6 = getelementptr inbounds float, float* %p, i64 %sub
  %1 = bitcast float* %arrayidx6 to i32*
  %2 = load i32, i32* %1, align 4, !tbaa !1
  store i32 %2, i32* %0, align 4, !tbaa !1
  %inc = add nuw nsw i64 %j.021, 1
  %exitcond = icmp eq i64 %inc, 101
  br i1 %exitcond, label %for.inc.8, label %for.body.3

for.inc.8:                                        ; preds = %for.body.3
  %inc9 = add nuw nsw i64 %i.022, 1
  %exitcond23 = icmp eq i64 %inc9, 101
  br i1 %exitcond23, label %for.end.10, label %for.cond.1.preheader

for.end.10:                                       ; preds = %for.inc.8
  ret void
}

; Function Attrs: nounwind uwtable
define void @sub5(float* nocapture %p, float* nocapture %q, i32 %n) #0 {
		
;;    for (i=1; i <= N; i++) {
;;        for (j=1; j <= N; j++) {
;;            p[100*i + j] = i;
;;            q[i] =  p[100*i - j +11] ; } }

; CHECK: 'Data Dependence Analysis' for function 'sub5'
; CHECK-DAG: {al:4}(%p)[100 * i1 + i2 + 101] --> {al:4}(i32*)(%p)[100 * i1 + -1 * i2 + 110] FLOW (<= <>)


entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.8, %entry
  %i.022 = phi i64 [ 1, %entry ], [ %inc9, %for.inc.8 ]
  %conv = sitofp i64 %i.022 to float
  %mul = mul nuw nsw i64 %i.022, 100
  %sub = add nuw nsw i64 %mul, 11
  %arrayidx7 = getelementptr inbounds float, float* %q, i64 %i.022
  %0 = bitcast float* %arrayidx7 to i32*
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.cond.1.preheader
  %j.021 = phi i64 [ 1, %for.cond.1.preheader ], [ %inc, %for.body.3 ]
  %add = add nuw nsw i64 %j.021, %mul
  %arrayidx = getelementptr inbounds float, float* %p, i64 %add
  store float %conv, float* %arrayidx, align 4, !tbaa !1
  %add5 = sub nuw nsw i64 %sub, %j.021
  %arrayidx6 = getelementptr inbounds float, float* %p, i64 %add5
  %1 = bitcast float* %arrayidx6 to i32*
  %2 = load i32, i32* %1, align 4, !tbaa !1
  store i32 %2, i32* %0, align 4, !tbaa !1
  %inc = add nuw nsw i64 %j.021, 1
  %exitcond = icmp eq i64 %inc, 101
  br i1 %exitcond, label %for.inc.8, label %for.body.3

for.inc.8:                                        ; preds = %for.body.3
  %inc9 = add nuw nsw i64 %i.022, 1
  %exitcond23 = icmp eq i64 %inc9, 101
  br i1 %exitcond23, label %for.end.10, label %for.cond.1.preheader

for.end.10:                                       ; preds = %for.inc.8
  ret void
}

; Function Attrs: nounwind uwtable
define void @sub6(float* nocapture %p, float* nocapture %q, i64 %n) #0 {
		
;;    for (i=0; i <  N; i++) {
;;        for (j=0; j <  N; j++) {
;;            p[2*i - 4*j] = 1;
;;            q[i] =  p[6*i +8 *j] ;
;;        }
;;     }

; CHECK: 'Data Dependence Analysis' for function 'sub6'
; CHECK-DAG: {al:4}(%p)[2 * i1 + -4 * i2] --> {al:4}(i32*)(%p)[6 * i1 + 8 * i2] FLOW (<= *)
; CHECK-DAG: {al:4}(i32*)(%p)[6 * i1 + 8 * i2] --> {al:4}(%p)[2 * i1 + -4 * i2] ANTI (<= *)

entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.9, %entry
  %i.022 = phi i64 [ 0, %entry ], [ %inc10, %for.inc.9 ]
  %mul = shl nsw i64 %i.022, 1
  %mul5 = mul nuw nsw i64 %i.022, 6
  %arrayidx8 = getelementptr inbounds float, float* %q, i64 %i.022
  %0 = bitcast float* %arrayidx8 to i32*
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.cond.1.preheader
  %j.021 = phi i64 [ 0, %for.cond.1.preheader ], [ %inc, %for.body.3 ]
  %mul4 = shl nsw i64 %j.021, 2
  %sub = sub nsw i64 %mul, %mul4
  %arrayidx = getelementptr inbounds float, float* %p, i64 %sub
  store float 1.000000e+00, float* %arrayidx, align 4, !tbaa !1
  %mul6 = shl i64 %j.021, 3
  %add = add nuw nsw i64 %mul6, %mul5
  %arrayidx7 = getelementptr inbounds float, float* %p, i64 %add
  %1 = bitcast float* %arrayidx7 to i32*
  %2 = load i32, i32* %1, align 4, !tbaa !1
  store i32 %2, i32* %0, align 4, !tbaa !1
  %inc = add nuw nsw i64 %j.021, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.inc.9, label %for.body.3

for.inc.9:                                        ; preds = %for.body.3
  %inc10 = add nuw nsw i64 %i.022, 1
  %exitcond23 = icmp eq i64 %inc10, 100
  br i1 %exitcond23, label %for.end.11, label %for.cond.1.preheader

for.end.11:                                       ; preds = %for.inc.9
  ret void
}

; Function Attrs: nounwind uwtable
define void @sub7(i64 %n) #0 {

		
;;    for (i=0; i < n; i++) {
;;        for (j=0; j < n; j++) {
;;            A[2*i][4*j] =  A[8*i][6*j+1] +1;
;;        }
;;    }
  
; CHECK: 'Data Dependence Analysis' for function 'sub7'
; INDEP expected for A, implying no EDGE
; CHECK-NOT:  @A 


entry:
  %cmp.25 = icmp sgt i64 %n, 0
  br i1 %cmp.25, label %for.body.3.lr.ph, label %for.end.13

for.body.3.lr.ph:                                 ; preds = %entry, %for.inc.11
  %i.026 = phi i64 [ %inc12, %for.inc.11 ], [ 0, %entry ]
  %mul4 = shl nsw i64 %i.026, 3
  %mul8 = shl nsw i64 %i.026, 1
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.body.3.lr.ph
  %j.024 = phi i64 [ 0, %for.body.3.lr.ph ], [ %inc, %for.body.3 ]
  %mul = mul nsw i64 %j.024, 6
  %add = or i64 %mul, 1
  %arrayidx5 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @A, i64 0, i64 %mul4, i64 %add
  %0 = load float, float* %arrayidx5, align 4, !tbaa !1
  %add6 = fadd float %0, 1.000000e+00
  %mul7 = shl nsw i64 %j.024, 2
  %arrayidx10 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @A, i64 0, i64 %mul8, i64 %mul7
  store float %add6, float* %arrayidx10, align 16, !tbaa !1
  %inc = add nuw nsw i64 %j.024, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.inc.11, label %for.body.3

for.inc.11:                                       ; preds = %for.body.3
  %inc12 = add nuw nsw i64 %i.026, 1
  %exitcond27 = icmp eq i64 %inc12, %n
  br i1 %exitcond27, label %for.end.13, label %for.body.3.lr.ph

for.end.13:                                       ; preds = %for.inc.11, %entry
  ret void
}

; Function Attrs: nounwind uwtable
define void @sub8(i64 %n) #0 {

		
;;    for (i1=0; i1 < n; i1++) {
;;        for (i2=0; i2 < n; i2++) {
;;            for (i3=0; i3 < n; i3++) {
;;                for (i4=0; i4 < n; i4++) {
;;                    for (i5=0; i5 < n; i5++) {
;;                        a[i1][i2][i3][i4][i5] = a[i1][i2-1][i3+1][i4-2][i5+3]; }}}}} 

; CHECK: 'Data Dependence Analysis' for function 'sub8'
; CHECK-DAG: FLOW (= < > < >)

entry:
  %cmp.71 = icmp sgt i64 %n, 0
  br i1 %cmp.71, label %for.cond.4.preheader.lr.ph, label %for.end.35

for.cond.4.preheader.lr.ph:                       ; preds = %entry, %for.inc.33
  %i1.072 = phi i64 [ %inc34, %for.inc.33 ], [ 0, %entry ]
  br label %for.cond.7.preheader.lr.ph

for.cond.7.preheader.lr.ph:                       ; preds = %for.cond.4.preheader.lr.ph, %for.inc.30
  %i2.069 = phi i64 [ 0, %for.cond.4.preheader.lr.ph ], [ %inc31, %for.inc.30 ]
  %sub14 = add nsw i64 %i2.069, -1
  br label %for.cond.10.preheader.lr.ph

for.cond.10.preheader.lr.ph:                      ; preds = %for.cond.7.preheader.lr.ph, %for.inc.27
  %i3.066 = phi i64 [ 0, %for.cond.7.preheader.lr.ph ], [ %add13, %for.inc.27 ]
  %add13 = add nuw nsw i64 %i3.066, 1
  br label %for.body.12.lr.ph

for.body.12.lr.ph:                                ; preds = %for.cond.10.preheader.lr.ph, %for.inc.24
  %i4.063 = phi i64 [ 0, %for.cond.10.preheader.lr.ph ], [ %inc25, %for.inc.24 ]
  %sub = add nsw i64 %i4.063, -2
  br label %for.body.12

for.body.12:                                      ; preds = %for.body.12, %for.body.12.lr.ph
  %i5.061 = phi i64 [ 0, %for.body.12.lr.ph ], [ %inc, %for.body.12 ]
  %add = add nuw nsw i64 %i5.061, 3
  %arrayidx18 = getelementptr inbounds [10 x [10 x [10 x [10 x [10 x float]]]]], [10 x [10 x [10 x [10 x [10 x float]]]]]* @a, i64 0, i64 %i1.072, i64 %sub14, i64 %add13, i64 %sub, i64 %add
  %0 = bitcast float* %arrayidx18 to i32*
  %1 = load i32, i32* %0, align 4, !tbaa !1
  %arrayidx23 = getelementptr inbounds [10 x [10 x [10 x [10 x [10 x float]]]]], [10 x [10 x [10 x [10 x [10 x float]]]]]* @a, i64 0, i64 %i1.072, i64 %i2.069, i64 %i3.066, i64 %i4.063, i64 %i5.061
  %2 = bitcast float* %arrayidx23 to i32*
  store i32 %1, i32* %2, align 4, !tbaa !1
  %inc = add nuw nsw i64 %i5.061, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.inc.24, label %for.body.12

for.inc.24:                                       ; preds = %for.body.12
  %inc25 = add nuw nsw i64 %i4.063, 1
  %exitcond75 = icmp eq i64 %inc25, %n
  br i1 %exitcond75, label %for.inc.27, label %for.body.12.lr.ph

for.inc.27:                                       ; preds = %for.inc.24
  %exitcond76 = icmp eq i64 %add13, %n
  br i1 %exitcond76, label %for.inc.30, label %for.cond.10.preheader.lr.ph

for.inc.30:                                       ; preds = %for.inc.27
  %inc31 = add nuw nsw i64 %i2.069, 1
  %exitcond77 = icmp eq i64 %inc31, %n
  br i1 %exitcond77, label %for.inc.33, label %for.cond.7.preheader.lr.ph

for.inc.33:                                       ; preds = %for.inc.30
  %inc34 = add nuw nsw i64 %i1.072, 1
  %exitcond78 = icmp eq i64 %inc34, %n
  br i1 %exitcond78, label %for.end.35, label %for.cond.4.preheader.lr.ph

for.end.35:                                       ; preds = %for.inc.33, %entry
  ret void
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 814) (llvm/branches/loopopt 930)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}



