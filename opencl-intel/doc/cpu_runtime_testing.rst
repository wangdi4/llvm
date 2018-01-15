================================
OpenCL CPU Runtime testing guide
================================

Overview
========

The goal of this document is to describe how OpenCL CPU Runtime is tested using
LIT infrastructure. This guide aims to answer the following questions:

* How can I launch unittests for OpenCL CPU Runtime?
* How can I add new unittests for OpenCL CPU Runtime?

Additional build options
========================

There are following cmake options related to OCL CPU RT unittests:

* ``OPENCL_RT_INCLUDE_TESTS`` Generate build targets for OpenCL RT unittest.
  Defaults to ON.  You can use this option to disable the generation of build
  targets for OpenCL RT unittests.

* ``OPENCL_RT_BUILD_TESTS`` Build OpenCL RT unittests. Defaults to OFF.
  Targets for building each unittest are generated in any case.

How to run OpenCL CPU Runtime unittests
=======================================

OCL CPU RT unittests are available as cmake targets. There are two basic options
to launch unittests:

* One target to launch them all!
* A bunch of targets to run LIT tests from each subdirectory separately.

So, to launch OCL CPU RT unittests you need to execute something like
``ics libbuild <target-to-launch-unittests> -j8``

Available unittests targets
---------------------------

To launch **all** OCL CPU RT unittests you need to specify target
``check-ocl-runtime``

Also, the following targets are available:

* ``check-ocl-bi_test_type``
* ``check-ocl-clang_compiler_test_type``
* ``check-ocl-cpu_device_test_type``
* ``check-ocl-name_mangling_test_type``
* ``check-ocl-framework_test_type``
* ``check-ocl-task_executor_test_type``

Special targets for each unittest are added by ``add_ocl_rt_unittest`` cmake
function.

How to add new unittests
========================

If you want to extend existing unittest - just do it! It doesn't require any
additional actions.

If you want to create a new one unittest you need to do some extra actions
except writing source code and CMakeLists.txt file.

First of all, read about cmake helper functions in the next sub-section and use
these functions in your CMakeLists.txt.

To integrate your test to LIT infrastructure you need to copy :file:``lit.cfg``
file from some other unittest and replace value of ``test_subdir`` variable in
the beginning of the file with name of subdirectory you created.

If you want to include your unittest to ``check-ocl-runtime`` target you need to
add your subdirectory name to ``unittest_subdirs`` variable in
file:``tests/lit.cfg`` and add your unittest target name to
``RUNTIME_UNITTESTS`` list in :file:``tests/CMakeLists.txt``

CMake helper functions
----------------------

add_ocl_rt_unittests
^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  add_ocl_unittests(<name> SOURCE_FILES source1 [source2 ...]
                    [LINK_LIBRARIES library1 [library2 ...]])

This function is a wrapper around ``add_executable`` cmake function. It adds
unittest executable.

This function respects ``OPENCL_RT_BUILD_TESTS``: ``EXCLUDE_FROM_ALL`` property
is added to unittest if ``OPENCL_RT_BUILD_TESTS`` is disabled.

.. note:: You no need to list googletest library in ``LINK_LIBRARIES`` argument.
   It is linked by function automatically.

.. note:: You no need to specify path go googletest headers in
   ``include_directories``. It is added by function.

.. note:: You no need to specify path to OpenCL Headers in
   ``include_directories``. It is added by function.

Resulting binary will be placed under ``${CMAKE_CURRENT_BINARY_DIR}``.

This function automatially adds ``check-ocl-runtime-your_test_name`` cmake
target to launch your unittest.

add_ocl_unittest_artifacts
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  add_ocl_unittest_artifacts(FILE <name> [RENAME <new_name>] [CONFIGURE])

  add_ocl_unittest_artifacts(FILES file1 [file2...])

This function is useful when you need to copy some artifacts (i.e. ``.cl``
files) to the same dir with unittest.

First variant of this function can be used when you need to copy some file to 
build directory on build time and rename or configure it.

The second variant of this function can be used when you need to copy set of
files to build directory.

In both variants copies of files will be placed under
``${CMAKE_CURRENT_BINARY_DIR}``.

Internal details
================

Here you can find out how this all staff works.

First of all, it would be great if you look into official documentation for
`lit - LLVM Integrated Tester`_.

.. _lit - LLVM Integrated Tester: https://llvm.org/docs/CommandGuide/lit.html

Okay, let's go.

check-ocl-runtime
-----------------

What ``check-ocl-runtime`` exactly does and how it works?

Basically, ``check-ocl-runtime`` is a custom cmake command which launches
:program:`lit` tool. :program:`lit` requires at least one test to be specified
as command line argument and it can be either individual test file or just some
directory containing tests. In our case :program:`lit` gets path to
``${OCL_BINARY_DIR}/tests`` directory.

First step of :program:`lit` is a test discovery. The purpose of this step is
convert inputs to a complete list of tests to run. All tests in the lit model
exists inside one test suite. Test suite is defined by :file:`lit.cfg` or
:file:`lit.site.cfg`. To discover test suites :program:`lit` searches for this
files upwards from the input path. In our case it finds the :file:`lit.site.cfg`
in the ``${OCL_BINARY_DIR}/tests``

:file:`lit.site.cfg` produced by ``configure_lit_site_cfg`` cmake function. It
configures :file:`tests/lit.site.cfg.in`. Configuring means copying file into a
new location with replacing all ``@VAR_NAME@`` strings with values of
corresponding cmake variables.

In our case :file:`lit.site.cfg` contains paths to directories containing
OpenCL binaries and tests.

:file:`lit.site.cfg` actually does nothing except loading :file:`lit.cfg` which
does all staff.

:file:`lit.cfg` uses ``GoogleTest`` lit format to launch gtest-based tests. It
searches for files ending with ``test_type`` (``test_type.exe`` on Windows) in
subdirectories listed in ``unittests_subdirs`` variable and launches tests from
ones.

check-ocl-your_unittest_name
------------------------------------

Special targets for each unittests works in a little bit different manner than
``check-ocl-runtime`` target.

In this case :program:`lit` is got path to :file:`lit.cfg` in test source
directory as tests directory.

:file:`lit.cfg` defines ``unittest_subdirs`` variable to instruct lit to search
unittests only in the certain subdirectory and loads
:file:`${OCL_BINARY_DIR}/tests/lit.site.cfg`. Then execution goes in the same
manner as for ``check-ocl-runtime`` target.

TODO
====

* Merge this document with :file:`cpu_backend_lit_tests.rst` to have one
  document describing how to test OpenCL run-time and compiler
