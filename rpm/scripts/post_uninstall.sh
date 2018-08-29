#!/bin/sh
export LC_ALL=C

rm -f /etc/systemd/system/dcss.service

systemctl daemon-reload

rm -f /usr/bin/engine
rm -f /usr/bin/dcsstg
rm -f /usr/bin/dcssmg

rm -rf /opt/dcss/master/lib