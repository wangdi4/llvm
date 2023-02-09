; REQUIRES: asserts
; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -vplan-vec -vplan-force-vf=2 -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>),vplan-vec' -vplan-force-vf=2 -S %s 2>&1 | FileCheck %s

; This code tests TYPED clause
; The test is passed if UNIFORM:TYPED clause is parsed correctly

; CHECK: UNIFORM clause (size=1): ({{.*}}, TYPED (TYPE: <2 x i64>, NUM_ELEMENTS: i32 1))

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare void @foo(<2 x i64>, i64)

declare <2 x i64> @bar(<2 x i64>) nounwind readnone

define void @test1(i64 %n, <2 x i64>* %arr) {
entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM:TYPED"(<2 x i64>* %arr, <2 x i64> <i64 0, i64 0>, i32 1) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.latch ]
  %cond = icmp eq i64 %indvars.iv, 42
  %gep = getelementptr inbounds <2 x i64>, <2 x i64>* %arr, i64 42
  %load.uni = load <2 x i64>, <2 x i64>* %gep
  call void @foo(<2 x i64> %load.uni, i64 %indvars.iv)

  %call.uni = call <2 x i64> @bar(<2 x i64> zeroinitializer)
  call void @foo(<2 x i64> %call.uni, i64 %indvars.iv)

; Both CGs re-use the scalar call resutls for lane zero.

  br i1 %cond, label %if.then, label %for.latch

if.then:
  %load.uni.pred = load <2 x i64>, <2 x i64>* %gep
  call void @foo(<2 x i64> %load.uni.pred, i64 %indvars.iv)
  br label %for.latch

for.latch:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %exit

exit:
  ret void
}
