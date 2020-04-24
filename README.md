BLE Contact Tracing daemon
==========================

This is a rough implementation of the jointly developed [Apple/Google bluetooth
contact tracing specification](https://www.apple.com/covid19/contacttracing). The
current implementation covers v1.0; there are changes in the v1.1 update that 
still need to be implemented.

This implementation interacts directly with the HCI; it is not intended to be run
in parallel with another bluetooth daemon. It's recommended that you stop the
bluetoothd process and bring up the hci device manually.

Building and running the contact tracing daemon
-----------------------------------------------

1. Build contact tracing daemon.
    1. Install dependencies. `sudo apt install git libbluetooth-dev libgcrypt-dev`
    2. Clone repository. `git clone https://github.com/phooky/contact-tracing-ble.git`
    3. Build ctd. `cd contact-tracing-ble && make`
2. Run the contact tracing daemon.
    1. Stop the existing bluetooth daemon. `sudo systemctl stop bluetooth`
    2. Bring up the hci0 interface `sudo hciconfig hci0 up`
    3. Start the BLE contact tracking daemon. `sudo ./ctd`

