==================================================================
MLPGO - Machine Learning inference of branch probabilities for PGO
==================================================================

.. contents::
   :local:

Introduction
============

Some optimizations use branch probability information(e.g. inliner). Without PGO
information LLVM doesn't know which basic block is more likely to be taken, so
it uses some heuristics, which often results in 50/50 for branch instruction with
two successors.

The feature tries to improve prediction by employing machine learning. So, the
probability is calculated by providing the machine learning engine with a feature
set describing specific branch instruction and its distinctive features.

In the future the machine learning technique might be used in other places where
LLVM uses heuristics.

The machine learning engine which is used for traning and inference is
ONNXRuntime - https://github.com/microsoft/onnxruntime

Collecting features for model training
======================================

Machine learning engine needs a pretrained model to do the inference. The model
is trained using a set of features and precise probabilities for each branch
instruction in the training set. The precise probabilities are collected during
compilation with instrumentation PGO profile, the step are:

1. Build a PGO instrumented application
2. Produce a profile by running it with representative input
3. Build the application again providing the profile collected during step #2
   and feature set dump enabled
   (TBD: MLPGO_OUTPUT=/tmp/profile.ai INTEL_MLPGO_USE="/tmp/code.profdata")
4. Train the model using produced dump

The PGOInstrumentationUse pass has been modified for feature dumping from the
third step. In addition to assigning branch probabilities the pass collects
branch instruction features and dumps them to a file.

Inference
=========

To enable machine learning based branch probabilities `-fprofile-ml-use` option
should be passed to the compiler driver(ICX/IFX). If this option is passed an
additional pass MLPGOInference is scheduled in the same place in the
optimization pipeline where regular PGO passes are.

The pass goes over code and for each conditional branch instruction:

1. Generates a feature vector
2. Calls machine learning engine providing it the feature vector to get branch
   probability
3. Updates the module metadata with the obtained branch probability

This information is used by subsequent passes as usual.


Branch feature vector
=====================

The feature vector is an array of numbers where each of them represents an IR
context the branch instruction is used in. For branch instruction the feature
vector consists of features describing basic block containing the branch
instruction(source basic block) and features describing each immediate successor
of such a basic block features. For example, the source basic block features
are number of instructions in this basic block and whether the basic block is
a loop header, the successors features are if the basic block is assumed to be
unlikely by LLVM heuristic and if the block has a store instruction.

The full list of collected features can be found in
llvm/include/llvm/Transforms/Instrumentation/MLPGO_Intel/FeatureDesc.def


Model integration
=================

The trained model is a file in a binary format (.onnx extension) which is used
by machine learning engine for inference of probability of each branch
instruction given a set of its IR features. The model is embedded to the
compiler binary as a resource during build using. This is done by converting the
binary file to a header file containing C-array with the same bytes the original
file has. This array is passed to the machine learning engine for
initialization.


ONNX Runtime
============

The compiler uses ONNX Runtime for inference. The ONNX runtime dynamic library
is linked as a dependency during compiler build. In the final compiler structure
the library is located in `lib/` directory on Linux and `bin/` directory on
Windows.


Environment variables
=====================

1. `INTEL_MLPGO_GEN` - dump features
2. `INTEL_MLPGO_USE=<filename>` - enable PGO mode with `<filename>` as profile
   data
3. `INTEL_MLPGO_MODEL_PATH=<filename>` - use `<filename` as machine learning
   model instead of embedded one
4. `MLPGO_OUTPUT=<filename>` - dump features to the `<filename>`
5. `MLPGO_REMOVE_NON_RUN` - do not dump features for not executed branches
6. `MLPGO_DUMP_WITH_INF` - dump feature with inference results
7. `MLPGO_DUMP_WITH_DEBUG_INFO` - dump features with additional debug info
8. `MLPGO_DUMP_JSON` - dump features in JSON format
9. `MLPGO_DUMP_UNKNOWN_FEATURES=<filename>` - dump unknown features to
    `<filename>`
10. `INTEL_MLPGO_PARTIAL_USE` - do not update function entry count
11. `INTEL_MLPGO_CG`
12. `INTEL_MLPGO_LTO`
13. `INTEL_MLPGO_CG_USE`
14. `INTEL_MLPGO_CG_GEN`
