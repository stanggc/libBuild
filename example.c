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
