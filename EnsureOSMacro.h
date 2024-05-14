#pragma once

#if !defined(WINDOWS) && !defined(MACOS) && !defined(LINUX) && !defined(UNIX)
#error "Unknown OS. Define one of the following macros when compiling (using compiler's `-D` switch): WINDOWS, MACOS, LINUX, UNIX"
#endif
