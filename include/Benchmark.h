#pragma once

#include <cstdint>
#include <cstddef>

constexpr size_t BENCHMARK_MAX_SAMPLES = 10000;

struct FrameSample {
    float physics_ms;
    float render_ms;
    float total_ms;
};

struct BenchmarkResult {
    uint32_t total_samples;

    float min_ms;
    float max_ms;
    float avg_ms;
    float p50_ms;
    float p99_ms;

    uint32_t missed_144hz;
    uint32_t missed_60hz;
};

enum class BenchmarkState : uint8_t {
    IDLE = 0,
    RECORDING,
    COMPLETED
};

struct BenchmarkContext {
    BenchmarkState state;
    size_t target_samples;
    size_t current_sample;
    FrameSample samples[BENCHMARK_MAX_SAMPLES];
    BenchmarkResult result;
};

void benchmark_init(BenchmarkContext *ctx);
void benchmark_start(BenchmarkContext *ctx, size_t num_samples = BENCHMARK_MAX_SAMPLES);
void benchmark_record_frame(BenchmarkContext *ctx, float physics_ms, float render_ms, float total_ms);
void benchmark_compute_results(BenchmarkContext *ctx);
bool benchmark_export_csv(const BenchmarkContext *ctx, const char *filepath);
