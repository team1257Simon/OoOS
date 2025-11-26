## OoOS
OoOS, or the Object-oriented Operating System (working name in development) is, as the name suggests, an OS developed using OoP concepts, among other things.
It exists because I am insane and, as an insane person, wanted to see if it was possible to do silly things like using lambdas to implement ISRs and using exceptions to handle internal error checking and using abstraction to implement a VFS (okay, the last one is pretty normal).
Feel free to contribute to my project, though be advised that it's still in relatively early stages and a lot of it is either untested or only partially-tested and prone to breaking if you breathe on it too hard.
Well, okay, it's a lot less fragile than it used to be, at least.

# Building
You can install the targeted compiler from the releases (under ooos_sysroot); the archive contains the sysroot directory which contains all the compiler binaries, libraries, and headers so you don't have to wrangle autotools. Remember to add the ooos_sysroot/usr/bin directory to your PATH and set the $SYSROOT environment variable to `/path/to/your/ooos_sysroot` (replace the placeholders obviously).
Alternatively, you can try to replicate what I did, but autotools might resist you the whole way.
The script `build-cross.sh` will setup the bare development environment. `./build-cross.sh -p ~/your/prefix/here` will let you decide where the script installs the cross-compiler.
To build the OS, you will need to install the libc and targeted toolchain as well; I created a script(currently untested) to build that toolchain, but the procedure goes like this:
 1. You'll need multiple different versions of the autotools programs because they're finnicky and will not work correctly if you so much as think about using a different minor version than the one the developer used originally. Specifically, it's autoconf2.65/automake1.11 for newlib and autoconf2.69/automake1.15 for binutils and GCC.
 2. You'll also need to build newlib the first time using the bare-platform compiler, but with the OS-specific binutils build. This will mean temporarily linking the bare-platform compiler a-la `ln /your/bin/dir/x86_64-elf-gcc /your/bin/dir/x86_64-ooos-gcc` and deleting those links after newlib is installed.
 3. The patches in the `lib` directory apply only to those exact releaases (see the README in that directory). You'll also need to `cp -lRf lib/newlib ../newlib`, `cp -lRf lib/binutils-gdb/* ../binutils-gdb`, and `cp -rf lib/gcc ../gcc` (no that isn't a typo, the binutils one is different).
 4. After applying the patches and copying the files, run `automake` in the `ld` subdirectory of the binutils source tree, `autoreconf` in the `newlib/libc/sys/ooos` directory and `autoconf` in the `newlib/libc/sys` directory of the newlib source tree, and `autoconf` in the `libstdc++-v3` directory of the gcc source tree.
 5. Create a separate directory for each of the builds (i.e. newlib, binutils, and gcc). Also, set the environment variable SYSROOT to equal /usr/local/ooos_sysroot (I'll work on a way to let that be configurable).
 6. In the binutils build directory, run `../binutils-gdb/configure --target=x86_64-ooos --with-sysroot=$SYSROOT --enable-shared --disable-werror`; you can also add `--prefix=~/your/prefix/here` to change where it will be installed, but make sure you add that to your PATH.
 7. Run make and install binutils.
 8. In the newlib directory, run `../newlib/configure --target=x86_64-ooos --prefix=$SYSROOT/usr`. This will tell it to install the library into the sysroot directory so that GCC can find the headers later. For this first pass, don't use --enable-shared; the bare-elf cross compiler won't support it.
 9. Run make and install newlib, then delete the linked cross-compilers.
 10. In the gcc directory, run `../gcc/configure --target=x86_64-ooos --enable-languages=c,c++ --enable-shared --with-sysroot=$SYSROOT --with-newlib`. Then, run `make all-gcc all-target-ligbcc` (not just `make`). Replace `all` with `install` to install a given target.
 11. After installing those, you can also run `make all-target-libstdc++-v3` to build the c++ library.
 12. Maybe go get a drink of water or something...the build process for GCC can take nearly an hour, even in this partial configuration.
 13. Now that the cross-compiler is built, go back into the `newlib/newlib` directory and run `autoreconf` to configure the whole library to build as a shared .so using libtool. If you get an error saying `autoheader failed with status: 1` it's safe to ignore it.
 14. Delete the entire contents of the newlib build directory, then configure again with `../newlib/configure --target=x86_64-ooos --prefix=$SYSROOT/usr --with-sysroot=$SYSROOT --enable-shared --enable-host-shared` to turn on the shared object build. As of right now, the library this builds isn't used in the image, but I plan to change that soon(ish).
In addition, the script `create_image.sh` has the following dependencies:
- `fuse fuse2fs libfuse-dev fuseloop`
- The last of these is a pretty simple program you can get from https://github.com/jmattsson/fuseloop.git which allows the script to build the image without root privileges.
Anyway, I'm going to be making improvements so that reproducing this is easier. Thanks for your interest in my silly little pet project!
