; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-link-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=16 -subscript -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; Check scatter/gather on A[i].x pattern where i is consecutive

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

;CHECK: test_positive
;CHECK: call <16 x i16> @"internal.gather.v16i16[i32].m1"({{.*}}, <16 x i32> <i32 0, i32 9, i32 18, i32 27, i32 36, i32 45, i32 54, i32 63, i32 72, i32 81, i32 90, i32 99, i32 108, i32 117, i32 126, i32 135>, {{.*}})
;CHECK: ret void
;CHECK: test_negative
;CHECK-NOT: internal.gather
;CHECK: ret void

%struct._TEST_STRUCT_PACKED = type <{ [3 x i8], i32, i8, i16, double }>

; Function Attrs: nounwind
define void @test_positive(i32 addrspace(1)* nocapture %out, %struct._TEST_STRUCT_PACKED addrspace(1)* nocapture %a) #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #2
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %zz = getelementptr inbounds %struct._TEST_STRUCT_PACKED addrspace(1)* %a, i64 %idxprom, i32 3
  %0 = load i16 addrspace(1)* %zz, align 1
  %conv1 = sext i16 %0 to i32
  %mul = mul nsw i32 %conv1, 3
  %arrayidx3 = getelementptr inbounds i32 addrspace(1)* %out, i64 %idxprom
  store i32 %mul, i32 addrspace(1)* %arrayidx3, align 4
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) #1

; Function Attrs: nounwind
define void @test_negative(i32 addrspace(1)* nocapture %out, %struct._TEST_STRUCT_PACKED addrspace(1)* nocapture %a) #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #2
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %yy = getelementptr inbounds %struct._TEST_STRUCT_PACKED addrspace(1)* %a, i64 %idxprom, i32 1
  %0 = load i32 addrspace(1)* %yy, align 1
  %mul = mul nsw i32 %0, 3
  %arrayidx2 = getelementptr inbounds i32 addrspace(1)* %out, i64 %idxprom
  store i32 %mul, i32 addrspace(1)* %arrayidx2, align 4
  ret void
}
