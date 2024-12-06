#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <vector>

using boost::asio::ip::udp;
namespace asio = boost::asio;

// 保存接收到的数据到PCM文件
void writePCMData(const std::string& fileName, const std::vector<char>& data) {
    std::ofstream file(fileName, std::ios::binary);
    file.write(data.data(), data.size());
}

int main() {
    try {
        // 初始化Boost.Asio的io_service对象
        asio::io_context io_context;

        // 创建UDP服务器端口
        udp::socket socket(io_context, udp::endpoint(udp::v4(), 12345));

        std::cout << "Server is listening on port 12345..." << std::endl;

        // 接收缓冲区，足够大以便接收每个UDP包
        size_t max_buffer_size = 1280;
        std::vector<char> recv_buffer(max_buffer_size);

        // 用来存储完整接收到的PCM数据
        std::vector<char> complete_data;
        // 累计接收到的字节数

        size_t total_received_bytes = 0;
        // 设置接收缓冲区大小为 1MB
        socket.set_option(boost::asio::socket_base::receive_buffer_size(1024 * 1024 * 2 ));
        // 设置发送缓冲区大小为 1MB
        socket.set_option(boost::asio::socket_base::send_buffer_size(2 * 1024 * 1024));
        while (true) {
            udp::endpoint client_endpoint;
            size_t len = socket.receive_from(boost::asio::buffer(recv_buffer), client_endpoint);

            std::cout << "Received " << len << " bytes of PCM data from " << client_endpoint << std::endl;

            // 将接收到的数据添加到完整数据容器
            complete_data.insert(complete_data.end(), recv_buffer.begin(), recv_buffer.begin() + len);
            
            // 更新累计接收的字节数
            total_received_bytes += len;

            // 这里可以根据协议或者数据大小来判断是否接收完成
            // 假设知道接收到的数据已完成，可以保存数据
            // 比如如果收到的数据大小达到预期值时，保存到文件
            writePCMData("D:/videos/udp/received_audio.pcm", complete_data);

            printf("total_received_bytes: %d\n", total_received_bytes);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
