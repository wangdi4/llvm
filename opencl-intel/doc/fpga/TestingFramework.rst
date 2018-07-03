======================
FPGA Testing Framework
======================

Motivation
==========

Main motivation of creating a new test suite is to separate FPGA-specific tests
from regular OpenCL CPU Runtime & Compiler tests.

Requirements
============

* Usable API
* Nice to have: No online compilation from source files
* Anything else?

Testing Framework Overview
==========================

Currently testing framework consists of two fixtures:

* ``OCLFPGASimpleFixture`` - the main fixture for tests development. It provides
  very simple and minimalistic API to simplify tests development process. It is
  perfect for simple test-cases. Simple means:

  * one context with one device in it
  * one program per test
  * test performs the following actions:

    * create and build program
    * create buffer/pipe
    * enqueue each kernel into a separate queue
    * read from buffer

* ``OCLFPGABaseFixture`` - when you need to write something more complex. For
  example: more than one device in the context or several programs per test,
  several kernels enqueued into a single queue.

Detailed description of available methods you can find in header files with
fixtures. Here you can find an example of ``OCLFPGASimpleFixture`` usage:

.. code-block:: c++

  TEST_F(OCLFPGASimpleFixture, Test) {
    ASSERT_TRUE(createAndBuildProgram("// program sources here"), "build options");
    cl_mem buf_a = createBuffer<cl_int4>(10); // buffer for 10 elements of type cl_int4
    ASSERT_NE(nullptr, buf_a);
    ASSERT_TRUE(enqueueTask("kernel_name", buf_a, 10)); // buf_a and 10 are kernel arguments
    cl_int4 data[10];
    ASSERT_TRUE(reqdBuffer<cl_int4>("kernel_name", buf_a, 10, data)); // blocking read
    // "kernel_name" is used to specify command queue which should be used to
    // enqueue read command
    // also you can manually finish queues by calling finish("kernel_name")
    ASSERT_EQ(reference_data, data);
  }


Environments
^^^^^^^^^^^^

Sometimes you need to launch tests under a specific environment. To achieve
this you need to define your own fixture in the following manner:

.. code-block:: c++

  class TestWithMyFancyEnvironment : public OCLFPGASimpleFixture {
  protected:
    typedef OCLFPGASimpleFixture parent_t;
    void SetUp() override {
      // setup your fancy environment here
      parent_t::SetUp();
    }

    void TearDown() override {
      parent_t::TearDown();
      // unset env here
    }
  };

  TEST_F(TestWithMyFancyEnvironment, Test1) {
    // test code
  }

