#include "M5_LoRa_E220.h"
#include <vector>

SemaphoreHandle_t xMutex;

template <typename T>
bool ConfRange(T target, T min, T max);

void LoRa_E220::Init(HardwareSerial *serial, unsigned long baud,
                     uint32_t config, uint8_t RX, uint8_t TX) {
    _serial = serial;
    _serial->begin(baud, config, RX, TX);
}

int LoRa_E220::InitLoRaSetting(struct LoRaConfigItem_t &config) {
    int ret = 0;

    if (!ConfRange((int)config.own_channel, 0, 80)) {
        return 1;
    }

    xMutex = xSemaphoreCreateMutex();
    xSemaphoreGive(xMutex);

    // Configuration
    std::vector<uint8_t> command  = {0xc0, 0x00, 0x08};
    std::vector<uint8_t> response = {};

    // Register Address 00H, 01H
    uint8_t ADDH = config.own_address >> 8;
    uint8_t ADDL = config.own_address & 0xff;
    command.push_back(ADDH);
    command.push_back(ADDL);

    // Register Address 02H
    uint8_t REG0 = 0;
    REG0         = REG0 | (config.baud_rate << 5);
    REG0         = REG0 | (config.uart_config << 3);
    REG0         = REG0 | (config.air_data_rate);
    command.push_back(REG0);

    // Register Address 03H
    uint8_t REG1 = 0;
    REG1         = REG1 | (config.subpacket_size << 6);
    REG1         = REG1 | (config.rssi_ambient_noise_flag << 5);
    REG1         = REG1 | (config.transmitting_power);
    command.push_back(REG1);

    // Register Address 04H
    uint8_t REG2 = config.own_channel;
    command.push_back(REG2);

    // Register Address 05H
    uint8_t REG3 = 0;
    REG3         = REG3 | (config.rssi_byte_flag << 7);
    REG3         = REG3 | (config.transmission_method_type << 6);
    REG3         = REG3 | (config.lbt_flag << 4);
    REG3         = REG3 | (config.wor_cycle);
    command.push_back(REG3);

    // Register Address 06H, 07H
    uint8_t CRYPT_H = config.encryption_key >> 8;
    uint8_t CRYPT_L = config.encryption_key & 0xff;
    command.push_back(CRYPT_H);
    command.push_back(CRYPT_L);

    // SerialMon.printf("# Command Request\n");
    // for (auto i : command) {
    // SerialMon.printf("0x%02x ", i);
    // }
    // SerialMon.printf("\n");

    if (xSemaphoreTake(xMutex, (portTickType)100) == pdTRUE) {
        for (auto i : command) {
            _serial->write(i);
        }
        _serial->flush();
        xSemaphoreGive(xMutex);
    }

    delay(100);

    while (_serial->available()) {
        if (xSemaphoreTake(xMutex, (portTickType)100) == pdTRUE) {
            uint8_t data = _serial->read();
            response.push_back(data);
            xSemaphoreGive(xMutex);
        }
    }

    // SerialMon.printf("# Command Response\n");
    // for (auto i : response) {
    // SerialMon.printf("0x%02x ", i);
    // }
    // SerialMon.printf("\n");

    if (response.size() != command.size()) {
        ret = 1;
    }

    return ret;
}

int LoRa_E220::RecieveFrame(struct RecvFrame_t *recv_frame) {
    int len          = 0;
    uint8_t *start_p = recv_frame->recv_data;

    memset(recv_frame->recv_data, 0x00,
           sizeof(recv_frame->recv_data) / sizeof(recv_frame->recv_data[0]));

    while (1) {
        while (_serial->available()) {
            if (xSemaphoreTake(xMutex, (portTickType)100) == pdTRUE) {
                uint8_t ch       = _serial->read();
                *(start_p + len) = ch;
                len++;
                xSemaphoreGive(xMutex);
            }
            if (len > 200) {
                return 1;
            }
        }
        if ((_serial->available() == 0) && (len > 0)) {
            delay(10);
            if (_serial->available() == 0) {
                recv_frame->recv_data_len = len - 1;
                recv_frame->rssi          = recv_frame->recv_data[len - 1] == 0
                                                ? 0
                                                : recv_frame->recv_data[len - 1] - 256;
                break;
            }
        }
        delay(100);
    }

    return 0;
}

int LoRa_E220::SendFrame(struct LoRaConfigItem_t &config, uint8_t *send_data,
                         int size) {
    uint8_t subpacket_size = 0;
    switch (config.subpacket_size) {
        case SUBPACKET_200_BYTE:
            subpacket_size = 200;
            break;
        case SUBPACKET_128_BYTE:
            subpacket_size = 128;
            break;
        case SUBPACKET_64_BYTE:
            subpacket_size = 64;
            break;
        case SUBPACKET_32_BYTE:
            subpacket_size = 32;
            break;
        default:
            subpacket_size = 200;
            break;
    }
    if (size > subpacket_size) {
        // SerialMon.printf("send data length too long\n");
        return 1;
    }
    uint8_t target_address_H = config.target_address >> 8;
    uint8_t target_address_L = config.target_address & 0xff;
    uint8_t target_channel   = config.target_channel;

    uint8_t frame[3 + size] = {target_address_H, target_address_L,
                               target_channel};

    memmove(frame + 3, send_data, size);

    // #if 1 /* print debug */
    //     for (int i = 0; i < 3 + size; i++) {
    //         if (i < 3) {
    //             // SerialMon.printf("%02x", frame[i]);
    //         } else {
    //             // SerialMon.printf("%c", frame[i]);
    //         }
    //     }
    //     // SerialMon.printf("\n");
    // #endif

    if (xSemaphoreTake(xMutex, (portTickType)100) == pdTRUE) {
        for (auto i : frame) {
            _serial->write(i);
        }
        _serial->flush();
        delay(100);
        while (_serial->available()) {
            while (_serial->available()) {
                _serial->read();
            }
            delay(100);
        }
        xSemaphoreGive(xMutex);
    }

    return 0;
}

void LoRa_E220::SetDefaultConfigValue(struct LoRaConfigItem_t &config) {
    const LoRaConfigItem_t default_config = {
        // REG 0-1
        0x0000,  // own_address 0

        // REG 2
        0b011,  // baud_rate 9600 bps
        0b00,   //  uart config 8N1
        0b000,  // datarate 2.4k

        // REG 3
        0b00,    // subpacket_size 200
        0b1000,  // rssi_ambient_noise_flag enable
        0b00,    // transmitting_power 22 dBm

        // REG 4
        0x00,  // own_channel 0 (850.125 + channel * 1M)

        // REG 5
        0b1,    // rssi_byte_flag enable
        0b1,    // transmission_method_type p2p
        0b0,    // lbt_flag disable
        0b011,  // wor_cycle 2000 ms

        // REG 6-7
        0x0000,  // encryption_key 0

        // LOCAL CONFIG FOR SEND API
        0x0000,  // target_address 0
        0x00};   // target_channel 0

    config = default_config;
}

// コンフィグ値が設定範囲内か否か
template <typename T>
bool ConfRange(T target, T min, T max) {
    if (target >= min && target <= max) {
        return true;
    } else {
        return false;
    }
}
