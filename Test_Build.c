#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>

#include "Build.h"

#if defined(WINDOWS)
static int DoVasprintf(char **out, const char *fmt, ...) {
	va_list args;
	int ret = 0;

	va_start(args, fmt);
	ret = vasprintf(out, fmt, args);
	va_end(args);

	return ret;
}

static void TestAsprintfAndFriends() {
	char *outAsprintfStr = NULL;

	assert(asprintf(&outAsprintfStr, "-DWINDOWS %s", "-I.") != -1);
	assert(outAsprintfStr);
	assert(!strcmp(outAsprintfStr, "-DWINDOWS -I."));
	if (outAsprintfStr) { free((void *) outAsprintfStr); outAsprintfStr = NULL; }
	assert(DoVasprintf(&outAsprintfStr, "-DWINDOWS %s", "-I.") != -1);
	assert(outAsprintfStr);
	assert(!strcmp(outAsprintfStr, "-DWINDOWS -I."));
	if (outAsprintfStr) { free((void *) outAsprintfStr); outAsprintfStr = NULL; }
}
#endif

int main(int argc, char *argv[]) {
	BuildConfig *b = NULL;
	char *exeFileName = NULL;
	char *exeDir = NULL;
	char *cwdBeforeChDir = NULL;
	char *cwd = NULL;
	const char *cmd = NULL;

	if (Build_SetConsoleCodePage("utf-8")) goto cleanUp;

#if defined(WINDOWS)
	// Test vasprintf and asprintf, only on Windows, since we'll
	// be using our own implementation.
	TestAsprintfAndFriends();
#endif

	// Test init/de-init.
	assert((b = Build_InitBuildConfig()));
	assert(!Build_DeinitBuildConfig(b));

	// Test.
	assert((b = Build_InitBuildConfig()));
	assert(Build_GetDryRun(b));

	// Don't print out invoked commands, unless we are debugging.
	assert(!Build_SetPrintCommandToStdout(b, false));
	assert((cmd = Build_GetCCCommand(b)));
	assert(!Build_CC(b, "-c %s", "Builder.c"));
	assert(!strcmp(Build_GetLastExecCommand(b), "gcc -c Builder.c"));

	// Test setting of C language standard.
	assert(!Build_SetCLanguageStandard(b, "c17"));
	assert(!Build_CC(b, "-c Builder.c"));
	assert(!strcmp(Build_GetLastExecCommand(b), "gcc -std=c17 -c Builder.c"));
	assert(!Build_CXX(b, "-o Test_Builder Test_Builder.cc Builder.cc"));
	assert(!strcmp(Build_GetLastExecCommand(b), "g++ -o Test_Builder Test_Builder.cc Builder.cc"));
	// Test setting of C++ language standard.
	assert(!Build_SetCXXLanguageStandard(b, "c++17"));
	assert(!Build_CXX(b, "-o Test_Builder Test_Builder.cc Builder.cc"));
	assert(!strcmp(Build_GetLastExecCommand(b), "g++ -std=c++17 -o Test_Builder Test_Builder.cc Builder.cc"));
	// Test AR command.
	assert(!Build_AR(b, "cru libbuild.a Build_Functions.o Build_Builder.o"));
	assert(!strcmp(Build_GetLastExecCommand(b), "ar cru libbuild.a Build_Functions.o Build_Builder.o"));
	// Test LD command.
	assert(!Build_LD(b, "-o Test test.o"));
	assert(!strcmp(Build_GetLastExecCommand(b), "ld -o Test test.o"));

	// Test executable file name.
	assert((exeFileName = Build_ExecutableFileName("Test_Build")));
	if (Build_IsWindows()) {
		assert(!strcmp(exeFileName, "Test_Build.exe"));
	} else if (Build_IsMacOS() || Build_IsLinux() || Build_IsUnix()) {
		assert(!strcmp(exeFileName, "Test_Build"));
	}

	// Test actual invocation, move, copy and remove.
	assert(!Build_SetDryRun(b, false));
	if (Build_IsWindows()) {
		if (Build_SetCCCommand(b, "gcc -DWINDOWS") || Build_SetCXXCommand(b, "g++ -DWINDOWS")) {
			goto cleanUp;
		}
	} else if (Build_IsMacOS()) {
		if (Build_SetCCCommand(b, "gcc -DMACOS") || Build_SetCXXCommand(b, "g++ -DMACOS")) {
			goto cleanUp;
		}
	} else if (Build_IsLinux()) {
		if (Build_SetCCCommand(b, "gcc -DLINUX") || Build_SetCXXCommand(b, "g++ -DLINUX")) {
			goto cleanUp;
		}
	} else if (Build_IsUnix()) {
		if (Build_SetCCCommand(b, "gcc -DUNIX") || Build_SetCXXCommand(b, "g++ -DUNIX")) {
			goto cleanUp;
		}
	} else {
		BStatusCode = B_UnknownOS;
		goto cleanUp;
	}

	assert(!Build_CXX(b, "-o Build_Functions__test.o -c Build_Functions.cc"));
	assert(Build_FileExists("Build_Functions__test.o"));
	assert(!Build_Copy(b, "Build_Functions__test.o", "Build_Functions__test2.o"));
	assert(!Build_Move(b, "Build_Functions__test.o", "Build_Functions__test1.o"));
	assert(Build_FileExists("Build_Functions__test1.o"));
	assert(Build_FileExists("Build_Functions__test2.o"));
	assert(!Build_FileExists("Build_Functions__test.o"));
	assert(!Build_Remove(b, "Build_Functions__test1.o"));
	assert(!Build_FileExists("Build_Functions__test1.o"));
	assert(!Build_Remove(b, "Build_Functions__test2.o"));
	assert(!Build_FileExists("Build_Functions__test2.o"));
	assert(!Build_Exec(b, "echo \"%s\"", "Testing..."));
	assert(!strcmp(Build_GetLastExecCommand(b), "echo \"Testing...\""));

cleanUp:
	if (cwdBeforeChDir) free((void *) cwdBeforeChDir);
	if (cwd) free((void *) cwd);
	assert(!Build_DeinitBuildConfig(b));
	if (exeFileName) free((void *) exeFileName);
	if (exeDir) free((void *) exeDir);
	if (BStatusCode) {
		printf("Error: %s\n", Build_StatusCodeMessage(BStatusCode));
		return 1;
	} else {
		printf("OK了: Test_Build_C\n");
		return 0;
	}
}
