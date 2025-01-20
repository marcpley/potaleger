#!/bin/bash
DIR=$(dirname "$0")
sudo apt update
sudo apt install libxcb-cursor0
cp ./bin/potaleger.sh ./
