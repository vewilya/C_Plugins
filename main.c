#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <stdbool.h>

#include "plugin_handler.h"
#include "gain.h"
#include "filter.h"

#define SAMPLE_RATE 48000.0
#define FRAMES_PER_BUFFER 1024
#define NUM_CHANNELS 2

int isEffectOn = 1;
bool keepRunning = true;

// extern Plugin_t gainerPlugin;

int audioProcessFunction(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
    float *out = (float *)outputBuffer;
    const float *in = (const float *)inputBuffer;
    (void)timeInfo;
    (void)statusFlags;
    (void)userData;

    if (inputBuffer == NULL)
    {
        for (uint16_t i = 0; i < framesPerBuffer; i++)
        {
            *out++ = 0.0f; /* left channel */
            *out++ = 0.0f; /* right channel */
        }
    }
    else
    {
        /* If the inut exists -> */
        if (!isEffectOn)
        {
            for (uint16_t i = 0; i < framesPerBuffer; i++)
            {
                *out++ = *in++; /* left channel */
                *out++ = *in++; /* right channel */
            }
        }
        else
        {
            // gainerPlugin.process(inputBuffer, outputBuffer, framesPerBuffer);
            processPluginChain(inputBuffer, outputBuffer, framesPerBuffer);
        }
    }

    return keepRunning ? paContinue : paComplete;
}

int main(int argc, const char *argv[])
{
    printf("Hello, from C_Plugins!\n");

    // registerPluginToChain(&gain);
    registerPluginToChain(&filter);
    initAllPlugins(SAMPLE_RATE, NUM_CHANNELS);

    // gain.setParameters("gain", 0.2f);
    filter.setParameters("frequency", 520.0f);

    /* Initilaise PortAudio*/
    PaError err;

    err = Pa_Initialize();
    if (err == paNoError)
        printf("Portaudio initialised\n");
    else
        printf("%s \n", Pa_GetErrorText(err));

    /* Get default devices and infos thereof */
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0)
    {
        printf("Error getting device count: %s\n", Pa_GetErrorText(numDevices));
        Pa_Terminate();

        return EXIT_FAILURE;
    }

    printf("Number of audio devices: %d\n", numDevices);

    const PaDeviceInfo *defaultInputDeviceInfo = Pa_GetDeviceInfo(Pa_GetDefaultInputDevice());
    const PaDeviceInfo *defaultOuputDeviceInfo = Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());

    /* Number of audio devices */
    int ndev = Pa_GetDeviceCount();
    printf("\nInput Devices: \n");
    for (uint16_t i = 0; i < ndev; i++)
    {
        const PaDeviceInfo *inputInfo = Pa_GetDeviceInfo((PaDeviceIndex)i);

        if (inputInfo->maxInputChannels > 0)
            printf("%d: %s\n", i, inputInfo->name);
    }

    int selectedInputDeviceIndex;
    printf("\nSelect Input Device: \n");
    scanf("%d", &selectedInputDeviceIndex);
    printf("\nYour selected input device: %s. \n", Pa_GetDeviceInfo((PaDeviceIndex)selectedInputDeviceIndex)->name);

    printf("\nOutput Devices: \n");
    for (uint16_t i = 0; i < ndev; i++)
    {
        const PaDeviceInfo *outputInfo = Pa_GetDeviceInfo((PaDeviceIndex)i);
        if (outputInfo->maxOutputChannels > 0)
            printf("%d: %s\n", i, outputInfo->name);
    }

    int selectedOutputDeviceIndex;
    printf("\nSelect output Device: \n");
    scanf("%d", &selectedOutputDeviceIndex);
    printf("\nYour selected output device: %s. \n", Pa_GetDeviceInfo((PaDeviceIndex)selectedOutputDeviceIndex)->name);
    getchar();

    if (!defaultInputDeviceInfo || !defaultOuputDeviceInfo)
    {
        printf("Error getting devie info\n");
        Pa_Terminate();

        return EXIT_FAILURE;
    }

    /* Audio Streaming */
    PaStream *stream;

    /* Custom stream parameters */
    PaStreamParameters inparam, outparam;
    inparam.device = selectedInputDeviceIndex;
    inparam.channelCount = NUM_CHANNELS; // Stereo input, adjust if needed
    inparam.sampleFormat = paFloat32;
    inparam.suggestedLatency = Pa_GetDeviceInfo(selectedInputDeviceIndex)->defaultHighInputLatency;
    inparam.hostApiSpecificStreamInfo = NULL;

    outparam.device = selectedOutputDeviceIndex;
    outparam.channelCount = NUM_CHANNELS; // Stereo output
    outparam.sampleFormat = paFloat32;
    outparam.suggestedLatency = Pa_GetDeviceInfo(selectedOutputDeviceIndex)->defaultHighOutputLatency;
    outparam.hostApiSpecificStreamInfo = NULL;

    /* Create audio stream with selected input ad output devices*/
    err = Pa_OpenStream(&stream,
                        &inparam,
                        &outparam,
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        paClipOff, // Don't clip
                        audioProcessFunction,
                        NULL);

    /* Print selected audio devices */
    const PaDeviceInfo *selectedInputDeviceInfo = Pa_GetDeviceInfo((PaDeviceIndex)selectedInputDeviceIndex);
    const PaDeviceInfo *selectedOutputDeviceInfo = Pa_GetDeviceInfo((PaDeviceIndex)selectedOutputDeviceIndex);

    printf("\nDefault input device: %s (max channels: %d)\n",
           selectedInputDeviceInfo->name, selectedInputDeviceInfo->maxInputChannels);
    printf("Default output device: %s (max channels: %d)\n",
           selectedOutputDeviceInfo->name, selectedOutputDeviceInfo->maxOutputChannels);

    /* Print Default audio devices */
    // printf("\nDefault input device: %s (max channels: %d)\n",
    //     defaultInputDeviceInfo->name, inputDeviceInfo->maxInputChannels);
    // printf("Default output device: %s (max channels: %d)\n",
    //     defaultOuputDeviceInfo->name, defaultOuputDeviceInfo->maxOutputChannels);

    /* Open Stream with default audio devices */
    // err = Pa_OpenDefaultStream(&stream,
    //                             1,
    //                             2,
    //                             paFloat32,
    //                             SAMPLE_RATE,
    //                             FRAMES_PER_BUFFER,
    //                             audioProcessFunction,
    //                             NULL);

    if (err != paNoError)
    {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();

        return EXIT_FAILURE;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));

        /* Now, we also need to close the audio stream before terminating the application */
        Pa_CloseStream(stream);
        Pa_Terminate();

        return EXIT_FAILURE;
    }

    printf("Audio stream is active.\n");
    printf("Press 1 to turn on the effect (currently %s)\n", isEffectOn ? "ON" : "OFF");
    printf("Press 2 to turn off the effect (currently %s)\n", isEffectOn ? "ON" : "OFF");
    printf("Press 4 to stop streaming and exit\n");

    while (keepRunning)
    {
        int key = getchar();

        if (key == 49)
        {
            isEffectOn = true;
            printf("Effect is %s\n", isEffectOn ? "ON" : "OFF");
        }
        else if (key == 50)
        {
            isEffectOn = false;
            printf("Effect is %s\n", isEffectOn ? "ON" : "OFF");
        }
        else if (key == 52)
        {
            printf("Stopping the audio stream...\n");
            keepRunning = false;
        }
        else
        {
            if (key != 10)
                printf("Invalid Input... Try again.\n");
        }
        // printf("ASCII code is: %d\n", (int)key);
        Pa_Sleep(50);
    }

    /* Stop and close the stream */
    err = Pa_StopStream(stream);
    if (err != paNoError)
    {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }

    err = Pa_CloseStream(stream);
    if (err != paNoError)
    {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }

    /* Terminate PortAudio */
    Pa_Terminate();

    return EXIT_SUCCESS;
}
