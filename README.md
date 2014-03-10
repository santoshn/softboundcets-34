This is a README FILE for SoftBoundCETS pointer-based checking. For
more technical details and algorithms, visit SoftBoundCETS website at
http://www.cs.rutgers.edu/~santosh.nagarakatte/softbound/



Using SoftBoundCETS with LLVM+CLANG-3.4 on a x86-64 machine with Linux OS
=========================================================================


1. Download the github repository from http://www.github.com/santoshn/softboundcets-34

2. Build SoftBoundCETS for LLVM+3.4

   1. Goto to directory softboundcets-llvm-clang34 by executing the following command

            cd softboundcets-llvm-clang34

   2. Configure LLVM, clang and softboundcets with the following command

            ./configure --enable-assertions --disable-optimized

      If you prefer a faster compiler and do not need to debug SoftBoundCETS,
      use `--enable-optimized`.

      If you want to use SoftBoundCETS with LTO, follow the instructions at
      http://llvm.org/docs/GoldPlugin.html and add the
      `--with-binutils-include=/usr/include` parameter. Replace `/usr/include` by
      the folder that contains the `plugin-api.h` file.

   3. Build softboundcets, LLVM, clang with the following command

            make -j8

3. Set up your environment to use SoftBoundCETS

   For example in bash, it would be

         export PATH=<git_repo>/softboundcets-llvm-clang34/Debug+Asserts/bin:$PATH

   If you compiled an optimized build, the path is `Release+Asserts` instead of
   `Debug+Asserts`.

4. Compile the SoftBoundCETS runtime library

         cd <git_repo>
         cd softboundcets-lib
         make

   If you compiled the LLVM gold plugin, add the line below before calling
   make, in order to also build the SoftBoundCETS runtime library with LTO
   support.

         export LLVM_GOLD=<git_repo>/softboundcets-llvm-clang34/Debug+Asserts/lib/LLVMgold.so

5. Test whether it all worked

   1. Compile

            cd tests
            clang -fsoftboundcets test.c -o test -L<git_repo>/softboundcets-lib -lm
            clang -fsoftboundcets -flto test.c -o test-lto -L<git_repo>/softboundcets-lib/lto -lm

   2. Run the test program

            ./test

      Enter 10; the program executes successfully.

      Enter 105; a memory safety violation is triggered.
