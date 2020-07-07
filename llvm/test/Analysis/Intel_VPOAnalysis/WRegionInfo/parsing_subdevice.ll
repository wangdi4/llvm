; REQUIRES: asserts
; RUN: opt -vpo-wrncollection -analyze -debug -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -analyze -debug -S < %s 2>&1 | FileCheck %s
;
; Test src: Input IR was hand modified because FE does not yet handle subdevice
; void foo() {
; double x = 0.0;
; double *out = &x;
; #pragma omp target data map(tofrom:out[0:1]) device(2)
;    out[0] = 123.0;
; }
;
; Check that subdevice was parsed.
; Check for the debug string.
; CHECK: SUBDEVICE(i32 {{.*}}:i32 {{.*}})
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 !dbg !7 {
entry:
  %x = alloca double, align 8
  %out = alloca double*, align 8
  call void @llvm.dbg.declare(metadata double* %x, metadata !10, metadata !DIExpression()), !dbg !12
  store double 0.000000e+00, double* %x, align 8, !dbg !12
  call void @llvm.dbg.declare(metadata double** %out, metadata !13, metadata !DIExpression()), !dbg !15
  store double* %x, double** %out, align 8, !dbg !15
  %0 = load double*, double** %out, align 8, !dbg !16
  %1 = load double*, double** %out, align 8, !dbg !17
  %arrayidx = getelementptr inbounds double, double* %1, i64 0, !dbg !17
  br label %DIR.OMP.TARGET.DATA.1, !dbg !16

DIR.OMP.TARGET.DATA.1:                            ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.SUBDEVICE"(i32 2, i32 4), "QUAL.OMP.MAP.TOFROM"(double* %0, double* %arrayidx, i64 8, i64 35) ], !dbg !16
  br label %DIR.OMP.TARGET.DATA.2, !dbg !19

DIR.OMP.TARGET.DATA.2:                            ; preds = %DIR.OMP.TARGET.DATA.1
  %3 = load double*, double** %out, align 8, !dbg !19
  %arrayidx1 = getelementptr inbounds double, double* %3, i64 0, !dbg !19
  store double 1.230000e+02, double* %arrayidx1, align 8, !dbg !20
  br label %DIR.OMP.END.TARGET.DATA.3, !dbg !16

DIR.OMP.END.TARGET.DATA.3:                        ; preds = %DIR.OMP.TARGET.DATA.2
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET.DATA"() ], !dbg !16
  br label %DIR.OMP.END.TARGET.DATA.4, !dbg !21

DIR.OMP.END.TARGET.DATA.4:                        ; preds = %DIR.OMP.END.TARGET.DATA.3
  ret void, !dbg !21
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test.c", directory: "/path/to/SubdeviceParsing")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 9.0.0"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{null}
!10 = !DILocalVariable(name: "x", scope: !7, file: !1, line: 2, type: !11)
!11 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!12 = !DILocation(line: 2, column: 9, scope: !7)
!13 = !DILocalVariable(name: "out", scope: !7, file: !1, line: 3, type: !14)
!14 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!15 = !DILocation(line: 3, column: 10, scope: !7)
!16 = !DILocation(line: 4, column: 2, scope: !7)
!17 = !DILocation(line: 4, column: 37, scope: !18)
!18 = distinct !DILexicalBlock(scope: !7, file: !1, line: 4, column: 2)
!19 = !DILocation(line: 5, column: 4, scope: !18)
!20 = !DILocation(line: 5, column: 11, scope: !18)
!21 = !DILocation(line: 6, column: 2, scope: !7)
