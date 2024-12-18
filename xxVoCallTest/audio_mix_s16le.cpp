#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
// ffplay -ar 48000 -ac 2 -f s16le -i mixed_audio.pcm
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
    // 选择较大的长度，填充较短的数组
    size_t maxSize = std::max(data1.size(), data2.size());
    std::vector<int16_t> mixedData(maxSize);

    // 遍历两个数据流，并进行混合
    for (size_t i = 0; i < maxSize; ++i) {
        // 获取当前数据流的值，超出范围部分默认为 0
        int16_t sample1 = (i < data1.size()) ? data1[i] : 0;
        int16_t sample2 = (i < data2.size()) ? data2[i] : 0;

        // 混合音频数据
        int sample = sample1 + sample2;

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


