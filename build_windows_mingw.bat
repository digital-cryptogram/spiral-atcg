@echo off
setlocal
if not exist build mkdir build
set CFLAGS=-O0 -std=c11 -Wall -Wextra -Wpedantic -Iinclude
gcc %CFLAGS% -c src\spiral_atcg.c -o build\spiral_atcg.o
if errorlevel 1 exit /b 1
ar rcs build\libspiral_atcg.a build\spiral_atcg.o
if errorlevel 1 exit /b 1
for %%t in (test_roundtrip test_pair_symmetry test_batch_api test_avalanche test_v225_regressions test_affine_rank kat_generator test_material_trace) do (
    gcc %CFLAGS% tests\%%t.c build\libspiral_atcg.a -o build\%%t.exe
    if errorlevel 1 exit /b 1
)
gcc %CFLAGS% tests\test_round_paths.c -o build\test_round_paths.exe
if errorlevel 1 exit /b 1
gcc %CFLAGS% bench\benchmark.c build\libspiral_atcg.a -o build\benchmark.exe
if errorlevel 1 exit /b 1
for %%t in (test_roundtrip test_pair_symmetry test_batch_api test_avalanche test_v225_regressions test_affine_rank test_round_paths kat_generator) do (
    build\%%t.exe
    if errorlevel 1 exit /b 1
)
build\benchmark.exe
if errorlevel 1 exit /b 1
endlocal
