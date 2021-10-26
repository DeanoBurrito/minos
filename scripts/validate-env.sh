#!/bin/bash
#validates whether root makefile can access all the build dependencies it needs or not.

echo "Validating minos toolchain install ..."
cd ..
make validate-toolchain
