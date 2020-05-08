#pragma once

#include <tuple>
#include <gcrypt.h>
#include <vector>
#include <string>
#include <time.h>

uint32_t getENIntervalNumber(time_t time = time(NULL), bool align = false);

const uint32_t TEKRollingPeriod = 144;

class TemporaryExposureKey {
    uint8_t key[16];
    uint8_t rpi_key[16];
    uint8_t aem_key[16];
    uint32_t valid_from;
    public:
    TemporaryExposureKey(const std::string& prefix = "EN-");
    TemporaryExposureKey(const uint8_t* key_data, uint32_t interval);
    bool is_still_valid();
    uint32_t get_valid_from() { return valid_from; }
    std::vector<uint8_t> make_rpi(uint32_t intervalNumber);
    std::vector<uint8_t> encrypt_aem(const std::vector<uint8_t>& rpi,
            const std::vector<uint8_t>& metadata);
    private:
    void generate_keys();
};

