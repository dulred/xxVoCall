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
std::ofstream outputPCM("D:/videos/output_audio.pcm", std::ios::binary);

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

// 初始化 WASAPI 进行环回捕获
void initWASAPICapture() {
    // 获取音频设备管理接口
    CoInitialize(nullptr);
    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    IMMDevice* device = nullptr;
    IAudioClient* audioClient = nullptr;
    IAudioCaptureClient* captureClient = nullptr;

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&deviceEnumerator));
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&audioClient);

    WAVEFORMATEX* waveFormat;
    hr = audioClient->GetMixFormat(&waveFormat);  // 获取音频格式

    if (SUCCEEDED(hr)) {
        int sampleRate = waveFormat->nSamplesPerSec;
        int numChannels = waveFormat->nChannels;
        int bitsPerSample = waveFormat->wBitsPerSample;
        
        printf("sampleRate = %d, numChannels = %d, bitsPerSample = %d,",sampleRate,numChannels,bitsPerSample);
        // 获取到的 sampleRate, numChannels, bitsPerSample 可以用于在 ffplay 命令中设置参数
    }

    hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 10000000, 0, waveFormat, nullptr);  // 环回模式

    hr = audioClient->GetService(IID_PPV_ARGS(&captureClient));  // 获取捕获客户端

    UINT32 bufferFrameCount;
    hr = audioClient->GetBufferSize(&bufferFrameCount);

    BYTE* data;
    UINT32 packetLength = 0;
    DWORD flags;

    audioClient->Start();  // 开始音频捕获

    ALCdevice* openALDevice = initOpenALPlayback();  // 初始化 OpenAL 播放

    // 开始捕获循环
    while (true) {
        hr = captureClient->GetNextPacketSize(&packetLength);
        while (packetLength != 0) {
            hr = captureClient->GetBuffer(&data, &packetLength, &flags, nullptr, nullptr);
            savePCMData(data, packetLength, BITS_PER_SAMPLE / 8);  // 保存 PCM 数据
            // playPCMDataOpenAL(data, packetLength);                 // 用 OpenAL 播放音频
            hr = captureClient->ReleaseBuffer(packetLength);
            hr = captureClient->GetNextPacketSize(&packetLength);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 延迟
    }

    // 停止捕获
    audioClient->Stop();
    outputPCM.close();
    alcCloseDevice(openALDevice);
}


int main() {
    initWASAPICapture();
    return 0;
}
