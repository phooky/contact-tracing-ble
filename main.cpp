#include "ct_beacon.h"
#include "ct_crypto.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <atomic>
#include <signal.h>
#include <getopt.h>

std::atomic<bool> no_sig = true;

static void on_signal(int s) {
    no_sig = false;
}

void usage(char* const path, std::ostream& output) {
    output << "Usage: " << path << " [OPTION]" << std::endl;
    output << "  -v           verbose mode" << std::endl;
    output << "  -lLOGBASE    base of logfile path" << std::endl;
}

class LogBuilder {
    const std::string base;
    std::ofstream out;
    const bool is_cout;
    public:
    LogBuilder(const std::string& logbase, const uint32_t dayNumber) : base(logbase), is_cout(logbase == "-") {
        update(dayNumber);
    }
    ~LogBuilder() {
        if (out.is_open()) out.close();
    }

    void update(const uint32_t dayNumber) {
        if (!is_cout) {
            if (out.is_open()) out.close();
            std::stringstream ss(base);
            ss << dayNumber << ".log";
            out.open(ss.str(), std::ofstream::out | std::ofstream::binary | std::ofstream::app);
        }
    }
    std::ostream& ostream() {
        if (is_cout) return std::cout;
        return out;
    }

};

int main(int argc, char* const argv[]) {
    bool verbose = false;
    std::string logbase = "ct_log-";
    while (true) {
        auto opt = getopt(argc,argv,"l:v");
        if (opt == -1) break;
        else if (opt == 'v') { verbose = true; }
        else if (opt == 'l') { logbase = optarg; }
        else {
            usage(argv[0],std::cout);
            return -1;
        }
    }
    struct sigaction sig_action = {};
    sig_action.sa_flags = SA_NOCLDSTOP;
    sig_action.sa_handler = on_signal;
    sigaction(SIGINT, &sig_action, NULL);

    TracingKey tk("test.key");
    CT_Beacon beacon;
    beacon.reset();
    auto [ day, time ] = getDayAndTimeInterval();
    auto dtk = tk.daily_tracing_key(day);
    std::cerr << "Begin advertising." << std::endl;
    beacon.start_advertising(make_rpi(dtk, time));
    std::cerr << "Begin listening." << std::endl;
    beacon.start_listening();
    LogBuilder log(logbase,day);
    while (no_sig) {
        auto [ cday, ctime ] = getDayAndTimeInterval();
        if (cday != day) {
            dtk = tk.daily_tracing_key(cday);
            log.update(day);
        }
        if (cday != day || ctime != ctime) { 
            beacon.start_advertising(make_rpi(dtk, ctime));
            day = cday;
            time = ctime;
        }
        beacon.log_to_stream(log.ostream(), 10000);
    }
    std::cerr << "End listening." << std::endl;
    beacon.stop_listening();
    std::cerr << "End advertising." << std::endl;
    beacon.stop_advertising();
    return 0;
}
