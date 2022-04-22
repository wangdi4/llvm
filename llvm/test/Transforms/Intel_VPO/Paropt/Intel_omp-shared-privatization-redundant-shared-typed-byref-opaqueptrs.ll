; RUN: opt -enable-new-pm=0 -opaque-pointers -vpo-paropt-shared-privatization -S <%s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='function(vpo-paropt-shared-privatization)' -S <%s 2>&1 | FileCheck %s

; Test src:
;
; # include <stdlib.h>
; a(size_t &b) {
; #pragma omp parallel
;   b;
; }

; CHECK:  "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED:BYREF.TYPED"(ptr null, i64 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %b.addr, ptr null, i32 1)

define i32 @a(ptr %b) {
bb0:
  %b.addr = alloca ptr, align 8
  store ptr %b, ptr %b.addr, align 8
  br label %bb1

bb1:                                              ; preds = %bb0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED:BYREF.TYPED"(ptr %b.addr, i64 0, i32 1) ]
  br label %bb2

bb2:                                              ; preds = %bb1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
