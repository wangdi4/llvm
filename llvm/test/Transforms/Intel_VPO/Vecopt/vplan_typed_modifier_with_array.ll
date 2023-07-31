; Test for array in private clause with typed modifier 

; RUN: opt -vplan-enable-soa=false -S -passes=vplan-vec -vplan-force-vf=2 %s | FileCheck %s

; CHECK:  [[VEC1:%.*]] = alloca [2 x [624 x i32]], align 4

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; Function Attrs: nounwind
define void @"test_typed_modifier"(){
  %1 = alloca [624 x i32], align 4
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %1, i32 0, i32 624) ]
  br label %simd.loop

simd.loop:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %2 = sext i32 %index to i64
  %add = add nuw i64 %2, %2
  %add1 = trunc i64 %add to i32
  store i32 %add1, ptr %1, align 4
  br label %simd.loop.exit

simd.loop.exit:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:
  ret void
}
