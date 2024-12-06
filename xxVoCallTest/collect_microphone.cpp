#include <audioclient.h>
#include <mmdeviceapi.h>
#include <thread>
#include <iostream>
#include <vector>
#include <fstream>
#include <al.h>
#include <alc.h>

// 定义采样率、通道数和位深度
const int SAMPLE_RATE = 48000;
const int NUM_CHANNELS = 2;
const int BITS_PER_SAMPLE = 32;

// 打开 PCM 文件进行写入
std::ofstream outputPCM("D:/videos/output_microphone.pcm", std::ios::binary);

// 保存 PCM 数据到文件
void savePCMData(const unsigned char* data, unsigned int numFrames, int bytesPerSample) {
    outputPCM.write(reinterpret_cast<const char*>(data), numFrames * NUM_CHANNELS * bytesPerSample);
}
// 初始化 OpenAL 播放音频数据
ALCdevice* initOpenALPlayback() {
    ALCdevice* device = alcOpenDevice(nullptr); // 使用默认设备
    ALCcontext* context = alcCreateContext(device, nullptr);
    alcMakeContextCurrent(context);
    return device;
}

// 播放 PCM 数据
void playPCMDataOpenAL(const unsigned char* data, unsigned int numFrames) {
    ALuint buffer, source;
    alGenBuffers(1, &buffer);
    alGenSources(1, &source);

    // 上传数据到 OpenAL 缓冲区
    alBufferData(buffer, AL_FORMAT_STEREO16, data, numFrames * NUM_CHANNELS * (BITS_PER_SAMPLE / 8), SAMPLE_RATE);

    // 播放
    alSourcei(source, AL_BUFFER, buffer);
    alSourcePlay(source);

    // 等待播放结束
    ALint state;
    do {
        alGetSourcei(source, AL_SOURCE_STATE, &state);
    } while (state == AL_PLAYING);

    // 清理资源
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
}

// 初始化 WASAPI 进行环回捕获 (采集桌面音频)
void initWASAPICaptureMicrophone() {
    // 初始化 COM 库
    CoInitialize(nullptr);

    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    IMMDevice* device = nullptr;
    IAudioClient* audioClient = nullptr;
    IAudioCaptureClient* captureClient = nullptr;

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&deviceEnumerator));
    if (FAILED(hr)) {
        std::cerr << "Failed to create MMDeviceEnumerator" << std::endl;
        return;
    }

    // 获取默认麦克风输入设备
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &device);
    if (FAILED(hr)) {
        std::cerr << "Failed to get default audio capture device" << std::endl;
        return;
    }

    // 激活音频客户端
    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&audioClient);
    if (FAILED(hr)) {
        std::cerr << "Failed to activate audio client" << std::endl;
        return;
    }

    // 获取音频格式（采样率、通道数、位深度等）
    WAVEFORMATEX* waveFormat;
    hr = audioClient->GetMixFormat(&waveFormat);
    if (FAILED(hr)) {
        std::cerr << "Failed to get mix format" << std::endl;
        return;
    }

    // 输出音频格式信息
    std::cout << "Sample Rate: " << waveFormat->nSamplesPerSec << std::endl;
    std::cout << "Channels: " << waveFormat->nChannels << std::endl;
    std::cout << "Bits Per Sample: " << waveFormat->wBitsPerSample << std::endl;

    // 初始化音频客户端
    hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, waveFormat, nullptr);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize audio client, HRESULT: " << hr << std::endl;
        return;
    }

    // 获取捕获客户端
    hr = audioClient->GetService(IID_PPV_ARGS(&captureClient));
    if (FAILED(hr)) {
        std::cerr << "Failed to get capture client" << std::endl;
        return;
    }

    UINT32 bufferFrameCount;
    hr = audioClient->GetBufferSize(&bufferFrameCount);  // 获取缓冲区大小
    if (FAILED(hr)) {
        std::cerr << "Failed to get buffer size" << std::endl;
        return;
    }

    BYTE* data = nullptr;
    UINT32 packetLength = 0;
    DWORD flags = 0;

    // 开始捕获
    hr = audioClient->Start();
    if (FAILED(hr)) {
        std::cerr << "Failed to start audio client" << std::endl;
        return;
    }

    // 开始捕获循环
    while (true) {
        hr = captureClient->GetNextPacketSize(&packetLength);
        while (packetLength != 0) {
            hr = captureClient->GetBuffer(&data, &packetLength, &flags, nullptr, nullptr);
            if (SUCCEEDED(hr)) {
                // 处理 PCM 数据
                // 这里假设数据为 16 位 PCM
                // 保存或者处理 data（此时是一个字节数组）
                std::cout << "Captured " << packetLength << " bytes of audio data" << std::endl;

                // 假设保存为文件或播放
                savePCMData(data, packetLength, BITS_PER_SAMPLE / 8);  // 保存 PCM 数据

                // 释放缓冲区
                hr = captureClient->ReleaseBuffer(packetLength);
            }
            hr = captureClient->GetNextPacketSize(&packetLength);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 延迟，避免CPU过高负载
    }

    // 停止捕获
    audioClient->Stop();
}


int main() {
    initWASAPICaptureMicrophone();
    return 0;
}
