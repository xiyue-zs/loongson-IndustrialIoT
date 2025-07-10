#include "drivers/camera_driver.hpp"
#include "drivers/mic_driver.hpp"
#include "drivers/dht11_driver.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <thread>
#include <cstring>
#include <json/json.h>

const int SERVER_PORT = 8080;
const char* SERVER_IP = "192.168.1.100"; // 目标PC的IP

void send_data(int sock, const std::vector<uint8_t>& data) {
    uint32_t size = htonl(data.size());
    send(sock, &size, sizeof(size), 0);
    send(sock, data.data(), data.size(), 0);
}

int main() {
    // 初始化设备
    USBCamera camera;
    if (!camera.init()) {
        std::cerr << "Camera init failed!" << std::endl;
        return 1;
    }

    INMP441_Mic mic;
    if (!mic.init()) {
        std::cerr << "Mic init failed!" << std::endl;
        return 1;
    }

    DHT11_Sensor dht(17); // GPIO17

    // 网络连接
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) {
        perror("Connect failed");
        return 1;
    }

    while (true) {
        // 采集数据
        std::vector<uint8_t> image;
        if (camera.capture_frame(image)) {
            send_data(sock, image);
        }

        std::vector<int16_t> audio(44100); // 1秒音频
        if (mic.capture_audio(audio, audio.size())) {
            send_data(sock, std::vector<uint8_t>(
                reinterpret_cast<uint8_t*>(audio.data()),
                reinterpret_cast<uint8_t*>(audio.data() + audio.size())
            ));
        }

        float temp, hum;
        if (dht.read(temp, hum)) {
            Json::Value json;
            json["temperature"] = temp;
            json["humidity"] = hum;
            Json::StreamWriterBuilder builder;
            std::string json_str = Json::writeString(builder, json);
            send_data(sock, std::vector<uint8_t>(json_str.begin(), json_str.end()));
        }

        std::this_thread::sleep_for(std::chrono::seconds(5)); // 5秒采集周期
    }

    close(sock);
    return 0;
}