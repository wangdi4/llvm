## Check that we can reorder test runs.

<<<<<<< HEAD
# INTEL_CUSTOMIZATION
# Disabled temporarily till further investigation.
# UNSUPPORTED: system-windows
# end INTEL_CUSTOMIZATION

# RUN: cp %{inputs}/reorder/.lit_test_times.txt %{inputs}/reorder/.lit_test_times.txt.orig
=======
# RUN: cp %{inputs}/reorder/lit_test_times %{inputs}/reorder/.lit_test_times.txt
>>>>>>> 1dbcb79498d0db465234c68adf52b88e7f87a832
# RUN: not %{lit-no-order-opt} %{inputs}/reorder > %t.out
# RUN: FileCheck --check-prefix=TIMES < %{inputs}/reorder/.lit_test_times.txt %s
# RUN: FileCheck < %t.out %s
# END.

# TIMES: not-executed.txt
# TIMES-NEXT: subdir/ccc.txt
# TIMES-NEXT: bbb.txt
# TIMES-NEXT: -{{.*}} fff.txt
# TIMES-NEXT: aaa.txt
# TIMES-NEXT: new-test.txt

# CHECK:     -- Testing: 5 tests, 1 workers --
# CHECK-NEXT: FAIL: reorder :: fff.txt
# CHECK-NEXT: PASS: reorder :: subdir/ccc.txt
# CHECK-NEXT: PASS: reorder :: bbb.txt
# CHECK-NEXT: PASS: reorder :: aaa.txt
# CHECK-NEXT: PASS: reorder :: new-test.txt
# CHECK:     Passed: 4
