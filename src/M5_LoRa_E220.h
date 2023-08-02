#ifndef __M5_LORA_E220_LIB_H__
#define __M5_LORA_E220_LIB_H__

#include <Arduino.h>
#include <SPIFFS.h>
#include "M5_LoRa_E220_DEF.h"

#define CONFIG_MODE_BAUD 9600

struct LoRaConfigItem_t {
    uint16_t own_address;
    uint8_t baud_rate;
    uint8_t uart_config;
    uint8_t air_data_rate;
    uint8_t subpacket_size;
    uint8_t rssi_ambient_noise_flag;
    uint8_t transmitting_power;
    uint8_t own_channel;
    uint8_t rssi_byte_flag;
    uint8_t transmission_method_type;
    uint8_t lbt_flag;
    uint16_t wor_cycle;
    uint16_t encryption_key;
    uint16_t target_address;
    uint8_t target_channel;
};

struct RecvFrame_t {
    uint8_t recv_data[201];
    uint8_t recv_data_len;
    int rssi;
};

class LoRa_E220 {
   public:
    /**
     * @brief Init Serial
     *
     * @param serial
     * @param RX
     * @param TX
     * @return bool
     */

    void Init(HardwareSerial *serial = &Serial2,
              unsigned long baud     = CONFIG_MODE_BAUD,
              uint32_t config = SERIAL_8N1, uint8_t RX = 16, uint8_t TX = 17);

    /**
     * @brief E220-900T22S(JP)へLoRa初期設定を行う
     * @param config LoRa設定値の格納先
     * @return 0:成功 1:失敗
     */
    int InitLoRaSetting(struct LoRaConfigItem_t &config);

    /**
     * @brief LoRa受信を行う
     * @param recv_data LoRa受信データの格納先
     * @return 0:成功 1:失敗
     */
    int RecieveFrame(struct RecvFrame_t *recv_frame);

    /**
     * @brief LoRa送信を行う
     * @param config LoRa設定値の格納先
     * @param send_data 送信データ
     * @param size 送信データサイズ
     * @return 0:成功 1:失敗
     */
    int SendFrame(struct LoRaConfigItem_t &config, uint8_t *send_data,
                  int size);

    void SetDefaultConfigValue(struct LoRaConfigItem_t &config);

   private:
    HardwareSerial *_serial;
};

#endif