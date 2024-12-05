#include <stdio.h>
#include <al.h>
#include <alc.h>
#include <vector>
#include <cstring>
#include <boost/locale.hpp> 
#include <iostream>
#include <fstream>

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswresample/swresample.h>
    #include <libavutil/time.h>
}


int audioPrint(const AVFrame* frame){

    int channel = frame->channels;
    int samples = frame->nb_samples;
    printf("Channel: %d\n",channel);
    printf("nb_samples:%d\n",frame->nb_samples);
    printf("sample_rate: %d\n",frame->sample_rate);

    AVSampleFormat format = (AVSampleFormat)(frame->format);
    
    char* str = (char *)malloc(128);
    str = av_get_sample_fmt_string(str,128,format);
    
    printf("Sample Format: %s\n",str);

    free(str);
    for (int i = 0; i < AV_NUM_DATA_POINTERS; i++)
    {
        printf("Linesize[%d]: %d \n",i,frame->linesize[i]);
    }



    return 0;
}
void save_pcm_data(const AVFrame* frame) {
    // 打开 PCM 文件以追加方式写入
    FILE* pcmFile = fopen("D:/videos/output.pcm", "ab");  // "ab" 以二进制方式打开文件并追加
    if (!pcmFile) {
        perror("Failed to open file");
        return;
    }

    // 计算音频数据大小：nb_samples * channels * 2 (16-bit per sample)
    int bytesPerSample = 2;  // 16-bit per sample
    int samples = frame->nb_samples;
    int channels = frame->channels;
    size_t dataSize = samples * channels * bytesPerSample;

    // 写入 PCM 数据
    size_t bytesWritten = fwrite(frame->data[0], 1, dataSize, pcmFile);
    if (bytesWritten != dataSize) {
        perror("Failed to write all data");
    }

    // 关闭文件
    fclose(pcmFile);
}

void decode_wav_to_pcm(const char* input_filename) {
    AVFormatContext* formatContext = nullptr;
    if (avformat_open_input(&formatContext, input_filename, nullptr, nullptr) != 0) {
        std::cerr << "Error opening file: " << input_filename << std::endl;
        return;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cerr << "Error finding stream info." << std::endl;
        return;
    }

    // 查找音频流
    int audioStreamIndex = -1;
    for (int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex == -1) {
        std::cerr << "No audio stream found." << std::endl;
        return;
    }

    AVStream* audioStream = formatContext->streams[audioStreamIndex];
    AVCodecParameters* codecParameters = audioStream->codecpar;

    // 查找解码器
    AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
    if (!codec) {
        std::cerr << "Codec not found!" << std::endl;
        return;
    }

    // 打开解码器
    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
        std::cerr << "Failed to copy codec parameters to context." << std::endl;
        return;
    }

    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        std::cerr << "Failed to open codec." << std::endl;
        return;
    }

    // 读取音频包并解码
    AVPacket packet;
    AVFrame* frame = av_frame_alloc();
    while (av_read_frame(formatContext, &packet) >= 0) {
        if (packet.stream_index == audioStreamIndex) {
            int ret = avcodec_send_packet(codecContext, &packet);
            if (ret < 0) {
                std::cerr << "Error sending packet for decoding." << std::endl;
                break;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(codecContext, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    std::cerr << "Error decoding audio frame." << std::endl;
                    break;
                }

                // 保存 PCM 数据
                save_pcm_data(frame);
            }
        }
        av_packet_unref(&packet);
    }

    // 清理
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
}

// 将 GBK 编码转换为 UTF-8 编码
std::string gbkToUtf8(const char* gbkStr) {
    try {
        // 使用 Boost.Locale 进行 GBK 到 UTF-8 的转换
        return boost::locale::conv::to_utf<char>(gbkStr, "GBK");
    } catch (const std::exception& e) {
        std::cerr << "Error converting GBK to UTF-8: " << e.what() << std::endl;
        return "";
    }
}

void listAudioOutputDevices(ALCdevice*& captureDevice) {

    if (!alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT")) {
        std::cerr << "Enumeration extension not available" << std::endl;
        return;
    }

    // 获取设备列表
    const ALCchar* devices = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
    const ALCchar* device = devices;
    int deviceIndex = 1;  // 设备的索引，从1开始（因为你可能想选择第三个设备）

    std::cout << "Available audio output devices:" << std::endl;
    
    // 遍历设备列表
    while (device && *device != '\0') {
        std::cout << "  Device " << deviceIndex << ": " <<gbkToUtf8(device)<< std::endl;
        device += strlen(device) + 1;
        deviceIndex++;
    }
    deviceIndex = 1;
    // 选择第三个设备并打开
    device = devices;
    int targetDeviceIndex = 1;  // 目标设备的索引，第三个设备

    while (device && *device != '\0') {
        if (deviceIndex == targetDeviceIndex) {
            std::cout << "Opening device: " << device << std::endl;
            captureDevice = alcOpenDevice(device); // 打开第三个设备
            
            if (captureDevice) {
                std::cout << "Device opened successfully!" << std::endl;
            } else {
                std::cerr << "Failed to open device!" << std::endl;
            }

            break;
        }
        device += strlen(device) + 1;
        deviceIndex++;
 
    }
}

void playAudioFromFile(const char* filename) {
    const int sampleRate = 48000;  // 采样率
    const int numChannels = 2;     // 双声道
    const int bitsPerSample = 16;  // 每个样本16位

    // 使用 fopen 读取 PCM 文件
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file for reading!\n");
        return;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long dataSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 创建 buffer 存储数据
    std::vector<ALshort> buffer(dataSize / sizeof(ALshort));

    // 读取文件内容到 buffer
    size_t read = fread(buffer.data(), sizeof(ALshort), buffer.size(), file);
    if (read != buffer.size()) {
        printf("Failed to read all audio data from file!\n");
        fclose(file);
        return;
    }
    fclose(file);

    // 初始化OpenAL设备和上下文
    // ALCdevice* device = alcOpenDevice(NULL); // 打开默认设备
    ALCdevice* device = nullptr;
    listAudioOutputDevices(device);
    
    if (!device) {
        printf("Failed to open OpenAL device!\n");
        return;
    }

    ALCcontext* context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);

    // 创建缓冲区并填充数据
    ALuint bufferID;
    alGenBuffers(1, &bufferID);
    alBufferData(bufferID, AL_FORMAT_STEREO16, buffer.data(), buffer.size() * sizeof(ALshort), sampleRate);

    // 创建源并播放
    ALuint sourceID;
    alGenSources(1, &sourceID);
    alSourcei(sourceID, AL_BUFFER, bufferID);
    alSourcePlay(sourceID);

    printf("Playing audio...\n");

    // 等待音频播放完成
    ALint state;
    do {
        alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
    } while (state == AL_PLAYING);

    // 清理
    alDeleteSources(1, &sourceID);
    alDeleteBuffers(1, &bufferID);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }

    const char* filename = argv[1];
    // decode_wav_to_pcm(filename);
    printf("Playing audio from file: %s\n", filename);
    playAudioFromFile(filename);
    return 0;
}

