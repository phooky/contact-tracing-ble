#include <time.h>
#include <tuple>
#include <gcrypt.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <endian.h>
#include <vector>
#include "ct_crypto.h"



uint32_t getENIntervalNumber() {
    time_t t = time(NULL);
    uint32_t intervalNumber = t / (60*10);
    return intervalNumber;
}

class HMAC {
    gcry_mac_hd_t hd;
public:
    HMAC(const uint8_t* key, size_t keylen) {
        gcry_mac_open(&hd, GCRY_MAC_HMAC_SHA256, 0, NULL);
        gcry_mac_setkey(hd, key, keylen);
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

    void read(uint8_t* buf, size_t sz) {
        size_t rsz = sz;
        gcry_mac_read(hd, buf, &rsz);
        if (sz != rsz) throw std::runtime_error("Unexpected SHA-256 HMAC size");
    }
};

 void HKDF(const uint8_t* key, size_t keylen,
        const uint8_t* salt, size_t saltlen,
        const uint8_t* info, size_t infolen,
        uint8_t* buf, size_t buflen) {
    // Step 1: extract
    HMAC extract_mac(salt,saltlen);
    extract_mac.write(key,keylen);
    uint8_t prk[32];
    extract_mac.read(prk,32);

    // Step 2: expand. We only need 16 octets, so this is an
    // incomplete implementation.
    HMAC expand_mac(prk,32);
    expand_mac.write(info,infolen);
    uint8_t end_octet = 0x01;
    expand_mac.write(&end_octet, 1);
    return expand_mac.read(buf,16);
}

TemporaryExposureKey::TemporaryExposureKey(const::std::string& prefix) {
    // Validation period
    uint32_t cur_interval = getENIntervalNumber();
    valid_from = (cur_interval / EKRollingPeriod) * EKRollingPeriod;
    // Load or generate key
    std::stringstream ss(prefix);
    ss << valid_from << ".tek";
    std::ifstream inf(ss.str());
    if (inf) {
        inf.read((char*)key, 16);
        inf.close();
    } else {
        gcry_randomize(key, 16, GCRY_VERY_STRONG_RANDOM);
        std::ofstream outf(ss.str(),std::ofstream::out|std::ofstream::binary);
        outf.write((const char*)key, 16);
        outf.close();
    }
    // Generate RPI key
    HKDF(key, 16, NULL, 0, (const uint8_t*)"EN-RPIK", 7, rpi_key, 16);
    // Generate AEM key
    HKDF(key, 16, NULL, 0, (const uint8_t*)"CT-AEMK", 7, aem_key, 16);
}

bool TemporaryExposureKey::is_still_valid() {
    uint32_t cur_interval = getENIntervalNumber();
    return cur_interval < (valid_from + EKRollingPeriod);
}

std::vector<uint8_t> TemporaryExposureKey::make_rpi(uint32_t intervalNumber) {
    uint8_t inblock[16] = { 'E','N','-','R','P','I' };
    *(uint32_t*)(inblock+12) = htole32(intervalNumber);
    gcry_cipher_hd_t handle;
    gcry_cipher_open(&handle, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(handle,rpi_key,16);
    gcry_cipher_encrypt(handle, inblock, 16, NULL, 0);
    gcry_cipher_close(handle);
    return std::vector<uint8_t>(inblock, inblock+16);
}

std::vector<uint8_t> TemporaryExposureKey::encrypt_aem(const std::vector<uint8_t>& rpi, const std::vector<uint8_t>& metadata) {
    gcry_cipher_hd_t handle;
    gcry_cipher_open(&handle, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CTR, 0);
    gcry_cipher_setkey(handle,aem_key,16);
    gcry_cipher_setiv(handle,rpi.data(),rpi.size());
    std::vector<uint8_t> out(metadata.size());
    gcry_cipher_encrypt(handle, out.data(), out.size(), metadata.data(), metadata.size());
    gcry_cipher_close(handle);
    return out;
}


