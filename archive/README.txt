            
                    
                    //  ) ) //  ) ) 
                 __//__  __//__     
                  //      //        
                 //      //         
                //      //     


    A traditional Forth system for UNIX and embedded systems

This is `ff`, a Forth interpreter and programming environment for UNIX
based operating systems. `ff` is completely self-contained and needs
no external libraries.

The following files are included in the distribution:

    ff-<ARCH>-<OS>          Binaries for various architectures and
                            operating systems
    README                  This file
    ff.blk                  Block file holding ff sources + documentation

The delivered bootstrap binaries contain only the most basic words.
To create a full Forth system with additional tools, execute the
following steps:

    $ ./ff-<ARCH>-<OS>
    include make.ff
    make ff
    bye

Now `ff` holds all core words, some extensions and tools and a 
simple line editor.

On x86/ARM Linux and x86_64 OpenBSD systems, a screen editor
with mouse support is available and can be built using the following
commands:

    $ ./ff-<ARCH>-<OS>
    include make.fa
    make fa
    bye

    blockfile ff.blk

in make.* to point to where your blockfile is located, e.g.

    blockfile /usr/local/share/ff.blk

To add more space in the blockfile `ff.blk` for your own code, you can 
use the `dd(1)` tool to create empty blocks and concatenate them
to the existing blockfile:

    $ dd if=/dev/zero count=<BLOCKNUM> bs=4096 | tr '\0' ' ' >blocks
    $ mv ff.blk ff.blk.old
    $ cat ff.blk.old blocks >ff.blk

(where "<BLOCKNUM>" is the number of 4k blocks that you want to
add to the existing blockfile.)

Only a minimal set of bootstrap binaries for some architectures and
operating systems are provided: Linux 32-bit binaries should run
on 64-bit installations. NetBSD/FreeBSD users can use the Linux
binary, provided they have Linux-emulation support enabled.

You can build binaries for other supported platforms by "metacompiling"
the core Forth system, see the "ff" manual contained in the blockfile
for more information.

Note that only Linux i386/x86_64/Arm, OpenBSD x86_64 and Darwin
x86_64 are regularly tested and "ff" may have bugs on other platforms
in addition to bugs and shortcomings that are known.
