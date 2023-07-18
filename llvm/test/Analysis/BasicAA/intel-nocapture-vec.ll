; CMPLRLLVM-33406: Incorrect handling of "nocapture" skips the analysis when
; the arg type is a vector-of-pointer type (or any type that is not a scalar
; pointer). The function call to "hoge" is not seen as a ref of "%tmp", even
; though %vecaddr aliases %tmp.

; RUN: opt -passes="dse" -S %s | FileCheck %s
; CHECK: store <16 x i32> <i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777>, ptr %tmp

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.foo = type { [6 x i32] }
%struct.pluto = type { %struct.wombat }
%struct.wombat = type { [1 x i64] }

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #0

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, ptr nocapture) #1


define void @widget(ptr addrspace(1) %arg, ptr %arg1, ptr %arg2, ptr %arg3, ptr addrspace(1) %arg4, ptr %arg5, ptr %arg6, ptr %arg7, ptr addrspace(3) %arg8, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %arg9, ptr %arg10, [4 x i64] %arg11, ptr %arg12, ptr %arg13) local_unnamed_addr {
bb:
  %tmp = alloca <16 x i32>, align 4
  br i1 undef, label %bb16, label %bb14

bb14:                                             ; preds = %bb
  %tmp15 = bitcast ptr %tmp to ptr
  %intcast = bitcast ptr %tmp to ptr
  %vecaddr = getelementptr i32, ptr %intcast, <16 x i64> <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  store <16 x i32> <i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777>, ptr %tmp, align 4
  call fastcc void @hoge(<16 x ptr addrspace(4)> nocapture align 4 dereferenceable(24) undef, <16 x i64> <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>, <16 x ptr> nocapture readonly %vecaddr, <16 x ptr addrspace(4)> nocapture readonly undef, [4 x i64] %arg11)
  call void @llvm.lifetime.end.p0i8(i64 64, ptr nonnull %tmp15) #8
  br label %bb16

bb16:                                             ; preds = %bb14, %bb
  ret void
}

declare hidden fastcc void @hoge(<16 x ptr addrspace(4)>, <16 x i64>, <16 x ptr>, <16 x ptr addrspace(4)>, [4 x i64]) unnamed_addr 

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.experimental.noalias.scope.decl(metadata) #0

attributes #0 = { inaccessiblememonly nofree nosync nounwind willreturn }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #8 = { nounwind }

