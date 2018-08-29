; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-temp-cleanup -hir-pm-symbolic-tripcount-completeunroll -print-before=hir-pm-symbolic-tripcount-completeunroll -print-after=hir-pm-symbolic-tripcount-completeunroll -disable-output < %s 2>&1 | FileCheck %s
;
; TODO: fix this test.
; XFAIL: *
;
; *** Source Code ***
;// remove_neighbour() function inside cpu2017/541.leela/FastBoard.cpp, BEFORE pattern matching
;void FastBoard::remove_neighbour(const int i, const int color) {
;  assert(color == WHITE || color == BLACK || color == EMPTY);
;
;  std::tr1::array<int, 4> nbr_pars;
;  int nbr_par_cnt = 0;
; 
;  for (int k = 0; k < 4; k++) {
;    int ai = i + m_dirs[k];
;
;    m_neighbours[ai] += (1 << (NBR_SHIFT * EMPTY)) - (1 << (NBR_SHIFT * color));
;
;    bool found = false;
;    for (int i = 0; i < nbr_par_cnt; i++) {
;      if (nbr_pars[i] == m_parent[ai]) {
;        found = true;
;        break;
;      }
;    }
;    if (!found) {
;      m_libs[m_parent[ai]]++;
;      nbr_pars[nbr_par_cnt++] = m_parent[ai];
;    }
;  }
;}
;
;// remove_neighbour() function inside cpu2017/541.leela/FastBoard.cpp, AFTER pattern matching
;void FastBoard::remove_neighbour(const int i, const int color) {       
;  assert(color == WHITE || color == BLACK || color == EMPTY);
;
;  std::tr1::array<int, 4> nbr_pars;
;  int nbr_par_cnt = 0;
;
;  //k=0:
;  int ai = i + m_dirs[0];
;  m_neighbours[ai] += (1 << (NBR_SHIFT * EMPTY)) - (1 << (NBR_SHIFT * color));
;  m_libs[m_parent[ai]]++;
;
;  //k=1:
;  int ai1 = i + m_dirs[1];
;  m_neighbours[ai1] += (1 << (NBR_SHIFT * EMPTY)) - (1 << (NBR_SHIFT * color));
;  m_libs[m_parent[ai1]]++;
;
;  //k=2:
;  int ai2 = i + m_dirs[2];
;  m_neighbours[ai2] += (1 << (NBR_SHIFT * EMPTY)) - (1 << (NBR_SHIFT * color));
;  m_libs[m_parent[ai2]]++;
;
;  //k=3:
;  int ai3 = i + m_dirs[3];
;  m_neighbours[ai3] += (1 << (NBR_SHIFT * EMPTY)) - (1 << (NBR_SHIFT * color));
;  m_libs[m_parent[ai3]]++;
;}
;
;
;
; CHECK: IR Dump Before HIR Symbolic TripCount CompleteUnroll Pattern Match Pass
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:        |   %nbr_par_cnt.061.out = %nbr_par_cnt.061;
; CHECK:        |   %2 = (%this)[0].12.0[i1];
; CHECK:        |   %3 = (%this)[0].10.0[%2 + %i];
; CHECK:        |   (%this)[0].10.0[%2 + %i] = %3 + trunc.i32.i16(%shl.neg) + 256;
; CHECK:        |   %5 = (%this)[0].7.0[%2 + %i];
; CHECK:        |   if (%nbr_par_cnt.061 > 0)
; CHECK:        |   {
; CHECK:        |      + DO i2 = 0, smax(1, sext.i32.i64(%nbr_par_cnt.061)) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 4>
; CHECK:        |      |   if ((%nbr_pars)[0].0[i2] == %5)
; CHECK:        |      |   {
; CHECK:        |      |      goto if.end32.loopexit;
; CHECK:        |      |   }
; CHECK:        |      + END LOOP
; CHECK:        |      
; CHECK:        |      goto if.then18;
; CHECK:        |      if.end32.loopexit:
; CHECK:        |      goto if.end32;
; CHECK:        |   }
; CHECK:        |   if.then18:
; CHECK:        |   %8 = (%this)[0].8.0[%5];
; CHECK:        |   (%this)[0].8.0[%5] = %8 + 1;
; CHECK:        |   %9 = (%this)[0].7.0[%2 + %i];
; CHECK:        |   %nbr_par_cnt.061 = %nbr_par_cnt.061  +  1;
; CHECK:        |   (%nbr_pars)[0].0[%nbr_par_cnt.061.out] = %9;
; CHECK:        |   if.end32:
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;  
; CHECK: IR Dump After HIR Symbolic TripCount CompleteUnroll Pattern Match Pass
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        %2 = (%this)[0].12.0[0];
; CHECK:        %3 = (%this)[0].10.0[%2 + %i];
; CHECK:        (%this)[0].10.0[%2 + %i] = %3 + trunc.i32.i16(%shl.neg) + 256;
; CHECK:        %5 = (%this)[0].7.0[%2 + %i];
; CHECK:        %8 = (%this)[0].8.0[%5];
;
; CHECK:        %mv = (%this)[0].12.0[1];
; CHECK:        %mv2 = (%this)[0].10.0[%i + %mv];
; CHECK:        (%this)[0].10.0[%i + %mv] = trunc.i32.i16(%shl.neg) + %mv2 + 256;
; CHECK:        %mv3 = (%this)[0].7.0[%i + %mv];
; CHECK:        %mv4 = (%this)[0].8.0[%mv3];
;
; CHECK:        %mv5 = (%this)[0].12.0[2];
; CHECK:        %mv6 = (%this)[0].10.0[%i + %mv5];
; CHECK:        (%this)[0].10.0[%i + %mv5] = trunc.i32.i16(%shl.neg) + %mv6 + 256;
; CHECK:        %mv7 = (%this)[0].7.0[%i + %mv5];
; CHECK:        %mv8 = (%this)[0].8.0[%mv7];
;
; CHECK:        %mv9 = (%this)[0].12.0[3];
; CHECK:        %mv10 = (%this)[0].10.0[%i + %mv9];
; CHECK:        (%this)[0].10.0[%i + %mv9] = trunc.i32.i16(%shl.neg) + %mv10 + 256;
; CHECK:        %mv11 = (%this)[0].7.0[%i + %mv9];
; CHECK:        %mv12 = (%this)[0].8.0[%mv11];
;
; CHECK:        (%this)[0].8.0[%5] = %8 + 1;
; CHECK:        (%this)[0].8.0[%mv3] = %mv4 + 1;
; CHECK:        (%this)[0].8.0[%mv7] = %mv8 + 1;
; CHECK:        (%this)[0].8.0[%mv11] = %mv12 + 1;
; CHECK:  END REGION
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
; ModuleID = '<stdin>'
source_filename = "Test1.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%class.FastBoard = type <{ %"class.boost::array", %"class.boost::array", i32, i32, i32, %"class.boost::array.0", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array", [2 x i8], %"class.boost::array.2", %"class.boost::array.3", %"class.boost::array.4", %"class.boost::array.4", %"class.std::vector", i32, [4 x i8] }>
%"class.boost::array.0" = type { [441 x i32] }
%"class.boost::array.1" = type { [442 x i16] }
%"class.boost::array" = type { [441 x i16] }
%"class.boost::array.2" = type { [4 x i32] }
%"class.boost::array.3" = type { [8 x i32] }
%"class.boost::array.4" = type { [2 x i32] }
%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<int, std::allocator<int> >::_Vector_impl" }
%"struct.std::_Vector_base<int, std::allocator<int> >::_Vector_impl" = type { i32*, i32*, i32* }

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_Test1.cpp, i8* null }]

declare void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) unnamed_addr #0

declare void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) unnamed_addr #0

; Function Attrs: nounwind
declare i32 @__cxa_atexit(void (i8*)*, i8*, i8*) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define void @_ZN9FastBoard16remove_neighbourEii(%class.FastBoard* %this, i32 %i, i32 %color) local_unnamed_addr #2 align 2 {
entry:
  %nbr_pars = alloca %"class.boost::array.2", align 4
  %0 = bitcast %"class.boost::array.2"* %nbr_pars to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %0) #1
  %mul = shl nsw i32 %color, 2
  %1 = and i32 %mul, 28
  %shl.neg = shl i32 -1, %1
  %sub = add nsw i32 %shl.neg, 256
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end32
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %0) #1
  ret void

for.body:                                         ; preds = %if.end32, %entry
  %indvars.iv64 = phi i64 [ 0, %entry ], [ %indvars.iv.next65, %if.end32 ]
  %nbr_par_cnt.061 = phi i32 [ 0, %entry ], [ %nbr_par_cnt.1, %if.end32 ]
  %arrayidx.i = getelementptr inbounds %class.FastBoard, %class.FastBoard* %this, i64 0, i32 12, i32 0, i64 %indvars.iv64
  %2 = load i32, i32* %arrayidx.i, align 4, !tbaa !1
  %add = add nsw i32 %2, %i
  %conv2 = sext i32 %add to i64
  %arrayidx.i56 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %this, i64 0, i32 10, i32 0, i64 %conv2
  %3 = load i16, i16* %arrayidx.i56, align 2, !tbaa !7
  %conv4 = zext i16 %3 to i32
  %add5 = add nsw i32 %sub, %conv4
  %conv6 = trunc i32 %add5 to i16
  store i16 %conv6, i16* %arrayidx.i56, align 2, !tbaa !7
  %cmp958 = icmp sgt i32 %nbr_par_cnt.061, 0
  %4 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %this, i64 0, i32 7, i32 0, i64 %conv2
  %5 = load i16, i16* %4, align 2, !tbaa !11
  %6 = sext i32 %nbr_par_cnt.061 to i64
  br i1 %cmp958, label %for.body11.preheader, label %if.then18

for.body11.preheader:                             ; preds = %for.body
  %conv16 = zext i16 %5 to i32
  br label %for.body11

for.cond8:                                        ; preds = %for.body11
  %cmp9 = icmp slt i64 %indvars.iv.next, %6
  br i1 %cmp9, label %for.body11, label %if.then18.loopexit

for.body11:                                       ; preds = %for.cond8, %for.body11.preheader
  %indvars.iv = phi i64 [ 0, %for.body11.preheader ], [ %indvars.iv.next, %for.cond8 ]
  %arrayidx.i55 = getelementptr inbounds %"class.boost::array.2", %"class.boost::array.2"* %nbr_pars, i64 0, i32 0, i64 %indvars.iv
  %7 = load i32, i32* %arrayidx.i55, align 4, !tbaa !1
  %cmp17 = icmp eq i32 %7, %conv16
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br i1 %cmp17, label %if.end32.loopexit, label %for.cond8

if.then18.loopexit:                               ; preds = %for.cond8
  br label %if.then18

if.then18:                                        ; preds = %if.then18.loopexit, %for.body
  %conv22 = zext i16 %5 to i64
  %arrayidx.i52 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %this, i64 0, i32 8, i32 0, i64 %conv22
  %8 = load i16, i16* %arrayidx.i52, align 2, !tbaa !11
  %inc24 = add i16 %8, 1
  store i16 %inc24, i16* %arrayidx.i52, align 2, !tbaa !11
  %9 = load i16, i16* %4, align 2, !tbaa !11
  %conv28 = zext i16 %9 to i32
  %inc29 = add nsw i32 %nbr_par_cnt.061, 1
  %arrayidx.i50 = getelementptr inbounds %"class.boost::array.2", %"class.boost::array.2"* %nbr_pars, i64 0, i32 0, i64 %6
  store i32 %conv28, i32* %arrayidx.i50, align 4, !tbaa !1
  br label %if.end32

if.end32.loopexit:                                ; preds = %for.body11
  br label %if.end32

if.end32:                                         ; preds = %if.end32.loopexit, %if.then18
  %nbr_par_cnt.1 = phi i32 [ %inc29, %if.then18 ], [ %nbr_par_cnt.061, %if.end32.loopexit ]
  %indvars.iv.next65 = add nuw nsw i64 %indvars.iv64, 1
  %exitcond = icmp eq i64 %indvars.iv.next65, 4
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #3

; Function Attrs: nounwind readnone
declare i32* @llvm.intel.fakeload.p0i32(i32*, metadata) #4

; Function Attrs: nounwind readnone
declare i16* @llvm.intel.fakeload.p0i16(i16*, metadata) #4

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_Test1.cpp() #5 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull @_ZSt8__ioinit)
  %0 = tail call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZSt8__ioinit, i64 0, i32 0), i8* nonnull @__dso_handle) #1
  ret void
}

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }
attributes #4 = { nounwind readnone }
attributes #5 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
!1 = !{!2, !4, i64 0}
!2 = !{!"struct@_ZTSN5boost5arrayIiLm4EEE", !3, i64 0}
!3 = !{!"array@_ZTSA4_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !10, i64 0}
!8 = !{!"struct@_ZTSN5boost5arrayItLm441EEE", !9, i64 0}
!9 = !{!"array@_ZTSA441_t", !10, i64 0}
!10 = !{!"short", !5, i64 0}
!11 = !{!12, !10, i64 0}
!12 = !{!"struct@_ZTSN5boost5arrayItLm442EEE", !13, i64 0}
!13 = !{!"array@_ZTSA442_t", !10, i64 0}
