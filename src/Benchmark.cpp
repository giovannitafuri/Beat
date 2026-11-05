#include "Benchmark.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static int compare_floats(const void *a, const void *b) {
    float fa = *(const float *)a;
    float fb = *(const float *)b;
    return (fa > fb) - (fa < fb);
}

void benchmark_init(BenchmarkContext *ctx) {
    if (!ctx) return;
    ctx->state = BenchmarkState::IDLE;
    ctx->target_samples = 0;
    ctx->current_sample = 0;
    memset(&ctx->result, 0, sizeof(BenchmarkResult));
}

void benchmark_start(BenchmarkContext *ctx, size_t num_samples) {
    if (!ctx) return;
    if (num_samples > BENCHMARK_MAX_SAMPLES) {
        num_samples = BENCHMARK_MAX_SAMPLES;
    }
    ctx->target_samples = num_samples;
    ctx->current_sample = 0;
    memset(&ctx->result, 0, sizeof(BenchmarkResult));
    ctx->state = BenchmarkState::RECORDING;
}

void benchmark_record_frame(BenchmarkContext *ctx, float physics_ms, float render_ms, float total_ms) {
    if (!ctx || ctx->state != BenchmarkState::RECORDING) return;

    ctx->samples[ctx->current_sample].physics_ms = physics_ms;
    ctx->samples[ctx->current_sample].render_ms = render_ms;
    ctx->samples[ctx->current_sample].total_ms = total_ms;
    ctx->current_sample++;

    if (ctx->current_sample >= ctx->target_samples) {
        ctx->state = BenchmarkState::COMPLETED;
        benchmark_compute_results(ctx);
    }
}

void benchmark_compute_results(BenchmarkContext *ctx) {
    if (!ctx || ctx->current_sample == 0) return;

    size_t count = ctx->current_sample;
    ctx->result.total_samples = (uint32_t)count;

    float sum_total = 0.0f;
    uint32_t missed_144 = 0;
    uint32_t missed_60 = 0;

    for (size_t i = 0; i < count; i++) {
        float t = ctx->samples[i].total_ms;
        sum_total += t;
        if (t > 6.9444f) missed_144++;
        if (t > 16.6667f) missed_60++;
    }

    ctx->result.avg_ms = sum_total / (float)count;
    ctx->result.missed_144hz = missed_144;
    ctx->result.missed_60hz = missed_60;

    static float sorted_totals[BENCHMARK_MAX_SAMPLES];
    for (size_t i = 0; i < count; i++) {
        sorted_totals[i] = ctx->samples[i].total_ms;
    }

    qsort(sorted_totals, count, sizeof(float), compare_floats);

    ctx->result.min_ms = sorted_totals[0];
    ctx->result.max_ms = sorted_totals[count - 1];
    ctx->result.p50_ms = sorted_totals[count / 2];

    size_t p99_idx = (size_t)((float)count * 0.99f);
    if (p99_idx >= count) p99_idx = count - 1;
    ctx->result.p99_ms = sorted_totals[p99_idx];
}

bool benchmark_export_csv(const BenchmarkContext *ctx, const char *filepath) {
    if (!ctx || !filepath || ctx->current_sample == 0) return false;

    FILE *f = fopen(filepath, "w");
    if (!f) return false;

    fprintf(f, "sample_index,physics_ms,render_ms,total_ms\n");
    for (size_t i = 0; i < ctx->current_sample; i++) {
        fprintf(f, "%zu,%.4f,%.4f,%.4f\n",
                i,
                (double)ctx->samples[i].physics_ms,
                (double)ctx->samples[i].render_ms,
                (double)ctx->samples[i].total_ms);
    }

    fclose(f);
    return true;
}
