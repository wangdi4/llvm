; This test verifies that a direct load from private alloca is considered profitable.

; RUN: opt -opaque-pointers -vplan-vec -vplan-enable-soa -vplan-dump-soa-info \
; RUN: -disable-output -disable-vplan-codegen %s 2>&1 | FileCheck %s

; CHECK: SOA profitability:
; CHECK: SOASafe = sPrivateStorage Profitable = 1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare void @llvm.lifetime.start.p0i8(i64, ptr nocapture)

declare i64 @_Z13get_global_idj(i32) local_unnamed_addr

declare void @llvm.lifetime.end.p0i8(i64, ptr nocapture)

define void @_ZGVdN8uuuu_test_fn(ptr addrspace(1) %src) {
entry:
  %sPrivateStorage = alloca [2 x i8], align 4
  %call = tail call i64 @_Z13get_global_idj(i32 0)
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
%entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(ptr addrspace(1) %src), "QUAL.OMP.PRIVATE:TYPED"(ptr %sPrivateStorage, i8 zeroinitializer, i32 2)]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = sext i32 %index to i64
  %add = add nuw i64 %0, %call
  call void @llvm.lifetime.start.p0i8(i64 16, ptr nonnull %sPrivateStorage)
  %1 = load i8, i8 addrspace(1)* %src, align 4
  store i8 %1, ptr %sPrivateStorage, align 4
  call void @llvm.lifetime.end.p0i8(i64 16, ptr nonnull %sPrivateStorage)
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
