#!/bin/sh

# Android-Entwicklungsumgebung kopieren
# Version 0.6 - 2016/02/28, Klaus Tockloth
#
# Verzeichnisse mit Bewegungsdaten werden nicht kopiert:
# - install
# - source
# - work
# - tmp
# - land-polygons-split-4326

# set -o xtrace

if [ $# -ne 2 ]; then
  echo " "
  echo "Benutzung : $0  <Quellverzeichnis>         <Zielverzeichnis>"
  echo "Beispiel  : $0  Freizeitkarte-Entwicklung  Freizeitkarte-Entwicklung-Neu"
  echo " "
  exit 1
fi

# set -o verbose

SOURCEDIR=$1
DESTDIR=$2

mkdir $DESTDIR
cp    $SOURCEDIR/*               $DESTDIR
cp -r $SOURCEDIR/tools           $DESTDIR
cp -r $SOURCEDIR/routes          $DESTDIR
cp -r $SOURCEDIR/poly            $DESTDIR
cp -r $SOURCEDIR/transform       $DESTDIR
cp -r $SOURCEDIR/theme           $DESTDIR
cp -r $SOURCEDIR/v4-to-locus     $DESTDIR

zip -r $DESTDIR_$(date +"%F").zip $DESTDIR
