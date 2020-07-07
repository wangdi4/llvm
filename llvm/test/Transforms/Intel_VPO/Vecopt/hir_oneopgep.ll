; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -enable-vp-value-codegen-hir -vplan-force-vf=4 -disable-output -tbaa -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck %s
;
; Test to check that we handle one operand GEPs correctly. This can happen for
; HLInsts like tmp = &(MEMREF) where the underlying instruction is a GEP.
;
; Scalar HIR:
;
;     DO i1 = 0, 99, 1   <DO_LOOP>
;       %1 = &((%0)[i1].0.0.0.0.0);
;       %3 = (i64*)(%1)[40];
;       %n.09 = %n.09  +  %3; <Safe Reduction>
;     END LOOP
; Relevant VPlan instructions:
;
;   i32 %vp62384 = phi  [ i32 %n.09, BB2 ],  [ i32 %vp240, BB3 ]
;   i64 %vp16064 = phi  [ i64 0, BB2 ],  [ i64 %vp688, BB3 ]
;   i8* %vp27872 = getelementptr inbounds %"class.std::set"* %0 i64 %vp16064 i32 0 i32 0 i32 0 i32 0 i32 0
;   i8* %vp61520 = getelementptr inbounds i8* %vp27872
;   i8* %vp38320 = getelementptr inbounds i8* %vp61520 i64 40
;   i64* %vp61840 = bitcast i8* %vp38320
;
; CHECK-LABEL: *** IR Dump After VPlan Vectorization Driver HIR ***
; CHECK:              %red.var = 0;
; CHECK-NEXT:         %red.var = insertelement %red.var,  %n.09,  0;
; CHECK-NEXT:         DO i1 = 0, 99, 4   <DO_LOOP> <novectorize>
; CHECK-NEXT:            %nsbgepcopy = &((<4 x i8*>)(%0)[i1 + <i64 0, i64 1, i64 2, i64 3>].0.0.0.0.0);
; CHECK-NEXT:            %.vec = (<4 x i64>*)(%nsbgepcopy)[40];
; CHECK-NEXT:            %red.var = %red.var  +  %.vec;
; CHECK-NEXT:         END LOOP
; CHECK-NEXT:         %n.09 = @llvm.experimental.vector.reduce.add.v4i32(%red.var);
;
%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<std::set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int>>, std::allocator<std::set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int>>>>::_Vector_impl" }
%"struct.std::_Vector_base<std::set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int>>, std::allocator<std::set<unsigned int, std::less<unsigned int>, std::allocator<unsigned int>>>>::_Vector_impl" = type { %"class.std::set"*, %"class.std::set"*, %"class.std::set"* }
%"class.std::set" = type { %"class.std::_Rb_tree" }
%"class.std::_Rb_tree" = type { %"struct.std::_Rb_tree<unsigned int, unsigned int, std::_Identity<unsigned int>, std::less<unsigned int>, std::allocator<unsigned int>>::_Rb_tree_impl" }
%"struct.std::_Rb_tree<unsigned int, unsigned int, std::_Identity<unsigned int>, std::less<unsigned int>, std::allocator<unsigned int>>::_Rb_tree_impl" = type { %"struct.std::_Rb_tree_key_compare", %"struct.std::_Rb_tree_header" }
%"struct.std::_Rb_tree_key_compare" = type { %"struct.std::less" }
%"struct.std::less" = type { i8 }
%"struct.std::_Rb_tree_header" = type { %"struct.std::_Rb_tree_node_base", i64 }
%"struct.std::_Rb_tree_node_base" = type { i32, %"struct.std::_Rb_tree_node_base"*, %"struct.std::_Rb_tree_node_base"*, %"struct.std::_Rb_tree_node_base"* }
%"struct.std::_Rb_tree_node" = type <{ %"struct.std::_Rb_tree_node_base", %"struct.__gnu_cxx::__aligned_membuf", [4 x i8] }>
%"struct.__gnu_cxx::__aligned_membuf" = type { [4 x i8] }

@column_indices = dso_local global %"class.std::vector" zeroinitializer, align 8
define i32 @_Z18n_nonzero_elementsv() {
entry:
  %0 = load %"class.std::set"*, %"class.std::set"** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @column_indices, i64 0, i32 0, i32 0, i32 0), align 8
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %conv3.lcssa = phi i32 [ %conv3, %for.body ]
  ret i32 %conv3.lcssa

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %n.09 = phi i32 [ 0, %entry ], [ %conv3, %for.body ]
  %1 = getelementptr inbounds %"class.std::set", %"class.std::set"* %0, i64 %indvars.iv, i32 0, i32 0, i32 0, i32 0, i32 0
  %_M_node_count.i.i = getelementptr inbounds i8, i8* %1, i64 40
  %2 = bitcast i8* %_M_node_count.i.i to i64*
  %3 = load i64, i64* %2, align 8
  %4 = trunc i64 %3 to i32
  %conv3 = add i32 %n.09, %4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}
