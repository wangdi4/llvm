; RUN: opt -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s

; Check that we can handle outlining a parallel region like this:

;             b0 <par>
;             /|
;            / |
;           /  |    b1 <no predecessors>
;          /   |    /|
;         b2   |   / |
;           \  |  /  |
;            \ | /   |
;             \|/    |
;              b3    |
;              |     |
;              |     |
;              b4    |
;         <end.par>  |
;               \    |
;                \   |
;                 \  |
;                  \ |
;                   \|
;                   b5

; Check that the branch b1 -> b3 is deleted, but b1 -> b5 is left as is.
; CHECK-LABEL: define dso_local void @foo()
; CHECK: b1
; CHECK-NEXT: br label %b5
; CHECK: b5:
; CHECK-NEXT: %arg1 = phi i1 [ true, %b1 ], [ false, %{{[^ ]+}} ]

; Check that an outlined function is created for the parallel region.
; CHECK-LABEL: define internal void @foo.b0{{[^ ]*}}(i32* %tid, i32* %bid)
; CHECK: b0:
; CHECK-NEXT: br i1 true, label %b2, label %b3
; CHECK: b2:
; CHECK-NEXT: br label %b3
; CHECK: b3:
; CHECK-NEXT: %arg = phi i1 [ true, %b2 ], [ false, %b0 ]
; CHECK-NEXT: call void @_Z3barb(i1 zeroext %arg)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  br label %b0

b0:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br i1 true, label %b2, label %b3

b1:                                               ; No predecessors!
  br i1 false, label %b3, label %b5

b2:                                               ; preds = %b0
  br label %b3

b3:                                               ; preds = %b0, %b1, %b2
  %arg = phi i1 [ true, %b1 ], [ true, %b2 ], [ false, %b0 ]
  call void @_Z3barb(i1 zeroext %arg) #1
  br label %b4

b4:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %b5

b5:                                               ; preds = %b1, %b4
  %arg1 = phi i1 [ true, %b1 ], [ false, %b4 ]
  call void @_Z3bazb(i1 zeroext %arg1) #1
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @_Z3barb(i1 zeroext) #2
declare dso_local void @_Z3bazb(i1 zeroext) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
