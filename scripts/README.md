# Scripts

This directory contains various utility scripts for building minos. Their various functions are listed below.
These default to running with bash, however they dont use any bash specific features, and should run under most shells.

### `setup-env.sh`
Downloads the various sources and pre-built tools required to create a minos build environment from scratch. 
The default install location is the same the one used by the default makefile setup, so it's just run and go.
However you can override these if you want, the script and makefiles have lots of inline docs.

### `validate-env.sh`
Just a shorthand for running `make validate`, mostly serves as a visual aide to remind me that I should do this
on new installs.
