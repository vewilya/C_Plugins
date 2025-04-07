#include "plugin_handler.h"

#define MAX_PLUGINS 10

Plugin_t *registeredPlugins[MAX_PLUGINS];
int plugin_count = 0;

int registerPluginToChain(Plugin_t *pluginToRegister)
{
    if (plugin_count >= MAX_PLUGINS)
    {
        printf("Error: Plugin Chain is full.");
        return -1;
    }

    registeredPlugins[plugin_count] = pluginToRegister;

    /* Return index where plugin was registered */
    return plugin_count++;
}

int initAllPlugins(double sampleRate, int numChannels)
{
    for (uint16_t i = 0; i < plugin_count; i++)
    {
        Plugin_t *nextPluginToInitialise = registeredPlugins[i];
        printf("Initialising plugin: %s\n", nextPluginToInitialise->name);

        if (nextPluginToInitialise->init(sampleRate,
                                         numChannels) != 0)
        {
            printf("Failed to initialise plugin.\n");
            return -1;
        }
    }
    return 0;
}

int processPluginChain(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer)
{
    for (uint16_t i = 0; i < plugin_count; i++)
    {
        Plugin_t *nextPluginToProcessWith = registeredPlugins[i];

        if (nextPluginToProcessWith == NULL)
        {
            return -1;
        }

        nextPluginToProcessWith->process(inputBuffer, outputBuffer, framesPerBuffer);
    }

    return 0;
}

void shutdownAllPlugins(void)
{
}
