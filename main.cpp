#include "bt.h"
#include "crypto.h"
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
    output << "  -d           debug logs (human-readable, includes addr)" << std::endl;
}

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

    TemporaryExposureKey tek;
    CT_Beacon beacon;
    beacon.reset();
    std::cerr << "Begin advertising." << std::endl;
    auto interval = getENIntervalNumber();
    beacon.start_advertising(tek.make_rpi(interval));
    std::cerr << "Begin listening." << std::endl;
    beacon.start_listening();
    LogBuilder log(logbase,tek.get_valid_from());
    while (no_sig) {
        if (!tek.is_still_valid()) {
            tek = TemporaryExposureKey();
            log.update(tek.get_valid_from());
        }
        auto cur_interval = getENIntervalNumber();
        if (cur_interval != interval) {
            beacon.stop_advertising(); // advertising must halt before updating addr/params
            beacon.start_advertising(tek.make_rpi(interval));
            interval = cur_interval;
        }
        beacon.log(log, 10000);
    }
    std::cerr << "End listening." << std::endl;
    beacon.stop_listening();
    std::cerr << "End advertising." << std::endl;
    beacon.stop_advertising();
    return 0;
}
