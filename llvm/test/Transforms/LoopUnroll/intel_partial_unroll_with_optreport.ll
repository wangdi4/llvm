; RUN: opt -loop-unroll -intel-opt-report=high -S < %s | FileCheck %s

; Verify that partial unrolling adds the expected opt report remark and does not
; drop any existing remarks.

target triple = "x86_64-unknown-linux-gnu"

define void @unroll_pragma(double* %A) {
entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1 ]
  %Ai = getelementptr inbounds double, double* %A, i64 %i
  %idouble = uitofp i64 %i to double
  store double %idouble, double* %Ai, align 8
  %i.next = add nuw nsw i64 %i, 1
  %cond = icmp ne i64 %i.next, 1048576
; CHECK: br i1 %{{.*}}, label %L1, label %L1.end, !llvm.loop ![[LOOP_PRAGMA:[0-9]+]]
  br i1 %cond, label %L1, label %L1.end, !llvm.loop !0

L1.end:
  ret void
}

define void @unroll_heuristics(double* %A) #0 {
entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1 ]
  %Ai = getelementptr inbounds double, double* %A, i64 %i
  %idouble = uitofp i64 %i to double
  store double %idouble, double* %Ai, align 8
  %i.next = add nuw nsw i64 %i, 1
  %cond = icmp ne i64 %i.next, 1048576
; CHECK: br i1 %{{.*}}, label %L1, label %L1.end, !llvm.loop ![[LOOP_HEUR:[0-9]+]]
  br i1 %cond, label %L1, label %L1.end, !llvm.loop !6

L1.end:
  ret void
}

attributes #0 = { "target-cpu"="skylake-avx512" }

; CHECK-DAG: ![[LOOP_PRAGMA]] = distinct !{![[LOOP_PRAGMA]], ![[OPTREPORT_ROOT_PRAGMA:[0-9]+]], ![[UNROLL_DISABLE:[0-9]+]]}
; CHECK-DAG: ![[OPTREPORT_ROOT_PRAGMA]] = distinct !{!"intel.optreport.rootnode", ![[OPTREPORT_PRAGMA:[0-9]+]]}
; CHECK-DAG: ![[OPTREPORT_PRAGMA]] = distinct !{!"intel.optreport", ![[REMARKS:[0-9]+]]}
; CHECK-DAG: ![[REMARKS]] = !{!"intel.optreport.remarks", ![[DUMMY:[0-9]+]], ![[LLORG_UNROLLED:[0-9]+]]}
; CHECK-DAG: ![[DUMMY]] = !{!"intel.optreport.remark", i32 0, !"dummy opt report"}
; CHECK-DAG: ![[LLORG_UNROLLED]] = !{!"intel.optreport.remark", i32 0, !"LLorg: Loop has been unrolled by %d factor", i32 8}
; CHECK-DAG: ![[UNROLL_DISABLE]] = !{!"llvm.loop.unroll.disable"}

; CHECK-DAG: ![[LOOP_HEUR]] = distinct !{![[LOOP_HEUR]], ![[OPTREPORT_ROOT_HEUR:[0-9]+]]}
; CHECK-DAG: ![[OPTREPORT_ROOT_HEUR]] = distinct !{!"intel.optreport.rootnode", ![[OPTREPORT_HEUR:[0-9]+]]}
; CHECK-DAG: ![[OPTREPORT_HEUR]] = distinct !{!"intel.optreport", ![[REMARKS]]}
!0 = distinct !{!0, !1, !2}
!1 = !{!"llvm.loop.unroll.count", i32 8}
!2 = distinct !{!"intel.optreport.rootnode", !3}
!3 = distinct !{!"intel.optreport", !4}
!4 = !{!"intel.optreport.remarks", !5}
!5 = !{!"intel.optreport.remark", i32 0, !"dummy opt report"}

!6 = distinct !{!6, !7}
!7 = distinct !{!"intel.optreport.rootnode", !8}
!8 = distinct !{!"intel.optreport", !4}
