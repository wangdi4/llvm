=========================================
DTrans Transformation Pass Infrastructure
=========================================

.. contents::
   :local:

.. toctree::
   :hidden:


Overview
========
This document provides a high-level design of data transformation (DTrans)
optimization passes that is common to all the specific transformations.

This document will evolve as the common infrastructure evolves.

High Level Design
=================
The DTrans optimizations need to change the layout of a data structure,
however, within the LLVM infrastructure once the body of StructureType has
been specified, there is not a means to change the body. This leads to the
need for the DTrans optimizations to create new StructureType objects for the
structure being optimized. However, this often requires other data structures
to need to be modified due to pointers to the original StructureType, or for
function signatures to need to be changed when pointers to structures are
passed. The DTransOptBase class, described in this document, is designed to
facilitate the replacement of these dependent types.

The DTransOptBase class is written to follow the template method design
pattern, where a skeleton of steps is defined, and a subclass is used to
implement steps that are optimization specific. Steps that are common to all
transformations will be handled by the base class.

The base class make extensive use of the existing LLVM infrastructure class to
perform the IR modifications to remap of one type to another type (ValueMapper),
and for cloning one function signature to a new function signature
(CloneFunctionInto).

Transformation Infrastructure Classes
=====================================

DTransTypeRemapper
~~~~~~~~~~~~~~~~~~

The DTransTypeRemapper is derived from the LLVM class ValueMapTypeRemapper
which is used by the ValueMapper functionality. This class provides the
functionality to compute the replacement types to use during the remapping
process. For example, if struct.testA is being replaced with MyOpt.struct.testA
then, a reference to the structure in the form of [4 x [5 x struct.testA*]]*
would need to be replaced with [4 x [5 x MyOpt.struct.testA*]]*. This class
is used to create a llvm::Type* object of that type.

A pointer to this class will be stored in the DTransOptBase class
for use by the transformation.

In order for remapped types to be computed, the transformation needs to inform
this class of the types the transformation is changing, and new type to be
used in its place using the following method:

.. code-block:: c++

  void addTypeMapping(llvm::Type *SrcTy, llvm::Type *DestTy);


ValueMaterializer
~~~~~~~~~~~~~~~~~
This is a standard LLVM class that is used by the ValueMapper, if supplied.
Use of this class is optional. Use of this class can enable a transformation
to replace one Value* with another Value*, when the remapFunction() and
cloneFunctionInto() methods are executing. Refer the ValueMapper.h header for
more details.

DTransOptBase
~~~~~~~~~~~~~
The DTransOptBase class is the base class that transformation specific classes
will derive from to get access to the common skeleton and type replacement
functionality.

An example of a simple derived class of the DTransOptBase can be found in the
DTransOptBaseTest class implementation. This class, which is used for testing
the base class functionality, simply replaces a StructureType with a new
StructureType that has the same layout. Reading the code for that class may
be helpful when following the description about usage of the base class
below.

Constructor
-----------

.. code-block:: c++

  // DTInfo        - DTrans Analysis Result
  // Context       - LLVM context for the module
  // DL            - Module's data layout
  // DepTypePrefix - Optional string to prefix structure names of rewritten
  //                 dependent types
  // TypeRemapper  - Class that will perform type mapping from old types
  //                 to new types
  // Materializer  - Optional class that works with ValueMapper to create
  //                 new Values during type remapping
  DTransOptBase(DTransAnalysisInfo &DTInfo, LLVMContext &Context,
                const DataLayout &DL, Twine &DepTypePrefix,
                DTransTypeRemapper *TypeRemapper,
                ValueMaterializer *Materializer = nullptr)

Prior to constructing this class, the TypeRemapper and Materialize class, if
used, should be created. This allows a transformation to create derived
versions of these classes, if customized behavior is needed.

There are two pure virtual functions of this class that the subclass must
define, prepareTypes() and populateTypes(). More details in the algorithm
description section.

The base class maintains the following variables that are accessible to the
derived classes:

.. code-block:: c++

  DTransTypeRemapper *TypeRemapper;
  ValueMaterializer *Materializer;
  // Mapping of original Value* to the replacement Value*. This mapping serves
  // two purposes.
  // 1: It is used by the ValueMapper to lookup whether a replacement for
  //    a value has been defined. Therefore, transformations can set items into
  //    this map prior to running the remapping to get those replacements to
  //    occur. This will be done for things like changing a function call to
  //    instead go to a cloned function.
  //  2: This mapping also gets populated as the replacements are created during
  //     the remapping process. This allows finding what value was used as
  //     the replacement.
  // Initially it will be primed with the global variables and functions that
  // need cloning. As the ValueMapper replaces values those will get inserted.
  ValueToValueMapTy VMap;


DTransOptBase Algorithm
=======================
Following the construction of the subclass of the DTransOptBase class, the
run method should be invoked with the Module to be transformed as an
argument. This will execute all the steps of transformation algorithm.

Example:

.. code-block:: c++

  DTransTypeRemapper TypeRemapper;
  DTransOptBaseTest Transformer(DTInfo, M.getContext(), M.getDataLayout(),
                                "__DDT_", &TypeRemapper);
  return Transformer.run(M);

The run method will perform the following steps:

Step 1: The DTransOptBase invokes the subclass method 'prepareTypes' which
is responsible for creating opaque types for any structures being replaced,
and adding mappings for them to the TypeRemapper.

This is done by the base class invoking the prepareTypes method of the
subclass.

.. code-block:: c++

  bool prepareTypes(Module &M)

For example the AOS to SOA transform class prepares opaque types for new types,
and informs the TypeRemapper 'struct.node' is to be replaced with a new type,
'__soa_struct.node', and the type 'struct.node*' is to be replaced with 'i64'.


Step 2: The DTransOptBase class determines which types contained references
to types defined in step 1, and creates opaque types for them. It also creates
new opaque types for any types these new types reference.

Step 3: The DTransOptBase class uses the TypeRemapper to compute the types
needed to populate the body of the opaque types created by the base class in
step 2.

Step 4: The DTransOptBase class invokes the subclass method 'populateTypes'
which is responsible for populating any new types that the derived class
created in step 1.

Step 5: The DTransOptBase class invokes the subclass method 'prepareModule', if
this method is defined. This method should perform any global variable creation
that are needed by for transformation.

Step 6: The DTransOptBase class creates any new function prototypes for any
functions that need their signature to be changed due to the types replacements
of steps 1 and 2. These get inserted into the VMap member to identify what the
original function or function arguments are being replaced with.

Step 7: The DTransOptBase class in coordination with the derived class creates
new global variables based on the type changes of steps 3 and 4. These get
inserted in the "VMap" member of the base class. For any global variable that
needs to be replaced, the derived class is given a chance to take control of
that variable with a call to createGlobalVariableReplacement(). If the derived
class takes control over that variable, then the derived class will be
responsible for initializing the replacement within a call to
initializeGlobalVariableReplacement(). After the initialization replacement
has been completed for each global variable the DTransOptBaseClass will call
postProcessGlobalVariable() to allow derived classes an opportunity to
update uses of that global within the IR.

Step 8: For each function:
  8a. The DTransOptBase class invokes the subclass method 'processFunction'.
  This is intended to be where instructions of the function get replaced by the
  specific replacements needed on the direct types being modified by the
  optimization.

  8b. The DTransOptBase class clones the function using cloneFunctionInto if
  the signature of the function needs changed. Otherwise, it uses the
  ValueMapper.remapFunction method to remap the types of the function. The
  LLVM infrastructure will update the "VMap" member of the base class to
  provide a mapping from the original Value* object to the new Value* object.

  8c. The DTransOptBase class invokes the subclass method 'postprocessFunction'
  with a pointer to the original function that has now been remapped. If the
  function was cloned, the child class should look up the Function* object of
  the cloned function in the VMap to get the remapped Function*.
