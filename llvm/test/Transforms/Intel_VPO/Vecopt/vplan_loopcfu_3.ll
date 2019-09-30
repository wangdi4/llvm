; Test inner loop control flow uniformity where inner loop is a while loop with loop index.

; REQUIRES: asserts
; RUN: opt -S %s -VPlanDriver -debug 2>&1 | FileCheck %s

; ModuleID = 'case2.c'
source_filename = "case2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: After inner loop control flow transformation
; CHECK: REGION: {{region[0-9]+}} (BP: NULL)
; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):[[OUTERLOOPREGION:loop[0-9]+]]
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: REGION: [[OUTERLOOPREGION]] (BP: NULL)
; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: SUCCESSORS(1):{{BB[0-9]+}}
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: phi
; CHECK-NEXT: getelementptr
; CHECK-NEXT: load
; CHECK-NEXT: i1 [[TOPTEST:%vp.*]] = icmp
; CHECK-NEXT: SUCCESSORS(1):{{region[0-9]+}}
; CHECK-NEXT: PREDECESSORS(2): {{BB[0-9]+}} {{BB[0-9]+}}
; CHECK-EMPTY:

; CHECK: REGION: {{region[0-9]+}} (BP: NULL)
; CHECK-NEXT: {{BB[0-9]+}} (BP: NULL) :
; CHECK-NEXT: i1 [[TOPTESTNOT:%vp.*]] = not i1 [[TOPTEST]]
; CHECK-NEXT: Condition({{BB[0-9]+}}): [DA: Divergent] i1 [[TOPTEST]] = icmp
; CHECK-NEXT: SUCCESSORS(2):{{BB[0-9]+}}(i1 [[TOPTEST]]), [[INNERLOOPREGION:loop[0-9]+]](!i1 [[TOPTEST]])
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: REGION: [[INNERLOOPREGION]] (BP: NULL)
; CHECK-NEXT: [[PREHEADER:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: trunc
; CHECK-NEXT: SUCCESSORS(1):[[HEADER:BB[0-9]+]]
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: [[HEADER]] (BP: NULL) :
; CHECK-NEXT: phi
; CHECK-NEXT: phi
; CHECK-NEXT: i1 [[MASKPHI:%vp.*]] = phi  [ i1 [[TOPTESTNOT]], [[PREHEADER]] ],  [ i1 [[BOTTOMTEST:%vp.*]], [[LATCH:BB[0-9]+]] ]
; CHECK-NEXT: SUCCESSORS(1):{{mask_region[0-9]+}}
; CHECK-NEXT: PREDECESSORS(2): [[LATCH]] [[PREHEADER]]
; CHECK-EMPTY:

; CHECK-NEXT: REGION: [[MASKREGION:mask_region[0-9]+]] (BP: NULL)
; CHECK-NEXT: [[MASKREGIONENTRY:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: Condition([[HEADER]]): [DA: Divergent] i1 [[MASKPHI]]
; CHECK-NEXT: SUCCESSORS(2):[[LOOPBODYHEADER:BB[0-9]+]](i1 [[MASKPHI]]), [[REGIONEXIT:BB[0-9]+]](!i1 [[MASKPHI]])
; CHECK-NEXT: no PREDECESSORS
; CHECK-EMPTY:

; CHECK-NEXT: [[LOOPBODYHEADER]] (BP: NULL) :
; CHECK-NEXT: mul
; CHECK-NEXT: getelementptr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr
; CHECK-NEXT: store
; CHECK-NEXT: add
; CHECK-NEXT: load
; CHECK-NEXT: sext
; CHECK-NEXT: SUCCESSORS(1):[[REGIONEXIT]]
; CHECK-NEXT: PREDECESSORS(1): [[MASKREGIONENTRY]]
; CHECK-EMPTY:

; CHECK-NEXT: [[REGIONEXIT]] (BP: NULL) :
; CHECK-NEXT: i1 [[BOTTOMTEST_1:%vp.*]] = icmp
; CHECK-NEXT: i1 [[BOTTOMTEST]] = and i1 [[BOTTOMTEST_1]] i1 [[MASKPHI]]
; CHECK-NEXT: i1 [[ALLZEROCHECK:%vp.*]] = all-zero-check i1 [[BOTTOMTEST]]
; CHECK-NEXT: i1 [[NOTALLZEROCHECK:%vp.*]] = not i1 [[ALLZEROCHECK]]
; CHECK-NEXT: no SUCCESSORS
; CHECK-NEXT: PREDECESSORS(2): [[LOOPBODYHEADER]] [[MASKREGIONENTRY]]
; CHECK-EMPTY:
; CHECK-NEXT: SUCCESSORS(1):[[LATCH]]
; CHECK-NEXT: END Region([[MASKREGION]])
; CHECK-EMPTY:

; CHECK-NEXT: [[LATCH]] (BP: NULL) :
; CHECK-NEXT: <Empty Block>
; CHECK-NEXT: Condition([[REGIONEXIT]]): [DA: Uniform]  i1 [[NOTALLZEROCHECK]] = not
; CHECK-NEXT: SUCCESSORS(2):[[HEADER]](i1 [[NOTALLZEROCHECK]]), [[EXIT:BB[0-9]+]](!i1 [[NOTALLZEROCHECK]])
; CHECK-NEXT: PREDECESSORS(1): [[MASKREGION]]
; CHECK-EMPTY:

; CHECK-NEXT: [[EXIT]] (BP: NULL) :

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind

@A = common local_unnamed_addr global [100 x [100 x i64]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32** nocapture readonly %a, i32 %m, i32* nocapture readonly %ub, i32 %k) local_unnamed_addr #0 {
entry:
  %cmp20 = icmp sgt i32 %m, 0
  br i1 %cmp20, label %simd.begin, label %func.exit

simd.begin:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %outer.preheader

outer.preheader:
  %m.sext = sext i32 %m to i64
  br label %outer.header

outer.header:
  %outer.iv = phi i64 [ 0, %outer.preheader ], [ %outer.iv.next, %outer.latch ]
  %arrayidx = getelementptr inbounds i32, i32* %ub, i64 %outer.iv
  %0 = load i32, i32* %arrayidx, align 4
  %inner.toptest = icmp slt i32 %0, 0
  br i1 %inner.toptest, label %outer.latch, label %inner.preheader

inner.preheader:
  %1 = trunc i64 %outer.iv to i32
  br label %inner.header

inner.header:
  %inner.iv = phi i64 [ 0, %inner.preheader ], [ %inner.iv.next, %inner.header ]
  %2 = phi i32 [ %0, %inner.preheader ], [ %4, %inner.header ]
  %mul = mul nsw i32 %2, %1
  %arrayidx5 = getelementptr inbounds i32*, i32** %a, i64 %inner.iv
  %3 = load i32*, i32** %arrayidx5, align 8
  %arrayidx7 = getelementptr inbounds i32, i32* %3, i64 %outer.iv
  store i32 %mul, i32* %arrayidx7, align 4
  %inner.iv.next = add nuw nsw i64 %inner.iv, 1
  %4 = load i32, i32* %arrayidx, align 4
  %5 = sext i32 %4 to i64
  %inner.exitcond = icmp slt i64 %inner.iv, %5
  br i1 %inner.exitcond, label %inner.header, label %outer.latch.loopexit

outer.latch.loopexit:
  br label %outer.latch

outer.latch:
  %outer.iv.next = add nuw nsw i64 %outer.iv, 1
  %outer.exitcond = icmp eq i64 %outer.iv.next, %m.sext
  br i1 %outer.exitcond, label %outer.exit, label %outer.header

outer.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %func.exit

func.exit:
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20869)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
