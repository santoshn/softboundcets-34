all: softboundcets_rt


softboundcets_rt: softboundcets-checks.c softboundcets.c
	clang -c -Wall -pedantic -O3 -D__SOFTBOUNDCETS_TRIE -D__SOFTBOUNDCETS_SPATIAL_TEMPORAL softboundcets-checks.c -o softboundcets-checks.o
	clang  -c -Wall -pedantic -O3 -D__SOFTBOUNDCETS_TRIE -D__SOFTBOUNDCETS_SPATIAL_TEMPORAL softboundcets.c -o softboundcets.o
	clang  -c -Wall -pedantic -O3 -D__SOFTBOUNDCETS_TRIE -D__SOFTBOUNDCETS_SPATIAL_TEMPORAL softboundcets-wrappers.c -o softboundcets-wrappers.o
	ar rcs libsoftboundcets_rt.a softboundcets.o softboundcets-checks.o softboundcets-wrappers.o

clean: 
	rm -rf *.o *.a *~

