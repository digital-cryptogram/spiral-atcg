@echo off
setlocal
if not exist build mkdir build
set CFLAGS=-O0 -std=c11 -Wall -Wextra -Wpedantic -Iinclude
gcc %CFLAGS% -c src\spiral_atcg.c -o build\spiral_atcg.o
ar rcs build\libspiral_atcg.a build\spiral_atcg.o
gcc %CFLAGS% tests\test_roundtrip.c build\libspiral_atcg.a -o build\test_roundtrip.exe
gcc %CFLAGS% tests\test_pair_symmetry.c build\libspiral_atcg.a -o build\test_pair_symmetry.exe
gcc %CFLAGS% tests\test_material_trace.c build\libspiral_atcg.a -o build\test_material_trace.exe
gcc %CFLAGS% bench\benchmark.c build\libspiral_atcg.a -o build\benchmark.exe
build\test_roundtrip.exe
build\test_pair_symmetry.exe
build\benchmark.exe
endlocal
