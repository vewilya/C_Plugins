#include "filter.h"

typedef struct
{
    float cutoff_frequency;
    bool bypass;
} FilterParameters;

static struct
{
    FilterParameters params;

    struct
    {
        double *prevSample;
        double normalised;
        double alpha;
        double _alpha;
    } internal;

    struct
    {
        double sample_rate;
        double nyquist;
        int num_channels;
    } config;
} state = {
    .params.cutoff_frequency = 100.0f,
    .params.bypass = false};

static void updateFilterCoefficients(void)
{
    state.internal.normalised = state.config.sample_rate / (M_PI * 2.0);

    state.internal.alpha = state.params.cutoff_frequency /
                           (state.params.cutoff_frequency + state.internal.normalised);
    state.internal._alpha = 1.0 - state.internal.alpha;
}

int filterInit(double sampleRate, int numChannels)
{
    state.config.sample_rate = sampleRate;
    state.config.num_channels = numChannels;
    state.config.nyquist = state.config.sample_rate * 0.5;

    state.internal.prevSample = calloc(numChannels, sizeof(double));
    updateFilterCoefficients();
    return 0;
}

void filterProcess(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer)
{
    const float *in = (const float *)inputBuffer;
    float *out = (float *)outputBuffer;

    /* One Pole Lowpass Filter */
    /* y[n] = alpha * x[n] + (1 - alpha) * y[n-1] */

    if (state.params.bypass)
    {
        // Simple copy of memory blocks for bypass
        memcpy(outputBuffer, inputBuffer, framesPerBuffer * state.config.num_channels * sizeof(float));
    }
    else
    {

        for (uint16_t frame = 0; frame < framesPerBuffer; frame++)
        {
            /* Advances in 2-steps */
            uint16_t base = frame * state.config.num_channels;

            for (uint16_t channel = 0; channel < state.config.num_channels; channel++)
            {
                out[base + channel] = state.internal.alpha * in[base + channel] + state.internal._alpha * (state.internal.prevSample[channel]);
                state.internal.prevSample[channel] = out[base + channel];
            }
        }
    }
}

void filterShutdown(void)
{
    free(state.internal.prevSample);
    state.internal.prevSample = NULL;
}

int setFilterParameters(const char *parameterName, float value)
{
    if (strcmp(parameterName, "frequency") == 0)
    {
        state.params.cutoff_frequency = value;

        if (state.params.cutoff_frequency > state.config.nyquist)
            state.params.cutoff_frequency = state.config.nyquist;
        if (state.params.cutoff_frequency < 20.0f)
            state.params.cutoff_frequency = 20.0f;

        updateFilterCoefficients();

        printf("Set cutoff to %.2f\n", state.params.cutoff_frequency);

        return 0;
    }
    else if (strcmp(parameterName, "bypass") == 0)
    {
        /* If value bigger than 0.5 -> bypass = true */
        state.params.bypass = (value > 0.5f);
        printf("Set bypass to %s\n", state.params.bypass ? "TRUE" : "FALSE");
        return 0;
    }

    printf("Unknown parameter: %s\n", parameterName);
    return -1;
}

Plugin_t filter = {
    .name = "filter_plugin",
    .init = filterInit,
    .process = filterProcess,
    .shutdown = filterShutdown,
    .setParameters = setFilterParameters};
