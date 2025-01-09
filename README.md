### Use mold
Download a recent mold release from [GitHub](https://github.com/rui314/mold/releases).

Ensure you have a ~/.local/bin, otherwise create it and relog so it gets added to your PATH.

```
cd ~
tar --strip-components=1 -xvf ~/Downloads/mold-2.36.0-x86_64-linux.tar.gz -C .local
```

Ensure you use a CMake setup like the one in .vscode/cmake-kits.json.

### GOTCHAs

When building with clang++ from the external cling/clang build using the provided .vscode/cmake-kits.json, you may
encounter a problem where CMake fails to configure with an error message "/usr/bin/ld: cannot find -lstdc++".

Solution: sudo apt install libstdc++-12-dev
Reference: https://stackoverflow.com/questions/74543715/usr-bin-ld-cannot-find-lstdc-no-such-file-or-directory-on-running-flutte