BLE Exposure Notification daemon
==========================

This is a rough Linux implementation of the jointly developed [Apple/Google bluetooth
exposure notification specification](https://www.apple.com/covid19/contacttracing),
formerly known as contact tracing. The current implementation covers v1.1.

This implementation interacts directly with the HCI; it is not intended to be run
in parallel with another bluetooth daemon. It's recommended that you stop the
bluetoothd process and bring up the hci device manually.

Support for Bluetooth Low Energy in Linux is all over the place; for example, recent kernels
seem to break it on the Intel 9560 card in my laptop. My current target and testing environment
is the Raspberry Pi Zero W.

(For a quick guide to setting this code up on a Raspberry Pi, see the `RPi_setup.md` document.)

Building and running the exposure notification daemon
-----------------------------------------------

1. Build the daemon.
    1. Install dependencies. `sudo apt install git libbluetooth-dev libgcrypt-dev`
    2. Clone repository. `git clone https://github.com/phooky/contact-tracing-ble.git`
    3. Build ctd. `cd contact-tracing-ble && make`
2. Run the daemon.
    1. Stop the existing bluetooth daemon. `sudo systemctl stop bluetooth`
    2. Bring up the hci0 interface `sudo hciconfig hci0 up`
    3. Start the BLE exposure notification daemon. `sudo ./ctd`

