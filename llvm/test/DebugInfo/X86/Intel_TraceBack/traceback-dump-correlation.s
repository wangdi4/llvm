# Check the dump information for correlation.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump --traceback - | FileCheck %s

# CHECK-LABEL: .trace contents:
# CHECK-NEXT: 00000000: bf                       # TB_TAG_CO1
# CHECK-NEXT:           (line: 1, delta line: 1)
# CHECK-NEXT:           (PC: +0, delta PC: 0x40)
# CHECK-NEXT: 00000001: ff 02                    # TB_TAG_CO2
# CHECK-NEXT:           (line: 3, delta line: 2)
# CHECK-NEXT:           (PC: +0x40, delta PC: 0x40)

    .section    .trace,"a",@progbits
    .byte    191                               # TB_TAG_CO1
    .byte    255, 2                            # TB_TAG_CO2
