#!/bin/sh

# Android-Transformations-Compiler uebersetzen
# Version 1.0 - 2015/02/06, Klaus Tockloth

# set -o xtrace
# set -o verbose

echo " "
echo "Transformations-Compiler uebersetzen"
echo "------------------------------------"
echo " "

kommando="perl mta.pl -language=de comp_filter DUMMY"
echo "$kommando"
$kommando

kommando="perl mta.pl -language=en comp_filter DUMMY"
echo "$kommando"
$kommando

kommando="perl mta.pl -language=fr comp_filter DUMMY"
echo "$kommando"
$kommando

kommando="perl mta.pl -language=it comp_filter DUMMY"
echo "$kommando"
$kommando

kommando="perl mta.pl -language=nl comp_filter DUMMY"
echo "$kommando"
$kommando

kommando="perl mta.pl -language=pl comp_filter DUMMY"
echo "$kommando"
$kommando

kommando="perl mta.pl -language=ru comp_filter DUMMY"
echo "$kommando"
$kommando

kommando="perl mta.pl -language=pt comp_filter DUMMY"
echo "$kommando"
$kommando
