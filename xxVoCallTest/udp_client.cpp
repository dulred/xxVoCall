#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <boost/locale.hpp>

using boost::asio::ip::udp;
namespace asio = boost::asio;


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

// 保存数据到 PCM 文件
void writePCMData(const std::string& fileName, const std::vector<char>& data) {
    std::ofstream file(fileName, std::ios::binary | std::ios::app);  // 以追加模式打开文件
    file.write(data.data(), data.size());
}


std::vector<char> readPCMData(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary);
    std::vector<char> data;

    if (file.is_open()) {
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        data.resize(size);
        file.read(data.data(), size);
    } else {
        std::cerr << "Failed to open PCM file." << std::endl;
    }
    return data;
}



int main() {
    try {
        // 初始化Boost.Asio的io_service对象
        asio::io_context io_context;

        // 客户端UDP socket
        udp::socket socket(io_context);
        udp::endpoint server_endpoint(asio::ip::address::from_string("127.0.0.1"), 12345);
        socket.open(udp::v4());

        // 读取PCM数据
        std::vector<char> pcm_data = readPCMData("D:/videos/output2.pcm");

        const size_t MAX_UDP_PACKET_SIZE = 1280;  // 最大 UDP 数据包大小

        size_t offset = 0;

        // 查询接收缓冲区大小
        boost::asio::socket_base::receive_buffer_size receive_size_option;
        socket.get_option(receive_size_option);
        std::cout << "Default receive buffer size: " << receive_size_option.value() << " bytes" << std::endl;

        // 查询发送缓冲区大小
        boost::asio::socket_base::send_buffer_size send_size_option;
        socket.get_option(send_size_option);
        std::cout << "Default send buffer size: " << send_size_option.value() << " bytes" << std::endl;

        // 设置接收缓冲区大小为 1MB
        socket.set_option(boost::asio::socket_base::receive_buffer_size(2 * 1024 * 1024));
        // 设置发送缓冲区大小为 1MB
        socket.set_option(boost::asio::socket_base::send_buffer_size(2 * 1024 * 1024));

        while (offset < pcm_data.size()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            size_t length = std::min(MAX_UDP_PACKET_SIZE, pcm_data.size() - offset);

            socket.send_to(boost::asio::buffer(pcm_data.data() + offset, length), server_endpoint);

            // 保存发送的数据到文件,验证是否成功发送
            // writePCMData("D:/videos/udp/sent_audio.pcm", std::vector<char>(pcm_data.data() + offset, pcm_data.data() + offset + length));
        
            offset += length;
        }

        std::cout << "Sent " << pcm_data.size() << " bytes of PCM data to the server." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << gbkToUtf8(e.what()) << std::endl;
    }

    return 0;
}
