; REQUIRES: asserts
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>),vplan-vec' -vplan-force-vf=2 -S %s 2>&1 | FileCheck %s

; This code tests TYPED clause
; The test is passed if UNIFORM:TYPED clause is parsed correctly

; CHECK: UNIFORM clause (size=1): (TYPED({{.*}}, TYPE: <2 x i64>, NUM_ELEMENTS: i32 1))

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

declare void @foo(<2 x i64>, i64)

; Function Attrs: nounwind readnone
declare <2 x i64> @bar(<2 x i64>) #1

define void @test1(i64 %n, ptr %arr) {
entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.UNIFORM:TYPED"(ptr %arr, <2 x i64> zeroinitializer, i32 1) ]
  br label %for.body

for.body:                                         ; preds = %for.latch, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.latch ]
  %cond = icmp eq i64 %indvars.iv, 42
  %gep = getelementptr inbounds <2 x i64>, ptr %arr, i64 42
  %load.uni = load <2 x i64>, ptr %gep, align 16
  call void @foo(<2 x i64> %load.uni, i64 %indvars.iv)
  %call.uni = call <2 x i64> @bar(<2 x i64> zeroinitializer)
  call void @foo(<2 x i64> %call.uni, i64 %indvars.iv)
  br i1 %cond, label %if.then, label %for.latch

if.then:                                          ; preds = %for.body
  %load.uni.pred = load <2 x i64>, ptr %gep, align 16
  call void @foo(<2 x i64> %load.uni.pred, i64 %indvars.iv)
  br label %for.latch

for.latch:                                        ; preds = %if.then, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.latch
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:                                             ; preds = %for.end, %entry
  ret void
}

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
