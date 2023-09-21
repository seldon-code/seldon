#!/bin/sh 
find . -regex '.*\.\(cpp\|hpp\|cc\|cxx\)' -exec clang-format -i {} \;