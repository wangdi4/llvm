/****************************************************************************
  Copyright (c) Intel Corporation (2012,2013).

  INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
  LICENSED ON AN AS IS BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
  ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
  PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
  DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
  PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
  including liability for infringement of any proprietary rights, relating to
  use of the code. No license, express or implied, by estoppels or otherwise,
  to any intellectual property rights is granted herein.

  File Name: TestRefCount.cpp
\****************************************************************************/

#include "Refcount.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <vector>

using namespace intel;

//
// On top of being functional tests, the test below tests the RefCounter for
// memory-leask. As for now they should be ran with cmd line tools such as
// valgrind: valgrind --tool=memcheck --leak-check=full ./NameManglingTest
//

class A {
  int num;

public:
  A(int n) : num(n) {}
  int getNum() const { return num; }
};

std::ostream &operator<<(std::ostream &o, const A &a) {
  o << a.getNum();
  return o;
}

static void printRef(const RefCount<A> &a) { std::cout << *a << std::endl; }

static void printA(const A *a) { std::cout << *a << std::endl; }

TEST(ReferenceCounter, l1) {
  RefCount<A> aref(new A(2));
  printRef(aref);
}

TEST(ReferenceCounter, sharedref) {
  RefCount<A> aref(new A(2));
  {
    RefCount<A> bref = aref;
    printRef(bref);
  }
  printRef(aref);
}

TEST(ReferenceCounter, NullInitialization) {
  RefCount<A> refa;
  refa.init(new A(3));
  printRef(refa);
}

TEST(ReferenceCounter, inVector) {
  std::vector<RefCount<A>> vec;
  vec.push_back(new A(0));
  vec.push_back(new A(1));
  vec.push_back(new A(2));
  std::for_each(vec.begin(), vec.end(), printRef);
  std::for_each(vec.begin(), vec.end(), printA);
}

// Tricky case, in which we use the = operator, to which the previous object is
// already initialized
TEST(ReferenceCounter, vectorSwap) {
  std::vector<RefCount<A>> vec;
  vec.push_back(new A(0));
  vec[0] = RefCount<A>(new A(1));
}

TEST(ReferenceCounter, pointer) {
  RefCount<A> refa(new A(0));
  ASSERT_EQ(0, refa->getNum());
}

TEST(ReferenceCounter, vectorChanges) {
  std::vector<RefCount<A>> vec;
  vec.push_back(new A(0));
  vec.push_back(new A(1));
  vec.resize(1);
}
