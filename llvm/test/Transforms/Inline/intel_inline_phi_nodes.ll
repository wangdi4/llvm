; TODO: This test case is for testing the issue documented in CMPLRLLVM-7142.
; The fix will be proposed and committed in llorg too. For now, this file will
; use the Intel convention until the changes are committed into llorg. We might
; need to change this filename once the fix is added in the community branch.

; This test case makes sure that the PHINodes are generated correctly after
; inlining and pruning basic blocks. It was created from the following
; C++ example:

; #include <iostream>
; #include <stdlib.h>
; #include <string.h>
;
; inline uint char_val(char X) {
;   return (uint)(X >= '0' && X <= '9'
;                     ? X - '0'
;                    : X >= 'A' && X <= 'Z' ? X - 'A' + 10 : X - 'a' + 10);
; }
;
; char*  make_hex_str(const char *str, size_t str_length) {
;   size_t max_length = (str_length + 1) / 2;
;   char *ret = NULL;
;   char *ptr = (char *)malloc(max_length + 1);
;   if (!ptr) return ret;
;   ret = ptr;
;   char *end = ptr + max_length;
;   if (max_length * 2 != str_length)
;     *ptr++ = char_val(*str++);
;   while (ptr != end) {
;     *ptr++ = (char)(char_val(str[0]) * 16 + char_val(str[1]));
;     str += 2;
;   }
;   *ptr = 0;
;   return ret;
; }
;
; const char* run_test(const char *str, size_t str_length) {
;   return make_hex_str(str, 0);
; }
;
; int main(){
;   char *str;
;   std::cin >> str;
;   size_t size = strlen(str);
;   const char *out = run_test((const char *)str, size);
;   std::cout << out;
; }

; The function char_val will be inlined into make_hex_str. This process
; will change the IR that represents the while loop into something similar:

; if.end:
;   br i1 true, label %while.end.i, label %while.body.i
;
; while.body.i:
;   %ptr.131.i = phi i8* [ %call.i, %while.body.lr.ph.i ],
;                        [ %incdec.ptr14.i, %_Z8char_valc.exit.i ]
;   %str.addr.130.i = phi i8* [ getelementptr inbounds ([1 x i8], [1 x i8]*
;                             @.str.7, i64 0, i64 0), %while.body.lr.ph.i ],
;                             [ %add.ptr15.i, %_Z8char_valc.exit14.i ]
;   %59 = load i8, i8* %str.addr.130.i, align 1, !tbaa !222
;   %cmp.i16.i = icmp sgt i8 %59, 47
;   br i1 %cmp.i16.i, label %land.lhs.true.i18.i, label %cond.false12.i26.i
;
;   ...
;
; _Z8char_valc.exit.i:
;   %cond17.i = phi i8 [ %sub.i, %cond.true.i ], [ %3, %cond.false.i ]
;   %incdec.ptr4 = getelementptr inbounds i8, i8* %ptr131.i, i64 1
;   store i8* %incdec.ptr4, i8** %ptr, align 8
;   store i8 %cond17.i, i8* %call, align 1
;   br label %if.end5

; The block pruning process catches that %while.body.i should be removed
; since it isn't going to enter into the loop. Removing the incoming value
; from the PHINode %ptr.131.i will propagate the value %incdec.ptr14.i.
; Since there is a cycle between %incdec.ptr4 and %ptr.131.i, this
; transformation can break the SSA form. In other words, we don't want
; to transform this instruction into the following statement:

;   %incdec.ptr4 = getelementptr inbounds i8, i8* %incdec.ptr4 , i64 1

; This test case makes sure that the SSA form is preserved after pruning the
; basic blocks during inlining.

; Run the test in the old pass manager
; RUN: opt < %s -sroa -loop-rotate -inline -S 2>&1 | FileCheck %s

; Run the test in the new pass manager
; RUN: opt < %s -passes='function(sroa),function(loop(rotate)),cgscc(inline)' -S 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%"class.std::basic_istream" = type { i32 (...)**, i64, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_ostream"*, i8, i8, %"class.std::basic_streambuf"*, %"class.std::ctype"*, %"class.std::num_put"*, %"class.std::num_get"* }
%"class.std::ios_base" = type { i32 (...)**, i64, i64, i32, i32, i32, %"struct.std::ios_base::_Callback_list"*, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, %"struct.std::ios_base::_Words"*, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { %"struct.std::ios_base::_Callback_list"*, void (i32, %"class.std::ios_base"*, i32)*, i32, i32 }
%"struct.std::ios_base::_Words" = type { i8*, i64 }
%"class.std::locale" = type { %"class.std::locale::_Impl"* }
%"class.std::locale::_Impl" = type { i32, %"class.std::locale::facet"**, i64, %"class.std::locale::facet"**, i8** }
%"class.std::locale::facet" = type <{ i32 (...)**, i32, [4 x i8] }>
%"class.std::basic_ostream" = type { i32 (...)**, %"class.std::basic_ios" }
%"class.std::basic_streambuf" = type { i32 (...)**, i8*, i8*, i8*, i8*, i8*, i8*, %"class.std::locale" }
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], %struct.__locale_struct*, i8, [7 x i8], i32*, i32*, i16*, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class.std::locale::facet.base" = type <{ i32 (...)**, i32 }>
%struct.__locale_struct = type { [13 x %struct.__locale_data*], i16*, i32*, i32*, [13 x i8*] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet.base", [4 x i8] }
%"class.std::num_get" = type { %"class.std::locale::facet.base", [4 x i8] }

$_Z8char_valc = comdat any

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@_ZSt3cin = external dso_local global %"class.std::basic_istream", align 8
@_ZSt4cout = external dso_local global %"class.std::basic_ostream", align 8
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_simple.cpp, i8* null }]

define internal void @__cxx_global_var_init() #0 section ".text.startup" {
entry:
  call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* @_ZSt8__ioinit)
  %0 = call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZSt8__ioinit, i32 0, i32 0), i8* @__dso_handle) #2
  ret void
}

declare dso_local void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) unnamed_addr #1

declare dso_local void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(void (i8*)*, i8*, i8*) #2

define dso_local i8* @_Z12make_hex_strPKcm(i8* %str, i64 %str_length) #3 {
entry:
  %retval = alloca i8*, align 8
  %str.addr = alloca i8*, align 8
  %str_length.addr = alloca i64, align 8
  %max_length = alloca i64, align 8
  %ret = alloca i8*, align 8
  %ptr = alloca i8*, align 8
  %end = alloca i8*, align 8
  store i8* %str, i8** %str.addr, align 8
  store i64 %str_length, i64* %str_length.addr, align 8
  %0 = load i64, i64* %str_length.addr, align 8
  %add = add i64 %0, 1
  %div = udiv i64 %add, 2
  store i64 %div, i64* %max_length, align 8
  store i8* null, i8** %ret, align 8
  %1 = load i64, i64* %max_length, align 8
  %add1 = add i64 %1, 1
  %call = call noalias i8* @malloc(i64 %add1) #2
  store i8* %call, i8** %ptr, align 8
  %2 = load i8*, i8** %ptr, align 8
  %tobool = icmp ne i8* %2, null
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %3 = load i8*, i8** %ret, align 8
  store i8* %3, i8** %retval, align 8
  br label %return

if.end:                                           ; preds = %entry
  %4 = load i8*, i8** %ptr, align 8
  store i8* %4, i8** %ret, align 8
  %5 = load i8*, i8** %ptr, align 8
  %6 = load i64, i64* %max_length, align 8
  %add.ptr = getelementptr inbounds i8, i8* %5, i64 %6
  store i8* %add.ptr, i8** %end, align 8
  %7 = load i64, i64* %max_length, align 8
  %mul = mul i64 %7, 2
  %8 = load i64, i64* %str_length.addr, align 8
  %cmp = icmp ne i64 %mul, %8
  br i1 %cmp, label %if.then2, label %if.end5

if.then2:                                         ; preds = %if.end
  %9 = load i8*, i8** %str.addr, align 8
  %incdec.ptr = getelementptr inbounds i8, i8* %9, i32 1
  store i8* %incdec.ptr, i8** %str.addr, align 8
  %10 = load i8, i8* %9, align 1
  %call3 = call i32 @_Z8char_valc(i8 signext %10)
  %conv = trunc i32 %call3 to i8
  %11 = load i8*, i8** %ptr, align 8
  %incdec.ptr4 = getelementptr inbounds i8, i8* %11, i32 1
  store i8* %incdec.ptr4, i8** %ptr, align 8
  store i8 %conv, i8* %11, align 1
  br label %if.end5

if.end5:                                          ; preds = %if.then2, %if.end
  br label %while.cond

while.cond:                                       ; preds = %while.body, %if.end5
  %12 = load i8*, i8** %ptr, align 8
  %13 = load i8*, i8** %end, align 8
  %cmp6 = icmp ne i8* %12, %13
  br i1 %cmp6, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %14 = load i8*, i8** %str.addr, align 8
  %arrayidx = getelementptr inbounds i8, i8* %14, i64 0
  %15 = load i8, i8* %arrayidx, align 1
  %call7 = call i32 @_Z8char_valc(i8 signext %15)
  %mul8 = mul i32 %call7, 16
  %16 = load i8*, i8** %str.addr, align 8
  %arrayidx9 = getelementptr inbounds i8, i8* %16, i64 1
  %17 = load i8, i8* %arrayidx9, align 1
  %call10 = call i32 @_Z8char_valc(i8 signext %17)
  %add11 = add i32 %mul8, %call10
  %conv12 = trunc i32 %add11 to i8
  %18 = load i8*, i8** %ptr, align 8
  %incdec.ptr13 = getelementptr inbounds i8, i8* %18, i32 1
  store i8* %incdec.ptr13, i8** %ptr, align 8
  store i8 %conv12, i8* %18, align 1
  %19 = load i8*, i8** %str.addr, align 8
  %add.ptr14 = getelementptr inbounds i8, i8* %19, i64 2
  store i8* %add.ptr14, i8** %str.addr, align 8
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %20 = load i8*, i8** %ptr, align 8
  store i8 0, i8* %20, align 1
  %21 = load i8*, i8** %ret, align 8
  store i8* %21, i8** %retval, align 8
  br label %return

return:                                           ; preds = %while.end, %if.then
  %22 = load i8*, i8** %retval, align 8
  ret i8* %22
}

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #4

define linkonce_odr dso_local i32 @_Z8char_valc(i8 signext %X) #5 comdat {
entry:
  %X.addr = alloca i8, align 1
  store i8 %X, i8* %X.addr, align 1
  %0 = load i8, i8* %X.addr, align 1
  %conv = sext i8 %0 to i32
  %cmp = icmp sge i32 %conv, 48
  br i1 %cmp, label %land.lhs.true, label %cond.false

land.lhs.true:                                    ; preds = %entry
  %1 = load i8, i8* %X.addr, align 1
  %conv1 = sext i8 %1 to i32
  %cmp2 = icmp sle i32 %conv1, 57
  br i1 %cmp2, label %cond.true, label %cond.false

cond.true:                                        ; preds = %land.lhs.true
  %2 = load i8, i8* %X.addr, align 1
  %conv3 = sext i8 %2 to i32
  %sub = sub nsw i32 %conv3, 48
  br label %cond.end16

cond.false:                                       ; preds = %land.lhs.true, %entry
  %3 = load i8, i8* %X.addr, align 1
  %conv4 = sext i8 %3 to i32
  %cmp5 = icmp sge i32 %conv4, 65
  br i1 %cmp5, label %land.lhs.true6, label %cond.false12

land.lhs.true6:                                   ; preds = %cond.false
  %4 = load i8, i8* %X.addr, align 1
  %conv7 = sext i8 %4 to i32
  %cmp8 = icmp sle i32 %conv7, 90
  br i1 %cmp8, label %cond.true9, label %cond.false12

cond.true9:                                       ; preds = %land.lhs.true6
  %5 = load i8, i8* %X.addr, align 1
  %conv10 = sext i8 %5 to i32
  %sub11 = sub nsw i32 %conv10, 65
  %add = add nsw i32 %sub11, 10
  br label %cond.end

cond.false12:                                     ; preds = %land.lhs.true6, %cond.false
  %6 = load i8, i8* %X.addr, align 1
  %conv13 = sext i8 %6 to i32
  %sub14 = sub nsw i32 %conv13, 97
  %add15 = add nsw i32 %sub14, 10
  br label %cond.end

cond.end:                                         ; preds = %cond.false12, %cond.true9
  %cond = phi i32 [ %add, %cond.true9 ], [ %add15, %cond.false12 ]
  br label %cond.end16

cond.end16:                                       ; preds = %cond.end, %cond.true
  %cond17 = phi i32 [ %sub, %cond.true ], [ %cond, %cond.end ]
  ret i32 %cond17
}

define dso_local i8* @_Z8run_testPKcm(i8* %str, i64 %str_length) #3 {
entry:
  %str.addr = alloca i8*, align 8
  %str_length.addr = alloca i64, align 8
  store i8* %str, i8** %str.addr, align 8
  store i64 %str_length, i64* %str_length.addr, align 8
  %0 = load i8*, i8** %str.addr, align 8
  %call = call i8* @_Z12make_hex_strPKcm(i8* %0, i64 0)
  ret i8* %call
}

define dso_local i32 @main() #6 {
entry:
  %str = alloca i8*, align 8
  %size = alloca i64, align 8
  %out = alloca i8*, align 8
  %0 = load i8*, i8** %str, align 8
  %call = call dereferenceable(280) %"class.std::basic_istream"* @_ZStrsIcSt11char_traitsIcEERSt13basic_istreamIT_T0_ES6_PS3_(%"class.std::basic_istream"* dereferenceable(280) @_ZSt3cin, i8* %0)
  %1 = load i8*, i8** %str, align 8
  %call1 = call i64 @strlen(i8* %1) #8
  store i64 %call1, i64* %size, align 8
  %2 = load i8*, i8** %str, align 8
  %3 = load i64, i64* %size, align 8
  %call2 = call i8* @_Z8run_testPKcm(i8* %2, i64 %3)
  store i8* %call2, i8** %out, align 8
  %4 = load i8*, i8** %out, align 8
  %call3 = call dereferenceable(272) %"class.std::basic_ostream"* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream"* dereferenceable(272) @_ZSt4cout, i8* %4)
  ret i32 0
}

declare dso_local dereferenceable(280) %"class.std::basic_istream"* @_ZStrsIcSt11char_traitsIcEERSt13basic_istreamIT_T0_ES6_PS3_(%"class.std::basic_istream"* dereferenceable(280), i8*) #1

; Function Attrs: nounwind readonly
declare dso_local i64 @strlen(i8*) #7

declare dso_local dereferenceable(272) %"class.std::basic_ostream"* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream"* dereferenceable(272), i8*) #1

define internal void @_GLOBAL__sub_I_simple.cpp() #0 section ".text.startup" {
entry:
  call void @__cxx_global_var_init()
  ret void
}


attributes #0 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { nounwind readonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e5f4f662867240aabc27bc9491b73d220049214d) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 70bfb0191e06c1d8eac2e851b03aef0be5d9d65e)"}

; The inlining will be as follows:

;    @_Z8char_valc -> @_Z12make_hex_strPKcm
;    (new) @_Z12make_hex_strPKcm -> @_Z8run_testPKcm

; The new version of @_Z12make_hex_strPKcm will be inlined into @_Z8run_testPKcm.
; The constant propagation and basic block pruning will be invoked during this
; process. Since the string size that is being passed into @_Z8run_testPKcm is
; 0, the constant propagation will try to fold this information and prune
; the basic blocks. We are going to check that the constant folding doesn't
; simplify the PHINodes in @_Z8run_testPKcm while @_Z12make_hex_strPKcm is
; being inlined.

; Check that the PHINodes were simplified with load and store instructions
; CHECK: while.body.i:                                     ; preds = %_Z8char_valc.{{exit[0-9]+}}.i
; CHECK: [[TMP0:%.*]] = alloca i8*
; CHECK-NEXT: store i8* %add.{{ptr[0-9]+}}.i, i8** [[TMP0:%.*]]
; CHECK-NEXT: [[TMP1:%.*]] = load i8*, i8** [[TMP0:%.*]]
; CHECK: [[TMP2:%.*]] = alloca i8*
; CHECK-NEXT: store i8* %incdec.{{ptr[0-9]+}}.i, i8** [[TMP2:%.*]]
; CHECK-NEXT: [[TMP3:%.*]] = load i8*, i8** [[TMP2:%.*]]

; Check that the broken SSA form isn't created.
; CHECK: _Z8char_valc.{{exit[0-9]+}}.i:                        ; preds = %cond.end.{{i[0-9]+}}.i, %cond.true.{{i[0-9]+}}.i
; CHECK: %incdec.{{ptr[0-9]+}}.i = getelementptr inbounds i8, i8* [[TMP3:%.*]], i32 1
; CHECK: %add.{{ptr[0-9]+}}.i = getelementptr inbounds i8, i8* [[TMP1:%.*]], i64 2
; CHECK-NOT: %incdec.{{ptr[0-9]+}}.i = getelementptr inbounds i8, i8* %incdec.{{ptr[0-9]+}}.i, i32 1
; CHECK-NOT: %add.{{ptr[0-9]+}}.i = getelementptr inbounds i8, i8* %add.{{ptr[0-9]+}}.i, i64 2

