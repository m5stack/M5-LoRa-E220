#ifndef __M5_LORA_E220_CONFIG_DEF_H__
#define __M5_LORA_E220_CONFIG_DEF_H__

#include <Arduino.h>

typedef enum {
    BAUD_1200   = 0b000,
    BAUD_2400   = 0b001,
    BAUD_4800   = 0b010,
    BAUD_9600   = 0b011,
    BAUD_19200  = 0b100,
    BAUD_38400  = 0b101,
    BAUD_57600  = 0b110,
    BAUD_115200 = 0b111
} baud_rate_val_t;

typedef enum {
    UART_8N1 = 0b00,
    UART_8O1 = 0b01,
    UART_8E1 = 0b10
} uart_config_val_t;

typedef enum {
    DATA_RATE_2_4Kbps  = 0b000,
    DATA_RATE_4_8Kbps  = 0b011,
    DATA_RATE_9_6Kbps  = 0b100,
    DATA_RATE_19_2Kbps = 0b101,
    DATA_RATE_38_4Kbps = 0b110,
    DATA_RATE_62_5Kbps = 0b111
} datarate_val_t;

typedef enum {
    SUBPACKET_200_BYTE = 0b00,
    SUBPACKET_128_BYTE = 0b01,
    SUBPACKET_64_BYTE  = 0b10,
    SUBPACKET_32_BYTE  = 0b11
} subpacket_size_val_t;

typedef enum {
    RSSI_AMBIENT_NOISE_ENABLE  = 0b1,
    RSSI_AMBIENT_NOISE_DISABLE = 0b0
} rssi_ambient_noise_flag_t;

typedef enum {
    TX_POWER_22dBm = 0b00,
    TX_POWER_17dBm = 0b01,
    TX_POWER_13dBm = 0b10,
    TX_POWER_10dBm = 0b11
} transmitting_power_val_t;

typedef enum {
    RSSI_BYTE_ENABLE  = 0b1,
    RSSI_BYTE_DISABLE = 0b0
} rssi_byte_flag_t;

typedef enum {
    UART_TT_MODE  = 0b0,
    UART_P2P_MODE = 0b1
} transmission_method_type_t;

typedef enum { LBT_ENABLE = 0b1, LBT_DISABLE = 0b0 } blt_flag_t;

typedef enum {
    WOR_500MS  = 0b000,
    WOR_1000MS = 0b001,
    WOR_1500MS = 0b010,
    WOR_2000MS = 0b011,
    WOR_2500MS = 0b100,
    WOR_3000MS = 0b101,
    WOR_3500MS = 0b110,
    WOR_4000MS = 0b111
} wor_cycle_val_t;

#endif