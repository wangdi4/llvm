; RUN: opt %s -passes=sycl-kernel-indirect-call-lowering -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes=sycl-kernel-indirect-call-lowering -S | FileCheck %s

define void @foo() {
simd.begin.region:
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  br label %VPlannedBB

VPlannedBB:                                       ; preds = %simd.loop.preheader
  br label %VPlannedBB1

VPlannedBB1:                                      ; preds = %VPlannedBB
  br i1 false, label %scalar.ph, label %vector.ph

vector.ph:                                        ; preds = %VPlannedBB1
  br label %vector.body

vector.body:                                      ; preds = %VPlannedBB3, %vector.ph
  %uni.phi = phi i32 [ 0, %vector.ph ], [ %2, %VPlannedBB3 ]
  %vec.phi = phi <16 x i32> [ <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>, %vector.ph ], [ %1, %VPlannedBB3 ]
  %0 = load ptr, ptr poison, align 8
  call void %0(<16 x i32> <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>, <16 x ptr> undef)
  br label %VPlannedBB3

VPlannedBB3:                                      ; preds = %vector.body
  %1 = add nuw <16 x i32> %vec.phi, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %2 = add nuw i32 %uni.phi, 16
  %3 = icmp ult i32 %2, 16
  br i1 false, label %vector.body, label %VPlannedBB4

VPlannedBB4:                                      ; preds = %VPlannedBB3
  br label %middle.block

middle.block:                                     ; preds = %VPlannedBB4
  br i1 false, label %scalar.ph, label %VPlannedBB5

scalar.ph:                                        ; preds = %middle.block, %VPlannedBB1
  %uni.phi6 = phi i32 [ 16, %middle.block ], [ 0, %VPlannedBB1 ]
  br label %VPlannedBB7

VPlannedBB7:                                      ; preds = %scalar.ph
  br label %simd.loop

VPlannedBB5:                                      ; preds = %simd.loop.exit, %middle.block
  %uni.phi8 = phi i32 [ %indvar, %simd.loop.exit ], [ 16, %middle.block ]
  br label %VPlannedBB9

VPlannedBB9:                                      ; preds = %VPlannedBB5
  br label %simd.end.region

simd.loop:                                        ; preds = %VPlannedBB7, %simd.loop.exit
  %index = phi i32 [ %uni.phi6, %VPlannedBB7 ], [ %indvar, %simd.loop.exit ]
  call void (ptr, i32, ptr, ...) @__intel_indirect_call.2(ptr poison, i32 3, ptr undef) #0
; CHECK:  %4 = load ptr, ptr poison, align 8
; CHECK:  call void %4(<16 x i32> <i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>, <16 x ptr> undef, <16 x i32> <i32 -1, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0>)
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop, label %VPlannedBB5

simd.end.region:                                  ; preds = %VPlannedBB9
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

declare void @__intel_indirect_call.2(ptr, i32, ptr, ...)

attributes #0 = { "vector-variants"="_ZGVeM16vv___intel_indirect_call_XXX,_ZGVeN16vv___intel_indirect_call_XXX" }

; DEBUGIFY-NOT: WARNING
