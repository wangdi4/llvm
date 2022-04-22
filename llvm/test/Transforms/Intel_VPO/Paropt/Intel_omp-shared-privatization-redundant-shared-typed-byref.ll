; RUN: opt -enable-new-pm=0 -vpo-paropt-shared-privatization -S <%s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-shared-privatization)' -S <%s 2>&1 | FileCheck %s

; Test src:
;
; # include <stdlib.h>
; a(size_t &b) {
; #pragma omp parallel
;   b;
; }

; CHECK:  "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED:BYREF.TYPED"(i64** null, i64 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(i64** %b.addr, i64* null, i32 1)

define i32 @a(i64* %b) {
bb0:
  %b.addr = alloca i64*, align 8
  store i64* %b, i64** %b.addr, align 8
  br label %bb1

bb1:                                              ; preds = %bb0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED:BYREF.TYPED"(i64** %b.addr, i64 0, i32 1) ]
  br label %bb2

bb2:                                              ; preds = %bb1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
