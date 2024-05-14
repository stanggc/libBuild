#pragma once

// `thread_local` keyword, for thread-local storage.
#define thread_local __thread

#if defined(__WIN32) && !defined(WINDOWS)
#define WINDOWS
#endif

#if defined(WINDOWS)
#include <windows.h>
#include <stdarg.h>

#if defined(__cplusplus)
extern "C" {
#endif
// Windows doesn't have asprintf and friends.
int asprintf(char **out, const char *fmt, ...);
int vasprintf(char **out, const char *fmt, va_list);
#if defined(__cplusplus)
}
#endif

#endif

#if defined(__cplusplus)
#include <string>
#include <cstdarg>
#else
#include <stdbool.h>
#include <stdarg.h>
#endif

#if !defined(__cplusplus)
typedef enum BStatusCode_ BStatusCode_;
typedef struct BuildConfig BuildConfig;
#endif


#if defined(__cplusplus)
extern "C" {
#endif

enum BStatusCode_ {
	B_OK = 0,
	B_Mem,
	B_Unknown,
	B_UnknownOS,
	B_UnknownConsoleCodePage,
	B_ObjectRequired,
	B_InvokeFailed,
	B_ShellInvokeFailed,
	B_StatFailed,
	B_DirNameFailed,
	B_ChDirFailed,
	B_CurrentWorkingDirFailed,
	B_MissingExecutableFilePath,
};
extern thread_local enum BStatusCode_ BStatusCode;

struct BuildConfig;

const char * Build_StatusCodeMessage(enum BStatusCode_ code);

bool Build_IsWindows();
bool Build_IsMacOS();
bool Build_IsLinux();
bool Build_IsUnix();

int Build_SetConsoleCodePage(const char *cpName);

// Caller owns the memory pointed by `out`, and will be responsible for freeing it.
char * Build_DirName(const char *path);

int Build_ChDir(const char *dirPath);
int Build_ChDirToProgramDir(int argc, char *argv[]);

// Caller owns the memory pointed by `out`, and will be responsible for freeing it.
char * Build_GetCurrentWorkingDir();

BuildConfig * Build_InitBuildConfig();
int Build_DeinitBuildConfig(BuildConfig *cfg);

int Build_SetDryRun(BuildConfig *cfg, bool dryRun);
bool Build_GetDryRun(BuildConfig *cfg);
int Build_SetPrintCommandToStdout(BuildConfig *cfg, bool printCommandToStdout);
bool Build_GetPrintCommandToStdout(BuildConfig *cfg);
const char * Build_GetLastExecCommand(BuildConfig *cfg);
int Build_SetCCCommand(BuildConfig *cfg, const char *cmd);
const char * Build_GetCCCommand(BuildConfig *cfg);
int Build_SetCLanguageStandard(BuildConfig *cfg, const char *std);
const char * Build_GetCLanguageStandard(BuildConfig *cfg);
int Build_SetCXXCommand(BuildConfig *cfg, const char *cmd);
const char * Build_GetCXXCommand(BuildConfig *cfg);
int Build_SetCXXLanguageStandard(BuildConfig *cfg, const char *std);
const char * Build_GetCXXLanguageStandard(BuildConfig *cfg);
int Build_SetARCommand(BuildConfig *cfg, const char *cmd);
const char * Build_GetARCommand(BuildConfig *cfg);
int Build_SetLDCommand(BuildConfig *cfg, const char *cmd);
const char * Build_GetLDCommand(BuildConfig *cfg);
int Build_SetMoveCommand(BuildConfig *cfg, const char *cmd);
const char * Build_GetMoveCommand(BuildConfig *cfg);
int Build_SetCopyCommand(BuildConfig *cfg, const char *cmd);
const char * Build_GetCopyCommand(BuildConfig *cfg);
int Build_SetRemoveCommand(BuildConfig *cfg, const char *cmd);
const char * Build_GetRemoveCommand(BuildConfig *cfg);

int Build_CC(BuildConfig *cfg, const char *fmt, ...);
int Build_CXX(BuildConfig *cfg, const char *fmt, ...);
int Build_AR(BuildConfig *cfg, const char *fmt, ...);
int Build_LD(BuildConfig *cfg, const char *fmt, ...);
int Build_Exec(BuildConfig *cfg, const char *fmt, ...);
int Build_Move(BuildConfig *cfg, const char *src, const char *dest);
int Build_Copy(BuildConfig *cfg, const char *src, const char *dest);
int Build_Remove(BuildConfig *cfg, const char *path);

// Caller owns the memory pointed by `out`, and will be responsible for freeing it.
char * Build_ExecutableFileName(const char *exeName);

bool Build_FileExists(const char *path);
#if defined(__cplusplus)
}
#endif


#if defined(__cplusplus)
namespace Build {
	struct Builder {
		Builder(bool dryRun = true, bool printCommandToStdout = true);

		static enum BStatusCode_ ExceptionToStatusCode(std::exception &e);

		static bool IsWindows();
		static bool IsMacOS();
		static bool IsLinux();
		static bool IsUnix();
		static void SetConsoleCodePage(std::string cpName);
		static std::string DirName(std::string path);
		static void ChDir(std::string dirPath);
		static void ChDirToProgramDir(int argc, char *argv[]);
		static std::string GetCurrentWorkingDir();

		void ExecCommandFV(std::string cmd, std::string fmt, va_list args);
		void CC(std::string fmt, ...);
		void CCFV(std::string fmt, va_list args);
		void CXX(std::string fmt, ...);
		void CXXFV(std::string fmt, va_list args);
		void AR(std::string fmt, ...);
		void ARFV(std::string fmt, va_list args);
		void LD(std::string fmt, ...);
		void LDFV(std::string fmt, va_list args);
		void ExecRaw(std::string cmdExpr);
		void Exec(std::string fmt, ...);
		void ExecFV(std::string fmt, va_list args);
		void Move(std::string src, std::string dest);
		void Copy(std::string src, std::string dest);
		void Remove(std::string path);
		static std::string ExecutableFileName(std::string exeName);
		static bool FileExists(std::string path);

		bool DryRun;
		bool PrintCommandToStdout;
		std::string LastExecCommand;
		std::string CCCommand;
		std::string CLanguageStandard;
		std::string CXXCommand;
		std::string CXXLanguageStandard;
		std::string ARCommand;
		std::string LDCommand;
		std::string MoveCommand;
		std::string CopyCommand;
		std::string RemoveCommand;
	};
}
#endif

