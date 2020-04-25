#include <tuple>
#include <gcrypt.h>
#include <vector>

uint32_t getENIntervalNumber();

const uint32_t EKRollingPeriod = 144;

class TemporaryExposureKey {
    uint8_t key[16];
    uint8_t rpi_key[16];
    uint8_t aem_key[16];
    uint32_t valid_from;
    public:
    TemporaryExposureKey();
    bool is_still_valid();
    std::vector<uint8_t> make_rpi(uint32_t intervalNumber);
    std::vector<uint8_t> encrypt_aem(const std::vector<uint8_t>& rpi,
            const std::vector<uint8_t>& metadata);
};

