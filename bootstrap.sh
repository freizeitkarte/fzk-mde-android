#!/bin/sh

# Android-Entwicklungsumgebung aktualisieren
# Version 1.0 - 2014/10/11, Klaus Tockloth
# Version 1.1 - 2019/08/16, Klaus Tockloth, Download-URL angepaßt

# set -o xtrace
# set -o verbose

echo " "
echo "Loeschen evtl. vorhandener Altdaten"
echo "-----------------------------------"
echo " "
kommando="rm -r land-polygons-split-4326"
echo "$kommando"
$kommando

echo " "
echo "Download der Land-Polygone"
echo "--------------------------"
echo " "
kommando="curl --location --url "https://osmdata.openstreetmap.de/download/land-polygons-split-4326.zip" --output "./land-polygons-split-4326.zip""
echo "$kommando"
$kommando

echo " "
echo "Entpacken der Land-Polygone"
echo "---------------------------"
echo " "
kommando="unzip land-polygons-split-4326.zip"
echo "$kommando"
$kommando

echo " "
echo "Bereinigen temporaerer Dateien"
echo "------------------------------"
echo " "
kommando="rm land-polygons-split-4326.zip"
echo "$kommando"
$kommando
