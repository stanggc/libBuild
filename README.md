# libBuild

libBuild is a cross-platform build automation library primarily for C/C++ projects,
implemented in C++, that lets you write build programs in C or C++.
A build program is similar to shell scripts and build scripts.

libBuild enables build logic to be written not just in C++, but also C via its
C bindings.

## How to use

To get a sense of how implementing your own build program using libBuild works, refer to
`build.cc` in this repository, which is the build source file for libBuild. Also,
refer to `Build.h`, `Test_Build.cc` and `Test_Build.c` for more usage examples.

libBuild initialises default commands based on GCC toolchain (`gcc`, `g++`, etc).
This can be changed by setting the various properties in `Builder` (C++) struct,
such as `CCCommand` and `CXXCommand`. If you are using the C bindings,
functions such as `Build_SetCCCommand()` and `Build_SetCXXCommand()` should be used instead.

If you are using different C/C++ compilers, you should change the commands,
after `Builder` (C++) / `BuildConfig` (C) is initialised.

Nonetheless, examples as follows.

### Example using C++

```c++
// Build program source file in C++.
#include <iostream>
#include <string>

#include <Build.h>

using std::cout;
using std::string;
using Build::Builder;

int main(int argc, char *argv[]) {
	Builder b;
	string cmd;

	try {
		if (argc > 1) {
			for (int i = 1; i < argc; ++i) {
				cmd = argv[i];
				if (cmd == "invoke") {
					b.DryRun = false;
				}
				if (cmd == "build") {
					b.CC("-o %s %s", b.ExecutableFileName("example_hello").c_str(), "example_hello.c");
				}
				if (cmd == "clean") {
					b.Remove(b.ExecutableFileName("example_hello"));
				}
			}
		}

		return 0;
	} catch (std::exception &e) {
		cout << "Error: " << e.what() << "\n";
		return 1;
	}
}

```

Compile and invoke:

```shell
$ g++ -o example_cxx example.cc -I. -L. -lBuild

# Dry run.
$ ./example_cxx build

# Actually invoke.
$ ./example_cxx invoke build

# Dry run clean.
$ ./example_cxx clean

# Actually clean.
$ ./example_cxx invoke clean
```

### Example using C

Note that the C APIs uses an `errno`-style approach for error handling.
The error status code is stored in a thread-local variable
named `BStatusCode`.

```c
// Build program source file in C.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Build.h>

int main(int argc, char *argv[]) {
	BuildConfig *b = NULL;
	char *exeFileName = NULL;

	b = Build_InitBuildConfig();
	if (!b) goto cleanUp;

	exeFileName = Build_ExecutableFileName("example_hello");
	if (!exeFileName) goto cleanUp;

	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "invoke")) {
			if (Build_SetDryRun(b, false)) goto cleanUp;
		}
		if (!strcmp(argv[i], "build")) {
			// Assume we have a hello world program, saved as hello.c.
			// We want to build a executable named `hello` (and `hello.exe` on Windows).
			if (Build_CC(b, "-o %s %s", exeFileName, "example_hello.c")) goto cleanUp;
		}
		if (!strcmp(argv[i], "clean")) {
			if (Build_Remove(b, exeFileName)) goto cleanUp;
		}
	}

cleanUp:
	if (exeFileName) free((void *) exeFileName);
	Build_DeinitBuildConfig(b);
	if (BStatusCode) {
		printf("Error: %s\n", Build_StatusCodeMessage(BStatusCode));
		return 1;
	} else {
		return 0;
	}
}

```

Compile and invoke the example with the following:

```shell
$ gcc -o example_c example.c -I. -L. -lBuild -lstdc++

# Dry run.
$ ./example_c build

# Actually invoke.
$ ./example_c invoke build

# Dry run clean.
$ ./example_c clean

# Actually clean.
$ ./example_c invoke clean
```

## Building

To build libBuild, the build program needs to be built first, before libBuild can be built.

```shell
# Windows.
$ g++ -o build.exe build.cc bootstrap.cc -I.
# - or -
$ g++ -o build.exe build.cc bootstrap.cc -I. -DWINDOWS

# MacOS.
$ g++ -o build build.cc bootstrap.cc -I. -DMACOS

# Linux.
$ g++ -o build build.cc bootstrap.cc -I. -DLINUX

# Unix-based OSes other than MacOS and Linux.
$ g++ -o build build.cc bootstrap.cc -I. -DUNIX
```

libBuild assumes that a GCC toolchain exists, even when building on Windows.

Note the use of the `-D` switch to define the macros for respective OSes.

Once the build program is built, invoke it to build the library.

```shell
# Build program help text.
$ ./build
# - or -
$ ./build help

# Dry run build.
$ ./build build

# Actually build.
$ ./build invoke build
```

A static library archive file `libBuild.a` will be generated.

If desired, you can now build the build program with the library instead:

```shell
$ g++ -o build build.cc -I. -L. -lBuild
```

To clean up the build artifacts:

```shell
# Clean the built library.
$ ./build invoke clean
```

You can provide multiple commands to `build`. It will be invoked in the order that were provided,
from left to right.

```shell
$ ./build invoke build build-tests build-examples clean-tests clean-examples clean
```

## Usage

To use libBuild in your projects, use something like the following:

```shell
$ g++ -I "<path to libBuild's Build.h>" -L "<path to libBuild.a>" "<your build program source file>" -lBuild
```

## Testing

To run the testsuite, if interested:

```shell
# Build the library first, if not yet done.
$ ./build invoke build

$ ./build invoke build-tests
$ ./Test_Build_C
$ ./Test_Build_CXX

$ ./build invoke clean-tests
```

The tests are in `Test_Build.c` (for testing C APIs), and `Test_Build.cc` (for testing C++ APIs).

Run `Test_Build_C` / `Test_Build_C.exe` (C), or `Test_Build_CXX` / `Test_Build_CXX.exe` (C++).

A return code of zero indicates tests pass; non-zero otherwise.

## Licence

MIT licence. See LICENSE.txt.
