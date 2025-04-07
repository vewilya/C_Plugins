#ifndef TYPES_H
#define TYPES_H

typedef struct Plugin_
{
    const char *name;
    int (*init)(double sampleRate, int numChannels);
    void (*process)(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer);
    void (*shutdown)(void);
    int (*setParameters)(const char *parameterName, float value);
} Plugin_t;

#endif // TYPES_H