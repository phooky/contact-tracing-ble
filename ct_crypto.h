#include <tuple>
#include <gcrypt.h>
#include <vector>
#include <string>

uint32_t getENIntervalNumber();

const uint32_t EKRollingPeriod = 144;

class TemporaryExposureKey {
    uint8_t key[16];
    uint8_t rpi_key[16];
    uint8_t aem_key[16];
    uint32_t valid_from;
    public:
    TemporaryExposureKey(const::std::string& prefix = "EN-");
    bool is_still_valid();
    uint32_t get_valid_from() { return valid_from; }
    std::vector<uint8_t> make_rpi(uint32_t intervalNumber);
    std::vector<uint8_t> encrypt_aem(const std::vector<uint8_t>& rpi,
            const std::vector<uint8_t>& metadata);
};

