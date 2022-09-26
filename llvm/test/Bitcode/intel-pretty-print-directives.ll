; INTEL_COLLAB
; RUN: opt -pretty-print-directives -S <%s 2>&1 | FileCheck %s

; Test src:
;
; #include <stdio.h>
; short x;
; typedef struct { short a; short *b; } S;
; S y;
;
; void f1() {
; #pragma omp target private(x) map(y.a) map(y.b[0:1])
;   ;
; }

; The IR was hand-modified to be more concise and use an invalid map-type 0x4000.

; Verify that clauses are printed in separate lines, and map-types are printed as comments..
; CHECK:        %i = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
; CHECK-NEXT:       "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
; CHECK-NEXT:       "QUAL.OMP.PRIVATE:TYPED"(ptr @x, i16 0, i32 1),
; CHECK-NEXT:       "QUAL.OMP.MAP.TOFROM"(ptr @y, ptr @y, i64 16, i64 0, ptr null, ptr null), ; MAP type: 0 = 0x0
; CHECK-NEXT:       "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr @y, ptr @y, i64 2, i64 281474976727043, ptr null, ptr null), ; MAP type: 281474976727043 = 0x1000000004003 = MEMBER_OF_1 (0x1000000000000) | FROM (0x2) | TO (0x1) | UNKNOWN (0x4000)
; CHECK-NEXT:       "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr getelementptr inbounds (%struct.S, ptr @y, i32 0, i32 1), ptr %b0, i64 2, i64 281474976710675, ptr null, ptr null) ] ; MAP type: 281474976710675 = 0x1000000000013 = MEMBER_OF_1 (0x1000000000000) | PTR_AND_OBJ (0x10) | FROM (0x2) | TO (0x1)
; CHECK-EMPTY:
; CHECK-NEXT:   call void @llvm.directive.region.exit(token %i) [ "DIR.OMP.END.TARGET"() ]
; CHECK-EMPTY:
; CHECK-NEXT:   ret void

%struct.S = type { i16, ptr }

@x = dso_local global i16 0, align 2
@y = dso_local global %struct.S zeroinitializer, align 8

define dso_local void @f1() {
entry:
  %b.load = load ptr, ptr getelementptr inbounds (%struct.S, ptr @y, i32 0, i32 1), align 8
  %b0 = getelementptr inbounds i16, ptr %b.load, i64 0
  %i = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE:TYPED"(ptr @x, i16 0, i32 1), "QUAL.OMP.MAP.TOFROM"(ptr @y, ptr @y, i64 16, i64 0, ptr null, ptr null), "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr @y, ptr @y, i64 2, i64 281474976727043, ptr null, ptr null), "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr getelementptr inbounds (%struct.S, ptr @y, i32 0, i32 1), ptr %b0, i64 2, i64 281474976710675, ptr null, ptr null) ]
  call void @llvm.directive.region.exit(token %i) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
; end INTEL_COLLAB
