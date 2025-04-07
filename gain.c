#include "gain.h"

static struct
{
    float gain;
    bool bypass;
} state = {
    .gain = 0.5f,
    .bypass = false};

int gainerInit(double sampleRate, int numChannels)
{
    printf("Init Gainer Plugin");
    return 1;
}

void gainerProcess(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer)
{
    float *out = (float *)outputBuffer;
    const float *in = (const float *)inputBuffer;

    if (state.bypass > 0.5)
    {
        for (uint16_t i = 0; i < framesPerBuffer; i++)
        {
            /* Interleaved Buffers */
            *out++ = *in++;
            *out++ = *in++;
        }
    }
    else
    {
        for (uint16_t i = 0; i < framesPerBuffer; i++)
        {
            /* Interleaved Buffers */
            *out++ = *in++ * state.gain;
            *out++ = *in++ * state.gain;
        }
    }
}

void gainerShutdown(void)
{
    printf("Shutdown Gainer Plugin");
}

int setParameters(const char *parameterName, float value)
{
    if (strcmp(parameterName, "gain") == 0)
    {
        state.gain = value;

        if (state.gain > 1.0f)
            state.gain = 1.0f;
        if (state.gain < 0.0f)
            state.gain = 0.0f;
        printf("Set gain to %.2f\n", state.gain);

        return 0;
    }
    else if (strcmp(parameterName, "bypass") == 0)
    {
        /* If value bigger than 0.5 -> bypass = true */
        state.bypass = (value > 0.5f);
        printf("Set bypass to %s\n", state.bypass ? "TRUE" : "FALSE");
        return 0;
    }

    printf("Unknown parameter: %s\n", parameterName);
    return -1;
}

Plugin_t gain = {
    .name = "Gain_Plugin",
    .init = gainerInit,
    .process = gainerProcess,
    .shutdown = gainerShutdown,
    .setParameters = setParameters};
