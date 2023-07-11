; REQUIRES: asserts

; Test with opaque pointer
; RUN: opt -opaque-pointers=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR %s
; RUN: opt -opaque-pointers=1 -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR %s

; Test with typed pointer
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck --check-prefixes=TYPEPTR %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck --check-prefixes=TYPEPTR %s

; Test src:
;
; #include <stdio.h>
;
; short *b = (short *)100;
;
; int main() {
;   char **c = (char **)100;
; #pragma omp for linear(b : 2) linear(c : 2)
;   for (int i = 0; i < 10; ++i) {
;     b += 2;
;     c += 2;
;   }
; }

; Test with opaque pointer
; OPQPTR: LINEAR clause (size=2): (TYPED(PTR_TO_PTR(ptr @b), TYPE: ptr, POINTEE_TYPE: i16, NUM_ELEMENTS: i32 1), i32 2) (TYPED(PTR_TO_PTR(ptr %c), TYPE: ptr, POINTEE_TYPE: ptr, NUM_ELEMENTS: i32 1), i32 2)

; Test with typed pointer
; TYPEPTR: LINEAR clause (size=2): (TYPED(PTR_TO_PTR(i16** @b), TYPE: i16*, POINTEE_TYPE: i16, NUM_ELEMENTS: i32 1), i32 2) (TYPED(PTR_TO_PTR(i8*** %c), TYPE: i8**, POINTEE_TYPE: i8*, NUM_ELEMENTS: i32 1), i32 2)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = dso_local global i16* inttoptr (i64 100 to i16*), align 8

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %c = alloca i8**, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i8** inttoptr (i64 100 to i8**), i8*** %c, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 9, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.LINEAR:PTR_TO_PTR.TYPED"(i16** @b, i16 0, i32 1, i32 2), "QUAL.OMP.LINEAR:PTR_TO_PTR.TYPED"(i8*** %c, i8* null, i32 1, i32 2), "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv, i32 0), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb, i32 0, i32 1), "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub, i32 0), "QUAL.OMP.PRIVATE:TYPED"(i32* %i, i32 0, i32 1) ]
  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %5 = load i16*, i16** @b, align 8
  %add.ptr = getelementptr inbounds i16, i16* %5, i64 2
  store i16* %add.ptr, i16** @b, align 8
  %6 = load i8**, i8*** %c, align 8
  %add.ptr1 = getelementptr inbounds i8*, i8** %6, i64 2
  store i8** %add.ptr1, i8*** %c, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, i32* %.omp.iv, align 4
  %add2 = add nsw i32 %7, 1
  store i32 %add2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  %8 = load i32, i32* %retval, align 4
  ret i32 %8
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
