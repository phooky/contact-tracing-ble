#include "ct_beacon.h"
#include "ct_crypto.h"
#include <iostream>

int main() {
    TracingKey tk("test.key");
    CT_Beacon beacon;
    beacon.reset();
    auto [ day, time ] = getDayAndTimeInterval();
    auto dtk = tk.daily_tracing_key(day);
    beacon.start_advertising(make_rpi(dtk, time));
    beacon.start_listening();
    while (true) {
        auto [ cday, ctime ] = getDayAndTimeInterval();
        if (cday != day) dtk = tk.daily_tracing_key(cday);
        if (cday != day || ctime != ctime) { 
            beacon.start_advertising(make_rpi(dtk, ctime));
            day = cday;
            time = ctime;
        }
        beacon.log_to_stream(std::cout, 10000);
    }
    beacon.stop_advertising();
    beacon.stop_listening();
    return 0;
}
