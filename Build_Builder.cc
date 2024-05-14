#include <cstdarg>
#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Build.h"
#include "EnsureOSMacro.h"

#if defined(WINDOWS)
#include <windows.h>
#elif defined(MACOS) || defined(LINUX) || defined(UNIX)
#include <locale.h>
#endif

using std::cout;
using std::string;
using std::runtime_error;

#if defined(WINDOWS)
int vasprintf(char **out, const char *fmt, va_list args) {
	int expectedSize = 0;
	char *content = NULL;
	size_t bufSize = 0;
	va_list argsCopy;

	va_copy(argsCopy, args);
	expectedSize = vsnprintf(NULL, 0, fmt, argsCopy);
	va_end(argsCopy);

	bufSize = expectedSize + 1;

	content = (char *) calloc(bufSize, sizeof(char));
	if (!content) {
		if (out) *out = NULL;
		expectedSize = -1;
		goto cleanUp;
	}
	vsnprintf(content, bufSize, fmt, args);

	if (out) *out = content;

cleanUp:
	return expectedSize;
}

int asprintf(char **out, const char *fmt, ...) {
	va_list args, argsCopy;
	size_t expectedSize = 0;
	char *content = NULL;
	size_t bufSize = 0;

	va_start(args, fmt);

	va_copy(argsCopy, args);
	expectedSize = vsnprintf(NULL, 0, fmt, argsCopy);
	va_end(argsCopy);

	bufSize = expectedSize + 1;

	content = (char *) calloc(bufSize, sizeof(char));
	if (!content) {
		if (out) *out = NULL;
		expectedSize = -1;
		goto cleanUp;
	}

	expectedSize = vsnprintf(content, bufSize, fmt, args);
	if (out) *out = content;

cleanUp:
	va_end(args);
	return expectedSize;
}
#endif

Build::Builder::Builder(bool dryRun, bool printCommandToStdout) :
DryRun(dryRun),
PrintCommandToStdout(printCommandToStdout) {
	CCCommand = "gcc";
	CXXCommand = "g++";
	ARCommand = "ar";
	LDCommand = "ld";
	if (IsWindows()) {
		MoveCommand = "move";
		CopyCommand = "copy";
		RemoveCommand = "del";
	} else if (IsMacOS() || IsLinux() || IsUnix()) {
		MoveCommand = "mv";
		CopyCommand = "cp";
		RemoveCommand = "rm -f";
	} else {
		throw runtime_error("unknown OS");
	}
}

enum BStatusCode_ Build::Builder::ExceptionToStatusCode(std::exception &e) {
	string msg = string(e.what());

	if (msg.rfind("unknown OS") == 0) {
		return B_UnknownOS;
	} else if (msg.rfind(string("unknown console code page: ")) == 0) {
		return B_UnknownConsoleCodePage;
	} else if (msg.rfind("unable to get directory of ") == 0) {
		return B_DirNameFailed;
	} else if (msg.rfind("unable to change directory to ") == 0) {
		return B_ChDirFailed;
	} else if (msg.rfind("missing executable file path") == 0) {
		return B_MissingExecutableFilePath;
	} else if (msg.rfind("unable to get current working directory") == 0) {
		return B_CurrentWorkingDirFailed;
	} else if (msg.rfind("invocation error") == 0) {
		return B_InvokeFailed;
	} else if (msg.rfind("shell invocation error") == 0) {
		return B_ShellInvokeFailed;
	} else if (msg.rfind("unable to allocate memory") == 0) {
		return B_Mem;
	} else if (msg.rfind("unable to stat file: ") == 0) {
		return B_StatFailed;
	} else {
		return B_Unknown;
	}
}

bool Build::Builder::IsWindows() {
#if defined(WINDOWS)
	return true;
#else
	return false;
#endif
}

bool Build::Builder::IsMacOS() {
#if defined(MACOS)
	return true;
#else
	return false;
#endif
}

bool Build::Builder::IsLinux() {
#if defined(LINUX)
	return true;
#else
	return false;
#endif
}

bool Build::Builder::IsUnix() {
#if defined(UNIX)
	return true;
#else
	return false;
#endif
}

void Build::Builder::SetConsoleCodePage(string cpName) {
	if (cpName == "utf-8") {
#if defined(WINDOWS)
		SetConsoleOutputCP(65001);
#elif defined(MACOS) || defined(LINUX) || defined(UNIX)
		setlocale(LC_ALL, "C.UTF-8");
	#else
		throw runtime_error("unknown OS");
#endif
	} else {
		throw runtime_error(string("unknown console code page: ") + cpName);
	}
}

string Build::Builder::DirName(string path) {
	string pCopy = path;
	char *dirName = NULL;

	dirName = dirname((char *) pCopy.c_str());
	if (!dirName) throw runtime_error(string("unable to get directory of ") + path);

	return string(dirName);
}

void Build::Builder::ChDir(string dirPath) {
	if (chdir(dirPath.c_str())) {
		throw runtime_error(string("unable to change directory to ") + dirPath);
	}
}

void Build::Builder::ChDirToProgramDir(int argc, char *argv[]) {
	string exeDir;

	if (argc < 1) throw runtime_error("missing executable file path");

	exeDir = DirName(argv[0]);
	ChDir(exeDir);
}

string Build::Builder::GetCurrentWorkingDir() {
	string cwd;
	char *cCwd = NULL;

	cCwd = getcwd(NULL, 0);
	if (!cCwd) {
		throw runtime_error("unable to get current working directory");
	}

	cwd = cCwd;

cleanUp:
	if (cCwd) free((void *) cCwd);
	return cwd;
}

void Build::Builder::ExecRaw(string cmdExpr) {
	int ret = 0;

	LastExecCommand = cmdExpr;
	if (PrintCommandToStdout) {
		if (DryRun)
			cout << "[DRYRUN] " << cmdExpr << "\n";
		else
			cout << "[INVOKE] " << cmdExpr << "\n";
	}
	if (!DryRun) {
		ret = system(cmdExpr.c_str());
		switch (ret) {
		case -1:
			throw runtime_error("invocation error");
		case 127:
			throw runtime_error("shell invocation error");
		}
	}
}

void Build::Builder::ExecCommandFV(string cmd, string fmt, va_list args) {
	char *parameters = NULL;
	string params;
	string fullCmd;

	if (vasprintf(&parameters, fmt.c_str(), args) == -1) {
		throw runtime_error("unable to allocate memory");
	}
	params = parameters;
	if (parameters) { free((void *) parameters); parameters = NULL; }

	if (cmd == "") {
		fullCmd = params;
	} else {
		fullCmd = cmd + " " + params;
	}

	ExecRaw(fullCmd);
}

void Build::Builder::CC(string fmt, ...) {
	va_list args;

	try {
		va_start(args, fmt);
		CCFV(fmt, args);
		va_end(args);
	} catch (std::exception &e) {
		// Make sure to end var args.
		va_end(args);
		throw;
	}
}

void Build::Builder::CCFV(string fmt, va_list args) {
	string cmd = CCCommand;

	if (CLanguageStandard != string()) cmd = CCCommand + string(" -std=") + CLanguageStandard;

	ExecCommandFV(cmd, fmt, args);
}

void Build::Builder::CXX(string fmt, ...) {
	va_list args;

	try {
		va_start(args, fmt);
		CXXFV(fmt, args);
		va_end(args);
	} catch (std::exception &e) {
		// Make sure to end var args.
		va_end(args);
		throw;
	}
}

void Build::Builder::CXXFV(string fmt, va_list args) {
	string cmd = CXXCommand;

	if (CXXLanguageStandard != string()) cmd = CXXCommand + string(" -std=") + CXXLanguageStandard;

	ExecCommandFV(cmd, fmt, args);
}

void Build::Builder::AR(string fmt, ...) {
	va_list args;

	try {
		va_start(args, fmt);
		ARFV(fmt, args);
		va_end(args);
	} catch (std::exception &e) {
		// Make sure to end var args.
		va_end(args);
		throw;
	}
}

void Build::Builder::ARFV(string fmt, va_list args) {
	ExecCommandFV(ARCommand, fmt, args);
}

void Build::Builder::LD(string fmt, ...) {
	va_list args;

	try {
		va_start(args, fmt);
		LDFV(fmt, args);
		va_end(args);
	} catch (std::exception &e) {
		// Make sure to end var args.
		va_end(args);
		throw;
	}
}

void Build::Builder::LDFV(string fmt, va_list args) {
	ExecCommandFV(LDCommand, fmt, args);
}

void Build::Builder::Exec(string fmt, ...) {
	va_list args;

	try {
		va_start(args, fmt);
		ExecFV(fmt, args);
		va_end(args);
	} catch (std::exception &e) {
		// Make sure to end var args.
		va_end(args);
		throw;
	}
}

void Build::Builder::ExecFV(string fmt, va_list args) {
	ExecCommandFV(string(), fmt, args);
}

void Build::Builder::Move(string src, string dest) {
	string fullCmd =
		MoveCommand +
		string(" \"") + src + string("\" \"") + dest + string("\"");

	ExecRaw(fullCmd);
}

void Build::Builder::Copy(string src, string dest) {
	string fullCmd =
		CopyCommand +
		string(" \"") + src + string("\" \"") + dest + string("\"");

	ExecRaw(fullCmd);
}

void Build::Builder::Remove(string path) {
	string fullCmd =
		RemoveCommand +
		string(" \"") + path + string("\"");

	ExecRaw(fullCmd);
}

string Build::Builder::ExecutableFileName(string exeName) {
	string outName = exeName;

	if (IsWindows()) {
		outName += ".exe";
	}

	return outName;
}

bool Build::Builder::FileExists(string path) {
	struct stat sb = { 0 };

	if (stat(path.c_str(), &sb)) {
		if (errno == ENOENT) {
			return false;
		} else {
			throw runtime_error(string("unable to stat file: ") + path);
		}
	}

	return (sb.st_mode & S_IFMT) == S_IFREG;
}
