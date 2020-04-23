Setting up contact tracing on the Raspberry Pi
==============================================

1. Install Raspbian.
    1. Burn Raspbian Lite onto an SD card.
    2. Mount boot partition.
    3. Touch `boot/ssh` to enable sshd.
    4. Set up `boot/wpa_supplicant.conf` to prepare wireless networking.
2. Boot and configure Pi.
    1. Power up Pi. The initial boot may take a couple of minutes.
    2. `ssh pi@raspberry.localdomain`, pw raspberry.
    3. Immediately change password with `passwd` command.
    4. `sudo raspi-config` to configure the Pi.
        1. In "Interfacing Options", select "SPI" and enable the SPI interface.
        2. In "Networking Options", change the hostname.
        3. [Optional] In "Advanced Options", expand the filesystem to fill the entire SD card.
    5. `sudo apt update` and `sudo apt upgrade`, then `sudo reboot`.
3. Build contact tracing daemon.
    1. Install dependencies. `sudo apt install git libbluetooth-dev libgcrypt-dev`
    2. Clone repository. `git clone https://github.com/phooky/contact-tracing-ble.git`
    3. Build ctd. `cd contact-tracing-ble && make`
4. Run the contact tracing daemon with `sudo ./ctd`


