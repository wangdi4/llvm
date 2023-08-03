; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; void foo() {
; #pragma omp parallel
;   {
;     int x = 0;
;     printf ("x = %d\n", x);
;   }
; }

; CHECK-NOT: %{{[0-9]+}} = call token @llvm.directive.region.entry() {{.*}}
; CHECK-NOT: call void @llvm.directive.region.exit(token %{{[0-9]+}}) {{.*}}
; CHECK: call void {{.*}} @__kmpc_fork_call
; CHECK: call void @llvm.dbg.declare(metadata ptr %x{{.*}}, metadata [[VAR:![0-9]+]]

; CHECK: [[SUB:![0-9]+]] = distinct !DISubprogram(name: "foo{{.*}}PARALLEL
; CHECK: [[LB1:![0-9]+]] = distinct !DILexicalBlock(scope: [[SUB]], file: !2, line: 4, column: 1)
; CHECK: [[VAR]] = !DILocalVariable(name: "x", scope: [[LB1]],

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"x = %d\0A\00", align 1, !dbg !0
@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 !dbg !16 {
entry:
  %x = alloca i32, align 4
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  br label %DIR.OMP.PARALLEL.2, !dbg !20

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  br label %DIR.OMP.PARALLEL.11, !dbg !20

DIR.OMP.PARALLEL.11:                              ; preds = %DIR.OMP.PARALLEL.2
  br label %DIR.OMP.PARALLEL.12, !dbg !20

DIR.OMP.PARALLEL.12:                              ; preds = %DIR.OMP.PARALLEL.11
  br label %DIR.OMP.PARALLEL.13, !dbg !20

DIR.OMP.PARALLEL.13:                              ; preds = %DIR.OMP.PARALLEL.12
  br label %DIR.OMP.PARALLEL.14, !dbg !20

DIR.OMP.PARALLEL.14:                              ; preds = %DIR.OMP.PARALLEL.13
  br label %DIR.OMP.PARALLEL.14.split, !dbg !20

DIR.OMP.PARALLEL.14.split:                        ; preds = %DIR.OMP.PARALLEL.14
  %end.dir.temp = alloca i1, align 1, !dbg !20
  br label %DIR.OMP.PARALLEL.16, !dbg !20

DIR.OMP.PARALLEL.16:                              ; preds = %DIR.OMP.PARALLEL.14.split
  br label %DIR.OMP.PARALLEL.27, !dbg !20

DIR.OMP.PARALLEL.27:                              ; preds = %DIR.OMP.PARALLEL.16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %x, i32 0, i32 1),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
, !dbg !20
  br label %DIR.OMP.PARALLEL.38, !dbg !20

DIR.OMP.PARALLEL.38:                              ; preds = %DIR.OMP.PARALLEL.27
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1, !dbg !20
  %cmp = icmp ne i1 %temp.load, false, !dbg !20
  br i1 %cmp, label %DIR.OMP.END.PARALLEL.4.split, label %1, !dbg !20

1:                                                ; preds = %DIR.OMP.PARALLEL.38
  br label %DIR.OMP.PARALLEL.3, !dbg !20

DIR.OMP.PARALLEL.3:                               ; preds = %1
  call void @llvm.dbg.declare(metadata ptr %x, metadata !22, metadata !DIExpression()), !dbg !25
  store i32 0, ptr %x, align 4, !dbg !25
  %2 = load i32, ptr %x, align 4, !dbg !26
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %2) #1, !dbg !27
  br label %DIR.OMP.END.PARALLEL.4, !dbg !27

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.PARALLEL.3
  br label %DIR.OMP.END.PARALLEL.4.split, !dbg !20

DIR.OMP.END.PARALLEL.4.split:                     ; preds = %DIR.OMP.PARALLEL.38, %DIR.OMP.END.PARALLEL.4
  br label %DIR.OMP.END.PARALLEL.49, !dbg !20

DIR.OMP.END.PARALLEL.49:                          ; preds = %DIR.OMP.END.PARALLEL.4.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
, !dbg !20
  br label %DIR.OMP.END.PARALLEL.5, !dbg !20

DIR.OMP.END.PARALLEL.5:                           ; preds = %DIR.OMP.END.PARALLEL.49
  ret void, !dbg !28
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #2

declare dso_local i32 @printf(ptr noundef, ...) #3

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.dbg.cu = !{!7}
!llvm.module.flags = !{!9, !10, !11, !12, !13, !14}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(scope: null, file: !2, line: 7, type: !3, isLocal: true, isDefinition: true)
!2 = !DIFile(filename: "parallel_with_local.c", directory: "/full/path/of/test/directory")
!3 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 64, elements: !5)
!4 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!5 = !{!6}
!6 = !DISubrange(count: 8)
!7 = distinct !DICompileUnit(language: DW_LANG_C11, file: !2, producer: "clang version 17.0.0", runtimeVersion: 0, emissionKind: FullDebug, globals: !8, splitDebugInlining: false, nameTableKind: None)
!8 = !{!0}
!9 = !{i32 7, !"Dwarf Version", i32 4}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{i32 1, !"wchar_size", i32 4}
!12 = !{i32 7, !"openmp", i32 51}
!13 = !{i32 7, !"uwtable", i32 2}
!14 = !{i32 7, !"frame-pointer", i32 2}
!16 = distinct !DISubprogram(name: "foo", scope: !2, file: !2, line: 3, type: !17, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !7, retainedNodes: !19)
!17 = !DISubroutineType(types: !18)
!18 = !{null}
!19 = !{}
!20 = !DILocation(line: 4, column: 1, scope: !21)
!21 = distinct !DILexicalBlock(scope: !16, file: !2, line: 4, column: 1)
!22 = !DILocalVariable(name: "x", scope: !23, file: !2, line: 6, type: !24)
!23 = distinct !DILexicalBlock(scope: !21, file: !2, line: 5, column: 3)
!24 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!25 = !DILocation(line: 6, column: 9, scope: !23)
!26 = !DILocation(line: 7, column: 25, scope: !23)
!27 = !DILocation(line: 7, column: 5, scope: !23)
!28 = !DILocation(line: 9, column: 1, scope: !16)
