#include <iostream>
#include <string>
#include <stdexcept>

#include <Build.h>

using std::cout;
using std::string;
using std::runtime_error;
using Build::Builder;

static void PrintHelp(const char *exePath) {
	const char *cmds[] = {
		"help",
		"build", "clean",
		"build-tests", "clean-tests",
		"build-examples", "clean-examples",
		NULL,
	};

	cout << "Available commands:\n";
	for (int i = 0; cmds[i]; ++i) {
		cout << "\t" << cmds[i] << "\n";
	}
	cout << "\n";
	cout << "Commands are dry-run by default.\n";
	cout << "To actually invoke, specify `invoke` before the first command.\n";
	cout << "Example: " << exePath << " invoke build\n";
}

static void BuildLibrary(Builder &b) {
	b.CC("-fPIC -c Build_Builder.cc Build_Functions.cc");
	b.AR("cr libBuild.a Build_Builder.o Build_Functions.o");
}

static void CleanLibrary(Builder &b) {
	b.Remove("libBuild.a");
	b.Remove("Build_Builder.o");
	b.Remove("Build_Functions.o");
}

static void BuildTests(Builder &b) {
	string exeFileName;

	// NOTE
	// On Windows, GCC likely will not work correctly,
	// if `-l` switches are specified before source files,
	// so we need to put it behind.
	string compileParams = "-I. -L. -o \"%s\" \"%s\" -lBuild";

	// Test_Build, C version.
	exeFileName = b.ExecutableFileName("Test_Build_C");
	b.CC(compileParams + " -lstdc++", exeFileName.c_str(), "Test_Build.c");

	// Test_Build, C++ version.
	exeFileName = b.ExecutableFileName("Test_Build_CXX");
	b.CXX(compileParams, exeFileName.c_str(), "Test_Build.cc");
}

static void CleanTests(Builder &b) {
	string exeFileName;

	exeFileName = b.ExecutableFileName("Test_Build_C");
	b.Remove(exeFileName);

	exeFileName = b.ExecutableFileName("Test_Build_CXX");
	b.Remove(exeFileName);
}

static void BuildExamples(Builder &b) {
	string parameters = "-o %s %s -I. -L. -lBuild";
	string cExeFileName, cxxExeFileName;

	cExeFileName = b.ExecutableFileName("example_c");
	b.CC(parameters + " -lstdc++", cExeFileName.c_str(), "example.c");
	cxxExeFileName = b.ExecutableFileName("example_cxx");
	b.CXX(parameters, cxxExeFileName.c_str(), "example.cc");
}

static void CleanExamples(Builder &b) {
	string cExeFileName, cxxExeFileName;

	cExeFileName = b.ExecutableFileName("example_c");
	b.Remove(cExeFileName);
	cxxExeFileName = b.ExecutableFileName("example_cxx");
	b.Remove(cxxExeFileName);
}

int main(int argc, char *argv[]) {
	Builder b;
	string osMacro;
	string cmd;
	const char *exePath = argv[0];

	try {
		b.SetConsoleCodePage("utf-8");
		b.ChDirToProgramDir(argc, argv);
		b.DryRun = true;
		b.PrintCommandToStdout = true;

		if (b.IsWindows()) {
			osMacro = "-DWINDOWS";
		} else if (b.IsMacOS()) {
			osMacro = "-DMACOS";
		} else if (b.IsLinux()) {
			osMacro = "-DLINUX";
		} else if (b.IsUnix()) {
			osMacro = "-DUNIX";
		} else {
			throw runtime_error("unknown OS");
		}
		if (osMacro != "") {
			b.CCCommand = b.CCCommand + " " + osMacro;
			b.CXXCommand = b.CXXCommand + " " + osMacro;
		}

		if (argc > 1) {
			for (int i = 1; i < argc; ++i) {
				cmd = argv[i];
				if (cmd == "invoke") {
					b.DryRun = false;
				} else if (cmd == "build") {
					BuildLibrary(b);
				} else if (cmd == "clean") {
					CleanLibrary(b);
				} else if (cmd == "build-tests") {
					BuildTests(b);
				} else if (cmd == "clean-tests") {
					CleanTests(b);
				} else if (cmd == "build-examples") {
					BuildExamples(b);
				} else if (cmd == "clean-examples") {
					CleanExamples(b);
				} else if (cmd == "help") {
					PrintHelp(exePath);
				} else {
					cout << "Unknown command: " << cmd << "\n";
				}
			}
		} else {
			PrintHelp(exePath);
		}

		return 0;
	} catch (std::exception &e) {
		cout << "Error: " << e.what() << "\n";
		return 1;
	}
}
