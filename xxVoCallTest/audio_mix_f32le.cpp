#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
// ffplay -ar 48000 -ac 2 -f f32le -i mixed2_audio.pcm
// 读取 f32le 格式的 PCM 数据到 vector
std::vector<float> readPCMData(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary);
    std::vector<float> data;

    if (file.is_open()) {
        file.seekg(0, std::ios::end);
        size_t size = file.tellg() / sizeof(float);  // 每个样本是 float 类型（4 字节）
        file.seekg(0, std::ios::beg);

        data.resize(size);
        file.read(reinterpret_cast<char*>(data.data()), size * sizeof(float));
    }

    return data;
}

// 保存混合后的 PCM 数据为 f32le 格式
void writePCMData(const std::string& fileName, const std::vector<float>& data) {
    std::ofstream file(fileName, std::ios::binary);
    file.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(float));
}

// 混合两个 f32le 格式的 PCM 数据流
std::vector<float> mixPCMData(const std::vector<float>& data1, const std::vector<float>& data2) {
    // 选择较大的长度，填充较短的数组
    size_t maxSize = std::max(data1.size(), data2.size());
    std::vector<float> mixedData(maxSize);

    // 遍历两个数据流，并进行混合
    for (size_t i = 0; i < maxSize; ++i) {
        // 获取当前数据流的值，超出范围部分默认为 0
        float sample1 = (i < data1.size()) ? data1[i] : 0.0f;
        float sample2 = (i < data2.size()) ? data2[i] : 0.0f;

        // 混合音频数据
        float sample = sample1 + sample2;

        // 限制混合结果在 [-1.0, 1.0] 之间
        if (sample > 1.0f) {
            sample = 1.0f;
        } else if (sample < -1.0f) {
            sample = -1.0f;
        }

        mixedData[i] = sample;
    }

    return mixedData;
}

int main() {
    // 读取两个 f32le 格式的 PCM 文件
    std::vector<float> pcmData1 = readPCMData("D:/videos/output_microphone.pcm");
    std::vector<float> pcmData2 = readPCMData("D:/videos/output_desktop.pcm");

    // 混合 PCM 数据
    std::vector<float> mixedPCM = mixPCMData(pcmData1, pcmData2);

    // 将混合后的 PCM 数据保存为新的 f32le 格式文件
    writePCMData("D:/videos/mixed2_audio.pcm", mixedPCM);

    std::cout << "PCM 数据混合完成，已保存到 mixed_audio.pcm" << std::endl;
    return 0;
}
