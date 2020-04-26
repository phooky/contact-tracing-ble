#pragma once

#include <cstdint>

typedef struct {
    // flags section
    uint8_t flags_len;
    uint8_t flags_type;
    uint8_t flags_data;
    // uuid section
    uint8_t uuid_len;
    uint8_t uuid_type;
    uint16_t uuid_uuid;
    // data section
    uint8_t data_len;
    uint8_t data_type;
    uint16_t data_uuid;
    uint8_t data_rpi[16];
    uint8_t data_aem[4];
    int8_t data_txpower;
    uint8_t data_reserved[2];
} __attribute__ ((packed)) EN_packet;
