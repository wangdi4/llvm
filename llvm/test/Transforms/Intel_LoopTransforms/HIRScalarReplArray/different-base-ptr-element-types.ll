; Test that this lit test does not fail when -opaque-pointers is enabled.
; We skip comparing memrefs (double*)(%patch164.sroa.0.02736)[0].0[1] and (%patch164.sroa.0.02736)[0].0[1]
; if they have different base pointer element types [2 x double] and [4 x %"class.dealii::Point.56"].
; RUN: opt -opaque-pointers -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes="hir-ssa-deconstruction,print<hir>,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
;*** IR Dump Before HIR Scalar Replacement of Array  (hir-scalarrepl-array) ***
;
;<0>          BEGIN REGION { }
;<27>               + DO i1 = 0, 0, 1   <DO_LOOP>
;<28>               |   + DO i2 = 0, 0, 1   <DO_LOOP>
;<4>                |   |   if (undef false undef)
;<4>                |   |   {
;<8>                |   |      %t36 = (double*)(%patch164.sroa.0.02736)[0].0[1];
;<9>                |   |      %t38 = (%patch164.sroa.0.02736)[0];
;<10>               |   |      %t39 = (%patch164.sroa.0.02736)[0].0[1];
;<4>                |   |   }
;<4>                |   |   else
;<4>                |   |   {
;<13>               |   |      %t311 = (null)[0];
;<15>               |   |      %t34 = (%t31)[0];
;<4>                |   |   }
;<28>               |   + END LOOP
;<27>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Scalar Replacement of Array  (hir-scalarrepl-array) ***
;
; CHECK:          BEGIN REGION { }
;
%"struct.dealii::DataOutBase::Patch.55" = type <{ [4 x %"class.dealii::Point.56"], [4 x i32], i32, i32, %"class.dealii::Table", i8, [7 x i8] }>
%"class.dealii::Point.56" = type { %"class.dealii::Tensor.57" }
%"class.dealii::Tensor.57" = type { [2 x double] }
%"class.dealii::Table" = type { %"class.dealii::TableBase.base", [4 x i8] }
%"class.dealii::TableBase.base" = type <{ %"class.dealii::Subscriptor", ptr, i32, %"class.dealii::TableIndices" }>
%"class.dealii::Subscriptor" = type { ptr, i32, %"class.std::map", ptr }
%"class.std::map" = type { %"class.std::_Rb_tree" }
%"class.std::_Rb_tree" = type { %"struct.std::_Rb_tree<const char *, std::pair<const char *const, unsigned int>, std::_Select1st<std::pair<const char *const, unsigned int>>, std::less<const char *>>::_Rb_tree_impl" }
%"struct.std::_Rb_tree<const char *, std::pair<const char *const, unsigned int>, std::_Select1st<std::pair<const char *const, unsigned int>>, std::less<const char *>>::_Rb_tree_impl" = type { %"struct.std::_Rb_tree_key_compare", %"struct.std::_Rb_tree_header" }
%"struct.std::_Rb_tree_key_compare" = type { %"struct.std::less" }
%"struct.std::less" = type { i8 }
%"struct.std::_Rb_tree_header" = type { %"struct.std::_Rb_tree_node_base", i64 }
%"struct.std::_Rb_tree_node_base" = type { i32, ptr, ptr, ptr }
%"class.dealii::TableIndices" = type { %"class.dealii::TableIndicesBase" }
%"class.dealii::TableIndicesBase" = type { [2 x i32] }

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #1

define weak_odr void @_ZN6dealii11DataOutBase12write_povrayILi2ELi2EEEvRKSt6vectorINS0_5PatchIXT_EXT0_EEESaIS4_EERKS2_INSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEESaISE_EERKS2_IN5boost6tuples5tupleIjjSE_NSK_9null_typeESM_SM_SM_SM_SM_SM_EESaISN_EERKNS0_11PovrayFlagsERSo(ptr %t31, ptr %arrayidx.i49.i) personality ptr undef {
for.body173:
  %patch164.sroa.0.02736 = load ptr, ptr null, align 8
  br label %invoke.cont182

invoke.cont182:                                   ; preds = %for.body173
  br label %for.cond187.preheader.lr.ph

for.cond187.preheader.lr.ph:                      ; preds = %invoke.cont182
  %arrayidx.i1542 = getelementptr inbounds %"struct.dealii::DataOutBase::Patch.55", ptr %patch164.sroa.0.02736, i64 0, i32 0, i64 1
  %arrayidx.i4.i.i32.i = getelementptr inbounds %"class.dealii::Tensor.57", ptr %patch164.sroa.0.02736, i64 0, i32 0, i64 1
  br label %for.body190.lr.ph

for.body190.lr.ph:                                ; preds = %for.cond.cleanup189, %for.cond187.preheader.lr.ph
  br label %for.body190

for.cond.cleanup185.loopexit:                     ; preds = %for.cond.cleanup189
  ret void

for.cond.cleanup189:                              ; preds = %for.inc199
  %exitcond2764.not = icmp eq i32 1, 1
  br i1 %exitcond2764.not, label %for.cond.cleanup185.loopexit, label %for.body190.lr.ph

for.body190:                                      ; preds = %for.inc199, %for.body190.lr.ph
  br i1 false, label %if.else.i, label %if.then.i

if.then.i:                                        ; preds = %for.body190
  %t311 = load ptr, ptr null, align 8
  %arrayidx.i49.i2 = getelementptr inbounds float, ptr %t31, i64 undef
  %t34 = load float, ptr %t31, align 4
  br label %for.inc199

if.else.i:                                        ; preds = %for.body190
  %t36 = load double, ptr %arrayidx.i1542, align 8
  %t38 = load double, ptr %patch164.sroa.0.02736, align 8
  %t39 = load double, ptr %arrayidx.i4.i.i32.i, align 8
  br label %for.inc199

for.inc199:                                       ; preds = %if.else.i, %if.then.i
  %exitcond.not = icmp eq i64 0, 0
  br i1 %exitcond.not, label %for.cond.cleanup189, label %for.body190
}

attributes #0 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #1 = { argmemonly nofree nounwind willreturn writeonly }

