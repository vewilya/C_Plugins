#ifndef GAIN_H
#define GAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "types.h"
#include "stdbool.h"

extern Plugin_t gain;

int gainerInit(double sampleRate, int numChannels);
void gainerProcess(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer);
void gainerShutdown(void);
int setParameters(const char *parameterName, float value);

#endif // GAIN_H