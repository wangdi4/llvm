; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL

; Test src:
;
; #include <string>
;
; static const int N = 100;
;
; typedef struct my_struct{int a; int b; std::string c; my_struct(): a(0), b(0), c("0") {}; } TYPE;
; bool operator <(const TYPE& t1, const TYPE& t2) { return (t1.a < t2.a) || (t1.b < t2.b) || (t1.c < t2.c); }
;
; void my_init(TYPE& t) { t.a = 1; t.b = 1; t.c = "1"; }
; void my_add(TYPE& lhs, TYPE const &rhs) { lhs.a += rhs.a; lhs.b += rhs.b; lhs.c = std::to_string(static_cast<long long>(std::stoi(lhs.c) + std::stoi(rhs.c)));}
;
; static TYPE y[N];
; static TYPE x[N];
;
; #pragma omp declare reduction (my_reduction_add : TYPE : my_add(omp_out, omp_in)) initializer(my_init(omp_priv))
;
; // Reduction on:         Reference to array
; // Reduction type:       UDR on NONPOD (with initializer)
; // Array layout:         1-D
; // Access in body:       EXPR_SUBSCRIPT
; void cq415166_1d_d_ref(TYPE (&yref)[N]) {
;
; #pragma omp parallel for reduction(my_reduction_add:yref) num_threads(N)
;     for (int i = 0; i < N; i++) {
;         for (int j = 5; j <= 8; j++) {
;             my_add(yref[j], x[i]);
;         }
;     }
; }
;
; int main() {
;
;     for (int i = 0; i < N; i++) {
;         x[i].a = i; x[i].b = i * i; x[i].c = std::to_string(static_cast<long long>(i * i * i));
;         y[i].a = 0; y[i].b = 0; y[i].c = "0";
;     }
;
;     cq415166_1d_d_ref(y);
;
;     return 0;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { i32, i32, %"class.std::__cxx11::basic_string" }
%"class.std::__cxx11::basic_string" = type { %"struct.std::__cxx11::basic_string<char>::_Alloc_hider", i64, %union.anon }
%"struct.std::__cxx11::basic_string<char>::_Alloc_hider" = type { ptr }
%union.anon = type { i64, [8 x i8] }
%"class.std::allocator" = type { i8 }
%struct._Save_errno = type { i32 }
%"struct.std::integral_constant" = type { i8 }
%"struct.std::is_same" = type { i8 }

$_ZStltIcSt11char_traitsIcESaIcEEbRKNSt7__cxx1112basic_stringIT_T0_T1_EESA_ = comdat any

$__clang_call_terminate = comdat any

$_ZNSt7__cxx119to_stringEx = comdat any

$_ZNSt7__cxx114stoiERKNS_12basic_stringIcSt11char_traitsIcESaIcEEEPmi = comdat any

$_ZNSt8__detail14__to_chars_lenIyEEjT_i = comdat any

$_ZNSt8__detail18__to_chars_10_implIyEEvPcjT_ = comdat any

$_ZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_ = comdat any

$_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoC2Ev = comdat any

$_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN10_Range_chk6_S_chkElSt17integral_constantIbLb1EE = comdat any

$_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoD2Ev = comdat any

$_ZN9my_structC2Ev = comdat any

$_ZN9my_structD2Ev = comdat any

$_ZZNSt8__detail18__to_chars_10_implIyEEvPcjT_E8__digits = comdat any

@.str = private unnamed_addr constant [2 x i8] c"1\00", align 1
@_ZZNSt8__detail18__to_chars_10_implIyEEvPcjT_E8__digits = linkonce_odr dso_local constant [201 x i8] c"00010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899\00", comdat, align 16
@_ZZNSt8__detail18__to_chars_10_implIyEEvPcjT_E8__digits.const = private unnamed_addr constant [201 x i8] c"00010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899\00", align 16
@.str.1 = private unnamed_addr constant [5 x i8] c"stoi\00", align 1
@_ZL1y = internal global [100 x %struct.my_struct] zeroinitializer, align 16
@__dso_handle = external hidden global i8
@.str.2 = private unnamed_addr constant [2 x i8] c"0\00", align 1
@_ZL1x = internal global [100 x %struct.my_struct] zeroinitializer, align 16
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_omp45_array_reduction_1d_udr_with_nonpods_ref.cpp, ptr null }]

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef zeroext i1 @_ZltRK9my_structS1_(ptr noundef nonnull align 8 dereferenceable(40) %t1, ptr noundef nonnull align 8 dereferenceable(40) %t2) #0 {
entry:
  %t1.addr = alloca ptr, align 8
  %t2.addr = alloca ptr, align 8
  store ptr %t1, ptr %t1.addr, align 8
  store ptr %t2, ptr %t2.addr, align 8
  %0 = load ptr, ptr %t1.addr, align 8
  %a = getelementptr inbounds %struct.my_struct, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %a, align 8
  %2 = load ptr, ptr %t2.addr, align 8
  %a1 = getelementptr inbounds %struct.my_struct, ptr %2, i32 0, i32 0
  %3 = load i32, ptr %a1, align 8
  %cmp = icmp slt i32 %1, %3
  br i1 %cmp, label %lor.end, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %entry
  %4 = load ptr, ptr %t1.addr, align 8
  %b = getelementptr inbounds %struct.my_struct, ptr %4, i32 0, i32 1
  %5 = load i32, ptr %b, align 4
  %6 = load ptr, ptr %t2.addr, align 8
  %b2 = getelementptr inbounds %struct.my_struct, ptr %6, i32 0, i32 1
  %7 = load i32, ptr %b2, align 4
  %cmp3 = icmp slt i32 %5, %7
  br i1 %cmp3, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %lor.lhs.false
  %8 = load ptr, ptr %t1.addr, align 8
  %c = getelementptr inbounds %struct.my_struct, ptr %8, i32 0, i32 2
  %9 = load ptr, ptr %t2.addr, align 8
  %c4 = getelementptr inbounds %struct.my_struct, ptr %9, i32 0, i32 2
  %call = call noundef zeroext i1 @_ZStltIcSt11char_traitsIcESaIcEEbRKNSt7__cxx1112basic_stringIT_T0_T1_EESA_(ptr noundef nonnull align 8 dereferenceable(32) %c, ptr noundef nonnull align 8 dereferenceable(32) %c4) #10
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %lor.lhs.false, %entry
  %10 = phi i1 [ true, %lor.lhs.false ], [ true, %entry ], [ %call, %lor.rhs ]
  ret i1 %10
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZStltIcSt11char_traitsIcESaIcEEbRKNSt7__cxx1112basic_stringIT_T0_T1_EESA_(ptr noundef nonnull align 8 dereferenceable(32) %__lhs, ptr noundef nonnull align 8 dereferenceable(32) %__rhs) #0 comdat personality ptr @__gxx_personality_v0 {
entry:
  %__lhs.addr = alloca ptr, align 8
  %__rhs.addr = alloca ptr, align 8
  store ptr %__lhs, ptr %__lhs.addr, align 8
  store ptr %__rhs, ptr %__rhs.addr, align 8
  %0 = load ptr, ptr %__lhs.addr, align 8
  %1 = load ptr, ptr %__rhs.addr, align 8
  %call = invoke noundef i32 @_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7compareERKS4_(ptr noundef nonnull align 8 dereferenceable(32) %0, ptr noundef nonnull align 8 dereferenceable(32) %1)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %entry
  %cmp = icmp slt i32 %call, 0
  ret i1 %cmp

terminate.lpad:                                   ; preds = %entry
  %2 = landingpad { ptr, i32 }
          catch ptr null
  %3 = extractvalue { ptr, i32 } %2, 0
  call void @__clang_call_terminate(ptr %3) #13
  unreachable
}

declare dso_local noundef i32 @_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7compareERKS4_(ptr noundef nonnull align 8 dereferenceable(32), ptr noundef nonnull align 8 dereferenceable(32)) #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(ptr %0) #2 comdat {
  %2 = call ptr @__cxa_begin_catch(ptr %0) #10
  call void @_ZSt9terminatev() #13
  unreachable
}

declare dso_local ptr @__cxa_begin_catch(ptr)

declare dso_local void @_ZSt9terminatev()

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z7my_initR9my_struct(ptr noundef nonnull align 8 dereferenceable(40) %t) #3 {
entry:
  %t.addr = alloca ptr, align 8
  store ptr %t, ptr %t.addr, align 8
  %0 = load ptr, ptr %t.addr, align 8
  %a = getelementptr inbounds %struct.my_struct, ptr %0, i32 0, i32 0
  store i32 1, ptr %a, align 8
  %1 = load ptr, ptr %t.addr, align 8
  %b = getelementptr inbounds %struct.my_struct, ptr %1, i32 0, i32 1
  store i32 1, ptr %b, align 4
  %2 = load ptr, ptr %t.addr, align 8
  %c = getelementptr inbounds %struct.my_struct, ptr %2, i32 0, i32 2
  %call = call noundef nonnull align 8 dereferenceable(32) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEPKc(ptr noundef nonnull align 8 dereferenceable(32) %c, ptr noundef @.str)
  ret void
}

declare dso_local noundef nonnull align 8 dereferenceable(32) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEPKc(ptr noundef nonnull align 8 dereferenceable(32), ptr noundef) #1

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z6my_addR9my_structRKS_(ptr noundef nonnull align 8 dereferenceable(40) %lhs, ptr noundef nonnull align 8 dereferenceable(40) %rhs) #3 {
entry:
  %lhs.addr = alloca ptr, align 8
  %rhs.addr = alloca ptr, align 8
  %ref.tmp = alloca %"class.std::__cxx11::basic_string", align 8
  store ptr %lhs, ptr %lhs.addr, align 8
  store ptr %rhs, ptr %rhs.addr, align 8
  %0 = load ptr, ptr %rhs.addr, align 8
  %a = getelementptr inbounds %struct.my_struct, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %a, align 8
  %2 = load ptr, ptr %lhs.addr, align 8
  %a1 = getelementptr inbounds %struct.my_struct, ptr %2, i32 0, i32 0
  %3 = load i32, ptr %a1, align 8
  %add = add nsw i32 %3, %1
  store i32 %add, ptr %a1, align 8
  %4 = load ptr, ptr %rhs.addr, align 8
  %b = getelementptr inbounds %struct.my_struct, ptr %4, i32 0, i32 1
  %5 = load i32, ptr %b, align 4
  %6 = load ptr, ptr %lhs.addr, align 8
  %b2 = getelementptr inbounds %struct.my_struct, ptr %6, i32 0, i32 1
  %7 = load i32, ptr %b2, align 4
  %add3 = add nsw i32 %7, %5
  store i32 %add3, ptr %b2, align 4
  %8 = load ptr, ptr %lhs.addr, align 8
  %c = getelementptr inbounds %struct.my_struct, ptr %8, i32 0, i32 2
  %call = call noundef i32 @_ZNSt7__cxx114stoiERKNS_12basic_stringIcSt11char_traitsIcESaIcEEEPmi(ptr noundef nonnull align 8 dereferenceable(32) %c, ptr noundef null, i32 noundef 10)
  %9 = load ptr, ptr %rhs.addr, align 8
  %c4 = getelementptr inbounds %struct.my_struct, ptr %9, i32 0, i32 2
  %call5 = call noundef i32 @_ZNSt7__cxx114stoiERKNS_12basic_stringIcSt11char_traitsIcESaIcEEEPmi(ptr noundef nonnull align 8 dereferenceable(32) %c4, ptr noundef null, i32 noundef 10)
  %add6 = add nsw i32 %call, %call5
  %conv = sext i32 %add6 to i64
  call void @_ZNSt7__cxx119to_stringEx(ptr sret(%"class.std::__cxx11::basic_string") align 8 %ref.tmp, i64 noundef %conv)
  %10 = load ptr, ptr %lhs.addr, align 8
  %c7 = getelementptr inbounds %struct.my_struct, ptr %10, i32 0, i32 2
  %call8 = call noundef nonnull align 8 dereferenceable(32) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEOS4_(ptr noundef nonnull align 8 dereferenceable(32) %c7, ptr noundef nonnull align 8 dereferenceable(32) %ref.tmp) #10
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED1Ev(ptr noundef nonnull align 8 dereferenceable(32) %ref.tmp) #10
  ret void
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt7__cxx119to_stringEx(ptr noalias sret(%"class.std::__cxx11::basic_string") align 8 %agg.result, i64 noundef %__val) #3 comdat personality ptr @__gxx_personality_v0 {
entry:
  %result.ptr = alloca ptr, align 8
  %__val.addr = alloca i64, align 8
  %__neg = alloca i8, align 1
  %__uval = alloca i64, align 8
  %__len = alloca i32, align 4
  %nrvo = alloca i1, align 1
  %ref.tmp = alloca %"class.std::allocator", align 1
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  store ptr %agg.result, ptr %result.ptr, align 8
  store i64 %__val, ptr %__val.addr, align 8
  %0 = load i64, ptr %__val.addr, align 8
  %cmp = icmp slt i64 %0, 0
  %frombool = zext i1 %cmp to i8
  store i8 %frombool, ptr %__neg, align 1
  %1 = load i8, ptr %__neg, align 1
  %tobool = trunc i8 %1 to i1
  br i1 %tobool, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %2 = load i64, ptr %__val.addr, align 8
  %neg = xor i64 %2, -1
  %add = add i64 %neg, 1
  br label %cond.end

cond.false:                                       ; preds = %entry
  %3 = load i64, ptr %__val.addr, align 8
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i64 [ %add, %cond.true ], [ %3, %cond.false ]
  store i64 %cond, ptr %__uval, align 8
  %4 = load i64, ptr %__uval, align 8
  %call = call noundef i32 @_ZNSt8__detail14__to_chars_lenIyEEjT_i(i64 noundef %4, i32 noundef 10) #10
  store i32 %call, ptr %__len, align 4
  store i1 false, ptr %nrvo, align 1
  %5 = load i8, ptr %__neg, align 1
  %tobool1 = trunc i8 %5 to i1
  %conv = zext i1 %tobool1 to i32
  %6 = load i32, ptr %__len, align 4
  %add2 = add i32 %conv, %6
  %conv3 = zext i32 %add2 to i64
  call void @_ZNSaIcEC1Ev(ptr noundef nonnull align 1 dereferenceable(1) %ref.tmp) #10
  invoke void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC1EmcRKS3_(ptr noundef nonnull align 8 dereferenceable(32) %agg.result, i64 noundef %conv3, i8 noundef signext 45, ptr noundef nonnull align 1 dereferenceable(1) %ref.tmp)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %cond.end
  call void @_ZNSaIcED1Ev(ptr noundef nonnull align 1 dereferenceable(1) %ref.tmp) #10
  %7 = load i8, ptr %__neg, align 1
  %tobool4 = trunc i8 %7 to i1
  %conv5 = zext i1 %tobool4 to i64
  %call8 = invoke noundef nonnull align 1 dereferenceable(1) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEixEm(ptr noundef nonnull align 8 dereferenceable(32) %agg.result, i64 noundef %conv5)
          to label %invoke.cont7 unwind label %lpad6

invoke.cont7:                                     ; preds = %invoke.cont
  %8 = load i32, ptr %__len, align 4
  %9 = load i64, ptr %__uval, align 8
  call void @_ZNSt8__detail18__to_chars_10_implIyEEvPcjT_(ptr noundef %call8, i32 noundef %8, i64 noundef %9) #10
  store i1 true, ptr %nrvo, align 1
  %nrvo.val = load i1, ptr %nrvo, align 1
  br i1 %nrvo.val, label %nrvo.skipdtor, label %nrvo.unused

lpad:                                             ; preds = %cond.end
  %10 = landingpad { ptr, i32 }
          cleanup
  %11 = extractvalue { ptr, i32 } %10, 0
  store ptr %11, ptr %exn.slot, align 8
  %12 = extractvalue { ptr, i32 } %10, 1
  store i32 %12, ptr %ehselector.slot, align 4
  call void @_ZNSaIcED1Ev(ptr noundef nonnull align 1 dereferenceable(1) %ref.tmp) #10
  br label %eh.resume

lpad6:                                            ; preds = %invoke.cont
  %13 = landingpad { ptr, i32 }
          cleanup
  %14 = extractvalue { ptr, i32 } %13, 0
  store ptr %14, ptr %exn.slot, align 8
  %15 = extractvalue { ptr, i32 } %13, 1
  store i32 %15, ptr %ehselector.slot, align 4
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED1Ev(ptr noundef nonnull align 8 dereferenceable(32) %agg.result) #10
  br label %eh.resume

nrvo.unused:                                      ; preds = %invoke.cont7
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED1Ev(ptr noundef nonnull align 8 dereferenceable(32) %agg.result) #10
  br label %nrvo.skipdtor

nrvo.skipdtor:                                    ; preds = %nrvo.unused, %invoke.cont7
  ret void

eh.resume:                                        ; preds = %lpad6, %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val9 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val9
}

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef i32 @_ZNSt7__cxx114stoiERKNS_12basic_stringIcSt11char_traitsIcESaIcEEEPmi(ptr noundef nonnull align 8 dereferenceable(32) %__str, ptr noundef %__idx, i32 noundef %__base) #3 comdat {
entry:
  %__str.addr = alloca ptr, align 8
  %__idx.addr = alloca ptr, align 8
  %__base.addr = alloca i32, align 4
  store ptr %__str, ptr %__str.addr, align 8
  store ptr %__idx, ptr %__idx.addr, align 8
  store i32 %__base, ptr %__base.addr, align 4
  %0 = load ptr, ptr %__str.addr, align 8
  %call = call noundef ptr @_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5c_strEv(ptr noundef nonnull align 8 dereferenceable(32) %0) #10
  %1 = load ptr, ptr %__idx.addr, align 8
  %2 = load i32, ptr %__base.addr, align 4
  %call1 = call noundef i32 @_ZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_(ptr noundef @strtol, ptr noundef @.str.1, ptr noundef %call, ptr noundef %1, i32 noundef %2)
  ret i32 %call1
}

; Function Attrs: nounwind
declare dso_local noundef nonnull align 8 dereferenceable(32) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEOS4_(ptr noundef nonnull align 8 dereferenceable(32), ptr noundef nonnull align 8 dereferenceable(32)) #4

; Function Attrs: nounwind
declare dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED1Ev(ptr noundef nonnull align 8 dereferenceable(32)) unnamed_addr #4

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef i32 @_ZNSt8__detail14__to_chars_lenIyEEjT_i(i64 noundef %__value, i32 noundef %__base) #0 comdat {
entry:
  %retval = alloca i32, align 4
  %__value.addr = alloca i64, align 8
  %__base.addr = alloca i32, align 4
  %__n = alloca i32, align 4
  %__b2 = alloca i32, align 4
  %__b3 = alloca i32, align 4
  %__b4 = alloca i64, align 8
  store i64 %__value, ptr %__value.addr, align 8
  store i32 %__base, ptr %__base.addr, align 4
  store i32 1, ptr %__n, align 4
  %0 = load i32, ptr %__base.addr, align 4
  %1 = load i32, ptr %__base.addr, align 4
  %mul = mul nsw i32 %0, %1
  store i32 %mul, ptr %__b2, align 4
  %2 = load i32, ptr %__b2, align 4
  %3 = load i32, ptr %__base.addr, align 4
  %mul1 = mul i32 %2, %3
  store i32 %mul1, ptr %__b3, align 4
  %4 = load i32, ptr %__b3, align 4
  %5 = load i32, ptr %__base.addr, align 4
  %mul2 = mul i32 %4, %5
  %conv = zext i32 %mul2 to i64
  store i64 %conv, ptr %__b4, align 8
  br label %for.cond

for.cond:                                         ; preds = %if.end16, %entry
  %6 = load i64, ptr %__value.addr, align 8
  %7 = load i32, ptr %__base.addr, align 4
  %conv3 = zext i32 %7 to i64
  %cmp = icmp ult i64 %6, %conv3
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %for.cond
  %8 = load i32, ptr %__n, align 4
  store i32 %8, ptr %retval, align 4
  br label %return

if.end:                                           ; preds = %for.cond
  %9 = load i64, ptr %__value.addr, align 8
  %10 = load i32, ptr %__b2, align 4
  %conv4 = zext i32 %10 to i64
  %cmp5 = icmp ult i64 %9, %conv4
  br i1 %cmp5, label %if.then6, label %if.end7

if.then6:                                         ; preds = %if.end
  %11 = load i32, ptr %__n, align 4
  %add = add i32 %11, 1
  store i32 %add, ptr %retval, align 4
  br label %return

if.end7:                                          ; preds = %if.end
  %12 = load i64, ptr %__value.addr, align 8
  %13 = load i32, ptr %__b3, align 4
  %conv8 = zext i32 %13 to i64
  %cmp9 = icmp ult i64 %12, %conv8
  br i1 %cmp9, label %if.then10, label %if.end12

if.then10:                                        ; preds = %if.end7
  %14 = load i32, ptr %__n, align 4
  %add11 = add i32 %14, 2
  store i32 %add11, ptr %retval, align 4
  br label %return

if.end12:                                         ; preds = %if.end7
  %15 = load i64, ptr %__value.addr, align 8
  %16 = load i64, ptr %__b4, align 8
  %cmp13 = icmp ult i64 %15, %16
  br i1 %cmp13, label %if.then14, label %if.end16

if.then14:                                        ; preds = %if.end12
  %17 = load i32, ptr %__n, align 4
  %add15 = add i32 %17, 3
  store i32 %add15, ptr %retval, align 4
  br label %return

if.end16:                                         ; preds = %if.end12
  %18 = load i64, ptr %__b4, align 8
  %19 = load i64, ptr %__value.addr, align 8
  %div = udiv i64 %19, %18
  store i64 %div, ptr %__value.addr, align 8
  %20 = load i32, ptr %__n, align 4
  %add17 = add i32 %20, 4
  store i32 %add17, ptr %__n, align 4
  br label %for.cond, !llvm.loop !5

return:                                           ; preds = %if.then14, %if.then10, %if.then6, %if.then
  %21 = load i32, ptr %retval, align 4
  ret i32 %21
}

; Function Attrs: nounwind
declare dso_local void @_ZNSaIcEC1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #4

declare dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC1EmcRKS3_(ptr noundef nonnull align 8 dereferenceable(32), i64 noundef, i8 noundef signext, ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local void @_ZNSaIcED1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #4

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt8__detail18__to_chars_10_implIyEEvPcjT_(ptr noundef %__first, i32 noundef %__len, i64 noundef %__val) #0 comdat {
entry:
  %__first.addr = alloca ptr, align 8
  %__len.addr = alloca i32, align 4
  %__val.addr = alloca i64, align 8
  %__pos = alloca i32, align 4
  %__num = alloca i64, align 8
  %__num8 = alloca i64, align 8
  store ptr %__first, ptr %__first.addr, align 8
  store i32 %__len, ptr %__len.addr, align 4
  store i64 %__val, ptr %__val.addr, align 8
  %0 = load i32, ptr %__len.addr, align 4
  %sub = sub i32 %0, 1
  store i32 %sub, ptr %__pos, align 4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %1 = load i64, ptr %__val.addr, align 8
  %cmp = icmp uge i64 %1, 100
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %2 = load i64, ptr %__val.addr, align 8
  %rem = urem i64 %2, 100
  %mul = mul i64 %rem, 2
  store i64 %mul, ptr %__num, align 8
  %3 = load i64, ptr %__val.addr, align 8
  %div = udiv i64 %3, 100
  store i64 %div, ptr %__val.addr, align 8
  %4 = load i64, ptr %__num, align 8
  %add = add i64 %4, 1
  %arrayidx = getelementptr inbounds [201 x i8], ptr @_ZZNSt8__detail18__to_chars_10_implIyEEvPcjT_E8__digits.const, i64 0, i64 %add
  %5 = load i8, ptr %arrayidx, align 1
  %6 = load ptr, ptr %__first.addr, align 8
  %7 = load i32, ptr %__pos, align 4
  %idxprom = zext i32 %7 to i64
  %arrayidx1 = getelementptr inbounds i8, ptr %6, i64 %idxprom
  store i8 %5, ptr %arrayidx1, align 1
  %8 = load i64, ptr %__num, align 8
  %arrayidx2 = getelementptr inbounds [201 x i8], ptr @_ZZNSt8__detail18__to_chars_10_implIyEEvPcjT_E8__digits.const, i64 0, i64 %8
  %9 = load i8, ptr %arrayidx2, align 1
  %10 = load ptr, ptr %__first.addr, align 8
  %11 = load i32, ptr %__pos, align 4
  %sub3 = sub i32 %11, 1
  %idxprom4 = zext i32 %sub3 to i64
  %arrayidx5 = getelementptr inbounds i8, ptr %10, i64 %idxprom4
  store i8 %9, ptr %arrayidx5, align 1
  %12 = load i32, ptr %__pos, align 4
  %sub6 = sub i32 %12, 2
  store i32 %sub6, ptr %__pos, align 4
  br label %while.cond, !llvm.loop !7

while.end:                                        ; preds = %while.cond
  %13 = load i64, ptr %__val.addr, align 8
  %cmp7 = icmp uge i64 %13, 10
  br i1 %cmp7, label %if.then, label %if.else

if.then:                                          ; preds = %while.end
  %14 = load i64, ptr %__val.addr, align 8
  %mul9 = mul i64 %14, 2
  store i64 %mul9, ptr %__num8, align 8
  %15 = load i64, ptr %__num8, align 8
  %add10 = add i64 %15, 1
  %arrayidx11 = getelementptr inbounds [201 x i8], ptr @_ZZNSt8__detail18__to_chars_10_implIyEEvPcjT_E8__digits.const, i64 0, i64 %add10
  %16 = load i8, ptr %arrayidx11, align 1
  %17 = load ptr, ptr %__first.addr, align 8
  %arrayidx12 = getelementptr inbounds i8, ptr %17, i64 1
  store i8 %16, ptr %arrayidx12, align 1
  %18 = load i64, ptr %__num8, align 8
  %arrayidx13 = getelementptr inbounds [201 x i8], ptr @_ZZNSt8__detail18__to_chars_10_implIyEEvPcjT_E8__digits.const, i64 0, i64 %18
  %19 = load i8, ptr %arrayidx13, align 1
  %20 = load ptr, ptr %__first.addr, align 8
  %arrayidx14 = getelementptr inbounds i8, ptr %20, i64 0
  store i8 %19, ptr %arrayidx14, align 1
  br label %if.end

if.else:                                          ; preds = %while.end
  %21 = load i64, ptr %__val.addr, align 8
  %add15 = add i64 48, %21
  %conv = trunc i64 %add15 to i8
  %22 = load ptr, ptr %__first.addr, align 8
  %arrayidx16 = getelementptr inbounds i8, ptr %22, i64 0
  store i8 %conv, ptr %arrayidx16, align 1
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

declare dso_local noundef nonnull align 1 dereferenceable(1) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEixEm(ptr noundef nonnull align 8 dereferenceable(32), i64 noundef) #1

; Function Attrs: mustprogress noinline optnone uwtable
define linkonce_odr dso_local noundef i32 @_ZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_(ptr noundef %__convf, ptr noundef %__name, ptr noundef %__str, ptr noundef %__idx, i32 noundef %__base) #3 comdat personality ptr @__gxx_personality_v0 {
entry:
  %__convf.addr = alloca ptr, align 8
  %__name.addr = alloca ptr, align 8
  %__str.addr = alloca ptr, align 8
  %__idx.addr = alloca ptr, align 8
  %__base.addr = alloca i32, align 4
  %__ret = alloca i32, align 4
  %__endptr = alloca ptr, align 8
  %__save_errno = alloca %struct._Save_errno, align 4
  %__tmp = alloca i64, align 8
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  %agg.tmp = alloca %"struct.std::integral_constant", align 1
  %ref.tmp = alloca %"struct.std::is_same", align 1
  store ptr %__convf, ptr %__convf.addr, align 8
  store ptr %__name, ptr %__name.addr, align 8
  store ptr %__str, ptr %__str.addr, align 8
  store ptr %__idx, ptr %__idx.addr, align 8
  store i32 %__base, ptr %__base.addr, align 4
  call void @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %__save_errno)
  %0 = load ptr, ptr %__convf.addr, align 8
  %1 = load ptr, ptr %__str.addr, align 8
  %2 = load i32, ptr %__base.addr, align 4
  %call = invoke noundef i64 %0(ptr noundef %1, ptr noundef %__endptr, i32 noundef %2)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  store i64 %call, ptr %__tmp, align 8
  %3 = load ptr, ptr %__endptr, align 8
  %4 = load ptr, ptr %__str.addr, align 8
  %cmp = icmp eq ptr %3, %4
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %invoke.cont
  %5 = load ptr, ptr %__name.addr, align 8
  invoke void @_ZSt24__throw_invalid_argumentPKc(ptr noundef %5) #14
          to label %invoke.cont1 unwind label %lpad

invoke.cont1:                                     ; preds = %if.then
  unreachable

lpad:                                             ; preds = %if.then6, %lor.rhs, %if.then, %entry
  %6 = landingpad { ptr, i32 }
          cleanup
  %7 = extractvalue { ptr, i32 } %6, 0
  store ptr %7, ptr %exn.slot, align 8
  %8 = extractvalue { ptr, i32 } %6, 1
  store i32 %8, ptr %ehselector.slot, align 4
  call void @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %__save_errno) #10
  br label %eh.resume

if.else:                                          ; preds = %invoke.cont
  %call2 = call ptr @__errno_location() #15
  %9 = load i32, ptr %call2, align 4
  %cmp3 = icmp eq i32 %9, 34
  br i1 %cmp3, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %if.else
  %10 = load i64, ptr %__tmp, align 8
  %call5 = invoke noundef zeroext i1 @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN10_Range_chk6_S_chkElSt17integral_constantIbLb1EE(i64 noundef %10)
          to label %invoke.cont4 unwind label %lpad

invoke.cont4:                                     ; preds = %lor.rhs
  br label %lor.end

lor.end:                                          ; preds = %invoke.cont4, %if.else
  %11 = phi i1 [ true, %if.else ], [ %call5, %invoke.cont4 ]
  br i1 %11, label %if.then6, label %if.else8

if.then6:                                         ; preds = %lor.end
  %12 = load ptr, ptr %__name.addr, align 8
  invoke void @_ZSt20__throw_out_of_rangePKc(ptr noundef %12) #14
          to label %invoke.cont7 unwind label %lpad

invoke.cont7:                                     ; preds = %if.then6
  unreachable

if.else8:                                         ; preds = %lor.end
  %13 = load i64, ptr %__tmp, align 8
  %conv = trunc i64 %13 to i32
  store i32 %conv, ptr %__ret, align 4
  br label %if.end

if.end:                                           ; preds = %if.else8
  br label %if.end9

if.end9:                                          ; preds = %if.end
  %14 = load ptr, ptr %__idx.addr, align 8
  %tobool = icmp ne ptr %14, null
  br i1 %tobool, label %if.then10, label %if.end11

if.then10:                                        ; preds = %if.end9
  %15 = load ptr, ptr %__endptr, align 8
  %16 = load ptr, ptr %__str.addr, align 8
  %sub.ptr.lhs.cast = ptrtoint ptr %15 to i64
  %sub.ptr.rhs.cast = ptrtoint ptr %16 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %17 = load ptr, ptr %__idx.addr, align 8
  store i64 %sub.ptr.sub, ptr %17, align 8
  br label %if.end11

if.end11:                                         ; preds = %if.then10, %if.end9
  %18 = load i32, ptr %__ret, align 4
  call void @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %__save_errno) #10
  ret i32 %18

eh.resume:                                        ; preds = %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val12 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val12
}

; Function Attrs: nounwind
declare dso_local i64 @strtol(ptr noundef, ptr noundef, i32 noundef) #4

; Function Attrs: nounwind
declare dso_local noundef ptr @_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5c_strEv(ptr noundef nonnull align 8 dereferenceable(32)) #4

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this) unnamed_addr #5 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %_M_errno = getelementptr inbounds %struct._Save_errno, ptr %this1, i32 0, i32 0
  %call = call ptr @__errno_location() #15
  %0 = load i32, ptr %call, align 4
  store i32 %0, ptr %_M_errno, align 4
  %call2 = call ptr @__errno_location() #15
  store i32 0, ptr %call2, align 4
  ret void
}

; Function Attrs: noreturn
declare dso_local void @_ZSt24__throw_invalid_argumentPKc(ptr noundef) #6

; Function Attrs: nounwind readnone willreturn
declare dso_local ptr @__errno_location() #7

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef zeroext i1 @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN10_Range_chk6_S_chkElSt17integral_constantIbLb1EE(i64 noundef %__val) #0 comdat align 2 {
entry:
  %0 = alloca %"struct.std::integral_constant", align 1
  %__val.addr = alloca i64, align 8
  store i64 %__val, ptr %__val.addr, align 8
  %1 = load i64, ptr %__val.addr, align 8
  %cmp = icmp slt i64 %1, -2147483648
  br i1 %cmp, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %entry
  %2 = load i64, ptr %__val.addr, align 8
  %cmp1 = icmp sgt i64 %2, 2147483647
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %entry
  %3 = phi i1 [ true, %entry ], [ %cmp1, %lor.rhs ]
  ret i1 %3
}

; Function Attrs: noreturn
declare dso_local void @_ZSt20__throw_out_of_rangePKc(ptr noundef) #6

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this) unnamed_addr #5 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %call = call ptr @__errno_location() #15
  %0 = load i32, ptr %call, align 4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %_M_errno = getelementptr inbounds %struct._Save_errno, ptr %this1, i32 0, i32 0
  %1 = load i32, ptr %_M_errno, align 4
  %call2 = call ptr @__errno_location() #15
  store i32 %1, ptr %call2, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: noinline uwtable
define internal void @__cxx_global_var_init() #8 section ".text.startup" personality ptr @__gxx_personality_v0 {
entry:
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %invoke.cont, %entry
  %arrayctor.cur = phi ptr [ @_ZL1y, %entry ], [ %arrayctor.next, %invoke.cont ]
  invoke void @_ZN9my_structC2Ev(ptr noundef nonnull align 8 dereferenceable(40) %arrayctor.cur)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %arrayctor.loop
  %arrayctor.next = getelementptr inbounds %struct.my_struct, ptr %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr %arrayctor.next, getelementptr inbounds (%struct.my_struct, ptr @_ZL1y, i64 100)
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %invoke.cont
  %0 = call i32 @__cxa_atexit(ptr @__cxx_global_array_dtor, ptr null, ptr @__dso_handle) #10
  ret void

lpad:                                             ; preds = %arrayctor.loop
  %1 = landingpad { ptr, i32 }
          cleanup
  %2 = extractvalue { ptr, i32 } %1, 0
  store ptr %2, ptr %exn.slot, align 8
  %3 = extractvalue { ptr, i32 } %1, 1
  store i32 %3, ptr %ehselector.slot, align 4
  %arraydestroy.isempty = icmp eq ptr @_ZL1y, %arrayctor.cur
  br i1 %arraydestroy.isempty, label %arraydestroy.done1, label %arraydestroy.body

arraydestroy.body:                                ; preds = %arraydestroy.body, %lpad
  %arraydestroy.elementPast = phi ptr [ %arrayctor.cur, %lpad ], [ %arraydestroy.element, %arraydestroy.body ]
  %arraydestroy.element = getelementptr inbounds %struct.my_struct, ptr %arraydestroy.elementPast, i64 -1
  call void @_ZN9my_structD2Ev(ptr noundef nonnull align 8 dereferenceable(40) %arraydestroy.element) #10
  %arraydestroy.done = icmp eq ptr %arraydestroy.element, @_ZL1y
  br i1 %arraydestroy.done, label %arraydestroy.done1, label %arraydestroy.body

arraydestroy.done1:                               ; preds = %arraydestroy.body, %lpad
  br label %eh.resume

eh.resume:                                        ; preds = %arraydestroy.done1
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val2 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val2
}

; Function Attrs: noinline optnone uwtable
define linkonce_odr dso_local void @_ZN9my_structC2Ev(ptr noundef nonnull align 8 dereferenceable(40) %this) unnamed_addr #9 comdat align 2 personality ptr @__gxx_personality_v0 {
entry:
  %this.addr = alloca ptr, align 8
  %ref.tmp = alloca %"class.std::allocator", align 1
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %a = getelementptr inbounds %struct.my_struct, ptr %this1, i32 0, i32 0
  store i32 0, ptr %a, align 8
  %b = getelementptr inbounds %struct.my_struct, ptr %this1, i32 0, i32 1
  store i32 0, ptr %b, align 4
  %c = getelementptr inbounds %struct.my_struct, ptr %this1, i32 0, i32 2
  call void @_ZNSaIcEC1Ev(ptr noundef nonnull align 1 dereferenceable(1) %ref.tmp) #10
  invoke void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC1EPKcRKS3_(ptr noundef nonnull align 8 dereferenceable(32) %c, ptr noundef @.str.2, ptr noundef nonnull align 1 dereferenceable(1) %ref.tmp)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  call void @_ZNSaIcED1Ev(ptr noundef nonnull align 1 dereferenceable(1) %ref.tmp) #10
  ret void

lpad:                                             ; preds = %entry
  %0 = landingpad { ptr, i32 }
          cleanup
  %1 = extractvalue { ptr, i32 } %0, 0
  store ptr %1, ptr %exn.slot, align 8
  %2 = extractvalue { ptr, i32 } %0, 1
  store i32 %2, ptr %ehselector.slot, align 4
  call void @_ZNSaIcED1Ev(ptr noundef nonnull align 1 dereferenceable(1) %ref.tmp) #10
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val2 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val2
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN9my_structD2Ev(ptr noundef nonnull align 8 dereferenceable(40) %this) unnamed_addr #5 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %c = getelementptr inbounds %struct.my_struct, ptr %this1, i32 0, i32 2
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED1Ev(ptr noundef nonnull align 8 dereferenceable(32) %c) #10
  ret void
}

; Function Attrs: noinline uwtable
define internal void @__cxx_global_array_dtor(ptr noundef %0) #8 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  br label %arraydestroy.body

arraydestroy.body:                                ; preds = %arraydestroy.body, %entry
  %arraydestroy.elementPast = phi ptr [ getelementptr inbounds (%struct.my_struct, ptr @_ZL1y, i64 100), %entry ], [ %arraydestroy.element, %arraydestroy.body ]
  %arraydestroy.element = getelementptr inbounds %struct.my_struct, ptr %arraydestroy.elementPast, i64 -1
  call void @_ZN9my_structD2Ev(ptr noundef nonnull align 8 dereferenceable(40) %arraydestroy.element) #10
  %arraydestroy.done = icmp eq ptr %arraydestroy.element, @_ZL1y
  br i1 %arraydestroy.done, label %arraydestroy.done1, label %arraydestroy.body

arraydestroy.done1:                               ; preds = %arraydestroy.body
  ret void
}

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(ptr, ptr, ptr) #10

declare dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC1EPKcRKS3_(ptr noundef nonnull align 8 dereferenceable(32), ptr noundef, ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #1

; Function Attrs: noinline uwtable
define internal void @__cxx_global_var_init.3() #8 section ".text.startup" personality ptr @__gxx_personality_v0 {
entry:
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %invoke.cont, %entry
  %arrayctor.cur = phi ptr [ @_ZL1x, %entry ], [ %arrayctor.next, %invoke.cont ]
  invoke void @_ZN9my_structC2Ev(ptr noundef nonnull align 8 dereferenceable(40) %arrayctor.cur)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %arrayctor.loop
  %arrayctor.next = getelementptr inbounds %struct.my_struct, ptr %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr %arrayctor.next, getelementptr inbounds (%struct.my_struct, ptr @_ZL1x, i64 100)
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %invoke.cont
  %0 = call i32 @__cxa_atexit(ptr @__cxx_global_array_dtor.4, ptr null, ptr @__dso_handle) #10
  ret void

lpad:                                             ; preds = %arrayctor.loop
  %1 = landingpad { ptr, i32 }
          cleanup
  %2 = extractvalue { ptr, i32 } %1, 0
  store ptr %2, ptr %exn.slot, align 8
  %3 = extractvalue { ptr, i32 } %1, 1
  store i32 %3, ptr %ehselector.slot, align 4
  %arraydestroy.isempty = icmp eq ptr @_ZL1x, %arrayctor.cur
  br i1 %arraydestroy.isempty, label %arraydestroy.done1, label %arraydestroy.body

arraydestroy.body:                                ; preds = %arraydestroy.body, %lpad
  %arraydestroy.elementPast = phi ptr [ %arrayctor.cur, %lpad ], [ %arraydestroy.element, %arraydestroy.body ]
  %arraydestroy.element = getelementptr inbounds %struct.my_struct, ptr %arraydestroy.elementPast, i64 -1
  call void @_ZN9my_structD2Ev(ptr noundef nonnull align 8 dereferenceable(40) %arraydestroy.element) #10
  %arraydestroy.done = icmp eq ptr %arraydestroy.element, @_ZL1x
  br i1 %arraydestroy.done, label %arraydestroy.done1, label %arraydestroy.body

arraydestroy.done1:                               ; preds = %arraydestroy.body, %lpad
  br label %eh.resume

eh.resume:                                        ; preds = %arraydestroy.done1
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val2 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val2
}

; Function Attrs: noinline uwtable
define internal void @__cxx_global_array_dtor.4(ptr noundef %0) #8 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  br label %arraydestroy.body

arraydestroy.body:                                ; preds = %arraydestroy.body, %entry
  %arraydestroy.elementPast = phi ptr [ getelementptr inbounds (%struct.my_struct, ptr @_ZL1x, i64 100), %entry ], [ %arraydestroy.element, %arraydestroy.body ]
  %arraydestroy.element = getelementptr inbounds %struct.my_struct, ptr %arraydestroy.elementPast, i64 -1
  call void @_ZN9my_structD2Ev(ptr noundef nonnull align 8 dereferenceable(40) %arraydestroy.element) #10
  %arraydestroy.done = icmp eq ptr %arraydestroy.element, @_ZL1x
  br i1 %arraydestroy.done, label %arraydestroy.done1, label %arraydestroy.body

arraydestroy.done1:                               ; preds = %arraydestroy.body
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z17cq415166_1d_d_refRA100_9my_struct(ptr noundef nonnull align 8 dereferenceable(4000) %yref) #11 {
entry:
  %yref.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store ptr %yref, ptr %yref.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = load ptr, ptr %yref.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.UDR:BYREF.TYPED"(ptr %yref.addr, [100 x %struct.my_struct] zeroinitializer, i32 1, ptr null, ptr @_ZTS9my_struct.omp.destr, ptr @.omp_combiner., ptr @.omp_initializer.),
    "QUAL.OMP.NUM_THREADS"(i32 100),
    "QUAL.OMP.SHARED:TYPED"(ptr @_ZL1x, %struct.my_struct zeroinitializer, i64 100),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

; ALL-NOT: "QUAL.OMP.REDUCTION.UDR"
; ALL: red.init.body{{.*}}:
; ALL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.init.body{{.*}} ]
; ALL: call void @.omp_initializer.(ptr %{{.*}}, ptr %{{.*}})
; ALL: br i1 %red.cpy.done{{.*}}, label %red.init.done{{.*}}, label %red.init.body{{.*}}

; CRITICAL: call void @__kmpc_critical({{.*}})
; CRITICAL: red.update.body{{.*}}:
; CRITICAL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; CRITICAL: call void @.omp_combiner.(ptr %{{.*}}, ptr %{{.*}})
; CRITICAL: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; CRITICAL: call void @__kmpc_end_critical({{.*}})
; CRITICAL: call void @_ZTS9my_struct.omp.destr(ptr %red.cpy.dest.ptr{{.*}})

; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED-DAG: red.update.body{{.*}}:
; FASTRED-DAG: call void @.omp_combiner.(ptr %{{.*}}, ptr %{{.*}})
; FASTRED-DAG: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; FASTRED-DAG: call void @__kmpc_end_reduce({{.*}})
; FASTRED-DAG: call void @_ZTS9my_struct.omp.destr(ptr %red.cpy.dest.ptr{{.*}})

  %2 = load i32, ptr %.omp.lb, align 4
  store i32 %2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr %.omp.iv, align 4
  %4 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  store i32 5, ptr %j, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.inner.for.body
  %6 = load i32, ptr %j, align 4
  %cmp1 = icmp sle i32 %6, 8
  br i1 %cmp1, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %7 = load ptr, ptr %yref.addr, align 8
  %8 = load i32, ptr %j, align 4
  %idxprom = sext i32 %8 to i64
  %arrayidx = getelementptr inbounds [100 x %struct.my_struct], ptr %7, i64 0, i64 %idxprom
  %9 = load i32, ptr %i, align 4
  %idxprom2 = sext i32 %9 to i64
  %arrayidx3 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1x, i64 0, i64 %idxprom2
  call void @_Z6my_addR9my_structRKS_(ptr noundef nonnull align 8 dereferenceable(40) %arrayidx, ptr noundef nonnull align 8 dereferenceable(40) %arrayidx3) #10
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %10 = load i32, ptr %j, align 4
  %inc = add nsw i32 %10, 1
  store i32 %inc, ptr %j, align 4
  br label %for.cond, !llvm.loop !8

for.end:                                          ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, ptr %.omp.iv, align 4
  %add4 = add nsw i32 %11, 1
  store i32 %add4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #10

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #10

; Function Attrs: noinline uwtable
define internal void @.omp_combiner.(ptr noalias noundef %0, ptr noalias noundef %1) #8 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  call void @_Z6my_addR9my_structRKS_(ptr noundef nonnull align 8 dereferenceable(40) %3, ptr noundef nonnull align 8 dereferenceable(40) %2)
  ret void
}

; Function Attrs: noinline uwtable
define internal void @.omp_initializer.(ptr noalias noundef %0, ptr noalias noundef %1) #8 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  call void @_Z7my_initR9my_struct(ptr noundef nonnull align 8 dereferenceable(40) %3)
  ret void
}

; Function Attrs: noinline uwtable
define internal void @_ZTS9my_struct.omp.destr(ptr noundef %0) #8 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  %1 = load ptr, ptr %.addr, align 8
  call void @_ZN9my_structD2Ev(ptr noundef nonnull align 8 dereferenceable(40) %1) #10
  ret void
}

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main() #12 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %ref.tmp = alloca %"class.std::__cxx11::basic_string", align 8
  store i32 0, ptr %retval, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %0, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr %i, align 4
  %2 = load i32, ptr %i, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1x, i64 0, i64 %idxprom
  %a = getelementptr inbounds %struct.my_struct, ptr %arrayidx, i32 0, i32 0
  store i32 %1, ptr %a, align 8
  %3 = load i32, ptr %i, align 4
  %4 = load i32, ptr %i, align 4
  %mul = mul nsw i32 %3, %4
  %5 = load i32, ptr %i, align 4
  %idxprom1 = sext i32 %5 to i64
  %arrayidx2 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1x, i64 0, i64 %idxprom1
  %b = getelementptr inbounds %struct.my_struct, ptr %arrayidx2, i32 0, i32 1
  store i32 %mul, ptr %b, align 4
  %6 = load i32, ptr %i, align 4
  %7 = load i32, ptr %i, align 4
  %mul3 = mul nsw i32 %6, %7
  %8 = load i32, ptr %i, align 4
  %mul4 = mul nsw i32 %mul3, %8
  %conv = sext i32 %mul4 to i64
  call void @_ZNSt7__cxx119to_stringEx(ptr sret(%"class.std::__cxx11::basic_string") align 8 %ref.tmp, i64 noundef %conv)
  %9 = load i32, ptr %i, align 4
  %idxprom5 = sext i32 %9 to i64
  %arrayidx6 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1x, i64 0, i64 %idxprom5
  %c = getelementptr inbounds %struct.my_struct, ptr %arrayidx6, i32 0, i32 2
  %call = call noundef nonnull align 8 dereferenceable(32) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEOS4_(ptr noundef nonnull align 8 dereferenceable(32) %c, ptr noundef nonnull align 8 dereferenceable(32) %ref.tmp) #10
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED1Ev(ptr noundef nonnull align 8 dereferenceable(32) %ref.tmp) #10
  %10 = load i32, ptr %i, align 4
  %idxprom7 = sext i32 %10 to i64
  %arrayidx8 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1y, i64 0, i64 %idxprom7
  %a9 = getelementptr inbounds %struct.my_struct, ptr %arrayidx8, i32 0, i32 0
  store i32 0, ptr %a9, align 8
  %11 = load i32, ptr %i, align 4
  %idxprom10 = sext i32 %11 to i64
  %arrayidx11 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1y, i64 0, i64 %idxprom10
  %b12 = getelementptr inbounds %struct.my_struct, ptr %arrayidx11, i32 0, i32 1
  store i32 0, ptr %b12, align 4
  %12 = load i32, ptr %i, align 4
  %idxprom13 = sext i32 %12 to i64
  %arrayidx14 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1y, i64 0, i64 %idxprom13
  %c15 = getelementptr inbounds %struct.my_struct, ptr %arrayidx14, i32 0, i32 2
  %call16 = call noundef nonnull align 8 dereferenceable(32) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEPKc(ptr noundef nonnull align 8 dereferenceable(32) %c15, ptr noundef @.str.2)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %13 = load i32, ptr %i, align 4
  %inc = add nsw i32 %13, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond, !llvm.loop !9

for.end:                                          ; preds = %for.cond
  call void @_Z17cq415166_1d_d_refRA100_9my_struct(ptr noundef nonnull align 8 dereferenceable(4000) @_ZL1y)
  ret i32 0
}

; Function Attrs: noinline uwtable
define internal void @_GLOBAL__sub_I_omp45_array_reduction_1d_udr_with_nonpods_ref.cpp() #8 section ".text.startup" {
entry:
  call void @__cxx_global_var_init()
  call void @__cxx_global_var_init.3()
  ret void
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { noinline noreturn nounwind }
attributes #3 = { mustprogress noinline optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #5 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #6 = { noreturn "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #7 = { nounwind readnone willreturn "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #8 = { noinline uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #9 = { noinline optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #10 = { nounwind }
attributes #11 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #12 = { mustprogress noinline norecurse optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #13 = { noreturn nounwind }
attributes #14 = { noreturn }
attributes #15 = { nounwind readnone willreturn }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
!7 = distinct !{!7, !6}
!8 = distinct !{!8, !6}
!9 = distinct !{!9, !6}
