; Sanity Test(s) on HIR Loop Reversal: from llvmtestCpp/ary2.cpp
; 
; l1reversal-ary2.ll:
; 1-level loop, sanity test, invalid reversal case1
; 
; [REASONS]
; - PreliminaryCheck: fail 
;   . UBCE's Denominator is 10, not 1;  
; - Applicalbe: N/A  
; - Profitable: N/A
; - Legal:      N/A
;
; [SUGGESTION]
; - may allow UBCE's non-uniform denominator, so enable Reversal on the loop
;   
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;
;
; [AFTER LOOP REVERSAL]
;
; 
; ===-----------------------------------===
; *** Run0: BEFORE HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=BEFORE 
;
; ===-----------------------------------===
; *** Run1: AFTER HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-after=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=AFTER 
;
; === -------------------------------------- ===
; *** Tests0: W/O HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output before Loop Reversal
; 
; BEFORE:    BEGIN REGION { }
; BEFORE:       + DO i1 = 0, (%cond323330 + smax(-11, (-1 + (-1 * %cond323330))) + 10)/u10, 1   <DO_LOOP>
; BEFORE:       |   %12 = (%1)[-10 * i1 + %cond323330 + -1];
; BEFORE:       |   (%2)[-10 * i1 + %cond323330 + -1] = %12;
; BEFORE:       |   %13 = (%1)[-10 * i1 + %cond323330 + -2];
; BEFORE:       |   (%2)[-10 * i1 + %cond323330 + -2] = %13;
; BEFORE:       |   %14 = (%1)[-10 * i1 + %cond323330 + -3];
; BEFORE:       |   (%2)[-10 * i1 + %cond323330 + -3] = %14;
; BEFORE:       |   %15 = (%1)[-10 * i1 + %cond323330 + -4];
; BEFORE:       |   (%2)[-10 * i1 + %cond323330 + -4] = %15;
; BEFORE:       |   %16 = (%1)[-10 * i1 + %cond323330 + -5];
; BEFORE:       |   (%2)[-10 * i1 + %cond323330 + -5] = %16;
; BEFORE:       |   %17 = (%1)[-10 * i1 + %cond323330 + -6];
; BEFORE:       |   (%2)[-10 * i1 + %cond323330 + -6] = %17;
; BEFORE:       |   %18 = (%1)[-10 * i1 + %cond323330 + -7];
; BEFORE:       |   (%2)[-10 * i1 + %cond323330 + -7] = %18;
; BEFORE:       |   %19 = (%1)[-10 * i1 + %cond323330 + -8];
; BEFORE:       |   (%2)[-10 * i1 + %cond323330 + -8] = %19;
; BEFORE:       |   %20 = (%1)[-10 * i1 + %cond323330 + -9];
; BEFORE:       |   (%2)[-10 * i1 + %cond323330 + -9] = %20;
; BEFORE:       |   %21 = (%1)[-10 * i1 + %cond323330 + -10];
; BEFORE:       |   (%2)[-10 * i1 + %cond323330 + -10] = %21;
; BEFORE:       + END LOOP
; BEFORE:    END REGION
;
;
;
; === -------------------------------------- ===
; *** Tests1: Run HIR Loop Reversal, But DIDN'T DO REVERSAL, Output ***
; === -------------------------------------- ===
;
; AFTER:    BEGIN REGION { }
; AFTER:       + DO i1 = 0, (%cond323330 + smax(-11, (-1 + (-1 * %cond323330))) + 10)/u10, 1   <DO_LOOP>
; AFTER:       |   %12 = (%1)[-10 * i1 + %cond323330 + -1];
; AFTER:       |   (%2)[-10 * i1 + %cond323330 + -1] = %12;
; AFTER:       |   %13 = (%1)[-10 * i1 + %cond323330 + -2];
; AFTER:       |   (%2)[-10 * i1 + %cond323330 + -2] = %13;
; AFTER:       |   %14 = (%1)[-10 * i1 + %cond323330 + -3];
; AFTER:       |   (%2)[-10 * i1 + %cond323330 + -3] = %14;
; AFTER:       |   %15 = (%1)[-10 * i1 + %cond323330 + -4];
; AFTER:       |   (%2)[-10 * i1 + %cond323330 + -4] = %15;
; AFTER:       |   %16 = (%1)[-10 * i1 + %cond323330 + -5];
; AFTER:       |   (%2)[-10 * i1 + %cond323330 + -5] = %16;
; AFTER:       |   %17 = (%1)[-10 * i1 + %cond323330 + -6];
; AFTER:       |   (%2)[-10 * i1 + %cond323330 + -6] = %17;
; AFTER:       |   %18 = (%1)[-10 * i1 + %cond323330 + -7];
; AFTER:       |   (%2)[-10 * i1 + %cond323330 + -7] = %18;
; AFTER:       |   %19 = (%1)[-10 * i1 + %cond323330 + -8];
; AFTER:       |   (%2)[-10 * i1 + %cond323330 + -8] = %19;
; AFTER:       |   %20 = (%1)[-10 * i1 + %cond323330 + -9];
; AFTER:       |   (%2)[-10 * i1 + %cond323330 + -9] = %20;
; AFTER:       |   %21 = (%1)[-10 * i1 + %cond323330 + -10];
; AFTER:       |   (%2)[-10 * i1 + %cond323330 + -10] = %21;
; AFTER:       + END LOOP
; AFTER:    END REGION
;
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
; ModuleID = '<stdin>'
source_filename = "ary2.cpp"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%"class.std::basic_ostream" = type { i32 (...)**, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_ostream"*, i8, i8, %"class.std::basic_streambuf"*, %"class.std::ctype"*, %"class.std::num_put"*, %"class.std::num_get"* }
%"class.std::ios_base" = type { i32 (...)**, i32, i32, i32, i32, i32, %"struct.std::ios_base::_Callback_list"*, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, %"struct.std::ios_base::_Words"*, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { %"struct.std::ios_base::_Callback_list"*, void (i32, %"class.std::ios_base"*, i32)*, i32, i32 }
%"struct.std::ios_base::_Words" = type { i8*, i32 }
%"class.std::locale" = type { %"class.std::locale::_Impl"* }
%"class.std::locale::_Impl" = type { i32, %"class.std::locale::facet"**, i32, %"class.std::locale::facet"**, i8** }
%"class.std::locale::facet" = type { i32 (...)**, i32 }
%"class.std::basic_streambuf" = type { i32 (...)**, i8*, i8*, i8*, i8*, i8*, i8*, %"class.std::locale" }
%"class.std::ctype" = type <{ %"class.std::locale::facet", %struct.__locale_struct*, i8, [3 x i8], i32*, i32*, i16*, i8, [256 x i8], [256 x i8], i8, [2 x i8] }>
%struct.__locale_struct = type { [13 x %struct.__locale_data*], i16*, i32*, i32*, [13 x i8*] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet" }
%"class.std::num_get" = type { %"class.std::locale::facet" }

@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external global i8
@_ZSt4cout = external global %"class.std::basic_ostream", align 4
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_ary2.cpp, i8* null }]

declare void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) unnamed_addr #0

; Function Attrs: nounwind
declare void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) unnamed_addr #1

; Function Attrs: nounwind
declare i32 @__cxa_atexit(void (i8*)*, i8*, i8*) #2

; Function Attrs: norecurse
define i32 @main(i32 %argc, i8** nocapture readonly %argv) #3 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %cmp = icmp eq i32 %argc, 2
  br i1 %cmp, label %cond.end, label %for.body.i.i.preheader.i.i.i.i.i

cond.end:                                         ; preds = %entry
  %arrayidx = getelementptr inbounds i8*, i8** %argv, i32 1
  %0 = load i8*, i8** %arrayidx, align 4, !tbaa !1
  %call.i = tail call i32 @strtol(i8* nocapture nonnull %0, i8** null, i32 10) #2
  %phitmp = mul i32 %call.i, 10
  %cmp.i.i.i.i.i = icmp ugt i32 %phitmp, 1073741823
  br i1 %cmp.i.i.i.i.i, label %if.then.i.i.i.i.i, label %for.body.i.i.preheader.i.i.i.i.i

if.then.i.i.i.i.i:                                ; preds = %cond.end
  invoke void @_ZSt17__throw_bad_allocv() #9
          to label %.noexc unwind label %lpad

.noexc:                                           ; preds = %if.then.i.i.i.i.i
  unreachable

for.body.i.i.preheader.i.i.i.i.i:                 ; preds = %cond.end, %entry
  %cond323330 = phi i32 [ %phitmp, %cond.end ], [ 9000000, %entry ]
  %mul.i.i.i.i.i = shl i32 %cond323330, 2
  %call2.i.i.i6.i.i170 = invoke i8* @_Znwj(i32 %mul.i.i.i.i.i)
          to label %for.body.i.i.preheader.i.i.i.i.i185 unwind label %lpad

for.body.i.i.preheader.i.i.i.i.i185:              ; preds = %for.body.i.i.preheader.i.i.i.i.i
  tail call void @llvm.memset.p0i8.i32(i8* nonnull %call2.i.i.i6.i.i170, i8 0, i32 %mul.i.i.i.i.i, i32 4, i1 false) #2
  %call2.i.i.i6.i.i188 = invoke i8* @_Znwj(i32 %mul.i.i.i.i.i)
          to label %for.body.preheader unwind label %ehcleanup97.thread

for.body.preheader:                               ; preds = %for.body.i.i.preheader.i.i.i.i.i185
  %1 = bitcast i8* %call2.i.i.i6.i.i170 to i32*
  %2 = bitcast i8* %call2.i.i.i6.i.i188 to i32*
  %add.ptr.i.i.i184 = getelementptr inbounds i32, i32* %2, i32 %cond323330
  tail call void @llvm.memset.p0i8.i32(i8* nonnull %call2.i.i.i6.i.i188, i8 0, i32 %mul.i.i.i.i.i, i32 4, i1 false) #2
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %i.0356 = phi i32 [ %inc34, %for.body ], [ 0, %for.body.preheader ]
  %add.ptr.i = getelementptr inbounds i32, i32* %1, i32 %i.0356
  store i32 %i.0356, i32* %add.ptr.i, align 4, !tbaa !5
  %inc = or i32 %i.0356, 1
  %add.ptr.i195 = getelementptr inbounds i32, i32* %1, i32 %inc
  store i32 %inc, i32* %add.ptr.i195, align 4, !tbaa !5
  %inc10 = add nuw nsw i32 %i.0356, 2
  %add.ptr.i206 = getelementptr inbounds i32, i32* %1, i32 %inc10
  store i32 %inc10, i32* %add.ptr.i206, align 4, !tbaa !5
  %inc13 = add nuw nsw i32 %i.0356, 3
  %add.ptr.i208 = getelementptr inbounds i32, i32* %1, i32 %inc13
  store i32 %inc13, i32* %add.ptr.i208, align 4, !tbaa !5
  %inc16 = add nuw nsw i32 %i.0356, 4
  %add.ptr.i217 = getelementptr inbounds i32, i32* %1, i32 %inc16
  store i32 %inc16, i32* %add.ptr.i217, align 4, !tbaa !5
  %inc19 = add nuw nsw i32 %i.0356, 5
  %add.ptr.i219 = getelementptr inbounds i32, i32* %1, i32 %inc19
  store i32 %inc19, i32* %add.ptr.i219, align 4, !tbaa !5
  %inc22 = add nuw nsw i32 %i.0356, 6
  %add.ptr.i225 = getelementptr inbounds i32, i32* %1, i32 %inc22
  store i32 %inc22, i32* %add.ptr.i225, align 4, !tbaa !5
  %inc25 = add nuw nsw i32 %i.0356, 7
  %add.ptr.i271 = getelementptr inbounds i32, i32* %1, i32 %inc25
  store i32 %inc25, i32* %add.ptr.i271, align 4, !tbaa !5
  %inc28 = add nuw nsw i32 %i.0356, 8
  %add.ptr.i275 = getelementptr inbounds i32, i32* %1, i32 %inc28
  store i32 %inc28, i32* %add.ptr.i275, align 4, !tbaa !5
  %inc31 = add nuw nsw i32 %i.0356, 9
  %add.ptr.i273 = getelementptr inbounds i32, i32* %1, i32 %inc31
  store i32 %inc31, i32* %add.ptr.i273, align 4, !tbaa !5
  %inc34 = add nuw nsw i32 %i.0356, 10
  %cmp4 = icmp slt i32 %inc34, %cond323330
  br i1 %cmp4, label %for.body, label %for.body38.preheader

for.body38.preheader:                             ; preds = %for.body
  br label %for.body38

lpad:                                             ; preds = %for.body.i.i.preheader.i.i.i.i.i, %if.then.i.i.i.i.i
  %3 = landingpad { i8*, i32 }
          cleanup
  %4 = extractvalue { i8*, i32 } %3, 0
  %5 = extractvalue { i8*, i32 } %3, 1
  br label %ehcleanup99

ehcleanup97.thread:                               ; preds = %for.body.i.i.preheader.i.i.i.i.i185
  %6 = landingpad { i8*, i32 }
          cleanup
  %7 = extractvalue { i8*, i32 } %6, 0
  %8 = extractvalue { i8*, i32 } %6, 1
  br label %if.then.i.i.i

lpad5:                                            ; preds = %call1.i.noexc, %call.i199.noexc, %.noexc214, %if.end.i, %if.then.i221, %invoke.cont90
  %9 = landingpad { i8*, i32 }
          cleanup
  %10 = extractvalue { i8*, i32 } %9, 0
  %11 = extractvalue { i8*, i32 } %9, 1
  br i1 false, label %ehcleanup97, label %if.then.i.i.i268

if.then.i.i.i268:                                 ; preds = %lpad5
  tail call void @_ZdlPv(i8* %call2.i.i.i6.i.i188) #2
  br label %ehcleanup97

for.body38:                                       ; preds = %for.body38.preheader, %for.body38
  %i35.0.in353 = phi i32 [ %dec83, %for.body38 ], [ %cond323330, %for.body38.preheader ]
  %i35.0354 = add nsw i32 %i35.0.in353, -1
  %add.ptr.i265 = getelementptr inbounds i32, i32* %1, i32 %i35.0354
  %12 = load i32, i32* %add.ptr.i265, align 4, !tbaa !5
  %add.ptr.i263 = getelementptr inbounds i32, i32* %2, i32 %i35.0354
  store i32 %12, i32* %add.ptr.i263, align 4, !tbaa !5
  %dec = add nsw i32 %i35.0.in353, -2
  %add.ptr.i261 = getelementptr inbounds i32, i32* %1, i32 %dec
  %13 = load i32, i32* %add.ptr.i261, align 4, !tbaa !5
  %add.ptr.i259 = getelementptr inbounds i32, i32* %2, i32 %dec
  store i32 %13, i32* %add.ptr.i259, align 4, !tbaa !5
  %dec48 = add nsw i32 %i35.0.in353, -3
  %add.ptr.i257 = getelementptr inbounds i32, i32* %1, i32 %dec48
  %14 = load i32, i32* %add.ptr.i257, align 4, !tbaa !5
  %add.ptr.i255 = getelementptr inbounds i32, i32* %2, i32 %dec48
  store i32 %14, i32* %add.ptr.i255, align 4, !tbaa !5
  %dec53 = add nsw i32 %i35.0.in353, -4
  %add.ptr.i253 = getelementptr inbounds i32, i32* %1, i32 %dec53
  %15 = load i32, i32* %add.ptr.i253, align 4, !tbaa !5
  %add.ptr.i251 = getelementptr inbounds i32, i32* %2, i32 %dec53
  store i32 %15, i32* %add.ptr.i251, align 4, !tbaa !5
  %dec58 = add nsw i32 %i35.0.in353, -5
  %add.ptr.i249 = getelementptr inbounds i32, i32* %1, i32 %dec58
  %16 = load i32, i32* %add.ptr.i249, align 4, !tbaa !5
  %add.ptr.i247 = getelementptr inbounds i32, i32* %2, i32 %dec58
  store i32 %16, i32* %add.ptr.i247, align 4, !tbaa !5
  %dec63 = add nsw i32 %i35.0.in353, -6
  %add.ptr.i245 = getelementptr inbounds i32, i32* %1, i32 %dec63
  %17 = load i32, i32* %add.ptr.i245, align 4, !tbaa !5
  %add.ptr.i243 = getelementptr inbounds i32, i32* %2, i32 %dec63
  store i32 %17, i32* %add.ptr.i243, align 4, !tbaa !5
  %dec68 = add nsw i32 %i35.0.in353, -7
  %add.ptr.i241 = getelementptr inbounds i32, i32* %1, i32 %dec68
  %18 = load i32, i32* %add.ptr.i241, align 4, !tbaa !5
  %add.ptr.i239 = getelementptr inbounds i32, i32* %2, i32 %dec68
  store i32 %18, i32* %add.ptr.i239, align 4, !tbaa !5
  %dec73 = add nsw i32 %i35.0.in353, -8
  %add.ptr.i237 = getelementptr inbounds i32, i32* %1, i32 %dec73
  %19 = load i32, i32* %add.ptr.i237, align 4, !tbaa !5
  %add.ptr.i235 = getelementptr inbounds i32, i32* %2, i32 %dec73
  store i32 %19, i32* %add.ptr.i235, align 4, !tbaa !5
  %dec78 = add nsw i32 %i35.0.in353, -9
  %add.ptr.i233 = getelementptr inbounds i32, i32* %1, i32 %dec78
  %20 = load i32, i32* %add.ptr.i233, align 4, !tbaa !5
  %add.ptr.i231 = getelementptr inbounds i32, i32* %2, i32 %dec78
  store i32 %20, i32* %add.ptr.i231, align 4, !tbaa !5
  %dec83 = add nsw i32 %i35.0.in353, -10
  %add.ptr.i229 = getelementptr inbounds i32, i32* %1, i32 %dec83
  %21 = load i32, i32* %add.ptr.i229, align 4, !tbaa !5
  %add.ptr.i227 = getelementptr inbounds i32, i32* %2, i32 %dec83
  store i32 %21, i32* %add.ptr.i227, align 4, !tbaa !5
  %cmp37 = icmp sgt i32 %i35.0.in353, 10
  br i1 %cmp37, label %for.body38, label %invoke.cont90

invoke.cont90:                                    ; preds = %for.body38
  %add.ptr.i.i = getelementptr inbounds i32, i32* %add.ptr.i.i.i184, i32 -1
  %22 = load i32, i32* %add.ptr.i.i, align 4, !tbaa !5
  %call93 = invoke dereferenceable(140) %"class.std::basic_ostream"* @_ZNSolsEi(%"class.std::basic_ostream"* nonnull @_ZSt4cout, i32 %22)
          to label %invoke.cont92 unwind label %lpad5

invoke.cont92:                                    ; preds = %invoke.cont90
  %23 = bitcast %"class.std::basic_ostream"* %call93 to i8**
  %vtable.i = load i8*, i8** %23, align 4, !tbaa !7
  %vbase.offset.ptr.i = getelementptr i8, i8* %vtable.i, i32 -12
  %24 = bitcast i8* %vbase.offset.ptr.i to i32*
  %vbase.offset.i = load i32, i32* %24, align 4
  %25 = bitcast %"class.std::basic_ostream"* %call93 to i8*
  %add.ptr.i198 = getelementptr inbounds i8, i8* %25, i32 %vbase.offset.i
  %_M_ctype.i = getelementptr inbounds i8, i8* %add.ptr.i198, i32 124
  %26 = bitcast i8* %_M_ctype.i to %"class.std::ctype"**
  %27 = load %"class.std::ctype"*, %"class.std::ctype"** %26, align 4, !tbaa !9
  %tobool.i220 = icmp eq %"class.std::ctype"* %27, null
  br i1 %tobool.i220, label %if.then.i221, label %call.i209.noexc

if.then.i221:                                     ; preds = %invoke.cont92
  invoke void @_ZSt16__throw_bad_castv() #9
          to label %.noexc223 unwind label %lpad5

.noexc223:                                        ; preds = %if.then.i221
  unreachable

call.i209.noexc:                                  ; preds = %invoke.cont92
  %_M_widen_ok.i = getelementptr inbounds %"class.std::ctype", %"class.std::ctype"* %27, i32 0, i32 7
  %28 = load i8, i8* %_M_widen_ok.i, align 4, !tbaa !12
  %tobool.i = icmp eq i8 %28, 0
  br i1 %tobool.i, label %if.end.i, label %if.then.i

if.then.i:                                        ; preds = %call.i209.noexc
  %arrayidx.i = getelementptr inbounds %"class.std::ctype", %"class.std::ctype"* %27, i32 0, i32 8, i32 10
  %29 = load i8, i8* %arrayidx.i, align 1, !tbaa !14
  br label %call.i199.noexc

if.end.i:                                         ; preds = %call.i209.noexc
  invoke void @_ZNKSt5ctypeIcE13_M_widen_initEv(%"class.std::ctype"* nonnull %27)
          to label %.noexc214 unwind label %lpad5

.noexc214:                                        ; preds = %if.end.i
  %30 = bitcast %"class.std::ctype"* %27 to i8 (%"class.std::ctype"*, i8)***
  %vtable.i212 = load i8 (%"class.std::ctype"*, i8)**, i8 (%"class.std::ctype"*, i8)*** %30, align 4, !tbaa !7
  %vfn.i = getelementptr inbounds i8 (%"class.std::ctype"*, i8)*, i8 (%"class.std::ctype"*, i8)** %vtable.i212, i32 6
  %31 = load i8 (%"class.std::ctype"*, i8)*, i8 (%"class.std::ctype"*, i8)** %vfn.i, align 4
  %call.i213215 = invoke signext i8 %31(%"class.std::ctype"* nonnull %27, i8 signext 10)
          to label %call.i199.noexc unwind label %lpad5

call.i199.noexc:                                  ; preds = %.noexc214, %if.then.i
  %retval.0.i = phi i8 [ %29, %if.then.i ], [ %call.i213215, %.noexc214 ]
  %call1.i201 = invoke dereferenceable(140) %"class.std::basic_ostream"* @_ZNSo3putEc(%"class.std::basic_ostream"* nonnull %call93, i8 signext %retval.0.i)
          to label %call1.i.noexc unwind label %lpad5

call1.i.noexc:                                    ; preds = %call.i199.noexc
  %call.i203204 = invoke dereferenceable(140) %"class.std::basic_ostream"* @_ZNSo5flushEv(%"class.std::basic_ostream"* nonnull %call1.i201)
          to label %invoke.cont94 unwind label %lpad5

invoke.cont94:                                    ; preds = %call1.i.noexc
  br i1 false, label %_ZNSt6vectorIiSaIiEED2Ev.exit193, label %if.then.i.i.i192

if.then.i.i.i192:                                 ; preds = %invoke.cont94
  tail call void @_ZdlPv(i8* %call2.i.i.i6.i.i188) #2
  br label %_ZNSt6vectorIiSaIiEED2Ev.exit193

_ZNSt6vectorIiSaIiEED2Ev.exit193:                 ; preds = %if.then.i.i.i192, %invoke.cont94
  br i1 false, label %_ZNSt6vectorIiSaIiEED2Ev.exit174, label %if.then.i.i.i173

if.then.i.i.i173:                                 ; preds = %_ZNSt6vectorIiSaIiEED2Ev.exit193
  tail call void @_ZdlPv(i8* %call2.i.i.i6.i.i170) #2
  br label %_ZNSt6vectorIiSaIiEED2Ev.exit174

_ZNSt6vectorIiSaIiEED2Ev.exit174:                 ; preds = %if.then.i.i.i173, %_ZNSt6vectorIiSaIiEED2Ev.exit193
  ret i32 0

ehcleanup97:                                      ; preds = %if.then.i.i.i268, %lpad5
  br i1 false, label %ehcleanup99, label %if.then.i.i.i

if.then.i.i.i:                                    ; preds = %ehcleanup97, %ehcleanup97.thread
  %ehselector.slot.0348 = phi i32 [ %8, %ehcleanup97.thread ], [ %11, %ehcleanup97 ]
  %exn.slot.0346 = phi i8* [ %7, %ehcleanup97.thread ], [ %10, %ehcleanup97 ]
  tail call void @_ZdlPv(i8* %call2.i.i.i6.i.i170) #2
  br label %ehcleanup99

ehcleanup99:                                      ; preds = %if.then.i.i.i, %ehcleanup97, %lpad
  %exn.slot.1 = phi i8* [ %4, %lpad ], [ %10, %ehcleanup97 ], [ %exn.slot.0346, %if.then.i.i.i ]
  %ehselector.slot.1 = phi i32 [ %5, %lpad ], [ %11, %ehcleanup97 ], [ %ehselector.slot.0348, %if.then.i.i.i ]
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.1, 0
  %lpad.val101 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.1, 1
  resume { i8*, i32 } %lpad.val101
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #4

declare i32 @__gxx_personality_v0(...)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #4

declare dereferenceable(140) %"class.std::basic_ostream"* @_ZNSolsEi(%"class.std::basic_ostream"*, i32) #0

; Function Attrs: nounwind
declare i32 @strtol(i8* readonly, i8** nocapture, i32) #1

; Function Attrs: noreturn
declare void @_ZSt17__throw_bad_allocv() #5

; Function Attrs: nobuiltin
declare noalias nonnull i8* @_Znwj(i32) #6

declare void @__cxa_call_unexpected(i8*)

; Function Attrs: nobuiltin nounwind
declare void @_ZdlPv(i8*) #7

declare i8* @__cxa_begin_catch(i8*)

declare void @_ZSt9terminatev()

declare dereferenceable(140) %"class.std::basic_ostream"* @_ZNSo3putEc(%"class.std::basic_ostream"*, i8 signext) #0

declare dereferenceable(140) %"class.std::basic_ostream"* @_ZNSo5flushEv(%"class.std::basic_ostream"*) #0

; Function Attrs: noreturn
declare void @_ZSt16__throw_bad_castv() #5

declare void @_ZNKSt5ctypeIcE13_M_widen_initEv(%"class.std::ctype"*) #0

define internal void @_GLOBAL__sub_I_ary2.cpp() #8 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull @_ZStL8__ioinit)
  %0 = tail call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZStL8__ioinit, i32 0, i32 0), i8* nonnull @__dso_handle) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) #4

attributes #0 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { norecurse "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { argmemonly nounwind }
attributes #5 = { noreturn "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { nobuiltin "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { nobuiltin nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #9 = { noreturn }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 11232) (llvm/branches/loopopt 12506)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"any pointer", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"vtable pointer", !4, i64 0}
!9 = !{!10, !2, i64 124}
!10 = !{!"_ZTSSt9basic_iosIcSt11char_traitsIcEE", !2, i64 112, !3, i64 116, !11, i64 117, !2, i64 120, !2, i64 124, !2, i64 128, !2, i64 132}
!11 = !{!"bool", !3, i64 0}
!12 = !{!13, !3, i64 28}
!13 = !{!"_ZTSSt5ctypeIcE", !2, i64 8, !11, i64 12, !2, i64 16, !2, i64 20, !2, i64 24, !3, i64 28, !3, i64 29, !3, i64 285, !3, i64 541}
!14 = !{!3, !3, i64 0}
