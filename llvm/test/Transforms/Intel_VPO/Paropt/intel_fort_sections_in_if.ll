; INTEL_CUSTOMIZATION
; RUN: opt -opaque-pointers=0 -passes=vpo-paropt-loop-collapse -S %s | FileCheck %s

; Loop collapsing was asserting in the dominator tree update.

;  IF ( icase.EQ.1 ) THEN
;     !$OMP parallel
;     !
;     !$OMP sections private(jk)
;     DO  jk=0,nx3
;        wt(jk,0,0,ntlev) =  wt(jk,0,0,ntl2) * ytf(jk)
;     ENDDO
;        !
;     !$OMP section
;     DO  jk=0,nx3
;        wt(jk,nx2p1,nx1p1,ntlev) = wt(jk,nx2p1,nx1p1,ntl2) * ytf(jk)
;    ENDDO
;     !$OMP end sections
;     !
;     !$OMP end parallel
;  ENDIF
;  !


; CHECK-LABEL: .sloop.header.1
; CHECK-LABEL: .sloop.body.1
; CHECK-LABEL: ouitra_.sw.succBB.1
; CHECK-LABEL: ouitra_.sloop.latch.1
; CHECK-LABEL: ouitra_.sw.case0.1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank4$.0.2.4.6.8.10.12.14.40.78.80.82.84.86.104" = type { float*, i64, i64, i64, i64, i64, [4 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$.1.3.5.7.9.11.13.15.41.79.81.83.85.87.105" = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

define void @ouitra_(%"QNCA_a0$float*$rank4$.0.2.4.6.8.10.12.14.40.78.80.82.84.86.104"* %"ouitra_$WT", %"QNCA_a0$float*$rank1$.1.3.5.7.9.11.13.15.41.79.81.83.85.87.105"* %"ouitra_$YTF", i32* %"ouitra_$NTLEV", i32* %"ouitra_$NX2P1", i32* %"ouitra_$NX1P1", i32* %"ouitra_$NTL2", i32* %"ouitra_$NX3") {
alloca_0:
  br i1 undef, label %DIR.OMP.PARALLEL.9, label %bb11_endif

DIR.OMP.END.SECTION.1:                            ; preds = %DIR.OMP.SECTIONS.7
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.END.SECTION.2

DIR.OMP.END.SECTION.2:                            ; preds = %DIR.OMP.END.SECTION.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %DIR.OMP.END.SECTION.4

DIR.OMP.END.SECTION.4:                            ; preds = %DIR.OMP.END.SECTION.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.END.SECTION.5

DIR.OMP.END.SECTION.5:                            ; preds = %DIR.OMP.END.SECTION.4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SECTIONS"() ]
  br label %DIR.OMP.END.SECTIONS.6

DIR.OMP.END.SECTIONS.6:                           ; preds = %DIR.OMP.END.SECTION.5
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]
  br label %bb11_endif

bb_new4:                                          ; preds = %DIR.OMP.PARALLEL.9
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTIONS"(), "QUAL.OMP.PRIVATE"(i32* undef) ]
  br label %DIR.OMP.SECTIONS.7

DIR.OMP.SECTIONS.7:                               ; preds = %bb_new4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %DIR.OMP.END.SECTION.1

DIR.OMP.PARALLEL.9:                               ; preds = %alloca_0
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* %"ouitra_$NX1P1"), "QUAL.OMP.SHARED"(i32* %"ouitra_$NX2P1"), "QUAL.OMP.SHARED"(%"QNCA_a0$float*$rank1$.1.3.5.7.9.11.13.15.41.79.81.83.85.87.105"* %"ouitra_$YTF"), "QUAL.OMP.SHARED"(i32* %"ouitra_$NTL2"), "QUAL.OMP.SHARED"(i32* %"ouitra_$NTLEV"), "QUAL.OMP.SHARED"(%"QNCA_a0$float*$rank4$.0.2.4.6.8.10.12.14.40.78.80.82.84.86.104"* %"ouitra_$WT"), "QUAL.OMP.SHARED"(i32* %"ouitra_$NX3"), "QUAL.OMP.PRIVATE"(i32* undef) ]
  br label %bb_new4

bb11_endif:                                       ; preds = %DIR.OMP.END.SECTIONS.6, %alloca_0
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
; end INTEL_CUSTOMIZATION
