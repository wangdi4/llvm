; RUN: opt -S -passes="simplifycfg" %s | FileCheck %s

; CHECK-NOT: resume.common
; CHECK-LABEL: _ZN1a2blD2Ev.exit
; CHECK: resume

; #pragma omp parallel
;    if (co.bf())
;      throw a::az("");

; There is no "catch" clause, so the parallel region contains a generic
; "resume or terminate" handler (either is an undefined condition).
; SimplifyCFG merges the resume with one outside the parallel region, which
; causes an illegal exit edge.
; This optimization can safely be disabled for functions that have the
; "may-have-openmp" attribute.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.bb::(anonymous namespace)::cf" = type { i8 }
%class._ZTSZ2cqvEUlvE_ = type { i8 }
%"struct.(anonymous namespace)::(anonymous namespace)::bg::bh" = type { i8 }
%"class.bb::(anonymous namespace)::bv" = type { i8 }
%"class.a::b::j" = type { %"struct.a::b::j<char>::av" }
%"struct.a::b::j<char>::av" = type { i8 }
%"class.cl::cm" = type { %"class.a::bd" }
%"class.a::bd" = type { i8 }
%"class.a::bp" = type { %"class.a::bl" }
%"class.a::bl" = type { %"union.a::bi" }
%"union.a::bi" = type { i8 }

$__clang_call_terminate = comdat any

$_ZTSN1a2azE = comdat any

$_ZTIN1a2azE = comdat any

@.str = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr
@_ZTSN1a2azE = linkonce_odr dso_local constant [8 x i8] c"N1a2azE\00", comdat, align 1
@_ZTIN1a2azE = linkonce_odr dso_local constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTSN1a2azE }, comdat, align 8

; Function Attrs: uwtable
define dso_local void @_Z2cqv() local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
DIR.OMP.PARALLEL.36:
  %cr = alloca %"class.bb::(anonymous namespace)::cf", align 1
  %agg.tmp = alloca %class._ZTSZ2cqvEUlvE_, align 1
  %agg.tmp1 = alloca %"struct.(anonymous namespace)::(anonymous namespace)::bg::bh", align 1
  %undef.agg.tmp2 = alloca %"class.bb::(anonymous namespace)::bv", align 1
  %savedstack = call ptr @llvm.stacksave()
  %agg.tmp.i.i = alloca %"class.a::b::j", align 1
  %ref.tmp.i = alloca %"class.cl::cm", align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %ref.tmp.i) #2
  store i8 0, ptr %ref.tmp.i, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr %agg.tmp.i.i)
  %co.i.i = getelementptr inbounds %"class.cl::cm", ptr %ref.tmp.i, i64 0, i32 0
  %call.i.i = call noundef i32 @_ZN1a2bdINS_2baIN2cl1lEEEE2bfEv(ptr noundef nonnull align 1 dereferenceable(1) %co.i.i)
  %tobool.not.i.i = icmp eq i32 %call.i.i, 0
  br i1 %tobool.not.i.i, label %_ZN2cl2cpEv.exit, label %if.then.i.i

if.then.i.i:                                      ; preds = %DIR.OMP.PARALLEL.36
  %exception.i.i = call ptr @__cxa_allocate_exception(i64 1) #2
  %aw.i.i.i = getelementptr inbounds %"class.a::b::j", ptr %agg.tmp.i.i, i64 0, i32 0
  invoke void @_ZN1a1b1jIcccE2axEv(ptr noundef nonnull align 1 dereferenceable(1) %agg.tmp.i.i)
          to label %.noexc.i.i unwind label %lpad.i.i

.noexc.i.i:                                       ; preds = %if.then.i.i
  invoke void @_ZN1a1b1jIcccE2avC1EN2aq2arIcEEc(ptr noundef nonnull align 1 dereferenceable(1) %aw.i.i.i, i8 noundef signext 0)
          to label %_ZN1a1b1jIcccEC2EPcc.exit.i.i unwind label %lpad.i.i

_ZN1a1b1jIcccEC2EPcc.exit.i.i:                    ; preds = %.noexc.i.i
  invoke void @_ZN1a2azC1ENS_1b1jIcccEE(ptr noundef nonnull align 1 dereferenceable(1) %exception.i.i)
          to label %invoke.cont2.i.i unwind label %lpad.i.i

invoke.cont2.i.i:                                 ; preds = %_ZN1a1b1jIcccEC2EPcc.exit.i.i
  call void @__cxa_throw(ptr nonnull %exception.i.i, ptr nonnull @_ZTIN1a2azE, ptr null) #10
  unreachable

lpad.i.i:                                         ; preds = %_ZN1a1b1jIcccEC2EPcc.exit.i.i, %.noexc.i.i, %if.then.i.i
  %0 = landingpad { ptr, i32 }
          cleanup
  call void @__cxa_free_exception(ptr %exception.i.i) #2
  resume { ptr, i32 } %0

_ZN2cl2cpEv.exit:                                 ; preds = %DIR.OMP.PARALLEL.36
  call void @llvm.lifetime.end.p0(i64 1, ptr %agg.tmp.i.i)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %ref.tmp.i) #2
  call void @llvm.stackrestore(ptr %savedstack)
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %cr) #2
  %agg.tmp.addr = alloca ptr, align 8
  %agg.tmp1.addr = alloca ptr, align 8
  %undef.agg.tmp2.addr = alloca ptr, align 8
  %cr.addr = alloca ptr, align 8
  store ptr %agg.tmp, ptr %agg.tmp.addr, align 8
  store ptr %agg.tmp1, ptr %agg.tmp1.addr, align 8
  store ptr %undef.agg.tmp2, ptr %undef.agg.tmp2.addr, align 8
  store ptr %cr, ptr %cr.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %cr, %"class.bb::(anonymous namespace)::cf" zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %agg.tmp, %class._ZTSZ2cqvEUlvE_ zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %agg.tmp1, %"struct.(anonymous namespace)::(anonymous namespace)::bg::bh" zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %undef.agg.tmp2, %"class.bb::(anonymous namespace)::bv" zeroinitializer, i32 1),
    "QUAL.OMP.OPERAND.ADDR"(ptr %agg.tmp, ptr %agg.tmp.addr),
    "QUAL.OMP.OPERAND.ADDR"(ptr %agg.tmp1, ptr %agg.tmp1.addr),
    "QUAL.OMP.OPERAND.ADDR"(ptr %undef.agg.tmp2, ptr %undef.agg.tmp2.addr),
    "QUAL.OMP.OPERAND.ADDR"(ptr %cr, ptr %cr.addr),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.PARALLEL.5, label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %_ZN2cl2cpEv.exit
  %cr5 = load volatile ptr, ptr %cr.addr, align 8
  %savedstack9 = call ptr @llvm.stacksave()
  %agg.tmp.i = alloca %"class.a::bp", align 1
  invoke void @_ZN2bb12_GLOBAL__N_12cf2ckEN1a2bpIvEEN12_GLOBAL__N_112_GLOBAL__N_12bg2bhE(ptr noundef nonnull align 1 dereferenceable(1) %cr5, ptr noundef nonnull %agg.tmp.i)
          to label %invoke.cont.i unwind label %lpad.i

invoke.cont.i:                                    ; preds = %DIR.OMP.PARALLEL.3
  %call.i.i8 = invoke noundef zeroext i1 @_ZN1a2bl2bmENS_2biES1_NS_2bjE(ptr noundef nonnull align 1 dereferenceable(1) %agg.tmp.i, i32 noundef 0)
          to label %_ZN2bb12_GLOBAL__N_12cf2chIZ2cqvEUlvE_EENS0_2bvET_N12_GLOBAL__N_112_GLOBAL__N_12bg2bhE.exit unwind label %terminate.lpad.i.i

terminate.lpad.i.i:                               ; preds = %invoke.cont.i
  %2 = landingpad { ptr, i32 }
          catch ptr null
  %3 = extractvalue { ptr, i32 } %2, 0
  call void @__clang_call_terminate(ptr %3) #11
  unreachable

lpad.i:                                           ; preds = %DIR.OMP.PARALLEL.3
  %4 = landingpad { ptr, i32 }
          cleanup
  %call.i = invoke noundef zeroext i1 @_ZN1a2bl2bmENS_2biES1_NS_2bjE(ptr noundef nonnull align 1 dereferenceable(1) %agg.tmp.i, i32 noundef 0)
          to label %_ZN1a2blD2Ev.exit unwind label %terminate.lpad.i

terminate.lpad.i:                                 ; preds = %lpad.i
  %5 = landingpad { ptr, i32 }
          catch ptr null
  %6 = extractvalue { ptr, i32 } %5, 0
  call void @__clang_call_terminate(ptr %6) #11
  unreachable

_ZN1a2blD2Ev.exit:                                ; preds = %lpad.i
  resume { ptr, i32 } %4

_ZN2bb12_GLOBAL__N_12cf2chIZ2cqvEUlvE_EENS0_2bvET_N12_GLOBAL__N_112_GLOBAL__N_12bg2bhE.exit: ; preds = %invoke.cont.i
  call void @llvm.stackrestore(ptr %savedstack9)
  br label %DIR.OMP.END.PARALLEL.5

DIR.OMP.END.PARALLEL.5:                           ; preds = %_ZN2cl2cpEv.exit, %_ZN2bb12_GLOBAL__N_12cf2chIZ2cqvEUlvE_EENS0_2bvET_N12_GLOBAL__N_112_GLOBAL__N_12bg2bhE.exit
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %cr) #2
  ret void
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly mustprogress nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

declare dso_local noundef i32 @_ZN1a2bdINS_2baIN2cl1lEEEE2bfEv(ptr noundef nonnull align 1 dereferenceable(1)) local_unnamed_addr #4

; Function Attrs: nofree
declare dso_local noalias ptr @__cxa_allocate_exception(i64) local_unnamed_addr #5

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local void @_ZN1a2azC1ENS_1b1jIcccEE(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #4

; Function Attrs: nofree
declare dso_local void @__cxa_free_exception(ptr) local_unnamed_addr #5

; Function Attrs: nofree noreturn
declare dso_local void @__cxa_throw(ptr, ptr, ptr) local_unnamed_addr #6

declare dso_local void @_ZN1a1b1jIcccE2axEv(ptr noundef nonnull align 1 dereferenceable(1)) local_unnamed_addr #4

declare dso_local void @_ZN1a1b1jIcccE2avC1EN2aq2arIcEEc(ptr noundef nonnull align 1 dereferenceable(1), i8 noundef signext) unnamed_addr #4

declare dso_local void @_ZN2bb12_GLOBAL__N_12cf2ckEN1a2bpIvEEN12_GLOBAL__N_112_GLOBAL__N_12bg2bhE(ptr noundef nonnull align 1 dereferenceable(1), ptr noundef) local_unnamed_addr #4

declare dso_local noundef zeroext i1 @_ZN1a2bl2bmENS_2biES1_NS_2bjE(ptr noundef nonnull align 1 dereferenceable(1), i32 noundef) local_unnamed_addr #4

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(ptr %0) local_unnamed_addr #7 comdat {
  %2 = call ptr @__cxa_begin_catch(ptr %0) #2
  call void @_ZSt9terminatev() #11
  unreachable
}

; Function Attrs: nofree
declare dso_local ptr @__cxa_begin_catch(ptr) local_unnamed_addr #5

; Function Attrs: nofree noreturn nounwind
declare dso_local void @_ZSt9terminatev() local_unnamed_addr #8

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #9

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #9

attributes #0 = { uwtable "may-have-openmp-directive"="true"}
attributes #1 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { argmemonly mustprogress nocallback nofree nounwind willreturn writeonly }
attributes #4 = { willreturn }
attributes #5 = { nofree }
attributes #6 = { nofree noreturn }
attributes #7 = { noinline noreturn nounwind }
attributes #8 = { nofree noreturn nounwind }
attributes #9 = { nocallback nofree nosync nounwind willreturn }
attributes #10 = { noreturn }
attributes #11 = { noreturn nounwind }

