#!/bin/bash
DIR=$(dirname "$0")
sudo apt update
sudo apt install libxcb-cursor0

#Lanceur
LANCEUR="$HOME/.local/share/applications/PotalegerTest.desktop"
echo "[Desktop Entry]" > $LANCEUR
echo "Type=Application" >> $LANCEUR
echo "Name=PotalegerTest" >> $LANCEUR
echo "Comment=Gestion de cultures potagères" >> $LANCEUR
echo "Exec=$DIR/potaleger.sh" >> $LANCEUR
echo "Icon=$DIR/potaleger.svg" >> $LANCEUR
