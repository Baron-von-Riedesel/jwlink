# JWlink
JWlink is a linker that may run under DOS, Windows and Linux and is able to create binaries for these systems.

It understands OMF, COFF and ELF formats, COFF and ELF for both 32-bit and 64-bit.

Makefiles supplied are:

- *Makefile* ; supposed to create the Win32 version of JWlink, but may also create
  a DOS-extended variant with the help of HX. The Open Watcom toolchain is used.

- *OWDOS32.mak*; creates the DOS-extended version of JWlink. The Open Watcom toolchain is used.

- *GccUnix.mak*; will create the Linux version of JWlink. The GNU toolchain is used.

Besides JWlink, there's JWlib, a library manager. Similar to JWlink, 3 makefiles for Windows,
DOS and Linux are supplied.

JWlink documentation: [JWlink.html](https://baron-von-riedesel.github.io/jwlink/html/JWlink.html)
