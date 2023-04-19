; RUN: opt -passes=instcombine -S %s | FileCheck %s

; On offload targets, malloc may be expected to return zero.
; Ensure that the null check is not removed.
; The module flags must be checked for OpenMP, as the function itself
; may not have OpenMP regions.

;__attribute__((noinline)) int func() {
;  char *foo = (char *)malloc(256);
;  if (!foo) return 1;
;  return 0;
;}
;int allocFull(size_t size) {
;  int count;
;#pragma omp target teams map(count)
;  count = func();
;  return count;
;}


; CHECK-LABEL: _Z4funcv
; CHECK: [[REZ:%.*]] = call{{.*}}malloc
; CHECK-NEXT: icmp{{.*}}[[REZ]], null

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent mustprogress noinline nounwind
define protected spir_func noundef i32 @_Z9allocFullm(i64 noundef %size) local_unnamed_addr #0 {
DIR.OMP.TARGET.310:
  %count = alloca i32, align 4
  %count.ascast = addrspacecast ptr %count to ptr addrspace(4)
  %count.ascast.addr = alloca ptr addrspace(4), align 8
  %count.ascast.addr5 = alloca ptr addrspace(4), align 8
  store ptr addrspace(4) %count.ascast, ptr %count.ascast.addr5, align 8
  %end.dir.temp7 = alloca i1, align 1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %count.ascast, ptr addrspace(4) %count.ascast, i64 4, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %count.ascast, ptr %count.ascast.addr5),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp7) ]

  %temp.load8 = load volatile i1, ptr %end.dir.temp7, align 1
  br i1 %temp.load8, label %DIR.OMP.END.TARGET.9, label %DIR.OMP.TEAMS.6

DIR.OMP.TEAMS.6:                                  ; preds = %DIR.OMP.TARGET.310
  %count.ascast6 = load volatile ptr addrspace(4), ptr %count.ascast.addr5, align 8
  store ptr addrspace(4) %count.ascast6, ptr %count.ascast.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %count.ascast6, i32 0, i32 1),
    "QUAL.OMP.OPERAND.ADDR"(ptr addrspace(4) %count.ascast6, ptr %count.ascast.addr),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.TEAMS.7, label %DIR.OMP.TEAMS.5

DIR.OMP.TEAMS.5:                                  ; preds = %DIR.OMP.TEAMS.6
  %count.ascast4 = load volatile ptr addrspace(4), ptr %count.ascast.addr, align 8
  %call = call spir_func noundef i32 @_Z4funcv() #4
  store i32 %call, ptr addrspace(4) %count.ascast4, align 4, !tbaa !9
  br label %DIR.OMP.END.TEAMS.7

DIR.OMP.END.TEAMS.7:                              ; preds = %DIR.OMP.TEAMS.5, %DIR.OMP.TEAMS.6
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  br label %DIR.OMP.END.TARGET.9

DIR.OMP.END.TARGET.9:                             ; preds = %DIR.OMP.END.TEAMS.7, %DIR.OMP.TARGET.310
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %2 = load i32, ptr addrspace(4) %count.ascast, align 4, !tbaa !9
  ret i32 %2
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent mustprogress noinline nounwind
define protected spir_func noundef i32 @_Z4funcv() local_unnamed_addr #2 {
entry:
  %call = call spir_func noalias ptr addrspace(4) @malloc(i64 noundef 256) #5
  %tobool = icmp ne ptr addrspace(4) %call, null
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ 0, %if.end ], [ 1, %if.then ]
  ret i32 %retval.0
}

; Function Attrs: convergent inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0)
declare spir_func noalias noundef ptr addrspace(4) @malloc(i64 noundef) local_unnamed_addr #3

attributes #0 = { convergent mustprogress noinline nounwind "may-have-openmp-directive"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent mustprogress noinline nounwind }
attributes #3 = { convergent inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #4 = { convergent nounwind }
attributes #5 = { convergent nounwind allocsize(0) }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 2050, i32 111935567, !"_Z9allocFullm", i32 12, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C++ TBAA"}
