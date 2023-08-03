; This test checks that in case of recursive-types, we do not enter an infinite
; loop and pass the ScalableTypes check in Legality.

; RUN: opt -passes=vplan-vec -S %s | FileCheck %s
; CHECK: vector.body

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.Node = type { i32, i32, ptr addrspace(1) }
; Function Attrs: convergent nounwind
define void @test_scalable_type_passthrough() {
entry:
  %list.pNodes = alloca ptr addrspace(1), align 8
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),  "QUAL.OMP.SIMDLEN"(i32 2)]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %idx = sext i32 %index to i64
  %add = add nuw i64 %idx, 1
  %sext = shl i64 %add, 32
  %idxprom = ashr exact i64 %sext, 32
  %load.pNodes = load ptr addrspace(1), ptr %list.pNodes, align 8
  %global_id = getelementptr inbounds %struct.Node, ptr addrspace(1) %load.pNodes, i64 %add, i32 0
  br label %simd.loop.exit

simd.loop.exit:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
