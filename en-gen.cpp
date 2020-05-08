#include "crypto.h"
#include <iostream>
#include <getopt.h>
#include <time.h>
#include <string>
#include <cstdlib>
#include <iomanip>

void usage(char* const path, std::ostream& output) {
    output << "Usage: " << path << " [OPTION] TEK" << std::endl;
    output << "  -tTIME       unix epoch in seconds in which TEK was valid" << std::endl;
    output << "  -d           debug mode" << std::endl;
}

std::ostream& print_hex(std::ostream& o, uint8_t* data, size_t len = 16) {
    o << std::hex;
    for (auto i = 0; i < len; i++) o << (int)data[i];
    return o;
}

std::ostream& operator<<(std::ostream& o, const std::vector<uint8_t> v) {
    std::ios_base::fmtflags f(o.flags());
    for (auto b : v) {
        o << std::setfill('0') << std::setw(2) << std::hex;
        o << (int)b;
    }
    o.flags(f);
    return o;
} 

int main(int argc, char* const argv[]) {
    bool debug = false;
    time_t when = time(NULL);
    while (true) {
        auto opt = getopt(argc,argv,"dt:");
        if (opt == -1) break;
        else if (opt == 't') { when = strtol(optarg,NULL,10); }
        else if (opt == 'd') { debug = true; }
        else {
            usage(argv[0],std::cerr);
            return -1;
        }
    }
    if (optind >= argc) {
        std::cerr << "No TEK specified." << std::endl;
        usage(argv[0],std::cerr);
        return -1;
    }
    std::string TEKstr(argv[optind]);
    if (TEKstr.size() != 32) {
        std::cerr << "TEK must be exactly 16 octets long." << std::endl;
        return -1;
    }
    uint8_t key_data[16];
    for (auto i = 0; i < 16; i++) {
        key_data[i] = strtol(TEKstr.substr(i*2,2).c_str(), NULL, 16);
    }
    if (debug) {
        std::cout << "Parsed key as ";
        print_hex(std::cout, key_data) << std::endl;
    }
    TemporaryExposureKey tek(key_data, getENIntervalNumber(when, true));
    for (auto i = 0; i < 144; i++) {
        auto inum = tek.get_valid_from() + i;
        std::cout << "At " << std::dec << inum << " RPI " << tek.make_rpi(inum);
        std::cout << std::endl;
    }
}

