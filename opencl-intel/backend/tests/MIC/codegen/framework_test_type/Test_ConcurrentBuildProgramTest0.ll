; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; XFAIL: win32
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__k_original(float addrspace(1)* nocapture) nounwind

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

define void @__k_separated_args(float addrspace(1)* nocapture %pBuff, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  %pBuff.promoted = load float addrspace(1)* %pBuff, align 4
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %entry
  %add2092 = phi float [ %pBuff.promoted, %entry ], [ %add209, %thenBB ]
  %CurrWI..0 = phi i64 [ 0, %entry ], [ %"CurrWI++", %thenBB ]
  %add = fadd float %add2092, 1.000000e+00
  %add5 = fadd float %add, 1.000000e+00
  %add9 = fadd float %add5, 1.000000e+00
  %add13 = fadd float %add9, 1.000000e+00
  %add17 = fadd float %add13, 1.000000e+00
  %add21 = fadd float %add17, 1.000000e+00
  %add25 = fadd float %add21, 1.000000e+00
  %add29 = fadd float %add25, 1.000000e+00
  %add33 = fadd float %add29, 1.000000e+00
  %add37 = fadd float %add33, 1.000000e+00
  %add41 = fadd float %add37, 1.000000e+00
  %add45 = fadd float %add41, 1.000000e+00
  %add49 = fadd float %add45, 1.000000e+00
  %add53 = fadd float %add49, 1.000000e+00
  %add57 = fadd float %add53, 1.000000e+00
  %add61 = fadd float %add57, 1.000000e+00
  %add65 = fadd float %add61, 1.000000e+00
  %add69 = fadd float %add65, 1.000000e+00
  %add73 = fadd float %add69, 1.000000e+00
  %add77 = fadd float %add73, 1.000000e+00
  %add81 = fadd float %add77, 1.000000e+00
  %add85 = fadd float %add81, 1.000000e+00
  %add89 = fadd float %add85, 1.000000e+00
  %add93 = fadd float %add89, 1.000000e+00
  %add97 = fadd float %add93, 1.000000e+00
  %add101 = fadd float %add97, 1.000000e+00
  %add105 = fadd float %add101, 1.000000e+00
  %add109 = fadd float %add105, 1.000000e+00
  %add113 = fadd float %add109, 1.000000e+00
  %add117 = fadd float %add113, 1.000000e+00
  %add121 = fadd float %add117, 1.000000e+00
  %add125 = fadd float %add121, 1.000000e+00
  %add129 = fadd float %add125, 1.000000e+00
  %add133 = fadd float %add129, 1.000000e+00
  %add137 = fadd float %add133, 1.000000e+00
  %add141 = fadd float %add137, 1.000000e+00
  %add145 = fadd float %add141, 1.000000e+00
  %add149 = fadd float %add145, 1.000000e+00
  %add153 = fadd float %add149, 1.000000e+00
  %add157 = fadd float %add153, 1.000000e+00
  %add161 = fadd float %add157, 1.000000e+00
  %add165 = fadd float %add161, 1.000000e+00
  %add169 = fadd float %add165, 1.000000e+00
  %add173 = fadd float %add169, 1.000000e+00
  %add177 = fadd float %add173, 1.000000e+00
  %add181 = fadd float %add177, 1.000000e+00
  %add185 = fadd float %add181, 1.000000e+00
  %add189 = fadd float %add185, 1.000000e+00
  %add193 = fadd float %add189, 1.000000e+00
  %add197 = fadd float %add193, 1.000000e+00
  %add201 = fadd float %add197, 1.000000e+00
  %add205 = fadd float %add201, 1.000000e+00
  %add209 = fadd float %add205, 1.000000e+00
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %SyncBB
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

elseBB:                                           ; preds = %SyncBB
  store float %add209, float addrspace(1)* %pBuff, align 4
  ret void
}

define void @k(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 56
  %3 = bitcast i8* %2 to i64*
  %4 = load i64* %3, align 8
  %pBuff.promoted.i = load float addrspace(1)* %1, align 4
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %add2092.i = phi float [ %pBuff.promoted.i, %entry ], [ %add209.i, %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %add.i = fadd float %add2092.i, 1.000000e+00
  %add5.i = fadd float %add.i, 1.000000e+00
  %add9.i = fadd float %add5.i, 1.000000e+00
  %add13.i = fadd float %add9.i, 1.000000e+00
  %add17.i = fadd float %add13.i, 1.000000e+00
  %add21.i = fadd float %add17.i, 1.000000e+00
  %add25.i = fadd float %add21.i, 1.000000e+00
  %add29.i = fadd float %add25.i, 1.000000e+00
  %add33.i = fadd float %add29.i, 1.000000e+00
  %add37.i = fadd float %add33.i, 1.000000e+00
  %add41.i = fadd float %add37.i, 1.000000e+00
  %add45.i = fadd float %add41.i, 1.000000e+00
  %add49.i = fadd float %add45.i, 1.000000e+00
  %add53.i = fadd float %add49.i, 1.000000e+00
  %add57.i = fadd float %add53.i, 1.000000e+00
  %add61.i = fadd float %add57.i, 1.000000e+00
  %add65.i = fadd float %add61.i, 1.000000e+00
  %add69.i = fadd float %add65.i, 1.000000e+00
  %add73.i = fadd float %add69.i, 1.000000e+00
  %add77.i = fadd float %add73.i, 1.000000e+00
  %add81.i = fadd float %add77.i, 1.000000e+00
  %add85.i = fadd float %add81.i, 1.000000e+00
  %add89.i = fadd float %add85.i, 1.000000e+00
  %add93.i = fadd float %add89.i, 1.000000e+00
  %add97.i = fadd float %add93.i, 1.000000e+00
  %add101.i = fadd float %add97.i, 1.000000e+00
  %add105.i = fadd float %add101.i, 1.000000e+00
  %add109.i = fadd float %add105.i, 1.000000e+00
  %add113.i = fadd float %add109.i, 1.000000e+00
  %add117.i = fadd float %add113.i, 1.000000e+00
  %add121.i = fadd float %add117.i, 1.000000e+00
  %add125.i = fadd float %add121.i, 1.000000e+00
  %add129.i = fadd float %add125.i, 1.000000e+00
  %add133.i = fadd float %add129.i, 1.000000e+00
  %add137.i = fadd float %add133.i, 1.000000e+00
  %add141.i = fadd float %add137.i, 1.000000e+00
  %add145.i = fadd float %add141.i, 1.000000e+00
  %add149.i = fadd float %add145.i, 1.000000e+00
  %add153.i = fadd float %add149.i, 1.000000e+00
  %add157.i = fadd float %add153.i, 1.000000e+00
  %add161.i = fadd float %add157.i, 1.000000e+00
  %add165.i = fadd float %add161.i, 1.000000e+00
  %add169.i = fadd float %add165.i, 1.000000e+00
  %add173.i = fadd float %add169.i, 1.000000e+00
  %add177.i = fadd float %add173.i, 1.000000e+00
  %add181.i = fadd float %add177.i, 1.000000e+00
  %add185.i = fadd float %add181.i, 1.000000e+00
  %add189.i = fadd float %add185.i, 1.000000e+00
  %add193.i = fadd float %add189.i, 1.000000e+00
  %add197.i = fadd float %add193.i, 1.000000e+00
  %add201.i = fadd float %add197.i, 1.000000e+00
  %add205.i = fadd float %add201.i, 1.000000e+00
  %add209.i = fadd float %add205.i, 1.000000e+00
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %4
  br i1 %check.WI.iter.i, label %thenBB.i, label %__k_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__k_separated_args.exit:                          ; preds = %SyncBB.i
  store float %add209.i, float addrspace(1)* %1, align 4
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (float addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__k_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *", metadata !"opencl_k_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @k}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 1}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"float*"}
!5 = metadata !{metadata !"pBuff"}
