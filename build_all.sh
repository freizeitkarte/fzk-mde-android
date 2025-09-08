#!/bin/bash

# ---------------------------------------
# Program : build_all.sh (Freizeitkarte Android, builden aller Karten)
# Version : 1.0 - 2015/05/24 - Initiale Version
# Version : 1.1 - 2016/11/21 - Erweiterung wegen High-Quality-Elevation
# ==> Nicht mehr nachgeführt. build_serial.sh verwenden
# ---------------------------------------

# 0. Verzeichnis wechseln
# -----------------------
cd
cd Freizeitkarte-Entwicklung-Android

# 0. Landpolygone laden
# ---------------------

sh bootstrap.sh

# 1. Datenextrakte EUROPE + USSR laden (alle parallel ausführen)
# --------------------------------------------------------------

nohup  perl mta.pl load_data Freizeitkarte_EUROPE 1>load_region_eur.out 2>&1 &
nohup  perl mta.pl load_data Freizeitkarte_USSR 1>load_region_ussr.out 2>&1 &

# 2. Karten ausschneiden (alle parallel ausführen)
# ------------------------------------------------

nohup  perl mta.pl extract_maps_eur1 Freizeitkarte_EUROPE 1>extract_maps_eur1.out 2>&1 &
nohup  perl mta.pl extract_maps_eur2 Freizeitkarte_EUROPE 1>extract_maps_eur2.out 2>&1 &
nohup  perl mta.pl extract_maps_eur3 Freizeitkarte_EUROPE 1>extract_maps_eur3.out 2>&1 &
nohup  perl mta.pl extract_maps_ussr Freizeitkarte_USSR 1>extract_maps_ussr.out 2>&1 &

# 3. Karten USSR vorbereiten
# --------------------------

nohup parallel --gnu -j6 perl mta.pl prep_ussr Freizeitkarte_{} ::: RUS+CENTRAL RUS+CRIMEA RUS+NORTHCAUCASUS RUS+NORTHWEST RUS+SOUTH RUS+VOLGA  1>prep_ussr.out 2>&1 &

# 4. Karten EUROPE vorbereiten
# ----------------------------

nohup parallel --gnu -j6 perl mta.pl prep_eur Freizeitkarte_{} ::: NOR+ SWE+ FIN+ DNK+ ESP+ PRT+ GRC+ ITA+NORTH ITA+SOUTH ALB+ BGR+ BIH+ HRV+ HUN+ MKD+ MNE+ ROU+ SRB+ CARPATHIAN PYRENEES FRA+NORTH FRA+EAST FRA+CENTRE FRA+WEST FRA+SOUTH POL+ CZE+ AUT+ CHE+ LUX+ BEL+ NLD+ ALPS+WEST UKR+ MDA+ BLR+ LTU+ LVA+ EST+ RUS+KGD ALPS+EAST DEU+WEST DEU+OST DEU+NORD DEU+SUED BALEARICS SVK+ SVN+ MADEIRA 1>prep_eur.out 2>&1 &

# 5. Karten geofabrik laden
# -------------------------

nohup parallel --gnu -j6 perl mta.pl load_data Freizeitkarte_{} ::: BADEN-WUERTTEMBERG BAYERN BERLIN BRANDENBURG BREMEN HAMBURG HESSEN MECKLENBURG-VORPOMMERN NIEDERSACHSEN NORDRHEIN-WESTFALEN RHEINLAND-PFALZ SAARLAND SACHSEN SACHSEN-ANHALT SCHLESWIG-HOLSTEIN THUERINGEN CYP AZORES CANARY-ISLANDS ISL TUR MLT AND FRO GBR IMN IRL 1>load_data.out 2>&1 &
nohup parallel --gnu -j6 perl mta.pl -hqele load_data Freizeitkarte_{} ::: AUT LUX 1>load_data_hqe.out 2>&1 &

# 6. Karten geofabrik vorbereiten
# -------------------------------

nohup parallel --gnu -j6 perl mta.pl prep_geofabrik Freizeitkarte_{} ::: BADEN-WUERTTEMBERG BAYERN BERLIN BRANDENBURG BREMEN HAMBURG HESSEN MECKLENBURG-VORPOMMERN NIEDERSACHSEN NORDRHEIN-WESTFALEN RHEINLAND-PFALZ SAARLAND SACHSEN SACHSEN-ANHALT SCHLESWIG-HOLSTEIN THUERINGEN CYP AZORES CANARY-ISLANDS ISL TUR MLT AND FRO GBR IMN IRL 1>prep_geofabrik.out 2>&1 &
nohup parallel --gnu -j6 perl mta.pl prep_geofabrik Freizeitkarte_{} ::: AUT LUX 1>prep_geofabrik_hqe.out 2>&1 &

# 7. Karten geofabrik erzeugen (sequentiell)
# ------------------------------------------

nohup sh build_geofabrik_maps.sh 1>build_geofabrik.out 2>&1 &

# 8. Karten geofabrik zippen
# --------------------------

nohup parallel --gnu -j4 perl mta.pl zip Freizeitkarte_{} ::: BADEN-WUERTTEMBERG BAYERN BERLIN BRANDENBURG BREMEN HAMBURG HESSEN MECKLENBURG-VORPOMMERN NIEDERSACHSEN NORDRHEIN-WESTFALEN RHEINLAND-PFALZ SAARLAND SACHSEN SACHSEN-ANHALT SCHLESWIG-HOLSTEIN THUERINGEN CYP AZORES CANARY-ISLANDS ISL TUR MLT AND FRO GBR IMN IRL 1>zip_geofabrik.out 2>&1 &
nohup parallel --gnu -j4 perl mta.pl zip Freizeitkarte_{} ::: AUT LUX 1>zip_geofabrik_hqe.out 2>&1 &

# 9. Karten EUROPE erzeugen (sequentiell)
# ---------------------------------------

nohup sh build_europe_maps.sh 1>build_europe.out 2>&1 &

# 10. Karten EUROPE zippen
# ------------------------

nohup parallel --gnu -j4 perl mta.pl zip Freizeitkarte_{} ::: NOR+ SWE+ FIN+ DNK+ ESP+ PRT+ GRC+ ITA+NORTH ITA+SOUTH ALB+ BGR+ BIH+ HRV+ HUN+ MKD+ MNE+ ROU+ SRB+ CARPATHIAN PYRENEES FRA+NORTH FRA+EAST FRA+CENTRE FRA+WEST FRA+SOUTH POL+ CZE+  AUT+ CHE+ LUX+ BEL+ NLD+ ALPS+WEST UKR+ MDA+ BLR+ LTU+ LVA+ EST+ RUS+KGD ALPS+EAST DEU+WEST DEU+OST DEU+NORD DEU+SUED BALEARICS SVK+ SVN+ MADEIRA 1>zip_europe.out 2>&1 &

# 11. Karten USSR erzeugen (sequentiell)
# --------------------------------------

nohup sh build_ussr_maps.sh 1>build_ussr.out 2>&1 &

# 12. Karten USSR zippen
# ----------------------

nohup parallel --gnu -j4 perl mta.pl zip Freizeitkarte_{} ::: RUS+CENTRAL RUS+CRIMEA RUS+NORTHCAUCASUS RUS+NORTHWEST RUS+SOUTH RUS+VOLGA 1>zip_ussr.out 2>&1 &
