#!/usr/bin/perl
# ---------------------------------------
# Program : rta.pl (repository tool android)
# Version : 1.0.0 - Initiale Version
#           1.1.0 - check liest URL statt Datei
#           1.2.0 - generierung der locusaction-Datei erweitert
#           1.3.0 - Regionen hinzugefuegt
#           1.3.1 - Fehler beim Erstellen der locus-xml-Dateien korrigiert
#           1.4.0 - Regierungsbezirke Baden-Wuerttemberg hinzugefuegt
#           1.4.1 - neues Theme fzk-outdoor-contrast hinzugefuegt
#           1.5.0 - Theme fzk-outdoor-contrast in locusaction hinzugefuegt
#           1.6.0 - vollstaendige Abdeckung Europas
#           1.7.0 - Madeira ergaenzt
#           1.8.0 - V4- und Locus-Themes ergaenzt
#           1.8.1 - Defekt bei der Aktivierung des Locus-Themes korrigiert
#           1.9.0 - Luxemburg hinzugefügt, Soft-Themes hinzugefügt
#           1.10.0 - V1-Theme entfernt; fzk-outdoor-soft-locus-Theme in Locus-Download integriert
#                    NOR aufgeteilt in NORTH und SOUTH
#           17.09 - Ausgabe Herbst 2017
#           17.12 - Ausgabe Winter 2017 (mit V5-Themes)
#           18.12 - Ausgabe Winter 2018
#           19.03 - Ausgabe Frühjahr 2019 (fix: doppelte UTF-Kodierung)
#           19.06 - Ausgabe Sommer 2019: v5 Themes für Locus
#           23.03 - Ausgabe Frühjahr 2023: Theme fzk-outdoor-dark hinzugefügt
#           23.12 - Locus Themes nicht mehr auspacken
#
# Copyright (C) 2013-2023 Project Freizeitkarte <freizeitkarte@googlemail.com>
#
# Program code formatted with "perltidy".
# ---------------------------------------

use lib '.';
use strict;
use warnings;
use English '-no_match_vars';

use Cwd;
use File::Copy;
use File::Basename;
use Getopt::Long;
use Digest::MD5;
use Encode;
use LWP::UserAgent;

# Kartendefinitionen
use mapdefs;

my $ua = LWP::UserAgent->new;
$ua->timeout ( 30 );

# pseudo constants
my $EMPTY = q{};

my $THEME_NAME                = 0;
my $THEME_DESCRIPTION_GERMAN  = 1;
my $THEME_DESCRIPTION_ENGLISH = 2;

# 'Name', 'Description german', 'Description english'
# [ 'freizeitkarte', 'Standard-Design', 'Default design' ]
my @themes = (
    # V5-Themes
    [ 'freizeitkarte-v5', 'Standard-Design V5', 'Default design v5' ],                                   #
    [ 'fzk-outdoor-contrast-v5', 'Outdoor-Design Kontrast V5', 'Outdoor design contrast v5' ],           #
    [ 'fzk-outdoor-soft-v5', 'Outdoor-Design Soft V5', 'Outdoor design soft v5' ],                       #
    [ 'fzk-outdoor-dark-v5', 'Outdoor-Design Dunkel V5', 'Outdoor design dark v5' ],                     #
);

my $error   = -1;
my $command = $EMPTY;

# Mapsforge header data
my $mapsforge_magic_bytes         = '';
my $mapsforge_header_size         = 0;
my $mapsforge_file_version        = 0;
my $mapsforge_file_size           = 0;
my $mapsforge_date_of_creation    = 0;
my $mapsforge_bounding_box_minLat = 0;
my $mapsforge_bounding_box_minLon = 0;
my $mapsforge_bounding_box_maxLat = 0;
my $mapsforge_bounding_box_maxLon = 0;
my $mapsforge_tile_size           = 0;

# begin variable parts (modify this before creating repository)
# --------------------

# <Homepage>
my $homepage = 'http://freizeitkarte-osm.de';

# <Release>
my $release = "2312";

# <HeaderHintGerman>
my $header_hint_german = 'Die Freizeitkarten Ausgabe ' . $release . ' basierend auf den OpenStreetMap-Daten vom 11.12.2023.';

# <HeaderHintEnglish>
my $header_hint_english = 'The Freizeitkarte maps release ' . $release . ' are based on OpenStreetMap data of the 2023/11/12';

# <FooterHintGerman>
my $footer_hint_german = 'Viel Freude an den Freizeitkarten ... und viele interessante Touren damit.';

# <FooterHintEnglish>
my $footer_hint_english = 'We wish you lots of fun with the Freizeitkarte maps ... and many interesting trips.';

# <BackgroundPicture>
my $background_picture = $EMPTY;

# source URL
my $url_source = 'download.freizeitkarte-osm.de/android/2312/';

# map URLs
my $url_maps              = $url_source;
my $url_locusactions_maps = $url_source;

# theme URLs
my $url_themes              = $url_source;
my $url_locusactions_themes = $url_source;

# source Files
my $basepath_files = '/home/jenner/freizeitkarte-android-2312/android_2312';

# end variable parts
# ------------------

my $VERSION = '23.12 - 2023/11/12';

#  Source file locations
my $basepath_maps         = $basepath_files;
my $basepath_themes       = $basepath_files;
my $basepath_locusactions = $basepath_files;

# map / theme entry
my $name                = $EMPTY;
my $description_german  = $EMPTY;
my $description_english = $EMPTY;
my $map_zip             = $EMPTY;
my $map_map             = $EMPTY;
my $theme               = $EMPTY;
my $locusaction         = $EMPTY;

# basepath
my $BASEPATH = getcwd ( $PROGRAM_NAME );

# program startup
# ---------------
my $programName = basename ( $PROGRAM_NAME );
my $programInfo = "$programName - Repository Tool Android";
printf {*STDOUT} ( "\n%s, %s\n\n", $programInfo, $VERSION );

printf {*STDOUT} ( "Homepage            : $homepage\n" );
printf {*STDOUT} ( "Release             : $release\n\n" );
printf {*STDOUT} ( "Header Hint German  : $header_hint_german\n" );
printf {*STDOUT} ( "Footer Hint German  : $footer_hint_german\n" );
printf {*STDOUT} ( "Header Hint English : $header_hint_english\n" );
printf {*STDOUT} ( "Footer Hint English : $footer_hint_english\n\n" );
printf {*STDOUT} ( "Download von URL    : $url_source\n" );
printf {*STDOUT} ( "Sammelverzeichnis   : $basepath_files\n\n" );

# OS X = 'darwin'; Windows = 'MSWin32'; Linux = 'linux'; FreeBSD = 'freebsd'
printf { *STDOUT } ( "OSNAME = %s\n", $OSNAME );
printf { *STDOUT } ( "PERL_VERSION = %s\n", $PERL_VERSION );
printf { *STDOUT } ( "BASEPATH = %s\n\n", $BASEPATH );

# command line parameters
my $help  = $EMPTY;
my $copy  = $EMPTY;
my $build = $EMPTY;
my $check = $EMPTY;

# get the command line parameters
GetOptions ( 'h|?' => \$help, 'copy' => \$copy, 'build' => \$build, 'check' => \$check );

if ( $help || ( ( $copy eq $EMPTY ) && ( $check eq $EMPTY ) && ( $build eq $EMPTY ) ) ) {
    show_help ();
}

# my $filemode       = '+>:utf8';
my $filemode       = '+>';
my $repositoryfile = $basepath_maps . '/repository_freizeitkarte_android.xml';
my $REPOSITORY;

if ( $copy ) {
    # copy source data
    copy_maps   ();
    copy_themes ();
}
elsif ( $build ) {
    # printf { *STDOUT } ( "URL maps   : %s\n",   $url_maps );
    # printf { *STDOUT } ( "URL themes : %s\n\n", $url_themes );
    # build repository
    build_repository ();
}
elsif ( $check ) {
    # check URLs
    check_urls ();
}
else {
    printf {*STDERR} ( "ERROR: Illegal option.\n\n" );
}

exit ( 0 );


# -----------------------------------------
# Build repository.
# -----------------------------------------
sub build_repository {

    # open repository file
    open ( $REPOSITORY, $filemode, $repositoryfile ) or die ( "Error opening repository file \"$repositoryfile\": $!\n" );

    # write xml header
    # ----------------
    printf {$REPOSITORY} ( "<Freizeitkarte>\n" );

    # write general section
    # ---------------------
    printf {$REPOSITORY} ( "  <General>\n" );
    printf {$REPOSITORY} ( "    <Homepage>%s</Homepage>\n", $homepage );
    printf {$REPOSITORY} ( "    <Timestamp>%s</Timestamp>\n", get_timestamp ( time () ) );
    printf {$REPOSITORY} ( "    <Release>%s</Release>\n",                     $release );
    printf {$REPOSITORY} ( "    <HeaderHintGerman>%s</HeaderHintGerman>\n",   $header_hint_german );
    printf {$REPOSITORY} ( "    <FooterHintGerman>%s</FooterHintGerman>\n",   $footer_hint_german );
    printf {$REPOSITORY} ( "    <HeaderHintEnglish>%s</HeaderHintEnglish>\n", $header_hint_english );
    printf {$REPOSITORY} ( "    <FooterHintEnglish>%s</FooterHintEnglish>\n", $footer_hint_english );
    printf {$REPOSITORY} ( "    <BackgroundPicture>%s</BackgroundPicture>\n", $background_picture );
    printf {$REPOSITORY} ( "    <ChecksumAlgorithm>MD5</ChecksumAlgorithm>\n" );
    printf {$REPOSITORY} ( "  </General>\n" );

    # write map section
    # -----------------
    printf {*STDOUT} ( "Building repository ...\n\n" );
    printf {*STDOUT} ( "Processing maps ...\n" );

    for my $mapdata ( @maps ) {
        $name                = 'Freizeitkarte_' . @$mapdata[$NAME];
        $description_german  = 'Freizeitkarte ' . @$mapdata[$DESCRIPTION_GERMAN];
        $description_english = 'Freizeitkarte ' . @$mapdata[$DESCRIPTION_ENGLISH];
        $map_zip             = lc ( $name ) . '.map.zip';
        $map_map             = './install/' . $name . '/' . $name . '.map';
        $locusaction         = 'locus_install_map_' . lc ( $name ) . '.xml';
        if ( -e $map_map ) {
            read_mapsforge_header        ( $map_map );
            write_map_entry              ();
            create_locusaction_map_theme ();
        }
    }

    # write theme section
    # -------------------
    printf {*STDOUT} ( "\nProcessing themes ...\n" );

    for my $themedata ( @themes ) {
        $name                = @$themedata[$THEME_NAME];
        $description_german  = @$themedata[$THEME_DESCRIPTION_GERMAN];
        $description_english = @$themedata[$THEME_DESCRIPTION_ENGLISH];
        $theme               = lc ( $name ) . '.zip';
        $locusaction         = 'locus_install_theme_' . lc ( $name ) . '.xml';
        write_theme_entry        ();
        create_locusaction_theme ();
    }

    # write xml footer
    # ----------------
    printf {$REPOSITORY} ( "</Freizeitkarte>\n" );

    # close repository file
    close ( $REPOSITORY );

    printf {*STDOUT} ( "\nCreating repository file ...\n" );
    printf {*STDOUT} ( "%s\n\n", $repositoryfile );

    return;
}


# -----------------------------------------
# Write map entry.
# -----------------------------------------
sub write_map_entry {

    # get file size
    my $filename = $basepath_maps . '/' . $map_zip;
    my $filesize = -s $filename;

    if ( !$filesize ) {
        printf {*STDOUT} ( "\nError - non-existing or empty file: %s\n\n", $filename );
        exit ( 2 );
    }

    printf {*STDOUT} ( "%s (%d bytes)\n", $filename, $filesize );

    printf {$REPOSITORY} ( "  <Map>\n" );
    printf {$REPOSITORY} ( "    <Name>%s</Name>\n", $name );
    printf {$REPOSITORY} ( "    <DescriptionGerman>%s</DescriptionGerman>\n", $description_german );
    printf {$REPOSITORY} ( "    <DescriptionEnglish>%s</DescriptionEnglish>\n", $description_english );
    printf {$REPOSITORY} ( "    <Url>http://%s%s</Url>\n", $url_maps, $map_zip );
    printf {$REPOSITORY} ( "    <Size>%d</Size>\n",                                              $filesize );
    printf {$REPOSITORY} ( "    <Checksum>%s</Checksum>\n",                                      md5sum ( $filename ) );
    printf {$REPOSITORY} ( "    <LocusActionLink>locus-actions://http/%s%s</LocusActionLink>\n", $url_locusactions_maps, $locusaction );
    printf {$REPOSITORY} ( "    <MapsforgeFileVersion>%d</MapsforgeFileVersion>\n",              $mapsforge_file_version );
    printf {$REPOSITORY} ( "    <MapsforgeFileSize>%d</MapsforgeFileSize>\n",                    $mapsforge_file_size );
    printf {$REPOSITORY} ( "    <MapsforgeDateOfCreation>%s</MapsforgeDateOfCreation>\n", get_timestamp ( $mapsforge_date_of_creation / 1000 ) );
    printf {$REPOSITORY} ( "    <MapsforgeBoundingBoxMinLat>%d</MapsforgeBoundingBoxMinLat>\n", $mapsforge_bounding_box_minLat );
    printf {$REPOSITORY} ( "    <MapsforgeBoundingBoxMinLon>%d</MapsforgeBoundingBoxMinLon>\n", $mapsforge_bounding_box_minLon );
    printf {$REPOSITORY} ( "    <MapsforgeBoundingBoxMaxLat>%d</MapsforgeBoundingBoxMaxLat>\n", $mapsforge_bounding_box_maxLat );
    printf {$REPOSITORY} ( "    <MapsforgeBoundingBoxMaxLon>%d</MapsforgeBoundingBoxMaxLon>\n", $mapsforge_bounding_box_maxLon );
    printf {$REPOSITORY} ( "    <MapsforgeTileSize>%d</MapsforgeTileSize>\n",                   $mapsforge_tile_size );
    printf {$REPOSITORY} ( "  </Map>\n" );

    return;
}


# -----------------------------------------
# Write theme entry.
# -----------------------------------------
sub write_theme_entry {

    # get file size
    my $filename = $basepath_themes . '/' . $theme;
    my $filesize = -s $filename;

    if ( !$filesize ) {
        printf {*STDOUT} ( "\nError - non-existing or empty file: %s\n\n", $filename );
        exit ( 2 );
    }

    printf {*STDOUT} ( "%s (%d bytes)\n", $filename, $filesize );

    printf {$REPOSITORY} ( "  <Theme>\n" );
    printf {$REPOSITORY} ( "    <Name>%s</Name>\n", $name );
    printf {$REPOSITORY} ( "    <DescriptionGerman>%s</DescriptionGerman>\n", $description_german );
    printf {$REPOSITORY} ( "    <DescriptionEnglish>%s</DescriptionEnglish>\n", $description_english );
    printf {$REPOSITORY} ( "    <Url>http://%s%s</Url>\n", $url_themes, $theme );
    printf {$REPOSITORY} ( "    <Size>%d</Size>\n",                                              $filesize );
    printf {$REPOSITORY} ( "    <Checksum>%s</Checksum>\n",                                      md5sum ( $filename ) );
    printf {$REPOSITORY} ( "    <LocusActionLink>locus-actions://http/%s%s</LocusActionLink>\n", $url_locusactions_themes, $locusaction );
    printf {$REPOSITORY} ( "  </Theme>\n" );

    return;
}


# -----------------------------------------
# Create locus action xml file (map + theme).
# -----------------------------------------
sub create_locusaction_map_theme {

    my $filename = $basepath_locusactions . '/' . $locusaction;
    my $LOCUSACTION;

    # open locusaction xml file
    open ( $LOCUSACTION, $filemode, $filename ) or die ( "Error opening locus action file \"$filename\": $!\n" );

    printf {$LOCUSACTION} ( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" );
    printf {$LOCUSACTION} ( "<locusActions>\n" );

    printf {$LOCUSACTION} ( "  <!-- download vector map theme (downloaded file) -->\n" );
    printf {$LOCUSACTION} ( "  <download>\n" );
    printf {$LOCUSACTION} ( "    <source><![CDATA[http://%sfreizeitkarte-v5.zip]]></source>\n", $url_themes );
    printf {$LOCUSACTION} ( "    <dest><![CDATA[/mapsVector/_themes/freizeitkarte-v5.zip]]></dest>\n" );
    printf {$LOCUSACTION} ( "  </download>\n" );

    printf {$LOCUSACTION} ( "  <!-- download vector map theme (downloaded file) -->\n" );
    printf {$LOCUSACTION} ( "  <download>\n" );
    printf {$LOCUSACTION} ( "    <source><![CDATA[http://%sfzk-outdoor-contrast-v5.zip]]></source>\n", $url_themes );
    printf {$LOCUSACTION} ( "    <dest><![CDATA[/mapsVector/_themes/fzk-outdoor-contrast-v5.zip]]></dest>\n" );
    printf {$LOCUSACTION} ( "  </download>\n" );

    printf {$LOCUSACTION} ( "  <!-- download vector map theme (downloaded file) -->\n" );
    printf {$LOCUSACTION} ( "  <download>\n" );
    printf {$LOCUSACTION} ( "    <source><![CDATA[http://%sfzk-outdoor-soft-v5.zip]]></source>\n", $url_themes );
    printf {$LOCUSACTION} ( "    <dest><![CDATA[/mapsVector/_themes/fzk-outdoor-soft-v5.zip]]></dest>\n" );
    printf {$LOCUSACTION} ( "  </download>\n" );

    printf {$LOCUSACTION} ( "  <!-- download vector map theme (downloaded file) -->\n" );
    printf {$LOCUSACTION} ( "  <download>\n" );
    printf {$LOCUSACTION} ( "    <source><![CDATA[http://%sfzk-outdoor-dark-v5.zip]]></source>\n", $url_themes );
    printf {$LOCUSACTION} ( "    <dest><![CDATA[/mapsVector/_themes/fzk-outdoor-dark-v5.zip]]></dest>\n" );
    printf {$LOCUSACTION} ( "  </download>\n" );

    printf {$LOCUSACTION}
        ( "  <!-- download vector map, extract it and delete source (downloaded file); after that, also refresh list of maps -->\n" );
    printf {$LOCUSACTION} ( "  <download>\n" );
    printf {$LOCUSACTION} ( "    <source><![CDATA[http://%s%s]]></source>\n", $url_maps, $map_zip );
    printf {$LOCUSACTION} ( "    <dest><![CDATA[/mapsVector/%s]]></dest>\n", $map_zip );
    printf {$LOCUSACTION} ( "    <after>extract|deleteSource|refreshMaps</after>\n" );
    printf {$LOCUSACTION} ( "  </download>\n" );

    printf {$LOCUSACTION} ( "  <!-- set downloaded vector map; value needs to be relative path to map file -->\n" );
    printf {$LOCUSACTION} ( "  <event>\n" );
    printf {$LOCUSACTION} ( "    <key>setMapVector</key>\n" );
    printf {$LOCUSACTION} ( "    <value><![CDATA[/mapsVector/%s.map]]></value>\n", $name );
    printf {$LOCUSACTION} ( "  </event>\n" );

    printf {$LOCUSACTION} ( "  <!-- set also downloaded vector theme; value needs to be relative path to ZIP file -->\n" );
    printf {$LOCUSACTION} ( "  <event>\n" );
    printf {$LOCUSACTION} ( "    <key>setMapVectorTheme</key>\n" );
    printf {$LOCUSACTION} ( "    <value><![CDATA[/mapsVector/_themes/freizeitkarte-v5.zip]]></value>\n" );
    printf {$LOCUSACTION} ( "  </event>\n" );

    printf {$LOCUSACTION} ( "</locusActions>\n" );

    # close locusaction xml file
    close ( $LOCUSACTION );

    return;
}


# -----------------------------------------
# Create locus action xml file (theme).
# -----------------------------------------
sub create_locusaction_theme {

    my $filename = $basepath_locusactions . '/' . $locusaction;
    my $LOCUSACTION;

    # open locusaction xml file
    open ( $LOCUSACTION, $filemode, $filename ) or die ( "Error opening locus action file \"$filename\": $!\n" );

    printf {$LOCUSACTION} ( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" );
    printf {$LOCUSACTION} ( "<locusActions>\n" );

    printf {$LOCUSACTION} ( "  <download>\n" );
    printf {$LOCUSACTION} ( "    <source><![CDATA[http://%s%s]]></source>\n", $url_themes, $theme );
    printf {$LOCUSACTION} ( "    <dest><![CDATA[/mapsVector/_themes/%s]]></dest>\n", $theme );
    printf {$LOCUSACTION} ( "  </download>\n" );

    printf {$LOCUSACTION} ( "</locusActions>\n" );

    # close locusaction xml file
    close ( $LOCUSACTION );

    return;
}


# -----------------------------------------
# Copy maps.
# -----------------------------------------
sub copy_maps {

    my $source      = $EMPTY;
    my $destination = $basepath_maps;

    # Verzeichnis anlegen
    mkdir ( $destination );

    # Dateien kopieren
    for my $mapdata ( @maps ) {
        $source = './install/Freizeitkarte_' . @$mapdata[$NAME] . '/' . 'freizeitkarte_' . lc ( @$mapdata[$NAME] ) . '.map.zip';
        if ( -e $source ) {
            copy ( $source, $destination );
        }
    }

    return;
}


# -----------------------------------------
# Copy themes.
# -----------------------------------------
sub copy_themes {

    my $source      = $EMPTY;
    my $destination = $EMPTY;
    my $theme       = $EMPTY;
    my $zipper      = $EMPTY;

    # Verzeichnis anlegen
    mkdir ( $basepath_themes );

    # ins Quellverzeichnis wechseln (damit der Pfad nicht beruecksichtigt wird)
    chdir "./theme";

    # Themes zippen (ins Zielverzeichnis)
    for my $themedata ( @themes ) {
        $source      = @$themedata[$THEME_NAME];
        $theme       = @$themedata[$THEME_NAME] . '.zip';
        $destination = $basepath_themes . '/' . $theme;

        if ( ( $OSNAME eq 'darwin' ) || ( $OSNAME eq 'linux' ) || ( $OSNAME eq 'freebsd' ) ) {
            # OS X, Linux, FreeBSD
            $zipper = 'zip -r ';
        }
        elsif ( $OSNAME eq 'MSWin32' ) {
            # Windows
            $zipper = $BASEPATH . '/tools/7-Zip/7za.exe a ';
        }
        else {
            printf {*STDERR} ( "\nError: Operating system $OSNAME not supported.\n" );
        }

        $command = $zipper . "$destination $source";
        process_command ( $command );
    }

    return;
}


# -----------------------------------------
# Calculate md5 checksum.
# -----------------------------------------
sub md5sum {
    my $file   = shift;
    my $digest = "";

    eval {
        open ( FILE, $file ) or die "Can't find file $file\n";
        my $ctx = Digest::MD5->new;
        $ctx->addfile ( *FILE );
        $digest = $ctx->hexdigest;
        close ( FILE );
    };

    if ( $@ ) {
        print $@;
        return "";
    }

    return ( $digest );
}


# -----------------------------------------
# Read mapsforge header.
# 20  magic byte mapsforge binary OSM
#  4  header size; size of the file header in bytes (without magic byte) as 4-byte INT
#  4  file version; version number of the currently used binary file format as 4-byte INT
#  8  file size; The total size of the map file in bytes
#  8  date of creation; date in milliseconds since 01.01.1970 as 8-byte LONG
# 16  bounding box; geo coordinates of the bounding box in microdegrees as 4*4-byte INT, in the order minLat, minLon, maxLat, maxLon
#  2  tile size; the tile size in pixels (e.g. 256)
# -----------------------------------------
sub read_mapsforge_header {

    my $filename = shift;

    open ( FH, '<', $filename ) or die "Can't open $filename for reading!";
    binmode ( FH );

    my $buffer = $EMPTY;
    my $value = read ( FH, $buffer, ( 20 + 4 + 4 + 8 + 8 + 16 + 2 ), 0 );

    (  $mapsforge_magic_bytes,         $mapsforge_header_size,         $mapsforge_file_version,        $mapsforge_file_size,
       $mapsforge_date_of_creation,    $mapsforge_bounding_box_minLat, $mapsforge_bounding_box_minLon, $mapsforge_bounding_box_maxLat,
       $mapsforge_bounding_box_maxLon, $mapsforge_tile_size )
        = unpack ( "a20 l> l> q> q> l> l> l> l> s>", $buffer );

    # printf "magic_bytes = <%s>\n", $mapsforge_magic_bytes;
    # printf "header_size = %d\n", $mapsforge_header_size;
    # printf "file_version = %d\n", $mapsforge_file_version;
    # printf "file_size = %ld\n", $mapsforge_file_size;
    # printf "date_of_creation = %d\n", $mapsforge_date_of_creation;
    # printf "bounding_box_minLat = %d\n", $mapsforge_bounding_box_minLat;
    # printf "bounding_box_minLon = %d\n", $mapsforge_bounding_box_minLon;
    # printf "bounding_box_maxLat = %d\n", $mapsforge_bounding_box_maxLat;
    # printf "bounding_box_maxLon = %d\n", $mapsforge_bounding_box_maxLon;
    # printf "tile_size = %d\n", $mapsforge_tile_size;
    # printf ("%s\n", scalar (localtime ($mapsforge_date_of_creation / 1000)));

    close ( FH );

    return;
}


# -----------------------------------------
# get timestamp as string.
# -----------------------------------------
sub get_timestamp {

    my $epochetime = shift;

    my ( $sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst ) = localtime ( $epochetime );
    my $nice_timestamp = sprintf ( "%04d-%02d-%02d %02d:%02d:%02d.000000", $year + 1900, $mon + 1, $mday, $hour, $min, $sec );

    return ( $nice_timestamp );
}


# -----------------------------------------
# Check if all URLs are existing.
# -----------------------------------------
sub check_urls {

    my $command     = "curl --output /dev/null --silent --fail --head --url ";
    my $returnvalue = 0;

    my $start_map_url = -1;
    my $end_map_url   = -1;

    my $start_locus_action_link = -1;
    my $end_locus_action_link   = -1;

    my $start_map_url_string = '<Url>';
    my $end_map_url_string   = '</Url>';

    my $start_locus_action_link_string = '<LocusActionLink>locus-actions://';
    my $end_locus_action_link_string   = '</LocusActionLink>';

    my $offset    = 0;
    my $length    = 0;
    my $check_url = $EMPTY;

    printf {*STDOUT} ( "Checking URLs ...\n\n" );

    # read repository URL
    my $respository_URL = 'http://repository.freizeitkarte-osm.de/repository_freizeitkarte_android.xml';
    printf {*STDOUT} ( "Reading repository URL: %s\n", $respository_URL );
    my $response = $ua->get ( $respository_URL );
    if ( !$response->is_success ) {
        printf {*STDOUT} ( "Error reading repository URL: %s\n", $response->status_line );
        return;
    }

    my @lines = split ( /\n/, $response->content );
    foreach my $line ( @lines ) {
        # printf { *STDOUT } ( "line = %s\n", $line );
        $start_map_url = index ( $line, $start_map_url_string );
        if ( $start_map_url >= 0 ) {
            $end_map_url = index ( $line, $end_map_url_string );
            $offset      = $start_map_url + length ( $start_map_url_string ) + length ( 'http://' );
            $length      = $end_map_url - $offset;
            $check_url   = substr ( $line, $offset, $length );
        }

        $start_locus_action_link = index ( $line, $start_locus_action_link_string );
        if ( $start_locus_action_link >= 0 ) {
            $end_locus_action_link = index ( $line, $end_locus_action_link_string );
            $offset                = $start_locus_action_link + length ( $start_locus_action_link_string ) + length ( 'http/' );
            $length                = $end_locus_action_link - $offset;
            $check_url             = substr ( $line, $offset, $length );
        }
        # printf { *STDOUT } ( "check_url = %s\n", $check_url );

        # check URL
        if ( $check_url ) {
            $returnvalue = system ( ( $command . $check_url ) );
            if ( $returnvalue != 0 ) {
                printf {*STDOUT} ( "FAIL : %s\n", $check_url );
            }
            else {
                printf {*STDOUT} ( "OK : %s\n", $check_url );
            }
        }

        $start_map_url = -1;
        $end_map_url   = -1;

        $start_locus_action_link = -1;
        $end_locus_action_link   = -1;

        $check_url = $EMPTY;
    }

    printf {*STDOUT} ( "\n" );

    return;
}


# -----------------------------------------
# Systembefehl ausfuehren
# -----------------------------------------
sub process_command {

    my $temp_string = $EMPTY;
    my $t0          = time ();

    printf {*STDOUT} ( "\n%s\n", $command );

    my @args             = ( $command );
    my $systemReturncode = system ( @args );

    # The return value is the exit status of the program as returned by the wait call.
    # To get the actual exit value, shift right by eight (see below).
    if ( $systemReturncode != 0 ) {
        printf {*STDERR} ( "Warning: system($command) failed: $?\n" );

        if ( $systemReturncode == -1 ) {
            printf {*STDERR} ( "Failed to execute: $!\n" );
        }
        elsif ( $systemReturncode & 127 ) {
            $temp_string = sprintf ( "Child died with signal %d, %s coredump\n", ( $systemReturncode & 127 ),
                                     ( $systemReturncode & 128 ) ? 'with' : 'without' );
            printf {*STDERR} $temp_string;
        }
        else {
            $temp_string = sprintf ( "Child exited with value %d\n", $systemReturncode >> 8 );
            printf {*STDERR} $temp_string;
        }
    }

    my $t1 = time ();

    my $elapsed          = $t1 - $t0;
    my $actionReturncode = $systemReturncode >> 8;
    printf {*STDERR} ( "\nElapsed, System-RC, Action-RC: $elapsed, $systemReturncode, $actionReturncode\n" );

    return $systemReturncode;
}


# -----------------------------------------
# Show help and exit.
# -----------------------------------------
sub show_help {

    printf {*STDOUT} (
        "Usage:\n"                                                                          #
            . "perl $programName <-copy | -build | -check>\n\n"                             #
            . "Examples:\n"                                                                 #
            . "perl $programName -copy\n"                                                   #
            . "perl $programName -build\n"                                                  #
            . "perl $programName -check\n\n"                                                #
            . "Parameter:\n"                                                                #
            . "-copy  : kopiert alle Karten (*.zip) ins Zielverzeichnis\n"                  #
            . "         zipt alle Themes und kopiert sie ins Zielverzeichnis\n\n"           #
            . "-build : erzeugt eine Repositorydatei fuer alle Karten\n"                    #
            . "         erzeugt fuer jede Karte eine spez. XML-Datei (Locus Map)\n\n"       #
            . "-check : prueft alle URLs in der Repositorydatei auf Erreichbarkeit\n\n"     #
            . "Anmerkung:\n"                                                                #
            . "Dieses Programm erfordert eine 64-Bit-Version des Perl-Interpreters.\n\n"    #
    );

    exit ( 1 );
}
