#include "Build.h"
#include "EnsureOSMacro.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

thread_local enum BStatusCode_ BStatusCode;

using std::string;
using Build::Builder;

struct BuildConfig {
	Build::Builder *Builder;
};

const char * Build_StatusCodeMessage(BStatusCode_ code) {
	switch (code) {
	case B_OK:
		return "OK";
	case B_Mem:
		return "unable to allocate memory";
	case B_UnknownOS:
		return "unknown OS";
	case B_UnknownConsoleCodePage:
		return "unknown console code page";
	case B_ObjectRequired:
		return "object required";
	case B_InvokeFailed:
		return "invoke failed";
	case B_ShellInvokeFailed:
		return "shell invoke failed";
	case B_StatFailed:
		return "stat failed";
	case B_DirNameFailed:
		return "dirname failed";
	case B_ChDirFailed:
		return "change directory failed";
	case B_CurrentWorkingDirFailed:
		return "get current working directory failed";
	case B_MissingExecutableFilePath:
		return "missing executable path";
	default:
		return "unknown status code";
	}
}

bool Build_IsWindows() {
	return Builder::IsWindows();
}

bool Build_IsMacOS() {
	return Builder::IsMacOS();
}

bool Build_IsLinux() {
	return Builder::IsLinux();
}

bool Build_IsUnix() {
	return Builder::IsUnix();
}

int Build_SetConsoleCodePage(const char *cpName) {
	try {
		Builder::SetConsoleCodePage(string(cpName));
		return 0;
	} catch (std::exception &e) {
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return -1;
	}
}

char * Build_DirName(const char *path) {
	string dirName;
	char *out = NULL;

	try {
		dirName = Builder::DirName(string(path));
		if (asprintf(&out, "%s", dirName.c_str()) == -1) {
			BStatusCode = B_Mem;
			return NULL;
		}
		return out;
	} catch (std::exception &e) {
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return NULL;
	}
}

int Build_ChDir(const char *dirPath) {
	try {
		Builder::ChDir(string(dirPath));
		return 0;
	} catch (std::exception &e) {
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return -1;
	}
}

int Build_ChDirToProgramDir(int argc, char *argv[]) {
	try {
		Builder::ChDirToProgramDir(argc, argv);
		return 0;
	} catch (std::exception &e) {
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return -1;
	}
}

char * Build_GetCurrentWorkingDir() {
	char *out = NULL;
	string cwd;

	try {
		cwd = Builder::GetCurrentWorkingDir();
		if (asprintf(&out, "%s", cwd.c_str()) == -1) {
			BStatusCode = B_Mem;
			return NULL;
		}
		return out;
	} catch (std::exception &e) {
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return NULL;
	}
}

BuildConfig * Build_InitBuildConfig() {
	BuildConfig *cfg = NULL;

	cfg = (BuildConfig *) calloc(1, sizeof(struct BuildConfig));
	if (!cfg) {
		BStatusCode = B_Mem;
		return NULL;
	}

	cfg->Builder = new Builder;
	if (!cfg->Builder) {
		BStatusCode = B_Mem;
		goto failCleanUp;
	}
	return cfg;

failCleanUp:
	free((void *) cfg);
	return NULL;
}

static bool EnsureObjectProvidedOrFlagError(BuildConfig *cfg) {
	if (!cfg) {
		BStatusCode = B_ObjectRequired;
		return false;
	}
	return true;
}

int Build_DeinitBuildConfig(BuildConfig *cfg) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	if (cfg->Builder) delete cfg->Builder;
	free((void *) cfg);
	return 0;
}

int Build_SetDryRun(BuildConfig *cfg, bool dryRun) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	cfg->Builder->DryRun = dryRun;
	return 0;
}

bool Build_GetDryRun(BuildConfig *cfg) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return false;
	}

	return cfg->Builder->DryRun;
}

int Build_SetPrintCommandToStdout(BuildConfig *cfg, bool printCommandToStdout) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	cfg->Builder->PrintCommandToStdout = printCommandToStdout;
	return 0;
}

bool Build_GetPrintCommandToStdout(BuildConfig *cfg) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return false;
	}

	return cfg->Builder->PrintCommandToStdout;
}

const char * Build_GetLastExecCommand(BuildConfig *cfg) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return NULL;
	}

	return cfg->Builder->LastExecCommand.c_str();
}

int Build_SetCCCommand(BuildConfig *cfg, const char *cmd) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	cfg->Builder->CCCommand = cmd;
	return 0;
}

const char * Build_GetCCCommand(BuildConfig *cfg) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return NULL;
	}

	return cfg->Builder->CCCommand.c_str();
}

int Build_SetCLanguageStandard(BuildConfig *cfg, const char *std) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	cfg->Builder->CLanguageStandard = std;
	return 0;
}

const char * Build_GetCLanguageStandard(BuildConfig *cfg) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return NULL;
	}

	return cfg->Builder->CLanguageStandard.c_str();
}

int Build_SetCXXCommand(BuildConfig *cfg, const char *cmd) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	cfg->Builder->CXXCommand = cmd;
	return 0;
}

const char * Build_GetCXXCommand(BuildConfig *cfg) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return NULL;
	}

	return cfg->Builder->CXXCommand.c_str();
}

int Build_SetCXXLanguageStandard(BuildConfig *cfg, const char *std) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	cfg->Builder->CXXLanguageStandard = std;
	return 0;
}

const char * Build_GetCXXLanguageStandard(BuildConfig *cfg) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return NULL;
	}

	return cfg->Builder->CXXLanguageStandard.c_str();
}

int Build_SetARCommand(BuildConfig *cfg, const char *cmd) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	cfg->Builder->ARCommand = cmd;
	return 0;
}

const char * Build_GetARCommand(BuildConfig *cfg) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return NULL;
	}

	return cfg->Builder->ARCommand.c_str();
}

int Build_SetLDCommand(BuildConfig *cfg, const char *cmd) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	cfg->Builder->LDCommand = cmd;
	return 0;
}

const char * Build_GetLDCommand(BuildConfig *cfg) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return NULL;
	}

	return cfg->Builder->LDCommand.c_str();
}

int Build_SetMoveCommand(BuildConfig *cfg, const char *cmd) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	cfg->Builder->MoveCommand = cmd;
	return 0;
}

const char * Build_GetMoveCommand(BuildConfig *cfg) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return NULL;
	}

	return cfg->Builder->MoveCommand.c_str();
}

int Build_SetCopyCommand(BuildConfig *cfg, const char *cmd) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	cfg->Builder->CopyCommand = cmd;
	return 0;
}

const char * Build_GetCopyCommand(BuildConfig *cfg) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return NULL;
	}

	return cfg->Builder->CopyCommand.c_str();
}

int Build_SetRemoveCommand(BuildConfig *cfg, const char *cmd) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	cfg->Builder->RemoveCommand = cmd;
	return 0;
}

const char * Build_GetRemoveCommand(BuildConfig *cfg) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return NULL;
	}

	return cfg->Builder->RemoveCommand.c_str();
}

int Build_CC(BuildConfig *cfg, const char *fmt, ...) {
	va_list args;

	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	va_start(args, fmt);
	try {
		cfg->Builder->CCFV(fmt, args);
		va_end(args);
		return 0;
	} catch (std::exception &e) {
		va_end(args);
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return -1;
	}
}

int Build_CXX(BuildConfig *cfg, const char *fmt, ...) {
	va_list args;

	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	va_start(args, fmt);
	try {
		cfg->Builder->CXXFV(fmt, args);
		va_end(args);
		return 0;
	} catch (std::exception &e) {
		va_end(args);
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return -1;
	}
}

int Build_AR(BuildConfig *cfg, const char *fmt, ...) {
	va_list args;

	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	va_start(args, fmt);
	try {
		cfg->Builder->ARFV(fmt, args);
		va_end(args);
		return 0;
	} catch (std::exception &e) {
		va_end(args);
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return -1;
	}
}

int Build_LD(BuildConfig *cfg, const char *fmt, ...) {
	va_list args;

	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	va_start(args, fmt);
	try {
		cfg->Builder->LDFV(fmt, args);
		va_end(args);
		return 0;
	} catch (std::exception &e) {
		va_end(args);
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return -1;
	}
}

int Build_Exec(BuildConfig *cfg, const char *fmt, ...) {
	va_list args;

	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	va_start(args, fmt);
	try {
		cfg->Builder->ExecFV(fmt, args);
		va_end(args);
		return 0;
	} catch (std::exception &e) {
		va_end(args);
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return -1;
	}
}

int Build_Move(BuildConfig *cfg, const char *src, const char *dest) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	try {
		cfg->Builder->Move(string(src), string(dest));
		return 0;
	} catch (std::exception &e) {
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return -1;
	}
}

int Build_Copy(BuildConfig *cfg, const char *src, const char *dest) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	try {
		cfg->Builder->Copy(string(src), string(dest));
		return 0;
	} catch (std::exception &e) {
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return -1;
	}
}

int Build_Remove(BuildConfig *cfg, const char *path) {
	if (!EnsureObjectProvidedOrFlagError(cfg)) {
		return -1;
	}

	try {
		cfg->Builder->Remove(string(path));
		return 0;
	} catch (std::exception &e) {
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return -1;
	}
}

char * Build_ExecutableFileName(const char *exeName) {
	char *outName = NULL;

	if (asprintf(&outName, "%s", Builder::ExecutableFileName(string(exeName)).c_str()) == -1) {
		BStatusCode = B_Mem;
		return NULL;
	}

	return outName;
}

bool Build_FileExists(const char *path) {
	try {
		return Builder::FileExists(string(path));
	} catch (std::exception &e) {
		BStatusCode = Builder::ExceptionToStatusCode(e);
		return false;
	}
}
