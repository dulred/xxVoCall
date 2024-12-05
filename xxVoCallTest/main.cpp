#include "xxVoCall/xxVoCall.h"
#include <stdio.h>
#include <al.h>
#include <alc.h>
#include <vector>
#include <stdlib.h>
#include <cstdio>
#include <chrono>
#include <thread>

void recordAudioToFile(const char* filename, int durationInSeconds) {
    const int sampleRate = 44100;    // 采样率
    const int numChannels = 2;       // 双声道
    const int bitsPerSample = 16;    // 每个样本16位
    const int bytesPerSample = bitsPerSample / 8;
    const int bufferSize = sampleRate * numChannels * durationInSeconds;  // 总样本数

    // 创建录音设备（麦克风）
    ALCdevice* device = alcCaptureOpenDevice(NULL, sampleRate, AL_FORMAT_STEREO16, sampleRate * durationInSeconds);
    if (!device) {
        printf("Failed to open capture device!\n");
        return;
    }

    alcCaptureStart(device);  // 开始录音
    printf("Recording for %d seconds...\n", durationInSeconds);

    // 缓冲区，用来存储录音数据
    std::vector<ALshort> buffer;

    // 循环每500毫秒捕获音频数据，直到达到指定时长
    const int captureIntervalMs = 500;
    int totalSamplesCaptured = 0;
    while (totalSamplesCaptured < bufferSize) {
        std::this_thread::sleep_for(std::chrono::milliseconds(captureIntervalMs));  // 等待500毫秒

        ALint samplesAvailable;
        alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, 1, &samplesAvailable);  // 查询缓冲区中可用的样本数

        if (samplesAvailable > 0) {
            std::vector<ALshort> tempBuffer(samplesAvailable * numChannels);  // 每个样本的大小按通道数计算
            alcCaptureSamples(device, tempBuffer.data(), samplesAvailable);   // 捕获样本
            buffer.insert(buffer.end(), tempBuffer.begin(), tempBuffer.end());  // 添加到主缓冲区
            totalSamplesCaptured += samplesAvailable * numChannels;  // 更新总样本数
        }
    }

    alcCaptureStop(device);         // 停止录音
    alcCaptureCloseDevice(device);  // 关闭录音设备

    // 使用 fopen 写入文件
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Failed to open file for writing!\n");
        return;
    }

    // 将 PCM 数据写入文件
    size_t written = fwrite(buffer.data(), sizeof(ALshort), buffer.size(), file);
    if (written != buffer.size()) {
        printf("Failed to write all audio data to file!\n");
    } else {
        printf("Recording saved to %s\n", filename);
    }

    // 关闭文件
    fclose(file);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <output_file> <duration_in_seconds>\n", argv[0]);
        return -1;
    }

    const char* filename = argv[1];
    int durationInSeconds = atoi(argv[2]);

    recordAudioToFile(filename, durationInSeconds);
    return 0;
}
