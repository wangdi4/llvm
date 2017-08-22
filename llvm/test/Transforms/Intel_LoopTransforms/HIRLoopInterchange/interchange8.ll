;  for  i1 ; for i2, for i3,
;     a1_v[i1] = 1;   
;  Interchange should not occur.  
;    
; REQUIRES: asserts
; RUN: opt -hir-ssa-deconstruction -debug -hir-loop-interchange  < %s 2>&1 | FileCheck %s
; CHECK-NOT: Interchanged


;Module Before HIR; ModuleID = 'interchange8.cpp'
source_filename = "interchange8.cpp"
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

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external global i8
@g_s = local_unnamed_addr global i32 0, align 4
@main_v_tpw = local_unnamed_addr global i32 0, align 4
@main_v_yc = local_unnamed_addr global i32 0, align 4
@main_v_lbmz = local_unnamed_addr global i32 0, align 4
@a1_v = local_unnamed_addr global [92 x i32] zeroinitializer, align 16
@_ZSt4cout = external global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr constant [2 x i8] c" \00", align 1
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_interchange8.cpp, i8* null }]

declare void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) unnamed_addr #0

declare void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) unnamed_addr #0

; Function Attrs: nounwind
declare i32 @__cxa_atexit(void (i8*)*, i8*, i8*) local_unnamed_addr #1

; Function Attrs: norecurse uwtable
define i32 @main() local_unnamed_addr #2 {
entry:
  store i32 1, i32* @main_v_tpw, align 4, !tbaa !1
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc10, %entry
  %indvars.iv48 = phi i64 [ 1, %entry ], [ %indvars.iv.next49, %for.inc10 ]
  %arrayidx = getelementptr inbounds [92 x i32], [92 x i32]* @a1_v, i64 0, i64 %indvars.iv48
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc7, %for.cond1.preheader
  %inc843 = phi i32 [ 0, %for.cond1.preheader ], [ %inc8, %for.inc7 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %inc39 = phi i32 [ 2, %for.cond4.preheader ], [ %inc, %for.body6 ]
  store i32 1, i32* %arrayidx, align 4, !tbaa !5
  %inc = add nuw nsw i32 %inc39, 1
  %exitcond = icmp eq i32 %inc, 51
  br i1 %exitcond, label %for.inc7, label %for.body6

for.inc7:                                         ; preds = %for.body6
  %inc8 = add nuw nsw i32 %inc843, 1
  %exitcond47 = icmp eq i32 %inc8, 64
  br i1 %exitcond47, label %for.inc10, label %for.cond4.preheader

for.inc10:                                        ; preds = %for.inc7
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond50 = icmp eq i64 %indvars.iv.next49, 64
  br i1 %exitcond50, label %for.end12, label %for.cond1.preheader

for.end12:                                        ; preds = %for.inc10
  %0 = load i32, i32* getelementptr inbounds ([92 x i32], [92 x i32]* @a1_v, i64 0, i64 0), align 16, !tbaa !5
  store i32 64, i32* @main_v_tpw, align 4, !tbaa !1
  store i32 51, i32* @main_v_lbmz, align 4, !tbaa !1
  store i32 64, i32* @main_v_yc, align 4, !tbaa !1
  store i32 0, i32* @g_s, align 4, !tbaa !1
  %conv.i51 = zext i32 %0 to i64
  %call.i2952 = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo9_M_insertImEERSoT_(%"class.std::basic_ostream"* nonnull @_ZSt4cout, i64 %conv.i51)
  %call1.i3053 = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) %call.i2952, i8* nonnull getelementptr inbounds ([2 x i8], [2 x i8]* @.str, i64 0, i64 0), i64 1)
  br label %for.body15.for.body15_crit_edge

for.cond.cleanup:                                 ; preds = %for.body15.for.body15_crit_edge
  %vtable.i = load i8*, i8** bitcast (%"class.std::basic_ostream"* @_ZSt4cout to i8**), align 8, !tbaa !7
  %vbase.offset.ptr.i = getelementptr i8, i8* %vtable.i, i64 -24
  %1 = bitcast i8* %vbase.offset.ptr.i to i64*
  %vbase.offset.i = load i64, i64* %1, align 8
  %add.ptr.i = getelementptr inbounds i8, i8* bitcast (%"class.std::basic_ostream"* @_ZSt4cout to i8*), i64 %vbase.offset.i
  %_M_ctype.i = getelementptr inbounds i8, i8* %add.ptr.i, i64 240
  %2 = bitcast i8* %_M_ctype.i to %"class.std::ctype"**
  %3 = load %"class.std::ctype"*, %"class.std::ctype"** %2, align 8, !tbaa !9
  %tobool.i34 = icmp eq %"class.std::ctype"* %3, null
  br i1 %tobool.i34, label %if.then.i35, label %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit

if.then.i35:                                      ; preds = %for.cond.cleanup
  tail call void @_ZSt16__throw_bad_castv() #7
  unreachable

_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit:    ; preds = %for.cond.cleanup
  %_M_widen_ok.i = getelementptr inbounds %"class.std::ctype", %"class.std::ctype"* %3, i64 0, i32 8
  %4 = load i8, i8* %_M_widen_ok.i, align 8, !tbaa !17
  %tobool.i = icmp eq i8 %4, 0
  br i1 %tobool.i, label %if.end.i, label %if.then.i

if.then.i:                                        ; preds = %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit
  %arrayidx.i = getelementptr inbounds %"class.std::ctype", %"class.std::ctype"* %3, i64 0, i32 9, i64 10
  %5 = load i8, i8* %arrayidx.i, align 1, !tbaa !23
  br label %_ZNKSt5ctypeIcE5widenEc.exit

if.end.i:                                         ; preds = %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit
  tail call void @_ZNKSt5ctypeIcE13_M_widen_initEv(%"class.std::ctype"* nonnull %3)
  %6 = bitcast %"class.std::ctype"* %3 to i8 (%"class.std::ctype"*, i8)***
  %vtable.i32 = load i8 (%"class.std::ctype"*, i8)**, i8 (%"class.std::ctype"*, i8)*** %6, align 8, !tbaa !7
  %vfn.i = getelementptr inbounds i8 (%"class.std::ctype"*, i8)*, i8 (%"class.std::ctype"*, i8)** %vtable.i32, i64 6
  %7 = load i8 (%"class.std::ctype"*, i8)*, i8 (%"class.std::ctype"*, i8)** %vfn.i, align 8
  %call.i33 = tail call signext i8 %7(%"class.std::ctype"* nonnull %3, i8 signext 10)
  br label %_ZNKSt5ctypeIcE5widenEc.exit

_ZNKSt5ctypeIcE5widenEc.exit:                     ; preds = %if.then.i, %if.end.i
  %retval.0.i = phi i8 [ %5, %if.then.i ], [ %call.i33, %if.end.i ]
  %call1.i = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo3putEc(%"class.std::basic_ostream"* nonnull @_ZSt4cout, i8 signext %retval.0.i)
  %call.i = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo5flushEv(%"class.std::basic_ostream"* nonnull %call1.i)
  %8 = load i32, i32* getelementptr inbounds ([92 x i32], [92 x i32]* @a1_v, i64 0, i64 64), align 16, !tbaa !5
  ret i32 %8

for.body15.for.body15_crit_edge:                  ; preds = %for.end12, %for.body15.for.body15_crit_edge
  %indvars.iv.next54 = phi i64 [ 1, %for.end12 ], [ %indvars.iv.next, %for.body15.for.body15_crit_edge ]
  %arrayidx17.phi.trans.insert = getelementptr inbounds [92 x i32], [92 x i32]* @a1_v, i64 0, i64 %indvars.iv.next54
  %.pre = load i32, i32* %arrayidx17.phi.trans.insert, align 4, !tbaa !5
  %conv.i = zext i32 %.pre to i64
  %call.i29 = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo9_M_insertImEERSoT_(%"class.std::basic_ostream"* nonnull @_ZSt4cout, i64 %conv.i)
  %call1.i30 = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) %call.i29, i8* nonnull getelementptr inbounds ([2 x i8], [2 x i8]* @.str, i64 0, i64 0), i64 1)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv.next54, 1
  %cmp14 = icmp eq i64 %indvars.iv.next, 92
  br i1 %cmp14, label %for.cond.cleanup, label %for.body15.for.body15_crit_edge
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #3

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo9_M_insertImEERSoT_(%"class.std::basic_ostream"*, i64) local_unnamed_addr #0

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* dereferenceable(272), i8*, i64) local_unnamed_addr #0

declare void @_ZNSt9basic_iosIcSt11char_traitsIcEE5clearESt12_Ios_Iostate(%"class.std::basic_ios"*, i32) local_unnamed_addr #0

; Function Attrs: nounwind readonly
declare i64 @strlen(i8* nocapture) local_unnamed_addr #4

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo3putEc(%"class.std::basic_ostream"*, i8 signext) local_unnamed_addr #0

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo5flushEv(%"class.std::basic_ostream"*) local_unnamed_addr #0

; Function Attrs: noreturn
declare void @_ZSt16__throw_bad_castv() local_unnamed_addr #5

declare void @_ZNKSt5ctypeIcE13_M_widen_initEv(%"class.std::ctype"*) local_unnamed_addr #0

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_interchange8.cpp() #6 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull @_ZSt8__ioinit)
  %0 = tail call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZSt8__ioinit, i64 0, i32 0), i8* nonnull @__dso_handle) #1
  ret void
}

attributes #0 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { norecurse uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }
attributes #4 = { nounwind readonly "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { noreturn "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { noreturn }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 17939) (llvm/branches/loopopt 17975)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = !{!6, !2, i64 0}
!6 = !{!"array@_ZTSA92_j", !2, i64 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"vtable pointer", !4, i64 0}
!9 = !{!10, !14, i64 240}
!10 = !{!"struct@_ZTSSt9basic_iosIcSt11char_traitsIcEE", !11, i64 216, !3, i64 224, !12, i64 225, !13, i64 232, !14, i64 240, !15, i64 248, !16, i64 256}
!11 = !{!"pointer@_ZTSPSo", !3, i64 0}
!12 = !{!"bool", !3, i64 0}
!13 = !{!"pointer@_ZTSPSt15basic_streambufIcSt11char_traitsIcEE", !3, i64 0}
!14 = !{!"pointer@_ZTSPKSt5ctypeIcE", !3, i64 0}
!15 = !{!"pointer@_ZTSPKSt7num_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE", !3, i64 0}
!16 = !{!"pointer@_ZTSPKSt7num_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEE", !3, i64 0}
!17 = !{!18, !3, i64 56}
!18 = !{!"struct@_ZTSSt5ctypeIcE", !19, i64 16, !12, i64 24, !20, i64 32, !20, i64 40, !21, i64 48, !3, i64 56, !22, i64 57, !22, i64 313, !3, i64 569}
!19 = !{!"pointer@_ZTSP15__locale_struct", !3, i64 0}
!20 = !{!"pointer@_ZTSPKi", !3, i64 0}
!21 = !{!"pointer@_ZTSPKt", !3, i64 0}
!22 = !{!"array@_ZTSA256_c", !3, i64 0}
!23 = !{!18, !3, i64 57}
