Small note about file extensions here:
Currently the local makefile it include all '*.asm' files as part of the general compiled binary.
Since crti and crto have special meaning, I've given them the .S extension. 
Still assembly, just to avoid the makefile glob.