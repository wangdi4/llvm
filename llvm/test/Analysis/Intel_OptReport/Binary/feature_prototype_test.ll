; RUN: llc < %s -O3 -intel-opt-report=high -intel-opt-report-emitter=mir -opt-report-embed -enable-protobuf-opt-report=false -debug-only=opt-report-support-utils 2>&1 | FileCheck %s --check-prefixes=LEGACY,CHECK
; RUN: llc < %s -O3 -intel-opt-report=high -intel-opt-report-emitter=mir -opt-report-embed -enable-protobuf-opt-report=true -debug-only=opt-report-support-utils 2>&1 | FileCheck %s --check-prefixes=PROTO-BOR,CHECK
; REQUIRES: asserts, proto_bor

; Check for 7 loop with opt-reports.
; CHECK: LOOP BEGIN
; CHECK: LOOP BEGIN
; CHECK: LOOP BEGIN
; CHECK: LOOP BEGIN
; CHECK: LOOP BEGIN
; CHECK: LOOP BEGIN
; CHECK: LOOP BEGIN

; Check legacy mechanism of emitting binary stream for each loop.
; LEGACY: Opt-report binary stream:
; LEGACY: Opt-report binary stream:
; LEGACY: Opt-report binary stream:
; LEGACY: Opt-report binary stream:
; LEGACY: Opt-report binary stream:
; LEGACY: Opt-report binary stream:
; LEGACY: Opt-report binary stream:

; Check Protobuf-based binary opt-report feature.
; PROTO-BOR-LABEL:    --- Start Protobuf Binary OptReport Printer ---
; PROTO-BOR-NEXT:     Version: 1.5
; PROTO-BOR-NEXT:     Property Message Map:
; PROTO-BOR-DAG:        C_LOOP_VEC_VL --> vectorization support: vector length %s
; PROTO-BOR-DAG:        C_LOOP_COMPLETE_UNROLL_FACTOR --> Loop completely unrolled by %d
; PROTO-BOR-DAG:        C_LOOP_VECTORIZED --> LOOP WAS VECTORIZED
; PROTO-BOR-DAG:        C_LOOP_COMPLETE_UNROLL --> Loop completely unrolled
; PROTO-BOR-NEXT:     Number of reports: 7

; PROTO-BOR-DAG:      === Loop Begin ===
; PROTO-BOR-DAG:      Anchor ID: 59782deec23e72e34ee8c4f37ad91514
; PROTO-BOR-DAG:      Number of remarks: 0
; PROTO-BOR-DAG:      ==== Loop End ====

; PROTO-BOR-DAG:      === Loop Begin ===
; PROTO-BOR-DAG:      Anchor ID: fa1bbf46649ed0a2ce779d054e268257
; PROTO-BOR-DAG:      Number of remarks: 1
; PROTO-BOR-DAG:        Property: C_LOOP_COMPLETE_UNROLL, Remark ID: 25508, Remark Args:
; PROTO-BOR-DAG:      ==== Loop End ====

; PROTO-BOR-DAG:      === Loop Begin ===
; PROTO-BOR-DAG:      Anchor ID: 17c264a03c74ab4ffdb033981dff60d2
; PROTO-BOR-DAG:      Number of remarks: 2
; PROTO-BOR-DAG:        Property: C_LOOP_VECTORIZED, Remark ID: 15300, Remark Args:
; PROTO-BOR-DAG:        Property: C_LOOP_VEC_VL, Remark ID: 15305, Remark Args: 8
; PROTO-BOR-DAG:      ==== Loop End ====

; PROTO-BOR-DAG:      === Loop Begin ===
; PROTO-BOR-DAG:      Anchor ID: 7570d5ba2a865bcde3ab13af9e215a98
; PROTO-BOR-DAG:      Number of remarks: 1
; PROTO-BOR-DAG:        Property: C_LOOP_COMPLETE_UNROLL_FACTOR, Remark ID: 25436, Remark Args: 8
; PROTO-BOR-DAG:      ==== Loop End ====

; PROTO-BOR-DAG:      === Loop Begin ===
; PROTO-BOR-DAG:      Anchor ID: 63763e85a72f0bbe9041efd74d0475c5
; PROTO-BOR-DAG:      Number of remarks: 0
; PROTO-BOR-DAG:      ==== Loop End ====

; PROTO-BOR-DAG:      === Loop Begin ===
; PROTO-BOR-DAG:      Anchor ID: b5ce33333e51ead41791f54805249447
; PROTO-BOR-DAG:      Number of remarks: 0
; PROTO-BOR-DAG:      ==== Loop End ====

; PROTO-BOR-DAG:      === Loop Begin ===
; PROTO-BOR-DAG:      Anchor ID: e1a04b16fd921e22f9b82ca245f3f57b
; PROTO-BOR-DAG:      Number of remarks: 0
; PROTO-BOR-DAG:      ==== Loop End ====
; PROTO-BOR:          --- End Protobuf Binary OptReport Printer ---

; ModuleID = 'vec.cpp'
source_filename = "vec.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@sum = dso_local local_unnamed_addr global double* null, align 8, !dbg !0

; Function Attrs: norecurse nounwind uwtable
define dso_local double @_Z3fooPdi(double* nocapture readonly %x, i32 %n) local_unnamed_addr #0 !dbg !13 !intel.optreport.rootnode !28 {
entry:
  call void @llvm.dbg.value(metadata double* %x, metadata !18, metadata !DIExpression()), !dbg !37
  call void @llvm.dbg.value(metadata i32 %n, metadata !19, metadata !DIExpression()), !dbg !38
  call void @llvm.dbg.value(metadata i32 0, metadata !20, metadata !DIExpression()), !dbg !39
  %cmp106 = icmp sgt i32 %n, 0, !dbg !40
  %0 = load double*, double** @sum, align 8, !tbaa !42
  br i1 %cmp106, label %for.body.lr.ph, label %region.18, !dbg !46

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  %1 = add nsw i64 %wide.trip.count, -1, !dbg !47
  %arrayIdx = getelementptr inbounds double, double* %x, i64 %1, !dbg !47
  %hir.cmp.82 = icmp ult double* %arrayIdx, %0, !dbg !46
  %arrayIdx123 = getelementptr inbounds double, double* %0, i64 %1, !dbg !48
  %hir.cmp.83 = icmp ult double* %arrayIdx123, %x, !dbg !46
  %hir.cmp.85 = or i1 %hir.cmp.83, %hir.cmp.82
  br i1 %hir.cmp.85, label %then.85, label %loop.76

then.85:                                          ; preds = %for.body.lr.ph
  %2 = and i64 %wide.trip.count, -8
  %hir.cmp.106 = icmp eq i64 %2, 0
  br i1 %hir.cmp.106, label %ifmerge.106, label %loop.105

loop.105:                                         ; preds = %then.85, %loop.105
  %i1.i64.0 = phi i64 [ %nextivloop.105, %loop.105 ], [ 0, %then.85 ]
  %arrayIdx125 = getelementptr inbounds double, double* %x, i64 %i1.i64.0, !dbg !47
  %3 = bitcast double* %arrayIdx125 to <8 x double>*, !dbg !47
  %gepload = load <8 x double>, <8 x double>* %3, align 8, !dbg !47, !tbaa !49, !alias.scope !951, !noalias !953
  %4 = fmul <8 x double> %gepload, %gepload, !dbg !46
  %5 = fmul <8 x double> %gepload, %4, !dbg !46
  %arrayIdx129 = getelementptr inbounds double, double* %0, i64 %i1.i64.0, !dbg !48
  %6 = bitcast double* %arrayIdx129 to <8 x double>*, !dbg !48
  %gepload130 = load <8 x double>, <8 x double>* %6, align 8, !dbg !54, !tbaa !49, !alias.scope !953, !noalias !951
  %7 = fadd <8 x double> %gepload130, %5, !dbg !46
  store <8 x double> %7, <8 x double>* %6, align 8, !dbg !54, !tbaa !49, !alias.scope !953, !noalias !951
  %nextivloop.105 = add nuw nsw i64 %i1.i64.0, 8, !dbg !40
  %condloop.105 = icmp slt i64 %nextivloop.105, %2, !dbg !40
  br i1 %condloop.105, label %loop.105, label %ifmerge.106, !dbg !46, !llvm.loop !55

ifmerge.106:                                      ; preds = %loop.105, %then.85
  %hir.cmp.107 = icmp ult i64 %2, %wide.trip.count
  br i1 %hir.cmp.107, label %loop.72, label %region.18

loop.72:                                          ; preds = %ifmerge.106, %loop.72
  %i1.i64.1 = phi i64 [ %nextivloop.72, %loop.72 ], [ %2, %ifmerge.106 ]
  %arrayIdx135 = getelementptr inbounds double, double* %x, i64 %i1.i64.1, !dbg !47
  %gepload136 = load double, double* %arrayIdx135, align 8, !dbg !47, !tbaa !49, !alias.scope !951, !noalias !953
  %8 = fmul double %gepload136, %gepload136, !dbg !69
  %9 = fmul double %gepload136, %8, !dbg !70
  %arrayIdx140 = getelementptr inbounds double, double* %0, i64 %i1.i64.1, !dbg !48
  %gepload141 = load double, double* %arrayIdx140, align 8, !dbg !54, !tbaa !49, !alias.scope !953, !noalias !951
  %10 = fadd double %gepload141, %9, !dbg !54
  store double %10, double* %arrayIdx140, align 8, !dbg !54, !tbaa !49, !alias.scope !953, !noalias !951
  %nextivloop.72 = add nuw nsw i64 %i1.i64.1, 1, !dbg !40
  %condloop.72 = icmp slt i64 %i1.i64.1, %1, !dbg !40
  br i1 %condloop.72, label %loop.72, label %region.18, !dbg !46, !llvm.loop !71

loop.76:                                          ; preds = %for.body.lr.ph, %loop.76
  %i1.i64.2 = phi i64 [ %nextivloop.76, %loop.76 ], [ 0, %for.body.lr.ph ]
  %arrayIdx144 = getelementptr inbounds double, double* %x, i64 %i1.i64.2, !dbg !47
  %gepload145 = load double, double* %arrayIdx144, align 8, !dbg !47, !tbaa !49
  %11 = fmul double %gepload145, %gepload145, !dbg !69
  %12 = fmul double %gepload145, %11, !dbg !70
  %arrayIdx151 = getelementptr inbounds double, double* %0, i64 %i1.i64.2, !dbg !48
  %gepload152 = load double, double* %arrayIdx151, align 8, !dbg !54, !tbaa !49
  %13 = fadd double %gepload152, %12, !dbg !54
  store double %13, double* %arrayIdx151, align 8, !dbg !54, !tbaa !49
  %nextivloop.76 = add nuw nsw i64 %i1.i64.2, 1, !dbg !40
  %condloop.76 = icmp slt i64 %i1.i64.2, %1, !dbg !40
  br i1 %condloop.76, label %loop.76, label %region.18, !dbg !46, !llvm.loop !79

region.18:                                        ; preds = %loop.76, %loop.72, %ifmerge.106, %entry
  call void @llvm.dbg.value(metadata i32 0, metadata !22, metadata !DIExpression()), !dbg !84
  %arrayIdx158 = getelementptr inbounds double, double* %x, i64 9, !dbg !85
  %hir.cmp.92 = icmp ult double* %arrayIdx158, %0, !dbg !46
  %arrayIdx160 = getelementptr inbounds double, double* %0, i64 9, !dbg !87
  %hir.cmp.93 = icmp ult double* %arrayIdx160, %x, !dbg !46
  %hir.cmp.95 = or i1 %hir.cmp.93, %hir.cmp.92
  br i1 %hir.cmp.95, label %loop.113, label %loop.86

loop.113:                                         ; preds = %region.18
  %14 = bitcast double* %x to <8 x double>*, !dbg !85
  %gepload163 = load <8 x double>, <8 x double>* %14, align 8, !dbg !85, !tbaa !49, !alias.scope !988, !noalias !990
  %15 = fmul <8 x double> %gepload163, %gepload163, !dbg !46
  %16 = fmul <8 x double> %gepload163, %15, !dbg !46
  %17 = bitcast double* %0 to <8 x double>*, !dbg !87
  %gepload168 = load <8 x double>, <8 x double>* %17, align 8, !dbg !91, !tbaa !49, !alias.scope !990, !noalias !988
  %18 = fadd <8 x double> %gepload168, %16, !dbg !46
  store <8 x double> %18, <8 x double>* %17, align 8, !dbg !91, !tbaa !49, !alias.scope !990, !noalias !988
  %arrayIdx171 = getelementptr inbounds double, double* %x, i64 8, !dbg !85
  %gepload172 = load double, double* %arrayIdx171, align 8, !dbg !85, !tbaa !49, !alias.scope !988, !noalias !990
  %19 = fmul double %gepload172, %gepload172, !dbg !92
  %20 = fmul double %gepload172, %19, !dbg !93
  %arrayIdx176 = getelementptr inbounds double, double* %0, i64 8, !dbg !87
  %gepload177 = load double, double* %arrayIdx176, align 8, !dbg !91, !tbaa !49, !alias.scope !990, !noalias !988
  %21 = fadd double %gepload177, %20, !dbg !91
  store double %21, double* %arrayIdx176, align 8, !dbg !91, !tbaa !49, !alias.scope !990, !noalias !988
  %arrayIdx171.1 = getelementptr inbounds double, double* %x, i64 9, !dbg !85
  %gepload172.1 = load double, double* %arrayIdx171.1, align 8, !dbg !85, !tbaa !49, !alias.scope !988, !noalias !990
  %22 = fmul double %gepload172.1, %gepload172.1, !dbg !92
  %23 = fmul double %gepload172.1, %22, !dbg !93
  %arrayIdx176.1 = getelementptr inbounds double, double* %0, i64 9, !dbg !87
  %gepload177.1 = load double, double* %arrayIdx176.1, align 8, !dbg !91, !tbaa !49, !alias.scope !990, !noalias !988
  %24 = fadd double %gepload177.1, %23, !dbg !91
  store double %24, double* %arrayIdx176.1, align 8, !dbg !91, !tbaa !49, !alias.scope !990, !noalias !988
  br label %loop.165.preheader, !dbg !94

loop.86:                                          ; preds = %region.18, %loop.86
  %i1.i64161.2 = phi i64 [ %nextivloop.86, %loop.86 ], [ 0, %region.18 ]
  %arrayIdx180 = getelementptr inbounds double, double* %x, i64 %i1.i64161.2, !dbg !85
  %gepload181 = load double, double* %arrayIdx180, align 8, !dbg !85, !tbaa !49
  %25 = fmul double %gepload181, %gepload181, !dbg !92
  %26 = fmul double %gepload181, %25, !dbg !93
  %arrayIdx187 = getelementptr inbounds double, double* %0, i64 %i1.i64161.2, !dbg !87
  %gepload188 = load double, double* %arrayIdx187, align 8, !dbg !91, !tbaa !49
  %27 = fadd double %gepload188, %26, !dbg !91
  store double %27, double* %arrayIdx187, align 8, !dbg !91, !tbaa !49
  %nextivloop.86 = add nuw nsw i64 %i1.i64161.2, 1, !dbg !96
  %condloop.86 = icmp ult i64 %nextivloop.86, 10, !dbg !96
  br i1 %condloop.86, label %loop.86, label %loop.165.preheader, !dbg !97, !llvm.loop !98

loop.165.preheader:                               ; preds = %loop.86, %loop.113
  br label %loop.165, !dbg !94

loop.165:                                         ; preds = %loop.165.preheader, %loop.165
  %i1.i64194.0 = phi i64 [ %nextivloop.165, %loop.165 ], [ 0, %loop.165.preheader ]
  %28 = shl nuw i64 %i1.i64194.0, 1, !dbg !94
  %arrayIdx195 = getelementptr inbounds double, double* %x, i64 %28, !dbg !94
  %gepload196 = load double, double* %arrayIdx195, align 8, !dbg !94, !tbaa !49
  %29 = fmul double %gepload196, %gepload196, !dbg !108
  %30 = fmul double %gepload196, %29, !dbg !109
  %arrayIdx199 = getelementptr inbounds double, double* %0, i64 %28, !dbg !110
  %gepload200 = load double, double* %arrayIdx199, align 8, !dbg !111, !tbaa !49
  %31 = fadd double %gepload200, %30, !dbg !111
  store double %31, double* %arrayIdx199, align 8, !dbg !111, !tbaa !49
  %32 = or i64 %28, 1, !dbg !94
  %arrayIdx203 = getelementptr inbounds double, double* %x, i64 %32, !dbg !94
  %gepload204 = load double, double* %arrayIdx203, align 8, !dbg !94, !tbaa !49
  %33 = fmul double %gepload204, %gepload204, !dbg !108
  %34 = fmul double %gepload204, %33, !dbg !109
  %arrayIdx210 = getelementptr inbounds double, double* %0, i64 %32, !dbg !110
  %gepload211 = load double, double* %arrayIdx210, align 8, !dbg !111, !tbaa !49
  %35 = fadd double %gepload211, %34, !dbg !111
  store double %35, double* %arrayIdx210, align 8, !dbg !111, !tbaa !49
  %nextivloop.165 = add nuw nsw i64 %i1.i64194.0, 1, !dbg !112
  %condloop.165 = icmp ult i64 %nextivloop.165, 5, !dbg !112
  br i1 %condloop.165, label %loop.165, label %region.54, !dbg !113, !llvm.loop !114

region.54:                                        ; preds = %loop.165
  %gepload217 = load double, double* %x, align 8, !dbg !121, !tbaa !49
  %36 = fmul double %gepload217, %gepload217, !dbg !123
  %37 = fmul double %gepload217, %36, !dbg !124
  %gepload220 = load double, double* %0, align 8, !dbg !125, !tbaa !49
  %38 = fadd double %gepload220, %37, !dbg !125
  store double %38, double* %0, align 8, !dbg !125, !tbaa !49
  %arrayIdx222 = getelementptr inbounds double, double* %x, i64 1, !dbg !121
  %gepload223 = load double, double* %arrayIdx222, align 8, !dbg !121, !tbaa !49
  %39 = fmul double %gepload223, %gepload223, !dbg !123
  %40 = fmul double %gepload223, %39, !dbg !124
  %arrayIdx229 = getelementptr inbounds double, double* %0, i64 1, !dbg !126
  %gepload230 = load double, double* %arrayIdx229, align 8, !dbg !125, !tbaa !49
  %41 = fadd double %gepload230, %40, !dbg !125
  store double %41, double* %arrayIdx229, align 8, !dbg !125, !tbaa !49
  %arrayIdx235 = getelementptr inbounds double, double* %x, i64 2, !dbg !121
  %gepload236 = load double, double* %arrayIdx235, align 8, !dbg !121, !tbaa !49
  %42 = fmul double %gepload236, %gepload236, !dbg !123
  %43 = fmul double %gepload236, %42, !dbg !124
  %arrayIdx242 = getelementptr inbounds double, double* %0, i64 2, !dbg !126
  %gepload243 = load double, double* %arrayIdx242, align 8, !dbg !125, !tbaa !49
  %44 = fadd double %gepload243, %43, !dbg !125
  store double %44, double* %arrayIdx242, align 8, !dbg !125, !tbaa !49
  %arrayIdx248 = getelementptr inbounds double, double* %x, i64 3, !dbg !121
  %gepload249 = load double, double* %arrayIdx248, align 8, !dbg !121, !tbaa !49
  %45 = fmul double %gepload249, %gepload249, !dbg !123
  %46 = fmul double %gepload249, %45, !dbg !124
  %arrayIdx255 = getelementptr inbounds double, double* %0, i64 3, !dbg !126
  %gepload256 = load double, double* %arrayIdx255, align 8, !dbg !125, !tbaa !49
  %47 = fadd double %gepload256, %46, !dbg !125
  store double %47, double* %arrayIdx255, align 8, !dbg !125, !tbaa !49
  %arrayIdx261 = getelementptr inbounds double, double* %x, i64 4, !dbg !121
  %gepload262 = load double, double* %arrayIdx261, align 8, !dbg !121, !tbaa !49
  %48 = fmul double %gepload262, %gepload262, !dbg !123
  %49 = fmul double %gepload262, %48, !dbg !124
  %arrayIdx268 = getelementptr inbounds double, double* %0, i64 4, !dbg !126
  %gepload269 = load double, double* %arrayIdx268, align 8, !dbg !125, !tbaa !49
  %50 = fadd double %gepload269, %49, !dbg !125
  store double %50, double* %arrayIdx268, align 8, !dbg !125, !tbaa !49
  %arrayIdx274 = getelementptr inbounds double, double* %x, i64 5, !dbg !121
  %gepload275 = load double, double* %arrayIdx274, align 8, !dbg !121, !tbaa !49
  %51 = fmul double %gepload275, %gepload275, !dbg !123
  %52 = fmul double %gepload275, %51, !dbg !124
  %arrayIdx281 = getelementptr inbounds double, double* %0, i64 5, !dbg !126
  %gepload282 = load double, double* %arrayIdx281, align 8, !dbg !125, !tbaa !49
  %53 = fadd double %gepload282, %52, !dbg !125
  store double %53, double* %arrayIdx281, align 8, !dbg !125, !tbaa !49
  %arrayIdx287 = getelementptr inbounds double, double* %x, i64 6, !dbg !121
  %gepload288 = load double, double* %arrayIdx287, align 8, !dbg !121, !tbaa !49
  %54 = fmul double %gepload288, %gepload288, !dbg !123
  %55 = fmul double %gepload288, %54, !dbg !124
  %arrayIdx294 = getelementptr inbounds double, double* %0, i64 6, !dbg !126
  %gepload295 = load double, double* %arrayIdx294, align 8, !dbg !125, !tbaa !49
  %56 = fadd double %gepload295, %55, !dbg !125
  store double %56, double* %arrayIdx294, align 8, !dbg !125, !tbaa !49
  %arrayIdx300 = getelementptr inbounds double, double* %x, i64 7, !dbg !121
  %gepload301 = load double, double* %arrayIdx300, align 8, !dbg !121, !tbaa !49
  %57 = fmul double %gepload301, %gepload301, !dbg !123
  %58 = fmul double %gepload301, %57, !dbg !124
  %arrayIdx307 = getelementptr inbounds double, double* %0, i64 7, !dbg !126
  %gepload308 = load double, double* %arrayIdx307, align 8, !dbg !125, !tbaa !49
  %59 = fadd double %gepload308, %58, !dbg !125
  store double %59, double* %arrayIdx307, align 8, !dbg !125, !tbaa !49
  %arrayIdx313 = getelementptr inbounds double, double* %x, i64 8, !dbg !121
  %gepload314 = load double, double* %arrayIdx313, align 8, !dbg !121, !tbaa !49
  %60 = fmul double %gepload314, %gepload314, !dbg !123
  %61 = fmul double %gepload314, %60, !dbg !124
  %arrayIdx320 = getelementptr inbounds double, double* %0, i64 8, !dbg !126
  %gepload321 = load double, double* %arrayIdx320, align 8, !dbg !125, !tbaa !49
  %62 = fadd double %gepload321, %61, !dbg !125
  store double %62, double* %arrayIdx320, align 8, !dbg !125, !tbaa !49
  %gepload327 = load double, double* %arrayIdx158, align 8, !dbg !121, !tbaa !49
  %63 = fmul double %gepload327, %gepload327, !dbg !123
  %64 = fmul double %gepload327, %63, !dbg !124
  %gepload334 = load double, double* %arrayIdx160, align 8, !dbg !125, !tbaa !49
  %65 = fadd double %gepload334, %64, !dbg !125
  store double %65, double* %arrayIdx160, align 8, !dbg !125, !tbaa !49
  %div = sdiv i32 %n, 2, !dbg !127
  %idxprom65 = sext i32 %div to i64, !dbg !128
  %arrayidx66 = getelementptr inbounds double, double* %0, i64 %idxprom65, !dbg !128
  %66 = load double, double* %arrayidx66, align 8, !dbg !128, !tbaa !49
  ret double %66, !dbg !129
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!8, !9, !10}
!llvm.dbg.intel.emit_class_debug_always = !{!11}
!llvm.ident = !{!12}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "sum", scope: !2, file: !3, line: 1, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !3, producer: "clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang dba1f222b81823040e16528a12868e26b01725a8) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 92b95874f5220d42dfb1a813c07a9da5ba45eb9f)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: None)
!3 = !DIFile(filename: "vec.cpp", directory: "/icsmnt/scels84_iusers/vzakhari/workspaces/xmain02/test")
!4 = !{}
!5 = !{!0}
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!7 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!8 = !{i32 2, !"Dwarf Version", i32 4}
!9 = !{i32 2, !"Debug Info Version", i32 3}
!10 = !{i32 1, !"wchar_size", i32 4}
!11 = !{!"true"}
!12 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang dba1f222b81823040e16528a12868e26b01725a8) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 92b95874f5220d42dfb1a813c07a9da5ba45eb9f)"}
!13 = distinct !DISubprogram(name: "foo", linkageName: "_Z3fooPdi", scope: !3, file: !3, line: 3, type: !14, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !17)
!14 = !DISubroutineType(types: !15)
!15 = !{!7, !6, !16}
!16 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!17 = !{!18, !19, !20, !22, !24, !26}
!18 = !DILocalVariable(name: "x", arg: 1, scope: !13, file: !3, line: 3, type: !6)
!19 = !DILocalVariable(name: "n", arg: 2, scope: !13, file: !3, line: 3, type: !16)
!20 = !DILocalVariable(name: "i", scope: !21, file: !3, line: 10, type: !16)
!21 = distinct !DILexicalBlock(scope: !13, file: !3, line: 10, column: 3)
!22 = !DILocalVariable(name: "i", scope: !23, file: !3, line: 13, type: !16)
!23 = distinct !DILexicalBlock(scope: !13, file: !3, line: 13, column: 3)
!24 = !DILocalVariable(name: "i", scope: !25, file: !3, line: 17, type: !16)
!25 = distinct !DILexicalBlock(scope: !13, file: !3, line: 17, column: 3)
!26 = !DILocalVariable(name: "i", scope: !27, file: !3, line: 21, type: !16)
!27 = distinct !DILexicalBlock(scope: !13, file: !3, line: 21, column: 3)
!28 = distinct !{!"intel.optreport.rootnode", !29}
!29 = distinct !{!"intel.optreport", !30}
!30 = !{!"intel.optreport.first_child", !31}
!31 = distinct !{!"intel.optreport.rootnode", !32}
!32 = distinct !{!"intel.optreport", !33, !35}
!33 = !{!"intel.optreport.debug_location", !34}
!34 = !DILocation(line: 21, column: 3, scope: !27)
!35 = !{!"intel.optreport.remarks", !36}
!36 = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 8}
!37 = !DILocation(line: 3, column: 20, scope: !13)
!38 = !DILocation(line: 3, column: 27, scope: !13)
!39 = !DILocation(line: 10, column: 12, scope: !21)
!40 = !DILocation(line: 10, column: 21, scope: !41)
!41 = distinct !DILexicalBlock(scope: !21, file: !3, line: 10, column: 3)
!42 = !{!43, !43, i64 0}
!43 = !{!"pointer@_ZTSPd", !44, i64 0}
!44 = !{!"omnipotent char", !45, i64 0}
!45 = !{!"Simple C++ TBAA"}
!46 = !DILocation(line: 10, column: 3, scope: !21)
!47 = !DILocation(line: 11, column: 15, scope: !41)
!48 = !DILocation(line: 11, column: 5, scope: !41)
!49 = !{!50, !50, i64 0}
!50 = !{!"double", !44, i64 0}
!951 = !{!51}
!51 = distinct !{!51, !52}
!52 = distinct !{!52}
!953 = !{!53}
!53 = distinct !{!53, !52}
!54 = !DILocation(line: 11, column: 12, scope: !41)
!55 = distinct !{!55, !46, !56, !57, !58, !59, !60, !61}
!56 = !DILocation(line: 11, column: 32, scope: !21)
!57 = !{!"llvm.loop.vectorize.enable", i1 true}
!58 = !{!"llvm.loop.vectorize.width", i32 1}
!59 = !{!"llvm.loop.interleave.count", i32 1}
!60 = !{!"llvm.loop.unroll.disable"}
!61 = distinct !{!"intel.optreport.rootnode", !62}
!62 = distinct !{!"intel.optreport", !63, !64, !66}
!63 = !{!"intel.optreport.debug_location", !46}
!64 = !{!"intel.optreport.origin", !65}
!65 = !{!"intel.optreport.remark", i32 0, !"Multiversioned loop"}
!66 = !{!"intel.optreport.remarks", !67, !68}
!67 = !{!"intel.optreport.remark", i32 15300, !"LOOP WAS VECTORIZED"}
!68 = !{!"intel.optreport.remark", i32 15305, !"vectorization support: vector length %s", !"8"}
!69 = !DILocation(line: 11, column: 20, scope: !41)
!70 = !DILocation(line: 11, column: 27, scope: !41)
!71 = distinct !{!71, !46, !56, !57, !72, !58, !59, !73}
!72 = !{!"llvm.loop.intel.loopcount_maximum", i32 7}
!73 = distinct !{!"intel.optreport.rootnode", !74}
!74 = distinct !{!"intel.optreport", !63, !75, !77}
!75 = !{!"intel.optreport.origin", !76}
!76 = !{!"intel.optreport.remark", i32 0, !"Remainder loop for vectorization"}
!77 = !{!"intel.optreport.remarks", !78}
!78 = !{!"intel.optreport.remark", i32 15441, !"remainder loop was not vectorized: %s ", !""}
!79 = distinct !{!79, !46, !56, !57, !58, !59, !60, !80}
!80 = distinct !{!"intel.optreport.rootnode", !81}
!81 = distinct !{!"intel.optreport", !63, !82}
!82 = !{!"intel.optreport.remarks", !83}
!83 = !{!"intel.optreport.remark", i32 0, !"The loop has been multiversioned"}
!84 = !DILocation(line: 13, column: 12, scope: !23)
!85 = !DILocation(line: 14, column: 15, scope: !86)
!86 = distinct !DILexicalBlock(scope: !23, file: !3, line: 13, column: 3)
!87 = !DILocation(line: 14, column: 5, scope: !86)
!988 = !{!88}
!88 = distinct !{!88, !89}
!89 = distinct !{!89}
!990 = !{!90}
!90 = distinct !{!90, !89}
!91 = !DILocation(line: 14, column: 12, scope: !86)
!92 = !DILocation(line: 14, column: 20, scope: !86)
!93 = !DILocation(line: 14, column: 27, scope: !86)
!94 = !DILocation(line: 18, column: 15, scope: !95)
!95 = distinct !DILexicalBlock(scope: !25, file: !3, line: 17, column: 3)
!96 = !DILocation(line: 13, column: 21, scope: !86)
!97 = !DILocation(line: 13, column: 3, scope: !23)
!98 = distinct !{!98, !97, !99, !58, !59, !60, !100}
!99 = !DILocation(line: 14, column: 32, scope: !23)
!100 = distinct !{!"intel.optreport.rootnode", !101}
!101 = distinct !{!"intel.optreport", !102, !82, !103}
!102 = !{!"intel.optreport.debug_location", !97}
!103 = !{!"intel.optreport.next_sibling", !104}
!104 = distinct !{!"intel.optreport.rootnode", !105}
!105 = distinct !{!"intel.optreport", !102, !75, !106}
!106 = !{!"intel.optreport.remarks", !78, !107, !130}
!107 = !{!"intel.optreport.remark", i32 0, !"LLorg: Loop has been completely unrolled"}
!108 = !DILocation(line: 18, column: 20, scope: !95)
!109 = !DILocation(line: 18, column: 27, scope: !95)
!110 = !DILocation(line: 18, column: 5, scope: !95)
!111 = !DILocation(line: 18, column: 12, scope: !95)
!112 = !DILocation(line: 17, column: 21, scope: !95)
!113 = !DILocation(line: 17, column: 3, scope: !25)
!114 = distinct !{!114, !113, !115, !60, !116}
!115 = !DILocation(line: 18, column: 32, scope: !25)
!116 = distinct !{!"intel.optreport.rootnode", !117}
!117 = distinct !{!"intel.optreport", !118, !119}
!118 = !{!"intel.optreport.debug_location", !113}
!119 = !{!"intel.optreport.remarks", !120}
!120 = !{!"intel.optreport.remark", i32 0, !"Loop has been unrolled by %d factor", i32 2}
!121 = !DILocation(line: 22, column: 15, scope: !122)
!122 = distinct !DILexicalBlock(scope: !27, file: !3, line: 21, column: 3)
!123 = !DILocation(line: 22, column: 20, scope: !122)
!124 = !DILocation(line: 22, column: 27, scope: !122)
!125 = !DILocation(line: 22, column: 12, scope: !122)
!126 = !DILocation(line: 22, column: 5, scope: !122)
!127 = !DILocation(line: 24, column: 16, scope: !13)
!128 = !DILocation(line: 24, column: 10, scope: !13)
!129 = !DILocation(line: 24, column: 3, scope: !13)
!130 = !{!"intel.optreport.remark", i32 25508, !"Loop completely unrolled"}
