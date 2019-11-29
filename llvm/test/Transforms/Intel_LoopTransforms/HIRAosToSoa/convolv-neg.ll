; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-aos-to-soa -print-after=hir-aos-to-soa < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-aos-to-soa,print<hir>" -aa-pipeline="basic-aa"  < %s 2>&1 | FileCheck %s

; Aos-to-Soa doesn't kick in because subscripts do not have all IV, i2 throug i4 in
; <54>               |   |   |   |   %conv26 = uitofp.i16.double((%p)[(-1 + %1 + %0) * i3 + i4].0);
; TODO: Consider to make Aos-to-Soa this loop since i4, the iv of the innermost loop, is unit-strided.

; CHECK: Function: foo
; CHECK-NOT: stacksave

;       BEGIN REGION { }
;             + DO i1 = 0, %2 + -1, 1   <DO_LOOP>
;             |      %3 = (%kernel)[0].1;
;             |   + DO i2 = 0, %0 + -1, 1   <DO_LOOP>
;             |   |   %result.sroa.0.0.copyload = (%q)[i2].0;
;             |   |   %result.sroa.6.0.copyload = (%q)[i2].1;
;             |   |   %result.sroa.9.0.copyload = (%q)[i2].2;
;             |   |   %result.sroa.9.0.lcssa = %result.sroa.9.0.copyload;
;             |   |   %result.sroa.6.0.lcssa = %result.sroa.6.0.copyload;
;             |   |   %result.sroa.0.0.lcssa = %result.sroa.0.0.copyload;
;             |   |
;             |   |      %result.sroa.0.0157 = %result.sroa.0.0.copyload;
;             |   |      %result.sroa.6.0156 = %result.sroa.6.0.copyload;
;             |   |      %result.sroa.9.0155 = %result.sroa.9.0.copyload;
;             |   |   + DO i3 = 0, %3 + -1, 1   <DO_LOOP>
;             |   |   |      %4 = (%kernel)[0].2;
;             |   |   |      %result.sroa.0.1150 = %result.sroa.0.0157;
;             |   |   |      %result.sroa.6.1149 = %result.sroa.6.0156;
;             |   |   |      %result.sroa.9.1148 = %result.sroa.9.0155;
;             |   |   |   + DO i4 = 0, %1 + -1, 1   <DO_LOOP>
;             |   |   |   |   %5 = (%4)[%1 * i3 + -1 * i4 + ((-1 + %3) * %1)];
;             |   |   |   |   %conv26 = uitofp.i16.double((%p)[(-1 + %1 + %0) * i3 + i4].0);
;             |   |   |   |   %mul27 = %5  *  %conv26;
;             |   |   |   |   %result.sroa.0.1150 = %result.sroa.0.1150  +  %mul27;
;             |   |   |   |   %conv46 = uitofp.i16.double((%p)[(-1 + %1 + %0) * i3 + i4].1);
;             |   |   |   |   %mul47 = %5  *  %conv46;
;             |   |   |   |   %result.sroa.6.1149 = %result.sroa.6.1149  +  %mul47;
;             |   |   |   |   %conv66 = uitofp.i16.double((%p)[(-1 + %1 + %0) * i3 + i4].2);
;             |   |   |   |   %mul67 = %5  *  %conv66;
;             |   |   |   |   %result.sroa.9.1148 = %result.sroa.9.1148  +  %mul67;
;             |   |   |   + END LOOP
;             |   |   |      %result.sroa.0.0157 = %result.sroa.0.1150;
;             |   |   |      %result.sroa.6.0156 = %result.sroa.6.1149;
;             |   |   |      %result.sroa.9.0155 = %result.sroa.9.1148;
;             |   |   + END LOOP
;             |   |      %result.sroa.9.0.lcssa = %result.sroa.9.0155;
;             |   |      %result.sroa.6.0.lcssa = %result.sroa.6.0156;
;             |   |      %result.sroa.0.0.lcssa = %result.sroa.0.0157;
;             |   |
;             |   |   (%q)[i2].0 = %result.sroa.0.0.lcssa;
;             |   |   (%q)[i2].1 = %result.sroa.6.0.lcssa;
;             |   |   (%q)[i2].2 = %result.sroa.9.0.lcssa;
;             |   + END LOOP
;             + END LOOP
;       END REGION

;Module Before HIR
; ModuleID = 'convolv-two.c'
source_filename = "convolv-two.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._Image = type { i64, i64, i16, i16, i16 }
%struct._KernelInfo = type { i64, i64, double* }
%struct._ShortPixelPacket = type { i16, i16, i16 }
%struct._DoublePixelPacket = type { double, double, double }

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(%struct._Image* nocapture readonly %image, %struct._KernelInfo* nocapture readonly %kernel, %struct._ShortPixelPacket* noalias nocapture readonly %p, double* noalias nocapture readnone %k, %struct._DoublePixelPacket* noalias nocapture %q) local_unnamed_addr #0 {
entry:
  %columns = getelementptr inbounds %struct._Image, %struct._Image* %image, i64 0, i32 0, !intel-tbaa !2
  %0 = load i64, i64* %columns, align 8, !tbaa !2
  %width = getelementptr inbounds %struct._KernelInfo, %struct._KernelInfo* %kernel, i64 0, i32 0, !intel-tbaa !8
  %1 = load i64, i64* %width, align 8, !tbaa !8
  %add = add i64 %0, -1
  %sub = add i64 %add, %1
  %rows = getelementptr inbounds %struct._Image, %struct._Image* %image, i64 0, i32 1, !intel-tbaa !11
  %2 = load i64, i64* %rows, align 8, !tbaa !11
  %cmp165 = icmp eq i64 %2, 0
  br i1 %cmp165, label %for.end87, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp3163 = icmp eq i64 %0, 0
  %height = getelementptr inbounds %struct._KernelInfo, %struct._KernelInfo* %kernel, i64 0, i32 1
  %cmp10147 = icmp eq i64 %1, 0
  %values = getelementptr inbounds %struct._KernelInfo, %struct._KernelInfo* %kernel, i64 0, i32 2
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc85
  %y.0166 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %inc86, %for.inc85 ]
  br i1 %cmp3163, label %for.inc85, label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader
  %3 = load i64, i64* %height, align 8, !tbaa !12
  %cmp6154 = icmp eq i64 %3, 0
  br label %for.body4

for.body4:                                        ; preds = %for.cond.cleanup, %for.body4.lr.ph
  %x.0164 = phi i64 [ 0, %for.body4.lr.ph ], [ %inc83, %for.cond.cleanup ]
  %result.sroa.0.0..sroa_idx = getelementptr inbounds %struct._DoublePixelPacket, %struct._DoublePixelPacket* %q, i64 %x.0164, i32 0
  %result.sroa.0.0.copyload = load double, double* %result.sroa.0.0..sroa_idx, align 8
  %result.sroa.6.0..sroa_idx106 = getelementptr inbounds %struct._DoublePixelPacket, %struct._DoublePixelPacket* %q, i64 %x.0164, i32 1
  %result.sroa.6.0.copyload = load double, double* %result.sroa.6.0..sroa_idx106, align 8
  %result.sroa.9.0..sroa_idx109 = getelementptr inbounds %struct._DoublePixelPacket, %struct._DoublePixelPacket* %q, i64 %x.0164, i32 2
  %result.sroa.9.0.copyload = load double, double* %result.sroa.9.0..sroa_idx109, align 8
  br i1 %cmp6154, label %for.cond.cleanup, label %for.cond8.preheader.preheader

for.cond8.preheader.preheader:                    ; preds = %for.body4
  br label %for.cond8.preheader

for.cond8.preheader:                              ; preds = %for.cond8.preheader.preheader, %for.cond.cleanup11
  %v.0158 = phi i64 [ %inc71, %for.cond.cleanup11 ], [ 0, %for.cond8.preheader.preheader ]
  %result.sroa.0.0157 = phi double [ %result.sroa.0.1.lcssa, %for.cond.cleanup11 ], [ %result.sroa.0.0.copyload, %for.cond8.preheader.preheader ]
  %result.sroa.6.0156 = phi double [ %result.sroa.6.1.lcssa, %for.cond.cleanup11 ], [ %result.sroa.6.0.copyload, %for.cond8.preheader.preheader ]
  %result.sroa.9.0155 = phi double [ %result.sroa.9.1.lcssa, %for.cond.cleanup11 ], [ %result.sroa.9.0.copyload, %for.cond8.preheader.preheader ]
  br i1 %cmp10147, label %for.cond.cleanup11, label %for.body12.lr.ph

for.body12.lr.ph:                                 ; preds = %for.cond8.preheader
  %4 = load double*, double** %values, align 8, !tbaa !13
  %reass.add = add i64 %3, %v.0158
  %reass.mul = mul i64 %reass.add, %1
  %add17 = sub i64 %reass.mul, %1
  %mul23 = mul i64 %v.0158, %sub
  br label %for.body12

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup11
  %result.sroa.9.1.lcssa.lcssa = phi double [ %result.sroa.9.1.lcssa, %for.cond.cleanup11 ]
  %result.sroa.6.1.lcssa.lcssa = phi double [ %result.sroa.6.1.lcssa, %for.cond.cleanup11 ]
  %result.sroa.0.1.lcssa.lcssa = phi double [ %result.sroa.0.1.lcssa, %for.cond.cleanup11 ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %for.body4
  %result.sroa.9.0.lcssa = phi double [ %result.sroa.9.0.copyload, %for.body4 ], [ %result.sroa.9.1.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  %result.sroa.6.0.lcssa = phi double [ %result.sroa.6.0.copyload, %for.body4 ], [ %result.sroa.6.1.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  %result.sroa.0.0.lcssa = phi double [ %result.sroa.0.0.copyload, %for.body4 ], [ %result.sroa.0.1.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  store double %result.sroa.0.0.lcssa, double* %result.sroa.0.0..sroa_idx, align 8, !tbaa !14
  store double %result.sroa.6.0.lcssa, double* %result.sroa.6.0..sroa_idx106, align 8, !tbaa !17
  store double %result.sroa.9.0.lcssa, double* %result.sroa.9.0..sroa_idx109, align 8, !tbaa !18
  %inc83 = add nuw i64 %x.0164, 1
  %exitcond167 = icmp eq i64 %inc83, %0
  br i1 %exitcond167, label %for.inc85.loopexit, label %for.body4

for.cond.cleanup11.loopexit:                      ; preds = %for.body12
  %add29.lcssa = phi double [ %add29, %for.body12 ]
  %add49.lcssa = phi double [ %add49, %for.body12 ]
  %add69.lcssa = phi double [ %add69, %for.body12 ]
  br label %for.cond.cleanup11

for.cond.cleanup11:                               ; preds = %for.cond.cleanup11.loopexit, %for.cond8.preheader
  %result.sroa.9.1.lcssa = phi double [ %result.sroa.9.0155, %for.cond8.preheader ], [ %add69.lcssa, %for.cond.cleanup11.loopexit ]
  %result.sroa.6.1.lcssa = phi double [ %result.sroa.6.0156, %for.cond8.preheader ], [ %add49.lcssa, %for.cond.cleanup11.loopexit ]
  %result.sroa.0.1.lcssa = phi double [ %result.sroa.0.0157, %for.cond8.preheader ], [ %add29.lcssa, %for.cond.cleanup11.loopexit ]
  %inc71 = add nuw i64 %v.0158, 1
  %cmp6 = icmp ult i64 %inc71, %3
  br i1 %cmp6, label %for.cond8.preheader, label %for.cond.cleanup.loopexit

for.body12:                                       ; preds = %for.body12, %for.body12.lr.ph
  %u.0151 = phi i64 [ 0, %for.body12.lr.ph ], [ %inc, %for.body12 ]
  %result.sroa.0.1150 = phi double [ %result.sroa.0.0157, %for.body12.lr.ph ], [ %add29, %for.body12 ]
  %result.sroa.6.1149 = phi double [ %result.sroa.6.0156, %for.body12.lr.ph ], [ %add49, %for.body12 ]
  %result.sroa.9.1148 = phi double [ %result.sroa.9.0155, %for.body12.lr.ph ], [ %add69, %for.body12 ]
  %add21 = sub i64 %add17, %u.0151
  %arrayidx22 = getelementptr inbounds double, double* %4, i64 %add21
  %5 = load double, double* %arrayidx22, align 8, !tbaa !19
  %add24 = add i64 %u.0151, %mul23
  %r = getelementptr inbounds %struct._ShortPixelPacket, %struct._ShortPixelPacket* %p, i64 %add24, i32 0
  %6 = load i16, i16* %r, align 2, !tbaa !20
  %conv26 = uitofp i16 %6 to double
  %mul27 = fmul double %5, %conv26
  %add29 = fadd double %result.sroa.0.1150, %mul27
  %g = getelementptr inbounds %struct._ShortPixelPacket, %struct._ShortPixelPacket* %p, i64 %add24, i32 1
  %7 = load i16, i16* %g, align 2, !tbaa !22
  %conv46 = uitofp i16 %7 to double
  %mul47 = fmul double %5, %conv46
  %add49 = fadd double %result.sroa.6.1149, %mul47
  %b = getelementptr inbounds %struct._ShortPixelPacket, %struct._ShortPixelPacket* %p, i64 %add24, i32 2
  %8 = load i16, i16* %b, align 2, !tbaa !23
  %conv66 = uitofp i16 %8 to double
  %mul67 = fmul double %5, %conv66
  %add69 = fadd double %result.sroa.9.1148, %mul67
  %inc = add nuw i64 %u.0151, 1
  %exitcond = icmp eq i64 %inc, %1
  br i1 %exitcond, label %for.cond.cleanup11.loopexit, label %for.body12

for.inc85.loopexit:                               ; preds = %for.cond.cleanup
  br label %for.inc85

for.inc85:                                        ; preds = %for.inc85.loopexit, %for.cond1.preheader
  %inc86 = add nuw i64 %y.0166, 1
  %cmp = icmp ult i64 %inc86, %2
  br i1 %cmp, label %for.cond1.preheader, label %for.end87.loopexit

for.end87.loopexit:                               ; preds = %for.inc85
  br label %for.end87

for.end87:                                        ; preds = %for.end87.loopexit, %entry
  ret void
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"struct@_Image", !4, i64 0, !4, i64 8, !7, i64 16, !7, i64 18, !7, i64 20}
!4 = !{!"long", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!"short", !5, i64 0}
!8 = !{!9, !4, i64 0}
!9 = !{!"struct@_KernelInfo", !4, i64 0, !4, i64 8, !10, i64 16}
!10 = !{!"pointer@_ZTSPd", !5, i64 0}
!11 = !{!3, !4, i64 8}
!12 = !{!9, !4, i64 8}
!13 = !{!9, !10, i64 16}
!14 = !{!15, !16, i64 0}
!15 = !{!"struct@_DoublePixelPacket", !16, i64 0, !16, i64 8, !16, i64 16}
!16 = !{!"double", !5, i64 0}
!17 = !{!15, !16, i64 8}
!18 = !{!15, !16, i64 16}
!19 = !{!16, !16, i64 0}
!20 = !{!21, !7, i64 0}
!21 = !{!"struct@_ShortPixelPacket", !7, i64 0, !7, i64 2, !7, i64 4}
!22 = !{!21, !7, i64 2}
!23 = !{!21, !7, i64 4}
