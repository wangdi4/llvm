// RUN: %clang_cc1 -fsyntax-only -verify=expected,cxx2b -std=c++2b -Wpre-c++2b-compat %s
// RUN: %clang_cc1 -fsyntax-only -verify=expected,cxx20 -std=c++20 %s
// INTEL RUN: %clang_cc1 -fsyntax-only -verify=expected,intel-cxx2b -std=c++2b -Wpre-c++2b-compat -fintel-compatibility %s
// INTEL RUN: %clang_cc1 -fsyntax-only -verify=expected,intel-cxx20 -std=c++20 -fintel-compatibility %s

void test_label_in_func() {
label1:
    int x;
label2:
    x = 1;
label3: label4: label5: // intel-cxx2b-warning {{expected statement}} \
                           intel-cxx20-warning {{expected statement}}
} // cxx20-warning {{label at end of compound statement is a C++2b extension}} \
     cxx2b-warning {{label at end of compound statement is incompatible with C++ standards before C++2b}}

int test_label_in_switch(int v) {
    switch (v) {
    case 1:
        return 1;
    case 2:
        return 2;
    case 3: case 4: case 5: // intel-cxx2b-warning {{label at end of switch compound statement: expected statement}} \
                               intel-cxx20-warning {{label at end of switch compound statement: expected statement}}
    } // cxx20-warning {{label at end of compound statement is a C++2b extension}} \
         cxx2b-warning {{label at end of compound statement is incompatible with C++ standards before C++2b}}

    switch (v) {
    case 6:
        return 6;
    default: // intel-cxx2b-warning {{label at end of switch compound statement: expected statement}} \
                intel-cxx20-warning {{label at end of switch compound statement: expected statement}}
    } // cxx20-warning {{label at end of compound statement is a C++2b extension}} \
         cxx2b-warning {{label at end of compound statement is incompatible with C++ standards before C++2b}}

    return 0;
}
