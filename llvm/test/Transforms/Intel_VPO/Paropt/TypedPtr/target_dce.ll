; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,sroa),ipsccp,function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Original code:
; void foo() {
;   int x = 1;
;   int y = 1;
;   if (x < y) {
; #pragma omp target
;     ;
;   }
; }

; The test checks that Paropt completes successfully, when the target region
; is optimized away (due to constant propagation: sroa+ipsccp):
; CHECK-NOT: define internal void @__omp_offloading_804_5200f18_foo_l5()

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

define dso_local void @foo() {
entry:
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %0 = bitcast i32* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0)
  store i32 1, i32* %x, align 4
  %1 = bitcast i32* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1)
  store i32 1, i32* %y, align 4
  %2 = load i32, i32* %x, align 4
  %3 = load i32, i32* %y, align 4
  %cmp = icmp slt i32 %2, %3
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %5 = bitcast i32* %y to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %5)
  %6 = bitcast i32* %x to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %6)
  ret void
}

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2052, i32 85987096, !"foo", i32 5, i32 0, i32 0}
