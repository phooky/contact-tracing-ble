#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <poll.h>

#include <iostream>
#include <sstream>
#include <unistd.h>

#include <sys/random.h>

#include "ct_beacon.h"
#include <cerrno>

// Hi. Guess who learned a lot about Bluetooth Low Energy advertising today?

const uint8_t FLAGS_TYPE = 0x01;
const uint8_t SERVICE_UUID16_TYPE = 0x03;
const uint8_t SERVICE_DATA16_TYPE = 0x16;

const uint8_t CT_FLAGS = 0x1A;
const uint16_t CT_SERVICE_UUID16 = 0xFD6F;

const uint8_t EN_MAJOR_VERSION = 0x01;
const uint8_t EN_MINOR_VERSION = 0x00;
const int8_t TX_POWER_DEFAULT = 0x10;
typedef struct {
    // flags section
    uint8_t flags_len;
    uint8_t flags_type;
    uint8_t flags_data;
    // uuid section
    uint8_t uuid_len;
    uint8_t uuid_type;
    uint16_t uuid_uuid;
    // data section
    uint8_t data_len;
    uint8_t data_type;
    uint16_t data_uuid;
    uint8_t data_rpi[16];
    uint8_t data_version;
    int8_t data_txpower;
    uint8_t data_reserved[2];
} __attribute__ ((packed)) EN_packet;

uint8_t build_ct_packet(uint8_t* packet_data, const std::vector<uint8_t>& rpi) {
    EN_packet* p = (EN_packet*)packet_data;
    // Flags section
    p->flags_len = 0x02;
    p->flags_type = FLAGS_TYPE;
    p->flags_data = CT_FLAGS;
    // UUID16 section
    p->uuid_len = 0x03;
    p->uuid_type = SERVICE_UUID16_TYPE;
    p->uuid_uuid = htobs(CT_SERVICE_UUID16);
    // Data section
    p->data_len = 0x13;
    p->data_type = SERVICE_DATA16_TYPE;
    p->data_uuid = htobs(CT_SERVICE_UUID16);
    for (auto i = 0; i < 16; i++) p->data_rpi[i] = rpi[i];
    p->data_version = (EN_MAJOR_VERSION << 6) | (EN_MINOR_VERSION << 4);
    p->data_txpower = TX_POWER_DEFAULT;
    p->data_reserved[0] = p->data_reserved[1] = 0;
    return sizeof(EN_packet);
}

// recommended advertising interval -- ~200-270 ms
const auto MIN_INTERVAL_MS = 200;
const auto MAX_INTERVAL_MS = 270;
const auto MS_PER_INTERVAL = 0.625;

CT_Beacon::CT_Beacon(const std::string& device_name) {
    int dev_id = hci_devid(device_name.c_str());
    if (dev_id < 0) throw std::runtime_error("failed hci_devid");
    dev = hci_open_dev(dev_id);
    if (dev < 0) throw std::runtime_error("failed to open hci device");
}

CT_Beacon::~CT_Beacon() {
    if (dev >= 0) hci_close_dev(dev);
}

void CT_Beacon::do_req(uint16_t ocf, void* cparam, int clen) {
    struct hci_request rq = {};
    rq.ogf = OGF_LE_CTL;
    rq.ocf = ocf;
    rq.cparam = cparam;
    rq.clen = clen;
    uint8_t status = 0;
    rq.rparam = &status;
    rq.rlen = 1;
    int ret = hci_send_req(dev, &rq, 1000);
    if (ret < 0) {
        throw std::runtime_error("Could not send HCI request");
    } else if (status != 0) {
        std::stringstream s;
        s << "HCI error during " << std::hex << (int)rq.ocf << ": " << (int)status;
        throw std::runtime_error(s.str());
    }
}

void CT_Beacon::reset() {}

bool bdaddr_invalid(const bdaddr_t& a) {
    bool zeros = true;
    bool ones = true;
    for (auto i = 0; i < 6; i++) {
        zeros = zeros && (a.b[i] == 0);
        ones = ones && (a.b[i] == (i==5)?0x3f:0xff);
    }
    return zeros || ones;
}

void CT_Beacon::start_advertising(const std::vector<uint8_t>& rpi) {
    //
    // Set random address (v4 sec E 7.8.52) */
    //
#ifdef DEBUG_ADDR
    le_set_random_address_cp randaddr_cp = { 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
#else
    le_set_random_address_cp randaddr_cp;
    getrandom( &randaddr_cp, 6, 0 );
    randaddr_cp.bdaddr.b[5] &= 0x3f; // non-resolvable private address
    // ensure that the address is not all 1s or 0s!
    if (bdaddr_invalid(randaddr_cp.bdaddr) ) randaddr_cp.bdaddr.b[0] = 0x55;
#endif
    do_req(OCF_LE_SET_RANDOM_ADDRESS, &randaddr_cp, LE_SET_RANDOM_ADDRESS_CP_SIZE);

    //
    // Set advertising parameters
    //
    le_set_advertising_parameters_cp adv_params_cp = {};
    adv_params_cp.min_interval = htobs(MIN_INTERVAL_MS/MS_PER_INTERVAL);
    adv_params_cp.max_interval = htobs(MAX_INTERVAL_MS/MS_PER_INTERVAL);
    adv_params_cp.advtype = 0x03; // ADV_NOCONN_IND
    adv_params_cp.own_bdaddr_type = 0x01; // Random device address
    adv_params_cp.direct_bdaddr_type = 0x01; // Random device address
    adv_params_cp.direct_bdaddr = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 }; // test address for today
    adv_params_cp.chan_map = 0x07; // All three channels in use

    do_req(OCF_LE_SET_ADVERTISING_PARAMETERS, &adv_params_cp, LE_SET_ADVERTISING_PARAMETERS_CP_SIZE);

    //
    // Enable advertising
    // (If advertising is already enabled this will still force the random address
    // to update)
    //
    le_set_advertise_enable_cp advertise_cp = {};
    advertise_cp.enable = 0x01;
    do_req(OCF_LE_SET_ADVERTISE_ENABLE, &advertise_cp, LE_SET_ADVERTISE_ENABLE_CP_SIZE);

    //
    // Set advertising data
    //
    le_set_advertising_data_cp adv_data_cp = {};
    adv_data_cp.length = build_ct_packet(adv_data_cp.data,rpi);
    do_req(OCF_LE_SET_ADVERTISING_DATA, &adv_data_cp, LE_SET_ADVERTISING_DATA_CP_SIZE);
}

void CT_Beacon::stop_advertising() {

    le_set_advertise_enable_cp advertise_cp = {};

    struct hci_request rq = {};
    do_req(OCF_LE_SET_ADVERTISE_ENABLE, &advertise_cp, LE_SET_ADVERTISE_ENABLE_CP_SIZE);
}

void CT_Beacon::start_listening() {
    // disable scanning
    try {
        stop_listening();
    } catch (std::runtime_error& e) {
        std::cerr << "Command disallowed on initial scan disable." << std::endl;
    }
	if (hci_le_set_scan_enable(dev, 0x00, 0x01, 1000) < 0) {
        //throw std::runtime_error("Could not disable LE scan.");
    }

    // set filter
    struct hci_filter filter;
    hci_filter_clear(&filter);
    hci_filter_set_ptype(HCI_EVENT_PKT, &filter);
    hci_filter_set_event(EVT_LE_META_EVENT, &filter);
    if (setsockopt(dev, SOL_HCI, HCI_FILTER, &filter, sizeof(filter)) < 0)
        throw std::runtime_error("Could not set filter on socket.");

    // set scan parameters
    // scan type = 0 (passive, no PDUs sent)
    // interval = 0x40 (40ms)
    // window = 0x30 (30ms)
    // own_address = random (0x1)
    // scanning filter policy = 0 (everything not directed to another device)
    //
	if (hci_le_set_scan_parameters(dev, 0x00, htobs(0x40), htobs(0x30),
            0x01, 0x00, 1000) < 0) 
        throw std::runtime_error("Could not set LE scan parameters.");
    // enable scanning with duplicate filtering enabled
    le_set_scan_enable_cp scan_cp = {};
    scan_cp.enable = 0x01;
    scan_cp.filter_dup = 0x01;
    do_req(OCF_LE_SET_SCAN_ENABLE, &scan_cp, LE_SET_SCAN_ENABLE_CP_SIZE);
}

void CT_Beacon::stop_listening() {
    le_set_scan_enable_cp scan_cp = {};
    do_req(OCF_LE_SET_SCAN_ENABLE, &scan_cp, LE_SET_SCAN_ENABLE_CP_SIZE);
}


int CT_Beacon::log_to_stream(std::ostream& out, int timeout_ms) {
    struct pollfd fds = { dev, POLLIN, 0 };
    int rv = poll(&fds, 1, timeout_ms); 
    if (rv < 0) {
        if (errno != EINTR) throw new std::runtime_error("Error during poll.");
    }
    if (rv > 0) {
        uint8_t buf[HCI_MAX_EVENT_SIZE];
        ssize_t len = read(dev, buf, HCI_MAX_EVENT_SIZE);
        evt_le_meta_event* mevt = (evt_le_meta_event*)(buf + 1 + HCI_EVENT_HDR_SIZE);
        if (mevt->subevent == 0x02) { // advertising report
            le_advertising_info* ad = (le_advertising_info*)(mevt->data + 1);
            len -= (uint8_t*)ad - buf;
            len = std::min(len - 1, (ssize_t)ad->length);
            auto start = ad->data;
            auto end = start + len;
            while (start <= end) {
                uint8_t sz = start[0];
                if (sz < 1) return 0; // malformed
                if (start + sz > end) return 0; // OOB
                uint8_t type = start[1];
                if (type == SERVICE_UUID16_TYPE) {
                    if (sz < 3) return 0;
                    // check for correct service UUID
                    if (*(uint16_t*)(start+2) != htobs(CT_SERVICE_UUID16)) return 0;
                    std::cerr << "Correct service UUID" << std::endl;
                } else if (type == SERVICE_DATA16_TYPE) {
                    if (sz < 0x13) return 0;
                    // check for correct UUID
                    if (*(uint16_t*)(start+2) != htobs(CT_SERVICE_UUID16)) return 0;
                    auto rpi = start+4;
                    auto t = time(NULL);
                    out.write((const char*)&t, sizeof(time_t));
                    out.write((const char*)(start+4),0x10);
                    std::cerr << "At epoch " << t << " got RPI " << std::hex << rpi[0];
                    for (auto i = 1; i < 0x10; i++) std::cerr << ":" << rpi[i];
                    std::cerr << std::endl;
                }
                start += sz + 1;
            }
        }
        return 1;
    }
    return 0;
}

int test_beacon_main() {
    std::vector<uint8_t> rpi(16);
    for (auto i = 0; i < 16; i++) rpi[i] = i;
    CT_Beacon beacon;
    beacon.start_advertising(rpi);
    std::cout << "Advertising started..." << std::flush;
    std::cin.get();
    beacon.stop_advertising();
    std::cout << "advertising stopped." << std::endl;
    return 0;
}

