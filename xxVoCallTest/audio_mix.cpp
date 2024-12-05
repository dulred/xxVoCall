#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

// 读取 PCM 文件的数据到 vector
std::vector<int16_t> readPCMData(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary);
    std::vector<int16_t> data;

    if (file.is_open()) {
        file.seekg(0, std::ios::end);
        size_t size = file.tellg() / sizeof(int16_t);
        file.seekg(0, std::ios::beg);

        data.resize(size);
        file.read(reinterpret_cast<char*>(data.data()), size * sizeof(int16_t));
    }

    return data;
}

// 保存混合后的 PCM 数据
void writePCMData(const std::string& fileName, const std::vector<int16_t>& data) {
    std::ofstream file(fileName, std::ios::binary);
    file.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(int16_t));
}

// 混合两个 PCM 数据流
std::vector<int16_t> mixPCMData(const std::vector<int16_t>& data1, const std::vector<int16_t>& data2) {
    size_t minSize = std::min(data1.size(), data2.size());
    std::vector<int16_t> mixedData(minSize);

    for (size_t i = 0; i < minSize; ++i) {
        int sample = data1[i] + data2[i];

        // 剪裁到 int16_t 的范围，避免溢出
        if (sample > 32767) {
            sample = 32767;
        } else if (sample < -32768) {
            sample = -32768;
        }

        mixedData[i] = static_cast<int16_t>(sample);
    }

    return mixedData;
}

int main() {
    // 读取两个 PCM 文件
    std::vector<int16_t> pcmData1 = readPCMData("D:/videos/output.pcm");
    std::vector<int16_t> pcmData2 = readPCMData("D:/videos/output2.pcm");

    // 混合 PCM 数据
    std::vector<int16_t> mixedPCM = mixPCMData(pcmData1, pcmData2);

    // 将混合后的 PCM 数据保存为新文件
    writePCMData("D:/videos/mixed_audio.pcm", mixedPCM);

    std::cout << "PCM 数据混合完成，已保存到 mixed_audio.pcm" << std::endl;
    return 0;
}


