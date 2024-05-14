#include <iostream>
#include <string>
#include <cassert>
#include <libgen.h>
#include <unistd.h>

#include "Build.h"

using std::cout;
using std::string;
using std::runtime_error;
using Build::Builder;

static string OSNameUpper() {
	if (Builder::IsWindows())
		return "WINDOWS";
	else if (Builder::IsMacOS())
		return "MACOS";
	else if (Builder::IsLinux())
		return "LINUX";
	else if (Builder::IsUnix())
		return "UNIX";
	else
		throw runtime_error("unknown OS");
}

static const char *unknownCode = "unknown status code";

int main(int argc, char *argv[]) {
	Builder b, b2;
	char *cCwd = NULL;
	string cwdBeforeChDir;
	string cwd;
	string exeDir;

	try {
		// Test change dir to executable's directory.
		try {
			b.ChDirToProgramDir(0, argv);
			assert(false); // should not be able to chdir to program dir.
		} catch (std::exception &e) {
			assert(string(e.what()) == "missing executable file path");
		}
	#if defined(WINDOWS)
		assert(b.DirName(argv[0]) == b.GetCurrentWorkingDir());
	#else
		assert(b.DirName(argv[0]) == ".");
	#endif
		cCwd = getcwd(NULL, 0);
		cwdBeforeChDir = cCwd;
		if (cCwd) free((void *) cCwd);
		assert(cwdBeforeChDir != "");
		b.ChDirToProgramDir(argc, argv);
		cwd = b.GetCurrentWorkingDir();
		assert(cwd == cwdBeforeChDir);

		// Ensure we can display UTF-8 on the console.
		Builder::SetConsoleCodePage("utf-8");

		// Test that dry run is enabled by default.
		assert(b.DryRun);

		// Don't print commands for tests, unless we are debugging.
		b.PrintCommandToStdout = false;

		// Test that get methods are OK.
		assert(b.CCCommand == "gcc");
		assert(b.CLanguageStandard == "");
		assert(b.CXXCommand == "g++");
		assert(b.CXXLanguageStandard == "");
		assert(b.ARCommand == "ar");
		assert(b.LDCommand == "ld");
		if (b.IsWindows()) {
			assert(b.MoveCommand == "move");
			assert(b.CopyCommand == "copy");
			assert(b.RemoveCommand == "del");
		} else if (b.IsMacOS() || b.IsLinux() || b.IsUnix()) {
			assert(b.MoveCommand == "mv");
			assert(b.CopyCommand == "cp");
			assert(b.RemoveCommand == "rm -f");
		} else {
			throw runtime_error("unknown OS");
		}

		// Test.
		// Test CC.
		b.CC("-c %s", "Builder.cc");
		assert(b.LastExecCommand == "gcc -c Builder.cc");
		// Test setting C language standard.
		b.CLanguageStandard = "c17";
		b.CC("-c %s", "Builder.cc");
		assert(b.LastExecCommand == "gcc -std=c17 -c Builder.cc");
		// Test CXX.
		b.CXX("-o Test_Builder Test_Builder.cc Builder.cc");
		assert(b.LastExecCommand == "g++ -o Test_Builder Test_Builder.cc Builder.cc");
		// Test setting C++ language standard.
		b.CXXLanguageStandard = "c++17";
		b.CXX("-o Test_Builder Test_Builder.cc Builder.cc");
		assert(b.LastExecCommand == "g++ -std=c++17 -o Test_Builder Test_Builder.cc Builder.cc");
		// Test AR command.
		b.AR("cru %s %s", "libbuild.a", "Builder.o");
		assert(b.LastExecCommand == "ar cru libbuild.a Builder.o");
		// Test LD command.
		b.LD("-o Test test.o");
		assert(b.LastExecCommand == "ld -o Test test.o");

		// Test executable file name.
		b.CXX("-o %s %s", b.ExecutableFileName("Test").c_str(), "Test.cc");
		if (Builder::IsWindows()) {
			assert(b.LastExecCommand == "g++ -std=c++17 -o Test.exe Test.cc");
		} else if (Builder::IsMacOS() || Builder::IsLinux() || Builder::IsUnix()) {
			assert(b.LastExecCommand == "g++ -std=c++17 -o Test Test.cc");
		}

		// Test actual invocation, compilation, move, copy and remove.
		b.DryRun = false;
		b.Exec("echo \"Testing...\"");
		assert(b.LastExecCommand == "echo \"Testing...\"");
		string OSMacro = "-D" + OSNameUpper();
		b.CXXCommand = b.CXXCommand + " " + OSMacro;
		b.CXX("-o Builder__test.o -c Build_Builder.cc");
		assert(b.FileExists("Builder__test.o"));
		assert(b.LastExecCommand == "g++ " + OSMacro + " -std=c++17 " + "-o Builder__test.o -c Build_Builder.cc");
		b.Copy("Builder__test.o", "Builder__test2.o");
		b.Move("Builder__test.o", "Builder__test1.o");
		assert(b.FileExists("Builder__test1.o"));
		assert(b.FileExists("Builder__test2.o"));
		assert(!b.FileExists("Builder__test.o"));
		b.Remove("Builder__test1.o");
		b.Remove("Builder__test2.o");
		assert(!b.FileExists("Builder__test.o"));
		assert(!b.FileExists("Builder__test1.o"));
		assert(!b.FileExists("Builder__test2.o"));

		// Test that we don't double free() the CBuilder's LastExecCommand property,
		// if copy assignment is to be used.
		b2 = b;


		// Test status codes have messages.
		assert(string(Build_StatusCodeMessage(B_OK)) != unknownCode);
		assert(string(Build_StatusCodeMessage(B_Mem)) != unknownCode);
		assert(string(Build_StatusCodeMessage(B_UnknownOS)) != unknownCode);
		assert(string(Build_StatusCodeMessage(B_UnknownConsoleCodePage)) != unknownCode);
		assert(string(Build_StatusCodeMessage(B_ObjectRequired)) != unknownCode);
		assert(string(Build_StatusCodeMessage(B_InvokeFailed)) != unknownCode);
		assert(string(Build_StatusCodeMessage(B_ShellInvokeFailed)) != unknownCode);
		assert(string(Build_StatusCodeMessage(B_StatFailed)) != unknownCode);
		assert(string(Build_StatusCodeMessage(B_DirNameFailed)) != unknownCode);
		assert(string(Build_StatusCodeMessage(B_ChDirFailed)) != unknownCode);
		assert(string(Build_StatusCodeMessage(B_CurrentWorkingDirFailed)) != unknownCode);
		assert(string(Build_StatusCodeMessage(B_MissingExecutableFilePath)) != unknownCode);


		cout << "OK了: Test_Build_CXX\n";
		return 0;
	} catch (std::exception &e) {
		cout << "Error: " << e.what() << "\n";
		return 1;
	}
}
