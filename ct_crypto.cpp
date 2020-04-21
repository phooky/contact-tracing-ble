#include <time.h>
#include <tuple>
#include <gcrypt.h>
#include <fstream>
#include <iostream>
#include <endian.h>
#include <vector>

std::tuple<uint32_t,uint8_t> getDayAndTimeInterval() {
    time_t t = time(NULL);
    uint32_t dayNumber = t / (60L * 60L * 24L);
    uint32_t secsIntoDay = t - (dayNumber * 60L * 60L * 24L);
    uint8_t timeIntervalNumber = secsIntoDay / (60*10);
    return std::make_tuple(dayNumber, timeIntervalNumber);
}

void hmac(uint8_t* key, size_t keylen, uint8_t* msg, size_t msglen, uint8_t* buffer, size_t buflen) {
    gcry_mac_hd_t hd;
    gcry_mac_open(&hd, GCRY_MAC_HMAC_SHA256, 0, NULL);
    gcry_mac_setkey(hd, key, keylen);
    gcry_mac_write(hd, msg, msglen);
    gcry_mac_read(hd,buffer,&buflen);
    if (buflen != 32) throw std::runtime_error("Unexpected SHA-256 HMAC size");
    gcry_mac_close(hd);
}



struct DailyTracingKey;
struct RollingProximityIdentifier;

struct TracingKey {
    const static size_t KEYLEN = 32;
    uint8_t key[KEYLEN];

    TracingKey(const std::string& path) {
        std::ifstream f(path);
        if (f) { 
            f.read((char*)key, KEYLEN);
            f.close();
        } else {
            gcry_randomize(key, KEYLEN, GCRY_VERY_STRONG_RANDOM);
            // store to path
            std::ofstream f(path);
            f.write((char*)key, KEYLEN);
            f.close();
        }
    }
    
    DailyTracingKey daily_tracing_key(uint32_t dayNumber);
};

struct DailyTracingKey {
    const static size_t KEYLEN = 16;
    uint8_t key[KEYLEN];

    RollingProximityIdentifier make_rpi(uint8_t timeIntervalNumber);
};

struct RollingProximityIdentifier {
    const static size_t RPILEN = 16;
    uint8_t rpi[RPILEN];
};

DailyTracingKey TracingKey::daily_tracing_key(uint32_t dayNumber) {
    // HKDF (tk , NULL , ( UTF8("CT-DTK") || Di ),16)
    DailyTracingKey dtk;

    // Step 1: extract
    uint8_t prk[32];
    uint8_t empty_salt[32] = {};
    gcry_mac_hd_t hd;
    gcry_mac_open(&hd, GCRY_MAC_HMAC_SHA256, 0, NULL);
    gcry_mac_setkey(hd, empty_salt, 32); // IKM is the message, not the key
    gcry_mac_write(hd, key, 32);
    size_t buflen = 32;
    gcry_mac_read(hd,prk,&buflen);
    if (buflen != 32) throw std::runtime_error("Unexpected SHA-256 HMAC size");
    gcry_mac_close(hd);

    // Step 2: expand. We only need 16 octets, so this is trivial.
    gcry_mac_open(&hd, GCRY_MAC_HMAC_SHA256, 0, NULL);
    gcry_mac_setkey(hd, prk, 32); 
    gcry_mac_write(hd, "CT-DTK", 6);
    uint32_t dn_le = htole32(dayNumber);
    gcry_mac_write(hd, (uint8_t*)&dn_le, 4);
    uint8_t end_octet = 0x01;
    gcry_mac_write(hd, &end_octet, 1);
    buflen = 32;
    uint8_t buffer[32];
    gcry_mac_read(hd,buffer,&buflen);
    if (buflen != 32) throw std::runtime_error("Unexpected SHA-256 HMAC size");
    gcry_mac_close(hd);
    for (auto i = 0; i < 16; i++) dtk.key[i] = buffer[i];

    return dtk;
}

RollingProximityIdentifier DailyTracingKey::make_rpi(uint8_t timeIntervalNumber) {
    RollingProximityIdentifier rpi;
    gcry_mac_hd_t hd;
    gcry_mac_open(&hd, GCRY_MAC_HMAC_SHA256, 0, NULL);
    gcry_mac_setkey(hd, key, KEYLEN); 
    gcry_mac_write(hd, "CT-RPI", 6);
    gcry_mac_write(hd, &timeIntervalNumber, 1);
    size_t buflen = 32;
    uint8_t buffer[32];
    gcry_mac_read(hd,buffer,&buflen);
    if (buflen != 32) throw std::runtime_error("Unexpected SHA-256 HMAC size");
    gcry_mac_close(hd);
    for (auto i = 0; i < 16; i++) rpi.rpi[i] = buffer[i];

    return rpi;
}


int main() {
    TracingKey tk("test.key");
    for (auto i = 0;i < tk.KEYLEN; i++) std::cout << tk.key[i];
}
    

