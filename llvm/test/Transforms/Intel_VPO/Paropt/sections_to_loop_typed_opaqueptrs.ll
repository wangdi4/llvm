; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S <%s | FileCheck %s --check-prefix=SECTOLOOP
; RUN: opt -passes="function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)" -S <%s | FileCheck %s --check-prefix=SECTOLOOP

; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt -S <%s | FileCheck %s --check-prefix=TFORM
; RUN: opt -passes="function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring),vpo-paropt" -S <%s | FileCheck %s --check-prefix=TFORM

; a() {
; #pragma omp sections
;   { ; }
; }

; Check that typed IV/UB clauses are emitted after the par-sec transformation
; (invoked as part of collapse pass currently).

; SECTOLOOP: %num.sects = alloca i32, align 4
; SECTOLOOP: store i32 0, ptr %num.sects, align 4
; SECTOLOOP: %.sloop.iv.1 = alloca i32, align 4
; SECTOLOOP: [ "DIR.OMP.SECTIONS"(), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.sloop.iv.1, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %num.sects, i32 0) ]

; Ensure that Paropt is able to handle the incoming IR and emit code for it.
; TFORM: call void @__kmpc_for_static_init_4({{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @a() {
entry:
  %retval = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTIONS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SECTIONS"() ]
  %1 = load i32, ptr %retval, align 4
  ret i32 %1
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
