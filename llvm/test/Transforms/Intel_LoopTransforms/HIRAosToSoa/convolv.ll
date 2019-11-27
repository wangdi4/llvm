; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-aos-to-soa -print-after=hir-aos-to-soa < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-aos-to-soa,print<hir>" -aa-pipeline="basic-aa"  < %s 2>&1 | FileCheck %s

; Check if array of structures are copied into temp arrays and later read from temp arrays.
; Temp arrays are allocated with alloca.

; CHECK: Function: foo

; CHECK: DO i1 =
; CHECK: [[ADDR:%[0-9a-z]+]] = @llvm.stacksave();

; CHECK: [[ALLOC_0:%[0-9a-z]+]] = alloca %array_size;
; CHECK-NEXT: [[ALLOC_1:%[0-9a-z]+]] = alloca %array_size;
; CHECK-NEXT: [[ALLOC_2:%[0-9a-z]+]] = alloca %array_size;

; CHECK: DO i2 =
; CHECK: DO i3 =
; CHECK:     [[ALLOC_0]]
; CHECK:     [[ALLOC_1]]
; CHECK:     [[ALLOC_2]]

; CHECK: DO i2 =
; CHECK: DO i3 =
; CHECK: DO i4 =
; CHECK:  = ([[ALLOC_0]])[i2 + %add13 * i3 + i4];
; CHECK:  = ([[ALLOC_1]])[i2 + %add13 * i3 + i4];
; CHECK:  = ([[ALLOC_2]])[i2 + %add13 * i3 + i4];
; CHECK: END LOOP
; CHECK: END LOOP
; CHECK: END LOOP

; CHECK: @llvm.stackrestore(&(([[ADDR]])[0]));
; CHECK: END LOOP


; *** IR Dump Before HIR AOS to SOA ***
; Function: foo
;
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
;             |   |      %result.sroa.0.0163 = %result.sroa.0.0.copyload;
;             |   |      %result.sroa.6.0162 = %result.sroa.6.0.copyload;
;             |   |      %result.sroa.9.0161 = %result.sroa.9.0.copyload;
;             |   |   + DO i3 = 0, %3 + -1, 1   <DO_LOOP>
;             |   |   |      %4 = (%kernel)[0].2;
;             |   |   |      %result.sroa.0.1156 = %result.sroa.0.0163;
;             |   |   |      %result.sroa.6.1155 = %result.sroa.6.0162;
;             |   |   |      %result.sroa.9.1154 = %result.sroa.9.0161;
;             |   |   |   + DO i4 = 0, %1 + -1, 1   <DO_LOOP>
;             |   |   |   |   %5 = (%4)[%1 * i3 + -1 * i4 + ((-1 + %3) * %1)];
;             |   |   |   |   %conv27 = uitofp.i16.double((%p)[i2 + (-1 + %1 + %0) * i3 + i4].0);
;             |   |   |   |   %mul28 = %5  *  %conv27;
;             |   |   |   |   %result.sroa.0.1156 = %result.sroa.0.1156  +  %mul28;
;             |   |   |   |   %conv48 = uitofp.i16.double((%p)[i2 + (-1 + %1 + %0) * i3 + i4].1);
;             |   |   |   |   %mul49 = %5  *  %conv48;
;             |   |   |   |   %result.sroa.6.1155 = %result.sroa.6.1155  +  %mul49;
;             |   |   |   |   %conv69 = uitofp.i16.double((%p)[i2 + (-1 + %1 + %0) * i3 + i4].2);
;             |   |   |   |   %mul70 = %5  *  %conv69;
;             |   |   |   |   %result.sroa.9.1154 = %result.sroa.9.1154  +  %mul70;
;             |   |   |   + END LOOP
;             |   |   |      %result.sroa.0.0163 = %result.sroa.0.1156;
;             |   |   |      %result.sroa.6.0162 = %result.sroa.6.1155;
;             |   |   |      %result.sroa.9.0161 = %result.sroa.9.1154;
;             |   |   + END LOOP
;             |   |      %result.sroa.9.0.lcssa = %result.sroa.9.0161;
;             |   |      %result.sroa.6.0.lcssa = %result.sroa.6.0162;
;             |   |      %result.sroa.0.0.lcssa = %result.sroa.0.0163;
;             |   |
;             |   |   (%q)[i2].0 = %result.sroa.0.0.lcssa;
;             |   |   (%q)[i2].1 = %result.sroa.6.0.lcssa;
;             |   |   (%q)[i2].2 = %result.sroa.9.0.lcssa;
;             |   + END LOOP
;             + END LOOP
;       END REGION
;
; *** IR Dump After HIR AOS to SOA ***
; Function: foo
;
;        BEGIN REGION { }
;              + DO i1 = 0, %2 + -1, 1   <DO_LOOP>
;              |   if (%0 != 0)                          // i2-loop's preheader is extracted
;              |   {
;              |      %3 = (%kernel)[0].1;
;              |      %call = @llvm.stacksave();         // stack save
;              |      %add13 = %1  +  %0;                // 3 allocas for three trailing offsets
;              |      %array_size = %add13  *  %3;
;              |      %alloca = alloca %array_size;
;              |      %alloca14 = alloca %array_size;
;              |      %alloca15 = alloca %array_size;
;              |
;                     if (%3 != 0 && %1 != 0)            // Ztt: Notice ztt is not
;                                                        //      verified because not extracted.
;                     {
;              |      + DO i2 = 0, %3 + -1, 1   <DO_LOOP>         // copy loop
;              |      |   + DO i3 = 0, %add13 + -1, 1   <DO_LOOP>
;              |      |   |   %tmp = uitofp.i16.double((%p)[(-1 + %1 + %0) * i2 + i3].0);
;              |      |   |   (%alloca)[%add13 * i2 + i3] = %tmp;
;              |      |   |   %tmp18 = uitofp.i16.double((%p)[(-1 + %1 + %0) * i2 + i3].1);
;              |      |   |   (%alloca14)[%add13 * i2 + i3] = %tmp18;
;              |      |   |   %tmp19 = uitofp.i16.double((%p)[(-1 + %1 + %0) * i2 + i3].2);
;              |      |   |   (%alloca15)[%add13 * i2 + i3] = %tmp19;
;              |      |   + END LOOP
;              |      + END LOOP
;                     }
;              |
;              |
;              |      + DO i2 = 0, %0 + -1, 1   <DO_LOOP>
;              |      |   %result.sroa.0.0.copyload = (%q)[i2].0;
;              |      |   %result.sroa.6.0.copyload = (%q)[i2].1;
;              |      |   %result.sroa.9.0.copyload = (%q)[i2].2;
;              |      |   %result.sroa.9.0.lcssa = %result.sroa.9.0.copyload;
;              |      |   %result.sroa.6.0.lcssa = %result.sroa.6.0.copyload;
;              |      |   %result.sroa.0.0.lcssa = %result.sroa.0.0.copyload;
;              |      |
;              |      |      %result.sroa.0.0163 = %result.sroa.0.0.copyload;
;              |      |      %result.sroa.6.0162 = %result.sroa.6.0.copyload;
;              |      |      %result.sroa.9.0161 = %result.sroa.9.0.copyload;
;              |      |   + DO i3 = 0, %3 + -1, 1   <DO_LOOP>
;              |      |   |      %4 = (%kernel)[0].2;
;              |      |   |      %result.sroa.0.1156 = %result.sroa.0.0163;
;              |      |   |      %result.sroa.6.1155 = %result.sroa.6.0162;
;              |      |   |      %result.sroa.9.1154 = %result.sroa.9.0161;
;              |      |   |   + DO i4 = 0, %1 + -1, 1   <DO_LOOP>
;                                 // Main loop replacement
;              |      |   |   |   %5 = (%4)[%1 * i3 + -1 * i4 + ((-1 + %3) * %1)];
;              |      |   |   |   %conv27 = (%alloca)[i2 + %add13 * i3 + i4];
;              |      |   |   |   %mul28 = %5  *  %conv27;
;              |      |   |   |   %result.sroa.0.1156 = %result.sroa.0.1156  +  %mul28;
;              |      |   |   |   %conv48 = (%alloca14)[i2 + %add13 * i3 + i4];
;              |      |   |   |   %mul49 = %5  *  %conv48;
;              |      |   |   |   %result.sroa.6.1155 = %result.sroa.6.1155  +  %mul49;
;              |      |   |   |   %conv69 = (%alloca15)[i2 + %add13 * i3 + i4];
;              |      |   |   |   %mul70 = %5  *  %conv69;
;              |      |   |   |   %result.sroa.9.1154 = %result.sroa.9.1154  +  %mul70;
;              |      |   |   + END LOOP
;              |      |   |      %result.sroa.0.0163 = %result.sroa.0.1156;
;              |      |   |      %result.sroa.6.0162 = %result.sroa.6.1155;
;              |      |   |      %result.sroa.9.0161 = %result.sroa.9.1154;
;              |      |   + END LOOP
;              |      |      %result.sroa.9.0.lcssa = %result.sroa.9.0161;
;              |      |      %result.sroa.6.0.lcssa = %result.sroa.6.0162;
;              |      |      %result.sroa.0.0.lcssa = %result.sroa.0.0163;
;              |      |
;              |      |   (%q)[i2].0 = %result.sroa.0.0.lcssa;
;              |      |   (%q)[i2].1 = %result.sroa.6.0.lcssa;
;              |      |   (%q)[i2].2 = %result.sroa.9.0.lcssa;
;              |      + END LOOP
;              |
;              |      @llvm.stackrestore(&((%call)[0]));
;              |   }
;              + END LOOP
;        END REGION


;Module Before HIR
; ModuleID = 'convolv.c'
source_filename = "convolv.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
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
  %cmp172 = icmp eq i64 %2, 0
  br i1 %cmp172, label %for.end90, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp3169 = icmp eq i64 %0, 0
  %height = getelementptr inbounds %struct._KernelInfo, %struct._KernelInfo* %kernel, i64 0, i32 1
  %cmp10153 = icmp eq i64 %1, 0
  %values = getelementptr inbounds %struct._KernelInfo, %struct._KernelInfo* %kernel, i64 0, i32 2
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc88
  %y.0173 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %inc89, %for.inc88 ]
  br i1 %cmp3169, label %for.inc88, label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader
  %3 = load i64, i64* %height, align 8, !tbaa !12
  %cmp6160 = icmp eq i64 %3, 0
  br label %for.body4

for.body4:                                        ; preds = %for.cond.cleanup, %for.body4.lr.ph
  %x.0170 = phi i64 [ 0, %for.body4.lr.ph ], [ %inc86, %for.cond.cleanup ]
  %result.sroa.0.0..sroa_idx = getelementptr inbounds %struct._DoublePixelPacket, %struct._DoublePixelPacket* %q, i64 %x.0170, i32 0
  %result.sroa.0.0.copyload = load double, double* %result.sroa.0.0..sroa_idx, align 8
  %result.sroa.6.0..sroa_idx109 = getelementptr inbounds %struct._DoublePixelPacket, %struct._DoublePixelPacket* %q, i64 %x.0170, i32 1
  %result.sroa.6.0.copyload = load double, double* %result.sroa.6.0..sroa_idx109, align 8
  %result.sroa.9.0..sroa_idx112 = getelementptr inbounds %struct._DoublePixelPacket, %struct._DoublePixelPacket* %q, i64 %x.0170, i32 2
  %result.sroa.9.0.copyload = load double, double* %result.sroa.9.0..sroa_idx112, align 8
  br i1 %cmp6160, label %for.cond.cleanup, label %for.cond8.preheader.preheader

for.cond8.preheader.preheader:                    ; preds = %for.body4
  br label %for.cond8.preheader

for.cond8.preheader:                              ; preds = %for.cond8.preheader.preheader, %for.cond.cleanup11
  %v.0164 = phi i64 [ %inc74, %for.cond.cleanup11 ], [ 0, %for.cond8.preheader.preheader ]
  %result.sroa.0.0163 = phi double [ %result.sroa.0.1.lcssa, %for.cond.cleanup11 ], [ %result.sroa.0.0.copyload, %for.cond8.preheader.preheader ]
  %result.sroa.6.0162 = phi double [ %result.sroa.6.1.lcssa, %for.cond.cleanup11 ], [ %result.sroa.6.0.copyload, %for.cond8.preheader.preheader ]
  %result.sroa.9.0161 = phi double [ %result.sroa.9.1.lcssa, %for.cond.cleanup11 ], [ %result.sroa.9.0.copyload, %for.cond8.preheader.preheader ]
  br i1 %cmp10153, label %for.cond.cleanup11, label %for.body12.lr.ph

for.body12.lr.ph:                                 ; preds = %for.cond8.preheader
  %4 = load double*, double** %values, align 8, !tbaa !13
  %reass.add = add i64 %3, %v.0164
  %reass.mul = mul i64 %reass.add, %1
  %add17 = sub i64 %reass.mul, %1
  %mul24 = mul i64 %v.0164, %sub
  %add23 = add i64 %mul24, %x.0170
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
  store double %result.sroa.6.0.lcssa, double* %result.sroa.6.0..sroa_idx109, align 8, !tbaa !17
  store double %result.sroa.9.0.lcssa, double* %result.sroa.9.0..sroa_idx112, align 8, !tbaa !18
  %inc86 = add nuw i64 %x.0170, 1
  %exitcond174 = icmp eq i64 %inc86, %0
  br i1 %exitcond174, label %for.inc88.loopexit, label %for.body4

for.cond.cleanup11.loopexit:                      ; preds = %for.body12
  %add30.lcssa = phi double [ %add30, %for.body12 ]
  %add51.lcssa = phi double [ %add51, %for.body12 ]
  %add72.lcssa = phi double [ %add72, %for.body12 ]
  br label %for.cond.cleanup11

for.cond.cleanup11:                               ; preds = %for.cond.cleanup11.loopexit, %for.cond8.preheader
  %result.sroa.9.1.lcssa = phi double [ %result.sroa.9.0161, %for.cond8.preheader ], [ %add72.lcssa, %for.cond.cleanup11.loopexit ]
  %result.sroa.6.1.lcssa = phi double [ %result.sroa.6.0162, %for.cond8.preheader ], [ %add51.lcssa, %for.cond.cleanup11.loopexit ]
  %result.sroa.0.1.lcssa = phi double [ %result.sroa.0.0163, %for.cond8.preheader ], [ %add30.lcssa, %for.cond.cleanup11.loopexit ]
  %inc74 = add nuw i64 %v.0164, 1
  %cmp6 = icmp ult i64 %inc74, %3
  br i1 %cmp6, label %for.cond8.preheader, label %for.cond.cleanup.loopexit

for.body12:                                       ; preds = %for.body12, %for.body12.lr.ph
  %u.0157 = phi i64 [ 0, %for.body12.lr.ph ], [ %inc, %for.body12 ]
  %result.sroa.0.1156 = phi double [ %result.sroa.0.0163, %for.body12.lr.ph ], [ %add30, %for.body12 ]
  %result.sroa.6.1155 = phi double [ %result.sroa.6.0162, %for.body12.lr.ph ], [ %add51, %for.body12 ]
  %result.sroa.9.1154 = phi double [ %result.sroa.9.0161, %for.body12.lr.ph ], [ %add72, %for.body12 ]
  %add21 = sub i64 %add17, %u.0157
  %arrayidx22 = getelementptr inbounds double, double* %4, i64 %add21
  %5 = load double, double* %arrayidx22, align 8, !tbaa !19
  %add25 = add i64 %add23, %u.0157
  %r = getelementptr inbounds %struct._ShortPixelPacket, %struct._ShortPixelPacket* %p, i64 %add25, i32 0
  %6 = load i16, i16* %r, align 2, !tbaa !20
  %conv27 = uitofp i16 %6 to double
  %mul28 = fmul double %5, %conv27
  %add30 = fadd double %result.sroa.0.1156, %mul28
  %g = getelementptr inbounds %struct._ShortPixelPacket, %struct._ShortPixelPacket* %p, i64 %add25, i32 1
  %7 = load i16, i16* %g, align 2, !tbaa !22
  %conv48 = uitofp i16 %7 to double
  %mul49 = fmul double %5, %conv48
  %add51 = fadd double %result.sroa.6.1155, %mul49
  %b = getelementptr inbounds %struct._ShortPixelPacket, %struct._ShortPixelPacket* %p, i64 %add25, i32 2
  %8 = load i16, i16* %b, align 2, !tbaa !23
  %conv69 = uitofp i16 %8 to double
  %mul70 = fmul double %5, %conv69
  %add72 = fadd double %result.sroa.9.1154, %mul70
  %inc = add nuw i64 %u.0157, 1
  %exitcond = icmp eq i64 %inc, %1
  br i1 %exitcond, label %for.cond.cleanup11.loopexit, label %for.body12

for.inc88.loopexit:                               ; preds = %for.cond.cleanup
  br label %for.inc88

for.inc88:                                        ; preds = %for.inc88.loopexit, %for.cond1.preheader
  %inc89 = add nuw i64 %y.0173, 1
  %cmp = icmp ult i64 %inc89, %2
  br i1 %cmp, label %for.cond1.preheader, label %for.end90.loopexit

for.end90.loopexit:                               ; preds = %for.inc88
  br label %for.end90

for.end90:                                        ; preds = %for.end90.loopexit, %entry
  ret void
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"DPC++ Compiler 2021.1 (YYYY.8.x.0.MMDD)"}
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
