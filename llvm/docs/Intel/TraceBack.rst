=======================
Traceback Specification
=======================

.. contents::
   :local:

Introduction
============

Traceback is a set of run-time library routines and associated image file data
structures that allows the RTL to display the PC where an error occurred, the
routine, file, and line number containing that PC, the PC/routine/file/line from
which that routine was called, the same info for its caller, and so on up the
call stack.

The traceback facility has two major components: executable code that can walk
the call stack (depending on .eh_frame), displaying the traceback information as
it goes, and a set of data structures that provide the PC-to-source correlation.
This specification describes the data structure component of traceback.


Goals, Non-goals and Alternative Designs
========================================

These are not in any priority order. “Must” indicates hard requirements.
“Should” indicates desirable characteristics that may be traded off against
other design concerns.

``Goals``
---------
- **Orthogonal to debug information** Traceback must be able to operate
  independent of the presence of symbolic debugging information in an executable
  or shareable image.
- **Compactness** Traceback information should take up as little space in object
  and image files as possible.
- **Platform support** Traceback must be supported on at least 32/64-bit
  Windows, Linux and MacOS(not supported yet).
- **Platform independence** To reduce the burden of supporting multiple variants
  of the code that produces and interprets the traceback data, the data format
  should be as similar as possible between the various target platforms.
- **Source language independence** Traceback should be designed such that it can
  be used with programming languages other than Fortran.

``Non-Goals``
-------------
- **Run-time performance** The primary purpose of Traceback is to provide a
  formatted call stack trace when a fatal application error occurs. Since the
  application usually will be terminating anyway, the time taken to produce the
  trace is not a critical concern.

``Alternative Designs Considered``
----------------------------------
Object module debug information (DWARF, CodeView) contains all of the source
correlation information needed to perform a traceback dump. One could,
therefore, implement traceback by extracting the interesting data from the
debug information of DWARF/CodeView in the case of a debug compilation.

The **advantage** to this approach is:

- Object file size savings for the "-traceback -g" case.

The **disadvantages** are:

- Loss of traceback information when debug information is stripped.
- Verbosity. The DWARF line number information contains information not needed
  for traceback and is not as compact as it could be. Studies of some
  representative object modules show that the DWARF source correlation
  information is about 50% larger than the same information represented as
  documented below.

Use of stripped-down DWARF to represent traceback information was rejected
because it was thought that the disadvantages outweigh the advantages.


Design Overview
===============
In the object file, the traceback information is contained in a dedicated,
readily identifiable object file section (conventionally named .trace). When a
routine is included in an object file section that may conditionally be excluded
from the image (i.e., COMDAT), its traceback information appears in a section
that will only be included in the image if the routine is (i.e., an associated
COMDAT). Normal Linker section processing (concatenation) consolidates the
traceback information into a single, contiguous data block that the RTL
traceback code can readily find by navigating the image file header information.

A traceback section contains a sequence of records that, together, describe the
mapping of program counter (PC) values to file/module/routine/line values. The
records are interpreted in sequence to obtain the correlations. The interpreter
must keep track of a current file (curfile), current module (curmodule), current
routine (curroutine), current PC (curPC), and current line number (curline) as
it processes the records. The first byte of each record identifies the record
type.

The first record in the sequence must be a header (HDR) record. The header
describes the properties of a contiguous set of traceback records:

- The major and minor version ID of the traceback information format that this
  set of records conforms to. Any upward-compatible change to the format
  requires incrementing the minor ID. An incompatible change requires changing
  the major ID. Thus, a processor of traceback section data knows that it can
  understand the set of records covered by a HDR record if the major ID exactly
  matches that of the format it understands and the minor ID is less than or
  equal to that of the format it understands. Changes to the traceback format
  must not change the HDR record’s record type value, nor the position in a HDR
  record of the major ID, minor ID, and length fields.
- The length in bytes of the set of records covered by the header.
- The contiguous range of PC values used by the records covered by the header.
- The module name (if any) associated with the records covered by the header.
- The file specification strings referred to by other records covered by the
  header. The strings reside in the HDR record so that they need appear only
  once.

A routine (RTNxx) record starts a set of records associated with a particular
routine. It contains a starting PC value and a routine name.

A file (FIL) record starts a set of records associated with a particular file.
It designates the file by an index into the list of files in the HDR record.

A PAD record is a no-op to allow other records to be aligned on natural
boundaries.

The remaining records define source correlations. A source correlation maps a
range of PC values to a source file line. The correlations direct the
interpreter to add a signed delta value to curline, giving a new current line
value that is the line number for the correlation. The curPC value is the start
of the PC range, and the correlation specifies an unsigned delta value that
gives the length of the PC range. Thus, to interpret a source correlation, the
interpreter follows these steps:

1. Add the correlation’s line number delta to curline.

2. Record the mapping: the PC range starting a curPC, and extending for the PC
   delta given in the correlation, maps to the routine, module, file and line
   number given by curroutine, curmodule, curfile, and curline.

3. Add the PC delta given in the correlation to curPC.

Note that since 0 is not a valid PC delta value, and PC deltas are always
positive, the value stored in a record is one less than the actual delta value
(i.e., 0 in the record means a delta value of 1). This ekes out one more value
in each range.

There are two “short form” source correlations that encode small PC and line
deltas compactly into 1-byte and 2-byte records. The CO1 record covers a line
delta of exactly +1 and PC deltas of 1-64, inclusive. The CO2 record covers
line deltas in the range –128 to 127 and PC deltas 1 to 64, both inclusive.

If a correlation’s deltas won’t fit into one of the “short form” records, the
correlation is expressed as a pair of records, the first giving the line delta
and the second giving the PC delta. The line delta records are LN1 (one-byte
line delta, covering –128 to 127, inclusive), LN2 (two-byte line delta, covering
–32768 to 32767, inclusive), and LN4 (4-byte line delta). The PC delta records
are PC1 (one-byte PC delta, covering 1 to 256, inclusive), PC2 (two-byte PC
delta, covering 1 to 65536, inclusive), PC4 (four-byte PC delta).

Object File Formats
===================

``Section Format``
------------------
In these descriptions “address” means a 4-byte field for IA-32 and an 8-byte
field for x86-64.

The section must be named .trace on all the platforms and must be aligned to
the system's address-sized boundary. The different target platforms use
different object file formats and hence have different requirements regarding
section header characteristics.

``Windows COFF``
----------------

.. code-block:: C

 IMAGE_SCN_MEM_READ
 IMAGE_SCN_CNT_INITIALIZED_DATA

The routines referred to by the records in a .trace section may be in more than
one object file section provided that none of the sections are communal data
sections (i.e., have the characteristic IMAGE_SCN_LNK_COMDAT). A .trace section
that refers to a routine in a COMDAT must not refer to routines contained in any
other object file section (it may refer to other routines contained in the same
COMDAT). Furthermore a .trace section that references routines in a COMDAT must
itself be a COMDAT associated with the section containing the routines (i.e.,
the .trace section must have additional characteristic IMAGE_SCN_LNK_COMDAT, and
the section’s symbol table entry must specify that it is associated with the
COMDAT containing the referenced routines). These special rules regarding
COMDATs insure that traceback information for a routine will appear only if the
Linker includes the code for the routine.

``Linux ELF``
-------------
The section header must have fields set as follows:

========  ============
sh_type   SHT_PROGBITS
sh_flags  SHF_ALLOC
========  ============

And if PIC is enabled, the section header has one more flag SHF_WRITE.

``MacOS MachO``
---------------
To be done.

``Records Formats``
-------------------
Unless otherwise specified, records and their fields are unaligned.

The major/minor ID value of 2.0, and the record type numeric values, are chosen
so that attempts to mix traceback sections conforming to this specification with
those previously used by icc/ifort can be detected.

**HDR**—Header. Describes a range of traceback records, the version of the
traceback format that the records use, the PC range covered by the records, and
the module and file names that will be referred to by those records.

The interpreter must clear curroutine and curfile and must set curPC and curline
to zero. It must set curmodule to the module name given in the record. The
fields are:

- **Record type** (1 byte): always 0x0a
- **Major ID value** (2 bytes): unsigned major ID value (always 2 for the format
  described here)
- **Minor ID value** (1 byte): unsigned minor ID value (always 0 for the format
  described here)
- **Length** (4 bytes): unsigned length in bytes of the records that this HDR
  record describes (including the length of the HDR record itself)
- **Base PC** (address): lowest PC value covered by this header
- **File count** (4 bytes): unsigned number of file name strings
- **Code length** (4 bytes): bytes of PC (starting at the base) covered by this
  header
- **Module name**: a 2-byte unsigned length (in bytes) followed by the module
  name string itself. (always empty in current implementation) The string is not
  NUL-terminated. A length of 0 means that there is no module name.

**RTN32**—Set Routine (32-bit addresses). Sets new curroutine and curPC values.
The record must be aligned on a 4-byte boundary.

- **Record type** (1 byte): always 0x02
- **Pad** (1 byte): always zero
- **Routine name length** (2 bytes): unsigned length of the routine name field
- **Start PC** (4 bytes): starting PC value for the routine. The interpreter
  sets curPC to this value.
- **Routine name**: the name of the routine as a string that is not
  NUL-terminated. The interpreter sets curroutine to this value.

**RTN64**—Set Routine (64-bit addresses). Sets new curroutine and curPC values.
The record must be aligned on a 8-byte boundary.

- **Record type** (1 byte): always 0x0c
- **Pad** (1 byte): always zero
- **Routine name length** (2 bytes): unsigned length of the routine name field
- **Start PC** (8 bytes): starting PC value for the routine. The interpreter
  sets curPC to this value.
- **Routine name**: the name of the routine as a string that is not
  NUL-terminated. The interpreter sets curroutine to this value.

**FIL**—Set File. Sets a new curfile value.

- **Record type** (1 byte): always 0x03
- **File index** (4 bytes): zero-based index into the list of files in the HDR
  record of the file string. The interpreter sets curfile to this value.

**LN1**—One-byte Line Delta.

- **Record type** (1 byte): always 0x04
- **Line delta** (1 byte): signed line number delta value

**LN2**—Two-byte Line Delta.

- **Record type** (1 byte): always 0x05
- **Line delta** (2 byte): signed line number delta value


**LN4**—Four-byte Line Delta.

- **Record type** (1 byte): always 0x06
- **Line delta** (4 byte): signed line number delta value


**PC1**—One-byte PC Delta.

- **Record type** (1 byte): always 0x07
- **PC delta** (1 byte): unsigned PC delta value minus 1

**PC2**—Two-byte PC Delta.

- **Record type** (1 byte): always 0x08
- **PC delta** (2 byte): unsigned PC delta value minus 1

**PC4**—Four-byte PC Delta.

- **Record type** (1 byte): always 0x09
- **PC delta** (4 byte): unsigned PC delta value minus 1

**CO1**—One-byte “short form” correlation.

- **Record type** (high 2 bits): always 10 (binary)
- **PC delta** (low 6 bits): unsigned PC delta value minus 1

**CO2**—Two-byte “short form” correlation.

- **Record type** (high 2 bits): always 11 (binary)
- **PC delta** (low 6 bits): unsigned PC delta value minus 1
- **Line delta** (1 byte): signed line number delta value


Options for Xmain
=================

To generate the debug information of traceback, you can pass option "-traceback"
to the driver

.. code-block:: bash

 clang/icx/ifx -traceback -c test.c

And llvm-objdump can dump the information of .trace section with option
"--traceback"

.. code-block:: bash

 llvm-objdump --traceback test.o
