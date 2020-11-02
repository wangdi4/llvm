; RUN: SATest -BUILD --config=%s.cfg --dump-llvm-file - | FileCheck %s

; Check that no boundary functions are left after optimizer.

; CHECK-NOT: define {{.*}} @WG.boundaries.
