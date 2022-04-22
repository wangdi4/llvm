; RUN: opt -enable-new-pm=0 -opaque-pointers -vpo-paropt-shared-privatization -S <%s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='function(vpo-paropt-shared-privatization)' -S <%s 2>&1 | FileCheck %s

; Test src:
;
; # include <stdlib.h>
; a() {
;   size_t b;
; #pragma omp parallel
;   b;
; }

; CHECK:  "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(ptr null), "QUAL.OMP.PRIVATE:TYPED"(ptr %b, i64 0, i32 1)

define i32 @a() {
bb0:
  %b = alloca i64, align 8
  br label %bb1

bb1:                                              ; preds = %bb0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(ptr %b) ]
  br label %bb2

bb2:                                              ; preds = %bb1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
