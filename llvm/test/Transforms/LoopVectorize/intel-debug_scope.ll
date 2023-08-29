; Checks that VPlan does not make inconsistent debug info like this:
;	mismatched subprogram between llvm.dbg.value variable and !dbg attachment
;	  call void @llvm.dbg.value(metadata ptr addrspace(1) %0, metadata !5, metadata !DIExpression()), !dbg !1
;	label %vector.body
;	void (ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr)* @_ZGVdN8uuuuuuuuuuuu_TS10SimpleVaddIfE
;	!5 = !DILocalVariable(name: "Ptr", arg: 2, scope: !6)
;	!6 = distinct !DISubprogram(name: "__init", scope: null, spFlags: DISPFlagDefinition, unit: !3)
;	!1 = !DILocation(line: 0, scope: !2)
;	!2 = distinct !DISubprogram(name: "_ZTS10SimpleVaddIfE", scope: null, spFlags: DISPFlagDefinition, unit: !3)
;	LLVM ERROR: Broken module found, compilation aborted!

; TODO: VPValue-based CG cannot preserve debug info since they are not represented in VPlan.
; Check JIRA : CMPLRLLVM-9901
; RUN: opt -S -passes=vplan-vec < %s | FileCheck %s
; REQUIRES: asserts
; CHECK: call void @llvm.dbg.value(metadata ptr addrspace(1) {{.*}}, metadata ![[BR_LOC1:[0-9]+]], metadata !DIExpression()), !dbg ![[BR_LOC2:[0-9]+]]
; CHECK: ![[BR_LOC1]] = !DILocalVariable(name: "Ptr", arg: 2, scope: ![[BR_LOC:[0-9]+]])
; CHECK: ![[BR_LOC]] = distinct !DISubprogram(name: "__init",
; CHECK: ![[BR_LOC2]] = !DILocation(line: 0, scope: ![[BR_LOC]])

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"A" = type { %"B" }
%"B" = type { [1 x i64] }

declare void @_ZTS10SimpleVaddIiE(ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr) local_unnamed_addr #0

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare void @_ZTS10SimpleVaddIfE(ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr) local_unnamed_addr #2

; Function Attrs: readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #3

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

declare [7 x i64] @WG.boundaries._ZTS10SimpleVaddIiE(ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr)

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

declare [7 x i64] @WG.boundaries._ZTS10SimpleVaddIfE(ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr)

define void @_ZGVdN8uuuuuuuuuuuu_TS10SimpleVaddIfE(ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr) local_unnamed_addr #2 {
  br label %simd.begin.region

simd.begin.region:                                ; preds = %12
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(ptr addrspace(1) %0, ptr %1, ptr %2, ptr %3, ptr addrspace(1) %4, ptr %5, ptr %6, ptr %7, ptr addrspace(1) %8, ptr %9, ptr %10, ptr %11) ]
  br label %simd.loop, !dbg !1550

simd.loop:                                        ; preds = %simd.loop, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop ]
  call void @llvm.dbg.value(metadata ptr addrspace(1) %0, metadata !1551, metadata !DIExpression()), !dbg !1561
  %indvar = add nuw i32 %index, 1
  br i1 false, label %simd.loop, label %simd.end.region, !llvm.loop !1563

simd.end.region:                                  ; preds = %simd.loop
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void, !dbg !1565
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

declare void @_ZGVdN8uuuuuuuuuuuu_TS10SimpleVaddIiE(ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr, ptr addrspace(1), ptr, ptr, ptr) local_unnamed_addr #0

attributes #0 = { "vector-variants"="_ZGVdN8uuuuuuuuuuuu__ZTS10SimpleVaddIiE" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { "vector-variants"="_ZGVdN8uuuuuuuuuuuu__ZTS10SimpleVaddIfE" }
attributes #3 = { readnone }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.dbg.cu = !{}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = distinct !DISubprogram(name: "_ZTS10SimpleVaddIfE", unit: !233)
!2 = !DIFile(filename: "simple_vector_add.cpp", directory: ".")
!233 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !2)
!1371 = !DIFile(filename: "handler.hpp", directory: ".")
!1550 = !DILocation(line: 0, scope: !1)
!1551 = !DILocalVariable(name: "Ptr", arg: 2, scope: !1552)
!1552 = distinct !DISubprogram(name: "__init", unit: !233)
!1561 = !DILocation(line: 0, scope: !1552)
!1563 = distinct !{!1563, !1564}
!1564 = !{!"llvm.loop.unroll.disable"}
!1565 = !DILocation(line: 571, column: 16, scope: !1566)
!1566 = !DILexicalBlockFile(scope: !1, file: !1371, discriminator: 0)
