#!/bin/sh
cp -r base-feeds.conf /etc/opkg/base-feeds.conf
sleep 1
opkg update
sleep 10
opkg install libusb-0.1-dev
exit 0
