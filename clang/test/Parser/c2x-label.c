// RUN: %clang_cc1 -fsyntax-only -std=c17 -Wc2x-compat -verify=c17 %s
// RUN: %clang_cc1 -fsyntax-only -std=c2x -Wpre-c2x-compat -verify=c2x %s
// INTEL RUN: %clang_cc1 -fsyntax-only -std=c17 -Wc2x-compat -verify=intel-c17 -fintel-compatibility %s
// INTEL RUN: %clang_cc1 -fsyntax-only -std=c2x -Wpre-c2x-compat -verify=intel-c2x -fintel-compatibility %s

void test_label_in_func() {
    int x;
label1:
    x = 1;
label2: label3: label4: // intel-c17-warning {{expected statement}} \
                           intel-c2x-warning {{expected statement}}
} // c17-warning {{label at end of compound statement is a C2x extension}} \
     c2x-warning {{label at end of compound statement is incompatible with C standards before C2x}} \

int test_label_in_switch(int v) {
    switch (v) {
    case 1:
        return 1;
    case 2:
        return 2;
    case 3: case 4: case 5: // intel-c17-warning {{label at end of switch compound statement: expected statement}} \
                               intel-c2x-warning {{label at end of switch compound statement: expected statement}}
    } // c17-warning {{label at end of compound statement is a C2x extension}} \
         c2x-warning {{label at end of compound statement is incompatible with C standards before C2x}} \

    switch (v) {
    case 6:
        return 6;
    default: // intel-c17-warning {{label at end of switch compound statement: expected statement}} \
                intel-c2x-warning {{label at end of switch compound statement: expected statement}}
    } // c17-warning {{label at end of compound statement is a C2x extension}} \
         c2x-warning {{label at end of compound statement is incompatible with C standards before C2x}} \

    return 0;
}
