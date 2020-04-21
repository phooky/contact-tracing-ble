#include <cstdint>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <string>

class CT_Beacon {
    int dev;
    void do_req(struct hci_request& rq);
public:
    CT_Beacon(const std::string& device_name = "hci0");
    ~CT_Beacon();
    
    void reset();
    void start_advertising(const uint8_t (&rpi)[16]);
    void stop_advertising();
};

