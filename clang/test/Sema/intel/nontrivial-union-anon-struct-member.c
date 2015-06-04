// CQ#364709
// RUN: %clang_cc1 -x c++ -fintel-ms-compatibility -DDESTRUCTOR -verify %s
// RUN: %clang_cc1 -x c++ -fintel-ms-compatibility -DCONSTRUCTOR -verify %s
// RUN: %clang_cc1 -x c++ -fintel-ms-compatibility -DCOPY_CONSTRUCTOR -verify %s
// RUN: %clang_cc1 -x c++ -fintel-ms-compatibility -DCOPY_ASSIGNMENT -verify %s
// RUN: %clang_cc1 -x c++ -DERROR -verify %s

struct X
{
#if DESTRUCTOR

  ~X(); // expected-note {{because type 'X' has a user-provided destructor}}

#elif CONSTRUCTOR

  X(); // expected-note {{because type 'X' has a user-provided default constructor}}

#elif COPY_CONSTRUCTOR

  X();
  X(const X &other); // expected-note {{because type 'X' has a user-provided copy constructor}}

#elif COPY_ASSIGNMENT

  X& operator=(const X &other); // expected-note {{because type 'X' has a user-provided copy assignment operator}}

#elif ERROR

  ~X(); // expected-note {{because type 'X' has a user-provided destructor}}

#else

#error Unknown test mode.

#endif
};

struct Y
{
  struct {
#if DESTRUCTOR

    X ax[1]; // expected-warning {{anonymous struct member 'ax' has a non-trivial destructor}}

#elif CONSTRUCTOR

    X ax[1]; // expected-warning {{anonymous struct member 'ax' has a non-trivial constructor}}

#elif COPY_CONSTRUCTOR

    X ax[1]; // expected-warning {{anonymous struct member 'ax' has a non-trivial copy constructor}}

#elif COPY_ASSIGNMENT

    X ax[1]; // expected-warning {{anonymous struct member 'ax' has a non-trivial copy assignment operator}}

#elif ERROR

    X ax[1]; // expected-error {{anonymous struct member 'ax' has a non-trivial destructor}}

#else

#error Unknown test mode.

#endif
  };
};

Y y;
