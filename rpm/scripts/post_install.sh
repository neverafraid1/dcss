#!/bin/sh

DCSS_HOME_DIR=/opt/dcss

DCSS_DATA_DIR=/shared/dcss
DCSS_SOCKETS_DIR=$DCSS_DATA_DIR/socket
DCSS_LOGGING_DIR=$DCSS_DATA_DIR/log
DCSS_PID_DIR=$DCSS_DATA_DIR/pid
DCSS_RUNTIME_DIR=$DCSS_DATA_DIR/runtime
DCSS_UNIT_DIR=$DCSS_DATA_DIR/unit

mkdir -p $DCSS_SOCKETS_DIR
mkdir -p $DCSS_LOGGING_DIR
mkdir -p $DCSS_LOGGING_DIR/strategy
mkdir -p $DCSS_LOGGING_DIR/supervisor

mkdir -p $DCSS_PID_DIR

mkdir -p $DCSS_RUNTIME_DIR

mkdir -p $DCSS_UNIT_DIR
mkdir -p $DCSS_UNIT_DIR/MD
mkdir -p $DCSS_UNIT_DIR/MD/BINA
mkdir -p $DCSS_UNIT_DIR/MD/OKEX
mkdir -p $DCSS_UNIT_DIR/strategy
mkdir -p $DCSS_UNIT_DIR/system

chmod +x /opt/dcss/master/bin/*
ln -sf /opt/dcss/master/bin/engine /usr/bin/engine
ln -sf /opt/dcss/master/bin/dcsstg /usr/bin/dcsstg
ln -sf /opt/dcss/master/bin/dcssmg /usr/bin/dcssmg
ln -sf /opt/dcss/master/bin/dcssctl /usr/bin/dcssctl

ln -sf /opt/dcss/master/etc/systemd/user/dcss.service /etc/systemd/system/dcss.service


systemctl daemon-reload
systemctl start dcss