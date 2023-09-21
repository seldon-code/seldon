#!/bin/sh 
find ./include -regex '.*\.\(cpp\|hpp\|cc\|cxx\)' -exec clang-format -i {} \;
find ./src -regex '.*\.\(cpp\|hpp\|cc\|cxx\)' -exec clang-format -i {} \;
