; CMPLRLLVM-33406: Incorrect handling of "nocapture" skips the analysis when
; the arg type is a vector-of-pointer type (or any type that is not a scalar
; pointer). The function call to "hoge" is not seen as a ref of "%tmp", even
; though %vecaddr aliases %tmp.

; RUN: opt -dse -S %s | FileCheck %s
; CHECK: store <16 x i32> <i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777>, <16 x i32>* %tmp

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.foo = type { [6 x i32] }
%struct.pluto = type { %struct.wombat }
%struct.wombat = type { [1 x i64] }

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #0

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1


define void @widget(float addrspace(1)* %arg, %struct.pluto* %arg1, %struct.pluto* %arg2, %struct.pluto* %arg3, float addrspace(1)* %arg4, %struct.pluto* %arg5, %struct.pluto* %arg6, %struct.pluto* %arg7, i8 addrspace(3)* %arg8, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %arg9, i64* %arg10, [4 x i64] %arg11, i8* %arg12, {}* %arg13) local_unnamed_addr {
bb:
  %tmp = alloca <16 x i32>, align 4
  br i1 undef, label %bb16, label %bb14

bb14:                                             ; preds = %bb
  %tmp15 = bitcast <16 x i32>* %tmp to i8*
  %intcast = bitcast <16 x i32>* %tmp to i32*
  %vecaddr = getelementptr i32, i32* %intcast, <16 x i64> <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  store <16 x i32> <i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777, i32 777>, <16 x i32>* %tmp, align 4
  call fastcc void @hoge(<16 x %struct.foo addrspace(4)*> nocapture align 4 dereferenceable(24) undef, <16 x i64> <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>, <16 x i32*> nocapture readonly %vecaddr, <16 x i64 addrspace(4)*> nocapture readonly undef, [4 x i64] %arg11)
  call void @llvm.lifetime.end.p0i8(i64 64, i8* nonnull %tmp15) #8
  br label %bb16

bb16:                                             ; preds = %bb14, %bb
  ret void
}

declare hidden fastcc void @hoge(<16 x %struct.foo addrspace(4)*>, <16 x i64>, <16 x i32*>, <16 x i64 addrspace(4)*>, [4 x i64]) unnamed_addr 

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.experimental.noalias.scope.decl(metadata) #0

attributes #0 = { inaccessiblememonly nofree nosync nounwind willreturn }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #8 = { nounwind }

