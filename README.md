AmbiLed
=======

Ambilight clone for arduino or raspberry pi

- Raspberry pi:

  · Enable SPI:
    Comment 'blacklist spi-bcm2708' in /etc/modprobe.d/raspi-blacklist.conf

  · libv4l2: error allocating conversion buffer
    add 'vm.overcommit_memory=1' in /etc/sysctl.conf
    or 'sysctl vm.overcommit_memory=1' & 'systctl vm.overcommit_memory=0'
