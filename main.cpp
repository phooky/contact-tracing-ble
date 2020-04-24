#include "ct_beacon.h"
#include "ct_crypto.h"
#include <iostream>
#include <atomic>
#include <signal.h>

std::atomic<bool> no_sig = true;

static void on_signal(int s) {
    no_sig = false;
}

int main() {
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
    while (no_sig) {
        auto [ cday, ctime ] = getDayAndTimeInterval();
        if (cday != day) dtk = tk.daily_tracing_key(cday);
        if (cday != day || ctime != ctime) { 
            beacon.start_advertising(make_rpi(dtk, ctime));
            day = cday;
            time = ctime;
        }
        beacon.log_to_stream(std::cout, 10000);
    }
    std::cerr << "End listening." << std::endl;
    beacon.stop_listening();
    std::cerr << "End advertising." << std::endl;
    beacon.stop_advertising();
    return 0;
}
