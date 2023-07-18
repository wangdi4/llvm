; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s

; Test for vector variant arguments and return type legalization as per VFABI

; Return: void
; Arguments: ptr (vector)
; ISA class: YMM1
; "vector-variants"="_ZGVyN4v_foo"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr nocapture %p) local_unnamed_addr #0 {
; CHECK-LABEL: define dso_local x86_regcallcc void @_ZGVyN4v_foo
; CHECK-SAME: (<2 x ptr> nocapture [[P_0:%.*]], <2 x ptr> nocapture [[P_1:%.*]]) local_unnamed_addr #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[VEC_P:%.*]] = alloca <4 x ptr>, align 32
; CHECK-NEXT:    [[VEC_P_GEP_0:%.*]] = getelementptr inbounds <2 x ptr>, ptr [[VEC_P]], i32 0
; CHECK-NEXT:    store <2 x ptr> [[P_0]], ptr [[VEC_P_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_P_GEP_1:%.*]] = getelementptr inbounds <2 x ptr>, ptr [[VEC_P]], i32 1
; CHECK-NEXT:    store <2 x ptr> [[P_1]], ptr [[VEC_P_GEP_1]], align 16
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
; CHECK:       simd.begin.region:
; CHECK-NEXT:    [[ENTRY_REGION:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
; CHECK-NEXT:    br label [[SIMD_LOOP_PREHEADER:%.*]]
; CHECK:       simd.loop.preheader:
; CHECK-NEXT:    br label [[SIMD_LOOP_HEADER:%.*]]
; CHECK:       simd.loop.header:
; CHECK-NEXT:    [[INDEX:%.*]] = phi i32 [ 0, [[SIMD_LOOP_PREHEADER]] ], [ [[INDVAR:%.*]], [[SIMD_LOOP_LATCH:%.*]] ]
; CHECK-NEXT:    [[VEC_P_GEP:%.*]] = getelementptr ptr, ptr [[VEC_P]], i32 [[INDEX]]
; CHECK-NEXT:    [[VEC_P_ELEM:%.*]] = load ptr, ptr [[VEC_P_GEP]], align 8
; CHECK-NEXT:    store double 0.000000e+00, ptr [[VEC_P_ELEM]], align 8
; CHECK-NEXT:    br label [[SIMD_LOOP_LATCH]]
; CHECK:       simd.loop.latch:
; CHECK-NEXT:    [[INDVAR]] = add nuw nsw i32 [[INDEX]], 1
; CHECK-NEXT:    [[VL_COND:%.*]] = icmp ult i32 [[INDVAR]], 4
; CHECK-NEXT:    br i1 [[VL_COND]], label [[SIMD_LOOP_HEADER]], label [[SIMD_END_REGION:%.*]], !llvm.loop [[LOOP0:![0-9]+]]
; CHECK:       simd.end.region:
; CHECK-NEXT:    call void @llvm.directive.region.exit(token [[ENTRY_REGION]]) [ "DIR.OMP.END.SIMD"() ]
; CHECK-NEXT:    br label [[RETURN:%.*]]
; CHECK:       return:
; CHECK-NEXT:    ret void
;
entry:
  store double 0.000000e+00, ptr %p, align 8
  ret void
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVyN4v_foo" }
