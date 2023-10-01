#!/bin/sh 
find ./include -regex '.*\.\(cpp\|hpp\|cc\|cxx\)' -exec clang-tidy {} \;
find ./src -regex '.*\.\(cpp\|hpp\|cc\|cxx\)' -exec clang-tidy {} \;
find ./test -regex '.*\.\(cpp\|hpp\|cc\|cxx\)' -exec clang-tidy {} \;