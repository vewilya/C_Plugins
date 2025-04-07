#ifndef FILTER_H
#define FILTER_H

#include "types.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern Plugin_t filter;

int filterInit(double sampleRate, int numChannels);
void filterProcess(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer);
void filterShutdown(void);
int setFilterParameters(const char *parameterName, float value);

#endif // FILTER_H