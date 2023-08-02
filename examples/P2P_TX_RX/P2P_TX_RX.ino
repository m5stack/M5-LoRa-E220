/**
 * @file main.cpp
 * @author SeanKwok (shaoxiang@m5stack.com)
 * @brief M5-LoRa-E220 Unit TX/RX Demo.
 * @version 0.2
 * @date 2023-10-19
 *
 *
 * @Hardwares: M5Core,M5-LoRa-E220 Unit
 * @Platform Version: Arduino M5Stack Board Manager v2.0.7
 * @Dependent Library:
 * M5_LoRa_E220: https://github.com/m5stack/M5-LoRa-E220
 * M5Unified: https://github.com/m5stack/M5Unified
 * M5GFX: https://github.com/m5stack/M5GFX
 */

#include <M5Unified.h>
#include <M5GFX.h>
#include "M5_LoRa_E220.h"
#include <Arduino.h>

LoRa_E220 lora;
struct LoRaConfigItem_t config;
struct RecvFrame_t data;

/** prototype declaration **/
void LoRaRecvTask(void *pvParameters);
void LoRaSendTask(void *pvParameters);
void LoRaBtnTask(void *pvParameters);
void ReadDataFromConsole(char *msg, int max_msg_len);

M5GFX display;
M5Canvas canvas(&display);

void print_log(String info) {
    canvas.println(info);
    canvas.pushSprite(0, 0);
    Serial.println(info);
}

void gfx_canvas_init() {
    display.begin();
    canvas.setColorDepth(1);  // mono color
    canvas.setFont(&fonts::efontCN_14);
    canvas.createSprite(display.width(), display.height());
    canvas.setTextSize(1);
    canvas.setPaletteColor(1, GREEN);
    canvas.setTextScroll(true);
    canvas.println(">>LoRa E220 TEST");
    canvas.pushSprite(0, 0);
}

void setup() {
    // put your setup code here, to run once:
    M5.begin();
    gfx_canvas_init();
    delay(1000);  // Serial init wait

    // if use port a
    // lora.Init(&Serial2, 9600, SERIAL_8N1, 22, 21);
    lora.Init();

    lora.SetDefaultConfigValue(config);

    config.own_address              = 0x0000;
    config.baud_rate                = BAUD_9600;
    config.uart_config              = UART_8N1;
    config.air_data_rate            = DATA_RATE_2_4Kbps;
    config.subpacket_size           = SUBPACKET_200_BYTE;
    config.rssi_ambient_noise_flag  = RSSI_AMBIENT_NOISE_ENABLE;
    config.transmitting_power       = TX_POWER_22dBm;
    config.own_channel              = 0x00;
    config.rssi_byte_flag           = RSSI_BYTE_ENABLE;
    config.transmission_method_type = UART_P2P_MODE;
    config.lbt_flag                 = LBT_DISABLE;
    config.wor_cycle                = WOR_2000MS;
    config.encryption_key           = 0x1234;
    config.target_address           = 0x0000;
    config.target_channel           = 0x00;

    if (lora.InitLoRaSetting(config) != 0) {
        print_log("init error, pls pull the M0,M1 to 1");
        print_log("or click Btn to skip");
        while (lora.InitLoRaSetting(config) != 0) {
            M5.update();
            if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() ||
                M5.BtnC.wasPressed()) {
                break;
            }
        }
    }
    print_log("init succeeded, pls pull the M0,M1 to 0");
    print_log("Click Btn to Send Data");

    // マルチタスク
    xTaskCreateUniversal(LoRaRecvTask, "LoRaRecvTask", 8192, NULL, 1, NULL,
                         APP_CPU_NUM);
    xTaskCreateUniversal(LoRaSendTask, "LoRaSendTask", 8192, NULL, 1, NULL,
                         APP_CPU_NUM);
    xTaskCreateUniversal(LoRaBtnTask, "LoRaBtnTask", 8192, NULL, 1, NULL,
                         APP_CPU_NUM);
}

void loop() {
    // put your main code here, to run repeatedly:
    delay(1000);
}

void LoRaBtnTask(void *pvParameters) {
    char msg[] = "Hello LoRa!";
    while (1) {
        M5.update();
        if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() ||
            M5.BtnC.wasPressed()) {
            // ESP32がコンソールから読み込む
            if (lora.SendFrame(config, (uint8_t *)msg, strlen(msg)) == 0) {
                print_log("send succeeded.");
                print_log("");
            } else {
                print_log("send failed.");
                print_log("");
            }
        }
        delay(1);
    }
}

void LoRaRecvTask(void *pvParameters) {
    while (1) {
        if (lora.RecieveFrame(&data) == 0) {
            print_log("recv data:");
            for (int i = 0; i < data.recv_data_len; i++) {
                Serial.printf("%c", data.recv_data[i]);
                canvas.printf("%c", data.recv_data[i]);
            }
            print_log("");
            print_log("hex dump:");
            for (int i = 0; i < data.recv_data_len; i++) {
                Serial.printf("%02x ", data.recv_data[i]);
                canvas.printf("%02x ", data.recv_data[i]);
            }
            Serial.printf("\n");
            Serial.printf("RSSI: %d dBm\n", data.rssi);
            canvas.printf("RSSI: %d dBm\n", data.rssi);
            Serial.printf("\n");
            Serial.flush();
            canvas.pushSprite(0, 0);
        }

        delay(1);
    }
}

void LoRaSendTask(void *pvParameters) {
    while (1) {
        char msg[200] = {0};
        // ESP32がコンソールから読み込む
        ReadDataFromConsole(msg, (sizeof(msg) / sizeof(msg[0])));
        if (lora.SendFrame(config, (uint8_t *)msg, strlen(msg)) == 0) {
            print_log("send succeeded.");
            print_log("");
        } else {
            print_log("send failed.");
            print_log("");
        }

        Serial.flush();

        delay(1);
    }
}

void ReadDataFromConsole(char *msg, int max_msg_len) {
    int len       = 0;
    char *start_p = msg;

    while (len < max_msg_len) {
        if (Serial.available() > 0) {
            char incoming_byte = Serial.read();
            if (incoming_byte == 0x00 || incoming_byte > 0x7F) continue;
            *(start_p + len) = incoming_byte;
            // 最短で3文字(1文字 + CR LF)
            if (incoming_byte == 0x0a && len >= 2 &&
                (*(start_p + len - 1)) == 0x0d) {
                break;
            }
            len++;
        }
        delay(1);
    }

    // msgからCR LFを削除
    len = strlen(msg);
    for (int i = 0; i < len; i++) {
        if (msg[i] == 0x0D || msg[i] == 0x0A) {
            msg[i] = '\0';
        }
    }
}
