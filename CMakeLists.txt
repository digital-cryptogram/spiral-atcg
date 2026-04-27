cmake_minimum_required(VERSION 3.16)
project(spiral_atcg_v2_2_3_scalarized_round C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(spiral_atcg STATIC src/spiral_atcg.c)
target_include_directories(spiral_atcg PUBLIC include)
if(MSVC)
    target_compile_options(spiral_atcg PRIVATE /O2 /W4)
else()
    target_compile_options(spiral_atcg PRIVATE -O0 -Wall -Wextra -Wpedantic)
endif()

add_executable(test_roundtrip tests/test_roundtrip.c)
target_link_libraries(test_roundtrip PRIVATE spiral_atcg)

add_executable(test_pair_symmetry tests/test_pair_symmetry.c)
target_link_libraries(test_pair_symmetry PRIVATE spiral_atcg)

add_executable(test_material_trace tests/test_material_trace.c)
target_link_libraries(test_material_trace PRIVATE spiral_atcg)

add_executable(test_batch_api tests/test_batch_api.c)
target_link_libraries(test_batch_api PRIVATE spiral_atcg)

add_executable(benchmark bench/benchmark.c)
target_link_libraries(benchmark PRIVATE spiral_atcg)

add_executable(test_avalanche tests/test_avalanche.c)
target_link_libraries(test_avalanche PRIVATE spiral_atcg)

add_executable(test_thorough_avalanche tests/test_thorough_avalanche.c)
target_link_libraries(test_thorough_avalanche PRIVATE spiral_atcg)

add_executable(kat_generator tests/kat_generator.c)
target_link_libraries(kat_generator PRIVATE spiral_atcg)

add_executable(qres_v2_runner tests/qres_v2_runner.c)
target_link_libraries(qres_v2_runner PRIVATE spiral_atcg m)

add_executable(test_sec_battery tests/test_sec_battery.c)
target_link_libraries(test_sec_battery PRIVATE spiral_atcg m)

add_executable(test_q_battery tests/test_q_battery.c)
target_link_libraries(test_q_battery PRIVATE spiral_atcg m)
add_executable(test_adv_battery tests/test_adv_battery.c)
target_link_libraries(test_adv_battery PRIVATE spiral_atcg m)

add_executable(test_adv_c1_distinct tests/test_adv_c1_distinct.c)
target_link_libraries(test_adv_c1_distinct PRIVATE spiral_atcg m)

add_executable(test_adv_c3_distinct tests/test_adv_c3_distinct.c)
target_link_libraries(test_adv_c3_distinct PRIVATE spiral_atcg m)
