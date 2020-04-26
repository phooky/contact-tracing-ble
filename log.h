#pragma once
#include <string>
#include <fstream>

class LogBuilder {
    const std::string base;
    std::ofstream out;
    bool debug;
    const bool is_cout;
    public:
    LogBuilder(const std::string& logbase, const uint32_t interval, bool debug = false);
    ~LogBuilder(); 
    void update(const uint32_t interval);
    void log_report(uint8_t* report_data, size_t sz);
    std::ostream& ostream(); 

};

