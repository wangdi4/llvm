====================================================
Extra Clang Tools |release| |ReleaseNotesTitle|
====================================================

.. contents::
   :local:
   :depth: 3

Written by the `LLVM Team <https://llvm.org/>`_

.. only:: PreRelease

  .. warning::
     These are in-progress notes for the upcoming Extra Clang Tools |version| release.
     Release notes for previous releases can be found on
     `the Download Page <https://releases.llvm.org/download.html>`_.

Introduction
============

This document contains the release notes for the Extra Clang Tools, part of the
Clang release |release|. Here we describe the status of the Extra Clang Tools in
some detail, including major improvements from the previous release and new
feature work. All LLVM releases may be downloaded from the `LLVM releases web
site <https://llvm.org/releases/>`_.

For more information about Clang or LLVM, including information about
the latest release, please see the `Clang Web Site <https://clang.llvm.org>`_ or
the `LLVM Web Site <https://llvm.org>`_.

Note that if you are reading this file from a Git checkout or the
main Clang web page, this document applies to the *next* release, not
the current one. To see the release notes for a specific release, please
see the `releases page <https://llvm.org/releases/>`_.

What's New in Extra Clang Tools |release|?
==========================================

Some of the major new features and improvements to Extra Clang Tools are listed
here. Generic improvements to Extra Clang Tools as a whole or to its underlying
infrastructure are described first, followed by tool-specific sections.

Major New Features
------------------

...

Improvements to clangd
----------------------

Inlay hints
^^^^^^^^^^^

Diagnostics
^^^^^^^^^^^

Semantic Highlighting
^^^^^^^^^^^^^^^^^^^^^

Compile flags
^^^^^^^^^^^^^

Hover
^^^^^

Code completion
^^^^^^^^^^^^^^^

Signature help
^^^^^^^^^^^^^^

Cross-references
^^^^^^^^^^^^^^^^

Objective-C
^^^^^^^^^^^

Miscellaneous
^^^^^^^^^^^^^

Improvements to clang-doc
-------------------------

The improvements are...

Improvements to clang-query
---------------------------

The improvements are...

Improvements to clang-rename
----------------------------

The improvements are...

Improvements to clang-tidy
--------------------------

New checks
^^^^^^^^^^

<<<<<<< HEAD
=======
- New :doc:`bugprone-stringview-nullptr
  <clang-tidy/checks/bugprone-stringview-nullptr>` check.

  Checks for various ways that the ``const CharT*`` constructor of
  ``std::basic_string_view`` can be passed a null argument.

- New :doc:`abseil-cleanup-ctad
  <clang-tidy/checks/abseil-cleanup-ctad>` check.

  Suggests switching the initialization pattern of ``absl::Cleanup``
  instances from the factory function to class template argument
  deduction (CTAD), in C++17 and higher.

- New :doc:`bugprone-suspicious-memory-comparison
  <clang-tidy/checks/bugprone-suspicious-memory-comparison>` check.

  Finds potentially incorrect calls to ``memcmp()`` based on properties of the
  arguments.

- New :doc:`cppcoreguidelines-virtual-class-destructor
  <clang-tidy/checks/cppcoreguidelines-virtual-class-destructor>` check.

  Finds virtual classes whose destructor is neither public and virtual nor
  protected and non-virtual.

- New :doc:`misc-misleading-identifier <clang-tidy/checks/misc-misleading-identifier>` check.

  Reports identifier with unicode right-to-left characters.

- New :doc:`readability-container-data-pointer
  <clang-tidy/checks/readability-container-data-pointer>` check.

  Finds cases where code could use ``data()`` rather than the address of the
  element at index 0 in a container.

- New :doc:`readability-identifier-length
  <clang-tidy/checks/readability-identifier-length>` check.

  Reports identifiers whose names are too short. Currently checks local
  variables and function parameters only.

- New :doc:`misc-misleading-bidirectional <clang-tidy/checks/misc-misleading-bidirectional>` check.

  Inspects string literal and comments for unterminated bidirectional Unicode
  characters.

..
  INTEL_CUSTOMIZATION
- New :doc:`misc-homoglyph <clang-tidy/checks/misc-homoglyph>` check.

  Detects confusable unicode identifiers.
..
  end INTEL_CUSTOMIZATION

>>>>>>> 77fae35910cba043e49e07879841fd0ad38fe888
New check aliases
^^^^^^^^^^^^^^^^^

Changes in existing checks
^^^^^^^^^^^^^^^^^^^^^^^^^^

Removed checks
^^^^^^^^^^^^^^

Improvements to include-fixer
-----------------------------

The improvements are...

Improvements to clang-include-fixer
-----------------------------------

The improvements are...

Improvements to modularize
--------------------------

The improvements are...

Improvements to pp-trace
------------------------

The improvements are...

Clang-tidy Visual Studio plugin
-------------------------------
