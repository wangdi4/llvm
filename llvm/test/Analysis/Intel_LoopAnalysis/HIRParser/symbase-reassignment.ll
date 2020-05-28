; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser -hir-details | FileCheck %s

; Verify that rval use of %and and %sub are assigned a generic symbase.

; CHECK: |   %and = i1  &  7;
; CHECK-NEXT: |   <LVAL-REG> LINEAR zext.i3.i32(i1) {sb:[[AND_LVAL_SYM:[0-9]+]]}
; CHECK-NEXT: |   <RVAL-REG> LINEAR trunc.i64.i32(i1) {sb:[[GEN_RVAL_SYM:[0-9]+]]}

; CHECK: |   %conv = sitofp.i32.double(i1);
; CHECK-NEXT: |   <LVAL-REG> NON-LINEAR double %conv
; CHECK-NEXT: |   <RVAL-REG> LINEAR zext.i3.i32(i1) {sb:[[GEN_RVAL_SYM]]}

; CHECK: |   %conv2 = sitofp.i32.double(%and + 1);
; CHECK-NEXT: |   <LVAL-REG> NON-LINEAR double %conv2
; CHECK-NEXT: |   <RVAL-REG> NON-LINEAR i32 %and + 1 {sb:[[GEN_RVAL_SYM]]}
; CHECK-NEXT: |      <BLOB> NON-LINEAR i32 %and {sb:[[AND_LVAL_SYM]]}


@b = common global [100 x double] zeroinitializer, align 16
@a3 = common global [100 x double] zeroinitializer, align 16
@a4 = common global [100 x double] zeroinitializer, align 16
@a2 = common global [100 x double] zeroinitializer, align 16
@a1 = common global [100 x double] zeroinitializer, align 16
@c = common global [10 x i32] zeroinitializer, align 16
@.str = private unnamed_addr constant [5 x i8] c"%lf \00", align 1
@.str.1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1


define i32 @main() {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv63 = phi i64 [ 0, %entry ], [ %indvars.iv.next64, %for.body ]
  %0 = trunc i64 %indvars.iv63 to i32
  %and = and i32 %0, 7
  %conv = sitofp i32 %and to double
  %arrayidx = getelementptr inbounds [100 x double], [100 x double]* @b, i64 0, i64 %indvars.iv63
  store double %conv, double* %arrayidx, align 8
  %add = add nuw nsw i32 %and, 1
  %conv2 = sitofp i32 %add to double
  %arrayidx4 = getelementptr inbounds [100 x double], [100 x double]* @a1, i64 0, i64 %indvars.iv63
  store double %conv2, double* %arrayidx4, align 8
  %sub = add nsw i32 %and, -1
  %conv6 = sitofp i32 %sub to double
  %arrayidx8 = getelementptr inbounds [100 x double], [100 x double]* @a2, i64 0, i64 %indvars.iv63
  store double %conv6, double* %arrayidx8, align 8
  %add10 = add nuw nsw i32 %and, 2
  %conv11 = sitofp i32 %add10 to double
  %arrayidx13 = getelementptr inbounds [100 x double], [100 x double]* @a3, i64 0, i64 %indvars.iv63
  store double %conv11, double* %arrayidx13, align 8
  %sub15 = add nsw i32 %and, -2
  %conv16 = sitofp i32 %sub15 to double
  %arrayidx18 = getelementptr inbounds [100 x double], [100 x double]* @a4, i64 0, i64 %indvars.iv63
  store double %conv16, double* %arrayidx18, align 8
  %indvars.iv.next64 = add nuw nsw i64 %indvars.iv63, 1
  %exitcond65 = icmp eq i64 %indvars.iv.next64, 100
  br i1 %exitcond65, label %for.body22, label %for.body

for.body22:
  ret i32 0
}
