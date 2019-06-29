offload: offload.c.c
	clang -fopenmp -fopenmp-targets=nvptx64 $< -o $@

%.c.c:  %.c FORCE
	clang -fsyntax-only -Xclang -load -Xclang /home/pschen/llvm/latest/build/lib/OpenMPRewrite.so -Xclang -plugin -Xclang omp-rewtr $< -o $@ 

.PHONY: FORCE
FORCE:
