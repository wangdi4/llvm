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
; // Reduction on:         Element of Array
; // Reduction type:       UDR on NONPOD (with initializer)
; // Array section layout: 1-D
; // Access in body:       EXPR_SUBSCRIPT
; void cq415166_1d_h() {
;
; #pragma omp parallel for reduction(my_reduction_add:y[5]) num_threads(N)
;     for (int i = 0; i < N; i++) {
;         my_add(y[5], x[i]);
;     }
; }
;
; int main() {
;     for (int i = 0; i < N; i++) {
;         x[i].a = i; x[i].b = i * i; x[i].c = std::to_string(static_cast<long long>(i * i * i));
;         y[i].a = 0; y[i].b = 0; y[i].c = "0";
;     }
;     cq415166_1d_h();
;
;     return 0;
; }

; The test IR was hand-modified to use a constant section offset.
; CFE currently generates IR instructions to compute it.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { i32, i32, %"class.std::__cxx11::basic_string" }
%"class.std::__cxx11::basic_string" = type { %"struct.std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider", i64, %union.anon }
%"struct.std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" = type { ptr }
%union.anon = type { i64, [8 x i8] }
%"class.std::allocator" = type { i8 }
%struct.__va_list_tag = type { i32, i32, ptr, ptr }
%"struct.std::__false_type" = type { i8 }
%"struct.std::forward_iterator_tag" = type { i8 }
%"struct.std::random_access_iterator_tag" = type { i8 }
%struct._Save_errno = type { i32 }
%"struct.std::integral_constant" = type { i8 }
%"struct.std::is_same" = type { i8 }

$_ZStltIcSt11char_traitsIcESaIcEEbRKNSt7__cxx1112basic_stringIT_T0_T1_EESA_ = comdat any

$_ZNSt7__cxx119to_stringEx = comdat any

$_ZNSt7__cxx114stoiERKNS_12basic_stringIcSt11char_traitsIcESaIcEEEPmi = comdat any

$_ZN9my_structC2Ev = comdat any

$_ZN9my_structD2Ev = comdat any

$_ZN9__gnu_cxx12__to_xstringINSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEcEET_PFiPT0_mPKS8_P13__va_list_tagEmSB_z = comdat any

$_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2IPcvEET_S7_RKS3_ = comdat any

$_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructIPcEEvT_S7_ = comdat any

$_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_Alloc_hiderD2Ev = comdat any

$_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE16_M_construct_auxIPcEEvT_S7_St12__false_type = comdat any

$_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructIPcEEvT_S7_St20forward_iterator_tag = comdat any

$_ZN9__gnu_cxx17__is_null_pointerIcEEbPT_ = comdat any

$_ZSt8distanceIPcENSt15iterator_traitsIT_E15difference_typeES2_S2_ = comdat any

$__clang_call_terminate = comdat any

$_ZSt10__distanceIPcENSt15iterator_traitsIT_E15difference_typeES2_S2_St26random_access_iterator_tag = comdat any

$_ZSt19__iterator_categoryIPcENSt15iterator_traitsIT_E17iterator_categoryERKS2_ = comdat any

$_ZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_ = comdat any

$_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoC2Ev = comdat any

$_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN10_Range_chk6_S_chkElSt17integral_constantIbLb1EE = comdat any

$_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoD2Ev = comdat any

@.str = private unnamed_addr constant [2 x i8] c"1\00", align 1
@_ZL1y_f73166f30f655dd789033585661ec02c = internal global [100 x %struct.my_struct] zeroinitializer, align 16
@__dso_handle = external hidden global i8
@_ZL1x_f73166f30f655dd789033585661ec02c = internal global [100 x %struct.my_struct] zeroinitializer, align 16
@.str.3 = private unnamed_addr constant [2 x i8] c"0\00", align 1
@.str.4 = private unnamed_addr constant [5 x i8] c"%lld\00", align 1
@.str.5 = private unnamed_addr constant [42 x i8] c"basic_string::_M_construct null not valid\00", align 1
@.str.6 = private unnamed_addr constant [5 x i8] c"stoi\00", align 1
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_omp45_array_section_reduction_1d_udr_with_nonpods_subscript.cpp, ptr null }]

; Function Attrs: noinline nounwind optnone uwtable
define dso_local zeroext i1 @_ZltRK9my_structS1_(ptr dereferenceable(40) %t1, ptr dereferenceable(40) %t2) #0 {
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
  %call = call zeroext i1 @_ZStltIcSt11char_traitsIcESaIcEEbRKNSt7__cxx1112basic_stringIT_T0_T1_EESA_(ptr dereferenceable(32) %c, ptr dereferenceable(32) %c4) #5
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %lor.lhs.false, %entry
  %10 = phi i1 [ true, %lor.lhs.false ], [ true, %entry ], [ %call, %lor.rhs ]
  ret i1 %10
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local zeroext i1 @_ZStltIcSt11char_traitsIcESaIcEEbRKNSt7__cxx1112basic_stringIT_T0_T1_EESA_(ptr dereferenceable(32) %__lhs, ptr dereferenceable(32) %__rhs) #0 comdat personality ptr @__gxx_personality_v0 {
entry:
  %__lhs.addr = alloca ptr, align 8
  %__rhs.addr = alloca ptr, align 8
  store ptr %__lhs, ptr %__lhs.addr, align 8
  store ptr %__rhs, ptr %__rhs.addr, align 8
  %0 = load ptr, ptr %__lhs.addr, align 8
  %1 = load ptr, ptr %__rhs.addr, align 8
  %call = invoke i32 @_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7compareERKS4_(ptr %0, ptr dereferenceable(32) %1)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %entry
  %cmp = icmp slt i32 %call, 0
  ret i1 %cmp

terminate.lpad:                                   ; preds = %entry
  %2 = landingpad { ptr, i32 }
          catch ptr null
  %3 = extractvalue { ptr, i32 } %2, 0
  call void @__clang_call_terminate(ptr %3) #12
  unreachable
}

; Function Attrs: noinline optnone uwtable
define dso_local void @_Z7my_initR9my_struct(ptr dereferenceable(40) %t) #1 {
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
  %call = call dereferenceable(32) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEPKc(ptr %c, ptr @.str)
  ret void
}

declare dso_local dereferenceable(32) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEPKc(ptr, ptr) #2

; Function Attrs: noinline optnone uwtable
define dso_local void @_Z6my_addR9my_structRKS_(ptr dereferenceable(40) %lhs, ptr dereferenceable(40) %rhs) #1 {
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
  %call = call i32 @_ZNSt7__cxx114stoiERKNS_12basic_stringIcSt11char_traitsIcESaIcEEEPmi(ptr dereferenceable(32) %c, ptr null, i32 10)
  %9 = load ptr, ptr %rhs.addr, align 8
  %c4 = getelementptr inbounds %struct.my_struct, ptr %9, i32 0, i32 2
  %call5 = call i32 @_ZNSt7__cxx114stoiERKNS_12basic_stringIcSt11char_traitsIcESaIcEEEPmi(ptr dereferenceable(32) %c4, ptr null, i32 10)
  %add6 = add nsw i32 %call, %call5
  %conv = sext i32 %add6 to i64
  call void @_ZNSt7__cxx119to_stringEx(ptr sret(%"class.std::__cxx11::basic_string") %ref.tmp, i64 %conv)
  %10 = load ptr, ptr %lhs.addr, align 8
  %c7 = getelementptr inbounds %struct.my_struct, ptr %10, i32 0, i32 2
  %call8 = call dereferenceable(32) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEOS4_(ptr %c7, ptr dereferenceable(32) %ref.tmp) #5
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED1Ev(ptr %ref.tmp) #5
  ret void
}

; Function Attrs: noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt7__cxx119to_stringEx(ptr noalias sret(%"class.std::__cxx11::basic_string") %agg.result, i64 %__val) #1 comdat {
entry:
  %result.ptr = alloca ptr, align 8
  %__val.addr = alloca i64, align 8
  %0 = bitcast ptr %agg.result to ptr
  store ptr %0, ptr %result.ptr, align 8
  store i64 %__val, ptr %__val.addr, align 8
  %1 = load i64, ptr %__val.addr, align 8
  call void (ptr, ptr, i64, ptr, ...) @_ZN9__gnu_cxx12__to_xstringINSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEcEET_PFiPT0_mPKS8_P13__va_list_tagEmSB_z(ptr sret(%"class.std::__cxx11::basic_string") %agg.result, ptr @vsnprintf, i64 32, ptr @.str.4, i64 %1)
  ret void
}

; Function Attrs: noinline optnone uwtable
define linkonce_odr dso_local i32 @_ZNSt7__cxx114stoiERKNS_12basic_stringIcSt11char_traitsIcESaIcEEEPmi(ptr dereferenceable(32) %__str, ptr %__idx, i32 %__base) #1 comdat {
entry:
  %__str.addr = alloca ptr, align 8
  %__idx.addr = alloca ptr, align 8
  %__base.addr = alloca i32, align 4
  store ptr %__str, ptr %__str.addr, align 8
  store ptr %__idx, ptr %__idx.addr, align 8
  store i32 %__base, ptr %__base.addr, align 4
  %0 = load ptr, ptr %__str.addr, align 8
  %call = call ptr @_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5c_strEv(ptr %0) #5
  %1 = load ptr, ptr %__idx.addr, align 8
  %2 = load i32, ptr %__base.addr, align 4
  %call1 = call i32 @_ZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_(ptr @strtol, ptr @.str.6, ptr %call, ptr %1, i32 %2)
  ret i32 %call1
}

; Function Attrs: nounwind
declare dso_local dereferenceable(32) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEOS4_(ptr, ptr dereferenceable(32)) #3

; Function Attrs: nounwind
declare dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED1Ev(ptr) unnamed_addr #3

; Function Attrs: noinline uwtable
define internal void @__cxx_global_var_init() #4 section ".text.startup" personality ptr @__gxx_personality_v0 {
entry:
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %invoke.cont, %entry
  %arrayctor.cur = phi ptr [ @_ZL1y_f73166f30f655dd789033585661ec02c, %entry ], [ %arrayctor.next, %invoke.cont ]
  invoke void @_ZN9my_structC2Ev(ptr %arrayctor.cur)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %arrayctor.loop
  %arrayctor.next = getelementptr inbounds %struct.my_struct, ptr %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr %arrayctor.next, getelementptr inbounds (%struct.my_struct, ptr @_ZL1y_f73166f30f655dd789033585661ec02c, i64 100)
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %invoke.cont
  %0 = call i32 @__cxa_atexit(ptr @__cxx_global_array_dtor, ptr null, ptr @__dso_handle) #5
  ret void

lpad:                                             ; preds = %arrayctor.loop
  %1 = landingpad { ptr, i32 }
          cleanup
  %2 = extractvalue { ptr, i32 } %1, 0
  store ptr %2, ptr %exn.slot, align 8
  %3 = extractvalue { ptr, i32 } %1, 1
  store i32 %3, ptr %ehselector.slot, align 4
  %arraydestroy.isempty = icmp eq ptr @_ZL1y_f73166f30f655dd789033585661ec02c, %arrayctor.cur
  br i1 %arraydestroy.isempty, label %arraydestroy.done1, label %arraydestroy.body

arraydestroy.body:                                ; preds = %arraydestroy.body, %lpad
  %arraydestroy.elementPast = phi ptr [ %arrayctor.cur, %lpad ], [ %arraydestroy.element, %arraydestroy.body ]
  %arraydestroy.element = getelementptr inbounds %struct.my_struct, ptr %arraydestroy.elementPast, i64 -1
  call void @_ZN9my_structD2Ev(ptr %arraydestroy.element) #5
  %arraydestroy.done = icmp eq ptr %arraydestroy.element, @_ZL1y_f73166f30f655dd789033585661ec02c
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
define linkonce_odr dso_local void @_ZN9my_structC2Ev(ptr %this) unnamed_addr #1 comdat align 2 personality ptr @__gxx_personality_v0 {
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
  call void @_ZNSaIcEC1Ev(ptr %ref.tmp) #5
  invoke void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC1EPKcRKS3_(ptr %c, ptr @.str.3, ptr dereferenceable(1) %ref.tmp)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  call void @_ZNSaIcED1Ev(ptr %ref.tmp) #5
  ret void

lpad:                                             ; preds = %entry
  %0 = landingpad { ptr, i32 }
          cleanup
  %1 = extractvalue { ptr, i32 } %0, 0
  store ptr %1, ptr %exn.slot, align 8
  %2 = extractvalue { ptr, i32 } %0, 1
  store i32 %2, ptr %ehselector.slot, align 4
  call void @_ZNSaIcED1Ev(ptr %ref.tmp) #5
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val2 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val2
}

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN9my_structD2Ev(ptr %this) unnamed_addr #0 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %c = getelementptr inbounds %struct.my_struct, ptr %this1, i32 0, i32 2
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED1Ev(ptr %c) #5
  ret void
}

; Function Attrs: noinline uwtable
define internal void @__cxx_global_array_dtor(ptr %0) #4 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  br label %arraydestroy.body

arraydestroy.body:                                ; preds = %arraydestroy.body, %entry
  %arraydestroy.elementPast = phi ptr [ getelementptr inbounds (%struct.my_struct, ptr @_ZL1y_f73166f30f655dd789033585661ec02c, i64 100), %entry ], [ %arraydestroy.element, %arraydestroy.body ]
  %arraydestroy.element = getelementptr inbounds %struct.my_struct, ptr %arraydestroy.elementPast, i64 -1
  call void @_ZN9my_structD2Ev(ptr %arraydestroy.element) #5
  %arraydestroy.done = icmp eq ptr %arraydestroy.element, @_ZL1y_f73166f30f655dd789033585661ec02c
  br i1 %arraydestroy.done, label %arraydestroy.done1, label %arraydestroy.body

arraydestroy.done1:                               ; preds = %arraydestroy.body
  ret void
}

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(ptr, ptr, ptr) #5

; Function Attrs: noinline uwtable
define internal void @__cxx_global_var_init.1() #4 section ".text.startup" personality ptr @__gxx_personality_v0 {
entry:
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %invoke.cont, %entry
  %arrayctor.cur = phi ptr [ @_ZL1x_f73166f30f655dd789033585661ec02c, %entry ], [ %arrayctor.next, %invoke.cont ]
  invoke void @_ZN9my_structC2Ev(ptr %arrayctor.cur)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %arrayctor.loop
  %arrayctor.next = getelementptr inbounds %struct.my_struct, ptr %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr %arrayctor.next, getelementptr inbounds (%struct.my_struct, ptr @_ZL1x_f73166f30f655dd789033585661ec02c, i64 100)
  br i1 %arrayctor.done, label %arrayctor.cont, label %arrayctor.loop

arrayctor.cont:                                   ; preds = %invoke.cont
  %0 = call i32 @__cxa_atexit(ptr @__cxx_global_array_dtor.2, ptr null, ptr @__dso_handle) #5
  ret void

lpad:                                             ; preds = %arrayctor.loop
  %1 = landingpad { ptr, i32 }
          cleanup
  %2 = extractvalue { ptr, i32 } %1, 0
  store ptr %2, ptr %exn.slot, align 8
  %3 = extractvalue { ptr, i32 } %1, 1
  store i32 %3, ptr %ehselector.slot, align 4
  %arraydestroy.isempty = icmp eq ptr @_ZL1x_f73166f30f655dd789033585661ec02c, %arrayctor.cur
  br i1 %arraydestroy.isempty, label %arraydestroy.done1, label %arraydestroy.body

arraydestroy.body:                                ; preds = %arraydestroy.body, %lpad
  %arraydestroy.elementPast = phi ptr [ %arrayctor.cur, %lpad ], [ %arraydestroy.element, %arraydestroy.body ]
  %arraydestroy.element = getelementptr inbounds %struct.my_struct, ptr %arraydestroy.elementPast, i64 -1
  call void @_ZN9my_structD2Ev(ptr %arraydestroy.element) #5
  %arraydestroy.done = icmp eq ptr %arraydestroy.element, @_ZL1x_f73166f30f655dd789033585661ec02c
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
define internal void @__cxx_global_array_dtor.2(ptr %0) #4 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  br label %arraydestroy.body

arraydestroy.body:                                ; preds = %arraydestroy.body, %entry
  %arraydestroy.elementPast = phi ptr [ getelementptr inbounds (%struct.my_struct, ptr @_ZL1x_f73166f30f655dd789033585661ec02c, i64 100), %entry ], [ %arraydestroy.element, %arraydestroy.body ]
  %arraydestroy.element = getelementptr inbounds %struct.my_struct, ptr %arraydestroy.elementPast, i64 -1
  call void @_ZN9my_structD2Ev(ptr %arraydestroy.element) #5
  %arraydestroy.done = icmp eq ptr %arraydestroy.element, @_ZL1x_f73166f30f655dd789033585661ec02c
  br i1 %arraydestroy.done, label %arraydestroy.done1, label %arraydestroy.body

arraydestroy.done1:                               ; preds = %arraydestroy.body
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z13cq415166_1d_hv() #6 {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.UDR:ARRSECT.TYPED"(ptr @_ZL1y_f73166f30f655dd789033585661ec02c, %struct.my_struct zeroinitializer, i64 1, i64 5, ptr null, ptr @_ZTS9my_struct.omp.destr, ptr @.omp_combiner., ptr @.omp_initializer.),
    "QUAL.OMP.NUM_THREADS"(i32 100),
    "QUAL.OMP.SHARED:TYPED"(ptr @_ZL1x_f73166f30f655dd789033585661ec02c, %struct.my_struct zeroinitializer, i64 100),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

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
; CRITICAL: call void @_ZTS9my_struct.omp.destr(ptr %{{.*}})

; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED-DAG: red.update.body{{.*}}:
; FASTRED-DAG: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; FASTRED-DAG: call void @.omp_combiner.(ptr %{{.*}}, ptr %{{.*}})
; FASTRED-DAG: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; FASTRED-DAG: call void @__kmpc_end_reduce({{.*}})
; FASTRED-DAG: call void @_ZTS9my_struct.omp.destr(ptr %{{.*}})

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %5 = load i32, ptr %i, align 4
  %idxprom = sext i32 %5 to i64
  %arrayidx = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1x_f73166f30f655dd789033585661ec02c, i64 0, i64 %idxprom
  call void @_Z6my_addR9my_structRKS_(ptr dereferenceable(40) getelementptr inbounds ([100 x %struct.my_struct], ptr @_ZL1y_f73166f30f655dd789033585661ec02c, i64 0, i64 5), ptr dereferenceable(40) %arrayidx) #5
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %6, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #5

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #5

; Function Attrs: noinline uwtable
define internal void @.omp_combiner.(ptr noalias %0, ptr noalias %1) #4 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  call void @_Z6my_addR9my_structRKS_(ptr dereferenceable(40) %3, ptr dereferenceable(40) %2)
  ret void
}

; Function Attrs: noinline uwtable
define internal void @.omp_initializer.(ptr noalias %0, ptr noalias %1) #4 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  call void @_Z7my_initR9my_struct(ptr dereferenceable(40) %3)
  ret void
}

; Function Attrs: noinline uwtable
define internal void @_ZTS9my_struct.omp.destr(ptr %0) #4 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  %1 = load ptr, ptr %.addr, align 8
  call void @_ZN9my_structD2Ev(ptr %1) #5
  ret void
}

; Function Attrs: noinline norecurse optnone uwtable
define dso_local i32 @main() #7 {
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
  %arrayidx = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1x_f73166f30f655dd789033585661ec02c, i64 0, i64 %idxprom
  %a = getelementptr inbounds %struct.my_struct, ptr %arrayidx, i32 0, i32 0
  store i32 %1, ptr %a, align 8
  %3 = load i32, ptr %i, align 4
  %4 = load i32, ptr %i, align 4
  %mul = mul nsw i32 %3, %4
  %5 = load i32, ptr %i, align 4
  %idxprom1 = sext i32 %5 to i64
  %arrayidx2 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1x_f73166f30f655dd789033585661ec02c, i64 0, i64 %idxprom1
  %b = getelementptr inbounds %struct.my_struct, ptr %arrayidx2, i32 0, i32 1
  store i32 %mul, ptr %b, align 4
  %6 = load i32, ptr %i, align 4
  %7 = load i32, ptr %i, align 4
  %mul3 = mul nsw i32 %6, %7
  %8 = load i32, ptr %i, align 4
  %mul4 = mul nsw i32 %mul3, %8
  %conv = sext i32 %mul4 to i64
  call void @_ZNSt7__cxx119to_stringEx(ptr sret(%"class.std::__cxx11::basic_string") %ref.tmp, i64 %conv)
  %9 = load i32, ptr %i, align 4
  %idxprom5 = sext i32 %9 to i64
  %arrayidx6 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1x_f73166f30f655dd789033585661ec02c, i64 0, i64 %idxprom5
  %c = getelementptr inbounds %struct.my_struct, ptr %arrayidx6, i32 0, i32 2
  %call = call dereferenceable(32) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEOS4_(ptr %c, ptr dereferenceable(32) %ref.tmp) #5
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED1Ev(ptr %ref.tmp) #5
  %10 = load i32, ptr %i, align 4
  %idxprom7 = sext i32 %10 to i64
  %arrayidx8 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1y_f73166f30f655dd789033585661ec02c, i64 0, i64 %idxprom7
  %a9 = getelementptr inbounds %struct.my_struct, ptr %arrayidx8, i32 0, i32 0
  store i32 0, ptr %a9, align 8
  %11 = load i32, ptr %i, align 4
  %idxprom10 = sext i32 %11 to i64
  %arrayidx11 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1y_f73166f30f655dd789033585661ec02c, i64 0, i64 %idxprom10
  %b12 = getelementptr inbounds %struct.my_struct, ptr %arrayidx11, i32 0, i32 1
  store i32 0, ptr %b12, align 4
  %12 = load i32, ptr %i, align 4
  %idxprom13 = sext i32 %12 to i64
  %arrayidx14 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1y_f73166f30f655dd789033585661ec02c, i64 0, i64 %idxprom13
  %c15 = getelementptr inbounds %struct.my_struct, ptr %arrayidx14, i32 0, i32 2
  %call16 = call dereferenceable(32) ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEaSEPKc(ptr %c15, ptr @.str.3)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %13 = load i32, ptr %i, align 4
  %inc = add nsw i32 %13, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @_Z13cq415166_1d_hv()
  ret i32 0
}

; Function Attrs: noinline optnone uwtable
define linkonce_odr dso_local void @_ZN9__gnu_cxx12__to_xstringINSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEcEET_PFiPT0_mPKS8_P13__va_list_tagEmSB_z(ptr noalias sret(%"class.std::__cxx11::basic_string") %agg.result, ptr %__convf, i64 %__n, ptr %__fmt, ...) #1 comdat personality ptr @__gxx_personality_v0 {
entry:
  %result.ptr = alloca ptr, align 8
  %__convf.addr = alloca ptr, align 8
  %__n.addr = alloca i64, align 8
  %__fmt.addr = alloca ptr, align 8
  %__s = alloca ptr, align 8
  %__args = alloca [1 x %struct.__va_list_tag], align 16
  %__len = alloca i32, align 4
  %ref.tmp = alloca %"class.std::allocator", align 1
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  %0 = bitcast ptr %agg.result to ptr
  store ptr %0, ptr %result.ptr, align 8
  store ptr %__convf, ptr %__convf.addr, align 8
  store i64 %__n, ptr %__n.addr, align 8
  store ptr %__fmt, ptr %__fmt.addr, align 8
  %1 = load i64, ptr %__n.addr, align 8
  %mul = mul i64 1, %1
  %2 = alloca i8, i64 %mul, align 16
  store ptr %2, ptr %__s, align 8
  %arraydecay = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %__args, i64 0, i64 0
  %arraydecay1 = bitcast ptr %arraydecay to ptr
  call void @llvm.va_start(ptr %arraydecay1)
  %3 = load ptr, ptr %__convf.addr, align 8
  %4 = load ptr, ptr %__s, align 8
  %5 = load i64, ptr %__n.addr, align 8
  %6 = load ptr, ptr %__fmt.addr, align 8
  %arraydecay2 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %__args, i64 0, i64 0
  %call = call i32 %3(ptr %4, i64 %5, ptr %6, ptr %arraydecay2)
  store i32 %call, ptr %__len, align 4
  %arraydecay3 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %__args, i64 0, i64 0
  %arraydecay34 = bitcast ptr %arraydecay3 to ptr
  call void @llvm.va_end(ptr %arraydecay34)
  %7 = load ptr, ptr %__s, align 8
  %8 = load ptr, ptr %__s, align 8
  %9 = load i32, ptr %__len, align 4
  %idx.ext = sext i32 %9 to i64
  %add.ptr = getelementptr inbounds i8, ptr %8, i64 %idx.ext
  call void @_ZNSaIcEC1Ev(ptr %ref.tmp) #5
  invoke void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2IPcvEET_S7_RKS3_(ptr %agg.result, ptr %7, ptr %add.ptr, ptr dereferenceable(1) %ref.tmp)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  call void @_ZNSaIcED1Ev(ptr %ref.tmp) #5
  ret void

lpad:                                             ; preds = %entry
  %10 = landingpad { ptr, i32 }
          cleanup
  %11 = extractvalue { ptr, i32 } %10, 0
  store ptr %11, ptr %exn.slot, align 8
  %12 = extractvalue { ptr, i32 } %10, 1
  store i32 %12, ptr %ehselector.slot, align 4
  call void @_ZNSaIcED1Ev(ptr %ref.tmp) #5
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val5 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val5
}

; Function Attrs: nounwind
declare dso_local i32 @vsnprintf(ptr, i64, ptr, ptr) #3

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.va_start(ptr) #8

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.va_end(ptr) #8

; Function Attrs: nounwind
declare dso_local void @_ZNSaIcEC1Ev(ptr) unnamed_addr #3

; Function Attrs: noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC2IPcvEET_S7_RKS3_(ptr %this, ptr %__beg, ptr %__end, ptr dereferenceable(1) %__a) unnamed_addr #1 comdat align 2 personality ptr @__gxx_personality_v0 {
entry:
  %this.addr = alloca ptr, align 8
  %__beg.addr = alloca ptr, align 8
  %__end.addr = alloca ptr, align 8
  %__a.addr = alloca ptr, align 8
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8
  store ptr %__beg, ptr %__beg.addr, align 8
  store ptr %__end, ptr %__end.addr, align 8
  store ptr %__a, ptr %__a.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %_M_dataplus = getelementptr inbounds %"class.std::__cxx11::basic_string", ptr %this1, i32 0, i32 0
  %call = call ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE13_M_local_dataEv(ptr %this1)
  %0 = load ptr, ptr %__a.addr, align 8
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_Alloc_hiderC1EPcRKS3_(ptr %_M_dataplus, ptr %call, ptr dereferenceable(1) %0)
  %1 = load ptr, ptr %__beg.addr, align 8
  %2 = load ptr, ptr %__end.addr, align 8
  invoke void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructIPcEEvT_S7_(ptr %this1, ptr %1, ptr %2)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  ret void

lpad:                                             ; preds = %entry
  %3 = landingpad { ptr, i32 }
          cleanup
  %4 = extractvalue { ptr, i32 } %3, 0
  store ptr %4, ptr %exn.slot, align 8
  %5 = extractvalue { ptr, i32 } %3, 1
  store i32 %5, ptr %ehselector.slot, align 4
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_Alloc_hiderD2Ev(ptr %_M_dataplus) #5
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val2 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val2
}

; Function Attrs: nounwind
declare dso_local void @_ZNSaIcED1Ev(ptr) unnamed_addr #3

declare dso_local ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE13_M_local_dataEv(ptr) #2

declare dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_Alloc_hiderC1EPcRKS3_(ptr, ptr, ptr dereferenceable(1)) unnamed_addr #2

; Function Attrs: noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructIPcEEvT_S7_(ptr %this, ptr %__beg, ptr %__end) #1 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  %__beg.addr = alloca ptr, align 8
  %__end.addr = alloca ptr, align 8
  %agg.tmp = alloca %"struct.std::__false_type", align 1
  store ptr %this, ptr %this.addr, align 8
  store ptr %__beg, ptr %__beg.addr, align 8
  store ptr %__end, ptr %__end.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load ptr, ptr %__beg.addr, align 8
  %1 = load ptr, ptr %__end.addr, align 8
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE16_M_construct_auxIPcEEvT_S7_St12__false_type(ptr %this1, ptr %0, ptr %1)
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_Alloc_hiderD2Ev(ptr %this) unnamed_addr #0 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = bitcast ptr %this1 to ptr
  call void @_ZNSaIcED2Ev(ptr %0) #5
  ret void
}

; Function Attrs: noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE16_M_construct_auxIPcEEvT_S7_St12__false_type(ptr %this, ptr %__beg, ptr %__end) #1 comdat align 2 {
entry:
  %0 = alloca %"struct.std::__false_type", align 1
  %this.addr = alloca ptr, align 8
  %__beg.addr = alloca ptr, align 8
  %__end.addr = alloca ptr, align 8
  %agg.tmp = alloca %"struct.std::forward_iterator_tag", align 1
  %ref.tmp = alloca %"struct.std::random_access_iterator_tag", align 1
  store ptr %this, ptr %this.addr, align 8
  store ptr %__beg, ptr %__beg.addr, align 8
  store ptr %__end, ptr %__end.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %1 = load ptr, ptr %__beg.addr, align 8
  %2 = load ptr, ptr %__end.addr, align 8
  %3 = bitcast ptr %ref.tmp to ptr
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructIPcEEvT_S7_St20forward_iterator_tag(ptr %this1, ptr %1, ptr %2)
  ret void
}

; Function Attrs: noinline optnone uwtable
define linkonce_odr dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructIPcEEvT_S7_St20forward_iterator_tag(ptr %this, ptr %__beg, ptr %__end) #1 comdat align 2 personality ptr @__gxx_personality_v0 {
entry:
  %0 = alloca %"struct.std::forward_iterator_tag", align 1
  %this.addr = alloca ptr, align 8
  %__beg.addr = alloca ptr, align 8
  %__end.addr = alloca ptr, align 8
  %__dnew = alloca i64, align 8
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8
  store ptr %__beg, ptr %__beg.addr, align 8
  store ptr %__end, ptr %__end.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %1 = load ptr, ptr %__beg.addr, align 8
  %call = call zeroext i1 @_ZN9__gnu_cxx17__is_null_pointerIcEEbPT_(ptr %1)
  br i1 %call, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %2 = load ptr, ptr %__beg.addr, align 8
  %3 = load ptr, ptr %__end.addr, align 8
  %cmp = icmp ne ptr %2, %3
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  call void @_ZSt19__throw_logic_errorPKc(ptr @.str.5) #13
  unreachable

if.end:                                           ; preds = %land.lhs.true, %entry
  %4 = load ptr, ptr %__beg.addr, align 8
  %5 = load ptr, ptr %__end.addr, align 8
  %call2 = call i64 @_ZSt8distanceIPcENSt15iterator_traitsIT_E15difference_typeES2_S2_(ptr %4, ptr %5)
  store i64 %call2, ptr %__dnew, align 8
  %6 = load i64, ptr %__dnew, align 8
  %cmp3 = icmp ugt i64 %6, 15
  br i1 %cmp3, label %if.then4, label %if.end6

if.then4:                                         ; preds = %if.end
  %call5 = call ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_createERmm(ptr %this1, ptr dereferenceable(8) %__dnew, i64 0)
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7_M_dataEPc(ptr %this1, ptr %call5)
  %7 = load i64, ptr %__dnew, align 8
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE11_M_capacityEm(ptr %this1, i64 %7)
  br label %if.end6

if.end6:                                          ; preds = %if.then4, %if.end
  %call7 = invoke ptr @_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7_M_dataEv(ptr %this1)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %if.end6
  %8 = load ptr, ptr %__beg.addr, align 8
  %9 = load ptr, ptr %__end.addr, align 8
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE13_S_copy_charsEPcS5_S5_(ptr %call7, ptr %8, ptr %9) #5
  br label %try.cont

lpad:                                             ; preds = %if.end6
  %10 = landingpad { ptr, i32 }
          catch ptr null
  %11 = extractvalue { ptr, i32 } %10, 0
  store ptr %11, ptr %exn.slot, align 8
  %12 = extractvalue { ptr, i32 } %10, 1
  store i32 %12, ptr %ehselector.slot, align 4
  br label %catch

catch:                                            ; preds = %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  %13 = call ptr @__cxa_begin_catch(ptr %exn) #5
  invoke void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv(ptr %this1)
          to label %invoke.cont9 unwind label %lpad8

invoke.cont9:                                     ; preds = %catch
  invoke void @__cxa_rethrow() #13
          to label %unreachable unwind label %lpad8

lpad8:                                            ; preds = %invoke.cont9, %catch
  %14 = landingpad { ptr, i32 }
          cleanup
  %15 = extractvalue { ptr, i32 } %14, 0
  store ptr %15, ptr %exn.slot, align 8
  %16 = extractvalue { ptr, i32 } %14, 1
  store i32 %16, ptr %ehselector.slot, align 4
  invoke void @__cxa_end_catch()
          to label %invoke.cont10 unwind label %terminate.lpad

invoke.cont10:                                    ; preds = %lpad8
  br label %eh.resume

try.cont:                                         ; preds = %invoke.cont
  %17 = load i64, ptr %__dnew, align 8
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE13_M_set_lengthEm(ptr %this1, i64 %17)
  ret void

eh.resume:                                        ; preds = %invoke.cont10
  %exn11 = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn11, 0
  %lpad.val12 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val12

terminate.lpad:                                   ; preds = %lpad8
  %18 = landingpad { ptr, i32 }
          catch ptr null
  %19 = extractvalue { ptr, i32 } %18, 0
  call void @__clang_call_terminate(ptr %19) #12
  unreachable

unreachable:                                      ; preds = %invoke.cont9
  unreachable
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local zeroext i1 @_ZN9__gnu_cxx17__is_null_pointerIcEEbPT_(ptr %__ptr) #0 comdat {
entry:
  %__ptr.addr = alloca ptr, align 8
  store ptr %__ptr, ptr %__ptr.addr, align 8
  %0 = load ptr, ptr %__ptr.addr, align 8
  %cmp = icmp eq ptr %0, null
  ret i1 %cmp
}

; Function Attrs: noreturn
declare dso_local void @_ZSt19__throw_logic_errorPKc(ptr) #9

; Function Attrs: noinline optnone uwtable
define linkonce_odr dso_local i64 @_ZSt8distanceIPcENSt15iterator_traitsIT_E15difference_typeES2_S2_(ptr %__first, ptr %__last) #1 comdat {
entry:
  %__first.addr = alloca ptr, align 8
  %__last.addr = alloca ptr, align 8
  %agg.tmp = alloca %"struct.std::random_access_iterator_tag", align 1
  %undef.agg.tmp = alloca %"struct.std::random_access_iterator_tag", align 1
  store ptr %__first, ptr %__first.addr, align 8
  store ptr %__last, ptr %__last.addr, align 8
  %0 = load ptr, ptr %__first.addr, align 8
  %1 = load ptr, ptr %__last.addr, align 8
  call void @_ZSt19__iterator_categoryIPcENSt15iterator_traitsIT_E17iterator_categoryERKS2_(ptr dereferenceable(8) %__first.addr)
  %call = call i64 @_ZSt10__distanceIPcENSt15iterator_traitsIT_E15difference_typeES2_S2_St26random_access_iterator_tag(ptr %0, ptr %1)
  ret i64 %call
}

declare dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7_M_dataEPc(ptr, ptr) #2

declare dso_local ptr @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_createERmm(ptr, ptr dereferenceable(8), i64) #2

declare dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE11_M_capacityEm(ptr, i64) #2

; Function Attrs: nounwind
declare dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE13_S_copy_charsEPcS5_S5_(ptr, ptr, ptr) #3

declare dso_local ptr @_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7_M_dataEv(ptr) #2

declare dso_local ptr @__cxa_begin_catch(ptr)

declare dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv(ptr) #2

declare dso_local void @__cxa_rethrow()

declare dso_local void @__cxa_end_catch()

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(ptr %0) #10 comdat {
  %2 = call ptr @__cxa_begin_catch(ptr %0) #5
  call void @_ZSt9terminatev() #12
  unreachable
}

declare dso_local void @_ZSt9terminatev()

declare dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE13_M_set_lengthEm(ptr, i64) #2

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local i64 @_ZSt10__distanceIPcENSt15iterator_traitsIT_E15difference_typeES2_S2_St26random_access_iterator_tag(ptr %__first, ptr %__last) #0 comdat {
entry:
  %0 = alloca %"struct.std::random_access_iterator_tag", align 1
  %__first.addr = alloca ptr, align 8
  %__last.addr = alloca ptr, align 8
  store ptr %__first, ptr %__first.addr, align 8
  store ptr %__last, ptr %__last.addr, align 8
  %1 = load ptr, ptr %__last.addr, align 8
  %2 = load ptr, ptr %__first.addr, align 8
  %sub.ptr.lhs.cast = ptrtoint ptr %1 to i64
  %sub.ptr.rhs.cast = ptrtoint ptr %2 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  ret i64 %sub.ptr.sub
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZSt19__iterator_categoryIPcENSt15iterator_traitsIT_E17iterator_categoryERKS2_(ptr dereferenceable(8) %0) #0 comdat {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  ret void
}

; Function Attrs: nounwind
declare dso_local void @_ZNSaIcED2Ev(ptr) unnamed_addr #3

; Function Attrs: noinline optnone uwtable
define linkonce_odr dso_local i32 @_ZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_(ptr %__convf, ptr %__name, ptr %__str, ptr %__idx, i32 %__base) #1 comdat personality ptr @__gxx_personality_v0 {
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
  call void @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoC2Ev(ptr %__save_errno)
  %0 = load ptr, ptr %__convf.addr, align 8
  %1 = load ptr, ptr %__str.addr, align 8
  %2 = load i32, ptr %__base.addr, align 4
  %call = invoke i64 %0(ptr %1, ptr %__endptr, i32 %2)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  store i64 %call, ptr %__tmp, align 8
  %3 = load ptr, ptr %__endptr, align 8
  %4 = load ptr, ptr %__str.addr, align 8
  %cmp = icmp eq ptr %3, %4
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %invoke.cont
  %5 = load ptr, ptr %__name.addr, align 8
  invoke void @_ZSt24__throw_invalid_argumentPKc(ptr %5) #13
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
  call void @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoD2Ev(ptr %__save_errno) #5
  br label %eh.resume

if.else:                                          ; preds = %invoke.cont
  %call2 = call ptr @__errno_location() #14
  %9 = load i32, ptr %call2, align 4
  %cmp3 = icmp eq i32 %9, 34
  br i1 %cmp3, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %if.else
  %10 = load i64, ptr %__tmp, align 8
  %11 = bitcast ptr %ref.tmp to ptr
  %call5 = invoke zeroext i1 @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN10_Range_chk6_S_chkElSt17integral_constantIbLb1EE(i64 %10)
          to label %invoke.cont4 unwind label %lpad

invoke.cont4:                                     ; preds = %lor.rhs
  br label %lor.end

lor.end:                                          ; preds = %invoke.cont4, %if.else
  %12 = phi i1 [ true, %if.else ], [ %call5, %invoke.cont4 ]
  br i1 %12, label %if.then6, label %if.else8

if.then6:                                         ; preds = %lor.end
  %13 = load ptr, ptr %__name.addr, align 8
  invoke void @_ZSt20__throw_out_of_rangePKc(ptr %13) #13
          to label %invoke.cont7 unwind label %lpad

invoke.cont7:                                     ; preds = %if.then6
  unreachable

if.else8:                                         ; preds = %lor.end
  %14 = load i64, ptr %__tmp, align 8
  %conv = trunc i64 %14 to i32
  store i32 %conv, ptr %__ret, align 4
  br label %if.end

if.end:                                           ; preds = %if.else8
  br label %if.end9

if.end9:                                          ; preds = %if.end
  %15 = load ptr, ptr %__idx.addr, align 8
  %tobool = icmp ne ptr %15, null
  br i1 %tobool, label %if.then10, label %if.end11

if.then10:                                        ; preds = %if.end9
  %16 = load ptr, ptr %__endptr, align 8
  %17 = load ptr, ptr %__str.addr, align 8
  %sub.ptr.lhs.cast = ptrtoint ptr %16 to i64
  %sub.ptr.rhs.cast = ptrtoint ptr %17 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %18 = load ptr, ptr %__idx.addr, align 8
  store i64 %sub.ptr.sub, ptr %18, align 8
  br label %if.end11

if.end11:                                         ; preds = %if.then10, %if.end9
  %19 = load i32, ptr %__ret, align 4
  call void @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoD2Ev(ptr %__save_errno) #5
  ret i32 %19

eh.resume:                                        ; preds = %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val12 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val12
}

; Function Attrs: nounwind
declare dso_local i64 @strtol(ptr, ptr, i32) #3

; Function Attrs: nounwind
declare dso_local ptr @_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5c_strEv(ptr) #3

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoC2Ev(ptr %this) unnamed_addr #0 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %_M_errno = getelementptr inbounds %struct._Save_errno, ptr %this1, i32 0, i32 0
  %call = call ptr @__errno_location() #14
  %0 = load i32, ptr %call, align 4
  store i32 %0, ptr %_M_errno, align 4
  %call2 = call ptr @__errno_location() #14
  store i32 0, ptr %call2, align 4
  ret void
}

; Function Attrs: noreturn
declare dso_local void @_ZSt24__throw_invalid_argumentPKc(ptr) #9

; Function Attrs: nounwind readnone
declare dso_local ptr @__errno_location() #11

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local zeroext i1 @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN10_Range_chk6_S_chkElSt17integral_constantIbLb1EE(i64 %__val) #0 comdat align 2 {
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
declare dso_local void @_ZSt20__throw_out_of_rangePKc(ptr) #9

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZZN9__gnu_cxx6__stoaIlicJiEEET0_PFT_PKT1_PPS3_DpT2_EPKcS5_PmS9_EN11_Save_errnoD2Ev(ptr %this) unnamed_addr #0 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %call = call ptr @__errno_location() #14
  %0 = load i32, ptr %call, align 4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %_M_errno = getelementptr inbounds %struct._Save_errno, ptr %this1, i32 0, i32 0
  %1 = load i32, ptr %_M_errno, align 4
  %call2 = call ptr @__errno_location() #14
  store i32 %1, ptr %call2, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

declare dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC1EPKcRKS3_(ptr, ptr, ptr dereferenceable(1)) unnamed_addr #2

declare dso_local i32 @_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7compareERKS4_(ptr, ptr dereferenceable(32)) #2

; Function Attrs: noinline uwtable
define internal void @_GLOBAL__sub_I_omp45_array_section_reduction_1d_udr_with_nonpods_subscript.cpp() #4 section ".text.startup" {
entry:
  call void @__cxx_global_var_init()
  call void @__cxx_global_var_init.1()
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind }
attributes #6 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { noinline norecurse optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { nocallback nofree nosync nounwind willreturn }
attributes #9 = { noreturn "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #10 = { noinline noreturn nounwind }
attributes #11 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #12 = { noreturn nounwind }
attributes #13 = { noreturn }
attributes #14 = { nounwind readnone }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
