; RUN: opt -passes=instcombine -S %s | FileCheck %s

; CHECK-NOT: call{{.*}}directive

; When IC performs unreachable block removal, it may delete an exit block
; without deleting the corresponding entry.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @_Z3tstv() {
entry:
  br label %preheader

preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.SIMDLEN"(i32 8),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0),
    "QUAL.OMP.NORMALIZED.UB"(ptr null, i32 0) ]
  br label %exitblock

DIR.OMP.END.SIMD.2:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %exitblock

exitblock:
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

attributes #0 = { nounwind }
