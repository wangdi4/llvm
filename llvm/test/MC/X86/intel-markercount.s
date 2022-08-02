// INTEL_FEATURE_MARKERCOUNT
# REQUIRES: intel_feature_markercount
# RUN: llvm-mc --show-encoding  %s | FileCheck %s --check-prefix=ENCODE
# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump -d - | FileCheck %s --check-prefix=DUMP

# ENCODE: markercount_loopheader              # encoding: [0x37]
# ENCODE: markercount_function                # encoding: [0x0e]
# DUMP: 37                            markercount_loopheader
# DUMP: 0e                            markercount_function
  .text
  markercount_loopheader
  markercount_function

// end INTEL_FEATURE_MARKERCOUNT
