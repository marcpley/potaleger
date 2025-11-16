#!/bin/bash
DIR=$(dirname "$0")
echo $1 | sudo -S apt-get update
echo $1 | sudo -S apt-get install libxcb-cursor0

#Lanceur dans menu.
LANCEUR="$HOME/.local/share/applications/Potaleger.desktop"
echo "[Desktop Entry]" > $LANCEUR
echo "Type=Application" >> $LANCEUR
echo "Name=Potaleger" >> $LANCEUR
echo "Comment=Gestion de cultures potagères" >> $LANCEUR
echo "Exec=$DIR/potaleger.sh" >> $LANCEUR
echo "Icon=$DIR/potaleger.svg" >> $LANCEUR

#Lanceur dans dossier install.
LANCEUR2="$DIR/../Potaleger.desktop"
echo "[Desktop Entry]" > $LANCEUR2
echo "Type=Application" >> $LANCEUR2
echo "Name=Potaleger" >> $LANCEUR2
echo "Comment=Gestion de cultures potagères" >> $LANCEUR2
echo "Exec=$DIR/potaleger.sh" >> $LANCEUR2
echo "Icon=$DIR/potaleger.svg" >> $LANCEUR2

chmod +x "$LANCEUR2"
