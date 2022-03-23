; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck %s -check-prefix=TFORM
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR

; float za[10];
; void bar(float*);
;
; void foo() {
;   #pragma omp parallel firstprivate(za)
;   #pragma omp target map(to:za[0:1])
;   bar(&za[0]);
; }

; PREPR:      store [10 x float]* @za, [10 x float]** [[ZA_ADDR1:%za.addr[0-9]*]], align 8
; PREPR:      call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; PREPR_SAME:   "QUAL.OMP.FIRSTPRIVATE"([10 x float]* @za)
; PREPR-SAME:   "QUAL.OMP.OPERAND.ADDR"([10 x float]* @za, [10 x float]** [[ZA_ADDR1]])
; PREPR:      [[ZA1:%za[0-9]*]] = load volatile [10 x float]*, [10 x float]** [[ZA_ADDR1]], align 8
; PREPR:      {{%[^ ]+}} = getelementptr inbounds [10 x float], [10 x float]* [[ZA1]], i64 0, i64 0
; PREPR:      [[ZA_GEP1:%[^ ]+]] = getelementptr inbounds [10 x float], [10 x float]* [[ZA1]], i64 0, i64 0
; PREPR:      call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"()
; PREPR-SAME:   "QUAL.OMP.MAP.TO"([10 x float]* [[ZA1]], float* [[ZA_GEP1]], i64 4, i64 33, i8* null, i8* null)
; PREPR-SAME:   "QUAL.OMP.OPERAND.ADDR"(float* [[ZA_GEP1]], float** %{{[^ ]+}})
; PREPR-SAME:   "QUAL.OMP.OPERAND.ADDR"([10 x float]* [[ZA1]], [10 x float]** [[ZA_ADDR2:%za.addr[0-9]*]])
; PREPR:      [[ZA2:%za[0-9]*]] = load volatile [10 x float]*, [10 x float]** [[ZA_ADDR2]], align 8
; PREPR:      [[ZA_GEP2:%[^ ]+]] = getelementptr inbounds [10 x float], [10 x float]* [[ZA2]], i64 0, i64 0
; PREPR:      call void @bar(float* [[ZA_GEP2]])

; TFORM-NOT: CodeExtractor captured out-of-clause argument

; Check that the outlined function for the target region takes a single argument for za,
; and doesn't try to pass in the section-ptr GEP as an argument.
; TFORM: define internal void @__omp_offloading_{{.*}}foo{{.*}}([10 x float]* [[ZA:%za[0-9]*]])
; TFORM: [[ZA_GEP3:%[^ ]+]] = getelementptr inbounds [10 x float], [10 x float]* [[ZA]], i64 0, i64 0
; TFORM: call void @bar(float* [[ZA_GEP3]])

target device_triples = "x86_64"

@za = external global [10 x float]

define void @foo() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE"([10 x float]* @za) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"([10 x float]* @za, float* getelementptr inbounds ([10 x float], [10 x float]* @za, i64 0, i64 0), i64 4, i64 33, i8* null, i8* null) ]

  call void @bar(float* getelementptr inbounds ([10 x float], [10 x float]* @za, i64 0, i64 0))

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

; Function Attrs: nounwind
declare void @bar(float*) #0

attributes #0 = { nounwind }
!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66309, i32 64032527, !"foo", i32 6, i32 0, i32 0}
