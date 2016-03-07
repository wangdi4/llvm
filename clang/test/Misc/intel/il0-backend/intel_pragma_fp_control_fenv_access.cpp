// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -fp-model precise -o - %s | FileCheck %s
//***INTEL: pragma fp_control, fenv_access test

// CHECK: define void @{{.*}}f1{{.*}}() #0
// CHECK: ret void
// CHECK-NEXT: }
#pragma fp_contract(on)
void f1(){}

// CHECK: define void @{{.*}}f2{{.*}}() #1
// CHECK: ret void
// CHECK-NEXT: }
#pragma fp_contract(off)
void f2(){}

// CHECK: define void @{{.*}}f3{{.*}}() #2
// CHECK: ret void
// CHECK-NEXT: }
#pragma fenv_access(on)
void f3(){}

// CHECK: define void @{{.*}}f4{{.*}}() #3
// CHECK: ret void
// CHECK-NEXT: }
#pragma fenv_access(off)
void f4(){}

#pragma fp_contract(default) // expected-warning {{'on' or 'off' is expected}}
#pragma fenv_access(default) // expected-warning {{'on' or 'off' is expected}}
#pragma fp_contract // expected-warning {{missing '(' after '#pragma fp_contract' - ignoring}}
#pragma fenv_access // expected-warning {{missing '(' after '#pragma fenv_access' - ignoring}}
#pragma fp_contract ww // expected-warning {{missing '(' after '#pragma fp_contract' - ignoring}}
#pragma fenv_access ss // expected-warning {{missing '(' after '#pragma fenv_access' - ignoring}}
#pragma fp_contract (// expected-warning {{'on' or 'off' is expected}}
#pragma fenv_access (// expected-warning {{'on' or 'off' is expected}}
#pragma fp_contract ( ww // expected-warning {{'on' or 'off' is expected}}
#pragma fenv_access (ss // expected-warning {{'on' or 'off' is expected}}
#pragma fp_contract (on  // expected-warning {{missing ')' after '#pragma fp_contract' - ignoring}}
#pragma fenv_access (off  // expected-warning {{missing ')' after '#pragma fenv_access' - ignoring}}
#pragma fp_contract (off  // expected-warning {{missing ')' after '#pragma fp_contract' - ignoring}}
#pragma fenv_access (on  // expected-warning {{missing ')' after '#pragma fenv_access' - ignoring}}

// CHECK: define i32 @main{{.*}} #4
int main(int argc, char **argv)
{
#pragma fp_contract(on)
#pragma fp_contract(off)
#pragma fenv_access(on)
#pragma fenv_access(off)
    return (argc);
}

// CHECK: attributes #0 = { nounwind "INTEL:EXCEPT-OFF" "INTEL:FP_CONTRACT-ON" "INTEL:PRECISE-ON"
// CHECK: attributes #1 = { nounwind "INTEL:EXCEPT-OFF" "INTEL:FP_CONTRACT-OFF" "INTEL:PRECISE-ON"
// CHECK: attributes #2 = { nounwind "INTEL:EXCEPT-OFF" "INTEL:FENV_ACCESS-ON" "INTEL:FP_CONTRACT-OFF" "INTEL:PRECISE-ON"
// CHECK: attributes #3 = { nounwind "INTEL:EXCEPT-OFF" "INTEL:FENV_ACCESS-OFF" "INTEL:FP_CONTRACT-OFF" "INTEL:PRECISE-ON"
// CHECK: attributes #4 = { norecurse nounwind "INTEL:EXCEPT-OFF" "INTEL:FENV_ACCESS-OFF" "INTEL:FP_CONTRACT-OFF" "INTEL:PRECISE-ON"
