### GOTCHAs

When building with clang++ from the external cling/clang build using the provided .vscode/cmake-kits.json, you may
encounter a problem where CMake fails to configure with an error message "/usr/bin/ld: cannot find -lstdc++".

Solution: sudo apt install libstdc++-12-dev
Reference: https://stackoverflow.com/questions/74543715/usr-bin-ld-cannot-find-lstdc-no-such-file-or-directory-on-running-flutte