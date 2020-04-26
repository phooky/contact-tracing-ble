#include "log.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include "adv_packet.h"
#include <iostream>
#include <sstream>

LogBuilder::LogBuilder(const std::string& logbase, const uint32_t interval, bool debug) : 
    base(logbase), debug(debug), is_cout(logbase == "-") {
    update(interval);
}
LogBuilder::~LogBuilder() {
    if (out.is_open()) out.close();
}

void LogBuilder::log_report(uint8_t* report_data, size_t sz) {
    std::ostream& o = ostream();
    le_advertising_info* ad = (le_advertising_info*)report_data;
    EN_packet* p = (EN_packet*)ad->data;
    if (debug) {
        o << std::dec << time(NULL) << ": got RPI " << std::hex << (int)p->data_rpi[0];
        for (auto i = 1; i < 0x10; i++) o << ":" << (int)p->data_rpi[i];
        if (sz == 31) {
            o << " and AEM " << (int)p->data_aem[0];
            for (auto i = 1; i < 4; i++) o << ":" << (int)p->data_aem[i];
        }
        o << std::endl;
    } else {
        o << time(NULL);
        for (auto i = 0; i < 0x10; i++) o << p->data_rpi[i];
        for (auto i = 0; i < 4; i++) o << (sz==31)?p->data_aem[i]:0;
    }
}

void LogBuilder::update(const uint32_t interval) {
    if (!is_cout) {
        if (out.is_open()) out.close();
        std::stringstream ss(base);
        ss << interval << debug?".dbg_log" : ".log";
        auto mode = std::ofstream::out | std::ofstream::app;
        if (!debug) mode |= std::ofstream::binary;
        out.open(ss.str(), mode);
    }
}

std::ostream& LogBuilder::ostream() {
    if (is_cout) return std::cout;
    return out;
}
