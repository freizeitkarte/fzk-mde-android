#!/usr/bin/perl

# perl v5-to-locus.pl fzk-outdoor-contrast-v5.xml fzk-outdoor-contrast-locus.xml
# perl v5-to-locus.pl fzk-outdoor-soft-v5.xml fzk-outdoor-soft-locus.xml
# perl v5-to-locus.pl freizeitkarte-v5.xml freizeitkarte-locus.xml

# ---------------------------------------
# Program : v5-to-locus.pl
# Version : 1.0 - Initial Version
# Version : 1.1 - all_scale 3 > 6
# Version : 1.2 - dy scale w/o dp

# ---------------------------------------

use strict;
use warnings;
use English '-no_match_vars';

my $stroke_scale=2;
my $font_scale=2;
my $circle_scale=2;
my $symbol_scale=0.8;
my $pattern_scale=2; # Area symbols
my $dasharray_scale=0.5;
my $dasharray_all_scale=6; # Convert "all" to "stroke"
my $dy_scale=2;

if ( ( $#ARGV + 1 ) < 2 ) {
  exit ( 1 );
}
my $fsource = $ARGV[0];
my $ftarget = $ARGV[1];

my $EMPTY = q{};
my $search = $EMPTY;
my $NewValue = $EMPTY;
my $dasharray = $EMPTY;
my $NewArray = $EMPTY;
my $i = $EMPTY;
my @V = $EMPTY;

open (my $data, '<:encoding(UTF-8)', $fsource) or die "Could not open $fsource: $!";
open (my $target, '>:encoding(UTF-8)', $ftarget) or die "Could not open $ftarget: $!";
 
while( my $line = <$data>)  { 
 
  # Header
  $line =~ s/(<rendertheme[^>]+)(xsi:schemaLocation=)"[^"]+"\s(version=)"\d"\s(map-background="[^"]+")\s(map-background-outside="[^"]+")/$1$2\"http:\/\/mapsforge.org\/renderTheme ..\/renderTheme.xsd\" locus-extended="1" scale-line-dy-by-zoom="1" fill-sea-areas="0" $3"1" $4/;
  # Symbol & LineSymbol: Set default symbol-width to 20
  if (($line =~ /(<(symbol|lineSymbol))/) && ($line !~ /symbol-width/)) {$line =~ s/\/>/ symbol-width="20" \/>/;}
  # Symbol: Remove id
  $line =~ s/(<symbol[^>]+)(id="[^"]+"\s)/$1/;
  # Symbol & LineSymbol & Area: Remove symbol-scaling
  $line =~ s/(<(symbol|lineSymbol|area)[^>]+)(symbol-scaling="[^"]+"\s)/$1/;
  # Caption: Replace symbol-id and position="below" with dy="22" (25 needed?)
  $line =~ s/(<caption[^>]+)(symbol-id="[^"]+"\sposition="below")/$1dy="20"/;
  # All: Replace radius with r
  $line =~ s/ radius=/ r=/;
  # All: Replace display="always" with force-draw="1"
  $line =~ s/display="always"/force-draw="1"/;
  
  # All: Zoom stroke-width
  if($line =~ /(stroke-width="(.*?)")/) {
    $search = $1; 
    $NewValue = sprintf("%.2f",$2*$stroke_scale);
    $line =~ s/$search/stroke-width="$NewValue"/g;
  }
  
  # All: Zoom font-size
  if($line =~ /(font-size="(.*?)")/) {
    $search = $1; 
    $NewValue = sprintf("%.0f",$2*$font_scale);
    $line =~ s/$search/font-size="$NewValue"/g;
  }

  # All: Zoom circle
  if($line =~ /(\br\b="(.*?)")/) {
    $search = $1; 
    $NewValue = sprintf("%.2f",$2*$circle_scale);
    $line =~ s/$search/r="$NewValue"/g;
  }
  
  # Symbol & LineSymbol: Zoom symbol-width
  if($line =~ /(<(symbol|lineSymbol)[^>]+)(symbol-width="(.*?)")/) {
    $search = $3; 
    $NewValue = sprintf("%.0f",$4*$symbol_scale);
    $line =~ s/$search/symbol-width="$NewValue"/g;
  }
  
  # Area: Zoom symbol-width
  if($line =~ /(<(area)[^>]+)(symbol-width="(.*?)")/) {
    $search = $3; 
    $NewValue = sprintf("%.0f",$4*$pattern_scale);
    $line =~ s/$search/symbol-width="$NewValue"/g;
  }
  # All: Zoom stroke-dasharray with scale=all
  if($line =~ /^(?=.*scale="all")(?=.*(stroke-dasharray="(.*?)")).*$/) {
    $search = $1;
    $dasharray = $2;
    @V = "";
    @V = split(',',$dasharray);
    for($i = 0; $i < scalar(@V); $i++) {
      $V[$i] *= $dasharray_all_scale;
    }
    $NewArray = join(",",@V);
    $line =~ s/$search/stroke-dasharray="$NewArray"/g;
    # Remove scale
    $line =~ s/(scale="[^"]+"\s)//;
  } 
 
  # All: Zoom stroke-dasharray
  if($line =~ /(stroke-dasharray="(.*?)")/) {
    $search = $1;
    $dasharray = $2;
    @V = "";
    @V = split(',',$dasharray);
    for($i = 0; $i < scalar(@V); $i++) {
      $V[$i] *= $dasharray_scale;
    }
    $NewArray = join(",",@V);
    $line =~ s/$search/stroke-dasharray="$NewArray"/g;
  }

  # Line: Zoom dy
  if($line =~ /(<(line )[^>]+)(dy="(.*?)")/) {
    $search = $3;
    $NewValue = sprintf("%.2f",$4*$dy_scale);
    $line =~ s/$search/dy="$NewValue"/g;
  }
  
  # All: Add "dp" to symbol-width
  $line =~ s/(symbol-width="[^"]+)/$1dp/;
  
  # caption & pathText: Add "dp" to dy
  $line =~ s/(<(caption|pathText)[^>]+)(dy="[^"]+)/$1$3dp/;
  #$line =~ s/(dy="[^"]+)/$1dp/;
  
  # Area: set symbol-height
  $line =~ s/(<area[^>]+)(symbol-width="(.*?)")/$1$2 symbol-height="$3"/;  
  
  #write line
  print $target $line;
}

close $data;
close $target;

# https://regex101.com/#pcre

