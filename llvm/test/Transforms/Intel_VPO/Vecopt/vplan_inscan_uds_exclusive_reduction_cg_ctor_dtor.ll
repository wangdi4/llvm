; RUN: opt -S -passes="vplan-vec" -vplan-force-vf=2 < %s 2>&1 | FileCheck %s

;; Original source for reference:

;; struct Dummy {
;;   Dummy() { }
;;   ~Dummy() {}
;;   Dummy& operator+(const Dummy&);
;;   Dummy& operator=(const Dummy&);
;;   float x;
;;   float y;
;; };
;;
;; void my_add(Dummy &lhs, const Dummy &rhs) { lhs.x += rhs.x; lhs.y = rhs.y; }
;; void my_init(Dummy &t) { t.x = 0; t.y = 0; }
;;
;; #pragma omp declare reduction(my_scan_add: struct Dummy : my_add(omp_out, omp_in)) initializer(my_init(omp_priv))
;;
;; float uds(float* m_scan_vec, float* m_out_vec, float* m_in, int c_size)
;; {
;;   Dummy red;
;;     red.x = m_scan_vec[0];
;;     red.y = m_scan_vec[0];
;;     #pragma omp simd reduction(inscan, my_scan_add: red)
;;     for (int i = 0; i < c_size; ++i) {
;;       m_out_vec[i] = red.x;
;;     #pragma omp scan exclusive(red)
;;         Dummy op;
;;         op.x = m_in[i];
;;         op.y = m_in[i];
;;         my_add(red, op);
;;     }
;;     return red.x;
;; }

; CHECK-LABEL:  entry:
; CHECK:         [[RED_ORIG:%.*]] = alloca %struct.Dummy
; CHECK:         [[RED_VEC_TEMP:%.*]] = alloca [2 x %struct.Dummy]
; CHECK-NEXT:    [[RED_VEC_TEMP_BASE_ADDR:%.*]] = getelementptr %struct.Dummy, ptr [[RED_VEC_TEMP]], <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0:%.*]] = extractelement <2 x ptr> [[RED_VEC_TEMP_BASE_ADDR]], i32 0
; CHECK-NEXT:    [[RED_VEC:%.*]] = alloca [2 x %struct.Dummy]
; CHECK-NEXT:    [[RED_VEC_BASE_ADDR:%.*]] = getelementptr %struct.Dummy, ptr [[RED_VEC]], <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[RED_VEC_BASE_ADDR_EXTRACT_1:%.*]] = extractelement <2 x ptr> [[RED_VEC_BASE_ADDR]], i32 1
; CHECK-NEXT:    [[RED_VEC_BASE_ADDR_EXTRACT_0:%.*]] = extractelement <2 x ptr> [[RED_VEC_BASE_ADDR]], i32 0

; CHECK-LABEL: vector.body:
; CHECK:         call ptr @_ZTS5Dummy.omp.def_constr(ptr [[RED_VEC_BASE_ADDR_EXTRACT_0]])
; CHECK-NEXT:    call ptr @_ZTS5Dummy.omp.def_constr(ptr [[RED_VEC_BASE_ADDR_EXTRACT_1]])
; CHECK-NEXT:    call void @.omp_initializer.(ptr [[RED_VEC_BASE_ADDR_EXTRACT_0]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_initializer.(ptr [[RED_VEC_BASE_ADDR_EXTRACT_1]], ptr [[RED_ORIG]])

; CHECK-LABEL: VPlannedBB17:
; CHECK-NEXT:    call ptr @_ZTS5Dummy.omp.def_constr(ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]])
; CHECK-NEXT:    call void @.omp_initializer.(ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_combiner.(ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]], ptr [[RED_VEC_BASE_ADDR_EXTRACT_0]])
; CHECK-NEXT:    call void @.omp_initializer.(ptr [[RED_VEC_BASE_ADDR_EXTRACT_0]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_combiner.(ptr [[RED_VEC_BASE_ADDR_EXTRACT_0]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_combiner.(ptr [[RED_ORIG]], ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]])
; CHECK-NEXT:    call void @.omp_initializer.(ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_combiner.(ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]], ptr [[RED_VEC_BASE_ADDR_EXTRACT_1]])
; CHECK-NEXT:    call void @.omp_initializer.(ptr [[RED_VEC_BASE_ADDR_EXTRACT_1]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_combiner.(ptr [[RED_VEC_BASE_ADDR_EXTRACT_1]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_combiner.(ptr [[RED_ORIG]], ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]])
; CHECK-NEXT:    call ptr @_ZTS5Dummy.omp.def_constr(ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]])
; CHECK-NEXT:    br label

;; Latch:
; CHECK:         call void @_ZTS5Dummy.omp.destr(ptr [[RED_VEC_BASE_ADDR_EXTRACT_0]])
; CHECK-NEXT:    call void @_ZTS5Dummy.omp.destr(ptr [[RED_VEC_BASE_ADDR_EXTRACT_1]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Dummy = type { float, float }

define float @_Z3udsPfS_S_i(ptr %m_scan_vec, ptr %m_out_vec, ptr %m_in, i32 %c_size) {
entry:
  %red.red = alloca %struct.Dummy, align 4
  %i.linear.iv = alloca i32, align 4
  %0 = load float, ptr %m_scan_vec, align 4
  %cmp = icmp sgt i32 %c_size, 0
  br i1 %cmp, label %DIR.OMP.SIMD.236, label %omp.precond.end

DIR.OMP.SIMD.236:                                 ; preds = %entry
  store float 0.000000e+00, ptr %red.red, align 4
  %y.i.i53 = getelementptr inbounds %struct.Dummy, ptr %red.red, i64 0, i32 1
  store float 0.000000e+00, ptr %y.i.i53, align 4
  br label %DIR.VPO.GUARD.MEM.MOTION.4.lr.ph

DIR.VPO.GUARD.MEM.MOTION.4.lr.ph:                 ; preds = %DIR.OMP.SIMD.236
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.UDR:INSCAN.TYPED"(ptr %red.red, %struct.Dummy zeroinitializer, i32 1, ptr @_ZTS5Dummy.omp.def_constr, ptr @_ZTS5Dummy.omp.destr, ptr @.omp_combiner., ptr @.omp_initializer., i64 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.VPO.GUARD.MEM.MOTION.4.lr.ph
  %wide.trip.count = sext i32 %c_size to i64
  br label %DIR.VPO.GUARD.MEM.MOTION.4

DIR.VPO.GUARD.MEM.MOTION.4:                       ; preds = %DIR.OMP.SIMD.1, %DIR.VPO.END.GUARD.MEM.MOTION.6
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %DIR.VPO.END.GUARD.MEM.MOTION.6 ]
  br label %DIR.VPO.GUARD.MEM.MOTION.3.split

DIR.VPO.GUARD.MEM.MOTION.3.split:                 ; preds = %DIR.VPO.GUARD.MEM.MOTION.4
  %phase1.guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %red.red) ]
  br label %DIR.VPO.GUARD.MEM.MOTION.2

DIR.VPO.GUARD.MEM.MOTION.2:                       ; preds = %DIR.VPO.GUARD.MEM.MOTION.3.split
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, ptr %i.linear.iv, align 4
  %3 = load float, ptr %red.red, align 4
  %arrayidx7 = getelementptr inbounds float, ptr %m_out_vec, i64 %indvars.iv
  store float %3, ptr %arrayidx7, align 4
  br label %DIR.VPO.END.GUARD.MEM.MOTION.552

DIR.VPO.END.GUARD.MEM.MOTION.552:                 ; preds = %DIR.VPO.GUARD.MEM.MOTION.2
  call void @llvm.directive.region.exit(token %phase1.guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.OMP.SCAN.7

DIR.OMP.SCAN.7:                                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.552
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.EXCLUSIVE"(ptr %red.red, i64 1) ]
  br label %DIR.OMP.SCAN.3

DIR.OMP.SCAN.3:                                   ; preds = %DIR.OMP.SCAN.7
  fence acq_rel
  br label %DIR.OMP.END.SCAN.9

DIR.OMP.END.SCAN.9:                               ; preds = %DIR.OMP.SCAN.3
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.9.split

DIR.OMP.END.SCAN.9.split:                         ; preds = %DIR.OMP.END.SCAN.9  
  %phase2.guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"() ]
  br label %DIR.OMP.END.SCAN.4

DIR.OMP.END.SCAN.4:                               ; preds = %DIR.OMP.END.SCAN.9.split
  br label %DIR.VPO.GUARD.MEM.MOTION.11.split

DIR.VPO.GUARD.MEM.MOTION.11.split:                ; preds = %DIR.OMP.END.SCAN.4
  br label %DIR.VPO.GUARD.MEM.MOTION.5

DIR.VPO.GUARD.MEM.MOTION.5:                       ; preds = %DIR.VPO.GUARD.MEM.MOTION.11.split
  %5 = load i32, ptr %i.linear.iv, align 4
  %idxprom8 = sext i32 %5 to i64
  %arrayidx9 = getelementptr inbounds float, ptr %m_in, i64 %idxprom8
  %6 = load float, ptr %arrayidx9, align 4
  %7 = load float, ptr %red.red, align 4
  %add.i = fadd fast float %7, %6
  store float %add.i, ptr %red.red, align 4
  %y2.i = getelementptr inbounds %struct.Dummy, ptr %red.red, i64 0, i32 1
  store float %6, ptr %y2.i, align 4
  br label %DIR.VPO.END.GUARD.MEM.MOTION.13

DIR.VPO.END.GUARD.MEM.MOTION.13:                  ; preds = %DIR.VPO.GUARD.MEM.MOTION.5
  call void @llvm.directive.region.exit(token %phase2.guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.6

DIR.VPO.END.GUARD.MEM.MOTION.6:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.13
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.15.loopexit_crit_edge, label %DIR.VPO.GUARD.MEM.MOTION.4

omp.inner.for.cond.DIR.OMP.END.SIMD.15.loopexit_crit_edge: ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.6
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.7

DIR.OMP.END.SIMD.7:                               ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.15.loopexit_crit_edge
  %.fca.0.load = load float, ptr %red.red, align 4
  %add.i.i = fadd fast float %.fca.0.load, %0
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.7, %entry
  %red.sroa.0.1 = phi float [ %0, %entry ], [ %add.i.i, %DIR.OMP.END.SIMD.7 ]
  ret float %red.sroa.0.1
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @.omp_combiner.(ptr %0, ptr %1)

declare void @.omp_initializer.(ptr %0, ptr %1)

declare void @_ZTS5Dummy.omp.destr(ptr %0)

declare ptr @_ZTS5Dummy.omp.def_constr(ptr %0)
