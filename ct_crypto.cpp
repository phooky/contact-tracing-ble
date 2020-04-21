#include <time.h>
#include <tuple>
#include <gcrypt.h>
#include <fstream>
#include <iostream>
#include <endian.h>
#include <vector>
#include "ct_crypto.h"

std::tuple<uint32_t,uint8_t> getDayAndTimeInterval() {
    time_t t = time(NULL);
    uint32_t dayNumber = t / (60L * 60L * 24L);
    uint32_t secsIntoDay = t - (dayNumber * 60L * 60L * 24L);
    uint8_t timeIntervalNumber = secsIntoDay / (60*10);
    return std::make_tuple(dayNumber, timeIntervalNumber);
}

class HMAC {
    gcry_mac_hd_t hd;
public:
    HMAC(const std::vector<uint8_t>& key) {
        gcry_mac_open(&hd, GCRY_MAC_HMAC_SHA256, 0, NULL);
        gcry_mac_setkey(hd, key.data(), key.size());
    }
    ~HMAC() {
        gcry_mac_close(hd);
    }

    void write(const std::vector<uint8_t>& v) {
        gcry_mac_write(hd, v.data(), v.size());
    }

    void write(const std::string& s) {
        gcry_mac_write(hd, s.data(), s.size());
    }

    void write(const uint8_t* d, const size_t len) {
        gcry_mac_write(hd, d, len);
    }

    std::vector<uint8_t> read(size_t sz) {
        std::vector<uint8_t> v(sz);
        gcry_mac_read(hd, v.data(), &sz);
        if (sz != v.size()) throw std::runtime_error("Unexpected SHA-256 HMAC size");
        return v;
    }
};

TracingKey::TracingKey(const std::string& path) : std::vector<uint8_t>(KEYLEN) {
    std::ifstream f(path);
    if (f) { 
        f.read((char*)data(), size());
        f.close();
    } else {
        gcry_randomize(data(), size(), GCRY_VERY_STRONG_RANDOM);
        // store to path
        std::ofstream f(path);
        f.write((char*)data(), size());
        f.close();
    }
}

std::vector<uint8_t> TracingKey::daily_tracing_key(uint32_t dayNumber) {
    // HKDF (tk , NULL , ( UTF8("CT-DTK") || Di ),16)

    // Step 1: extract
    HMAC extract_mac(std::vector<uint8_t>(32,0));
    extract_mac.write(*this);
    auto prk = extract_mac.read(32);

    // Step 2: expand. We only need 16 octets, so this is trivial.
    HMAC expand_mac(prk);
    expand_mac.write(std::string("CT-DTK"));
    uint32_t dn_le = htole32(dayNumber);
    expand_mac.write((uint8_t*)&dn_le, 4);
    uint8_t end_octet = 0x01;
    expand_mac.write(&end_octet, 1);
    return expand_mac.read(16);
}

std::vector<uint8_t> make_rpi(const std::vector<uint8_t>& dtk, uint8_t timeIntervalNumber) {
    HMAC mac(dtk);
    mac.write(std::string("CT-RPI"));
    mac.write(&timeIntervalNumber,1);
    return mac.read(16);
}

// most of this is cacheable
std::vector<uint8_t> getRPI() {
    TracingKey tk("test.key");
    auto [ day, time ] = getDayAndTimeInterval();
    auto dtk = tk.daily_tracing_key(day);
    return make_rpi(dtk, time);
}

int main_test_crypto() {
    TracingKey tk("test.key");
    auto [ day, time ] = getDayAndTimeInterval();
    auto dtk = tk.daily_tracing_key(day);
    auto rpi = make_rpi(dtk, time);
    for (auto i = 0;i < 16; i++) std::cout << rpi[i];
    return 0;
}
    

