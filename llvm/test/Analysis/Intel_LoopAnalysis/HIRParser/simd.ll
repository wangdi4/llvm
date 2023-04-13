; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the simd loop verifying that it has the simd directives prepended/appended to it

; CHECK: %t4 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]

; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   %mask5 = (%veccast.3)[i1];
; CHECK: |   if (%mask5 != 0)
; CHECK: |   {
; CHECK: |      %0 = (%veccast.1)[i1];
; CHECK: |      %1 = (%veccast.2)[i1];
; CHECK: |      (%veccast)[i1] = %0 + %1;
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: @llvm.directive.region.exit(%t4); [ DIR.OMP.END.SIMD() ]

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
  %t4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
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
  br i1 %vlcond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %t4) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  %cast = bitcast i32* %veccast to <4 x i32>*
  %vec_ret = load <4 x i32>, <4 x i32>* %cast
  ret <4 x i32> %vec_ret
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

