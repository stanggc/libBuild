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
