#pragma once

#include <cstdint>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <string>
#include <vector>
#include "log.h"

class CT_Beacon {
    int dev;
    void do_req(uint16_t ocf, void* cparam, int clen);
public:
    CT_Beacon(const std::string& device_name = "hci0");
    ~CT_Beacon();
    
    void reset();
    void start_advertising(const std::vector<uint8_t> &rpi, const std::vector<uint8_t> &aem);
    void stop_advertising();

    void start_listening();
    void stop_listening();

    int log(LogBuilder& log, int timeout_ms);
};

