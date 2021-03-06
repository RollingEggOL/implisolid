#!/bin/bash

# echo "@rem em++ -I /usr/local/inlcude/boost_1_61_0/ -s EXPORTED_FUNCTIONS="['_main']" -pedantic -std=c++14  -DNDEBUG -DBOOST_UBLAS_NDEBUG -DBOOST_DISABLE_ASSERTS  -O3     vect_test_2.cpp    -o vect_test_2.js"

BOOST_FOLDER=/usr/local/include/boost_1_61_0
OPTIM=2
if [ $OPTIM -eq 0 ]
then

    echo "** dev version **"

    ~/Deskop/emsdk_portable/emscripten/master/em++ -I $BOOST_FOLDER  \
        -s TOTAL_MEMORY=30100100 \
        -s EXPORTED_FUNCTIONS="[ '_main', '_build_geometry', '_get_v_size', '_get_f_size', '_get_f', '_get_v', '_finish_geometry', '_get_f_ptr', '_get_v_ptr',]" \
        -s NO_EXIT_RUNTIME=1 \
        -s DEMANGLE_SUPPORT=1 \
         -s ASSERTIONS=1 \
        -pedantic -std=c++14 \
        mcc2_MS.cpp -o mcc2_MS.cpp.js

fi


if [ $OPTIM -eq 1 ]
then
    echo "** optimized version **"

    ~/Desktop/emsdk_portable/emscripten/master/em++ \
            -I $BOOST_FOLDER   \
            -O3   \
           -DNDEBUG -DBOOST_UBLAS_NDEBUG -DBOOST_DISABLE_ASSERTS  \
            -s EXPORTED_FUNCTIONS="[ '_main', '_build_geometry', '_get_v_size', '_get_f_size', '_get_f', '_get_v', '_finish_geometry', '_get_f_ptr', '_get_v_ptr']" \
            -s NO_EXIT_RUNTIME=1          \
            --llvm-lto 1 \
            -Winline         \
            -s TOTAL_MEMORY=30100100    \
            -pedantic -std=c++14  \
        mcc2_MS.cpp  \
            -o  mcc2_MS.cpp.js
fi


if [ $OPTIM -eq 2 ]
then
    echo "** old **"
    ~/Desktop/emsdk_portable/emscripten/master/em++ \
            -I $BOOST_FOLDER   \
            -s TOTAL_MEMORY=33554432    \
            -s EXPORTED_FUNCTIONS="[ '_main', '_build_geometry', '_get_v_size', '_get_f_size', '_get_f', '_get_v', '_finish_geometry', '_get_f_ptr', '_get_v_ptr']" \
            -s NO_EXIT_RUNTIME=1          \
            -s DEMANGLE_SUPPORT=1 \
             -s ASSERTIONS=1 \
            -pedantic -std=c++14  \
        mcc2_MS.cpp  \
            -o  mcc2_MS.cpp.js
fi
# copy the path for em++
# echo "@rem em++ -I /usr/local/inlcude/boost_1_61_0/ -s EXPORTED_FUNCTIONS="['_main']" -pedantic -std=c++14  -DNDEBUG -DBOOST_UBLAS_NDEBUG -DBOOST_DISABLE_ASSERTS  -O3     vect_test_2.cpp    -o vect_test_2.js"
#   ~/emsdk_portable/emscripten/master/em++ -I /usr/local/include/boost_1_61_0/  \
#   -s TOTAL_MEMORY=30100100 -s EXPORTED_FUNCTIONS="['_produce_object_old2', '_main', '_build_geometry', '_get_v_size', '_get_f_size', '_get_f', '_get_v', '_finish_geometry', '_get_f_ptr', '_get_v_ptr']" -O3 -s NO_EXIT_RUNTIME=1 -s ASSERTIONS=1 -s DEMANGLE_SUPPORT=1 -pedantic -std=c++14 mcc2.cpp -o mcc2.cpp.js

# ::     -s EXPORTED_FUNCTIONS="['_main', '_make_object']" ^
# ::     -s EXPORTED_FUNCTIONS="['_main']" ^1
# :: @rem     -O3   ^
# :: @rem     -DNDEBUG -DBOOST_UBLAS_NDEBUG -DBOOST_DISABLE_ASSERTS  ^
#
#
# ::     -O3   ^
# ::     -DNDEBUG -DBOOST_UBLAS_NDEBUG -DBOOST_DISABLE_ASSERTS  ^
#
#
# ::        -DNDEBUG -DBOOST_UBLAS_NDEBUG -DBOOST_DISABLE_ASSERTS  ^
# ::        -O3                           ^
#

# em++ ^
#         -I C:\sohail\March\emscripten\boost_1_61_0\   ^
#         -s ASSERTIONS=1               ^
#         -s DEMANGLE_SUPPORT=1   ^
#         -s TOTAL_MEMORY=30100100    ^
#                                     ^
#         -s EXPORTED_FUNCTIONS="['_produce_object_old2', '_main', '_build_geometry', '_get_v_size', '_get_f_size', '_get_f', '_get_v', '_finish_geometry', '_get_f_ptr', '_get_v_ptr']" ^
#         -s NO_EXIT_RUNTIME=1          ^
#                                           ^
#         -pedantic -std=c++14  ^
#     mcc2.cpp  ^
#         -o  mcc2.cpp.js
