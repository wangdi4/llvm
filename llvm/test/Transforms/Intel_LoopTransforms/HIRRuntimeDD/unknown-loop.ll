; Check that unknown loops are skipped in RTDD pass
; REQUIRES: asserts
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
%"struct.std::_Rb_tree_node_base" = type { i32, ptr, ptr, ptr }
%"struct.std::_Rb_tree_node" = type { %"struct.std::_Rb_tree_node_base", ptr }

; Function Attrs: nounwind uwtable
define void @_Z3fooRSt3setIPiSt4lessIS0_ESaIS0_EES0_i(ptr readonly dereferenceable(48) %a, ptr nocapture readonly %b, i32 %n) local_unnamed_addr #0 {
entry:
  %_M_left.i.i = getelementptr inbounds %"class.std::set", ptr %a, i64 0, i32 0, i32 0, i32 1, i32 2
  %0 = load ptr, ptr %_M_left.i.i, align 8
  %_M_header.i.i = getelementptr inbounds %"class.std::set", ptr %a, i64 0, i32 0, i32 0, i32 1
  %cmp.i19 = icmp eq ptr %0, %_M_header.i.i
  br i1 %cmp.i19, label %for.cond.cleanup, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %__begin.sroa.0.020 = phi ptr [ %call.i, %for.body ], [ %0, %for.body.preheader ]
  %_M_value_field.i = getelementptr inbounds %"struct.std::_Rb_tree_node_base", ptr %__begin.sroa.0.020, i64 1
  %1 = load ptr, ptr %_M_value_field.i, align 8
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds i32, ptr %1, i64 %indvars.iv
  store i32 %2, ptr %arrayidx6, align 4
  %indvars.iv.next = add nuw i64 %indvars.iv, 1
  %call.i = tail call ptr @_ZSt18_Rb_tree_incrementPKSt18_Rb_tree_node_base(ptr %__begin.sroa.0.020) #2
  %cmp.i = icmp eq ptr %call.i, %_M_header.i.i
  br i1 %cmp.i, label %for.cond.cleanup.loopexit, label %for.body
}

; Function Attrs: nounwind readonly
declare ptr @_ZSt18_Rb_tree_incrementPKSt18_Rb_tree_node_base(ptr) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readonly }


