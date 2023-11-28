; RUN: opt -passes='function(vpo-paropt-guard-memory-motion,vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-guard-memory-motion-for-scan -S %s -o %t1.ll && FileCheck --input-file=%t1.ll %s
; RUN: opt -passes="function(vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt" %t1.ll -S -o %t2.ll && FileCheck --input-file=%t2.ll %s --check-prefix=PAROPT
; RUN: opt -passes="function(vpo-cfg-restructuring,vpo-rename-operands)" %t2.ll -S -o %t3.ll && FileCheck --input-file=%t3.ll %s --check-prefix=RENAME
; RUN: opt -passes="function(vpo-restore-operands)" %t3.ll -S -o %t4.ll && FileCheck --input-file=%t4.ll %s --check-prefix=RESTORE

; RUN: opt -passes='function(vpo-paropt-guard-memory-motion,vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-guard-memory-motion-for-scan -vpo-paropt-disable-guard-memory-motion-for-scan -S < %s 2>&1 | FileCheck %s --check-prefix=DISABLE_SCAN

; Test to verify the functionality of VPOParoptGuardMemoryMotion and VPORenameOperands passes.

; Test src:
; typedef struct Dummy {
;   float x;
;   float y;
; } Dummy;
;
; void my_add(Dummy &lhs, const Dummy &rhs) { lhs.x += rhs.x; lhs.y = rhs.y; }
; void my_init(Dummy &t) { t.x = 0; t.y = 0; }
;
; #pragma omp declare reduction(my_scan_add: Dummy : my_add(omp_out, omp_in)) initializer(my_init(omp_priv))
;
; float uds(float* m_scan_vec, float* m_out_vec, float* m_in, int c_size)
; {
;   Dummy red;
;     red.x = m_scan_vec[0];
;     red.y = m_scan_vec[0];
;     #pragma omp simd reduction(inscan, my_scan_add: red)
;     for (int i = 0; i < c_size; ++i) {
;         m_out_vec[i] = red.x;
;         #pragma omp scan exclusive(red)
;         my_add(red, {m_in[i], m_in[i]});
;     }
;     return red.x;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Dummy = type { float, float }

@"@tid.addr" = external global i32

define float @_Z3udsPfS_S_i(ptr %m_scan_vec, ptr %m_out_vec, ptr %m_in, i32 %c_size) {
; CHECK: [[RED:%.*]] = alloca %struct.Dummy
; CHECK: store ptr [[RED]], ptr [[RED_ADDR1:%.*]],
; CHECK: "DIR.OMP.SIMD"{{.*}}"QUAL.OMP.REDUCTION.UDR:INSCAN.TYPED"(ptr [[RED]], %struct.Dummy {{.*}} ]
; CHECK: [[RED2:%.*]] = load volatile ptr, ptr [[RED_ADDR1]],
; CHECK: store ptr [[RED2]], ptr [[RED_ADDR1:%.*]],
; CHECK: "DIR.VPO.GUARD.MEM.MOTION"(){{.*}}"QUAL.OMP.LIVEIN"(ptr [[RED2]]){{.*}}"QUAL.OMP.OPERAND.ADDR"(ptr [[RED2]], ptr [[RED_ADDR1]])
; CHECK: load volatile ptr, ptr [[RED_ADDR1]]
; CHECK: "DIR.VPO.END.GUARD.MEM.MOTION"
; CHECK: "DIR.OMP.SCAN"
; CHECK: "DIR.OMP.END.SCAN"
; CHECK: store ptr [[RED2]], ptr [[RED_ADDR2:%.*]],
; CHECK: "DIR.VPO.GUARD.MEM.MOTION"(){{.*}}"QUAL.OMP.LIVEIN"(ptr [[RED2]]){{.*}}"QUAL.OMP.OPERAND.ADDR"(ptr [[RED2]], ptr [[RED_ADDR2]])
; CHECK: "DIR.VPO.END.GUARD.MEM.MOTION"
; CHECK: "DIR.OMP.END.SIMD"

; PAROPT: %ref.tmp.priv = alloca %struct.Dummy
; PAROPT: [[RED:%.*]] = alloca %struct.Dummy
; PAROPT: "DIR.OMP.SIMD"{{.*}}"QUAL.OMP.REDUCTION.UDR:INSCAN.TYPED"(ptr [[RED]], %struct.Dummy {{.*}} ]
; PAROPT: "DIR.VPO.GUARD.MEM.MOTION"(){{.*}}"QUAL.OMP.LIVEIN"(ptr [[RED]]) ]
; PAROPT: "DIR.VPO.END.GUARD.MEM.MOTION"
; PAROPT: "DIR.OMP.SCAN"{{.*}}"QUAL.OMP.EXCLUSIVE"(ptr [[RED]], {{.*}} ]
; PAROPT: "DIR.OMP.END.SCAN"
; PAROPT: "DIR.VPO.GUARD.MEM.MOTION"(){{.*}}"QUAL.OMP.LIVEIN"(ptr [[RED]]) ]
; PAROPT: "DIR.VPO.END.GUARD.MEM.MOTION"
; PAROPT: "DIR.OMP.END.SIMD"

; RENAME: %ref.tmp.priv = alloca %struct.Dummy
; RENAME: [[RED:%.*]] = alloca %struct.Dummy
; RENAME: "DIR.OMP.SIMD"{{.*}}"QUAL.OMP.REDUCTION.UDR:INSCAN.TYPED"(ptr [[RED]], %struct.Dummy {{.*}})
; RENAME: store ptr [[RED]], ptr [[RED_ADDR1:%.*]],
; RENAME: "DIR.VPO.GUARD.MEM.MOTION"(){{.*}}"QUAL.OMP.LIVEIN"(ptr [[RED]]){{.*}}"QUAL.OMP.OPERAND.ADDR"(ptr [[RED]], ptr [[RED_ADDR1]])
; RENAME: load volatile ptr, ptr [[RED_ADDR1]]
; RENAME: "DIR.VPO.END.GUARD.MEM.MOTION"
; RENAME: "DIR.OMP.SCAN"
; RENAME: "DIR.OMP.END.SCAN"
; RENAME: store ptr [[RED]], ptr [[RED_ADDR2:%.*]],
; RENAME: "DIR.VPO.GUARD.MEM.MOTION"(){{.*}}"QUAL.OMP.LIVEIN"(ptr [[RED]]){{.*}}"QUAL.OMP.OPERAND.ADDR"(ptr [[RED]], ptr [[RED_ADDR2]])
; RENAME: "DIR.VPO.END.GUARD.MEM.MOTION"
; RENAME: "DIR.OMP.END.SIMD"

; RESTORE: %ref.tmp.priv = alloca %struct.Dummy
; RESTORE: [[RED:%.*]] = alloca %struct.Dummy
; RESTORE: "DIR.OMP.SIMD"{{.*}}"QUAL.OMP.REDUCTION.UDR:INSCAN.TYPED"(ptr [[RED]], %struct.Dummy {{.*}})
; RESTORE: "DIR.VPO.GUARD.MEM.MOTION"(){{.*}}"QUAL.OMP.LIVEIN"(ptr [[RED]]) ]
; RESTORE: "DIR.VPO.END.GUARD.MEM.MOTION"
; RESTORE: "DIR.OMP.SCAN"
; RESTORE: "DIR.OMP.END.SCAN"
; RESTORE: "DIR.VPO.GUARD.MEM.MOTION"(){{.*}}"QUAL.OMP.LIVEIN"(ptr [[RED]]) ]
; RESTORE: "DIR.VPO.END.GUARD.MEM.MOTION"
; RESTORE: "DIR.OMP.END.SIMD"

; DISABLE_SCAN-NOT: "DIR.VPO{{.*}}.GUARD.MEM.MOTION"

entry:
  %m_scan_vec.addr = alloca ptr, align 8
  %m_out_vec.addr = alloca ptr, align 8
  %m_in.addr = alloca ptr, align 8
  %c_size.addr = alloca i32, align 4
  %red = alloca %struct.Dummy, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %ref.tmp = alloca %struct.Dummy, align 4
  store ptr %m_scan_vec, ptr %m_scan_vec.addr, align 8
  store ptr %m_out_vec, ptr %m_out_vec.addr, align 8
  store ptr %m_in, ptr %m_in.addr, align 8
  store i32 %c_size, ptr %c_size.addr, align 4
  %0 = load ptr, ptr %m_scan_vec.addr, align 8
  %arrayidx = getelementptr inbounds float, ptr %0, i64 0
  %1 = load float, ptr %arrayidx, align 4
  %x = getelementptr inbounds %struct.Dummy, ptr %red, i32 0, i32 0
  store float %1, ptr %x, align 4
  %2 = load ptr, ptr %m_scan_vec.addr, align 8
  %arrayidx1 = getelementptr inbounds float, ptr %2, i64 0
  %3 = load float, ptr %arrayidx1, align 4
  %y = getelementptr inbounds %struct.Dummy, ptr %red, i32 0, i32 1
  store float %3, ptr %y, align 4
  %4 = load i32, ptr %c_size.addr, align 4
  store i32 %4, ptr %.capture_expr.0, align 4
  %5 = load i32, ptr %.capture_expr.0, align 4
  %sub = sub nsw i32 %5, 0
  %sub2 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub2, 1
  %div = sdiv i32 %add, 1
  %sub3 = sub nsw i32 %div, 1
  store i32 %sub3, ptr %.capture_expr.1, align 4
  %6 = load i32, ptr %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %6
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %7 = load i32, ptr %.capture_expr.1, align 4
  store i32 %7, ptr %.omp.ub, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.precond.then
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
 "QUAL.OMP.REDUCTION.UDR:INSCAN.TYPED"(ptr %red, %struct.Dummy zeroinitializer, i32 1, ptr null, ptr null, ptr @.omp_combiner., ptr @.omp_initializer., i64 1),
 "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
 "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
 "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %ref.tmp, %struct.Dummy zeroinitializer, i32 1) ]

  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.2
  %9 = load i32, ptr %.omp.iv, align 4
  %10 = load i32, ptr %.omp.ub, align 4
  %cmp4 = icmp sle i32 %9, %10
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %11, 1
  %add5 = add nsw i32 0, %mul
  store i32 %add5, ptr %i, align 4
  %x6 = getelementptr inbounds %struct.Dummy, ptr %red, i32 0, i32 0
  %12 = load float, ptr %x6, align 4
  %13 = load ptr, ptr %m_out_vec.addr, align 8
  %14 = load i32, ptr %i, align 4
  %idxprom = sext i32 %14 to i64
  %arrayidx7 = getelementptr inbounds float, ptr %13, i64 %idxprom
  store float %12, ptr %arrayidx7, align 4
  br label %DIR.OMP.SCAN.3

DIR.OMP.SCAN.3:                                   ; preds = %omp.inner.for.body
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(),
 "QUAL.OMP.EXCLUSIVE"(ptr %red, i64 1) ]

  br label %DIR.OMP.SCAN.4

DIR.OMP.SCAN.4:                                   ; preds = %DIR.OMP.SCAN.3
  fence acq_rel
  br label %DIR.OMP.END.SCAN.5

DIR.OMP.END.SCAN.5:                               ; preds = %DIR.OMP.SCAN.4
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.6

DIR.OMP.END.SCAN.6:                               ; preds = %DIR.OMP.END.SCAN.5
  %x8 = getelementptr inbounds %struct.Dummy, ptr %ref.tmp, i32 0, i32 0
  %16 = load ptr, ptr %m_in.addr, align 8
  %17 = load i32, ptr %i, align 4
  %idxprom9 = sext i32 %17 to i64
  %arrayidx10 = getelementptr inbounds float, ptr %16, i64 %idxprom9
  %18 = load float, ptr %arrayidx10, align 4
  store float %18, ptr %x8, align 4
  %y11 = getelementptr inbounds %struct.Dummy, ptr %ref.tmp, i32 0, i32 1
  %19 = load ptr, ptr %m_in.addr, align 8
  %20 = load i32, ptr %i, align 4
  %idxprom12 = sext i32 %20 to i64
  %arrayidx13 = getelementptr inbounds float, ptr %19, i64 %idxprom12
  %21 = load float, ptr %arrayidx13, align 4
  store float %21, ptr %y11, align 4
  call void @.omp_combiner.(ptr %red, ptr %ref.tmp)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %DIR.OMP.END.SCAN.6
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, ptr %.omp.iv, align 4
  %add14 = add nsw i32 %22, 1
  store i32 %add14, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %x15 = getelementptr inbounds %struct.Dummy, ptr %red, i32 0, i32 0
  %23 = load float, ptr %x15, align 4
  ret float %23
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @.omp_combiner.(ptr %0, ptr %1)

declare void @.omp_initializer.(ptr %0, ptr %1)
