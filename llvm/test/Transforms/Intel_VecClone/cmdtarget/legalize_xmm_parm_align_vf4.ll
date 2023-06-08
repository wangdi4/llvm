; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s

; Test for vector variant arguments and return type legalization as per VFABI
; Check to see that aligned parameters are properly marked with the 'align'
; attr in cloned variants, and that alignment assumptions are inserted in the
; appropriate locations.
; Return: i32
; Arguments: ptr, ptr, ptr
; ISA class: XMM
; "vector-variants"="_ZGVxN4va16ua32l4a64_foo,_ZGVxM4va16ua32l4a64_foo"
; Origin: aligned_params_opaque.ll

; Function Attrs: nounwind uwtable
define i32 @foo(ptr %a, ptr %b, ptr %c) #0 {
; CHECK-LABEL: define x86_regcallcc <4 x i32> @_ZGVxN4va16ua32l4a64_foo
; CHECK-SAME: (<2 x ptr> align 16 [[A_0:%.*]], <2 x ptr> align 16 [[A_1:%.*]], ptr align 32 [[B:%.*]], ptr align 64 [[C:%.*]]) #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ALLOCA_C:%.*]] = alloca ptr, align 8
; CHECK-NEXT:    store ptr [[C]], ptr [[ALLOCA_C]], align 8
; CHECK-NEXT:    [[ALLOCA_B:%.*]] = alloca ptr, align 8
; CHECK-NEXT:    store ptr [[B]], ptr [[ALLOCA_B]], align 8
; CHECK-NEXT:    [[VEC_A:%.*]] = alloca <4 x ptr>, align 32
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <4 x i32>, align 16
; CHECK-NEXT:    [[A_ADDR:%.*]] = alloca ptr, align 4
; CHECK-NEXT:    [[B_ADDR:%.*]] = alloca ptr, align 4
; CHECK-NEXT:    [[C_ADDR:%.*]] = alloca ptr, align 4
; CHECK-NEXT:    [[VEC_A_GEP_0:%.*]] = getelementptr inbounds <2 x ptr>, ptr [[VEC_A]], i32 0
; CHECK-NEXT:    store <2 x ptr> [[A_0]], ptr [[VEC_A_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_A_GEP_1:%.*]] = getelementptr inbounds <2 x ptr>, ptr [[VEC_A]], i32 1
; CHECK-NEXT:    store <2 x ptr> [[A_1]], ptr [[VEC_A_GEP_1]], align 16
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
;    ...skip...
; CHECK:       return:
; CHECK-NEXT:    [[VEC_RET:%.*]] = load <4 x i32>, ptr [[VEC_RETVAL]], align 16
; CHECK-NEXT:    ret <4 x i32> [[VEC_RET]]
;
; CHECK-LABEL: define x86_regcallcc <4 x i32> @_ZGVxM4va16ua32l4a64_foo
; CHECK-SAME: (<2 x ptr> align 16 [[A_0:%.*]], <2 x ptr> align 16 [[A_1:%.*]], ptr align 32 [[B:%.*]], ptr align 64 [[C:%.*]], <4 x i32> [[MASK:%.*]]) #[[ATTR0]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ALLOCA_C:%.*]] = alloca ptr, align 8
; CHECK-NEXT:    store ptr [[C]], ptr [[ALLOCA_C]], align 8
; CHECK-NEXT:    [[ALLOCA_B:%.*]] = alloca ptr, align 8
; CHECK-NEXT:    store ptr [[B]], ptr [[ALLOCA_B]], align 8
; CHECK-NEXT:    [[VEC_A:%.*]] = alloca <4 x ptr>, align 32
; CHECK-NEXT:    [[VEC_MASK:%.*]] = alloca <4 x i32>, align 16
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <4 x i32>, align 16
; CHECK-NEXT:    [[A_ADDR:%.*]] = alloca ptr, align 4
; CHECK-NEXT:    [[B_ADDR:%.*]] = alloca ptr, align 4
; CHECK-NEXT:    [[C_ADDR:%.*]] = alloca ptr, align 4
; CHECK-NEXT:    [[VEC_A_GEP_0:%.*]] = getelementptr inbounds <2 x ptr>, ptr [[VEC_A]], i32 0
; CHECK-NEXT:    store <2 x ptr> [[A_0]], ptr [[VEC_A_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_A_GEP_1:%.*]] = getelementptr inbounds <2 x ptr>, ptr [[VEC_A]], i32 1
; CHECK-NEXT:    store <2 x ptr> [[A_1]], ptr [[VEC_A_GEP_1]], align 16
; CHECK-NEXT:    store <4 x i32> [[MASK]], ptr [[VEC_MASK]], align 16
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
;    ...skip...
; CHECK:       return:
; CHECK-NEXT:    [[VEC_RET:%.*]] = load <4 x i32>, ptr [[VEC_RETVAL]], align 16
; CHECK-NEXT:    ret <4 x i32> [[VEC_RET]]
;
entry:
  %a.addr = alloca ptr, align 4
  %b.addr = alloca ptr, align 4
  %c.addr = alloca ptr, align 4
  store ptr %a, ptr %a.addr, align 4
  store ptr %b, ptr %b.addr, align 4
  store ptr %c, ptr %c.addr, align 4
  %0 = load ptr, ptr %a.addr, align 4
  %1 = load ptr, ptr %b.addr, align 4
  %2 = load ptr, ptr %c.addr, align 4
  %3 = load i32, ptr %0, align 4
  %4 = load i32, ptr %1, align 4
  %5 = load i32, ptr %2, align 4
  %add1 = add nsw i32 %3, %4
  %add2 = add nsw i32 %add1, %5
  ret i32 %add2
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVxN4va16ua32l4a64_foo,_ZGVxM4va16ua32l4a64_foo" }
