; RUN: opt -S -VPlanDriver -disable-vplan-predicator -disable-vplan-subregions < %s | FileCheck %s

; CHECK-LABEL: foo
; CHECK: ._crit_edge:                                      ; preds = %middle.block, %.lr.ph
; CHECK:  %inc.lcssa = phi i64 [ %inc, %.lr.ph ], [ %n.vec, %middle.block ]
; CHECK: store i64 %inc.lcssa, i64* %pos
  
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.TypOutputFile = type { [256 x i8], i64, i64 }

@Output = common global %struct.TypOutputFile* null, align 8

define void @foo() local_unnamed_addr #0 {
  %1 = load %struct.TypOutputFile*, %struct.TypOutputFile** @Output, align 8
  %pos = getelementptr inbounds %struct.TypOutputFile, %struct.TypOutputFile* %1, i64 0, i32 1
  store i64 0, i64* %pos, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %0
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.SIMD.1
  %indent = getelementptr inbounds %struct.TypOutputFile, %struct.TypOutputFile* %1, i64 0, i32 2
  %2 = load i64, i64* %indent, align 8
  %cmp6 = icmp sgt i64 %2, 0
  br i1 %cmp6, label %.lr.ph.preheader, label %4

.lr.ph.preheader:                                 ; preds = %DIR.QUAL.LIST.END.2
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph.preheader, %.lr.ph
  %3 = phi i64 [ %inc, %.lr.ph ], [ 0, %.lr.ph.preheader ]
  %i.07 = phi i32 [ %inc3, %.lr.ph ], [ 0, %.lr.ph.preheader ]
  %inc = add i64 %3, 1
  %arrayidx = getelementptr %struct.TypOutputFile, %struct.TypOutputFile* %1, i64 0, i32 0, i64 %3
  store i8 32, i8* %arrayidx, align 1
  %inc3 = add i32 %i.07, 1
  %conv = sext i32 %inc3 to i64
  %cmp = icmp slt i64 %conv, %2
  br i1 %cmp, label %.lr.ph, label %._crit_edge

._crit_edge:                                      ; preds = %.lr.ph
  %inc.lcssa = phi i64 [ %inc, %.lr.ph ]
  store i64 %inc.lcssa, i64* %pos, align 8
  br label %4

; <label>:4:                                      ; preds = %._crit_edge, %DIR.QUAL.LIST.END.2
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.4

DIR.QUAL.LIST.END.4:                              ; preds = %4
  ret void
}
declare void @llvm.intel.directive(metadata)
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32)
