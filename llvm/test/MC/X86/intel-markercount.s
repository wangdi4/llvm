// INTEL_FEATURE_MARKERCOUNT
# REQUIRES: intel_feature_markercount
# RUN: llvm-mc --show-encoding  %s | FileCheck %s --check-prefix=ENCODE
# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump -d - | FileCheck %s --check-prefix=DUMP

# ENCODE: marker_loop              # encoding: [0x37]
# ENCODE: marker_function          # encoding: [0x0e]
# DUMP: 37                            marker_loop
# DUMP: 0e                            marker_function
  .text
  marker_loop
  marker_function

// end INTEL_FEATURE_MARKERCOUNT
