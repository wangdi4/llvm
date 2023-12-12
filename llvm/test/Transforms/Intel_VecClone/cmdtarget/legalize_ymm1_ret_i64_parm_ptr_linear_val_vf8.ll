; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s

; Test for vector variant arguments and return type legalization as per VFABI
; Return: i64
; Arguments: ptr
; ISA class: YMM1
; "vector-variants"="_ZGVyN8L2__Z3fooRl"
; Origin: linear_val_optimized.ll


; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define dso_local noundef i64 @_Z3fooRl(ptr nocapture noundef nonnull readonly align 8 dereferenceable(8) %x) local_unnamed_addr #0 {
; CHECK-LABEL: define dso_local x86_regcallcc noundef { <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64> } @_ZGVyN8L2__Z3fooRl
; CHECK-SAME: (<2 x ptr> nocapture noundef nonnull readonly align 8 dereferenceable(8) [[X_0:%.*]], <2 x ptr> nocapture noundef nonnull readonly align 8 dereferenceable(8) [[X_1:%.*]], <2 x ptr> nocapture noundef nonnull readonly align 8 dereferenceable(8) [[X_2:%.*]], <2 x ptr> nocapture noundef nonnull readonly align 8 dereferenceable(8) [[X_3:%.*]]) local_unnamed_addr #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[VEC_X:%.*]] = alloca <8 x ptr>, align 64
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <8 x i64>, align 64
; CHECK-NEXT:    [[VEC_X_GEP_0:%.*]] = getelementptr inbounds <2 x ptr>, ptr [[VEC_X]], i32 0
; CHECK-NEXT:    store <2 x ptr> [[X_0]], ptr [[VEC_X_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_X_GEP_1:%.*]] = getelementptr inbounds <2 x ptr>, ptr [[VEC_X]], i32 1
; CHECK-NEXT:    store <2 x ptr> [[X_1]], ptr [[VEC_X_GEP_1]], align 16
; CHECK-NEXT:    [[VEC_X_GEP_2:%.*]] = getelementptr inbounds <2 x ptr>, ptr [[VEC_X]], i32 2
; CHECK-NEXT:    store <2 x ptr> [[X_2]], ptr [[VEC_X_GEP_2]], align 16
; CHECK-NEXT:    [[VEC_X_GEP_3:%.*]] = getelementptr inbounds <2 x ptr>, ptr [[VEC_X]], i32 3
; CHECK-NEXT:    store <2 x ptr> [[X_3]], ptr [[VEC_X_GEP_3]], align 16
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
;    ...skip...
; CHECK:       return:
; CHECK-NEXT:    [[VEC_RETVAL_GEP_0:%.*]] = getelementptr inbounds <2 x i64>, ptr [[VEC_RETVAL]], i32 0
; CHECK-NEXT:    [[VEC_RET_0:%.*]] = load <2 x i64>, ptr [[VEC_RETVAL_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_0:%.*]] = insertvalue { <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64> } poison, <2 x i64> [[VEC_RET_0]], 0
; CHECK-NEXT:    [[VEC_RETVAL_GEP_1:%.*]] = getelementptr inbounds <2 x i64>, ptr [[VEC_RETVAL]], i32 1
; CHECK-NEXT:    [[VEC_RET_1:%.*]] = load <2 x i64>, ptr [[VEC_RETVAL_GEP_1]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_1:%.*]] = insertvalue { <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64> } [[VEC_RETVAL_INS_0]], <2 x i64> [[VEC_RET_1]], 1
; CHECK-NEXT:    [[VEC_RETVAL_GEP_2:%.*]] = getelementptr inbounds <2 x i64>, ptr [[VEC_RETVAL]], i32 2
; CHECK-NEXT:    [[VEC_RET_2:%.*]] = load <2 x i64>, ptr [[VEC_RETVAL_GEP_2]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_2:%.*]] = insertvalue { <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64> } [[VEC_RETVAL_INS_1]], <2 x i64> [[VEC_RET_2]], 2
; CHECK-NEXT:    [[VEC_RETVAL_GEP_3:%.*]] = getelementptr inbounds <2 x i64>, ptr [[VEC_RETVAL]], i32 3
; CHECK-NEXT:    [[VEC_RET_3:%.*]] = load <2 x i64>, ptr [[VEC_RETVAL_GEP_3]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_3:%.*]] = insertvalue { <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64> } [[VEC_RETVAL_INS_2]], <2 x i64> [[VEC_RET_3]], 3
; CHECK-NEXT:    ret { <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64> } [[VEC_RETVAL_INS_3]]
;
entry:
  %0 = load i64, ptr %x, align 8
  %add = add nsw i64 %0, 1
  ret i64 %add
}

attributes #0 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(argmem: read) uwtable "vector-variants"="_ZGVyN8L2__Z3fooRl" }

