; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the simd loop verifying that it has the simd directives prepended/appended to it. The begin and end directives in this case are not located in immediate loop predecessor/successor blocks.

; CHECK: @llvm.intel.directive(!7);
; CHECK: @llvm.intel.directive.qual.opndlist(!8,  %a,  %b,  %mask);
; CHECK: @llvm.intel.directive.qual.opnd.i32(!9,  4);
; CHECK: @llvm.intel.directive.qual(!10);
; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   %mask5 = (%veccast.3)[i1];
; CHECK: |   if (%mask5 != 0)
; CHECK: |   {
; CHECK: |      %0 = (%veccast.1)[i1];
; CHECK: |      %1 = (%veccast.2)[i1];
; CHECK: |      (%veccast)[i1] = %0 + %1;
; CHECK: |   }
; CHECK: + END LOOP
; CHECK: @llvm.intel.directive(!12);

; ModuleID = 'simd.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @vec_sum(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, i32* %a.addr, align 4
  store i32 %b, i32* %b.addr, align 4
  %0 = load i32, i32* %a.addr, align 4
  %1 = load i32, i32* %b.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add
}

define <4 x i32> @_ZGVxM4vv_vec_sum(<4 x i32> %a, <4 x i32> %b, <4 x i32> %mask) {
entry:
  %vec_a.addr = alloca <4 x i32>
  %vec_b.addr = alloca <4 x i32>
  %vec_mask = alloca <4 x i32>
  %vec_retval = alloca <4 x i32>
  store <4 x i32> %a, <4 x i32>* %vec_a.addr, align 4
  store <4 x i32> %b, <4 x i32>* %vec_b.addr, align 4
  store <4 x i32> %mask, <4 x i32>* %vec_mask
  %veccast = bitcast <4 x i32>* %vec_retval to i32*
  %veccast.1 = bitcast <4 x i32>* %vec_a.addr to i32*
  %veccast.2 = bitcast <4 x i32>* %vec_b.addr to i32*
  %veccast.3 = bitcast <4 x i32>* %vec_mask to i32*
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  call void @llvm.intel.directive(metadata !7)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !8, <4 x i32> %a, <4 x i32> %b, <4 x i32> %mask)
  call void @llvm.intel.directive.qual.opnd.i32(metadata !9, i32 4)
  call void @llvm.intel.directive.qual(metadata !10)
  br label %simd.loop.pre

simd.loop.pre:                                    ; preds = %simd.begin.region
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.loop.pre
  %index = phi i32 [ 0, %simd.loop.pre ], [ %indvar, %simd.loop.exit ]
  %maskgep = getelementptr i32, i32* %veccast.3, i32 %index
  %mask5 = load i32, i32* %maskgep
  %maskcond = icmp ne i32 %mask5, 0
  br i1 %maskcond, label %simd.loop.then, label %simd.loop.else

simd.loop.then:                                   ; preds = %simd.loop
  %vecgep = getelementptr i32, i32* %veccast.1, i32 %index
  %0 = load i32, i32* %vecgep, align 4
  %vecgep4 = getelementptr i32, i32* %veccast.2, i32 %index
  %1 = load i32, i32* %vecgep4, align 4
  %add = add nsw i32 %0, %1
  %vec_gep = getelementptr i32, i32* %veccast, i32 %index
  store i32 %add, i32* %vec_gep
  br label %simd.loop.exit

simd.loop.else:                                   ; preds = %simd.loop
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop.else, %simd.loop.then
  %indvar = add nuw i32 1, %index
  %vlcond = icmp ult i32 %indvar, 4
  br i1 %vlcond, label %simd.loop, label %simd.loop.post

simd.loop.post:                                   ; preds = simd.loop.exit
  br label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.post
  call void @llvm.intel.directive(metadata !11)
  br label %return

return:                                           ; preds = %simd.end.region
  %cast = bitcast i32* %veccast to <4 x i32>*
  %vec_ret = load <4 x i32>, <4 x i32>* %cast
  ret <4 x i32> %vec_ret
}

declare void @llvm.intel.directive(metadata)

declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32)

declare void @llvm.intel.directive.qual.opndlist(metadata, ...)

declare void @llvm.intel.directive.qual(metadata)

!cilk.functions = !{!0}
!llvm.ident = !{!6}

!0 = !{i32 (i32, i32)* @vec_sum, !1, !2, !3, !4, !5}
!1 = !{!"elemental"}
!2 = !{!"arg_name", !"a", !"b"}
!3 = !{!"arg_step", i32 undef, i32 undef}
!4 = !{!"arg_alig", i32 undef, i32 undef}
!5 = !{!"vec_length", i32 undef, i32 4}
!6 = !{!"clang version 3.8.0 (branches/vpo 1412)"}
!7 = !{!"DIR.OMP.SIMD"}
!8 = !{!"QUAL.OMP.PRIVATE"}
!9 = !{!"QUAL.OMP.SIMDLEN"}
!10 = !{!"QUAL.LIST.END"}
!11 = !{!"DIR.OMP.END.SIMD"}
