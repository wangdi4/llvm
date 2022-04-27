; Check that unknown loops are skipped in RTDD pass
; REQUIRES: asserts
; RUN: opt -hir-ssa-deconstruction -disable-output -hir-runtime-dd -hir-cost-model-throttling=0 -debug-only=hir-runtime-dd < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd" -aa-pipeline="basic-aa" -disable-output -hir-cost-model-throttling=0 -debug-only=hir-runtime-dd < %s 2>&1 | FileCheck %s

; Source:
; void foo(set<int*> &a, int *b, int n) {
; int i = 0;
; for (auto *c : a) {
;   c[i] = b[i];
;   i++;
; }
; }

; HIR:
; BEGIN REGION { }
;    + UNKNOWN LOOP i1
;    |   <i1 = 0>
;    |   for.body:
;    |   %__begin.sroa.0.020.out = &((%__begin.sroa.0.020)[0]);
;    |   %4 = (i32**)(%__begin.sroa.0.020.out)[1];
;    |   %5 = (%b)[i1];
;    |   (%4)[i1] = %5;
;    |   %call.i = @_ZSt18_Rb_tree_incrementPKSt18_Rb_tree_node_base(&((%__begin.sroa.0.020.out)[0]));
;    |   %__begin.sroa.0.020 = &((%call.i)[0]);
;    |   if (&((%call.i)[0]) != &((%a)[0].0.0.1))
;    |   {
;    |      <i1 = i1 + 1>
;    |      goto for.body;
;    |   }
;    + END LOOP
; END REGION

; CHECK: LOOPOPT_OPTREPORT: [RTDD]
; CHECK-SAME: Loop is either not a DO loop or cannot be converted to a DO loop

;Module Before HIR; ModuleID = 'unknown-loop-module.ll'
source_filename = "unknown-loop_2.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::set" = type { %"class.std::_Rb_tree" }
%"class.std::_Rb_tree" = type { %"struct.std::_Rb_tree<int *, int *, std::_Identity<int *>, std::less<int *>, std::allocator<int *> >::_Rb_tree_impl" }
%"struct.std::_Rb_tree<int *, int *, std::_Identity<int *>, std::less<int *>, std::allocator<int *> >::_Rb_tree_impl" = type { %"struct.std::less", %"struct.std::_Rb_tree_node_base", i64 }
%"struct.std::less" = type { i8 }
%"struct.std::_Rb_tree_node_base" = type { i32, %"struct.std::_Rb_tree_node_base"*, %"struct.std::_Rb_tree_node_base"*, %"struct.std::_Rb_tree_node_base"* }
%"struct.std::_Rb_tree_node" = type { %"struct.std::_Rb_tree_node_base", i32* }

; Function Attrs: nounwind uwtable
define void @_Z3fooRSt3setIPiSt4lessIS0_ESaIS0_EES0_i(%"class.std::set"* readonly dereferenceable(48) %a, i32* nocapture readonly %b, i32 %n) local_unnamed_addr #0 {
entry:
  %_M_left.i.i = getelementptr inbounds %"class.std::set", %"class.std::set"* %a, i64 0, i32 0, i32 0, i32 1, i32 2
  %0 = bitcast %"struct.std::_Rb_tree_node_base"** %_M_left.i.i to %"struct.std::_Rb_tree_node"**
  %1 = load %"struct.std::_Rb_tree_node"*, %"struct.std::_Rb_tree_node"** %0, align 8
  %2 = getelementptr inbounds %"struct.std::_Rb_tree_node", %"struct.std::_Rb_tree_node"* %1, i64 0, i32 0
  %_M_header.i.i = getelementptr inbounds %"class.std::set", %"class.std::set"* %a, i64 0, i32 0, i32 0, i32 1
  %cmp.i19 = icmp eq %"struct.std::_Rb_tree_node_base"* %2, %_M_header.i.i
  br i1 %cmp.i19, label %for.cond.cleanup, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %__begin.sroa.0.020 = phi %"struct.std::_Rb_tree_node_base"* [ %call.i, %for.body ], [ %2, %for.body.preheader ]
  %_M_value_field.i = getelementptr inbounds %"struct.std::_Rb_tree_node_base", %"struct.std::_Rb_tree_node_base"* %__begin.sroa.0.020, i64 1
  %3 = bitcast %"struct.std::_Rb_tree_node_base"* %_M_value_field.i to i32**
  %4 = load i32*, i32** %3, align 8
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %5 = load i32, i32* %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds i32, i32* %4, i64 %indvars.iv
  store i32 %5, i32* %arrayidx6, align 4
  %indvars.iv.next = add nuw i64 %indvars.iv, 1
  %call.i = tail call %"struct.std::_Rb_tree_node_base"* @_ZSt18_Rb_tree_incrementPKSt18_Rb_tree_node_base(%"struct.std::_Rb_tree_node_base"* %__begin.sroa.0.020) #2
  %cmp.i = icmp eq %"struct.std::_Rb_tree_node_base"* %call.i, %_M_header.i.i
  br i1 %cmp.i, label %for.cond.cleanup.loopexit, label %for.body
}

; Function Attrs: nounwind readonly
declare %"struct.std::_Rb_tree_node_base"* @_ZSt18_Rb_tree_incrementPKSt18_Rb_tree_node_base(%"struct.std::_Rb_tree_node_base"*) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readonly }


