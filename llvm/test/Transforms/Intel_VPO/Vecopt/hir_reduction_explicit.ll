;RUN: opt -hir-ssa-deconstruction -VPODriverHIR -hir-cg -mem2reg -S %s | FileCheck %s

; CHECK: loop
; CHECK: phi <8 x float> [ zeroinitializer
; CHECK: fadd <8 x float>
; CHECK: afterloop
; CHECK: shufflevector <8 x float>
; CHECK: shufflevector <8 x float>
; CHECK: fadd <4 x float>
; CHECK: shufflevector <4 x float>
; CHECK: shufflevector <4 x float>
; CHECK: fadd <2 x float>
; CHECK: extractelement <2 x float>
; CHECK: extractelement <2 x float>
; CHECK: fadd float
; CHECK: br label %for.end

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"x = %f\0A\00", align 1

; Function Attrs: nounwind uwtable
define float @foo(float* nocapture %a) #0 {
entry:
  %x = alloca float, align 4
  tail call void @llvm.dbg.value(metadata float* %a, i64 0, metadata !13, metadata !14), !dbg !15
  tail call void @llvm.dbg.value(metadata float 0.000000e+00, i64 0, metadata !16, metadata !14), !dbg !17
  store float 0.000000e+00, float* %x, align 4, !dbg !17
  br label %entry.split

entry.split:
  tail call void @llvm.intel.directive(metadata !40), !dbg !18
  call void @llvm.intel.directive.qual.opnd.i32(metadata !42, i32 8)
  tail call void @llvm.dbg.value(metadata float* %x, i64 0, metadata !16, metadata !14), !dbg !17
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !41, float* nonnull %x), !dbg !18
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END"), !dbg !18
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !19, metadata !14), !dbg !21
  %x.promoted = load float, float* %x, align 4, !dbg !22
  br label %for.body, !dbg !26

for.body:                                         ; preds = %for.body, %entry.split
  %indvars.iv = phi i64 [ 0, %entry.split ], [ %indvars.iv.next, %for.body ], !dbg !27
  %add7 = phi float [ %x.promoted, %entry.split ], [ %add, %for.body ], !dbg !27
  %add = fadd float %add7, 0.5, !dbg !34
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !26
  %exitcond = icmp eq i64 %indvars.iv.next, 1000, !dbg !26
  br i1 %exitcond, label %for.end, label %for.body, !dbg !26

for.end:                                          ; preds = %for.body
  store float %add, float* %x, align 4, !dbg !22
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD"), !dbg !35
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END"), !dbg !35
  call void @llvm.dbg.value(metadata float* %x, i64 0, metadata !16, metadata !14), !dbg !17
  %conv6 = fpext float %add to double, !dbg !36
  %call = call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), double %conv6) #4, !dbg !37
  call void @llvm.dbg.value(metadata float* %x, i64 0, metadata !16, metadata !14), !dbg !17
  %x1 = load float, float* %x, align 4, !dbg !38
  ret float %x1, !dbg !39
}

; Function Attrs: nounwind argmemonly
declare void @llvm.intel.directive(metadata) #1

declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32)

; Function Attrs: nounwind argmemonly
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) #2

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #3

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }
attributes #2 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!9, !10}
!llvm.dbg.intel.emit_class_debug_always = !{!11}
!llvm.ident = !{!12}

!0 = distinct !DICompileUnit(language: DW_LANG_C89, file: !1, producer: "clang version 3.8.0 (branches/vpo 1936)", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2)
!1 = !DIFile(filename: "simdloop.c", directory: "/export/iusers/xtian/llvm_temp")
!2 = !{}
!3 = !{!4}
!4 = distinct !DISubprogram(unit: !0, name: "foo", scope: !1, file: !1, line: 2, type: !5, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: false,  variables: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8}
!7 = !DIBasicType(name: "float", size: 32, align: 32, encoding: DW_ATE_float)
!8 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64, align: 64)
!9 = !{i32 2, !"Dwarf Version", i32 4}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{!"true"}
!12 = !{!"clang version 3.8.0 (branches/vpo 1936)"}
!13 = !DILocalVariable(name: "a", arg: 1, scope: !4, file: !1, line: 2, type: !8)
!14 = !DIExpression()
!15 = !DILocation(line: 2, column: 18, scope: !4)
!16 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 5, type: !7)
!17 = !DILocation(line: 5, column: 9, scope: !4)
!18 = !DILocation(line: 7, column: 9, scope: !4)
!19 = !DILocalVariable(name: "k", scope: !4, file: !1, line: 4, type: !20)
!20 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!21 = !DILocation(line: 4, column: 9, scope: !4)
!22 = !DILocation(line: 10, column: 7, scope: !23)
!23 = distinct !DILexicalBlock(scope: !24, file: !1, line: 8, column: 28)
!24 = distinct !DILexicalBlock(scope: !25, file: !1, line: 8, column: 3)
!25 = distinct !DILexicalBlock(scope: !4, file: !1, line: 8, column: 3)
!26 = !DILocation(line: 8, column: 3, scope: !25)
!27 = !DILocation(line: 8, column: 13, scope: !28)
!28 = !DILexicalBlockFile(scope: !29, file: !1, discriminator: 2)
!29 = !DILexicalBlockFile(scope: !24, file: !1, discriminator: 1)
!30 = !DILocation(line: 9, column: 13, scope: !23)
!31 = !DILocation(line: 9, column: 15, scope: !23)
!32 = !DILocation(line: 9, column: 5, scope: !23)
!33 = !DILocation(line: 9, column: 11, scope: !23)
!34 = !DILocation(line: 10, column: 11, scope: !23)
!35 = !DILocation(line: 11, column: 3, scope: !25)
!36 = !DILocation(line: 12, column: 21, scope: !4)
!37 = !DILocation(line: 12, column: 2, scope: !4)
!38 = !DILocation(line: 13, column: 9, scope: !4)
!39 = !DILocation(line: 13, column: 2, scope: !4)
!40 = !{!"DIR.OMP.SIMD"}
!41 = !{!"QUAL.OMP.REDUCTION.ADD"}
!42 = !{!"QUAL.OMP.SIMDLEN"}
