// CQ#364268
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST1
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST2
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST3
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST4
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST5
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST6


#if TEST1

typedef int Type1;
typedef char * Type2;

int main(Type1 arg1, Type2 arg2) { // expected-warning {{second parameter of 'main' (argument array) should be of type 'char **'}}
}

#elif TEST2

typedef int Type1;
typedef int * Type2;

int main(Type1 arg1, Type2 arg2) { // expected-warning {{second parameter of 'main' (argument array) should be of type 'char **'}}
}


#elif TEST3

typedef int * Type1;
typedef int * Type2;
int main(Type1 arg1, Type2 arg2) { // expected-warning {{first parameter of 'main' (argument count) should be of type 'int'}} expected-warning {{second parameter of 'main' (argument array) should be of type 'char **'}}
}


#elif TEST4
	
typedef char ** Type1;
typedef int Type2;

int main(Type1 arg1, Type2 arg2) { // expected-warning {{first parameter of 'main' (argument count) should be of type 'int'}} expected-warning {{second parameter of 'main' (argument array) should be of type 'char **'}}
}


#elif TEST5
	
typedef const char ** Type1;
typedef int Type2;

int main(Type1 arg1, Type2 arg2) { // expected-warning {{first parameter of 'main' (argument count) should be of type 'int'}} expected-warning {{second parameter of 'main' (argument array) should be of type 'char **'}}
}


#elif TEST6

typedef struct SC {
    unsigned char a;
    unsigned char b;
    unsigned short c;
} SC;

typedef struct _SA {
    SC p[1];
} *Type1;

typedef struct _SB {
    SC p[1];
} *Type2;

int main(Type1 arg1, Type2 arg2) { // expected-warning {{first parameter of 'main' (argument count) should be of type 'int'}} expected-warning {{second parameter of 'main' (argument array) should be of type 'char **'}}
}

#else

#error Unknown test mode

#endif
