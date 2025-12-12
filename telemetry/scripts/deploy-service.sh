#!/bin/bash

sudo systemctl stop tel-server.service

sudo cp build/bin/tel-server /usr/local/bin/tel-server
sudo cp build/bin/tel-cli /usr/bin/tel-cli
sudo cp src/tel-server/systemd/tel-server.service /lib/systemd/system/tel-server.service

sudo systemctl daemon-reload
sudo systemctl enable tel-server.service
sudo systemctl start tel-server.service
