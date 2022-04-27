; RUN: opt -instcombine -S %s | FileCheck %s
; RUN: opt -passes='function(instcombine)' -S %s | FileCheck %s
;
; // Test C source:
; void bar(int*);
; void foo()
; {
;     int *a = (int *)malloc(sizeof(int));
;     #pragma omp target
;     {}
;     bar(a);
; }
;
; Check that instcombine does not move the bitcast into the TARGET region.
; Before the fix, instcombine moved the bitcast from "entry" to the
; top of "split" to be closer to its use. As a result, it landed inside
; the TARGET region, corrupting the OpenMP semantics.
;
; Check that there is no bitcast in the "split" BB:
; CHECK: split:
; CHECK-NOT: bitcast i8* %call to i32*

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  %call = call i8* @malloc(i64 4)
  %0 = bitcast i8* %call to i32*
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  br label %split

split:                           ; preds = %entry
  ; make sure instcombine does not move the bitcast instruction here
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @bar(i32* %0)
  ret void
}

; Function Attrs: nofree nounwind
declare dso_local noalias i8* @malloc(i64) local_unnamed_addr #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local void @bar(i32*) local_unnamed_addr #3

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 57, i32 -698066943, !"foo", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}
