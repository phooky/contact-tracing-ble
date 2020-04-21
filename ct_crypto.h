#include <tuple>
#include <gcrypt.h>
#include <vector>

std::tuple<uint32_t,uint8_t> getDayAndTimeInterval();

struct TracingKey : public std::vector<uint8_t> {
    const static size_t KEYLEN = 32;
    TracingKey(const std::string& path);
    std::vector<uint8_t> daily_tracing_key(uint32_t dayNumber);
};

std::vector<uint8_t> make_rpi(const std::vector<uint8_t>& dtk, uint8_t timeIntervalNumber);

