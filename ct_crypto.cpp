#include <time.h>
#include <tuple>
#include <gcrypt.h>
#include <fstream>
#include <iostream>

std::tuple<uint32_t,uint8_t> getDayAndTimeInterval() {
    time_t t = time(NULL);
    uint32_t dayNumber = t / (60L * 60L * 24L);
    uint32_t secsIntoDay = t - (dayNumber * 60L * 60L * 24L);
    uint8_t timeIntervalNumber = secsIntoDay / (60*10);
    return std::make_tuple(dayNumber, timeIntervalNumber);
}

class TracingKey {
    public:
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
};

int main() {
    TracingKey tk("test.key");
    for (auto i = 0;i < tk.KEYLEN; i++) std::cout << tk.key[i];
}
    

