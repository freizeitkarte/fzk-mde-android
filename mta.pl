#!/usr/bin/perl
# ---------------------------------------
# Program : mta.pl (map tool android)
# Version : 3.11.0 - 2020/09/05: Datenvorverarbeitung mittels 'osmpp' integriert
#                                Sonderbehandlung für turning_cycle implementiert
# Version : 3.13.0 - 2022/09/30: Postgresql import/export angepasst für Pisten
# Version : 3.14.0 - 2022/10/20: Unterstuetzung fuer macOS (darwin) ergaenzt
# Version : 3.15.0 - 2024/01/06: Umbau keine Europe Elevation mehr, Anpassungen an Garmin Poly
# Version : 3.16.0 - 2024/04/08  Kartendefinition entfernt. Europe Ausschnitt via Parent Definition aus mapdefs.pm
# Version : 3.17.0 - 2025/09/08  Default-Heapsize reduziert (32 -> 8 MB)
#
# Copyright (C) 2011-2025 Project Freizeitkarte <freizeitkarte@googlemail.com>
#
# Verarbeitung (Beispiel AZORES):
# background
# Hintergrund_Meer_Freizeitkarte_AZORES.osm : wird mit diesem Programm erzeugt
# Hintergrund_Land_Freizeitkarte_AZORES.osm : wird via ogr2ogr und ogr2osm erzeugt
#
# fetch_osm
# Kartendaten_Freizeitkarte_AZORES.osm.pbf : wird geladen
#
# fetch_ele
# Hoehendaten_Freizeitkarte_AZORES.osm.pbf : wird geladen
#
# join_ele
# Freizeitkarte_AZORES.osm.pbf : wird via osmosis aus Karten- und Hoehendaten erzeugt
#
# join_nodes
# Freizeitkarte_AZORES.bbox.o5m
# Freizeitkarte_AZORES.alltonodesonly.o5m
# Freizeitkarte_AZORES.alltonodes.o5m
# Freizeitkarte_AZORES.temp1.pbf
# Freizeitkarte_AZORES.temp2.pbf
# Freizeitkarte_AZORES.temp3.pbf : Ergebnis der Verarbeitung
#
# join_back
# Hintergrund_Komplett_Freizeitkarte_AZORES.osm : wird aus Hintergrund_Meer und -Land erzeugt
# Freizeitkarte_AZORES.temp4.pbf : Ergebnis wird aus Hintergrund + (Karten- + Hoehendaten)  erzeugt
# Freizeitkarte_AZORES.temp4.o5m : Ergebnis als o5m-Datei
#
# transform
# Freizeitkarte_AZORES.temp5.o5m
# Freizeitkarte_AZORES.temp6.o5m
# Freizeitkarte_AZORES.transformed.osm.pbf : Ergebnis der Verarbeitung
#
# build
# Freizeitkarte_AZORES.map : Fertige Karte im map-Format (install-Verzeichnis)
#
# zip
# freizeitkarte_azores.map.zip : fertige Karte im zip-Format (install-Verzeichnis)
#
# Links:
# http://polygons.openstreetmap.fr
#
# Program code formatted with "perltidy".
#
# ---------------------------------------
#
# Route extraction routines from route_extract.pl
# Version : 0.11
# Author  : Stephan
#
# Requirements:
#   OSM Tools installed - OSMOSIS, OSMCONVERT, OSMFILTER
#   PostgreSQL installed incl contrib, postgis, postgis-scripts, pgadmin3 (GUI optional)
#   PostgreSQL: Set password for user postgres: sudo -u postgres psql; alter user postgres password 'osm.route';
#   PostgreSQL: Optimize options in file /etc/postgresql/9.4/main/postgresql.conf
#     shared_buffers = 2GB (1/4 of RAM)
#     effective_cache_size = 4GB (1/2 of RAM)
#     work_mem = 64MB (for 8GB RAM), 128MB (for 16GB RAM)
#     maintenance_work_mem = 1GB (1/8 of RAM)
#     checkpoint_segments = 32 (for 8GB RAM), 64 (for 16GB RAM)
#     checkpoint_timeout = 15min
#     checkpoint_completion_target = 0.9
#     default_statistics_target = 1000
#   PostgreSQL: Set login rights in file /etc/postgresql/9.4/main/pg_hba.conf Replace line [1] with [2]
#     [1]local   all     postgres        peer
#     [2]local   all     postgres        trust
#     [1]host    all     all     127.0.0.1/32    md5
#     [2]host    all     all     127.0.0.1/32    trust
#     [1]host    all             all             ::1/128                 md5
#     [2]host    all             all             ::1/128                 trust
#   PostgreSQL: The Route DB uses the tablespace pg_default which is located at "/var/lib/postgresql/9.4/main/base".
#     2-4GB are needed for the current Europe data. It is possible to create a new tablespace somewhere else.
#     chown postgres /ssd1/postgresql/data
#     psql -U postgres -c "CREATE TABLESPACE bigspace LOCATION '/ssd1/postgresql/data';"
#     >> DB creation script needs to be adjusted too.

use lib '.';
use strict;
use warnings;
use English '-no_match_vars';

use Cwd;
use File::Copy;
use File::Path;
use File::Basename;
use Getopt::Long;

# Kartendefinitionen
use mapdefs;

# 'Action',  'Description'
my @actions = (
    # elementare Aktionen
    [ 'create_dir',     'create all directories' ],
    [ 'create_bg',      'create meer and land background' ],
    [ 'create_routes',  'create route information' ],
    [ 'extract_osm',    'extract map parent data' ],
    [ 'fetch_osm',      'fetch osm data from url' ],
    [ 'fetch_ele',      'fetch elevation data from url' ],
    [ 'join_ele',       'join osm and elevation data' ],
    [ 'join_nodes',     'join all-to-nodes data' ],
    [ 'join_bg',        'join background data' ],
    [ 'join_routes',    'join route information' ],
    [ 'transform',      'transform / filter map data' ],
    [ 'build',          'build mapsforge map file (.map)' ],
    [ 'zip',            'zip map (.zip)' ],
    # Arbeitsablaeufe
    [ 'load_data',      'load map data: create_dir, fetch_osm, fetch_ele, join_ele' ],
    [ 'load_parent',    'load map data: create_dir, fetch_osm' ],
    [ 'load_extract',   'load map data: create_dir, extract_osm, fetch_ele, join_ele' ],
    [ 'prep',           'prepare map data: create_bg, join_nodes, join_bg, create_routes, join_routes, transform' ],
    # Sonderaktionen
    [ 'xmlvalid',    'validate xml-file against xsd-file' ],
    [ 'xmlformat',   'format xml-file for better reading' ],
    [ 'comp_filter', 'compile (language dependand) transform filter' ], );

# pseudo constants
my $EMPTY = q{};

my $ACTIONNAME = 0;
my $ACTIONDESC = 1;

my $VERSION = '3.17.0 - 2025/09/08';

# Maximale Speichernutzung (Heapsize im MB)
my $javaheapsize = 8 * 1024;

# basepath
my $BASEPATH = getcwd ( $PROGRAM_NAME );

# program startup
# ---------------
my $programName = basename ( $PROGRAM_NAME );
my $programInfo = "$programName - Map Tool for creating Android (mapsforge) maps";
printf {*STDOUT} ( "\n%s, %s\n\n", $programInfo, $VERSION );

# OS X = 'darwin'; Windows = 'MSWin32'; Linux = 'linux'; FreeBSD = 'freebsd'
printf {*STDOUT} ( "OSNAME = %s\n", $OSNAME );
printf {*STDOUT} ( "PERL_VERSION = %s\n", $PERL_VERSION );
printf {*STDOUT} ( "BASEPATH = %s\n", $BASEPATH );


# Get number of cpus and amount of physical memory
my $number_of_cpus = 0;
my $physmemKB = 0;
if ( $OSNAME eq "linux" ) {
  open my $handle, "/proc/cpuinfo" or die "Can't open cpuinfo: $!\n";
  $number_of_cpus = scalar (map /^processor/, <$handle>);
  close $handle;
  # Physical memory in kByte
  $physmemKB = (`cat /proc/meminfo | grep "MemTotal" | awk '{print \$2}'`) ;
  chomp($physmemKB);
} elsif ( $OSNAME eq "freebsd" ) {
  $number_of_cpus = `sysctl -n hw.ncpu`;
  chomp($number_of_cpus);
  # Physical memory in Byte
  my $physmem = `sysctl -n hw.physmem`;
  chomp($physmem);
  $physmemKB = $physmem / 1024;
} elsif ( $OSNAME eq "darwin" ) {
  $number_of_cpus = `sysctl -n hw.ncpu`;
  chomp($number_of_cpus);
  # Physical memory in Byte
  my $physmem = `sysctl -n hw.memsize`;
  chomp($physmem);
  $physmemKB = $physmem / 1024;
}
my $physmemMB = $physmemKB  / 1024;
printf {*STDOUT} ( "CPUs = %d\n",  $number_of_cpus );
printf {*STDOUT} ( "Physical Memory (MBytes) = %d\n\n",  $physmemMB );

# command line parameters
my $help     = $EMPTY;
my $ram      = $EMPTY;
my $ele      = 20;
my $hqele    = 0;
my $language = $EMPTY;
my $type     = 'auto';
my $threads  = $number_of_cpus;

my $actionname = $EMPTY;
my $actiondesc = $EMPTY;

my $mapname   = $EMPTY;
my $mapparent = $EMPTY;
my $osmurl    = $EMPTY;
my $maplang   = $EMPTY;
my $multilang = $EMPTY;
my $bbminlat  = $EMPTY;
my $bbminlon  = $EMPTY;
my $bbmaxlat  = $EMPTY;
my $bbmaxlon  = $EMPTY;
my $hasmeer   = $EMPTY;
my $startlat  = $EMPTY;
my $startlon  = $EMPTY;
my $startzoom = $EMPTY;

my $error   = -1;
my $command = $EMPTY;

# get the command line parameters
GetOptions ( 'h|?' => \$help, 'ram=s' => \$ram, 'ele=s' => \$ele, 'hqele' => \$hqele, 'language=s' => \$language, 'type=s' => \$type, 'threads=s' => \$threads );

if ( ( $help ) || ( ( $#ARGV + 1 ) < 2 ) ) {
    show_help ();
}

if ( $ram ne $EMPTY ) {
    $javaheapsize = $ram;
}

$actionname = $ARGV[0];
$mapname    = $ARGV[1];

$error = 1;
for my $actiondata ( @actions ) {
    if ( @$actiondata[$ACTIONNAME] eq $actionname ) {
        $actionname = @$actiondata[$ACTIONNAME];
        $actiondesc = @$actiondata[$ACTIONDESC];
        $error      = 0;
        last;
    }
}

if ( $error ) {
    show_help ();
}

printf {*STDOUT} ( "actionname = %s\n", $actionname );

if ( ( $actionname eq 'xmlvalid' ) || ( $actionname eq 'xmlformat' ) || ( $actionname eq 'comp_filter' ) ) {
    # nothing to do
    $error = 0;
}
else {
    $error = 1;
    for my $mapdata ( @maps ) {
        if ( @$mapdata[$MAPNAME] eq $mapname ) {
            $mapname   = @$mapdata[$MAPNAME];
            $mapparent = @$mapdata[$MAPPARENT];
            $osmurl    = @$mapdata[$OSMURL];
            $maplang   = @$mapdata[$MAPLANG];
            $multilang = @$mapdata[$MULTILANG];
            $bbminlat  = @$mapdata[$BBMINLAT];
            $bbminlon  = @$mapdata[$BBMINLON];
            $bbmaxlat  = @$mapdata[$BBMAXLAT];
            $bbmaxlon  = @$mapdata[$BBMAXLON];
            $hasmeer   = @$mapdata[$HASMEER];
            $startlat  = @$mapdata[$STARTLAT];
            $startlon  = @$mapdata[$STARTLON];
            $startzoom = @$mapdata[$STARTZOOM];
            $error     = 0;
            last;
        }
    }
}

if ( $error ) {
    show_help ();
}

# Default Map Language overwritten ?
if ( $language ne $EMPTY ) {
    $maplang = $language;
}

printf {*STDOUT} ( "ram        : $javaheapsize\n" );
printf {*STDOUT} ( "type       : $type\n" );
printf {*STDOUT} ( "threads    : $threads\n" );
printf {*STDOUT} ( "ele        : $ele\n" );
printf {*STDOUT} ( "hqele      : $hqele\n" );
printf {*STDOUT} ( "language   : $language\n" );
printf {*STDOUT} ( "actionname : $actionname\n" );
printf {*STDOUT} ( "actiondesc : $actiondesc\n\n" );
printf {*STDOUT} ( "mapname    : $mapname\n" );
printf {*STDOUT} ( "mapparent  : $mapparent\n" );
printf {*STDOUT} ( "osmurl     : $osmurl\n" );
printf {*STDOUT} ( "maplang    : $maplang\n" );
printf {*STDOUT} ( "multilang  : $multilang\n" );
printf {*STDOUT} ( "bb         : %s_%s ... %s_%s\n", $bbminlat, $bbminlon, $bbmaxlat, $bbmaxlon );
printf {*STDOUT} ( "hasmeer    : $hasmeer\n" );
printf {*STDOUT} ( "startpos   : %s, %s\n", $startlat, $startlon );
printf {*STDOUT} ( "startzoom  : $startzoom\n" );

# Entwicklungsumgebung auf Konsistenz pruefen.
my $directory = 'install';
if ( !( -e $directory ) ) {
    mkdir ( $directory );
    printf {*STDOUT} ( "Directory %s created.\n", $directory );
}

$directory = 'work';
if ( !( -e $directory ) ) {
    mkdir ( $directory );
    printf {*STDOUT} ( "Directory %s created.\n\n", $directory );
}

$directory = 'tmp';
if ( !( -e $directory ) ) {
    mkdir ( $directory );
    printf {*STDOUT} ( "Directory %s created.\n\n", $directory );
}

# Create the WORKDIR (used at a lot of places)
my $WORKDIR = "$BASEPATH/work/$mapname";

if ( $actionname eq 'create_dir' ) {
    create_dirs ();
}
elsif ( $actionname eq 'create_bg' ) {
    create_background ();
}
elsif ( $actionname eq 'fetch_osm' ) {
    fetch_osmdata ();
}
elsif ( $actionname eq 'fetch_ele' ) {
    fetch_eledata ();
}
elsif ( $actionname eq 'join_ele' ) {
    join_eledata ();
}
elsif ( $actionname eq 'join_nodes' ) {
    join_nodesdata ();
}
elsif ( $actionname eq 'join_bg' ) {
    join_backgrounddata ();
}
elsif ( $actionname eq 'transform' ) {
    # create_styletranslations ();
    transform_mapdata ();
    create_routes     ();
    join_routes       ();
}
elsif ( $actionname eq 'build' ) {
    build_mapsforge ();
}
elsif ( $actionname eq 'zip' ) {
    zip_maps ();
}
elsif ( $actionname eq 'load_data' ) {
    create_dirs   ();
    fetch_osmdata ();
    fetch_eledata ();
    join_eledata  ();
}
elsif ( $actionname eq 'load_parent' ) {
    create_dirs   ();
    fetch_osmdata ();
}
elsif ( $actionname eq 'load_extract' ) {
    create_dirs         ();
    extract_osm         ();
    fetch_eledata       ();
    join_eledata        ();
}
elsif ( $actionname eq 'prep' ) {
    create_background   ();
    join_nodesdata      ();
    join_backgrounddata ();
    transform_mapdata   ();
    create_routes       ();
    join_routes         ();
}
elsif ( $actionname eq 'xmlvalid' ) {
    xmllint_validate ();
}
elsif ( $actionname eq 'xmlformat' ) {
    xmllint_format ();
}
elsif ( $actionname eq 'extract_osm' ) {
    extract_osm ();
}
elsif ( $actionname eq 'comp_filter' ) {
    $error = 1;
    if (    ( $language eq 'de' )
         || ( $language eq 'en' )
         || ( $language eq 'fr' )
         || ( $language eq 'it' )
         || ( $language eq 'nl' )
         || ( $language eq 'pl' )
         || ( $language eq 'ru' )
         || ( $language eq 'pt' ) )
    {
        $error = 0;
    }
    if ( $error ) {
        show_help ();
    }
    create_styletranslations ();
    comp_filter              ();
}
elsif ( $actionname eq 'create_routes' ) {
    create_routes         ();
}
elsif ( $actionname eq 'join_routes' ) {
    join_routes         ();
}

exit ( 0 );


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
            $temp_string = sprintf ( "Child died with signal %d, %s coredump\n",
                                     ( $systemReturncode & 127 ), ( $systemReturncode & 128 ) ? 'with' : 'without' );
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
# Trim whitespaces from the start and end of the string.
# -----------------------------------------
sub trim {

    my $string = shift;

    $string =~ s/^\s+//;
    $string =~ s/\s+$//;

    return ( $string );
}


# -----------------------------------------
# Sprachabhaengigen Transformationsfilter compilieren.
# -----------------------------------------
sub comp_filter {

    printf {*STDOUT} ( "Sprachabhaengigen Transformationsfilter compilieren ...\n" );

    # osmfilter mit FZK-Erweiterungen compilieren
    chdir "$BASEPATH/transform";
    my $gcc_parameter = "fzk_osmfilter.c -O3 -o fzk_osmfilter_$language";

    $command = "gcc $gcc_parameter";
    process_command ( $command );

    return;
}


# -----------------------------------------
# Verzeichnisse neu anlegen.
# -----------------------------------------
sub create_dirs {

    printf {*STDOUT} ( "Creating directories ...\n" );

    # Verzeichnisstruktur loeschen
    rmtree ( "$BASEPATH/work/$mapname", 0, 1 );

    # Verzeichnisstruktur neu anlegen
    mkpath ( "$BASEPATH/work/$mapname" );

    # Verzeichnisstruktur loeschen
    rmtree ( "$BASEPATH/install/$mapname", 0, 1 );

    # Verzeichnisstruktur neu anlegen
    mkpath ( "$BASEPATH/install/$mapname" );

    return;
}


# -----------------------------------------
# Kartenhintergrunddaten (Meer, Land) erzeugen.
# -----------------------------------------
sub create_background {

    # OSM-Datei mit Meeresrechteck (Bounding-Box) erzeugen, IDs negativ, aufsteigend
    my $filename = "$BASEPATH/work/$mapname/Hintergrund_Meer_$mapname.osm";
    if ( $hasmeer ) {
        printf {*STDOUT} ( "Creating map background file (Meer) ...\n%s\n", $filename );

        open ( my $MEER, '>:encoding(UTF-8)', $filename ) or die ( "Error opening output file \"$filename\": $OS_ERROR\n" );

        printf {$MEER} ( "<?xml version='1.0' encoding='UTF-8'?>\n" );
        printf {$MEER} ( "<osm version='0.6' upload='false' generator='mta.pl'>\n" );
        printf {$MEER}
            ( "  <bounds minlat='%s' minlon='%s' maxlat='%s' maxlon='%s' origin='mta.pl' />\n", $bbminlat, $bbminlon, $bbmaxlat, $bbmaxlon );

        printf {$MEER}
            ( "  <node id='-999' visible='true' timestamp='2014-10-07T11:55:48Z' version='1' lat='%s' lon='%s' />\n", $bbminlat, $bbminlon );
        printf {$MEER}
            ( "  <node id='-998' visible='true' timestamp='2014-10-07T11:55:48Z' version='1' lat='%s' lon='%s' />\n", $bbminlat, $bbmaxlon );
        printf {$MEER}
            ( "  <node id='-997' visible='true' timestamp='2014-10-07T11:55:48Z' version='1' lat='%s' lon='%s' />\n", $bbmaxlat, $bbmaxlon );
        printf {$MEER}
            ( "  <node id='-996' visible='true' timestamp='2014-10-07T11:55:48Z' version='1' lat='%s' lon='%s' />\n", $bbmaxlat, $bbminlon );

        printf {$MEER} ( "  <way id='-995' visible='true' timestamp='2014-10-07T11:55:48Z' version='1' >\n" );
        printf {$MEER} ( "    <nd ref='-996' />\n" );
        printf {$MEER} ( "    <nd ref='-997' />\n" );
        printf {$MEER} ( "    <nd ref='-998' />\n" );
        printf {$MEER} ( "    <nd ref='-999' />\n" );
        printf {$MEER} ( "    <nd ref='-996' />\n" );
        printf {$MEER} ( "    <tag k='freizeitkarte' v='meer' />\n" );
        printf {$MEER} ( "  </way>\n" );

        printf {$MEER} ( "</osm>\n" );

        close ( $MEER ) or die ( "Error closing output file \"$filename\": $OS_ERROR\n" );
    }

    # OSM-Datei mit Landpolygonen fuer Bounding-Box erzeugen, IDs negativ aufsteigend
    printf {*STDOUT} ( "\nCreating map background file (Land) ...\n" );

    # 1. Landpolygone ausschneiden
    $command =                                                        #
        "ogr2ogr"                                                     #
        . " -overwrite"                                               #
        . " -skipfailures"                                            #
        . " -clipsrc"                                                 #
        . " $bbminlon $bbminlat $bbmaxlon $bbmaxlat"                  #
        . " $BASEPATH/work/$mapname/Hintergrund_Land_Shapes"          #
        . " $BASEPATH/land-polygons-split-4326/land_polygons.shp";    #
    process_command ( $command );

    # 2. Landpolygone vom Shape- ins OSM-Format ueberfuehren
    $filename = "$BASEPATH/work/$mapname/Hintergrund_Land_$mapname.osm";
    $command =                                                                    #
        "python3"                                                                  #
        . " $BASEPATH/tools/ogr2osm/ogr2osm.py"                                   #
        . " $BASEPATH/work/$mapname/Hintergrund_Land_Shapes/land_polygons.shp"    #
        . " --output=$filename"                                                   #
        . " --force"                                                              #
        . " --add-version"                                                        #
        . " --add-timestamp"                                                      #
        . " --id=-1000000000"                                                     #
        . " --positive-id";                                                       #
    process_command ( $command );

    return;
}


# -----------------------------------------
# OpenStreetMap-Daten laden.
# -----------------------------------------
sub fetch_osmdata {

    my $filename = "$BASEPATH/work/$mapname/Kartendaten_$mapname.osm.pbf";

    $command = "curl --location --url \"$osmurl\" --output \"$filename\"";
    process_command ( $command );

    # auf gueltige osm.pbf-Datei pruefen
    if ( !check_osmpbf ( $filename ) ) {
        printf {*STDERR} ( "\nError: File <$filename> is not a valid osm.pbf file.\n" );
        printf {*STDERR} ( "Please check this file concerning error hints (eg. communications errors).\n" );
    }

    return;
}


# -----------------------------------------
# Hoehendaten laden.
# -----------------------------------------
sub fetch_eledata {

    my $filename = "$BASEPATH/work/$mapname/Hoehendaten_$mapname.osm.pbf";

    # download elevation data
    my $eleurl = '';
    if ( $ele == 10 ) {
        if ( $hqele ) {
            $eleurl = "http://develop.freizeitkarte-osm.de/ele_special/ele_10_100_200/Hoehendaten_$mapname.osm.pbf";
        }
        else {
            $eleurl = "http://develop.freizeitkarte-osm.de/ele_10_100_200/Hoehendaten_$mapname.osm.pbf";
        }
    }
    elsif  ( $ele == 20 ) {
        if ( $hqele ) {
            $eleurl = "http://develop.freizeitkarte-osm.de/ele_special/ele_20_100_500/Hoehendaten_$mapname.osm.pbf";
        }
        else {
            $eleurl = "http://develop.freizeitkarte-osm.de/ele_20_100_500/Hoehendaten_$mapname.osm.pbf";
        }
    }
    else {
         printf {*STDERR} ( "\nError: Elevation <$ele> is not a valid value.\n" );
    }

    $command = "curl --location --url \"$eleurl\" --output \"$filename\"";
    process_command ( $command );

    # auf gueltige osm.pbf-Datei pruefen
    if ( !check_osmpbf ( $filename ) ) {
        printf {*STDERR} ( "\nError: File <$filename> is not a valid osm.pbf file.\n" );
        printf {*STDERR} ( "Please check this file concerning error hints (eg. communications errors).\n" );
    }

    # download license data
    my $licurl = '';
    if ( $ele == 10 ) {
        if ( $hqele ) {
            $licurl = "http://develop.freizeitkarte-osm.de/ele_special/ele_10_100_200/Hoehendaten_$mapname.osm.pbf.license";
        }
        else {
            $licurl = "http://develop.freizeitkarte-osm.de/ele_10_100_200/Hoehendaten_$mapname.osm.pbf.license";
        }
    }
    elsif  ( $ele == 20 ) {
        if ( $hqele ) {
            $licurl = "http://develop.freizeitkarte-osm.de/ele_special/ele_20_100_500/Hoehendaten_$mapname.osm.pbf.license";
        }
        else {
            $licurl = "http://develop.freizeitkarte-osm.de/ele_20_100_500/Hoehendaten_$mapname.osm.pbf.license";
        }
    }
    else {
         printf {*STDERR} ( "\nError: Elevation <$ele> is not a valid value.\n" );
    }
    $filename = "$BASEPATH/work/$mapname/Hoehendaten_$mapname.osm.pbf.license";
    $command = "curl --location --url \"$licurl\" --output \"$filename\" --fail";
    process_command ( $command );

    return;
}


# -----------------------------------------
# OpenStreetMap- und Höhendaten zu Kartendaten zusammenfuehren.
# -----------------------------------------
sub join_eledata {

    # Verzeichnis wechseln
    chdir "$BASEPATH/work/$mapname";

    my $filename_kartendaten  = "$BASEPATH/work/$mapname/Kartendaten_$mapname.osm.pbf";
    my $available_kartendaten = 0;

    if ( -e $filename_kartendaten ) {
        # auf gueltige osm.pbf-Datei pruefen
        if ( check_osmpbf ( $filename_kartendaten ) ) {
            $available_kartendaten = 1;
        }
    }

    my $filename_hoehendaten  = "$BASEPATH/work/$mapname/Hoehendaten_$mapname.osm.pbf";
    my $available_hoehendaten = 0;

    if ( -e $filename_hoehendaten ) {
        # auf gueltige osm.pbf-Datei pruefen
        if ( check_osmpbf ( $filename_hoehendaten ) ) {
            $available_hoehendaten = 1;
        }
    }

    my $filename_ergebnisdaten = "$BASEPATH/work/$mapname/$mapname.osm.pbf";

    if ( $available_kartendaten && $available_hoehendaten ) {
        # Karten- und Hoehendaten vorhanden
        printf {*STDERR} ( "\nJoining map and elevation data ...\n" );

        # Java-Optionen in Osmosis-Aufruf einbringen
        my $javacmd_options = '-Xmx4096M -Djava.io.tmpdir=' . $BASEPATH . '/tmp';
        $ENV{JAVACMD_OPTIONS} = $javacmd_options;

        # osmosis-Aufrufparameter
        my $osmosis_parameter =                         #
            " --read-pbf $filename_kartendaten"         #
            . " --read-pbf $filename_hoehendaten"       #
            . " --merge"                                #
            . " bufferCapacity=10000"                   #
            . " --write-pbf $filename_ergebnisdaten"    #
            . " omitmetadata=true";                     #

        $command = "sh $BASEPATH/tools/osmosis/bin/osmosis $osmosis_parameter";
        process_command ( $command );
    }
    elsif ( $available_kartendaten ) {
        # nur Kartendaten vorhanden
        printf {*STDERR} ( "\nWarning: Elevation data file <$filename_hoehendaten> not found.\n" );
        printf {*STDERR} ( "\nCopying map data ...\n" );

        # Kartendaten kopieren
        copy ( $filename_kartendaten, $filename_ergebnisdaten ) or die ( "copy() failed: $!\n" );
    }
    else {
        # weder Karten- noch Hoehendaten vorhanden
        printf {*STDERR} ( "\nError: Map data file <$filename_kartendaten> not found.\n" );
    }

    return;
}


# -----------------------------------------
# Kartendaten erweitern:
# - Umgrenzungsrechtecke errechnen
# - Nodes für alle Ways erzeugen
# - Daten mit Kartendaten zusammenfuehren
# -----------------------------------------
sub join_nodesdata {

    my $filename_input          = "$BASEPATH/work/$mapname/$mapname.osm.pbf";
    my $filename_bbox           = "$BASEPATH/work/$mapname/$mapname.bbox.o5m";
    my $filename_alltonodes     = "$BASEPATH/work/$mapname/$mapname.alltonodes.o5m";
    my $filename_alltonodesonly = "$BASEPATH/work/$mapname/$mapname.alltonodesonly.o5m";
    my $filename_temp1pbf       = "$BASEPATH/work/$mapname/$mapname.temp1.pbf";
    my $filename_temp2pbf       = "$BASEPATH/work/$mapname/$mapname.temp2.pbf";
    my $filename_temp3pbf       = "$BASEPATH/work/$mapname/$mapname.temp3.pbf";
    my $filename_osmppxml       = "$BASEPATH/work/$mapname/$mapname.osmpp.xml";
    my $filename_osmpppbf       = "$BASEPATH/work/$mapname/$mapname.osmpp.pbf";

    if ( -e $filename_input ) {
        # auf gueltige osm.pbf-Datei pruefen
        if ( !check_osmpbf ( $filename_input ) ) {
            printf {*STDERR} ( "\nError: Source file <$filename_input> is not a valid osm.pbf-file.\n" );
            return ( 1 );
        }
    }
    else {
        printf {*STDERR} ( "\nError: Source file <$filename_input> does not exists.\n" );
        return ( 1 );
    }

   # osmpp-Objekte erzeugen
   my $osmpp_parameter = "-inputOSM=$filename_input -outputNodes=$filename_osmppxml -startNode=1000000000000";

   $command = "$BASEPATH/tools/osmpp/$^O/osmpp $osmpp_parameter";
   process_command ( $command );

    # Java-Optionen in Osmosis-Aufruf einbringen
    my $javacmd_options = '-Xmx4096M -Djava.io.tmpdir=' . $BASEPATH . '/tmp';
    $ENV{JAVACMD_OPTIONS} = $javacmd_options;

    # osmpp-Objekte sortieren
    my $osmosis_parameter =                    #
        " --read-xml $filename_osmppxml"       #
        . " --sort"                            #
        . " --write-pbf $filename_osmpppbf";   #

    $command = "sh $BASEPATH/tools/osmosis/bin/osmosis $osmosis_parameter";
    process_command ( $command );

    # Tags fuer "bBoxArea" und "bBoxWeight" hinzufuegen; Daten vom pbf- ins o5m-Format konvertieren
    my $osmconvert_parameter = "$filename_input --max-objects=800000000 --add-bboxarea-tags --add-bboxweight-tags --verbose -o=$filename_bbox";

    $command = "$BASEPATH/tools/osmconvert/$^O/osmconvert $osmconvert_parameter";
    process_command ( $command );

    # Statistikdaten ausgeben (nur zur Kontrolle)
    # print_statistics ($filename_bbox);

    # Alle Wege und Relationen in Nodes umwandeln (loescht die umgewandelten Objekte)
    $osmconvert_parameter = "$filename_bbox --max-objects=800000000 --verbose --all-to-nodes -o=$filename_alltonodes";

    $command = "$BASEPATH/tools/osmconvert/$^O/osmconvert $osmconvert_parameter";
    process_command ( $command );

    # Statistikdaten ausgeben (nur zur Kontrolle)
    # print_statistics ($filename_alltonodes);

    # Alle Objekte "all-to-nodes" in getrennte Datei ueberfuehren (nicht jedoch die als Nodes erfasste Objekte)
    my $osmfilter_parameter = "$filename_alltonodes --keep=\"bBoxWeight\" --verbose -o=$filename_alltonodesonly";

    $command = "$BASEPATH/tools/osmfilter/$^O/osmfilter $osmfilter_parameter";
    process_command ( $command );

    # Statistikdaten ausgeben (nur zur Kontrolle)
    # print_statistics ($filename_alltonodesonly);

    # Daten ins PBF-Format konvertieren (osmosis unterstuetzt o5m-Format nicht)
    o5m_to_pbf ( $filename_bbox, $filename_temp1pbf );

    # Statistikdaten ausgeben (nur zur Kontrolle)
    # print_statistics ($filename_temp1pbf);

    # Daten ins PBF-Format konvertieren (osmosis unterstuetzt o5m-Format nicht)
    o5m_to_pbf ( $filename_alltonodesonly, $filename_temp2pbf );

    # Statistikdaten ausgeben (nur zur Kontrolle)
    # print_statistics ($filename_temp2pbf);

    # alltonodes-Objekte in Kartendaten uebernehmen
    printf {*STDERR} ( "\nJoining all-to-nodes objects ...\n" );

    # osmosis-Aufrufparameter
    $osmosis_parameter =                       #
        " --read-pbf $filename_temp1pbf"       #
        . " --read-pbf $filename_temp2pbf"     #
        . " --merge"                           #
        . " bufferCapacity=10000"              #
        . " --write-pbf $filename_temp3pbf"    #
        . " omitmetadata=true";                #

    $command = "sh $BASEPATH/tools/osmosis/bin/osmosis $osmosis_parameter";
    process_command ( $command );

    # Statistikdaten ausgeben (nur zur Kontrolle)
    # print_statistics ($filename_temp3pbf);

    # Temporäre Dateien löschen
    $command = "rm $filename_bbox $filename_alltonodes $filename_alltonodesonly $filename_temp1pbf $filename_temp2pbf";
    process_command ( $command );

    return;
}


# -----------------------------------------
# Kartenhintergrunddaten mit Kartendaten zusammenfuehren.
# -----------------------------------------
sub join_backgrounddata {

    # Verzeichnis wechseln
    chdir "$BASEPATH/work/$mapname";

    my $filename_meer        = "$BASEPATH/work/$mapname/Hintergrund_Meer_$mapname.osm";
    my $filename_land        = "$BASEPATH/work/$mapname/Hintergrund_Land_$mapname.osm";
    my $filename_hintergrund = "$BASEPATH/work/$mapname/Hintergrund_Komplett_$mapname.osm";

    # Java-Optionen in Osmosis-Aufruf einbringen
    my $javacmd_options = '-Xmx4096M -Djava.io.tmpdir=' . $BASEPATH . '/tmp';
    $ENV{JAVACMD_OPTIONS} = $javacmd_options;
    my $osmosis_parameter = $EMPTY;

    if ( $hasmeer ) {
        # 1a) Meer und Land zusammenfuehren
        printf {*STDERR} ( "\nJoining Meer and Land background data ...\n" );

        # osmosis-Aufrufparameter
        $osmosis_parameter =                           #
            " --read-xml $filename_meer"               #
            . " --read-xml $filename_land"             #
            . " --merge"                               #
            . " --write-xml $filename_hintergrund";    #
    }
    else {
        # 1b) nur Land beruecksichtigen
        printf {*STDERR} ( "\nTransfering Land into background data ...\n" );

        # osmosis-Aufrufparameter
        $osmosis_parameter =                           #
            " --read-xml $filename_land --sort"        #
            . " --write-xml $filename_hintergrund";    #
    }
    $command = "sh $BASEPATH/tools/osmosis/bin/osmosis $osmosis_parameter";
    process_command ( $command );

    # 2. Hintergrund- und OSM-Daten zusammenfuehren
    my $filename_temp3pbf = "$BASEPATH/work/$mapname/$mapname.temp3.pbf";
    my $filename_temp4pbf = "$BASEPATH/work/$mapname/$mapname.temp4.pbf";

    # osmosis-Aufrufparameter
    $osmosis_parameter =                               #
        " --read-xml $filename_hintergrund"            #
        . " --read-pbf $filename_temp3pbf"             #
        . " --merge"                                   #
        . " bufferCapacity=10000"                      #
        . " --write-pbf $filename_temp4pbf"            #
        . " omitmetadata=true";                        #

    $command = "sh $BASEPATH/tools/osmosis/bin/osmosis $osmosis_parameter";
    process_command ( $command );

    # Daten ins o5m-Format konvertieren (osmfilter unterstuetzt pbf-Format nicht)
    my $filename_temp4o5m = "$BASEPATH/work/$mapname/$mapname.temp4.o5m";

    pbf_to_o5m ( $filename_temp4pbf, $filename_temp4o5m );

    # Statistikdaten ausgeben (nur zur Kontrolle)
    # print_statistics ($filename_temp4o5m);

    # Temporäre Dateien löschen
    $command = "rm $filename_temp3pbf $filename_temp4pbf";
    process_command ( $command );
    
    return;
}


# -----------------------------------------
# Kartendaten transformieren: bestimmte Tags und Objekte entfernen / hinzufügen
# -----------------------------------------
sub transform_mapdata {

    my $filename_temp4o5m = "$BASEPATH/work/$mapname/$mapname.temp4.o5m";
    my $filename_temp5o5m = "$BASEPATH/work/$mapname/$mapname.temp5.o5m";
    my $filename_temp6o5m = "$BASEPATH/work/$mapname/$mapname.temp6.o5m";
    my $filename_temp7pbf = "$BASEPATH/work/$mapname/$mapname.temp7.pbf";
    my $filename_output   = "$BASEPATH/work/$mapname/$mapname.transformed.osm.pbf";
    my $filename_osmpppbf = "$BASEPATH/work/$mapname/$mapname.osmpp.pbf";

    # o5m-Daten modifizieren (bestimmte Tags entfernen; kann nicht mit fzk_osmfilter kombiniert werden)
    # id=7681896 = Küstenlinien-Multipolygon Ireland (sehr gross)
    # id=6038068 = = Küstenlinien-Multipolygon Great-Britain (sehr gross)
    # Weitere, weniger grosse, potentielle Problemkandidaten:
    # 5685658, Halbinsel Attika (Griechenland)
    # 5695352, Halbinsel Chalkidiki (Griechenland)
    # 5729610, Halbinsel Cotentin (Frankreich)
    # 5868101, Halbinsel Cola (Russland)
    # im Bau befindliche Wege (track, path, cycleway, footway, steps, bridleway) werden entfernt
    my $osmfilter_parameter =                                                                          #
        "$filename_temp4o5m"                                                                           #
        . " --verbose"                                                                                 #
        . " --drop-tags=\"bridge=no tunnel=no building=no oneway=no access=yes noexit=no\""            #
        . " --drop-relations=\"\@id=7681896 or \@id=6038068\""                                         #
        . " --drop-ways=\"construction=track construction=path construction=cycleway construction=footway construction=steps construction=bridleway\""
        . " --drop-nodes=\"highway=turning_circle highway=turning_loop\""                              #
        . " -o=$filename_temp5o5m";                                                                    #
    $command = "$BASEPATH/tools/osmfilter/$^O/osmfilter $osmfilter_parameter";
    process_command ( $command );

    # o5m-Daten transformieren
    my $transform_parameter = "$filename_temp5o5m --verbose -o=$filename_temp6o5m";
    $command = "$BASEPATH/transform/fzk_osmfilter_$maplang $transform_parameter";
    process_command ( $command );

    # Daten vom o5m- ins pbf-Format konvertieren
    o5m_to_pbf ( $filename_temp6o5m, $filename_temp7pbf );

    # osmpp-Objekte nach Transformation mit 'fzk_osmfilter' in Kartendaten übernehmen
    # osmpp-Objekte können somit nicht mittels 'fzk_osmfilter' transformiert werden
    printf {*STDERR} ( "\nJoining osmpp objects ...\n" );

    # Java-Optionen in Osmosis-Aufruf einbringen
    my $javacmd_options = '-Xmx4096M -Djava.io.tmpdir=' . $BASEPATH . '/tmp';
    $ENV{JAVACMD_OPTIONS} = $javacmd_options;

    # osmpp-Objekte mergen
    my $osmosis_parameter =                    #
        " --read-pbf $filename_temp7pbf"       #
        . " --read-pbf $filename_osmpppbf"     #
        . " --merge"                           #
        . " bufferCapacity=10000"              #
        . " --write-pbf $filename_output"      #
        . " omitmetadata=true";                #

    $command = "sh $BASEPATH/tools/osmosis/bin/osmosis $osmosis_parameter";
    process_command ( $command );

    # Statistikdaten ausgeben (nur zur Kontrolle)
    # print_statistics ($filename_output);

    # temporäre Dateien löschen
    $command = "rm $filename_temp4o5m $filename_temp5o5m $filename_temp6o5m $filename_temp7pbf";
    process_command ( $command );

    return;
}


# -----------------------------------------
# Daten vom o5m- ins pbf-Format konvertieren (osmosis kann nicht mit o5m-Daten umgehen).
# -----------------------------------------
sub o5m_to_pbf {

    my $o5m_filename = shift;
    my $pbf_filename = shift;

    my $osmconvert_parameter = "$o5m_filename --verbose -o=$pbf_filename";

    $command = "$BASEPATH/tools/osmconvert/$^O/osmconvert $osmconvert_parameter";
    process_command ( $command );

    return;
}


# -----------------------------------------
# Daten vom pbf- ins o5m-Format konvertieren (osmfilter kann nicht mit pbf-Daten umgehen).
# -----------------------------------------
sub pbf_to_o5m {

    my $pbf_filename = shift;
    my $o5m_filename = shift;

    my $osmconvert_parameter = "$pbf_filename --verbose -o=$o5m_filename";

    $command = "$BASEPATH/tools/osmconvert/$^O/osmconvert $osmconvert_parameter";
    process_command ( $command );

    return;
}


# -----------------------------------------
# Statistikdaten fuer die OSM-Daten ausgeben (nur zur Kontrolle).
# -----------------------------------------
sub print_statistics {

    my $filename             = shift;
    my $osmconvert_parameter = "$filename --out-statistics";

    printf {*STDOUT} ( "Statistikdaten fuer %s ...\n", $filename );

    $command = "$BASEPATH/tools/osmconvert/$^O/osmconvert $osmconvert_parameter";
    process_command ( $command );

    debug_statistics ( $filename );

    return;
}


# -----------------------------------------
# Statistikdaten fuer die OSM-Daten ausgeben (fuer Debugging-Zwecke).
# -----------------------------------------
sub debug_statistics {

    my $filename            = shift;
    my $osmfilter_parameter = $EMPTY;

    printf {*STDOUT} ( "Debugdaten fuer %s ...\n", $filename );

    # Liste aller Schlüssel sortiert nach Häufigkeit
    $osmfilter_parameter = $filename . " --out-count";
    $command = "$BASEPATH/tools/osmfilter/$^O/osmfilter $osmfilter_parameter";
    process_command ( $command );

    # Liste aller Werte eines Schlüssels
    $osmfilter_parameter = $filename . " --out-key=place";
    $command = "$BASEPATH/tools/osmfilter/$^O/osmfilter $osmfilter_parameter";
    process_command ( $command );

    return;
}


# -----------------------------------------
# Create VectorMap with the mapsforge 'map-writer plugin'. It allows to convert
# OpenStreetMap data into the .map format which is needed to display maps with
# mapsforge-based applications. The tool is implemented as a plugin to the Osmosis software
# To use the tool, you are required to have a working installation of Osmosis and the
# map-writer plugin copied to the plugins directory of Osmosis. You should also be familiar
# with the Osmosis tool.
# Commands see: http://code.google.com/p/mapsforge/wiki/GettingStartedMapWriter
# -----------------------------------------
sub build_mapsforge {

    # Eingabefile ist $mapname.transformed_plus_routes.osm.pbf falls existent, ansonsten $mapname.transformed.osm.pbf
    my $filename_inputdaten_routes = "$BASEPATH/work/$mapname/$mapname.transformed_plus_routes.osm.pbf";
    my $filename_inputdaten_other  = "$BASEPATH/work/$mapname/$mapname.transformed.osm.pbf";
    my $filename_outputdaten       = "$BASEPATH/install/$mapname/$mapname.map";

    if ( -e $filename_inputdaten_routes ) {
        printf {*STDERR} ( "\nSource file $filename_inputdaten_routes with routes does exist.\n" );
        $filename_inputdaten_other = $filename_inputdaten_routes;
    }

    if ( -e $filename_inputdaten_other ) {
        # nothing to do
    }
    else {
        printf {*STDERR} ( "\nError: Source file <$filename_inputdaten_other> does not exist.\n" );
        return ( 1 );
    }

    # ggf. bereits vorhandene Android-Vectorkarte loeschen
    if ( -e $filename_outputdaten ) {
        unlink ( $filename_outputdaten );
    }

    # Java-Optionen in Osmosis-Aufruf einbringen
    my $javacmd_options = '-Xmx' . $javaheapsize . 'M -Djava.io.tmpdir=' . $BASEPATH . '/tmp';
    $ENV{JAVACMD_OPTIONS} = $javacmd_options;

    my $map_license          = $EMPTY;
    my $map_data_license     = "OpenStreetMap contributors";
    my $contour_data_license = "U.S. Geological Survey and J. de Ferranti";
    $map_license = sprintf ("\"(c) Map: FZK project (free for private use); Map data: %s; Contour data: %s\"", $map_data_license, $contour_data_license);

    # Lizenzdatei einlesen
    my $filename_contour_data_license = "$BASEPATH/work/$mapname/Hoehendaten_$mapname.osm.pbf.license";
    if ( -e $filename_contour_data_license ) {
        my %ele_tmphash = ();
        %ele_tmphash = read_licensefile ($filename_contour_data_license);
        $contour_data_license = $ele_tmphash{"license_string_short"};
        $map_license = sprintf ("\"(c) Map: FZK project (CC-BY-3.0); Map data: %s; Contour data: %s\"", $map_data_license, $contour_data_license);
    }
    
    # auto type definieren (bei 64GB: hd ab 1 GB Dateigroesse; bei 16 GB: ab 256 MB)
    my $type_run = $type;
    if ( $type eq 'auto' ) {
        my $type_size = 256 * 1024 * 1024;      # 256MB
        if( $physmemMB > 56000 ){
    	    $type_size = 1073741824;            # 1 GB
        }
        if ( ( -s $filename_inputdaten_other ) > $type_size ) {
            $type_run = 'hd';
        }
        else {
            $type_run = 'ram';
        }
    }

    # mapsforge map (Android VectorMap) ".map" erzeugen
    # mapwriter options: https://github.com/mapsforge/mapsforge/blob/master/docs/Getting-Started-Map-Writer.md
    # possible map formats:
    #   version 3 = option 'preferred-languages' not set
    #   version 4 = option 'preferred-languages' set (multilingual map)
    #   example = preferred-languages=en,zh,hi,es,fr,ar,ru,pt,bn,de,ja,ko
    my $osmosis_parameter =                                               #
        "--read-pbf $filename_inputdaten_other"                           #
        . " --mapfile-writer"                                             #
        . " file=$filename_outputdaten"                                   #
        . " bbox='$bbminlat,$bbminlon,$bbmaxlat,$bbmaxlon'"               #
        . " type=$type_run"                                               #
        . " threads=$threads"                                             #
        . " debug-file=false"                                             #
        . " progress-logs=false"                                          #
        . " map-start-position='$startlat,$startlon'"                     #
        . " map-start-zoom=$startzoom"                                    #
        . " tag-conf-file=$BASEPATH/theme/tag_mapping.xml"                #
        . " simplification-factor=1.0"                                    #
        . " preferred-languages=$multilang"                               #
        . " comment=$map_license";                                        #

    $command = "sh $BASEPATH/tools/osmosis/bin/osmosis $osmosis_parameter";
    process_command ( $command );

    return;
}


# -----------------------------------------
# Read a given local licensefile into hash (snippet from keenonkites)
# -----------------------------------------
sub read_licensefile {

  # Argument handed over
  my $licensefile = shift;
  
  # Some needed local variables
  my $line;
  my @alllines;
  my %tmphash = ();
  
  # Let's try to open 
  if ( open IN,  "< $licensefile" ) {
    
    while ( <IN> ) {
      # Get the line
      $line = $_;
      
      # Only look at lines with '=' somewhere in the middle
      next unless $line =~ /^.*=.*+/;
      
      # Get rid of leading and trailing whitespace
      $line =~ s/^\s+//;
      $line =~ s/\s+$//;
      
      # Put result in the prepared input array
      chomp ( $line );
      push ( @alllines, $line );
      
    }
    
    # Close the file again
    close IN;
    
    # run through the array and fill the tmphash
    foreach $line ( @alllines ) {
      # create the hashtable
      $line =~ /^(.*)=(.*)$/;
      $tmphash{ $1 } = "$2";
    }
    
  }
  
  return ( %tmphash );

}


# -----------------------------------------
# Alle erzeugten Karten (soweit sinnvoll) zippen.
# -----------------------------------------
sub zip_maps {

    # in installl-Verzeichnis wechseln
    chdir "$BASEPATH/install/$mapname";

    my $source      = $EMPTY;
    my $destination = $EMPTY;
    my $zipper      = $EMPTY;

    $zipper = 'zip -r ';

    # mapsforge map (Android Vector-Map) (Beispiel: Freizeitkarte_DEU_MW.map -> freizeitkarte_deu_mw.map.zip)
    $source      = $mapname . '.map';
    $destination = lc ( $mapname ) . '.map.zip';
    $command     = $zipper . "$destination $source";
    process_command ( $command );

    return;
}


# -----------------------------------------
# Pruefen, ob Datei im osm.pbf-Format vorliegt.
# - nach String 'OSMHeader' im Dateiheader suchen
# - Implementierung moeglicherweise unzureichend.
# -----------------------------------------
sub check_osmpbf {

    my $filename         = shift;
    my $is_osmpbf_format = 0;

    if ( -e $filename ) {
        my $datablock = '';
        open ( my $fh, '<', $filename ) or die ( "Can't open $filename: $OS_ERROR\n" );
        my $chars_read = read ( $fh, $datablock, 128 );
        if ( $chars_read == 128 ) {
            if ( $datablock =~ /OSMHeader/ ) {
                $is_osmpbf_format = 1;
            }
        }
        close ( $fh ) or die ( "Can't close $filename: $OS_ERROR\n" );
    }

    return ( $is_osmpbf_format );
}


# -----------------------------------------
# XML-Datei gegen XSD-Datei validieren (via xmllint).
# -----------------------------------------
sub xmllint_validate {

    my $xml_filename = "$BASEPATH/theme/" . $ARGV[1];
    my $xsd_filename = "$BASEPATH/theme/" . $ARGV[2];

    if ( !-e $xml_filename ) {
        printf {*STDERR} ( "\nError: XML file <$xml_filename> does not exists.\n" );
        return ( 1 );
    }

    if ( !-e $xsd_filename ) {
        printf {*STDERR} ( "\nError: XSD file <$xsd_filename> does not exists.\n" );
        return ( 1 );
    }

    # validate xml-file against xsd-file (scheme)
    my $xmllint_parameter = "--noout --schema $xsd_filename $xml_filename";

    $command = "xmllint $xmllint_parameter";
    process_command ( $command );

    return;
}


# -----------------------------------------
# XML-Datei zur besseren Lesbarkeit (einheitlich) formattieren (via xmllint).
# -----------------------------------------
sub xmllint_format {

    my $xml_filename           = "$BASEPATH/theme/" . $ARGV[1];
    my $xml_filename_formatted = $xml_filename . '.formatted';

    if ( !-e $xml_filename ) {
        printf {*STDERR} ( "\nError: XML file <$xml_filename> does not exists.\n" );
        return ( 1 );
    }

    # format xml-file
    my $xmllint_parameter = "--output $xml_filename_formatted --format $xml_filename";

    printf {*STDERR} ( "\nFormatter input : $xml_filename\n" );
    printf {*STDERR} ( "Formatter output: $xml_filename_formatted\n" );

    $command = "xmllint $xmllint_parameter";
    process_command ( $command );

    return;
}


# ---------------------------------------------------------------------
# Either extract the needed OSM data from the parent Region file or,
# if already cut, just copy it
# ---------------------------------------------------------------------
sub extract_osm {

  # If this map is a regions that needed to be extracted, try to fetch the extracted region
  if ( $mapparent ne "NA" ) {
    # fill out source and destination variables
    my $source_filename      = "$BASEPATH/work/$mapparent/Kartendaten_$mapname.osm.pbf";
    my $parent_filename      = "$BASEPATH/work/$mapparent/Kartendaten_$mapparent.osm.pbf";
    my $destination_filename = "$WORKDIR/Kartendaten_$mapname.osm.pbf";
    my $polyname             = lc ($mapname =~ s/Freizeitkarte/fzk/r);

    # Check if the source file does exist already
    if ( !(-e $source_filename ) ) {
      # NOT existing, let's try to cut it

      # Check if the source file exists and is a valid osm.pbf file
      if ( -e $parent_filename ) {
        if ( !check_osmpbf ( $parent_filename ) ) {
          printf { *STDERR } ( "\nError: Resulting data file <$parent_filename> is not a valid osm.pbf file.\n" );
          return ( 1 );
        }
      }
      else {
        printf { *STDERR } ( "\nError: Source data file <$parent_filename> does not exists.\n" );
        printf ( "       Did you already download the osmdata of the map <$mapparent> ?\n\n" );
        return ( 1 );
      }

      # Java-Optionen in Osmosis-Aufruf einbringen
      my $javacmd_options = '-Xmx4096M';
      $ENV{JAVACMD_OPTIONS} = $javacmd_options;
    
		  # Put the parameters together for the osmosis run
      my $osmosis_parameter =
        " --read-pbf file=$parent_filename"
        . " --bounding-polygon file=$BASEPATH/poly/$polyname.poly"
        . " --write-pbf file=$source_filename omitmetadata=yes";

      # run osmosis
      printf { *STDERR } ( "\nExtracting needed data from OSM data file $source_filename ...\n" );
      $command = "sh $BASEPATH/tools/osmosis/bin/osmosis $osmosis_parameter";
      process_command ( $command );
    }

  	 # File already exists or has just been created: let's try to copy the file
     printf { *STDERR } ( "\nCopying the existing OSM data file $source_filename ...\n" );
     copy ( "$source_filename", "$destination_filename" ) or die ( "copy($source_filename , $destination_filename) failed: $!\n" );
     printf { *STDERR } ( "\n");

  }
  else {
     die ( "\nERROR: $mapname is not a region that needed local extraction.\n" );
  }
  
  return;

}


# -----------------------------------------
# Ableitung des 'style-translations' file mit allen sprachspezifischen Strings
# -----------------------------------------
sub create_styletranslations {

    # Set some Variables
    my $inputfile  = "$BASEPATH/transform/style-translations-master";
    my $outputfile = "$BASEPATH/transform/style-translations";
    my $langcode   = "\U$language";
    my @input;
    my %translation = ();
    my $hashkey;
    my $line;

    printf {*STDOUT} ( "\nCreating [%s] translations (%s -> %s)\n", $langcode, $inputfile, $outputfile );

    # Open input and output files
    open IN,  "< $inputfile"  or die "Can't open $inputfile : $!";
    open OUT, "> $outputfile" or die "Can't open $outputfile : $!";

    # Read only the #define values from the file (ignoring trailing and tailing whitespace)
    while ( <IN> ) {
        # Get the line
        $line = $_;

        # Skip everything except the #define lines
        next unless $line =~ /^\s*\#define\s+/;

        # Get rid of leading and trailing whitespace
        $line =~ s/^\s+//;
        $line =~ s/\s+$//;

        # Put result in the prepared input array
        chomp ( $line );
        push ( @input, $line );
    }

    # run through sorted array (to have defines without langcode above those with langcode)
    foreach $line ( sort ( @input ) ) {
        # Remove the wished langcode if existing
        $line =~ s/^(\#define \$__.*__)($langcode)\s+(.*)/$1 $3/;

        # Skip all the other language codes, just leave stuff without langcode
        next unless $line =~ /^\#define \$__.*__\s+.*/;

        # create the hashtable
        $line =~ /^\#define \$(__.*__)\s+(.*)$/;
        $translation{$1} = "$2";
    }

    # Write the output file
    foreach $hashkey ( sort ( keys %translation ) ) {
        print OUT "#define \$$hashkey \"$translation{$hashkey}\"\n";
    }

    # Close input and output files again
    close IN;
    close OUT;

    return;
}

# -----------------------------------------
# Routendaten (Wandern, Radfahren, ...) erzeugen.
# -----------------------------------------
sub create_routes {

  # paths and files
  my $filename_input          = "$BASEPATH/work/$mapname/$mapname.osm.pbf";
  my $filename_output         = "$BASEPATH/work/$mapname/$mapname.routes.osm.pbf";
  my $filename_temp1          = "$BASEPATH/work/$mapname/$mapname.routes1.o5m";
  my $filename_temp2          = "$BASEPATH/work/$mapname/$mapname.routes2.o5m";
  my $filename_temp3          = "$BASEPATH/work/$mapname/$mapname.routes3.osm.pbf";
  my $routepath               = "$BASEPATH/routes";
  my $dumppath                = "$BASEPATH/work/$mapname";

  # programs
  my $OSMOSIS                 = "$BASEPATH/tools/osmosis/bin/osmosis";
  my $OSMCONVERT              = "$BASEPATH/tools/osmconvert/$^O/osmconvert";
  my $OSMFILTER               = "$BASEPATH/tools/osmfilter/$^O/osmfilter";

  # Postgres
  my $DB_serial               = time();
  my $DB_name                 = lc("$mapname");
  $DB_name =~ s/\-|\+/_/g;

  # Convert pbf to o5m
  $command = "$OSMCONVERT $filename_input --drop-nodes -o=$filename_temp1";
  process_command ( $command );

  # filter routes
  $command = "$OSMFILTER $filename_temp1 --keep= --keep-relations=\"( type=route type=superroute ) and ( route=hiking route=foot route=mtb route=bicycle route=inline_skates route=horse route=canoe route=motorboat route=piste route=ski )\" --keep-ways=\"piste:type=downhill piste:type=nordic piste:type=sled \" --drop-nodes -o=$filename_temp2";
  process_command ( $command );
  unlink $filename_temp1;

  # Convert o5m to pbf
  $command = "$OSMCONVERT $filename_temp2 -o=$filename_temp3";
  process_command ( $command );
  unlink $filename_temp2;

  # Dump Import data with OSMOSIS
  $command = "$OSMOSIS --read-pbf $filename_temp3 --log-progress interval=60 --write-pgsql-dump directory=$dumppath";
  process_command ( $command );
  unlink $filename_temp3;

  # Drop users from DB
  # $command = "psql -U postgres -c \"SELECT pg_terminate_backend(pg_stat_activity.procpid) FROM pg_stat_activity WHERE pg_stat_activity.datname = '$DB_name';\"";
  $command = "psql -U postgres -c \"SELECT pg_terminate_backend(pg_stat_activity.pid) FROM pg_stat_activity WHERE pg_stat_activity.datname = '$DB_name';\"";
  process_command ( $command );
  # Drop DB
  $command = "psql -U postgres -c \"DROP DATABASE IF EXISTS $DB_name;\"";
  process_command ( $command );
  # Create DB
  $command = "psql -U postgres -c \"CREATE DATABASE $DB_name;\"";
  process_command ( $command );

  # Postgis Extensions
  $command = "psql -U postgres -d $DB_name -f $routepath/02_postgis_extensions.sql";
  process_command ( $command );

  # Load Snapshot Schema
  $command = "psql -U postgres -d $DB_name -f $routepath/pgsnapshot_schema_0.6.sql";
  process_command ( $command );

  # Create route functions
  $command = "psql -U postgres -d $DB_name -f $routepath/03_create_routefunctions.sql";
  process_command ( $command );

  # Import data
  chdir $dumppath;
  $command = "psql -U postgres -d $DB_name -f $routepath/pgsnapshot_load_0.6_mod2.sql";
  process_command ( $command );
  unlink glob("$dumppath/*.txt");
  chdir $BASEPATH;
  # Create additional index, Optimize db, and run route functions
  $command = "psql -U postgres -d $DB_name -f $routepath/04_run_routefunctions.sql";
  process_command ( $command );

  # Export data with OSMOSIS
  $command = "$OSMOSIS --read-pgsql host=localhost database=$DB_name user=postgres --dataset-dump --log-progress interval=60 --tf reject-relations --tf accept-ways highway=routehiking,routebicycle,routemtb,routeinline,routehorse,routecanoe,routemotorboat,node_network_hiking,node_network_bicycle,node_network_inline_skates,node_network_horse,node_network_canoe,node_network_motorboat,pistedownhill,pistenordic,pistesled --tf reject-nodes --write-pbf $filename_output";
  process_command ( $command );

  # Drop users from DB
  # $command = "psql -U postgres -c \"SELECT pg_terminate_backend(pg_stat_activity.procpid) FROM pg_stat_activity WHERE pg_stat_activity.datname = '$DB_name';\"";
  $command = "psql -U postgres -c \"SELECT pg_terminate_backend(pg_stat_activity.pid) FROM pg_stat_activity WHERE pg_stat_activity.datname = '$DB_name';\"";
  process_command ( $command );
  # Drop DB
  $command = "psql -U postgres -c \"DROP DATABASE IF EXISTS $DB_name;\"";
  process_command ( $command );

}

# -----------------------------------------
# Routendaten mit Kartendaten zusammenfuehren.
# -----------------------------------------
sub join_routes {

    # Verzeichnis wechseln
    chdir "$BASEPATH/work/$mapname";

    my $filename_input       = "$BASEPATH/work/$mapname/$mapname.transformed.osm.pbf";
    my $filename_routes      = "$BASEPATH/work/$mapname/$mapname.routes.osm.pbf";
    my $filename_complete    = "$BASEPATH/work/$mapname/$mapname.transformed_plus_routes.osm.pbf";

    # Java-Optionen in Osmosis-Aufruf einbringen
    my $javacmd_options = '-Xmx4096M -Djava.io.tmpdir=' . $BASEPATH . '/tmp';
    $ENV{JAVACMD_OPTIONS} = $javacmd_options;
    my $osmosis_parameter = $EMPTY;


    if ( -f $filename_routes && -s $filename_routes ) {
        # Karte und Routen zusammenfuehren, transform_map nutzt dann mapname.transformed_plus_routes.osm.pbf
        printf {*STDERR} ( "\nJoining hiking/bicycle route data ...\n" );

        # osmosis-Aufrufparameter
        $osmosis_parameter =
            " --read-pbf $filename_input"
            . " --read-pbf $filename_routes"
            . " --merge"
            . " bufferCapacity=10000"
            . " --write-pbf $filename_complete"
            . " omitmetadata=true";
        $command = "sh $BASEPATH/tools/osmosis/bin/osmosis $osmosis_parameter";
        process_command ( $command );
    } else {
        # Keine Routen zusammenfuehren, transform_map nutzt dann mapname.transformed.osm.pbf
        printf {*STDERR} ( "\nNo hiking/bicycle route data found, not joining.\n" );
    }

    return;
}


# -----------------------------------------
# Show help and exit.
# -----------------------------------------
sub show_help {

    for my $mapdata ( @maps ) {
        printf {*STDOUT} (
                    "%-50s(%s)  %-6s / %6s ... %-6s / %6s\n",
                    @$mapdata[$MAPNAME], @$mapdata[$MAPLANG], @$mapdata[$BBMINLAT], @$mapdata[$BBMINLON], @$mapdata[$BBMAXLAT], @$mapdata[$BBMAXLON]
        );
    }
    printf {*STDOUT} ( "\n" );

    printf {*STDOUT} (
        "Benutzung:\n"                                                                                      #
            . "perl $programName [-ram=Value] [-ele=Value] [-language=\"lang\"] <Action> <Map>\n\n"         #
            . "Beispiele:\n"                                                                                #
            . "perl $programName  load_parent Freizeitkarte_EUROPE\n"                                       #
            . "perl $programName  -ram=30000 build Freizeitkarte_MUENSTER\n\n"                              #
            . "Optionen:\n"                                                                                 #
            . "-ram      = javaheapsize in MB (fuer build) (default = %d)\n"                                #
            . "-type     = mapwriter build type (hd oder ram, default = %s)\n"                              #
            . "-threads  = mapwriter build threads (1-n, default = %s [number of CPUs])\n"                  #
            . "-ele      = equidistance of elevation lines (fetch_ele) (10, 20; default = %d)\n"            #
            . "-hqele    = high qualitiy elevation lines (flag)\n"                                          #
            . "-language = overwrite the default language of a map (de, en, fr, it, nl, pl, ru, pt)\n\n"    #
            . "Parameter:\n"                                                                                #
            . "Action    = Action to be processed\n"                                                        #
            . "Map       = Name of the map to be processed\n"                                               #
            . "\nEUROPE - Daten laden und vorbereiten:\n"                                                   #
            . "-------------------------------------\n"                                                     #
            . "- perl $programName load_parent Freizeitkarte_EUROPE\n"                                      #
            . "- perl $programName load_extract Freizeitkarte_1\n"                                       #
            . "- perl $programName load_extract ...\n"                                                      #
            . "- perl $programName load_extract Freizeitkarte_N\n"                                          #
            . "- perl $programName prep Freizeitkarte_1\n"                                                  #
            . "- perl $programName prep ...\n"                                                              #
            . "- perl $programName prep Freizeitkarte_N\n"                                                  #
            . "\ngeofabrik - Daten laden und vorbereiten:\n"                                                #
            . "----------------------------------------\n"                                                  #
            . "- perl $programName load_data Freizeitkarte_1\n"                                             #
            . "- perl $programName load_data ...\n"                                                         #
            . "- perl $programName load_data Freizeitkarte_N\n"                                             #
            . "- perl $programName prep Freizeitkarte_1\n"                                                  #
            . "- perl $programName prep ...\n"                                                              #
            . "- perl $programName prep Freizeitkarte_N\n"                                                  #
            . "\nEUROPE / geofabrik - Karten erzeugen:\n"                                                   #
            . "--------------------------------------------\n"                                              #
            . "- perl $programName -ram=WERT build Freizeitkarte_1\n"                                       #
            . "- perl $programName -ram=WERT build ...\n"                                                   #
            . "- perl $programName -ram=WERT build Freizeitkarte_N\n"                                       #
            . "- perl $programName zip Freizeitkarte_1\n"                                                   #
            . "- perl $programName zip ...\n"                                                               #
            . "- perl $programName zip Freizeitkarte_N\n"                                                   #
            . "\nTransformationsfilter compilieren (nur Entwicklung):\n"                                    #
            . "----------------------------------------------------\n"                                      #
            . "- perl $programName -language=de comp_filter DUMMY\n"                                        #
            . "- perl $programName -language=en comp_filter DUMMY\n"                                        #
            . "- perl $programName -language=fr comp_filter DUMMY\n"                                        #
            . "- perl $programName -language=it comp_filter DUMMY\n"                                        #
            . "- perl $programName -language=nl comp_filter DUMMY\n"                                        #
            . "- perl $programName -language=pl comp_filter DUMMY\n"                                        #
            . "- perl $programName -language=ru comp_filter DUMMY\n"                                        #
            . "- perl $programName -language=pt comp_filter DUMMY\n"                                        #
            . "\nSpezielle Aktionen (nur Entwicklung):\n"                                                   #
            . "-------------------------------------\n"                                                     #
            . "perl $programName xmlvalid tag_mapping.xml tag_mapping.xsd\n"                                #
            . "perl $programName xmlformat tag_mapping.xml\n\n"                                             #
            . "perl $programName xmlvalid freizeitkarte/freizeitkarte.xml renderTheme.xsd\n"                #
            . "perl $programName xmlformat freizeitkarte/freizeitkarte.xml\n"                               #
            . "\nKarte fuer Entwicklung erzeugen:\n"                                                        #
            . "----------------------------------\n"                                                        #
            . "perl $programName load_data Freizeitkarte_MUENSTER\n"                                        #
            . "perl $programName prep Freizeitkarte_MUENSTER\n"                                             #
            . "( perl $programName -language=de comp_filter DUMMY )\n"                                      #
            . "( perl $programName transform Freizeitkarte_MUENSTER )\n"                                    #
            . "perl $programName build Freizeitkarte_MUENSTER\n",                                           #
        $javaheapsize, $type, $threads, $ele );

    printf {*STDOUT} ( "\nAktionen:\n" );
    printf {*STDOUT} ( "---------\n" );

    for my $actiondata ( @actions ) {
        printf {*STDOUT} ( "%-18s = %s\n", @$actiondata[$ACTIONNAME], @$actiondata[$ACTIONDESC] );
    }
    printf {*STDOUT} ( "\n" );

    exit ( 1 );
}
