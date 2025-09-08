#!/usr/bin/perl
# -----------------------------------------
# Program : poly2bb.pl (aus .poly-Datei die Bounding-Box ermitteln)
# Author  : (C) Klaus Tockloth (freizeitkarte@googlemail.com)
# Version : 1.0 - 2014/10/09
# -----------------------------------------

use warnings;
use strict;

use Math::Polygon::Tree;
use POSIX;

if ($#ARGV + 1 != 1) {
    printf ("\nFehler: Keine .poly-Datei uebergeben.\n");
    exit (1);
}

my $bound = Math::Polygon::Tree->new ($ARGV[0]);
my $bbox = $bound->bbox();
my ($xmin, $ymin, $xmax, $ymax) = @$bbox;
my $xminr = floor($xmin*100)/100;
my $yminr = floor($ymin*100)/100;
my $xmaxr = ceil($xmax*100)/100;
my $ymaxr = ceil($ymax*100)/100;

printf ("\nBounding Box (BB) fuer %s\n", $ARGV[0]);
printf ("Latitude, Breitengrad  : bottom, south, y1, ymin, minlat = %s\n", $ymin);
printf ("Latitude, Breitengrad  : top,    north, y2, ymax, maxlat = %s\n", $ymax);
printf ("Longitude, Laengengrad : left,   west,  x1, xmin, minlon = %s\n", $xmin);
printf ("Longitude, Laengengrad : right,  east,  x2, xmax, maxlon = %s\n", $xmax);

# 'Karte', 'URL der Quelle', 'Language', 'minLat', 'minLon', 'maxLat', 'maxLon', 'Meer' 'startLat', 'startLon', 'startZoom'
printf ("'%s', '%s', '%s', '%s',  #\n", $ymin, $xmin, $ymax, $xmax);
# gerundet (nicht ganz perfekt)
printf ("'%s', '%s', '%s', '%s',  #\n", $yminr, $xminr, $ymaxr, $xmaxr);

exit (0); 