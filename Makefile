CC ?= gcc
CFLAGS ?= -O0 -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -Iinclude
AR ?= ar
ARFLAGS ?= rcs

BUILD := build
LIB := $(BUILD)/libspiral_atcg.a
SRC := src/spiral_atcg.c
OBJ := $(BUILD)/spiral_atcg.o

.PHONY: all full clean test bench trace kat thorough qres

# Build the day-to-day targets only. Heavy probes are opt-in via `full`,
# `thorough`, or `qres` so normal edit/test cycles stay fast.
all: $(LIB) $(BUILD)/test_roundtrip $(BUILD)/test_pair_symmetry $(BUILD)/test_avalanche $(BUILD)/test_batch_api $(BUILD)/kat_generator $(BUILD)/benchmark

full: all $(BUILD)/test_thorough_avalanche $(BUILD)/test_material_trace $(BUILD)/qres_v2_runner

$(BUILD):
	mkdir -p $(BUILD)

$(OBJ): $(SRC) include/spiral_atcg.h | $(BUILD)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $(LIB) $(OBJ)

$(BUILD)/test_roundtrip: tests/test_roundtrip.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@

$(BUILD)/test_pair_symmetry: tests/test_pair_symmetry.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@

$(BUILD)/test_avalanche: tests/test_avalanche.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@

$(BUILD)/test_batch_api: tests/test_batch_api.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@

$(BUILD)/test_thorough_avalanche: tests/test_thorough_avalanche.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@

$(BUILD)/test_material_trace: tests/test_material_trace.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@

$(BUILD)/kat_generator: tests/kat_generator.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@

$(BUILD)/benchmark: bench/benchmark.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@

$(BUILD)/qres_v2_runner: tests/qres_v2_runner.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -lm -o $@

test: $(BUILD)/test_roundtrip $(BUILD)/test_pair_symmetry $(BUILD)/test_avalanche $(BUILD)/test_batch_api $(BUILD)/kat_generator
	./$(BUILD)/test_roundtrip
	./$(BUILD)/test_pair_symmetry
	./$(BUILD)/test_avalanche
	./$(BUILD)/test_batch_api
	./$(BUILD)/kat_generator >/tmp/spiral_atcg_kat_check.txt
	tail -2 /tmp/spiral_atcg_kat_check.txt

trace: $(BUILD)/test_material_trace
	./$(BUILD)/test_material_trace

kat: $(BUILD)/kat_generator
	./$(BUILD)/kat_generator

thorough: $(BUILD)/test_thorough_avalanche
	./$(BUILD)/test_thorough_avalanche

qres: $(BUILD)/qres_v2_runner
	./$(BUILD)/qres_v2_runner > docs/spiral_atcg_qres_v2_2_strength_comparison.csv
	wc -l docs/spiral_atcg_qres_v2_2_strength_comparison.csv

bench: $(BUILD)/benchmark
	./$(BUILD)/benchmark

clean:
	rm -rf $(BUILD)

FAST_CFLAGS ?= -O1 -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -Iinclude
fast-bench:
	$(CC) $(FAST_CFLAGS) -c $(SRC) -o $(BUILD)/spiral_atcg_fast.o
	$(AR) $(ARFLAGS) $(BUILD)/libspiral_atcg_fast.a $(BUILD)/spiral_atcg_fast.o
	$(CC) $(FAST_CFLAGS) bench/benchmark.c $(BUILD)/libspiral_atcg_fast.a -o $(BUILD)/benchmark_fast
	./$(BUILD)/benchmark_fast

# v2.2.4 security battery add-on
$(BUILD)/test_sec_battery: tests/test_sec_battery.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -lm -o $@

$(BUILD)/test_q_battery: tests/test_q_battery.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -lm -o $@

sec-battery: $(BUILD)/test_sec_battery
	./$(BUILD)/test_sec_battery > docs/spiral_atcg_v224_sec_battery.csv
	wc -l docs/spiral_atcg_v224_sec_battery.csv

q-battery: $(BUILD)/test_q_battery
	./$(BUILD)/test_q_battery > docs/spiral_atcg_v224_q_battery.csv
	wc -l docs/spiral_atcg_v224_q_battery.csv

security-battery: sec-battery q-battery
	@echo "Security and quantum batteries complete."

# v2.2.4 advanced cryptanalytic battery add-on
$(BUILD)/test_adv_battery: tests/test_adv_battery.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -lm -o $@

$(BUILD)/test_adv_c1_distinct: tests/test_adv_c1_distinct.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -lm -o $@

$(BUILD)/test_adv_c3_distinct: tests/test_adv_c3_distinct.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -lm -o $@

adv-battery: $(BUILD)/test_adv_battery
	./$(BUILD)/test_adv_battery > docs/spiral_atcg_v224_adv_battery.csv
	wc -l docs/spiral_atcg_v224_adv_battery.csv

adv-c1-distinct: $(BUILD)/test_adv_c1_distinct
	./$(BUILD)/test_adv_c1_distinct

adv-c3-distinct: $(BUILD)/test_adv_c3_distinct
	./$(BUILD)/test_adv_c3_distinct

advanced-battery: adv-battery adv-c1-distinct adv-c3-distinct
	@echo "Advanced cryptanalytic battery complete."
