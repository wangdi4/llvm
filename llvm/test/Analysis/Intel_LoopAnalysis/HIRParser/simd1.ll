; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the simd loop verifying that it has the simd directives prepended/appended to it. The begin and end directives in this case are not located in immediate loop predecessor/successor blocks.

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
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add
}

define <4 x i32> @_ZGVxM4vv_vec_sum(<4 x i32> %a, <4 x i32> %b, <4 x i32> %mask) {
entry:
  %vec_a.addr = alloca <4 x i32>
  %vec_b.addr = alloca <4 x i32>
  %vec_mask = alloca <4 x i32>
  %vec_retval = alloca <4 x i32>
  store <4 x i32> %a, ptr %vec_a.addr, align 4
  store <4 x i32> %b, ptr %vec_b.addr, align 4
  store <4 x i32> %mask, ptr %vec_mask
  %veccast = bitcast ptr %vec_retval to ptr
  %veccast.1 = bitcast ptr %vec_a.addr to ptr
  %veccast.2 = bitcast ptr %vec_b.addr to ptr
  %veccast.3 = bitcast ptr %vec_mask to ptr
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %t4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %simd.loop.pre

simd.loop.pre:                                    ; preds = %simd.begin.region
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.loop.pre
  %index = phi i32 [ 0, %simd.loop.pre ], [ %indvar, %simd.loop.exit ]
  %maskgep = getelementptr i32, ptr %veccast.3, i32 %index
  %mask5 = load i32, ptr %maskgep
  %maskcond = icmp ne i32 %mask5, 0
  br i1 %maskcond, label %simd.loop.then, label %simd.loop.else

simd.loop.then:                                   ; preds = %simd.loop
  %vecgep = getelementptr i32, ptr %veccast.1, i32 %index
  %0 = load i32, ptr %vecgep, align 4
  %vecgep4 = getelementptr i32, ptr %veccast.2, i32 %index
  %1 = load i32, ptr %vecgep4, align 4
  %add = add nsw i32 %0, %1
  %vec_gep = getelementptr i32, ptr %veccast, i32 %index
  store i32 %add, ptr %vec_gep
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
  call void @llvm.directive.region.exit(token %t4) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  %cast = bitcast ptr %veccast to ptr
  %vec_ret = load <4 x i32>, ptr %cast
  ret <4 x i32> %vec_ret
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

