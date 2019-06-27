This is a simple example demonstrating how to use clang plugin and clang rewriter.

The goal is to rewrite orinal source code, automatically add runtime function call in certain statements.

# Build

1. Have your own llvm/clang
2. Put this project under clang/examples
3. re-cmake and build
4. You will find OpenMPRewrite.so under $BUILD/lib

Once the plugin is built, you can run it using:
--
Linux:
$ clang -cc1 -load $BUILD/lib/OpenMPRewrite.so -plugin omp-rewtr some-input-file.c
$ clang -cc1 -load $BUILD/lib/OpenMPRewrite.so -plugin omp-rewtr -plugin-arg-omp-rewtr help -plugin-arg-omp-rewtr --example-argument some-input-file.c
