; RUN: opt -bugpoint-enable-legacy-pm -vpo-wrncollection -analyze %s | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -disable-output %s 2>&1 | FileCheck %s

; CMPLRLLVM_901: BY-REF support
; In the test below, bbb is passed by reference, so its representation
; in the firstprivate clause has the BYREF modifier, resulting
; in a tag name like this: "QUAL.OMP.FIRSTPRIVATE:BYREF"
;
; This test verifies that WRN construction parses this tag name
; and represents the BYREF property correctly. Given the test below
;
; void foo(int aaa, int &bbb) {
;   #pragma omp parallel firstprivate(aaa,bbb)
;     bbb = aaa + 123;
; }
;
; the WRN graph dump should result in something like this:
;
; BEGIN PARALLEL ID=1 {
;
;   IF_EXPR: UNSPECIFIED
;   NUM_THREADS: UNSPECIFIED
;   DEFAULT: UNSPECIFIED
;   PROCBIND: UNSPECIFIED
;   SHARED clause: UNSPECIFIED
;   PRIVATE clause: UNSPECIFIED
;   FIRSTPRIVATE clause (size=2): TYPED(ptr %aaa.addr, TYPE: i32, NUM_ELEMENTS: i32 1) BYREF,TYPED(ptr %bbb.addr, TYPE: i32, NUM_ELEMENTS: i32 1)
;   REDUCTION clause: UNSPECIFIED
;   ALLOCATE clause: UNSPECIFIED
;   COPYIN clause: UNSPECIFIED
;
;   EntryBB: DIR.OMP.PARALLEL.2
;   ExitBB: DIR.OMP.END.PARALLEL.4
;
; } END PARALLEL ID=1

; CHECK: FIRSTPRIVATE clause (size=2):{{.*}} BYREF,TYPED(ptr %bbb.addr, {{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3fooiRi(i32 noundef %aaa, ptr noundef nonnull align 4 dereferenceable(4) %bbb) #0 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  %0 = load ptr, ptr %bbb.addr, align 8
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %aaa.addr, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:BYREF.TYPED"(ptr %bbb.addr, i32 0, i32 1) ]
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.2
  %2 = load i32, ptr %aaa.addr, align 4
  %add = add nsw i32 %2, 123
  %3 = load ptr, ptr %bbb.addr, align 8
  store i32 %add, ptr %3, align 4
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.PARALLEL.3
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.5

DIR.OMP.END.PARALLEL.5:                           ; preds = %DIR.OMP.END.PARALLEL.4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
