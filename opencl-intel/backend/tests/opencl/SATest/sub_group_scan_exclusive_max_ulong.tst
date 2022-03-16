; This test is to check the correct builtin for subgroup collectives called.

; RUN: SATest -BUILD -native-subgroups --config=%s.cfg -tsize=0 --dump-llvm-file - | FileCheck %s
; CHECK-NOT: %elt.max{{.*}} = call <16 x i32> @llvm.smax.v16i32(<16 x i32> %{{.*}}
; CHECK-COUNT-26: %elt.max{{.*}} = call <8 x i64> @llvm.umax.v8i64(<8 x i64> %{{.*}}
