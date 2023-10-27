;
; RUN: opt -disable-output -passes='vplan-vec,print' -vplan-force-vf=4 %s 2>&1 | FileCheck %s
;
; LIT test to check that we do not crash during vector code generation when
; dealing with an array private whose specified alignment exceeds the
; array type's preferred alignment. The tests below use [2 x float] private
; with specified alignment of 8/16 when the preferred alignment for this
; array type is 4.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

;
; We have a [2 x float] array private with alignment 8 that is being written
; to as an i64 pointer. Check that we can generate unit stride store in the
; generated vector code for this case.
;
; CHECK-LABEL:      @foo
; CHECK:              {{.*}} = alloca [4 x [2 x float]], align 8
; CHECK:            vector.body:
; CHECK:              store <4 x i64> zeroinitializer, {{.*}}
;
define void @foo() {
entry:
  %arr.priv = alloca [2 x float], align 8
  br label %ph

ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %arr.priv, [2 x float] zeroinitializer, i64 1) ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %ph ], [ %add2, %for.body ]
  store i64 0, ptr %arr.priv, align 8
  call void @baz(ptr %arr.priv) #1
  %add2 = add nuw nsw i64 %iv, 1
  %exitcond.not = icmp eq i64 %add2, 1024
  br i1 %exitcond.not, label %exit, label %for.body

exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

;
; We have a [2 x float] array private with alignment 16 that is being written
; to as an i64 pointer. In this case we need to generate scatters as the private
; allocation needs to be serialized.
;
; CHECK-LABEL: @foo2
; CHECK-COUNT-4:      {{.*}} = alloca [2 x float], align 16
; CHECK:   vector.body:
; CHECK:     call void @llvm.masked.scatter.v4i64.v4p0(<4 x i64> zeroinitializer, {{.*}})
;
define void @foo2() {
entry:
  %arr.priv = alloca [2 x float], align 16
  br label %ph

ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %arr.priv, [2 x float] zeroinitializer, i64 1) ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %ph ], [ %add2, %for.body ]
  store i64 0, ptr %arr.priv, align 8
  call void @baz(ptr %arr.priv) #1
  %add2 = add nuw nsw i64 %iv, 1
  %exitcond.not = icmp eq i64 %add2, 1024
  br i1 %exitcond.not, label %exit, label %for.body

exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare void @baz(ptr)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
