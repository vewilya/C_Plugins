#ifndef PLUGIN_HANDLER_H
#define PLUGIN_HANDLER_H

#include "types.h"
#include "stdlib.h"
#include "stdio.h"

int registerPluginToChain(Plugin_t *pluginToRegitser);

int initAllPlugins(double sampleRate, int numChannels);
int processPluginChain(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer);
void shutdownAllPlugins(void);

#endif // PLUGIN_HANDLER_H