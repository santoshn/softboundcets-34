This is a README FILE for SoftBoundCETS pointer-based checking. For
more technical details and algorithms, visit SoftBoundCETS website at
http://www.cs.rutgers.edu/~santosh.nagarakatte/softbound/



Instructions for using the integrated version of SoftBoundCETS with
LLVM+CLANG-3.4 on a x86-64 machine with Linux OS
===================================================================


(1) Download the github repository from http://www.github.com/santoshn/softboundcets-34

(2) Goto directory softboundcets-lib

(3) Build the softboundcets instrumenter library by typing "make" in
softboundcets-lib directory

(4) Build SoftBoundCETS for LLVM+3.4 

   (a) Goto to directory softboundcets-llvm-clang34 by executing the following command

      cd softboundcets-llvm-clang34

   (b) Configure the llvm, clang and softboundcets with the following command

     ./configure --enable-assertions --disable-optimized

   (c) Build softboundcets, llvm, clang with the following command
     
      make -j8

(5) Running a program with softboundcets
 
    (a) Set PATH environment variable to point to binary directory

     	For example in bash, it would be

	export PATH=<git_repo>/softboundcets-llvm-clang34/Debug+Asserts/bin:$PATH
	export LD_LIBRARY_PATH=<git_repo>/softboundcets-lib/


    (b) Goto to tests directory with the following command

        cd tests
	clang -fsoftboundcets test.c -o test.out -L/<git_repo>/softboundcets-lib/ -lm

	
    (c) Run the test program

        ./test.out

	enter 10

	Program executes successfully

	enter 105
	
	Memory safety violation is triggered.
	
	
    
	
	
     