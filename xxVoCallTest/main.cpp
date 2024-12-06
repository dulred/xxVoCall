#include "xxVoCall/xxVoCall.h"
#include <stdio.h>
#include <al.h>
#include <alc.h>
#include <vector>
#include <stdlib.h>
#include <cstdio>
#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <output_file> <duration_in_seconds>\n", argv[0]);
        return -1;
    }

    const char* filename = argv[1];
    int durationInSeconds = atoi(argv[2]);
    return 0;
}
