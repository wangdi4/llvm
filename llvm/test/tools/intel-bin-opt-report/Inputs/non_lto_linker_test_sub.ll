; The functions "vectorize" and "complete_unroll" are part of the
; non_lto_linker_test.ll test case. They are external functions for
; testing binary opt-report reader tool.

; ModuleID = 'linker_test_funcs.c'
source_filename = "linker_test_funcs.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @vectorize(float* nocapture %a, float* nocapture readonly %b, float* nocapture readonly %c) local_unnamed_addr #0 !dbg !7 !intel.optreport !9 {
DIR.OMP.SIMD.118:
  %0 = bitcast float* %b to <8 x float>*, !dbg !27
  %gepload = load <8 x float>, <8 x float>* %0, align 4, !dbg !27, !tbaa !28
  %1 = bitcast float* %c to <8 x float>*, !dbg !32
  %gepload24 = load <8 x float>, <8 x float>* %1, align 4, !dbg !32, !tbaa !28
  %2 = fadd fast <8 x float> %gepload24, %gepload, !dbg !33
  %3 = bitcast float* %a to <8 x float>*, !dbg !34
  store <8 x float> %2, <8 x float>* %3, align 4, !dbg !35, !tbaa !28
  %4 = getelementptr inbounds float, float* %b, i64 8, !dbg !27
  %5 = bitcast float* %4 to <8 x float>*, !dbg !27
  %gepload.1 = load <8 x float>, <8 x float>* %5, align 4, !dbg !27, !tbaa !28
  %6 = getelementptr inbounds float, float* %c, i64 8, !dbg !32
  %7 = bitcast float* %6 to <8 x float>*, !dbg !32
  %gepload24.1 = load <8 x float>, <8 x float>* %7, align 4, !dbg !32, !tbaa !28
  %8 = fadd fast <8 x float> %gepload24.1, %gepload.1, !dbg !33
  %9 = getelementptr inbounds float, float* %a, i64 8, !dbg !34
  %10 = bitcast float* %9 to <8 x float>*, !dbg !34
  store <8 x float> %8, <8 x float>* %10, align 4, !dbg !35, !tbaa !28
  %11 = getelementptr inbounds float, float* %b, i64 16, !dbg !27
  %12 = bitcast float* %11 to <8 x float>*, !dbg !27
  %gepload.2 = load <8 x float>, <8 x float>* %12, align 4, !dbg !27, !tbaa !28
  %13 = getelementptr inbounds float, float* %c, i64 16, !dbg !32
  %14 = bitcast float* %13 to <8 x float>*, !dbg !32
  %gepload24.2 = load <8 x float>, <8 x float>* %14, align 4, !dbg !32, !tbaa !28
  %15 = fadd fast <8 x float> %gepload24.2, %gepload.2, !dbg !33
  %16 = getelementptr inbounds float, float* %a, i64 16, !dbg !34
  %17 = bitcast float* %16 to <8 x float>*, !dbg !34
  store <8 x float> %15, <8 x float>* %17, align 4, !dbg !35, !tbaa !28
  %18 = getelementptr inbounds float, float* %b, i64 24, !dbg !27
  %19 = bitcast float* %18 to <8 x float>*, !dbg !27
  %gepload.3 = load <8 x float>, <8 x float>* %19, align 4, !dbg !27, !tbaa !28
  %20 = getelementptr inbounds float, float* %c, i64 24, !dbg !32
  %21 = bitcast float* %20 to <8 x float>*, !dbg !32
  %gepload24.3 = load <8 x float>, <8 x float>* %21, align 4, !dbg !32, !tbaa !28
  %22 = fadd fast <8 x float> %gepload24.3, %gepload.3, !dbg !33
  %23 = getelementptr inbounds float, float* %a, i64 24, !dbg !34
  %24 = bitcast float* %23 to <8 x float>*, !dbg !34
  store <8 x float> %22, <8 x float>* %24, align 4, !dbg !35, !tbaa !28
  %25 = getelementptr inbounds float, float* %b, i64 32, !dbg !27
  %26 = bitcast float* %25 to <8 x float>*, !dbg !27
  %gepload.4 = load <8 x float>, <8 x float>* %26, align 4, !dbg !27, !tbaa !28
  %27 = getelementptr inbounds float, float* %c, i64 32, !dbg !32
  %28 = bitcast float* %27 to <8 x float>*, !dbg !32
  %gepload24.4 = load <8 x float>, <8 x float>* %28, align 4, !dbg !32, !tbaa !28
  %29 = fadd fast <8 x float> %gepload24.4, %gepload.4, !dbg !33
  %30 = getelementptr inbounds float, float* %a, i64 32, !dbg !34
  %31 = bitcast float* %30 to <8 x float>*, !dbg !34
  store <8 x float> %29, <8 x float>* %31, align 4, !dbg !35, !tbaa !28
  %32 = getelementptr inbounds float, float* %b, i64 40, !dbg !27
  %33 = bitcast float* %32 to <8 x float>*, !dbg !27
  %gepload.5 = load <8 x float>, <8 x float>* %33, align 4, !dbg !27, !tbaa !28
  %34 = getelementptr inbounds float, float* %c, i64 40, !dbg !32
  %35 = bitcast float* %34 to <8 x float>*, !dbg !32
  %gepload24.5 = load <8 x float>, <8 x float>* %35, align 4, !dbg !32, !tbaa !28
  %36 = fadd fast <8 x float> %gepload24.5, %gepload.5, !dbg !33
  %37 = getelementptr inbounds float, float* %a, i64 40, !dbg !34
  %38 = bitcast float* %37 to <8 x float>*, !dbg !34
  store <8 x float> %36, <8 x float>* %38, align 4, !dbg !35, !tbaa !28
  %39 = getelementptr inbounds float, float* %b, i64 48, !dbg !27
  %40 = bitcast float* %39 to <8 x float>*, !dbg !27
  %gepload.6 = load <8 x float>, <8 x float>* %40, align 4, !dbg !27, !tbaa !28
  %41 = getelementptr inbounds float, float* %c, i64 48, !dbg !32
  %42 = bitcast float* %41 to <8 x float>*, !dbg !32
  %gepload24.6 = load <8 x float>, <8 x float>* %42, align 4, !dbg !32, !tbaa !28
  %43 = fadd fast <8 x float> %gepload24.6, %gepload.6, !dbg !33
  %44 = getelementptr inbounds float, float* %a, i64 48, !dbg !34
  %45 = bitcast float* %44 to <8 x float>*, !dbg !34
  store <8 x float> %43, <8 x float>* %45, align 4, !dbg !35, !tbaa !28
  %46 = getelementptr inbounds float, float* %b, i64 56, !dbg !27
  %47 = bitcast float* %46 to <8 x float>*, !dbg !27
  %gepload.7 = load <8 x float>, <8 x float>* %47, align 4, !dbg !27, !tbaa !28
  %48 = getelementptr inbounds float, float* %c, i64 56, !dbg !32
  %49 = bitcast float* %48 to <8 x float>*, !dbg !32
  %gepload24.7 = load <8 x float>, <8 x float>* %49, align 4, !dbg !32, !tbaa !28
  %50 = fadd fast <8 x float> %gepload24.7, %gepload.7, !dbg !33
  %51 = getelementptr inbounds float, float* %a, i64 56, !dbg !34
  %52 = bitcast float* %51 to <8 x float>*, !dbg !34
  store <8 x float> %50, <8 x float>* %52, align 4, !dbg !35, !tbaa !28
  %53 = getelementptr inbounds float, float* %b, i64 64, !dbg !27
  %54 = bitcast float* %53 to <8 x float>*, !dbg !27
  %gepload.8 = load <8 x float>, <8 x float>* %54, align 4, !dbg !27, !tbaa !28
  %55 = getelementptr inbounds float, float* %c, i64 64, !dbg !32
  %56 = bitcast float* %55 to <8 x float>*, !dbg !32
  %gepload24.8 = load <8 x float>, <8 x float>* %56, align 4, !dbg !32, !tbaa !28
  %57 = fadd fast <8 x float> %gepload24.8, %gepload.8, !dbg !33
  %58 = getelementptr inbounds float, float* %a, i64 64, !dbg !34
  %59 = bitcast float* %58 to <8 x float>*, !dbg !34
  store <8 x float> %57, <8 x float>* %59, align 4, !dbg !35, !tbaa !28
  %60 = getelementptr inbounds float, float* %b, i64 72, !dbg !27
  %61 = bitcast float* %60 to <8 x float>*, !dbg !27
  %gepload.9 = load <8 x float>, <8 x float>* %61, align 4, !dbg !27, !tbaa !28
  %62 = getelementptr inbounds float, float* %c, i64 72, !dbg !32
  %63 = bitcast float* %62 to <8 x float>*, !dbg !32
  %gepload24.9 = load <8 x float>, <8 x float>* %63, align 4, !dbg !32, !tbaa !28
  %64 = fadd fast <8 x float> %gepload24.9, %gepload.9, !dbg !33
  %65 = getelementptr inbounds float, float* %a, i64 72, !dbg !34
  %66 = bitcast float* %65 to <8 x float>*, !dbg !34
  store <8 x float> %64, <8 x float>* %66, align 4, !dbg !35, !tbaa !28
  %67 = getelementptr inbounds float, float* %b, i64 80, !dbg !27
  %68 = bitcast float* %67 to <8 x float>*, !dbg !27
  %gepload.10 = load <8 x float>, <8 x float>* %68, align 4, !dbg !27, !tbaa !28
  %69 = getelementptr inbounds float, float* %c, i64 80, !dbg !32
  %70 = bitcast float* %69 to <8 x float>*, !dbg !32
  %gepload24.10 = load <8 x float>, <8 x float>* %70, align 4, !dbg !32, !tbaa !28
  %71 = fadd fast <8 x float> %gepload24.10, %gepload.10, !dbg !33
  %72 = getelementptr inbounds float, float* %a, i64 80, !dbg !34
  %73 = bitcast float* %72 to <8 x float>*, !dbg !34
  store <8 x float> %71, <8 x float>* %73, align 4, !dbg !35, !tbaa !28
  %74 = getelementptr inbounds float, float* %b, i64 88, !dbg !27
  %75 = bitcast float* %74 to <8 x float>*, !dbg !27
  %gepload.11 = load <8 x float>, <8 x float>* %75, align 4, !dbg !27, !tbaa !28
  %76 = getelementptr inbounds float, float* %c, i64 88, !dbg !32
  %77 = bitcast float* %76 to <8 x float>*, !dbg !32
  %gepload24.11 = load <8 x float>, <8 x float>* %77, align 4, !dbg !32, !tbaa !28
  %78 = fadd fast <8 x float> %gepload24.11, %gepload.11, !dbg !33
  %79 = getelementptr inbounds float, float* %a, i64 88, !dbg !34
  %80 = bitcast float* %79 to <8 x float>*, !dbg !34
  store <8 x float> %78, <8 x float>* %80, align 4, !dbg !35, !tbaa !28
  %81 = getelementptr inbounds float, float* %b, i64 96, !dbg !27
  %gepload29 = load float, float* %81, align 4, !dbg !27, !tbaa !28, !llvm.access.group !36
  %82 = getelementptr inbounds float, float* %c, i64 96, !dbg !32
  %gepload30 = load float, float* %82, align 4, !dbg !32, !tbaa !28, !llvm.access.group !36
  %83 = fadd fast float %gepload30, %gepload29, !dbg !33
  %84 = getelementptr inbounds float, float* %a, i64 96, !dbg !34
  store float %83, float* %84, align 4, !dbg !35, !tbaa !28, !llvm.access.group !36
  %85 = getelementptr inbounds float, float* %b, i64 97, !dbg !27
  %gepload29.1 = load float, float* %85, align 4, !dbg !27, !tbaa !28, !llvm.access.group !36
  %86 = getelementptr inbounds float, float* %c, i64 97, !dbg !32
  %gepload30.1 = load float, float* %86, align 4, !dbg !32, !tbaa !28, !llvm.access.group !36
  %87 = fadd fast float %gepload30.1, %gepload29.1, !dbg !33
  %88 = getelementptr inbounds float, float* %a, i64 97, !dbg !34
  store float %87, float* %88, align 4, !dbg !35, !tbaa !28, !llvm.access.group !36
  %89 = getelementptr inbounds float, float* %b, i64 98, !dbg !27
  %gepload29.2 = load float, float* %89, align 4, !dbg !27, !tbaa !28, !llvm.access.group !36
  %90 = getelementptr inbounds float, float* %c, i64 98, !dbg !32
  %gepload30.2 = load float, float* %90, align 4, !dbg !32, !tbaa !28, !llvm.access.group !36
  %91 = fadd fast float %gepload30.2, %gepload29.2, !dbg !33
  %92 = getelementptr inbounds float, float* %a, i64 98, !dbg !34
  store float %91, float* %92, align 4, !dbg !35, !tbaa !28, !llvm.access.group !36
  %93 = getelementptr inbounds float, float* %b, i64 99, !dbg !27
  %gepload29.3 = load float, float* %93, align 4, !dbg !27, !tbaa !28, !llvm.access.group !36
  %94 = getelementptr inbounds float, float* %c, i64 99, !dbg !32
  %gepload30.3 = load float, float* %94, align 4, !dbg !32, !tbaa !28, !llvm.access.group !36
  %95 = fadd fast float %gepload30.3, %gepload29.3, !dbg !33
  %96 = getelementptr inbounds float, float* %a, i64 99, !dbg !34
  store float %95, float* %96, align 4, !dbg !35, !tbaa !28, !llvm.access.group !36
  ret void, !dbg !37
}

; Function Attrs: nofree noinline norecurse nosync nounwind uwtable
define dso_local void @complete_unroll(float* nocapture %a, float* nocapture readonly %b, float* nocapture readonly %c) local_unnamed_addr #1 !dbg !38 !intel.optreport !39 {
entry:
  %gepload = load float, float* %c, align 4, !dbg !48, !tbaa !28
  %gepload11 = load float, float* %b, align 4, !dbg !49, !tbaa !28
  %0 = fadd fast float %gepload, %gepload11, !dbg !50
  store float %0, float* %a, align 4, !dbg !51, !tbaa !28
  %1 = getelementptr inbounds float, float* %c, i64 1, !dbg !48
  %gepload12 = load float, float* %1, align 4, !dbg !48, !tbaa !28
  %2 = getelementptr inbounds float, float* %b, i64 1, !dbg !49
  %gepload13 = load float, float* %2, align 4, !dbg !49, !tbaa !28
  %3 = fadd fast float %gepload12, %gepload13, !dbg !50
  %4 = getelementptr inbounds float, float* %a, i64 1, !dbg !52
  store float %3, float* %4, align 4, !dbg !51, !tbaa !28
  %5 = getelementptr inbounds float, float* %c, i64 2, !dbg !48
  %gepload15 = load float, float* %5, align 4, !dbg !48, !tbaa !28
  %6 = getelementptr inbounds float, float* %b, i64 2, !dbg !49
  %gepload16 = load float, float* %6, align 4, !dbg !49, !tbaa !28
  %7 = fadd fast float %gepload15, %gepload16, !dbg !50
  %8 = getelementptr inbounds float, float* %a, i64 2, !dbg !52
  store float %7, float* %8, align 4, !dbg !51, !tbaa !28
  %9 = getelementptr inbounds float, float* %c, i64 3, !dbg !48
  %gepload18 = load float, float* %9, align 4, !dbg !48, !tbaa !28
  %10 = getelementptr inbounds float, float* %b, i64 3, !dbg !49
  %gepload19 = load float, float* %10, align 4, !dbg !49, !tbaa !28
  %11 = fadd fast float %gepload18, %gepload19, !dbg !50
  %12 = getelementptr inbounds float, float* %a, i64 3, !dbg !52
  store float %11, float* %12, align 4, !dbg !51, !tbaa !28
  %13 = getelementptr inbounds float, float* %c, i64 4, !dbg !48
  %gepload21 = load float, float* %13, align 4, !dbg !48, !tbaa !28
  %14 = getelementptr inbounds float, float* %b, i64 4, !dbg !49
  %gepload22 = load float, float* %14, align 4, !dbg !49, !tbaa !28
  %15 = fadd fast float %gepload21, %gepload22, !dbg !50
  %16 = getelementptr inbounds float, float* %a, i64 4, !dbg !52
  store float %15, float* %16, align 4, !dbg !51, !tbaa !28
  %17 = getelementptr inbounds float, float* %c, i64 5, !dbg !48
  %gepload24 = load float, float* %17, align 4, !dbg !48, !tbaa !28
  %18 = getelementptr inbounds float, float* %b, i64 5, !dbg !49
  %gepload25 = load float, float* %18, align 4, !dbg !49, !tbaa !28
  %19 = fadd fast float %gepload24, %gepload25, !dbg !50
  %20 = getelementptr inbounds float, float* %a, i64 5, !dbg !52
  store float %19, float* %20, align 4, !dbg !51, !tbaa !28
  %21 = getelementptr inbounds float, float* %c, i64 6, !dbg !48
  %gepload27 = load float, float* %21, align 4, !dbg !48, !tbaa !28
  %22 = getelementptr inbounds float, float* %b, i64 6, !dbg !49
  %gepload28 = load float, float* %22, align 4, !dbg !49, !tbaa !28
  %23 = fadd fast float %gepload27, %gepload28, !dbg !50
  %24 = getelementptr inbounds float, float* %a, i64 6, !dbg !52
  store float %23, float* %24, align 4, !dbg !51, !tbaa !28
  %25 = getelementptr inbounds float, float* %c, i64 7, !dbg !48
  %gepload30 = load float, float* %25, align 4, !dbg !48, !tbaa !28
  %26 = getelementptr inbounds float, float* %b, i64 7, !dbg !49
  %gepload31 = load float, float* %26, align 4, !dbg !49, !tbaa !28
  %27 = fadd fast float %gepload30, %gepload31, !dbg !50
  %28 = getelementptr inbounds float, float* %a, i64 7, !dbg !52
  store float %27, float* %28, align 4, !dbg !51, !tbaa !28
  ret void, !dbg !53
}

attributes #0 = { noinline nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { nofree noinline norecurse nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "linker_test_funcs.c", directory: "/iusers/karthik1/tools/binoptrpt_reader/tests")
!2 = !{}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"uwtable", i32 1}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!7 = distinct !DISubprogram(name: "vectorize", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !2)
!9 = distinct !{!"intel.optreport", !11}
!11 = !{!"intel.optreport.first_child", !12}
!12 = distinct !{!"intel.optreport", !14, !16, !20}
!14 = !{!"intel.optreport.debug_location", !15}
!15 = !DILocation(line: 2, column: 1, scope: !7)
!16 = !{!"intel.optreport.remarks", !17, !18, !19}
!17 = !{!"intel.optreport.remark", i32 15300}
!18 = !{!"intel.optreport.remark", i32 15305, !"8"}
!19 = !{!"intel.optreport.remark", i32 25603}
!20 = !{!"intel.optreport.next_sibling", !21}
!21 = distinct !{!"intel.optreport", !14, !23, !25}
!23 = !{!"intel.optreport.origin", !24}
!24 = !{!"intel.optreport.remark", i32 25519}
!25 = !{!"intel.optreport.remarks", !26, !19}
!26 = !{!"intel.optreport.remark", i32 15441, !""}
!27 = !DILocation(line: 4, column: 12, scope: !7)
!28 = !{!29, !29, i64 0}
!29 = !{!"float", !30, i64 0}
!30 = !{!"omnipotent char", !31, i64 0}
!31 = !{!"Simple C/C++ TBAA"}
!32 = !DILocation(line: 4, column: 19, scope: !7)
!33 = !DILocation(line: 4, column: 17, scope: !7)
!34 = !DILocation(line: 4, column: 5, scope: !7)
!35 = !DILocation(line: 4, column: 10, scope: !7)
!36 = distinct !{}
!37 = !DILocation(line: 5, column: 1, scope: !7)
!38 = distinct !DISubprogram(name: "complete_unroll", scope: !1, file: !1, line: 7, type: !8, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!39 = distinct !{!"intel.optreport", !41}
!41 = !{!"intel.optreport.first_child", !42}
!42 = distinct !{!"intel.optreport", !44, !46}
!44 = !{!"intel.optreport.debug_location", !45}
!45 = !DILocation(line: 9, column: 3, scope: !38)
!46 = !{!"intel.optreport.remarks", !47}
!47 = !{!"intel.optreport.remark", i32 25436, i32 4}
!48 = !DILocation(line: 10, column: 19, scope: !38)
!49 = !DILocation(line: 10, column: 12, scope: !38)
!50 = !DILocation(line: 10, column: 17, scope: !38)
!51 = !DILocation(line: 10, column: 10, scope: !38)
!52 = !DILocation(line: 10, column: 5, scope: !38)
!53 = !DILocation(line: 11, column: 1, scope: !38)
