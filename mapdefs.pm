# ---------------------------------------
# Kartendefinitionen, Stand 2017/11/31
#                     Stand 2018/11/10, Israel / Palästina ergänzt
#                     Stand 2018/12/06, russische Föderationen ergänzt
#                     Stand 2019/04/20, Multilanguage codes ergänzt
#                     Stand 2019/08/19, Tschechien (CZE) ergänzt
#                     Stand 2021/10/14, Neuseeland (NZL) ergänzt
#                     Stand 2024/04/01, Auschnitte & Namen angepasst: ESP_CNARIAS, DEU+*, ALPS*, FRA+*, ESP+, BEL+
#                     Stand 2024/04/08, Parent Karte eingefügt.
# ---------------------------------------

package mapdefs;

use strict;
use warnings;

use parent 'Exporter';

# Sprachen: DE, EN, FR, IT, NL, PL, RU, PT

# 'Kartenname',
# 'Ausschnitt aus Karte',
# 'URL der Kartendaten',
# 'Kartensprache',
# 'Multilanguage - Map v4 und neuer',
# 'minLat', 'minLon', 'maxLat', 'maxLon',
# 'Meer',
# 'startLat', 'startLon',
# 'startZoom'
# 'Name', 'Beschreibung Deutsch', 'Beschreibung Englisch'

# mta.pl
our $MAPNAME   = 0;
our $MAPPARENT = 1;
our $OSMURL    = 2;
our $MAPLANG   = 3;
our $MULTILANG = 4;
our $BBMINLAT  = 5;
our $BBMINLON  = 6;
our $BBMAXLAT  = 7;
our $BBMAXLON  = 8;
our $HASMEER   = 9;
our $STARTLAT  = 10;
our $STARTLON  = 11;
our $STARTZOOM = 12;

# rta.pl
our $NAME                = 13;
our $DESCRIPTION_GERMAN  = 14;
our $DESCRIPTION_ENGLISH = 15;

# Koordinaten auf- bzw. abrunden (Beispiel):
# '45.81617', '5.952882', '47.81126', '10.49584',
# '45.82',    '5.96',     '47.81',    '10.49',

our @maps = (
    # Regierungsbezirke NRW (unvollständig)
    [  'Freizeitkarte_MUENSTER',                                                                            # (Münster, Regierungsbezirk)
       'NA',                                                                                                # ---------------------------
       'http://download.geofabrik.de/europe/germany/nordrhein-westfalen/muenster-regbez-latest.osm.pbf',    #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '51.47', '6.38', '52.48', '8.33',                                                                    #
       '0',                                                                                                 #
       '51.9613', '7.6251',                                                                                 # Münster
       '12',                                                                                                #
       'MUENSTER', 'Münster', 'Muenster'                                                                    #
    ],
    [  'Freizeitkarte_ARNSBERG',                                                                            # (Arnsberg, Regierungsbezirk)
       'NA',                                                                                                # ----------------------------
       'http://download.geofabrik.de/europe/germany/nordrhein-westfalen/arnsberg-regbez-latest.osm.pbf',    #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '50.67', '7.09', '51.75', '8.98',                                                                    #
       '0',                                                                                                 #
       '51.4016', '8.0597',                                                                                 # Arnsberg
       '12',                                                                                                #
       'ARNSBERG', 'Arnsberg', 'Arnsberg'                                                                   #
    ],
    # deutsche Bundeslaender
    [  'Freizeitkarte_BADEN-WUERTTEMBERG',                                                                  # Baden-Württemberg (Bundesland)
       'NA',                                                                                                # ------------------------------
       'http://download.geofabrik.de/europe/germany/baden-wuerttemberg-latest.osm.pbf',                     #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '47.53', '7.50', '49.80', '10.52',                                                                   #
       '0',                                                                                                 #
       '48.7802', '9.1857',                                                                                 # Stuttgart
       '12',                                                                                                #
       'BADEN-WUERTTEMBERG', 'Baden-Württemberg', 'Baden-Wuerttemberg'                                      #
    ],
    [  'Freizeitkarte_BAYERN',                                                                              # Bayern (Bundesland)
       'NA',                                                                                                # -------------------
       'http://download.geofabrik.de/europe/germany/bayern-latest.osm.pbf',                                 #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '47.26', '8.97', '50.57', '13.85',                                                                   #
       '0',                                                                                                 #
       '48.1405', '11.5760',                                                                                # München
       '12',                                                                                                #
       'BAYERN', 'Bayern', 'Bavaria'                                                                        #
    ],
    [  'Freizeitkarte_BERLIN',                                                                              # Berlin (Bundesland)
       'NA',                                                                                                # -------------------
       'http://download.geofabrik.de/europe/germany/berlin-latest.osm.pbf',                                 #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '52.33', '13.08', '52.68', '13.77',                                                                  #
       '0',                                                                                                 #
       '52.5220', '13.3958',                                                                                # Berlin
       '12',                                                                                                #
       'BERLIN', 'Berlin', 'Berlin'                                                                         #
    ],
    [  'Freizeitkarte_BRANDENBURG',                                                                         # Brandenburg (Bundesland)
       'NA',                                                                                                # ------------------------
       'http://download.geofabrik.de/europe/germany/brandenburg-latest.osm.pbf',                            #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '51.35', '11.22', '53.58', '14.78',                                                                  #
       '0',                                                                                                 #
       '52.4009', '13.0715 ',                                                                               # Potsdam
       '12',                                                                                                #
       'BRANDENBURG', 'Brandenburg', 'Brandenburg'                                                          #
    ],
    [  'Freizeitkarte_BREMEN',                                                                              # Bremen (Bundesland)
       'NA',                                                                                                # -------------------
       'http://download.geofabrik.de/europe/germany/bremen-latest.osm.pbf',                                 #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '53.01', '8.48', '53.62', '9.00',                                                                    #
       '1',                                                                                                 #
       '53.0771', '8.8081',                                                                                 # Bremen
       '12',                                                                                                #
       'BREMEN', 'Bremen', 'Bremen'                                                                         #
    ],
    [  'Freizeitkarte_HAMBURG',                                                                             # Hamburg (Bundesland)
       'NA',                                                                                                # --------------------
       'http://download.geofabrik.de/europe/germany/hamburg-latest.osm.pbf',                                #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '53.39', '9.72', '53.75', '10.34',                                                                   #
       '1',                                                                                                 #
       '53.5538', '9.9959',                                                                                 # Hamburg
       '12',                                                                                                #
       'HAMBURG', 'Hamburg', 'Hamburg'                                                                      #
    ],
    [  'Freizeitkarte_HESSEN',                                                                              # Hessen (Bundesland)
       'NA',                                                                                                # -------------------
       'http://download.geofabrik.de/europe/germany/hessen-latest.osm.pbf',                                 #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '49.39', '7.76', '51.66', '10.25',                                                                   #
       '0',                                                                                                 #
       '50.0835', '8.2496',                                                                                 # Wiesbaden
       '12',                                                                                                #
       'HESSEN', 'Hessen', 'Hesse'                                                                          #
    ],
    [  'Freizeitkarte_MECKLENBURG-VORPOMMERN',                                                              # Mecklenburg-Vorpommern (Bundesland)
       'NA',                                                                                                # -----------------------------------
       'http://download.geofabrik.de/europe/germany/mecklenburg-vorpommern-latest.osm.pbf',                 #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '53.10', '10.58', '54.99', '14.43',                                                                  #
       '1',                                                                                                 #
       '53.6248', '11.4174',                                                                                # Schwerin
       '12',                                                                                                #
       'MECKLENBURG-VORPOMMERN', 'Mecklenburg-Vorpommern', 'Mecklenburg-Western Pomerania'                  #
    ],
    [  'Freizeitkarte_NIEDERSACHSEN',                                                                       # Niedersachsen (Bundesland)
       'NA',                                                                                                # --------------------------
       'http://download.geofabrik.de/europe/germany/niedersachsen-latest.osm.pbf',                          #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '51.29', '6.29', '54.24', '11.61',                                                                   #
       '1',                                                                                                 #
       '52.3727', '9.7401',                                                                                 # Hannover
       '12',                                                                                                #
       'NIEDERSACHSEN', 'Niedersachsen', 'Lower Saxony'                                                     #
    ],
    [  'Freizeitkarte_NORDRHEIN-WESTFALEN',                                                                 # Nordrhein-Westfalen (Bundesland)
       'NA',                                                                                                # --------------------------------
       'http://download.geofabrik.de/europe/germany/nordrhein-westfalen-latest.osm.pbf',                    #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '50.32', '5.86', '52.54', '9.47',                                                                    #
       '0',                                                                                                 #
       '51.9613', '7.6251',                                                                                 # Münster
       '12',                                                                                                #
       'NORDRHEIN-WESTFALEN', 'Nordrhein-Westfalen', 'North Rhine-Westphalia'                               #
    ],
    [  'Freizeitkarte_RHEINLAND-PFALZ',                                                                     # Rheinland-Pfalz (Bundesland)
       'NA',                                                                                                # ----------------------------
       'http://download.geofabrik.de/europe/germany/rheinland-pfalz-latest.osm.pbf',                        #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '48.96', '6.10', '50.95', '8.52',                                                                    #
       '0',                                                                                                 #
       '50.0026', '8.2755',                                                                                 # Mainz
       '12',                                                                                                #
       'RHEINLAND-PFALZ', 'Rheinland-Pfalz', 'Rhineland-Palatinate'                                         #
    ],
    [  'Freizeitkarte_SAARLAND',                                                                            # Saarland (Bundesland)
       'NA',                                                                                                # ---------------------
       'http://download.geofabrik.de/europe/germany/saarland-latest.osm.pbf',                               #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '49.10', '6.35', '49.65', '7.41',                                                                    #
       '0',                                                                                                 #
       '49.2349', '6.9897',                                                                                 # Saarbrücken
       '12',                                                                                                #
       'SAARLAND', 'Saarland', 'Saarland'                                                                   #
    ],
    [  'Freizeitkarte_SACHSEN',                                                                             # Sachsen (Bundesland)
       'NA',                                                                                                # --------------------
       'http://download.geofabrik.de/europe/germany/sachsen-latest.osm.pbf',                                #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '50.16', '11.86', '51.69', '15.06',                                                                  #
       '0',                                                                                                 #
       '51.0528', '13.7391',                                                                                # Dresden
       '12',                                                                                                #
       'SACHSEN', 'Sachsen', 'Saxony'                                                                       #
    ],
    [  'Freizeitkarte_SACHSEN-ANHALT',                                                                      # Sachsen-Anhalt (Bundesland)
       'NA',                                                                                                # ---------------------------
       'http://download.geofabrik.de/europe/germany/sachsen-anhalt-latest.osm.pbf',                         #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '50.92', '10.54', '53.06', '13.22',                                                                  #
       '0',                                                                                                 #
       '52.1267', '11.6460',                                                                                # Magdeburg
       '12',                                                                                                #
       'SACHSEN-ANHALT', 'Sachsen-Anhalt', 'Saxony-Anhalt'                                                  #
    ],
    [  'Freizeitkarte_SCHLESWIG-HOLSTEIN',                                                                  # Schleswig-Holstein (Bundesland)
       'NA',                                                                                                # -------------------------------
       'http://download.geofabrik.de/europe/germany/schleswig-holstein-latest.osm.pbf',                     #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '53.35', '7.46', '55.15', '11.80',                                                                   #
       '1',                                                                                                 #
       '54.3211', '10.1454',                                                                                # Kiel
       '12',                                                                                                #
       'SCHLESWIG-HOLSTEIN', 'Schleswig-Holstein', 'Schleswig-Holstein'                                     #
    ],
    [  'Freizeitkarte_THUERINGEN',                                                                          # Thüringen (Bundesland)
       'NA',                                                                                                # ----------------------
       'http://download.geofabrik.de/europe/germany/thueringen-latest.osm.pbf',                             #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '50.19', '9.86', '51.66', '12.68',                                                                   #
       '0',                                                                                                 #
       '50.9800', '11.0284',                                                                                # Erfurt
       '12',                                                                                                #
       'THUERINGEN', 'Thüringen', 'Thuringia'                                                               #
    ],
    # Karten basierend auf geofabrik-Extrakten
    [  'Freizeitkarte_NOR',                                                                                 # Norwegen (NOR) (Test)
       'NA',                                                                                                # ---------------------
       'http://download.geofabrik.de/europe/norway-latest.osm.pbf',                                         #
       'en',                                                                                                #
       'en,de,fr,es,no',                                                                                    #
       '57.55', '-11.37', '81.06', '35.53',                                                                 #
       '1',                                                                                                 #
       '59.9178', '10.7476',                                                                                # Oslo
       '12',                                                                                                #
       'NOR', 'Norwegen', 'Norway'                                                                          #
    ],
    [  'Freizeitkarte_CYP',                                                                                 # Zypern (CYP)
       'NA',                                                                                                # ------------
       'http://download.geofabrik.de/europe/cyprus-latest.osm.pbf',                                         #
       'en',                                                                                                #
       'en,de,fr,es,el,tr',                                                                                 #
       '34.23', '31.95', '36.01', '34.97',                                                                  #
       '1',                                                                                                 #
       '35.1390', '33.3699',                                                                                # Nikosia
       '12',                                                                                                #
       'CYP', 'Zypern', 'Cyprus'                                                                            #
    ],
    [  'Freizeitkarte_AZORES',                                                                              # Azoren
       'NA',                                                                                                # ------
       'http://download.geofabrik.de/europe/azores-latest.osm.pbf',                                         #
       'pt',                                                                                                #
       'en,de,fr,es,pt',                                                                                    #
       '35.67', '-31.58', '40.08', '-23.71',                                                                #
       '1',                                                                                                 #
       '37.7374', '-25.6724',                                                                               # Ponta Delgada
       '12',                                                                                                #
       'AZORES', 'Azoren', 'Azores'                                                                         #
    ],
    [  'Freizeitkarte_MLT',                                                                                 # Malta (MLT)
       'NA',                                                                                                # -----------
       'http://download.geofabrik.de/europe/malta-latest.osm.pbf',                                          #
       'en',                                                                                                #
       'en,de,fr,es,mt',                                                                                    #
       '35.51', '14.00', '36.34', '14.86',                                                                  #
       '1',                                                                                                 #
       '35.8986', '14.5116',                                                                                # Valletta
       '12',                                                                                                #
       'MLT', 'Malta', 'Malta'                                                                              #
    ],
    [  'Freizeitkarte_ESP_CANARIAS',                                                                        # Kanarische Inseln
       'NA',                                                                                                # -----------------
       'http://download.geofabrik.de/africa/canary-islands-latest.osm.pbf',                                 #
       'en',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '26.36', '-18.93', '30.26', '-12.47',                                                                #
       '1',                                                                                                 #
       '28.4510', '-16.2809',                                                                               # Santa Cruz de Tenerife
       '12',                                                                                                #
       'ESP_CANARIAS', 'Kanarische Inseln', 'Canary Islands'                                                #
    ],
    [  'Freizeitkarte_ISL',                                                                                 # Island (ISL)
       'NA',                                                                                                # ------------
       'http://download.geofabrik.de/europe/iceland-latest.osm.pbf',                                        #
       'en',                                                                                                #
       'en,de,fr,es,is',                                                                                    #
       '62.84', '-25.70', '67.51', '-12.41',                                                                #
       '1',                                                                                                 #
       '64.1222', '-21.9008',                                                                               # Reykjavik
       '12',                                                                                                #
       'ISL', 'Island', 'Iceland'                                                                           #
    ],
    [  'Freizeitkarte_TUR',                                                                                 # Türkei (TUR)
       'NA',                                                                                                # ------------
       'http://download.geofabrik.de/europe/turkey-latest.osm.pbf',                                         #
       'en',                                                                                                #
       'en,de,fr,es,tr',                                                                                    #
       '35.71', '25.60', '42.40', '44.90',                                                                  #
       '1',                                                                                                 #
       '41.0216', '29.0065',                                                                                # Istanbul
       '12',                                                                                                #
       'TUR', 'Türkei', 'Turkey'                                                                            #
    ],
    [  'Freizeitkarte_FRO',                                                                                 # Färöer-Inseln (FRO)
       'NA',                                                                                                # -------------------
       'http://download.geofabrik.de/europe/faroe-islands-latest.osm.pbf',                                  #
       'en',                                                                                                #
       'en,de,fr,es,fo,da',                                                                                 #
       '61.30', '-8.00', '63.00', '-6.00',                                                                  #
       '1',                                                                                                 #
       '62.0112', '-6.7722',                                                                                # Torshavn
       '12',                                                                                                #
       'FRO', 'Färöer-Inseln', 'Faroe Islands'                                                              #
    ],
    [  'Freizeitkarte_GBR',                                                                                 # Grossbritannien (GBR)
       'NA',                                                                                                # ---------------------
       'http://download.geofabrik.de/europe/great-britain-latest.osm.pbf',                                  #
       'en',                                                                                                #
       'en,de,fr,es,cy,gd',                                                                                 #
       '49.52', '-15.00', '61.14', '2.52',                                                                  # (Insel)
       '1',                                                                                                 #
       '51.5055', '-0.1143',                                                                                # London
       '12',                                                                                                #
       'GBR', 'Gross Britannien (Insel)', 'Great Britain (Isle)'                                            #
    ],
    [  'Freizeitkarte_IMN',                                                                                 # Isle of Man (IMN)
       'NA',                                                                                                # -----------------
       'http://download.geofabrik.de/europe/isle-of-man-latest.osm.pbf',                                    #
       'en',                                                                                                #
       'en,de,fr,es,gv',                                                                                    #
       '53.73', '-5.44', '54.68', '-3.68',                                                                  #
       '1',                                                                                                 #
       '54.1503', '-4.4796',                                                                                # Douglas
       '12',                                                                                                #
       'IMN', 'Insel Man', 'Isle of man'                                                                    #
    ],
    [  'Freizeitkarte_IRL',                                                                                 # Irland (IRL)
       'NA',                                                                                                # ------------
       'http://download.geofabrik.de/europe/ireland-and-northern-ireland-latest.osm.pbf',                   #
       'en',                                                                                                #
       'en,de,fr,es,ga',                                                                                    #
       '51.08', '-11.40', '55.70', '-5.059',                                                                # (Insel)
       '1',                                                                                                 #
       '53.3476', '-6.2580',                                                                                # Dublin
       '12',                                                                                                #
       'IRL', 'Irland (Insel)', 'Ireland (Isle)'                                                            #
    ],
    [  'Freizeitkarte_AUT',                                                                                 # Österreich (AUT)
       'NA',                                                                                                # ----------------
       'http://download.geofabrik.de/europe/austria-latest.osm.pbf',                                        #
       'de',                                                                                                #
       'en,de,fr,es,hu,sl,sla',                                                                             #
       '46.36', '9.52', '49.03', '17.17',                                                                   #
       '0',                                                                                                 #
       '48.2218', '16.4079',                                                                                # Wien
       '12',                                                                                                #
       'AUT', 'Österreich', 'Austria'                                                                       #
    ],
    [  'Freizeitkarte_CHE',                                                                                 # Schweiz (CHE)
       'NA',                                                                                                # ------------
       'http://download.geofabrik.de/europe/switzerland-latest.osm.pbf',                                    #
       'de',                                                                                                #
       'en,de,fr,es,it,rm,gsw',                                                                             #
       '45.81', '5.95', '47.82', '10.50',                                                                   #
       '0',                                                                                                 #
       '47.3634', '8.5417',                                                                                 # Zürich
       '12',                                                                                                #
       'CHE', 'Schweiz', 'Switzerland'                                                                      #
    ],
    [  'Freizeitkarte_ESP',                                                                                 # Spanien (ESP)
       'NA',                                                                                                # -------------
       'http://download.geofabrik.de/europe/spain-latest.osm.pbf',                                          #
       'en',                                                                                                #
       'en,de,fr,es,ca,gl,eu',                                                                              #
       '35.26', '-9.78', '44.15', '5.10',                                                                   #
       '1',                                                                                                 #
       '40.4137', '-3.6930',                                                                                # Madrid
       '12',                                                                                                #
       'ESP', 'Spanien', 'Spain'                                                                            #
    ],
    [  'Freizeitkarte_ITA',                                                                                 # Italien (ITA)
       'NA',                                                                                                # -------------
       'http://download.geofabrik.de/europe/italy-latest.osm.pbf',                                          #
       'it',                                                                                                #
       'en,de,fr,es,it,sl',                                                                                 #
       '35.07', '6.60', '47.11', '19.13',                                                                   #
       '1',                                                                                                 #
       '41.8897', '12.4781',                                                                                # Rom
       '12',                                                                                                #
       'ITA', 'Italien', 'Italy'                                                                            #
    ],
    [  'Freizeitkarte_HRV',                                                                                 # Kroatien (HRV)
       'NA',                                                                                                # --------------
       'http://download.geofabrik.de/europe/croatia-latest.osm.pbf',                                        #
       'en',                                                                                                #
       'en,de,fr,es,hr,sr',                                                                                 #
       '42.16', '13.08', '46.56', '19.46',                                                                  #
       '1',                                                                                                 #
       '45.8140', '15.9794',                                                                                # Zagreb
       '12',                                                                                                #
       'HRV', 'Kroatien', 'Croatia'                                                                         #
    ],
    [  'Freizeitkarte_NLD',                                                                                 # Niederlande (NLD)
       'NA',                                                                                                # -----------------
       'http://download.geofabrik.de/europe/netherlands-latest.osm.pbf',                                    #
       'en',                                                                                                #
       'en,de,fr,es,nl,fy',                                                                                 #
       '50.74', '2.99', '54.02', '7.24',                                                                    #
       '1',                                                                                                 #
       '52.3842', '4.9072',                                                                                 # Amsterdam
       '12',                                                                                                #
       'NLD', 'Niederlande', 'Netherlands'                                                                  #
    ],
    # Klein-/Stadtstaaten (bereits in Pluskarten enthalten)
    [  'Freizeitkarte_AND',                                                                                 # Andorra (AND)
       'NA',                                                                                                # -------------
       'http://download.geofabrik.de/europe/andorra-latest.osm.pbf',                                        #
       'en',                                                                                                #
       'en,de,fr,es,ca',                                                                                    #
       '42.42', '1.41', '42.66', '1.79',                                                                    #
       '0',                                                                                                 #
       '42.5050', '1.5229',                                                                                 # Andorra la Vella
       '12',                                                                                                #
       'AND', 'Andorra', 'Andorra'                                                                          #
    ],
    [  'Freizeitkarte_LIE',                                                                                 # Liechtenstein (LIE)
       'NA',                                                                                                # -------------------
       'http://download.geofabrik.de/europe/liechtenstein-latest.osm.pbf',                                  #
       'en',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '47.04', '9.47', '47.28', '9.64',                                                                    #
       '0',                                                                                                 #
       '47.1368', '9.5211',                                                                                 # Vaduz
       '12',                                                                                                #
       'LIE', 'Liechtenstein', 'Liechtenstein'                                                              #
    ],
    [  'Freizeitkarte_MCO',                                                                                 # Monaco (MCO)
       'NA',                                                                                                # ------------
       'http://download.geofabrik.de/europe/monaco-latest.osm.pbf',                                         #
       'en',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '43.72', '7.40', '43.76', '7.45',                                                                    #
       '1',                                                                                                 #
       '43.7378', '7.4230',                                                                                 # Monaco
       '12',                                                                                                #
       'MCO', 'Monaco', 'Monaco'                                                                            #
    ],
    [  'Freizeitkarte_LUX',                                                                                 # Luxemburg (LUX)
       'NA',                                                                                                # ---------------
       'http://download.geofabrik.de/europe/luxembourg-latest.osm.pbf',                                     #
       'en',                                                                                                #
       'en,de,fr,es,lb',                                                                                    #
       '49.44', '5.73', '50.19', '6.54',                                                                    #
       '0',                                                                                                 #
       '49.6080', '6.1409',                                                                                 # Luxemburg
       '12',                                                                                                #
       'LUX', 'Luxemburg', 'Luxembourg'                                                                     #
    ],
    # Testkarten
    [  'Freizeitkarte_US_MIDWEST',                                                                          # USA (Mittelwesten)
       'NA',                                                                                                # ------------------
       'http://download.geofabrik.de/north-america/us-midwest-latest.osm.pbf',                              #
       'en',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '35.98', '-104.06', '49.41', '-80.50',                                                               #
       '1',                                                                                                 #
       '41.7068', '-87.5748',                                                                               # Chicago
       '12',                                                                                                #
       'US_MIDWEST', 'USA Mittelwesten', 'US Midwest'                                                       #
    ],
    [  'Freizeitkarte_US_NORTHEAST',                                                                        # USA (Nordosten)
       'NA',                                                                                                # ---------------
       'http://download.geofabrik.de/north-america/us-northeast-latest.osm.pbf',                            #
       'en',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '38.74', '-80.53', '47.47', '-66.87',                                                                #
       '1',                                                                                                 #
       '40.7117', '-74.0496',                                                                               # New York
       '12',                                                                                                #
       'US_NORTHEAST', 'USA Nordosten', 'US Northeast'                                                      #
    ],
    [  'Freizeitkarte_US_SOUTH',                                                                            # USA (Süden)
       'NA',                                                                                                # -----------
       'http://download.geofabrik.de/north-america/us-south-latest.osm.pbf',                                #
       'en',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '24.20', '-106.65', '40.65', '-71.50',                                                               #
       '1',                                                                                                 #
       '33.7577', '-84.4176',                                                                               # Atlanta
       '12',                                                                                                #
       'US_SOUTH', 'USA Süden', 'US South'                                                                  #   
    ],
    [  'Freizeitkarte_US_WEST',                                                                             # USA (Westen)
       'NA',                                                                                                # ------------
       'http://download.geofabrik.de/north-america/us-west-latest.osm.pbf',                                 #
       'en',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '31.32', '-133.07', '49.46', '-102.03',                                                              #
       '1',                                                                                                 #
       '34.0896', '-118.1895',                                                                              # Los Angeles
       '12',                                                                                                #
       'US_WEST', 'USA Westen', 'US West'                                                                   #
    ],
    [  'Freizeitkarte_US_WASHINGTON',                                                                       # Washington (USA, Bundesstaat)
       'NA',                                                                                                # -----------------------------
       'http://download.geofabrik.de/north-america/us/washington-latest.osm.pbf',                           #
       'en',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '45.54', '-126.75', '49.01', '-116.91',                                                              #
       '1',                                                                                                 #
       '47.6076', '-122.3566',                                                                              #  Seattle
       '12',                                                                                                #
       'US_WASHINGTON', 'USA Washington', 'US Washington'                                                   #
    ],
    [  'Freizeitkarte_ISR_PSE',                                                                             # Israel + Palästina
       'NA',                                                                                                # ------------------
       'http://download.geofabrik.de/asia/israel-and-palestine-latest.osm.pbf',                             #
       'en',                                                                                                #
       'en,de,fr,es,he,ar',                                                                                 #
       '29.37', '34.07', '33.36', '35.92',                                                                  #
       '1',                                                                                                 #
       '31.7784', '35.2249',                                                                                #  Jerusalem
       '12',                                                                                                #
       'ISR_PSE', 'Israel+Palästina', 'Israel+Palestine'                                                    #
    ],
    [  'Freizeitkarte_CZE',                                                                                 # Tschechische Republik (CZE)
       'NA',                                                                                                # ---------------------------
       'https://download.geofabrik.de/europe/czech-republic-latest.osm.pbf',                                #
       'en',                                                                                                #
       'en,de,fr,es,cs',                                                                                    #
       '48.54', '12.08', '51.07', '18.87',                                                                  #
       '0',                                                                                                 #
       '50.0878', '14.4149',                                                                                # Prag
       '12',                                                                                                #
       'CZE', 'Tschechien', 'Czechia'                                                                       #
    ],
    # übergeordnete Extrakte
    [  'Freizeitkarte_EUROPE',                                                                              # Europa (geografisch)
       'NA',                                                                                                # --------------------
       'http://download.geofabrik.de/europe-latest.osm.pbf',                                                # Parent Map
       'en',                                                                                                # Dummy
       'en',                                                                                                # Dummy
       '30.81', '-34.49', '81.48', '45.00',                                                                 # Dummy
       '1',                                                                                                 # Dummy
       '51.9613', '7.6251',                                                                                 # Dummy (Münster)
       '12',                                                                                                # Dummy
       'EUROPE', 'Europa', 'Europe'                                                                         # Dummy
    ],
    [  'Freizeitkarte_USSR',                                                                                # ehemalige UDSSR (Sowjetunion)
       'NA',                                                                                                # -----------------------------
       'http://data.gis-lab.info/osm_dump/dump/latest/local.osm.pbf',                                       # Parent Map
       'en',                                                                                                # Dummy
       'en',                                                                                                # Dummy
       '30.81', '-34.49', '81.48', '45.00',                                                                 # Dummy
       '1',                                                                                                 # Dummy
       '51.9613', '7.6251',                                                                                 # Dummy (Münster)
       '12',                                                                                                # Dummy
       'USSR', 'UDSSR', 'USSR'                                                                              # Dummy 
    ],
    # Karten basierend auf EUROPE Extrakt
    [  'Freizeitkarte_AUT+',                                                                                # Österreich (AUT)
       'Freizeitkarte_EUROPE',                                                                              # ----------------
       'NA',                                                                                                #
       'de',                                                                                                #
       'en,de,fr,es,hu,sl,sla',                                                                             #
       '45.80', '9.00', '49.60', '17.80',                                                                   #
       '0',                                                                                                 #
       '48.2218', '16.4079',                                                                                # Wien
       '12',                                                                                                #
       'AUT+', 'Österreich+', 'Austria+'                                                                    #
    ],
    [  'Freizeitkarte_CHE+',                                                                                # Schweiz (CHE)
       'Freizeitkarte_EUROPE',                                                                              # -------------
       'NA',                                                                                                #
       'de',                                                                                                #
       'en,de,fr,es,it,rm,gsw',                                                                             #
       '45.20', '5.40', '48.40', '11.10',                                                                   #
       '0',                                                                                                 #
       '47.3634', '8.5417',                                                                                 # Zürich
       '12',                                                                                                #
       'CHE+', 'Schweiz+', 'Switzerland+'                                                                   #
    ],
    [  'Freizeitkarte_DEU+NORTH',                                                                           # Deutschland (DEU, Nord)
       'Freizeitkarte_EUROPE',                                                                              # -----------------------
       'NA',                                                                                                #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '49.83', '5.30', '55.70', '15.60',                                                                   #
       '1',                                                                                                 #
       '52.5174', '13.4109',                                                                                # Berlin
       '12',                                                                                                #
       'DEU+NORTH', 'Deutschland+ Nord', 'Germany+ North'                                                   #
    ],
    [  'Freizeitkarte_DEU+SOUTH',                                                                           # Deutschland (DEU, Süd)
       'Freizeitkarte_EUROPE',                                                                              # ----------------------
       'NA',                                                                                                #
       'de',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '46.70', '5.50', '50.35', '15.24',                                                                   #
       '0',                                                                                                 #
       '48.1296', '11.5861',                                                                                # München
       '12',                                                                                                #
       'DEU+SOUTH', 'Deutschland+ Süd', 'Germany+ South'                                                    #
    ],
    [  'Freizeitkarte_ALPS_WEST',                                                                           # Alpen (ALPS, West)
       'Freizeitkarte_EUROPE',                                                                              # ------------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,it',                                                                                    #
       '42.91', '4.20', '48.65', '10.90',                                                                   #
       '1',                                                                                                 #
       '45.8953', '6.9257',                                                                                 # Charmonix-Mont Blanc
       '12',                                                                                                #
       'ALPS_WEST', 'Alpen West', 'Alps West'                                                               #
    ],
    [  'Freizeitkarte_ALPS_EAST',                                                                           # Alpen (ALPS, Ost)
       'Freizeitkarte_EUROPE',                                                                              # -----------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,it,sl',                                                                                 #
       '44.32', '9.30', '49.06', '17.41',                                                                   #
       '1',                                                                                                 #
       '47.2607', '11.3969',                                                                                # Innsbruck
       '12',                                                                                                #
       'ALPS_EAST', 'Alpen Ost', 'Alps East'                                                                #
    ],
    [  'Freizeitkarte_POL+',                                                                                # Polen (POL)
       'Freizeitkarte_EUROPE',                                                                              # -----------
       'NA',                                                                                                #
       'pl',                                                                                                #
       'en,de,fr,es,pl',                                                                                    #
       '48.40', '13.50', '55.60', '24.70',                                                                  #
       '1',                                                                                                 #
       '52.2325', '21.0454',                                                                                # Warschau
       '12',                                                                                                #
       'POL+', 'Polen+', 'Poland+'                                                                          #
    ],
    [  'Freizeitkarte_CZE+',                                                                                # Tschechische Republik (CZE)
       'Freizeitkarte_EUROPE',                                                                              # ---------------------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,cs',                                                                                    #
       '48.00', '11.50', '51.70', '19.50',                                                                  #
       '0',                                                                                                 #
       '50.0878', '14.4149',                                                                                # Prag
       '12',                                                                                                #
       'CZE+', 'Tschechien+', 'Czechia+'                                                                    #
    ],
    [  'Freizeitkarte_LUX+',                                                                                # Luxemburg (LUX)
       'Freizeitkarte_EUROPE',                                                                              # ---------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,lb',                                                                                    #
       '48.80', '5.10', '50.80', '7.10',                                                                    #
       '0',                                                                                                 #
       '49.6080', '6.1409',                                                                                 # Luxemburg
       '12',                                                                                                #
       'LUX+', 'Luxemburg+', 'Luxembourg+'                                                                  #
    ],
    [  'Freizeitkarte_BEL+',                                                                                # Belgien (BEL)
       'Freizeitkarte_EUROPE',                                                                              # -------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,nl',                                                                                    #
       '48.90', '1.80', '52.10', '7.00',                                                                    #
       '1',                                                                                                 #
       '50.8443', '4.3603',                                                                                 # Brüssel
       '12',                                                                                                #
       'BEL+', 'Belgien+', 'Belgium+'                                                                       #
    ],
    [  'Freizeitkarte_NLD+',                                                                                # Niederlande (NLD)
       'Freizeitkarte_EUROPE',                                                                              # -----------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,nl,fy',                                                                                 #
       '50.20', '2.50', '54.30', '7.80',                                                                    #
       '1',                                                                                                 #
       '52.3842', '4.9072',                                                                                 # Amsterdam
       '12',                                                                                                #
       'NLD+', 'Niederlande+', 'Netherlands+'                                                               #
    ],
    [  'Freizeitkarte_FRA+NORTHWEST',                                                                       # Frankreich (FRA, Nordwest)
       'Freizeitkarte_EUROPE',                                                                              # --------------------------
       'NA',                                                                                                #
       'fr',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '46.27', '-6.30', '51.59', '3.18',                                                                   #
       '1',                                                                                                 #
       '50.6309', '3.0663',                                                                                 # Lille
       '12',                                                                                                #
       'FRA+NORTHWEST', 'Frankreich+ Nordwest', 'France+ Northwest'                                         #
    ],
    [  'Freizeitkarte_FRA+NORTHEAST',                                                                       # Frankreich (FRA, Nordost)
       'Freizeitkarte_EUROPE',                                                                              # -------------------------
       'NA',                                                                                                #
       'fr',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '46.27', '1.68', '51.60', '9.00',                                                                    #
       '0',                                                                                                 #
       '48.85346', '2.35314',                                                                               # Paris
       '12',                                                                                                #
       'FRA+NORTHEAST', 'Frankreich+ Nordost', 'France+ Northeast'                                          #
    ],
    [  'Freizeitkarte_FRA+SOUTHWEST',                                                                       # Frankreich (FRA, Südwest)
       'Freizeitkarte_EUROPE',                                                                              # -------------------------
       'NA',                                                                                                #
       'fr',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '41.50', '-4.73', '46.87', '3.41',                                                                   #
       '1',                                                                                                 #
       '44.8455', '-0.5661',                                                                                # Bordeaux
       '12',                                                                                                #
       'FRA+SOUTHWEST', 'Frankreich+ Südwest', 'France+ Southwest'                                          #
    ],
    [  'Freizeitkarte_FRA+SOUTHEAST',                                                                       # Frankreich (FRA, Südost)
       'Freizeitkarte_EUROPE',                                                                              # ------------------------
       'NA',                                                                                                #
       'fr',                                                                                                #
       'en,de,fr,es',                                                                                       #
       '41.27', '2.15', '46.87', '10.04',                                                                   #
       '1',                                                                                                 #
       '43.2933', '5.3652',                                                                                 # Marseille
       '12',                                                                                                #
       'FRA+SOUTHEAST', 'Frankreich+ Südost', 'France+ Southeast'                                           #
    ],
    [  'Freizeitkarte_CARPATHIAN',                                                                          # Karpaten (CARPATHIAN)
       'Freizeitkarte_EUROPE',                                                                              # ---------------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,pl,sk,uk,ro',                                                                           #
       '43.15', '16.06', '50.25', '27.76',                                                                  #
       '0',                                                                                                 #
       '47.5037', '19.0430',                                                                                # Budapest
       '12',                                                                                                #
       'CARPATHIAN', 'Karpaten', 'Carpathian'                                                               #
    ],
    [  'Freizeitkarte_PYRENEES',                                                                            # Pyrenäen (PYRENEES)
       'Freizeitkarte_EUROPE',                                                                              # -------------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,ca,eu',                                                                                 #
       '40.79', '-2.55', '43.98', '3.56',                                                                   #
       '1',                                                                                                 #
       '41.3999', '2.1618',                                                                                 # Barcelona
       '12',                                                                                                #
       'PYRENEES', 'Pyrenäen', 'Pyrenees'                                                                   #
    ],
    [  'Freizeitkarte_DNK+',                                                                                # Dänemark (DNK)
       'Freizeitkarte_EUROPE',                                                                              # --------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,da',                                                                                    #
       '53.90', '7.10', '58.60', '16.59',                                                                   #
       '1',                                                                                                 #
       '55.6865', '12.5988',                                                                                # Kopenhagen
       '12',                                                                                                #
       'DNK+', 'Dänemark+', 'Denmark+'                                                                      #
    ],
    [  'Freizeitkarte_NOR+NORTH',                                                                           # Norwegen (NOR, Nord)
       'Freizeitkarte_EUROPE',                                                                              # --------------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,no',                                                                                    #
       '64.10', '8.55', '72.00', '32.40',                                                                   #
       '1',                                                                                                 #
       '69.6544', '18.9336',                                                                                # Tromso
       '12',                                                                                                #
       'NOR+NORTH', 'Norwegen+ Nord', 'Norway+ North'                                                       #
    ],
    [  'Freizeitkarte_NOR+SOUTH',                                                                           # Norwegen (NOR, Süd)
       'Freizeitkarte_EUROPE',                                                                              # -------------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,no',                                                                                    #
       '57.20', '3.50', '66.14', '15.47',                                                                   #
       '1',                                                                                                 #
       '59.9178', '10.7476',                                                                                # Oslo
       '12',                                                                                                #
       'NOR+SOUTH', 'Norwegen+ Süd', 'Norway+ South'                                                        #
    ],
    [  'Freizeitkarte_SWE+',                                                                                # Schweden (SWE)
       'Freizeitkarte_EUROPE',                                                                              # --------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,sv',                                                                                    #
       '54.50', '10.00', '69.70', '24.80',                                                                  #
       '1',                                                                                                 #
       '59.3204', '18.0630',                                                                                # Stockholm
       '12',                                                                                                #
       'SWE+', 'Schweden+', 'Sweden+'                                                                       #
    ],
    [  'Freizeitkarte_FIN+',                                                                                # Finnland (FIN)
       'Freizeitkarte_EUROPE',                                                                              # --------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,fi',                                                                                    #
       '58.90', '18.50', '70.70', '32.20',                                                                  #
       '1',                                                                                                 #
       '60.1577', '24.9329',                                                                                # Helsinki
       '12',                                                                                                #
       'FIN+', 'Finnland+', 'Finland+'                                                                      #
    ],
    [  'Freizeitkarte_ESP+',                                                                                # Spanien (ESP)
       'Freizeitkarte_EUROPE',                                                                              # -------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,ca,gl,eu',                                                                              #
       '35.26', '-10.20', '44.60', '5.20',                                                                  #
       '1',                                                                                                 #
       '40.4137', '-3.6930',                                                                                # Madrid
       '12',                                                                                                #
       'ESP+', 'Spanien+', 'Spain+'                                                                         #
    ],
    [  'Freizeitkarte_PRT+',                                                                                # Portugal (PRT)
       'Freizeitkarte_EUROPE',                                                                              # --------------
       'NA',                                                                                                #
       'pt',                                                                                                #
       'en,de,fr,es,pt',                                                                                    #
       '36.10', '-10.40', '42.80', '-5.60',                                                                 #
       '1',                                                                                                 #
       '38.7083', '-9.1425',                                                                                # Lissabon
       '12',                                                                                                #
       'PRT+', 'Portugal+', 'Portugal+'                                                                     #
    ],
    [  'Freizeitkarte_GRC+',                                                                                # Griechenland (GRC)
       'Freizeitkarte_EUROPE',                                                                              # ------------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,el',                                                                                    #
       '34.10', '18.60', '42.30', '30.30',                                                                  #
       '1',                                                                                                 #
       '37.9374', '23.6577',                                                                                # Athen
       '12',                                                                                                #
       'GRC+', 'Griechenland+', 'Greece+'                                                                   #
    ],
    [  'Freizeitkarte_ITA+NORTH',                                                                           # Italien (ITA, Nord)
       'Freizeitkarte_EUROPE',                                                                              # -------------------
       'NA',                                                                                                #
       'it',                                                                                                #
       'en,de,fr,es,it,sl',                                                                                 #
       '43.09', '6.08', '47.63', '14.90',                                                                   #
       '1',                                                                                                 #
       '45.4701', '9.1895',                                                                                 # Mailand
       '12',                                                                                                #
       'ITA+NORTH', 'Italien+ Nord', 'Italy+ North'                                                         #
    ],
    [  'Freizeitkarte_ITA+SOUTH',                                                                           # Italien (ITA, Süd)
       'Freizeitkarte_EUROPE',                                                                              # ------------------
       'NA',                                                                                                #
       'it',                                                                                                #
       'en,de,fr,es,it',                                                                                    #
       '34.45', '7.38', '44.16', '19.33',                                                                   #
       '1',                                                                                                 #
       '41.8897', '12.4781',                                                                                # Rom
       '12',                                                                                                #
       'ITA+SOUTH', 'Italien+ Süd', 'Italy+ South'                                                          #
    ],
    [  'Freizeitkarte_ALB+',                                                                                # Albanien (ALB)
       'Freizeitkarte_EUROPE',                                                                              # --------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,sq',                                                                                    #
       '39.00', '18.50', '43.30', '21.70',                                                                  #
       '1',                                                                                                 #
       '41.3262', '19.8449',                                                                                # Tirana
       '12',                                                                                                #
       'ALB+', 'Albanien+', 'Albania+'                                                                      #
    ],
    [  'Freizeitkarte_BGR+',                                                                                # Bulgarien (BGR)
       'Freizeitkarte_EUROPE',                                                                              # ---------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,bg',                                                                                    #
       '40.60', '21.80', '44.80', '29.50',                                                                  #
       '1',                                                                                                 #
       '42.6941', '23.3124',                                                                                # Sofia
       '12',                                                                                                #
       'BGR+', 'Bulgarien+', 'Bulgaria+'                                                                    #
    ],
    [  'Freizeitkarte_BIH+',                                                                                # Bosnien und Herzegowina (BIH)
       'Freizeitkarte_EUROPE',                                                                              # -----------------------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,bs,sr,hr',                                                                              #
       '42.00', '15.10', '45.90', '20.20',                                                                  #
       '1',                                                                                                 #
       '43.8501', '18.3920',                                                                                # Sarajevo
       '12',                                                                                                #
       'BIH+', 'Bosnien-Herzegowina+', 'Bosnia-Herzegovina+'                                                #
    ],
    [  'Freizeitkarte_HRV+',                                                                                # Kroatien (HRV)
       'Freizeitkarte_EUROPE',                                                                              # --------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,hr,sr',                                                                                 #
       '41.60', '12.60', '47.20', '20.00',                                                                  #
       '1',                                                                                                 #
       '45.8140', '15.9794',                                                                                # Zagreb
       '12',                                                                                                #
       'HRV+', 'Kroatien+', 'Croatia+'                                                                      #
    ],
    [  'Freizeitkarte_HUN+',                                                                                # Ungarn (HUN)
       'Freizeitkarte_EUROPE',                                                                              # ------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,hu',                                                                                    #
       '45.10', '15.50', '49.20', '23.50',                                                                  #
       '0',                                                                                                 #
       '47.4977', '19.0430',                                                                                # Budapest
       '12',                                                                                                #
       'HUN+', 'Ungarn+', 'Hungary+'                                                                        #
    ],
    [  'Freizeitkarte_MKD+',                                                                                # Mazedonien (MKD)
       'Freizeitkarte_EUROPE',                                                                              # ----------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,mk,sq',                                                                                 #
       '40.30', '19.90', '43.00', '23.60',                                                                  #
       '1',                                                                                                 #
       '41.9962', '21.4362',                                                                                # Skopje
       '12',                                                                                                #
       'MKD+', 'Mazedonien+', 'Macedonia+'                                                                  #
    ],
    [  'Freizeitkarte_MNE+',                                                                                # Montenegro (MNE)
       'Freizeitkarte_EUROPE',                                                                              # ----------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,cnr,sr,bs,sq,hr',                                                                       #
       '41.20', '17.80', '44.20', '21.00',                                                                  #
       '1',                                                                                                 #
       '42.4438', '19.2616',                                                                                # Podgorica
       '12',                                                                                                #
       'MNE+', 'Montenegro+', 'Montenegro+'                                                                 #
    ],
    [  'Freizeitkarte_ROU+',                                                                                # Rumänien (ROU)
       'Freizeitkarte_EUROPE',                                                                              # --------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,ro',                                                                                    #
       '43.00', '19.70', '48.90', '30.60',                                                                  #
       '1',                                                                                                 #
       '44.4355', '26.1006',                                                                                # Bukarest
       '12',                                                                                                #
       'ROU+', 'Rumänien+', 'Romania+'                                                                      #
    ],
    [  'Freizeitkarte_SRB+',                                                                                # Serbien (SRB)
       'Freizeitkarte_EUROPE',                                                                              # -------------
       'NA',                                                                                                # (inkl. Kosovo)
       'en',                                                                                                #
       'en,de,fr,es,sr,sq',                                                                                 #
       '41.30', '18.20', '46.80', '23.60',                                                                  #
       '1',                                                                                                 #
       '44.8219', '20.4549',                                                                                # Belgrad
       '12',                                                                                                #
       'SRB+', 'Serbien+', 'Serbia+'                                                                        #
    ],
    [  'Freizeitkarte_UKR+',                                                                                # Ukraine (UKR)
       'Freizeitkarte_EUROPE',                                                                              # -------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,uk,ru',                                                                                 #
       '43.60', '21.50', '53.00', '40.80',                                                                  #
       '1',                                                                                                 #
       '50.4579', '30.5281',                                                                                # Kiew
       '12',                                                                                                #
       'UKR+', 'Ukraine+', 'Ukraine+'                                                                       #
    ],
    [  'Freizeitkarte_MDA+',                                                                                # Moldawien (MDA)
       'Freizeitkarte_EUROPE',                                                                              # ---------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,ro,ru',                                                                                 #
       '44.90', '26.00', '49.10', '30.80',                                                                  #
       '1',                                                                                                 #
       '47.0101', '28.8607',                                                                                # Kischinau
       '12',                                                                                                #
       'MDA+', 'Moldawien+', 'Moldova+'                                                                     #
    ],
    [  'Freizeitkarte_BLR+',                                                                                # Weissrussland, Belarus (BLR)
       'Freizeitkarte_EUROPE',                                                                              # ----------------------------
       'NA',                                                                                                #
       'ru',                                                                                                #
       'en,de,fr,es,be,ru',                                                                                 #
       '50.70', '22.60', '56.80', '33.40',                                                                  #
       '0',                                                                                                 #
       '53.9017', '27.5601',                                                                                # Minsk
       '12',                                                                                                #
       'BLR+', 'Weißrussland+', 'Belarus+'                                                                  #
    ],
    [  'Freizeitkarte_LTU+',                                                                                # Litauen (LTU)
       'Freizeitkarte_EUROPE',                                                                              # -------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,lt,pl',                                                                                 #
       '53.30', '20.10', '57.00', '27.40',                                                                  #
       '1',                                                                                                 #
       '54.6860', '25.2895',                                                                                # Vilnius (Wilna)
       '12',                                                                                                #
       'LTU+', 'Litauen+', 'Lithuania+'                                                                     #
    ],
    [  'Freizeitkarte_LVA+',                                                                                # Lettland (LVA)
       'Freizeitkarte_EUROPE',                                                                              # --------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,lv,ru',                                                                                 #
       '55.10', '20.10', '58.70', '28.80',                                                                  #
       '1',                                                                                                 #
       '56.9790', '24.1121',                                                                                # Riga
       '12',                                                                                                #
       'LVA+', 'Lettland+', 'Latvia+'                                                                       #
    ],
    [  'Freizeitkarte_EST+',                                                                                # Estland (EST)
       'Freizeitkarte_EUROPE',                                                                              # -------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,et',                                                                                    #
       '56.90', '20.80', '60.50', '28.80',                                                                  #
       '1',                                                                                                 #
       '59.4552', '24.8046',                                                                                # Tallinn
       '12',                                                                                                #
       'EST+', 'Estland+', 'Estonia+'                                                                       #
    ],
    [  'Freizeitkarte_RUS+KGD',                                                                             # Russland, Kaliningrad (RUS-KGD)
       'Freizeitkarte_EUROPE',                                                                              # -------------------------------
       'NA',                                                                                                #
       'ru',                                                                                                #
       'en,de,fr,es,ru',                                                                                    #
       '53.70', '18.80', '56.00', '23.50',                                                                  #
       '1',                                                                                                 #
       '54.7046', '20.4371',                                                                                # Kaliningrad
       '12',                                                                                                #
       'RUS+KGD', 'Russland Kaliningrad+', 'Russia Kaliningrad+'
    ],
    [  'Freizeitkarte_BALEARICS',                                                                           # Balearen
       'Freizeitkarte_EUROPE',                                                                              # --------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,ca',                                                                                    #
       '38.35', '0.79', '40.39', '4.67',                                                                    #
       '1',                                                                                                 #
       '39.5350', '2.6218',                                                                                 # Palma de Mallorca
       '12',                                                                                                #
       'BALEARICS', 'Balearen', 'Balearics'                                                                 #
    ],
    [  'Freizeitkarte_SVK+',                                                                                # Slowakei (SVK)
       'Freizeitkarte_EUROPE',                                                                              # --------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,sk',                                                                                    #
       '47.10', '16.20', '50.20', '23.20',                                                                  #
       '0',                                                                                                 #
       '48.1352', '17.1206',                                                                                # Bratislava
       '12',                                                                                                #
       'SVK+', 'Slowakei+', 'Slovakia+'                                                                     #
    ],
    [  'Freizeitkarte_SVN+',                                                                                # Slowenien (SVN)
       'Freizeitkarte_EUROPE',                                                                              # ---------------
       'NA',                                                                                                #
       'en',                                                                                                #
       'en,de,fr,es,sl',                                                                                    #
       '44.80', '12.80', '47.50', '17.20',                                                                  #
       '1',                                                                                                 #
       '46.0480', '14.5313',                                                                                # Ljubljana
       '12',                                                                                                #
       'SVN+', 'Slowenien+', 'Slovenia+'                                                                    #
    ],
    [  'Freizeitkarte_MADEIRA',                                                                             # Madeira
       'Freizeitkarte_EUROPE',                                                                              # -------
       'NA',                                                                                                #
       'pt',                                                                                                #
       'en,de,fr,es,pt',                                                                                    #
       '32.03', '-17.76', '33.52', '-15.76',                                                                #
       '1',                                                                                                 #
       '32.6472', '-16.9051',                                                                               # Funchal
       '12',                                                                                                #
       'MADEIRA', 'Madeira', 'Madeira'                                                                      #
    ],
    # Karten basierend auf geofabrik Extrakten
    [  'Freizeitkarte_RUS_CENTRAL',                                                                         # RUS-Zentral (Föderationskreis)
       'NA',                                                                                                # ------------------------------
       'https://download.geofabrik.de/russia/central-fed-district-latest.osm.pbf',                          #
       'ru',                                                                                                #
       'en,de,fr,es,ru',                                                                                    #
       '49.50', '30.69', '59.66', '47.68',                                                                  #
       '1',                                                                                                 #
       '55.7427', '37.6110',                                                                                # Moskau
       '12',                                                                                                #
       'RUS_CENTRAL', 'Russland FK Zentralrussland', 'Russia Central FD'                                    #
    ],
    [  'Freizeitkarte_RUS_CRIMEA',                                                                          # RUS-Krim (Föderationskreis)
       'NA',                                                                                                # ---------------------------
       'https://download.geofabrik.de/russia/crimean-fed-district-latest.osm.pbf',                          #
       'ru',                                                                                                #
       'en,de,fr,es,ru',                                                                                    #
       '44.13', '32.15', '46.29', '36.69',                                                                  #
       '1',                                                                                                 #
       '44.9369', '34.0974',                                                                                # Simferopol
       '12',                                                                                                #
       'RUS_CRIMEA', 'Russland FK Krim', 'Russia Crimean FD'                                                #
    ],
    [  'Freizeitkarte_RUS_NORTHCAUCASUS',                                                                   # RUS-Nordkaukasus (Föderationskreis)
       'NA',                                                                                                # -----------------------------------
       'https://download.geofabrik.de/russia/north-caucasus-fed-district-latest.osm.pbf',                   #
       'ru',                                                                                                #
       'en,de,fr,es,ru',                                                                                    #
       '41.13', '40.67', '46.24', '48.87',                                                                  #
       '1',                                                                                                 #
       '44.0497', '43.0455',                                                                                # Pjatigorsk
       '12',                                                                                                #
       'RUS_NORTHCAUCASUS', 'Russland FK Nordkaukasus', 'Russia North Caucasian FD'                         #
    ],
    [  'Freizeitkarte_RUS_NORTHWEST',                                                                       # RUS-Nordwest (Föderationskreis)
       'NA',                                                                                                # -------------------------------
       'https://download.geofabrik.de/russia/northwestern-fed-district-latest.osm.pbf',                     #
       'ru',                                                                                                #
       'en,de,fr,es,ru',                                                                                    #
       '54.31', '19.40', '82.55', '72.63',                                                                  #
       '1',                                                                                                 #
       '59.9287', '30.3099',                                                                                # Sankt Petersburg
       '12',                                                                                                #
       'RUS_NORTHWEST', 'Russland FK Nordwestrussland', 'Russia Northwestern FD'                            #
    ],
    [  'Freizeitkarte_RUS_SOUTH',                                                                           # RUS-Süd (Föderationskreis)
       'NA',                                                                                                # --------------------------
       'https://download.geofabrik.de/russia/south-fed-district-latest.osm.pbf',                            #
       'ru',                                                                                                #
       'en,de,fr,es,ru',                                                                                    #
       '43.18', '36.52', '51.26', '49.94',                                                                  #
       '1',                                                                                                 #
       '47.2496', '39.6716',                                                                                # Rostow am Don
       '12',                                                                                                #
       'RUS_SOUTH', 'Russland FK Südrussland', 'Russia Southern FD'                                         #
    ],
    [  'Freizeitkarte_RUS_VOLGA',                                                                           # RUS-Wolga (Föderationskreis)
       'NA',                                                                                                # ----------------------------
       'https://download.geofabrik.de/russia/volga-fed-district-latest.osm.pbf',                            #
       'ru',                                                                                                #
       'en,de,fr,es,ru',                                                                                    #
       '49.75', '41.72', '61.72', '61.74',                                                                  #
       '0',                                                                                                 #
       '56.3243', '43.9896',                                                                                # Nischni Nowgorod
       '12',                                                                                                #
       'RUS_VOLGA', 'Russland FK Wolga', 'Russia Volga FD'                                                  #
    ],
    [  'Freizeitkarte_NZL',                                                                                 # Neuseeland (Sonderland)
       'NA',                                                                                                # -----------------------
       'https://download.geofabrik.de/australia-oceania/new-zealand-latest.osm.pbf',                        #
       'en',                                                                                                #
       'en,de,fr,es,mi',                                                                                    #
       '-49.10', '163.90', '-33.30', '179.90',                                                              #
       '1',                                                                                                 #
       '-36.8627', '174.7617',                                                                              # Auckland
       '12',                                                                                                #
       'NZL', 'Neuseeland', 'New Zealand'                                                                   #
    ],
);

# export global vars
our @EXPORT = qw($MAPNAME $MAPPARENT $OSMURL $MAPLANG $MULTILANG $BBMINLAT $BBMINLON $BBMAXLAT $BBMAXLON $HASMEER $STARTLAT $STARTLON $STARTZOOM $NAME $DESCRIPTION_GERMAN $DESCRIPTION_ENGLISH @maps);

# magic
1;