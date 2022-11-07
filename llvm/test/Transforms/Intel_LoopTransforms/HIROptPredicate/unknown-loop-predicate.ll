; RUN: opt -hir-ssa-deconstruction -disable-output -hir-cost-model-throttling=0 -hir-opt-predicate -print-after=hir-opt-predicate < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-cost-model-throttling=0 < %s 2>&1 | FileCheck %s

; Source:
; #include <iostream>
; #include <set>
;
; struct A {
;   bool some_value;
; };
;
; void foo(const std::set<A> &s, int n) {
;   for (auto &a : s) {
;     if (n > 50) {
;       std::cout << a.some_value;
;     }
;   }
; }

; HIR:
; BEGIN REGION { }
;      + UNKNOWN LOOP i1
;      |   <i1 = 0>
;      |   for.body:
;      |   %__begin.sroa.0.017.out = &((%__begin.sroa.0.017)[0]);
;      |   if (%n > 50)
;      |   {
;      |      %3 = (i8*)(%__begin.sroa.0.017.out)[1];
;      |      %tobool = %3 != 0;
;      |      %call.i10 = @_ZNSo9_M_insertIbEERSoT_(&((@_ZSt4cout)[0]),  %tobool);
;      |   }
;      |   %call.i = @_ZSt18_Rb_tree_incrementPKSt18_Rb_tree_node_base(&((%__begin.sroa.0.017.out)[0]));
;      |   %__begin.sroa.0.017 = &((%call.i)[0]);
;      |   if (&((%call.i)[0]) != &((%s)[0].0.0.1))
;      |   {
;      |      <i1 = i1 + 1>
;      |      goto for.body;
;      |   }
;      + END LOOP
; END REGION

; CHECK: Function
; CHECK: if (%n > 50)
; CHECK: UNKNOWN LOOP i1
; CHECK: END LOOP
; CHECK-NOT: UNKNOWN LOOP i1

;Module Before HIR; ModuleID = 'module.c'
source_filename = "1.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%"class.std::basic_ostream" = type { i32 (...)**, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_ostream"*, i8, i8, %"class.std::basic_streambuf"*, %"class.std::ctype"*, %"class.std::num_put"*, %"class.std::num_get"* }
%"class.std::ios_base" = type { i32 (...)**, i64, i64, i32, i32, i32, %"struct.std::ios_base::_Callback_list"*, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, %"struct.std::ios_base::_Words"*, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { %"struct.std::ios_base::_Callback_list"*, void (i32, %"class.std::ios_base"*, i32)*, i32, i32 }
%"struct.std::ios_base::_Words" = type { i8*, i64 }
%"class.std::locale" = type { %"class.std::locale::_Impl"* }
%"class.std::locale::_Impl" = type { i32, %"class.std::locale::facet"**, i64, %"class.std::locale::facet"**, i8** }
%"class.std::locale::facet" = type <{ i32 (...)**, i32, [4 x i8] }>
%"class.std::basic_streambuf" = type { i32 (...)**, i8*, i8*, i8*, i8*, i8*, i8*, %"class.std::locale" }
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], %struct.__locale_struct*, i8, [7 x i8], i32*, i32*, i16*, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class.std::locale::facet.base" = type <{ i32 (...)**, i32 }>
%struct.__locale_struct = type { [13 x %struct.__locale_data*], i16*, i32*, i32*, [13 x i8*] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet.base", [4 x i8] }
%"class.std::num_get" = type { %"class.std::locale::facet.base", [4 x i8] }
%"class.std::set" = type { %"class.std::_Rb_tree" }
%"class.std::_Rb_tree" = type { %"struct.std::_Rb_tree<A, A, std::_Identity<A>, std::less<A>, std::allocator<A> >::_Rb_tree_impl" }
%"struct.std::_Rb_tree<A, A, std::_Identity<A>, std::less<A>, std::allocator<A> >::_Rb_tree_impl" = type { %"struct.std::less", %"struct.std::_Rb_tree_node_base", i64 }
%"struct.std::less" = type { i8 }
%"struct.std::_Rb_tree_node_base" = type { i32, %"struct.std::_Rb_tree_node_base"*, %"struct.std::_Rb_tree_node_base"*, %"struct.std::_Rb_tree_node_base"* }
%"struct.std::_Rb_tree_node" = type <{ %"struct.std::_Rb_tree_node_base", %struct.A, [7 x i8] }>
%struct.A = type { i8 }

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@_ZSt4cout = external global %"class.std::basic_ostream", align 8
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_1.cpp, i8* null }]

declare void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) unnamed_addr #0

; Function Attrs: nounwind
declare void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) unnamed_addr #1

; Function Attrs: nounwind
declare i32 @__cxa_atexit(void (i8*)*, i8*, i8*) local_unnamed_addr #2

; Function Attrs: uwtable
define void @_Z3fooRKSt3setI1ASt4lessIS0_ESaIS0_EEi(%"class.std::set"* readonly dereferenceable(48) %s, i32 %n) local_unnamed_addr #3 personality i32 (...)* @__gxx_personality_v0 {
entry:
  %_M_left.i.i = getelementptr inbounds %"class.std::set", %"class.std::set"* %s, i64 0, i32 0, i32 0, i32 1, i32 2
  %0 = bitcast %"struct.std::_Rb_tree_node_base"** %_M_left.i.i to %"struct.std::_Rb_tree_node"**
  %1 = load %"struct.std::_Rb_tree_node"*, %"struct.std::_Rb_tree_node"** %0, align 8
  %2 = getelementptr inbounds %"struct.std::_Rb_tree_node", %"struct.std::_Rb_tree_node"* %1, i64 0, i32 0
  %_M_header.i.i = getelementptr inbounds %"class.std::set", %"class.std::set"* %s, i64 0, i32 0, i32 0, i32 1
  %cmp.i16 = icmp eq %"struct.std::_Rb_tree_node_base"* %2, %_M_header.i.i
  br i1 %cmp.i16, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %cmp = icmp sgt i32 %n, 50
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %if.end
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %if.end, %for.body.lr.ph
  %__begin.sroa.0.017 = phi %"struct.std::_Rb_tree_node_base"* [ %2, %for.body.lr.ph ], [ %call.i, %if.end ]
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %_M_value_field.i = getelementptr inbounds %"struct.std::_Rb_tree_node_base", %"struct.std::_Rb_tree_node_base"* %__begin.sroa.0.017, i64 1
  %some_value = bitcast %"struct.std::_Rb_tree_node_base"* %_M_value_field.i to i8*
  %3 = load i8, i8* %some_value, align 1
  %tobool = icmp ne i8 %3, 0
  %call.i10 = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo9_M_insertIbEERSoT_(%"class.std::basic_ostream"* nonnull @_ZSt4cout, i1 zeroext %tobool)
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %call.i = tail call %"struct.std::_Rb_tree_node_base"* @_ZSt18_Rb_tree_incrementPKSt18_Rb_tree_node_base(%"struct.std::_Rb_tree_node_base"* %__begin.sroa.0.017) #5
  %cmp.i = icmp eq %"struct.std::_Rb_tree_node_base"* %call.i, %_M_header.i.i
  br i1 %cmp.i, label %for.cond.cleanup.loopexit, label %for.body
}

declare i32 @__gxx_personality_v0(...)

; Function Attrs: nounwind memory(read)
declare %"struct.std::_Rb_tree_node_base"* @_ZSt18_Rb_tree_incrementPKSt18_Rb_tree_node_base(%"struct.std::_Rb_tree_node_base"*) local_unnamed_addr #4

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo9_M_insertIbEERSoT_(%"class.std::basic_ostream"*, i1 zeroext) local_unnamed_addr #0

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_1.cpp() #3 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull @_ZSt8__ioinit)
  %0 = tail call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZSt8__ioinit, i64 0, i32 0), i8* nonnull @__dso_handle) #2
  ret void
}

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind memory(read) willreturn "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind memory(read) }


