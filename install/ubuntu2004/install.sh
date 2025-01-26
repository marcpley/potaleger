#!/bin/bash
DIR=$(dirname "$0")
echo $1 | sudo -S apt update
echo $1 | sudo -S apt install libxcb-cursor0

#Lanceur
LANCEUR="$HOME/.local/share/applications/Potaleger.desktop"
echo "[Desktop Entry]" > $LANCEUR
echo "Type=Application" >> $LANCEUR
echo "Name=Potaleger" >> $LANCEUR
echo "Comment=Gestion de cultures potagères" >> $LANCEUR
echo "Exec=$DIR/potaleger.sh" >> $LANCEUR
echo "Icon=$DIR/potaleger.svg" >> $LANCEUR
