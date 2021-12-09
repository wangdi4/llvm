; RUN: llc < %s -print-after=global-fma -experimental-debug-variable-locations=false -o /dev/null 2>&1 | FileCheck %s

; Check global FMA does not crash due to debuginfo in other BB

; CHECK: bb.0.entry:
; CHECK:   %1:fr32x = nofpexcept VFMADD213SSZr %2:fr32x(tied-def 0), %3:fr32x, %4:fr32x, implicit $mxcsr
; CHECK: bb.1.bb_1:
; CHECK:   DBG_VALUE $noreg, $noreg, !"rsurf", !DIExpression(), debug-location !13; module_sf_ssib.fppized.f90:0 line no:5072


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define float @module_sf_ssib_mp_temrs2_(float %a, float %b, float %c) local_unnamed_addr #1 {
entry:                                      ; preds = %bb979_then, %bb973_else
  %mul.1726 = fmul fast float %a, %b
  %.pre = fadd fast float %mul.1726, %c
  br label %bb_1
bb_1:
  call void @llvm.dbg.value(metadata float %mul.1726, metadata !894, metadata !DIExpression()), !dbg !7080
  %.pre786 = fadd fast float %mul.1726, 1.0
  br label %bb_2
bb_2:
  ret float %.pre
}

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

attributes #1 = { nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!1265, !1266, !1267}
!llvm.dbg.cu = !{!80}
!omp_offload.info = !{}
!nvvm.annotations = !{}

!2 = !DIModule(scope: null, name: "module_sf_ssib", file: !3, line: 1)
!3 = !DIFile(filename: "module_sf_ssib.fppized.f90", directory: "/export/users2/liuchen/Jira/jira30505/benchspec/CPU/621.wrf_s/build/build_base_core_avx512.0000")
!5 = !DIBasicType(name: "REAL*4", size: 32, encoding: DW_ATE_float)
!7080 = !DILocation(line: 0, scope: !705)
!705 = distinct !DISubprogram(name: "temrs2", linkageName: "module_sf_ssib_mp_temrs2_", scope: !2, file: !3, line: 4909, type: !78, scopeLine: 4909, spFlags: DISPFlagDefinition, unit: !80, retainedNodes: !706)
!706 = !{!707}
!707 = !DILocalVariable(name: "dtt", arg: 1, scope: !705, file: !3, line: 4910, type: !5)
!78 = !DISubroutineType(types: !79)
!79 = !{null}
!80 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !3, producer: "Intel(R) Fortran 21.0-2723", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!894 = !DILocalVariable(name: "rsurf", scope: !705, file: !3, line: 5072, type: !5)
!1265 = !{i32 2, !"Debug Info Version", i32 3}
!1266 = !{i32 2, !"Dwarf Version", i32 4}
!1267 = !{i32 7, !"openmp", i32 50}
