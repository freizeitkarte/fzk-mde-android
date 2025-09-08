//------------------------------------------------------------
// Freizeitkarte-PlugIn fuer das Programm "osmfilter" von Markus Weber.
//
// Version : 1.0.0 - 02.02.2014
// Version : 1.1.0 - 25.05.2014 Verarbeitung von man_made=mineshaft implementiert
// Version : 1.2.0 - 14.07.2014 Umbennung privater Schwimmbecken (leisure/amenity=swimming_pool)
// Version : 1.3.0 - 01.01.2015 Pseudobezeichnung "N.N." bei pass, col, peak entfernt
// Version : 1.4.0 - 17.02.2016 Verarbeitung von station=* implementiert
// Version : 1.5.0 - 29.03.2016 generelles Konzept dokumentiert
// Version : 1.6.0 - 06.06.2016 Flaechen-POIs uebernommen: amenity=bicycle_parking, amenity=bbq; natural=waterfall entfernt
//                              Sonderbehandlung fuer man_made=obelisk; Behandlung von leisure=playground geändert
// Version : 1.7.0 - 20.06.2016 Sonderbehandlung für amenity=charging_station
// Version : 1.8.0 - 23.10.2016 Behandlung von "public_transport=stop_position" für "bus/tram=yes" geändert
// Version : 1.9.0 - 27.11.2016 Behandlung von "tourism=information" und "waterway=weir" geändert
// Version : 17.09 - 04.09.2017 Behandlung von emergency=fire_hydrant und emergency=suction_point ergänzt
// Version : 17.12 - 20.11.2017 Reduzierung der Zusatztexte, Flächengröße bei Zoo/Themenpark/Museum berücksichtigt
//                              Sonderbehandlung für sport=*
// Version : 19.12 - 22.09.2019 Behandlung node_network Punkte, amenity=charging_station (car & bicycle)
// Version : 20.03 - 22.02.2020 Auswertung charging_station zusätzlich motorcar=yes
// Version : 20.09 - 13.08.2020 Art des Shops entfernt: fabric, tailor, second_hand, bed, carpet, interior_decoration, kitchen
//                              Shops hinzugefügt: pastry, perfumery, chocolate
//                              Flaechen-POIs uebernommen: leisure=beach_resort, leisure=bird_hide, leisure=fishing
// Version : 20.12 - 04.09.2020 Verarbeitung "network:type=node_network" entfernt (siehe Verarbeitung in 'osmpp')
// Version : 24.09 - 07.07.2024 Limitierung von man_made=mast auf tower:type=communication
//
// Autor   : Klaus Tockloth
//
// Copyright (C) 2013-2024 FZK-Projekt <freizeitkarte@googlemail.com>
//
// Konzeptionelle Anmerkungen:
// ---------------------------
// Nodes können auf zwei Arten entstanden sein:
// - als erfaßter Node (vom Mapper erzeugt)
// - als abgeleiteter Node (im Rahmen der Vorverarbeitung erzeugt)
//
// Abgeleitete Nodes:
// - für jeden Way, jede Relation, wird ein Node erzeugt
// - bei einem Closed-Way (Fläche) wird der Node in die Mitte der Bounding-Box platziert
// - für jede Fläche wird im Rahmen der Vorverarbeitung die Größe und Gewichtung der Bounding-Box ermittelt
// - (Flächen-) Größe und Gewichtung werden wie alle anderes Tags auch in den abgeleiteten Node übernommen
// - hieran läßt sich ein abgeleiteter von einem erfaßten Node unterscheiden
//
// Viele OSM-Objekte können auf zwei Arten gemapped werden:
// - als Node
// - als Closed-Way (Fläche)
//
// Beispiel: Eine Schule kann erfaßt sein als
// - eigenständiger Node
// - zusätzliches Tag an der Grundfläche
//
// Eine Schule als erfaßter Node:
//
// <node id="60725445" lat="52.1187919" lon="7.1041056" version="1" timestamp="1970-01-01T00:00:01Z" changeset="1">
//   <tag k="name" v="Alexander-Hegius-Grundschule"/>
//   <tag k="amenity" v="school"/>
//   <tag k="wheelchair" v="no"/>
// </node>
//
// Eine Schule als erfaßter Way und abgeleiteter Node:
//
// <way id="85701595" version="1" timestamp="1970-01-01T00:00:01Z" changeset="1">
//  <nd ref="994620681"/>
//   ...
//  <nd ref="994620681"/>
//  <tag k="bBoxArea" v="24539"/>
//  <tag k="bBoxWeight" v="15"/>
//  <tag k="name" v="Maristenschule"/>
//  <tag k="amenity" v="school"/>
//  <tag k="wheelchair" v="yes"/>
// </way>
//
// <node id="1000000085701595" lat="51.6110455" lon="7.1867017">
//   <tag k="bBoxArea" v="24539"/>
//   <tag k="bBoxWeight" v="15"/>
//   <tag k="name" v="Maristenschule"/>
//   <tag k="amenity" v="school"/>
//   <tag k="wheelchair" v="yes"/>
// </node>

// Problem: Doppelte Labels (Namen)
// Wird der Name der Schule als Fläche und als abgeleiteter Node ausgegeben, so würde er auf der Karte doppelt erscheinen.
// Um dies zu vermeiden ist grundsätzlich immer zwischen zwei Fällen zu unterscheiden:
// Fall 1: Ausgabe über die Fläche
// - Der abgeleitete Node ist zu löschen, wohingegen vom Mapper erfaßte Nodes sehr wohl auszugeben sind.
// Fall 2: Ausgabe als Node
// - Beide Node-Typen (erfaßt, abgeleitet) sind auszugeben, wohingegen kein Label für die Fläche angezeigt werden darf.
//
// Allgemeine Anmerkungen:
// -----------------------
// - Neue TAGs sind auch in der Datei "fzk_globals.c" zu definieren.
// - Auf den Key des aktuellen TAGs kann via "*keyp" zugegriffen werden.
// - Auf den Value des aktuellen TAGs kann via "*valp" zugegriffen werden.
// - Auf den Key eines speziellen TAGs kann via "keyp_<tagname>" zugegriffen werden.
// - Auf den Value eines speziellen TAGs kann via "valp_<tagname>" zugegriffen werden.
// - Das Objekt "fzk_key" kann zum Erstellen eines neuen Keys benutzt werden.
// - Das Objekt "fzk_value" kann zum Erstellen eines neuen Values benutzt werden.
// - Ist der Key "bBoxWeight" vorhanden, so wurde der Node aus "way" oder "rel" abgeleitet.
// - Durch die Verknuepfung "&& KEY_NOT_EXISTS(bBoxWeight)" wird ein abgeleiteter Node nicht beruecksichtigt.
//
// Um ein spezielles TAG einzufuehren und zu verarbeiten sind folgende Schritte erforderlich:
// - TAG definieren (in "fzk_globals.c")
// - TAG initialisieren (TAG_INIT)
// - TAG speichern (TAG_STORE)
// - TAG logisch verarbeiten
//
// Debugging:
// ----------
// Beispiel für eine einfache Ergebniskontrolle:
// ~/fzk-mde-android/tools/osmconvert/linux/osmconvert Freizeitkarte_SCHLESWIG-HOLSTEIN.transformed.osm.pbf -o=Freizeitkarte_SCHLESWIG-HOLSTEIN.transformed.osm
// grep --after-context=15 --before-context=10 "<tag k=\"place\" v=\"island\"/>" Freizeitkarte_SCHLESWIG-HOLSTEIN.transformed.osm
// grep --after-context=15 --before-context=10 "<tag k=\"name\" v=\"Amrum\"/>" Freizeitkarte_SCHLESWIG-HOLSTEIN.transformed.osm
//
// cd  ~/freizeitkarte-android-1712/work/Freizeitkarte_MUENSTER
// ~/freizeitkarte-android-1712/tools/osmconvert/linux/osmconvert Freizeitkarte_MUENSTER.transformed.osm.pbf -o=Freizeitkarte_MUENSTER.transformed.osm
// grep --after-context=15 --before-context=10 "<tag k=\"leisure\" v=\"sports_centre_gebaeude\"/>" Freizeitkarte_MUENSTER.transformed.osm
//------------------------------------------------------------

//------------------------------------------------------------
// Sprachabhaengige Texte
//------------------------------------------------------------
#include "style-translations"

//------------------------------------------------------------
// Initialisierung der TAGs und Merker
//------------------------------------------------------------

// alle speziellen TAGs (key/value) initialisieren die nachfolgend verwendet werden
TAG_INIT(abandoned)
TAG_INIT(access)
TAG_INIT(addr_housenumber)
TAG_INIT(addr_street)
TAG_INIT(aerialway)
TAG_INIT(aeroway)
TAG_INIT(amenity)
TAG_INIT(atm)
TAG_INIT(bBoxWeight)
TAG_INIT(bicycle)
TAG_INIT(brand)
TAG_INIT(building)
TAG_INIT(bus)
TAG_INIT(capacity)
TAG_INIT(car)
TAG_INIT(cemetery)
TAG_INIT(contour)
TAG_INIT(cuisine)
TAG_INIT(denotation)
TAG_INIT(disused)
TAG_INIT(ele)
TAG_INIT(emergency)
TAG_INIT(generator_source)
TAG_INIT(geological)
TAG_INIT(highway)
TAG_INIT(historic)
TAG_INIT(icn_ref)
TAG_INIT(information)
TAG_INIT(iwn_ref)
TAG_INIT(lcn_ref)
TAG_INIT(leisure)
TAG_INIT(lwn_ref)
TAG_INIT(man_made)
TAG_INIT(memorial_type)
TAG_INIT(military)
TAG_INIT(motorcar)
TAG_INIT(mountain_pass)
TAG_INIT(name)
TAG_INIT(natural)
TAG_INIT(ncn_ref)
TAG_INIT(network)
TAG_INIT(nwn_ref)
TAG_INIT(office)
TAG_INIT(operator)
TAG_INIT(place)
TAG_INIT(population)
TAG_INIT(power)
TAG_INIT(power_source)
TAG_INIT(public_transport)
TAG_INIT(railway)
TAG_INIT(rcn_ref)
TAG_INIT(ref)
TAG_INIT(religion)
TAG_INIT(rhn_ref)
TAG_INIT(rin_ref)
TAG_INIT(rmn_ref)
TAG_INIT(rpn_ref)
TAG_INIT(ruins)
TAG_INIT(rwn_ref)
TAG_INIT(shelter_type)
TAG_INIT(shop)
TAG_INIT(sport)
TAG_INIT(stars)
TAG_INIT(station)
TAG_INIT(tourism)
TAG_INIT(tower_type)
TAG_INIT(train)
TAG_INIT(tram)
TAG_INIT(waterway)
TAG_INIT(website)
TAG_INIT(wikipedia)
TAG_INIT(wikipedia_de)
// hier weitere TAGs initialisieren

// Merker initalisieren
fzk_found = 0;

//------------------------------------------------------------
// key/value-Daten der speziellen TAGs merken
// Anmerkungen:
// - der Name des speziellen TAGs sollte aehnlich dem OSM-Objektes sein
// - Beispiel: TAG_STORE(natural, "natural")
// - Beispiel: TAG_STORE(natural_disused, "natural:disused")
//------------------------------------------------------------

FOR_ALL_TAGS
  if      TAG_STORE(abandoned, "abandoned")
  else if TAG_STORE(access, "access")
  else if TAG_STORE(addr_housenumber, "addr:housenumber")
  else if TAG_STORE(addr_street, "addr:street")
  else if TAG_STORE(aerialway, "aerialway")
  else if TAG_STORE(aeroway, "aeroway")
  else if TAG_STORE(amenity, "amenity")
  else if TAG_STORE(atm, "atm")
  else if TAG_STORE(bBoxWeight, "bBoxWeight")
  else if TAG_STORE(bicycle, "bicycle")
  else if TAG_STORE(brand, "brand")
  else if TAG_STORE(building, "building")
  else if TAG_STORE(bus, "bus")
  else if TAG_STORE(capacity, "capacity")
  else if TAG_STORE(car, "car")
  else if TAG_STORE(cemetery, "cemetery")
  else if TAG_STORE(contour, "contour")
  else if TAG_STORE(cuisine, "cuisine")
  else if TAG_STORE(denotation, "denotation")
  else if TAG_STORE(disused, "disused")
  else if TAG_STORE(ele, "ele")
  else if TAG_STORE(emergency, "emergency")
  else if TAG_STORE(generator_source, "generator:source")
  else if TAG_STORE(geological, "geological")
  else if TAG_STORE(highway, "highway")
  else if TAG_STORE(historic, "historic")
  else if TAG_STORE(icn_ref, "icn_ref")
  else if TAG_STORE(information, "information")
  else if TAG_STORE(iwn_ref, "iwn_ref")
  else if TAG_STORE(lcn_ref, "lcn_ref")
  else if TAG_STORE(leisure, "leisure")
  else if TAG_STORE(lwn_ref, "lwn_ref")
  else if TAG_STORE(man_made, "man_made")
  else if TAG_STORE(memorial_type, "memorial:type")
  else if TAG_STORE(military, "military")
  else if TAG_STORE(motorcar, "motorcar")
  else if TAG_STORE(mountain_pass, "mountain_pass")
  else if TAG_STORE(name, "name")
  else if TAG_STORE(natural, "natural")
  else if TAG_STORE(ncn_ref, "ncn_ref")
  else if TAG_STORE(network, "network")
  else if TAG_STORE(nwn_ref, "nwn_ref")
  else if TAG_STORE(office, "office")
  else if TAG_STORE(operator, "operator")
  else if TAG_STORE(place, "place")
  else if TAG_STORE(population, "population")
  else if TAG_STORE(power, "power")
  else if TAG_STORE(power_source, "power_source")
  else if TAG_STORE(public_transport, "public_transport")
  else if TAG_STORE(railway, "railway")
  else if TAG_STORE(rcn_ref, "rcn_ref")
  else if TAG_STORE(ref, "ref")
  else if TAG_STORE(religion, "religion")
  else if TAG_STORE(rhn_ref, "rhn_ref")
  else if TAG_STORE(rin_ref, "rin_ref")
  else if TAG_STORE(rmn_ref, "rmn_ref")
  else if TAG_STORE(rpn_ref, "rpn_ref")
  else if TAG_STORE(ruins, "ruins")
  else if TAG_STORE(rwn_ref, "rwn_ref")
  else if TAG_STORE(shelter_type, "shelter_type")
  else if TAG_STORE(shop, "shop")
  else if TAG_STORE(sport, "sport")
  else if TAG_STORE(stars, "stars")
  else if TAG_STORE(station, "station")
  else if TAG_STORE(tourism, "tourism")
  else if TAG_STORE(tower_type, "tower:type")
  else if TAG_STORE(train, "train")
  else if TAG_STORE(tram, "tram")
  else if TAG_STORE(waterway, "waterway")
  else if TAG_STORE(website, "website")
  else if TAG_STORE(wikipedia, "wikipedia")
  else if TAG_STORE(wikipedia_de, "wikipedia:de")
  // hier weitere TAGs hinzufuegen
END_FOR_ALL_TAGS

//------------------------------------------------------------
// TAGs (gegebenenfalls) modifizieren und schreiben
//------------------------------------------------------------

if (fzk_found == 0) {
  //
  // keine spezielle Bearbeitung erforderlich (da kein Spezial-TAG gefunden wurde)
  //
  FOR_ALL_TAGS
    TAG_WRITE(*keyp, *valp)
  END_FOR_ALL_TAGS
}
else {
  //
  // spezielle Bearbeitung pruefen und durchfuehren (da mindestens ein Spezial-TAG gefunden wurde)
  //

  // Sonderbehandlung fuer man_made=obelisk
  if ( TAG_EXISTS(man_made, "obelisk") && KEY_NOT_EXISTS(historic) ) {
    // man_made=obelisk -> historic=monument
    FOR_ALL_TAGS
      // printf ("%s = %s\n", *keyp, *valp);
      if (strcmp (*keyp, "man_made") == 0) {
        keyp_historic = "historic";
        *keyp = "historic";
        valp_historic = "monument";
        *valp = "monument";
        // printf ("man_made=obelisk -> historic=monument (nach tag transform)\n");
        break;
      }
    END_FOR_ALL_TAGS
  }

  // Sonderbehandlung fuer amenity=charging_station
  if ( TAG_EXISTS(amenity, "charging_station") && KEY_NOT_EXISTS(car) && KEY_NOT_EXISTS(motorcar) ) {
    // socket:type typical for cars -> car=yes
    strcpy (fzk_temp, "no");
    FOR_ALL_TAGS
      if (strcmp (*keyp, "socket:type2") == 0) {
        strcpy (fzk_temp, "car_yes");
        // printf ("tag %s found -> car=yes for charging_station\n", *keyp);
      }
      else if (strcmp (*keyp, "socket:type2_combo") == 0) {
        strcpy (fzk_temp, "car_yes");
        // printf ("tag %s found -> car=yes for charging_station\n", *keyp);
      }
      else if (strcmp (*keyp, "socket:chademo") == 0) {
        strcpy (fzk_temp, "car_yes");
        //printf ("tag %s found -> car=yes for charging_station\n", *keyp);
      }
      else if (strcmp (*keyp, "socket:cee_blue") == 0) {
        strcpy (fzk_temp, "car_yes");
        // printf ("tag %s found -> car=yes for charging_station\n", *keyp);
      }
      else if (strcmp (*keyp, "socket:cee_red_16a") == 0) {
        strcpy (fzk_temp, "car_yes");
        // printf ("tag %s found -> car=yes for charging_station\n", *keyp);
      }
      else if (strcmp (*keyp, "socket:cee_red_32a") == 0) {
        strcpy (fzk_temp, "car_yes");
        // printf ("tag %s found -> car=yes for charging_station\n", *keyp);
      }
      else if (strcmp (*keyp, "socket:cee_red_64a") == 0) {
        strcpy (fzk_temp, "car_yes");
        // printf ("tag %s found -> car=yes for charging_station\n", *keyp);
      }
    END_FOR_ALL_TAGS
    if (strcmp (fzk_temp, "car_yes") == 0) {
      // tag car ergaenzen
      TAG_WRITE("car", "car_yes");
    }
  }
  if ( TAG_EXISTS(amenity, "charging_station") && TAG_EXISTS(car, "yes") ) {
    // car=yes -> car=car_yes
    FOR_ALL_TAGS
      if (strcmp (*keyp, "car") == 0) {
        snprintf (fzk_car, sizeof(fzk_car), "car_yes" );
        valp_car = fzk_car;
        *valp = fzk_car;
        // printf ("car = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }
  if ( TAG_EXISTS(amenity, "charging_station") && KEY_NOT_EXISTS(car) && TAG_EXISTS(motorcar, "yes") ) {
    // motorcar=yes -> car=car_yes
    // tag car ergaenzen
    TAG_WRITE("car", "car_yes");
  }
  if ( TAG_EXISTS(amenity, "charging_station") && KEY_NOT_EXISTS(bicycle) ) {
    // socket:type typical for bicycle -> bicycle=yes
    strcpy (fzk_temp, "no");
    FOR_ALL_TAGS
      if (strcmp (*keyp, "socket:ropd") == 0) {
        strcpy (fzk_temp, "bicycle_yes");
        // printf ("tag %s found -> bicycle=yes for charging_station\n", *keyp);
      }
      else if (strcmp (*keyp, "socket:bikeenergy") == 0) { // bikeenergy = ropd
        strcpy (fzk_temp, "bicycle_yes");
        // printf ("tag %s found -> bicycle=yes for charging_station\n", *keyp);
      }
      else if (strcmp (*keyp, "socket:schuko") == 0) {
        strcpy (fzk_temp, "bicycle_yes");
        // printf ("tag %s found -> bicycle=yes for charging_station\n", *keyp);
      }
      else if (strcmp (*keyp, "socket:sev1011_t13") == 0) {
        strcpy (fzk_temp, "bicycle_yes");
        // printf ("tag %s found -> bicycle=yes for charging_station\n", *keyp);
      }
    END_FOR_ALL_TAGS
    if (strcmp (fzk_temp, "bicycle_yes") == 0) {
      // tag bicycle ergaenzen
      TAG_WRITE("bicycle", "bicycle_yes");
    }
  }
  if ( TAG_EXISTS(amenity, "charging_station") && TAG_EXISTS(bicycle, "yes") ) {
    // bicycle=yes -> bicycle=bicycle_yes
    FOR_ALL_TAGS
      if (strcmp (*keyp, "bicycle") == 0) {
        snprintf (fzk_bicycle, sizeof(fzk_bicycle), "bicycle_yes" );
        valp_bicycle = fzk_bicycle;
        *valp = fzk_bicycle;
        // printf ("bicycle = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }

  // Sonderbehandlung der sport-Tags
  if ( KEY_EXISTS(sport) ) {
     if ( TAG_EXISTS(sport, "10pin") ||
          TAG_EXISTS(sport, "9pin") ||
          TAG_EXISTS(sport, "archery") ||
          TAG_EXISTS(sport, "athletics") ||
          TAG_EXISTS(sport, "basketball") ||
          TAG_EXISTS(sport, "beachvolleyball") ||
          TAG_EXISTS(sport, "boules") ||
          TAG_EXISTS(sport, "canoe") ||
          TAG_EXISTS(sport, "chess") ||
          TAG_EXISTS(sport, "climbing") ||
          TAG_EXISTS(sport, "equestrian") ||
          TAG_EXISTS(sport, "fitness") ||
          TAG_EXISTS(sport, "golf") ||
          TAG_EXISTS(sport, "gymnastics") ||
          TAG_EXISTS(sport, "model_aerodrome") ||
          TAG_EXISTS(sport, "multi") ||
          TAG_EXISTS(sport, "rowing") ||
          TAG_EXISTS(sport, "running") ||
          TAG_EXISTS(sport, "scuba_diving") ||
          TAG_EXISTS(sport, "shooting") ||
          TAG_EXISTS(sport, "skateboard") ||
          TAG_EXISTS(sport, "soccer") ||
          TAG_EXISTS(sport, "swimming") ||
          TAG_EXISTS(sport, "table_tennis") ||
          TAG_EXISTS(sport, "tennis") ||
          TAG_EXISTS(sport, "volleyball")
        ) {
       // no operation
     }
     else {
       // alle anderen Sportarten in "fzk_sport_sonstige" ändern
       FOR_ALL_TAGS
         if (strcmp (*keyp, "sport") == 0) {
           snprintf (fzk_sport, sizeof(fzk_sport), "fzk_sport_sonstige" );
           valp_sport = fzk_sport;
           *valp = fzk_sport;
           // printf ("sport = %s (nach tag transform)\n", *valp);
           break;
         }
       END_FOR_ALL_TAGS
     }
  }

  // Megastadt (>1.000.000 Einwohner) oder Landeshauptstadt (abgeleitete POIs nicht beruecksichtigen)
  // ------------------------------------------------------
  if ( TAG_EXISTS(place, "city") && KEY_EXISTS(population) && (atol (valp_population) >= 1000000) && KEY_NOT_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "place") == 0) {
        TAG_WRITE(*keyp, "city_1_000_000")
        // printf ("node place=city >= 1.000.000\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
      if (strcmp (*keyp, "name") == 0) {
        printf ("node place=city >= 1.000.000 = %s\n", *valp);
      }
    END_FOR_ALL_TAGS
  }

  // Grossstadt (>500.000 Einwohner) (abgeleitete POIs nicht beruecksichtigen)
  // -------------------------------
  else if ( TAG_EXISTS(place, "city") && KEY_EXISTS(population) && (atol (valp_population) >= 500000) && KEY_NOT_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "place") == 0) {
        TAG_WRITE(*keyp, "city_500_000")
        // printf ("node place=city >= 500.000\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
      if (strcmp (*keyp, "name") == 0) {
        printf ("node place=city >= 500.000 = %s\n", *valp);
      }
    END_FOR_ALL_TAGS
  }

  // Grossstadt (>250.000 Einwohner) (abgeleitete POIs nicht beruecksichtigen)
  // -------------------------------
  else if ( TAG_EXISTS(place, "city") && KEY_EXISTS(population) && (atol (valp_population) >= 250000) && KEY_NOT_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "place") == 0) {
        TAG_WRITE(*keyp, "city_250_000")
        // printf ("node place=city >= 250.000\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
      if (strcmp (*keyp, "name") == 0) {
        printf ("node place=city >= 250.000 = %s\n", *valp);
      }
    END_FOR_ALL_TAGS
  }

  // Grossstadt (>100.000 Einwohner) (abgeleitete POIs nicht beruecksichtigen)
  // -------------------------------
  else if ( TAG_EXISTS(place, "city") && KEY_EXISTS(population) && (atol (valp_population) >= 100000) && KEY_NOT_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "place") == 0) {
        TAG_WRITE(*keyp, "city_100_000")
        // printf ("node place=city >= 100.000\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
      if (strcmp (*keyp, "name") == 0) {
        printf ("node place=city >= 100.000 = %s\n", *valp);
      }
    END_FOR_ALL_TAGS
  }

  // Ortsbezeichnung ohne Bezug auf Bevoelkerung (z.B. "Wiesengrund", "Deutsches Eck") (abgeleitete POIs nicht beruecksichtigen)
  // -------------------------------------------
  else if ( TAG_EXISTS(place, "locality") && KEY_EXISTS(ele) && KEY_EXISTS(name) && KEY_NOT_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // Beispiel: "Senneralm (2187)"
        // Nachkommastellen entfernen (Beispiel: "187.61" -> "187")
        snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_ele));
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node place=locality -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(place, "locality") && KEY_EXISTS(ele) && KEY_NOT_EXISTS(name) && KEY_NOT_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // Beispiel: "N.N. (1874)"
    // Nachkommastellen entfernen (Beispiel: "187.61" -> "187")
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "N.N. (%ld)", atol (valp_ele));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node place=locality -> %s\n", fzk_value);
  }
  // nur abgeleitete POIs beruecksichtigen
  else if ( TAG_EXISTS(place, "island") && KEY_EXISTS(name) && KEY_EXISTS(bBoxWeight) ) {
    // die Insel nach Größe bewerten und umtaggen
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "place") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "island_huge")
          // printf ("way/rel place=island >= 26 = island_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "island_large")
          // printf ("way/rel place=island >= 24 = island_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "island_medium")
          // printf ("way/rel place=island >= 22 = island_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "island_small")
          // printf ("way/rel place=island >= 20 = island_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "island_tiny")
          // printf ("way/rel place=island >= 18 = island_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "island_micro")
          // printf ("way/rel place=island >= 16 = island_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "island_nano")
          // printf ("way/rel place=island >= 14 = island_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel place=island < 14 = island\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( KEY_EXISTS(place) && KEY_EXISTS(name) ) {
    // alle benannten place-Objekte (erfaßt oder abgeleitet) übernehmen
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Gebaeude im Zusammenhang mit sportlichen Aktivitaeten:
  // - als Turnhalle / Sportzentrum / Sportanlage
  // - als Schwimmbad / -halle
  // - mit spezifischer Sportnutzung
  else if ( KEY_EXISTS(building) && TAG_EXISTS(leisure, "sports_centre") && KEY_EXISTS(sport) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "leisure") == 0) {
        TAG_WRITE(*keyp, "sports_centre_gebaeude")
        // printf ("node building=* && leisure=sports_centre = leisure=sports_centre_gebaeude\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( KEY_EXISTS(building) && TAG_EXISTS(leisure, "sports_centre") && KEY_NOT_EXISTS(sport) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "leisure") == 0) {
        TAG_WRITE(*keyp, "sports_centre_gebaeude")
        // printf ("node building=* && leisure=sports_centre = leisure=sports_centre_gebaeude\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    // Pseudo-Sport-Tag erzeugen
    // strcpy (fzk_key, "sport");
    // snprintf (fzk_value, sizeof(fzk_value), "fzk_sport_sonstige");
    // TAG_WRITE(fzk_key, fzk_value)
  }
  else if ( KEY_EXISTS(building) && TAG_EXISTS(leisure, "water_park") && KEY_EXISTS(sport) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "leisure") == 0) {
        TAG_WRITE(*keyp, "water_park_gebaeude")
        // printf ("node building=* && leisure=water_park = leisure=water_park_gebaeude\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( KEY_EXISTS(building) && TAG_EXISTS(leisure, "water_park") && KEY_NOT_EXISTS(sport) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "leisure") == 0) {
        TAG_WRITE(*keyp, "water_park_gebaeude")
        // printf ("node building=* && leisure=water_park = leisure=water_park_gebaeude\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    // Pseudo-Sport-Tag erzeugen
    // strcpy (fzk_key, "sport");
    // snprintf (fzk_value, sizeof(fzk_value), "fzk_sport_sonstige");
    // TAG_WRITE(fzk_key, fzk_value)
  }

  // Windenergieanlage
  // -----------------
  else if ( TAG_EXISTS(power, "generator") && ( TAG_EXISTS(power_source, "wind") || TAG_EXISTS(generator_source, "wind") ) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "power") == 0) {
        TAG_WRITE(*keyp, "windenergieanlage")
        // printf ("node power=generator -> power=windenergieanlage\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Flughafen
  // ---------
  else if ( ( TAG_EXISTS(aeroway, "aerodrome") || TAG_EXISTS(aeroway, "airport") ) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__AIRPORT__);
      //   // TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node aeroway=aerodrome || aeroway=airport -> %s\n", fzk_value);
      //   TAG_WRITE(*keyp, *valp)
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( ( TAG_EXISTS(aeroway, "aerodrome") || TAG_EXISTS(aeroway, "airport") ) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__AIRPORT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node aeroway=aerodrome || aeroway=airport -> %s\n", fzk_value);
  }

  // Hubschrauberlandeplatz
  // ----------------------
  else if ( TAG_EXISTS(aeroway, "helipad") && KEY_EXISTS(name)) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__HELIPAD__);
      //   TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node aeroway=helipad -> %s\n", fzk_value);
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(aeroway, "helipad") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__HELIPAD__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node aeroway=helipad -> %s\n", fzk_value);
  }

  // Flughafengebaeude
  // -----------------
  else if ( TAG_EXISTS(aeroway, "terminal") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__TERMINAL__);
      //   // TAG_WRITE(*keyp, fzk_value)
      //   TAG_WRITE(*keyp, *valp)
      //   // printf ("node aeroway=terminal -> %s\n", fzk_value);
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(aeroway, "terminal") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__TERMINAL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node aeroway=terminal -> %s\n", fzk_value);
  }

  // Bahnhof (mit mindestens einer Weiche)
  // -------
  else if ( TAG_EXISTS(railway, "station") && TAG_EXISTS(station, "light_rail") && KEY_EXISTS(name) ) {
    int layer = 0;
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        TAG_WRITE(*keyp, "station_light_rail")
        // printf ("node railway=station && station=light_rail -> railway=station_light_rail\n");
      }
      else if (strcmp (*keyp, "layer") == 0) {
        // layer-value auf Maximalwert setzen
        TAG_WRITE(*keyp, "5")
        layer = 1;
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    if (layer == 0)  {
      // layer-Tag ergaenzen
      TAG_WRITE("layer", "5")
    }
  }
  else if ( TAG_EXISTS(railway, "station") && TAG_EXISTS(station, "light_rail") && KEY_NOT_EXISTS(name) ) {
    int layer = 0;
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        TAG_WRITE(*keyp, "station_light_rail")
        // printf ("node railway=station && station=light_rail -> railway=station_light_rail\n");
      }
      else if (strcmp (*keyp, "layer") == 0) {
        // layer-value auf Maximalwert setzen
        TAG_WRITE(*keyp, "5")
        layer = 1;
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    if (layer == 0)  {
      // layer-Tag ergaenzen
      TAG_WRITE("layer", "5")
    }
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__RAILWAYSTATION__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node railway=station && station=light_rail -> name=%s\n", fzk_value);
  }
  else if ( TAG_EXISTS(railway, "station") && TAG_EXISTS(station, "subway") && KEY_EXISTS(name) ) {
    int layer = 0;
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        TAG_WRITE(*keyp, "station_subway")
        // printf ("node railway=station && station=subway -> railway=station_subway\n");
      }
      else if (strcmp (*keyp, "layer") == 0) {
        // layer-value auf Maximalwert setzen
        TAG_WRITE(*keyp, "5")
        layer = 1;
        // printf ("node railway=station && station=subway -> layer=5 (modified)\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    if (layer == 0)  {
      // layer-Tag ergaenzen
      TAG_WRITE("layer", "5")
      // printf ("node railway=station && station=subway -> layer=5 (added)\n");
    }
  }
  else if ( TAG_EXISTS(railway, "station") && TAG_EXISTS(station, "subway") && KEY_NOT_EXISTS(name) ) {
    int layer = 0;
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        TAG_WRITE(*keyp, "station_subway")
        // printf ("node railway=station && station=subway -> railway=station_subway\n");
      }
      else if (strcmp (*keyp, "layer") == 0) {
        // layer-value auf Maximalwert setzen
        TAG_WRITE(*keyp, "5")
        layer = 1;
        // printf ("node railway=station && station=subway -> layer=5 (modified)\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    if (layer == 0)  {
      // layer-Tag ergaenzen
      TAG_WRITE("layer", "5")
      // printf ("node railway=station && station=subway -> layer=5 (added)\n");
    }
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__RAILWAYSTATION__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node railway=station && station=subway -> name=%s\n", fzk_value);
  }
  else if ( TAG_EXISTS(railway, "station") && KEY_EXISTS(name) ) {
    int layer = 0;
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__RAILWAYSTATION__);
        // TAG_WRITE(*keyp, fzk_value)
        // printf ("node railway=station -> %s\n", fzk_value);
        TAG_WRITE(*keyp, *valp)
      }
      else if (strcmp (*keyp, "layer") == 0) {
        // layer-value auf Maximalwert setzen
        TAG_WRITE(*keyp, "5")
        layer = 1;
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    if (layer == 0)  {
      // layer-Tag ergaenzen
      TAG_WRITE("layer", "5")
    }
  }
  else if ( TAG_EXISTS(railway, "station") && KEY_NOT_EXISTS(name) ) {
    int layer = 0;
    FOR_ALL_TAGS
      if (strcmp (*keyp, "layer") == 0) {
        // layer-value auf Maximalwert setzen
        TAG_WRITE(*keyp, "5")
        layer = 1;
      }
      else  {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    if (layer == 0)  {
      // layer-Tag ergaenzen
      TAG_WRITE("layer", "5")
    }
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__RAILWAYSTATION__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node railway=station -> %s\n", fzk_value);
  }

  // U-Bahneingang
  // -------------
  else if ( TAG_EXISTS(railway, "subway_entrance") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__SUBWAYENTRANCE__);
      //   // TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node railway=subway_entrance -> %s\n", fzk_value);
      //   TAG_WRITE(*keyp, *valp)
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "subway_entrance") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__SUBWAYENTRANCE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node railway=subway_entrance -> %s\n", fzk_value);
  }

  // Haltepunkt (kein Bahnhof, z.T. ohne Bahnsteig, nur bei Bedarf)
  // ----------
  else if ( TAG_EXISTS(railway, "halt") && TAG_EXISTS(station, "light_rail") && KEY_EXISTS(name) ) {
    int layer = 0;
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        TAG_WRITE(*keyp, "halt_light_rail")
        // printf ("node railway=halt && station=light_rail -> railway=halt_light_rail\n");
      }
      else if (strcmp (*keyp, "layer") == 0) {
        // layer-value auf Maximalwert setzen
        TAG_WRITE(*keyp, "5")
        layer = 1;
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    if (layer == 0)  {
      // layer-Tag ergaenzen
      TAG_WRITE("layer", "5")
    }
  }
  else if ( TAG_EXISTS(railway, "halt") && TAG_EXISTS(station, "light_rail") && KEY_NOT_EXISTS(name) ) {
    int layer = 0;
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        TAG_WRITE(*keyp, "halt_light_rail")
        // printf ("node railway=halt && station=light_rail -> railway=halt_light_rail\n");
      }
      else if (strcmp (*keyp, "layer") == 0) {
        // layer-value auf Maximalwert setzen
        TAG_WRITE(*keyp, "5")
        layer = 1;
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    if (layer == 0)  {
      // layer-Tag ergaenzen
      TAG_WRITE("layer", "5")
    }
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__RAILWAYSTATION__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node railway=halt && station=light_rail -> name=%s\n", fzk_value);
  }
  else if ( TAG_EXISTS(railway, "halt") && KEY_EXISTS(name) ) {
    int layer = 0;
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__RAILWAYHALT__);
        // TAG_WRITE(*keyp, fzk_value)
        // printf ("node railway=halt -> %s\n", fzk_value);
        TAG_WRITE(*keyp, *valp)
      }
      else if (strcmp (*keyp, "layer") == 0) {
        // layer-value auf Maximalwert setzen
        TAG_WRITE(*keyp, "5")
        layer = 1;
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    if (layer == 0)  {
      // layer-Tag ergaenzen
      TAG_WRITE("layer", "5")
    }
  }
  else if ( TAG_EXISTS(railway, "halt") && KEY_NOT_EXISTS(name) ) {
    int layer = 0;
    FOR_ALL_TAGS
      if (strcmp (*keyp, "layer") == 0) {
        // layer-value auf Maximalwert setzen
        TAG_WRITE(*keyp, "5")
        layer = 1;
      }
      else  {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    if (layer == 0)  {
      // layer-Tag ergaenzen
      TAG_WRITE("layer", "5")
    }
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__RAILWAYHALT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node railway=halt -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(public_transport, "stop_position") && TAG_EXISTS(train, "yes") && KEY_EXISTS(name) ) {
    // Tags schreiben; public_transport-Tag modifizieren
    FOR_ALL_TAGS
      if (strcmp (*keyp, "public_transport") == 0) {
        TAG_WRITE(*keyp, "bahnhaltepunkt")
        // printf ("node public_transport=stop_position && train=yes -> public_transport=bahnhaltepunkt\n");
      }
      else if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__RAILWAYHALT__);
        // TAG_WRITE(*keyp, fzk_value)
        // printf ("node public_transport=stop_position && train=yes -> %s\n", fzk_value);
        TAG_WRITE(*keyp, *valp)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(public_transport, "stop_position") && TAG_EXISTS(train, "yes") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "public_transport") == 0) {
        TAG_WRITE(*keyp, "bahnhaltepunkt")
        // printf ("node public_transport=stop_position && train=yes -> public_transport=bahnhaltepunkt\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__RAILWAYHALT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node public_transport=stop_position && train=yes -> %s\n", fzk_value);
  }

  // Strassenbahnhaltestelle
  // -----------------------
  else if ( TAG_EXISTS(railway, "tram_stop") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__TRAMSTOP__);
        // TAG_WRITE(*keyp, fzk_value)
        // printf ("node railway=tram_stop -> %s\n", fzk_value);
        TAG_WRITE(*keyp, *valp)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "tram_stop") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__TRAMSTOP__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node railway=tram_stop -> %s\n", fzk_value);
  }
  // else if ( TAG_EXISTS(public_transport, "stop_position") && TAG_EXISTS(tram, "yes") && KEY_EXISTS(name) ) {
  //   FOR_ALL_TAGS
  //     if (strcmp (*keyp, "public_transport") == 0) {
  //       TAG_WRITE(*keyp, "tramhaltestelle")
  //       // printf ("node public_transport=stop_position && tram=yes -> public_transport=tramhaltestelle\n");
  //     }
  //     else if (strcmp (*keyp, "name") == 0) {
  //       // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__TRAMSTOP__);
  //       // TAG_WRITE(*keyp, fzk_value)
  //       // printf ("node public_transport=stop_position && tram=yes -> %s\n", fzk_value);
  //       TAG_WRITE(*keyp, *valp)
  //     }
  //     else {
  //       TAG_WRITE(*keyp, *valp)
  //     }
  //   END_FOR_ALL_TAGS
  // }
  // else if ( TAG_EXISTS(public_transport, "stop_position") && TAG_EXISTS(tram, "yes") && KEY_NOT_EXISTS(name) ) {
  //   FOR_ALL_TAGS
  //     if (strcmp (*keyp, "public_transport") == 0) {
  //       TAG_WRITE(*keyp, "tramhaltestelle")
  //       // printf ("node public_transport=stop_position && tram=yes -> public_transport=tramhaltestelle\n");
  //     }
  //     else {
  //       TAG_WRITE(*keyp, *valp)
  //     }
  //   END_FOR_ALL_TAGS
  //   strcpy (fzk_key, "name");
  //   snprintf (fzk_value, sizeof(fzk_value), "%s", $__TRAMSTOP__);
  //   TAG_WRITE(fzk_key, fzk_value)
  //   // printf ("node public_transport=stop_position && tram=yes -> %s\n", fzk_value);
  // }

  // Seilbahnstation
  // ---------------
  else if ( TAG_EXISTS(aerialway, "station") && KEY_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_ele));
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node aerialway=station && ele=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(aerialway, "station") && KEY_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", $__AERIALWAYSTATION__, atol (valp_ele));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node aerialway=station && ele=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(aerialway, "station") && KEY_NOT_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__AERIALWAYSTATION__);
    TAG_WRITE("name", fzk_value)
    // printf ("node aerialway=station -> name=Station\n");
  }

  // Bewirtschaftete Berghuette
  // --------------------------
  else if ( TAG_EXISTS(tourism, "alpine_hut") && TAG_EXISTS(amenity, "restaurant") && KEY_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "tourism") == 0) {
        TAG_WRITE(*keyp, "berghuette_bewirtschaftet")
      }
      else if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_ele));
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=alpine_hut && amenity=restaurant && ele=* && name=* -> %s\n", fzk_value);
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        // Tag entfernen
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "alpine_hut") && TAG_EXISTS(amenity, "restaurant") && KEY_NOT_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "tourism") == 0) {
        TAG_WRITE(*keyp, "berghuette_bewirtschaftet")
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        // Tag entfernen
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    // printf ("node tourism=alpine_hut && amenity=restaurant && name=*\n");
  }
  else if ( TAG_EXISTS(tourism, "alpine_hut") && TAG_EXISTS(amenity, "restaurant") && KEY_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "tourism") == 0) {
        TAG_WRITE(*keyp, "berghuette_bewirtschaftet")
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        // Tag entfernen
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", $__ALPINEHUT__, atol (valp_ele));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=alpine_hut && amenity=restaurant && ele=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(tourism, "alpine_hut") && TAG_EXISTS(amenity, "restaurant") && KEY_NOT_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "tourism") == 0) {
        TAG_WRITE(*keyp, "berghuette_bewirtschaftet")
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        // Tag entfernen
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__ALPINEHUT__);
    TAG_WRITE("name", fzk_value)
    // printf ("node tourism=alpine_hut && amenity=restaurant -> %s\n", fzk_value);
  }

  // Selbstversorger Berghuette
  // --------------------------
  else if ( TAG_EXISTS(tourism, "alpine_hut") && KEY_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
       // name-Tag modifizieren
       snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_ele));
       TAG_WRITE(*keyp, fzk_value)
       // printf ("node tourism=alpine_hut && ele=* && name=* -> %s\n", fzk_value);
     }
     else {
       TAG_WRITE(*keyp, *valp)
     }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "alpine_hut") && KEY_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", $__ALPINEHUT__, atol (valp_ele));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=alpine_hut && ele=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(tourism, "alpine_hut") && KEY_NOT_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__ALPINEHUT__);
    TAG_WRITE("name", fzk_value)
    // printf ("node tourism=alpine_hut -> %s\n", fzk_value);
  }

  // Basic hut (Schutzhuette, Berghuette)
  // ---------
  // Anmerkung: Das Tagging wurde inzwischen durch (amenity=shelter + shelter_type=basic_hut) ersetzt.
  else if ( TAG_EXISTS(tourism, "basic_hut") && KEY_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
       // name-Tag modifizieren
       snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_ele));
       TAG_WRITE(*keyp, fzk_value)
       // printf ("node tourism=basic_hut && ele=* && name=* -> %s\n", fzk_value);
     }
     else {
       TAG_WRITE(*keyp, *valp)
     }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "basic_hut") && KEY_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", $__BASICHUT__, atol (valp_ele));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=basic_hut && ele=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(tourism, "basic_hut") && KEY_NOT_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__BASICHUT__);
    TAG_WRITE("name", fzk_value)
    // printf ("node tourism=basic_hut -> %s\n", fzk_value);
  }

  // Lean-to (Schutzhuette, Berghuette)
  // -------
  // Anmerkung: Das Tagging wurde inzwischen durch (amenity=shelter + shelter_type=lean_to) ersetzt.
  else if ( TAG_EXISTS(tourism, "lean_to") && KEY_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
       // name-Tag modifizieren
       snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_ele));
       TAG_WRITE(*keyp, fzk_value)
       // printf ("node tourism=lean_to && ele=* && name=* -> %s\n", fzk_value);
     }
     else {
       TAG_WRITE(*keyp, *valp)
     }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "lean_to") && KEY_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", $__LEANTO__, atol (valp_ele));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=lean_to && ele=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(tourism, "lean_to") && KEY_NOT_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // snprintf (fzk_value, sizeof(fzk_value), "%s", $__LEANTO__);
    // TAG_WRITE("name", fzk_value)
    // printf ("node tourism=lean_to -> %s\n", fzk_value);
  }

  // Wilderness hut (Schutzhuette, Berghuette)
  // --------------
  else if ( TAG_EXISTS(tourism, "wilderness_hut") && KEY_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
       // name-Tag modifizieren
       snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_ele));
       TAG_WRITE(*keyp, fzk_value)
       // printf ("node tourism=wilderness_hut && ele=* && name=* -> %s\n", fzk_value);
     }
     else {
       TAG_WRITE(*keyp, *valp)
     }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "wilderness_hut") && KEY_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", $__WILDERNESSHUT__, atol (valp_ele));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=wilderness_hut && ele=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(tourism, "wilderness_hut") && KEY_NOT_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__WILDERNESSHUT__);
    TAG_WRITE("name", fzk_value)
    // printf ("node tourism=wilderness_hut -> %s\n", fzk_value);
  }

  // Basic Hut (Schutzhuette, Berghuette)
  // ---------
  else if ( TAG_EXISTS(amenity, "shelter") && TAG_EXISTS(shelter_type, "basic_hut") && KEY_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "shelter_basic_hut")
      }
      else if (strcmp (*keyp, "name") == 0) {
       snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_ele));
       TAG_WRITE(*keyp, fzk_value)
       // printf ("node amenity=shelter && shelter_type=basic_hut && ele=* && name=* -> %s\n", fzk_value);
     }
     else {
       TAG_WRITE(*keyp, *valp)
     }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "shelter") && TAG_EXISTS(shelter_type, "basic_hut") && KEY_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "shelter_basic_hut")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", $__BASICHUT__, atol (valp_ele));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=shelter && shelter_type=basic_hut && ele=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(amenity, "shelter") && TAG_EXISTS(shelter_type, "basic_hut") && KEY_NOT_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "shelter_basic_hut")
      }
      else if (strcmp (*keyp, "name") == 0) {
       snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
       TAG_WRITE(*keyp, fzk_value)
       // printf ("node amenity=shelter && shelter_type=basic_hut && ele=* && name=* -> %s\n", fzk_value);
     }
     else {
       TAG_WRITE(*keyp, *valp)
     }
    END_FOR_ALL_TAGS
    // printf ("node amenity=shelter && shelter_type=basic_hut -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(amenity, "shelter") && TAG_EXISTS(shelter_type, "basic_hut") && KEY_NOT_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "shelter_basic_hut")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__BASICHUT__);
    TAG_WRITE("name", fzk_value)
    // printf ("node amenity=shelter && shelter_type=basic_hut -> %s\n", fzk_value);
  }

  // Lean-to (Schutzhuette, Berghuette)
  // -------
  else if ( TAG_EXISTS(amenity, "shelter") && TAG_EXISTS(shelter_type, "lean_to") && KEY_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "shelter_lean_to")
      }
      else if (strcmp (*keyp, "name") == 0) {
       snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_ele));
       TAG_WRITE(*keyp, fzk_value)
       // printf ("node amenity=shelter && shelter_type=lean_to && ele=* && name=* -> %s\n", fzk_value);
     }
     else {
       TAG_WRITE(*keyp, *valp)
     }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "shelter") && TAG_EXISTS(shelter_type, "lean_to") && KEY_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "shelter_lean_to")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", $__LEANTO__, atol (valp_ele));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=shelter && shelter_type=lean_to && ele=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(amenity, "shelter") && TAG_EXISTS(shelter_type, "lean_to") && KEY_NOT_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "shelter_lean_to")
      }
      else if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=shelter && shelter_type=lean_to && ele=* && name=* -> %s\n", fzk_value);
     }
     else {
       TAG_WRITE(*keyp, *valp)
     }
    END_FOR_ALL_TAGS
    // printf ("node amenity=shelter && shelter_type=lean_to -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(amenity, "shelter") && TAG_EXISTS(shelter_type, "lean_to") && KEY_NOT_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "shelter_lean_to")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    // snprintf (fzk_value, sizeof(fzk_value), "%s", $__LEANTO__);
    // TAG_WRITE("name", fzk_value)
    // printf ("node amenity=shelter && shelter_type=lean_to -> %s\n", fzk_value);
  }

  // (Bus-)Wartehaeuschen: Kleine Unterstaende, die bei Haltestellen des oeffentlichen Verkehrs zu finden sind (haeufig in Staedten).
  //       --------------
  else if ( TAG_EXISTS(amenity, "shelter") && TAG_EXISTS(shelter_type, "public_transport") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "shelter_public_transport")
      }
      else {
       TAG_WRITE(*keyp, *valp)
     }
    END_FOR_ALL_TAGS
  }

  // Aquarium
  // --------
  else if ( TAG_EXISTS(tourism, "aquarium") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__AQUARIUM__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=aquarium -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "aquarium") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__AQUARIUM__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=aquarium -> %s\n", fzk_value);
  }

  // Sehenswuerdigkeit (abgeleitete POIs nicht beruecksichtigen; wird häufig zusätzlich getaggt)
  // -----------------
  else if ( TAG_EXISTS(tourism, "attraction") && KEY_EXISTS(name) && KEY_NOT_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__ATTRACTION__);
      //   TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node tourism=attraction -> %s\n", fzk_value);
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "attraction") && KEY_NOT_EXISTS(name) && KEY_NOT_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__ATTRACTION__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=attraction -> %s\n", fzk_value);
  }

  // Kunstwerk, Kunstobjekt
  // ----------------------
  else if ( TAG_EXISTS(tourism, "artwork") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__ARTWORK__);
      //   TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node tourism=artwork -> %s\n", fzk_value);
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "artwork") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__ARTWORK__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=artwork -> %s\n", fzk_value);
  }

  // Bed & Breakfast
  // ---------------
  else if ( TAG_EXISTS(tourism, "bed_and_breakfast") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__BEDANDBREAKFAST__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=bed_and_breakfast -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "bed_and_breakfast") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__BEDANDBREAKFAST__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=bed_and_breakfast -> %s\n", fzk_value);
  }

  // Campingplatz
  // ------------
  else if ( TAG_EXISTS(tourism, "camp_site") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__CAMPSITE__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=camp_site -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "camp_site") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CAMPSITE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=camp_site -> %s\n", fzk_value);
  }

  // Stellplatz fuer Wohnmobile (Reisemobile)
  // ----------------------------------------
  else if ( TAG_EXISTS(tourism, "caravan_site") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__CARAVANSITE__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=caravan_site -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "caravan_site") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CARAVANSITE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=caravan_site -> %s\n", fzk_value);
  }

  // Ferienwohnung, Ferienhaus, Sennhuette
  // -------------------------------------
  else if ( TAG_EXISTS(tourism, "chalet") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__CHALET__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=chalet -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "chalet") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CHALET__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=chalet -> %s\n", fzk_value);
  }

  // Pension, Gasthaus
  // -----------------
  else if ( TAG_EXISTS(tourism, "guest_house") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__GUESTHOUSE__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=guest_house -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "guest_house") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__GUESTHOUSE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=guest_house -> %s\n", fzk_value);
  }

  // Jugendherberge, Herberge, Naturfreundehaus usw.
  // -----------------------------------------------
  else if ( TAG_EXISTS(tourism, "hostel") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__HOSTEL__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=hostel -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "hostel") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__HOSTEL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=hostel -> %s\n", fzk_value);
  }

  // Hotel mit Restaurant (mit Beruecksichtigung der Hotel-Kategorie (Beispiel: 3* Gasthof Stern (Hotel, Restaurant)).
  // --------------------
  else if ( TAG_EXISTS(tourism, "hotel") && TAG_EXISTS(amenity, "restaurant") && KEY_EXISTS(stars) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "tourism") == 0) {
        TAG_WRITE(*keyp, "hotel_restaurant")
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        // amenity=restaurant entfernen
      }
      else if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s* %s (%s, %s)", valp_stars, *valp, $__HOTEL__, $__RESTAURANT__);
        snprintf (fzk_value, sizeof(fzk_value), "%s* %s", valp_stars, *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=hotel && restaurant=* && stars=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "hotel") && TAG_EXISTS(amenity, "restaurant") && KEY_EXISTS(stars) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "tourism") == 0) {
        TAG_WRITE(*keyp, "hotel_restaurant")
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        // amenity=restaurant entfernen
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s* %s, %s", valp_stars, $__HOTEL__, $__RESTAURANT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=hotel && restaurant=* && stars=* -> %s\n", fzk_value);
  }

  // Hotel mit Restaurant (ohne Beruecksichtigung der Hotel-Kategorie (Beispiel: Nordstern (Hotel, Restaurant)).
  // --------------------
  else if ( TAG_EXISTS(tourism, "hotel") && TAG_EXISTS(amenity, "restaurant") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "tourism") == 0) {
        TAG_WRITE(*keyp, "hotel_restaurant")
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        // amenity=restaurant entfernen
      }
      else if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s, %s)", *valp, $__HOTEL__, $__RESTAURANT__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=hotel && restaurant=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "hotel") && TAG_EXISTS(amenity, "restaurant") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "tourism") == 0) {
        TAG_WRITE(*keyp, "hotel_restaurant")
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        // amenity=restaurant entfernen
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s, %s", $__HOTEL__, $__RESTAURANT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=hotel && restaurant=* -> %s\n", fzk_value);
  }

  // Hotel ohne Restaurant (mit Beruecksichtigung der Hotel-Kategorie (Beispiel: 3* Gasthof Stern (Hotel)).
  // ---------------------
  else if ( TAG_EXISTS(tourism, "hotel") && KEY_EXISTS(stars) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s* %s (%s)", valp_stars, *valp, $__HOTEL__);
        snprintf (fzk_value, sizeof(fzk_value), "%s* %s", valp_stars, *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=hotel && stars=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "hotel") && KEY_EXISTS(stars) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s* %s", valp_stars, $__HOTEL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=hotel && stars=* -> %s\n", fzk_value);
  }

  // Hotel ohne Restaurant (ohne Beruecksichtigung der Hotel-Kategorie (Beispiel: Kemper (Hotel)).
  // ---------------------
  else if ( TAG_EXISTS(tourism, "hotel") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__HOTEL__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=hotel -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "hotel") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__HOTEL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=hotel -> %s\n", fzk_value);
  }

  // Motel (mit Beruecksichtigung der Motel-Kategorie (Beispiel: 3* Drive-In Montana (Motel)).
  // -----
  else if ( TAG_EXISTS(tourism, "motel") && KEY_EXISTS(stars) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__MOTEL__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=motel && stars=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "motel") && KEY_EXISTS(stars) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__MOTEL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=motel && stars=* -> %s\n", fzk_value);
  }

  // Motel (ohne Beruecksichtigung der Motel-Kategorie (Beispiel: Tecklenburger Land (Motel)).
  // -----
  else if ( TAG_EXISTS(tourism, "motel") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__MOTEL__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=motel && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "motel") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__MOTEL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=motel -> %s\n", fzk_value);
  }

  // Rastplatz (Picknick)
  // --------------------
  else if ( TAG_EXISTS(tourism, "picnic_site") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__PICNICSITE__);
      //   TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node tourism=picnic_site -> %s\n", fzk_value);
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "picnic_site") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__PICNICSITE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=picnic_site -> %s\n", fzk_value);
  }

  // Aussichtspunkt
  // --------------
  else if ( TAG_EXISTS(tourism, "viewpoint") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__VIEWPOINT__);
        // TAG_WRITE(*keyp, fzk_value)
        // printf ("node tourism=viewpoint -> %s\n", fzk_value);
        TAG_WRITE(*keyp, *valp)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "viewpoint") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__VIEWPOINT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=viewpoint -> %s\n", fzk_value);
  }

  // Zoologischer Garten (Zoo), Tierpark
  // -----------------------------------
  else if ( TAG_EXISTS(tourism, "zoo") && KEY_EXISTS(name) && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // Flächengröße hinzufügen
    if (my_bBoxWeight <= 15) {
      TAG_WRITE("area_size", "small")
    }
    else {
      TAG_WRITE("area_size", "large")
    }
  }
  else if ( TAG_EXISTS(tourism, "zoo") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // Flächengröße hinzufügen
    TAG_WRITE("area_size", "small")
  }
  else if ( TAG_EXISTS(tourism, "zoo") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__ZOO__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=zoo -> %s\n", fzk_value);
    // Flächengröße hinzufügen
    TAG_WRITE("area_size", "small")
  }

  // Freizeitpark, Themenpark
  // ------------------------
  else if ( TAG_EXISTS(tourism, "theme_park") && KEY_EXISTS(name) && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // Flächengröße hinzufügen
    if (my_bBoxWeight <= 15) {
      TAG_WRITE("area_size", "small")
    }
    else {
      TAG_WRITE("area_size", "large")
    }
  }
  else if ( TAG_EXISTS(tourism, "theme_park") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // Flächengröße hinzufügen
    TAG_WRITE("area_size", "small")
  }
  else if ( TAG_EXISTS(tourism, "theme_park") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__THEMEPARK__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=theme_park -> %s\n", fzk_value);
    // Flächengröße hinzufügen
    TAG_WRITE("area_size", "small")
  }

  // Museum
  // ------
  else if ( TAG_EXISTS(tourism, "museum") && KEY_EXISTS(name) && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // Flächengröße hinzufügen
    if (my_bBoxWeight <= 15) {
      TAG_WRITE("area_size", "small")
    }
    else {
      TAG_WRITE("area_size", "large")
    }
  }
  else if ( TAG_EXISTS(tourism, "museum") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // Flächengröße hinzufügen
    TAG_WRITE("area_size", "small")
  }
  else if ( TAG_EXISTS(tourism, "museum") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__MUSEUM__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node tourism=museum -> %s\n", fzk_value);
    // Flächengröße hinzufügen
    TAG_WRITE("area_size", "small")
  }

  // (Touristen-)Information
  // -----------------------
  else if ( TAG_EXISTS(tourism, "information") && TAG_EXISTS(information, "guidepost") ) {
    // keine besondere Verarbeitung
    FOR_ALL_TAGS
      if (strcmp (*keyp, "tourism") == 0) {
        TAG_WRITE(*keyp, "information_guidepost")
        // printf ("node tourism=information && information=guidepost -> information_guidepost\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(tourism, "information") && TAG_EXISTS(information, "route_marker") ) {
    // entfernen
    // printf ("node tourism=information && information=route_marker -> entfernt\n");
  }
  else if ( TAG_EXISTS(tourism, "information") && TAG_EXISTS(information, "trail_blaze") ) {
    // entfernen
    // printf ("node tourism=information && information=trail_blaze -> entfernt\n");
  }
  else if ( TAG_EXISTS(tourism, "information") ) {
    // keine besondere Verarbeitung
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Stollen, Zugang zu einer Mine (horizontal oder fast horizontal)
  // -----------------------------
  else if ( TAG_EXISTS(man_made, "adit") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__ADIT__);
        // TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=adit -> %s\n", fzk_value);
        TAG_WRITE(*keyp, *valp)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "adit") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__ADIT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=adit -> %s\n", fzk_value);
  }

  // Blinklicht / Bake / Leuchtfeuer
  // -------------------------------
  else if ( TAG_EXISTS(man_made, "beacon") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__BEACON__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=beacon -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "beacon") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__BEACON__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=beacon -> %s\n", fzk_value);
  }

  // Leuchtturm
  // ----------
  else if ( TAG_EXISTS(man_made, "lighthouse") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__LIGHTHOUSE__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=lighthouse -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "lighthouse") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__LIGHTHOUSE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=lighthouse -> %s\n", fzk_value);
  }

  // Mast (z.B. Telekommunkations- / Funkmast)
  // ----
  else if ( TAG_EXISTS(man_made, "mast") && TAG_EXISTS(tower_type, "communication") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__MAST__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=mast -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "mast") && TAG_EXISTS(tower_type, "communication") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__MAST__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=mast -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(man_made, "mast") ) {
    // übrige entfernen
    // printf ("node man_made=mast -> entfernt\n");
  }
  // Schachtanlage / Bergwerk
  // ------------------------
  else if ( TAG_EXISTS(man_made, "mineshaft") && (KEY_EXISTS(disused) || KEY_EXISTS(abandoned)) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "man_made") == 0) {
        TAG_WRITE(*keyp, "mineshaft_disused")
      }
      else if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__MINESHAFT__);
        // TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=mineshaft_disused -> %s\n", fzk_value);
        TAG_WRITE(*keyp, *valp)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "mineshaft") && (KEY_EXISTS(disused) || KEY_EXISTS(abandoned)) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "man_made") == 0) {
        TAG_WRITE(*keyp, "mineshaft_disused")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__MINESHAFT__);
    TAG_WRITE(fzk_key, fzk_value)
  }
  else if ( TAG_EXISTS(man_made, "mineshaft") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__MINESHAFT__);
        // TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=mineshaft -> %s\n", fzk_value);
        TAG_WRITE(*keyp, *valp)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "mineshaft") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__MINESHAFT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=mineshaft -> %s\n", fzk_value);
  }

  // Oelfoerdereinrichtung
  // ---------------------
  else if ( TAG_EXISTS(man_made, "petroleum_well") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__PETROLEUMWELL__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=petroleum_well -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "petroleum_well") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__PETROLEUMWELL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=petroleum_well -> %s\n", fzk_value);
  }

  // Ueberdecktes Speicherbasin
  // --------------------------
  else if ( TAG_EXISTS(man_made, "reservoir_covered") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__RESERVOIRCOVERED__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=reservoir_covered -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "reservoir_covered") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__RESERVOIRCOVERED__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=reservoir_covered -> %s\n", fzk_value);
  }

  // Trigonometrischer Punkt mit Hoehenangabe
  // ----------------------------------------
  else if ( TAG_EXISTS(man_made, "survey_point") && KEY_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "man_made") == 0) {
        TAG_WRITE(*keyp, "trigonometrischer_punkt")
      }
      else if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_ele));
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=trigonometrischer_punkt -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "survey_point") && KEY_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "man_made") == 0) {
        TAG_WRITE(*keyp, "trigonometrischer_punkt")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", $__SURVEYPOINT__, atol (valp_ele));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=trigonometrischer_punkt -> %s\n", fzk_value);
  }

  // Beobachtungsturm
  // ----------------
  else if ( TAG_EXISTS(man_made, "tower") && TAG_EXISTS(tower_type, "observation") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "man_made") == 0) {
        TAG_WRITE(*keyp, "beobachtungsturm")
      }
      else if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__OBSERVATIONTOWER__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=beobachtungsturm -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "tower") && TAG_EXISTS(tower_type, "observation") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "man_made") == 0) {
        TAG_WRITE(*keyp, "beobachtungsturm")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__OBSERVATIONTOWER__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=beobachtungsturm -> %s\n", fzk_value);
  }

  // Funkturm (s.a. "mast")
  // ----------------------
  else if ( TAG_EXISTS(man_made, "tower") && TAG_EXISTS(tower_type, "communication") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "man_made") == 0) {
        TAG_WRITE(*keyp, "funkturm")
      }
      else if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__COMMUNICATIONTOWER__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=funkturm -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "tower") && TAG_EXISTS(tower_type, "communication") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "man_made") == 0) {
        TAG_WRITE(*keyp, "funkturm")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__COMMUNICATIONTOWER__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=funkturm -> %s\n", fzk_value);
  }

  // allgemeiner Turm (z.B. auch Kirchturm)
  // ----------------
  else if ( TAG_EXISTS(man_made, "tower") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__TOWER__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=tower -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "tower") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__TOWER__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=tower -> %s\n", fzk_value);
  }

  // Klaeranlage
  // -----------
  else if ( TAG_EXISTS(man_made, "wastewater_plant") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__WASTEWATERPLANT__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=wastewater_plant -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "wastewater_plant") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__WASTEWATERPLANT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=wastewater_plant -> %s\n", fzk_value);
  }

  // Wassermuehle
  // ------------
  else if ( TAG_EXISTS(man_made, "watermill") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__WATERMILL__);
      //   TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node man_made=watermill -> %s\n", fzk_value);
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "watermill") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__WATERMILL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=watermill -> %s\n", fzk_value);
  }

  // Wasserturm
  // ----------
  else if ( TAG_EXISTS(man_made, "water_tower") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__WATERTOWER__);
      //   TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node man_made=water_tower -> %s\n", fzk_value);
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "water_tower") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__WATERTOWER__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=water_tower -> %s\n", fzk_value);
  }

  // Wasserwerk
  // ----------
  else if ( TAG_EXISTS(man_made, "water_works") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__WATERWORKS__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=water_works -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "water_works") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__WATERWORKS__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=water_works -> %s\n", fzk_value);
  }

  // (Tief-)Brunnen (Bauwerk zur Wassergewinnung aus einem Grundwasserleiter (Aquifer))
  // --------------
  else if ( TAG_EXISTS(man_made, "water_well") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__WATERWELL__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=water_well -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "water_well") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__WATERWELL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=water_well -> %s\n", fzk_value);
  }

  // Historische Windmuehle
  // ----------------------
  else if ( TAG_EXISTS(man_made, "windmill") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__WINDMILL__);
      //   TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node man_made=windmill -> %s\n", fzk_value);
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "windmill") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__WINDMILL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=windmill -> %s\n", fzk_value);
  }

  // Industriebauten (Anlage, Werk)
  // ---------------
  else if ( TAG_EXISTS(man_made, "works") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__INDUSTRIALPLANT__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node man_made=works -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(man_made, "works") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__INDUSTRIALPLANT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node man_made=works -> %s\n", fzk_value);
  }

  // Eingang zu einer Hoehle
  // -----------------------
  else if ( TAG_EXISTS(natural, "cave_entrance") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__CAVEENTRANCE__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node natural=cave_entrance -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(natural, "cave_entrance") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CAVEENTRANCE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node natural=cave_entrance -> %s\n", fzk_value);
  }

  // Klippe (abgeleitete POIs nicht beruecksichtigen)
  // ------
  else if ( TAG_EXISTS(natural, "cliff") && KEY_EXISTS(name) && KEY_NOT_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__CLIFF__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node natural=cliff -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(natural, "cliff") && KEY_NOT_EXISTS(name) && KEY_NOT_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CLIFF__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node natural=cliff -> %s\n", fzk_value);
  }

  // Pass (markiert die hoechste Stelle eines Gebirgspasses)
  // ----
  else if ( TAG_EXISTS(mountain_pass, "yes") && KEY_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // Beispiel: "Timberpass (2187)"
        snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_ele));
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node mountain_pass=yes && ele=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(mountain_pass, "yes") && KEY_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // Beispiel: "(1874)"
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "(%ld)", atol (valp_ele));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node mountain_pass=yes && ele=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(mountain_pass, "yes") && KEY_NOT_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Joch, Sattel
  // ------------
  else if ( ( TAG_EXISTS(natural, "col") || TAG_EXISTS(natural, "saddle") ) && KEY_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // Beispiel: "Teufelsjoch (2187)"
        snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_ele));
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node natural=col/saddle && ele=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( ( TAG_EXISTS(natural, "col") || TAG_EXISTS(natural, "saddle") ) && KEY_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // Beispiel: "(1874)"
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "(%ld)", atol (valp_ele));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node natural=col/saddle && ele=* -> %s\n", fzk_value);
  }
  else if ( ( TAG_EXISTS(natural, "col") || TAG_EXISTS(natural, "saddle") ) && KEY_NOT_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Bergspitze, Gipfel
  // ------------------
  else if ( TAG_EXISTS(natural, "peak") && KEY_EXISTS(ele) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // Beispiel: "Ameringkogel (2187)"
        // Nachkommastellen entfernen (Beispiel: "187.61" -> "187")
        snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_ele));
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node natural=peak && ele=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(natural, "peak") && KEY_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // Beispiel: "(1874)"
    // Nachkommastellen entfernen (Beispiel: "187.61" -> "187")
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "(%ld)", atol (valp_ele));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node natural=peak && ele=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(natural, "peak") && KEY_NOT_EXISTS(ele) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Quelle
  // ------
  else if ( TAG_EXISTS(natural, "spring") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__SPRING__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node natural=spring -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(natural, "spring") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__SPRING__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node natural=spring -> %s\n", fzk_value);
  }

  // Grosser Stein, Findling
  // -----------------------
  else if ( TAG_EXISTS(natural, "stone") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(natural, "stone") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__STONE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node natural=stone -> %s\n", fzk_value);
  }

  // Einzelner oder signifikanter Baum (als Landmarke)
  // ---------------------------------
  else if ( TAG_EXISTS(natural, "tree") && TAG_EXISTS(denotation, "landmark") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        TAG_WRITE(*keyp, "landmarke_baum")
      }
      else if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__TREE__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node natural=landmarke_baum -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(natural, "tree") && TAG_EXISTS(denotation, "landmark") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        TAG_WRITE(*keyp, "landmarke_baum")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__TREE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node natural=landmarke_baum -> %s\n", fzk_value);
  }

  // Vulkan (ruhend, erloschen, aktiv)
  // ------
  else if ( TAG_EXISTS(natural, "volcano") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__VOLCANO__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node natural=volcano -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(natural, "volcano") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__VOLCANO__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node natural=volcano -> %s\n", fzk_value);
  }

  // Ausgrabungsstaette, oberirdisch sichtbares Bodendenkmal
  // -------------------------------------------------------
  else if ( TAG_EXISTS(historic, "archaeological_site") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "archaeological_site") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__ARCHAELOGICALSITE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node historic=archaeological_site -> %s\n", fzk_value);
  }

  // Schlachtfeld
  // ------------
  else if ( TAG_EXISTS(historic, "battlefield") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "battlefield") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__BATTLEFIELD__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node historic=battlefield -> %s\n", fzk_value);
  }

  // Historischer Grenzstein
  // -----------------------
  else if ( TAG_EXISTS(historic, "boundary_stone") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "boundary_stone") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__BOUNDARYSTONE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node historic=boundary_stone -> %s\n", fzk_value);
  }

  // Burg oder Schloss
  // -----------------
  else if ( TAG_EXISTS(historic, "castle") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "castle") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CASTLE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node historic=castle -> %s\n", fzk_value);
  }

  // Militaerisches Fort (ist gegenueber einer Burg moderner)
  // --------------------------------------------------------
  else if ( TAG_EXISTS(historic, "fort") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "fort") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__FORT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node historic=fort -> %s\n", fzk_value);
  }

  // Gedenkstaette ("Stolperstein" - Gedenk-/Pflastersteine mit den Inschriften von Opfern des 3. Reiches)
  // -------------
  else if ( TAG_EXISTS(historic, "memorial") && TAG_EXISTS(memorial_type, "stolperstein") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "historic") == 0) {
        TAG_WRITE(*keyp, "memorial_stolperstein")
        // printf ("node historic=memorial && memorial_type=stolperstein >= historic=memorial_stolperstein\n");
      }
      else if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, "Gedenkstein");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node historic=memorial -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "memorial") && TAG_EXISTS(memorial_type, "stolperstein") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "historic") == 0) {
        TAG_WRITE(*keyp, "memorial_stolperstein")
        // printf ("node historic=memorial >= historic=memorial_stolperstein\n");
      }
      else if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s", "Gedenkstein");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node historic=memorial -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Gedenktafel / -plakette
  // -----------------------
  else if ( TAG_EXISTS(historic, "memorial") && (TAG_EXISTS(memorial_type, "plate") || TAG_EXISTS(memorial_type, "plaque")) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "historic") == 0) {
        TAG_WRITE(*keyp, "memorial_plate")
        // printf ("node historic=memorial && memorial_type=plate|plaque >= historic=memorial_plate\n");
      }
      else if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, "Gedenktafel");
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node historic=memorial -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "memorial") && (TAG_EXISTS(memorial_type, "plate") || TAG_EXISTS(memorial_type, "plaque")) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "historic") == 0) {
        TAG_WRITE(*keyp, "memorial_plate")
        // printf ("node historic=memorial && memorial_type=plate|plaque >= historic=memorial_plate\n");
      }
      else if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s", "memorial plate");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node historic=memorial -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Gedenkstaette (aehnlich historic=monument, aber ueblicherweise kleiner)
  // -------------
  else if ( TAG_EXISTS(historic, "memorial") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "memorial") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__MEMORIAL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node historic=memorial -> %s\n", fzk_value);
  }

  // Denkmal, Standbild
  // ------------------
  else if ( TAG_EXISTS(historic, "monument") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "monument") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__MONUMENT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node historic=monument -> %s\n", fzk_value);
  }

  // Bunkerruine
  // -----------
  else if ( TAG_EXISTS(historic, "ruins") && TAG_EXISTS(military, "bunker") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s, %s)", *valp, $__RUINS__, $__BUNKER__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node historic=ruins && military=bunker -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "ruins") && TAG_EXISTS(military, "bunker") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RUINS__, $__BUNKER__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node historic=ruins && military=bunker -> %s\n", fzk_value);
  }

  // Ruine
  // -----
  else if ( TAG_EXISTS(historic, "ruins") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__RUINS__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node historic=ruins -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "ruins") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__RUINS__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node historic=ruins -> %s\n", fzk_value);
  }

  // Runestone (typically a raised stone with a runic inscription).
  // ---------
  else if ( TAG_EXISTS(historic, "rune_stone") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__RUNESTONE__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node historic=rune_stone -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "rune_stone") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__RUNESTONE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node historic=rune_stone -> %s\n", fzk_value);
  }

  // Historisches Wegkreuz (ueblicherweise christlich)
  // ---------------------
  else if ( TAG_EXISTS(historic, "wayside_cross") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "wayside_cross") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__WAYSIDECROSS__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node historic=wayside_cross -> %s\n", fzk_value);
  }

  // Historischer Schrein (oft mit religioeser Darstellung)
  // --------------------
  else if ( TAG_EXISTS(historic, "wayside_shrine") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "wayside_shrine") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__WAYSIDESHRINE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node historic=wayside_shrine -> %s\n", fzk_value);
  }

  // Wrack (gesunkenes oder gestrandetes Schiff)
  // -----
  else if ( TAG_EXISTS(historic, "wreck") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(historic, "wreck") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__WRECK__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node historic=wreck -> %s\n", fzk_value);
  }

  // Restaurant (mit Angabe der Art der Kueche)
  // ----------
  else if ( TAG_EXISTS(amenity, "restaurant") && KEY_EXISTS(cuisine) && KEY_EXISTS(name) ) {
    // Tags schreiben; amenity-Value modifizieren; name-Value modifizieren
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "restaurant_cuisine")
      }
      else if (strcmp (*keyp, "name") == 0) {
        if (strcmp (valp_cuisine, "american") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__AMERICAN__); }
        else if (strcmp (valp_cuisine, "burger") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__AMERICAN__); }
        else if (strcmp (valp_cuisine, "thai") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__THAI__); 	 }
        else if (strcmp (valp_cuisine, "asian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__ASIAN__); }
        else if (strcmp (valp_cuisine, "japanese") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__JAPANESE__); }
        else if (strcmp (valp_cuisine, "korean") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__KOREAN__); }
        else if (strcmp (valp_cuisine, "vietnamese") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__VIETNAMESE__); }
        else if (strcmp (valp_cuisine, "chinese") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__CHINESE__); }
        else if (strcmp (valp_cuisine, "italian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__ITALIAN__); }
        else if (strcmp (valp_cuisine, "mediterranean") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__MEDITERRANEAN__); }
        else if (strcmp (valp_cuisine, "mexican") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__MEXICAN__); }
        else if (strcmp (valp_cuisine, "spanish") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__SPANISH__); }
        else if (strcmp (valp_cuisine, "seafood") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__SEAFOOD__); }
        else if (strcmp (valp_cuisine, "fish") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__FISH__); }
        else if (strcmp (valp_cuisine, "sushi") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__SUSHI__); }
        else if (strcmp (valp_cuisine, "steak_house") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__STEAKHOUSE__); }
        else if (strcmp (valp_cuisine, "coffee_shop") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__COFFEESHOP__); }
        else if (strcmp (valp_cuisine, "french") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__FRENCH__); }
        else if (strcmp (valp_cuisine, "german") == 0) {  snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__GERMAN__); }
        else if (strcmp (valp_cuisine, "bavarian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__BAVARIAN__); }
        else if (strcmp (valp_cuisine, "swabian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__SWABIAN__); }
        else if (strcmp (valp_cuisine, "british") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__BRITISH__); }
        else if (strcmp (valp_cuisine, "speciality") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__SPECIALITY__); }
        else if (strcmp (valp_cuisine, "greek") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__GREEK__); }
        else if (strcmp (valp_cuisine, "turkish") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__TURKISH__); }
        else if (strcmp (valp_cuisine, "indian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__INDIAN__); }
        else if (strcmp (valp_cuisine, "arab") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__ARAB__); }
        else if (strcmp (valp_cuisine, "portuguese") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__PORTUGUESE__); }
        else if (strcmp (valp_cuisine, "vegetarian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__VEGETARIAN__); }
        else if (strcmp (valp_cuisine, "austrian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__AUSTRIAN__); }
        else if (strcmp (valp_cuisine, "persian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__PERSIAN__); }
        else if (strcmp (valp_cuisine, "lebanese") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__LEBANESE__); }
        else if (strcmp (valp_cuisine, "african") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__AFRICAN__); }
        else if (strcmp (valp_cuisine, "russian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__RUSSIAN__); }
        else if (strcmp (valp_cuisine, "yugoslavian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__YUGOSLAVIAN__); }
        else if (strcmp (valp_cuisine, "ice_cream") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__ICECREAMRESTAURANT__); }
        else if (strcmp (valp_cuisine, "international") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__INTERNATIONAL__); }
        else if (strcmp (valp_cuisine, "regional") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__REGIONAL__); }
        else if (strcmp (valp_cuisine, "balkan") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__BALKAN__); }
        else if (strcmp (valp_cuisine, "pizza") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__PIZZA__); }
        else {
          snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, valp_cuisine);
        }
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=restaurant && cuisine=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "restaurant") && KEY_EXISTS(cuisine) && KEY_NOT_EXISTS(name) ) {
    // Tags schreiben; amenity-Value modifizieren; name-Tag erzeugen
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "restaurant_cuisine")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    if (strcmp (valp_cuisine, "american") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__AMERICAN__); }
    else if (strcmp (valp_cuisine, "burger") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__AMERICAN__); }
    else if (strcmp (valp_cuisine, "thai") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__THAI__); }
    else if (strcmp (valp_cuisine, "asian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__ASIAN__); }
    else if (strcmp (valp_cuisine, "japanese") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__JAPANESE__); }
    else if (strcmp (valp_cuisine, "korean") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__KOREAN__); }
    else if (strcmp (valp_cuisine, "vietnamese") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__VIETNAMESE__); }
    else if (strcmp (valp_cuisine, "chinese") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__CHINESE__); }
    else if (strcmp (valp_cuisine, "italian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__ITALIAN__); }
    else if (strcmp (valp_cuisine, "mediterranean") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__MEDITERRANEAN__); }
    else if (strcmp (valp_cuisine, "mexican") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__MEXICAN__); }
    else if (strcmp (valp_cuisine, "spanish") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__SPANISH__); }
    else if (strcmp (valp_cuisine, "seafood") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__SEAFOOD__); }
    else if (strcmp (valp_cuisine, "fish") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__FISH__); }
    else if (strcmp (valp_cuisine, "sushi") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__SUSHI__); }
    else if (strcmp (valp_cuisine, "steak_house") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__STEAKHOUSE__); }
    else if (strcmp (valp_cuisine, "coffee_shop") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__COFFEESHOP__); }
    else if (strcmp (valp_cuisine, "french") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__FRENCH__); }
    else if (strcmp (valp_cuisine, "german") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__GERMAN__); }
    else if (strcmp (valp_cuisine, "bavarian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__BAVARIAN__); }
    else if (strcmp (valp_cuisine, "swabian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__SWABIAN__); }
    else if (strcmp (valp_cuisine, "british") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__BRITISH__); }
    else if (strcmp (valp_cuisine, "speciality") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__SPECIALITY__); }
    else if (strcmp (valp_cuisine, "greek") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__GREEK__); }
    else if (strcmp (valp_cuisine, "turkish") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__TURKISH__); }
    else if (strcmp (valp_cuisine, "indian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__INDIAN__); }
    else if (strcmp (valp_cuisine, "arab") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__ARAB__); }
    else if (strcmp (valp_cuisine, "portuguese") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__PORTUGUESE__); }
    else if (strcmp (valp_cuisine, "vegetarian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__VEGETARIAN__); }
    else if (strcmp (valp_cuisine, "austrian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__AUSTRIAN__); }
    else if (strcmp (valp_cuisine, "persian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__PERSIAN__); }
    else if (strcmp (valp_cuisine, "lebanese") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__LEBANESE__); }
    else if (strcmp (valp_cuisine, "african") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__AFRICAN__); }
    else if (strcmp (valp_cuisine, "russian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__RUSSIAN__); }
    else if (strcmp (valp_cuisine, "yugoslavian") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__YUGOSLAVIAN__); }
    else if (strcmp (valp_cuisine, "ice_cream") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__ICECREAMRESTAURANT__); }
    else if (strcmp (valp_cuisine, "international") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__INTERNATIONAL__); }
    else if (strcmp (valp_cuisine, "regional") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__REGIONAL__); }
    else if (strcmp (valp_cuisine, "balkan") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__BALKAN__); }
    else if (strcmp (valp_cuisine, "pizza") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, $__PIZZA__); }
    else {
      snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RESTAURANT__, valp_cuisine);
    }
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=restaurant && cuisine=* -> %s\n", fzk_value);
  }

  // Alle verbleidenden Restaurants
  // ------------------------------
  else if ( TAG_EXISTS(amenity, "restaurant") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__RESTAURANT__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=restaurant -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "restaurant") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__RESTAURANT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=restaurant -> %s\n", fzk_value);
  }

  // Bar, Nachtlokal (es werden hauptsaechlich alkoholische Getraenke serviert)
  // ---------------
  else if ( TAG_EXISTS(amenity, "bar") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "bar") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__BAR__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=bar -> %s\n", fzk_value);
  }

  // Biergarten
  // ----------
  else if ( TAG_EXISTS(amenity, "biergarten") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "biergarten") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__BIERGARTEN__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=biergarten -> %s\n", fzk_value);
  }

  // Cafe, Bistro
  // ------------
  else if ( TAG_EXISTS(amenity, "cafe") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "cafe") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CAFE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=cafe -> %s\n", fzk_value);
  }

  // Schnell-Restaurant, Imbiss
  // --------------------------
  else if ( TAG_EXISTS(amenity, "fast_food") && KEY_EXISTS(cuisine) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "fast_food_cuisine")
      }
      else if (strcmp (*keyp, "name") == 0) {
        valp_cuisine[0] = toupper(valp_cuisine[0]);
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s, %s)", *valp, valp_cuisine, $__FASTFOOD__);
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, valp_cuisine);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=fast_food && cuisine=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "fast_food") && KEY_EXISTS(cuisine) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "fast_food_cuisine")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__FASTFOOD__, valp_cuisine);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=fast_food && cuisine=* -> %s\n", fzk_value);
  }

  // Alle verbleibenden Schnell-Restaurants, Imbisse
  // -----------------------------------------------
  else if ( TAG_EXISTS(amenity, "fast_food") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "fast_food") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__FASTFOOD__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=fast_food -> %s\n", fzk_value);
  }

  // Eiscafee, Eisdiele
  // ------------------
  else if ( TAG_EXISTS(amenity, "ice_cream") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "ice_cream") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__ICECREAM__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=ice_cream -> %s\n", fzk_value);
  }

  // Kneipe
  // ------
  else if ( TAG_EXISTS(amenity, "pub") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "pub") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__PUB__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=pub -> %s\n", fzk_value);
  }

  // Kindergarten
  // ------------
  else if ( TAG_EXISTS(amenity, "kindergarten") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "kindergarten") && KEY_NOT_EXISTS(name) ) {
    // Objekt nicht übernehmen
  }

  // Schule
  // ------
  else if ( TAG_EXISTS(amenity, "school") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "school") && KEY_NOT_EXISTS(name) ) {
    // Objekt nicht übernehmen
  }

  // Hochschule
  // ----------
  else if ( TAG_EXISTS(amenity, "college") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "college") && KEY_NOT_EXISTS(name) ) {
    // Objekt nicht übernehmen
  }

  // Bibliothek, Buecherei
  // ---------------------
  else if ( TAG_EXISTS(amenity, "library") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__LIBRARY__);
        // TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=library -> %s\n", fzk_value);
        TAG_WRITE(*keyp, *valp)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "library") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__LIBRARY__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=library -> %s\n", fzk_value);
  }

  // Universitaet (Campus oder Gebaeude)
  // ------------
  else if ( TAG_EXISTS(amenity, "university") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "university") && KEY_NOT_EXISTS(name) ) {
    // Objekt nicht übernehmen
  }

  // Fahrradverleih
  // --------------
  else if ( TAG_EXISTS(amenity, "bicycle_rental") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "bicycle_rental") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__BICYCLERENTAL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=bicycle_rental -> %s\n", fzk_value);
  }

  // Fahrradparkplatz
  // ----------------
  else if ( TAG_EXISTS(amenity, "bicycle_parking") ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Grillplatz (Barbecue (BBQ))
  // ----------
  else if ( TAG_EXISTS(amenity, "bbq") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "bbq") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    TAG_WRITE("name", "BBQ")
    // printf ("node amenity=bbq -> name=BBQ\n");
  }

  // Busbahnhof (oder grosse Haltestelle)
  // ----------
  else if ( TAG_EXISTS(amenity, "bus_station") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__BUSSTATION__);
        // TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=bus_station -> %s\n", fzk_value);
        TAG_WRITE(*keyp, *valp)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "bus_station") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__BUSSTATION__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=bus_station -> %s\n", fzk_value);
  }
  // else if ( TAG_EXISTS(public_transport, "stop_position") && TAG_EXISTS(bus, "yes") && KEY_EXISTS(name) ) {
  //   FOR_ALL_TAGS
  //     if (strcmp (*keyp, "public_transport") == 0) {
  //       TAG_WRITE(*keyp, "bushaltestelle")
  //       // printf ("node public_transport=stop_position && bus=yes -> public_transport=bushaltestelle\n");
  //     }
  //     else if (strcmp (*keyp, "name") == 0) {
  //       // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__BUSSTOP__);
  //       // TAG_WRITE(*keyp, fzk_value)
  //       // printf ("node public_transport=stop_position && bus=yes -> %s\n", fzk_value);
  //       TAG_WRITE(*keyp, *valp)
  //     }
  //     else {
  //       TAG_WRITE(*keyp, *valp)
  //     }
  //   END_FOR_ALL_TAGS
  // }
  // else if ( TAG_EXISTS(public_transport, "stop_position") && TAG_EXISTS(bus, "yes") && KEY_NOT_EXISTS(name) ) {
  //   FOR_ALL_TAGS
  //     if (strcmp (*keyp, "public_transport") == 0) {
  //       TAG_WRITE(*keyp, "bushaltestelle")
  //       // printf ("node public_transport=stop_position && bus=yes -> public_transport=bushaltestelle\n");
  //     }
  //     else {
  //       TAG_WRITE(*keyp, *valp)
  //     }
  //   END_FOR_ALL_TAGS
  //   strcpy (fzk_key, "name");
  //   snprintf (fzk_value, sizeof(fzk_value), "%s", $__BUSSTOP__);
  //   TAG_WRITE(fzk_key, fzk_value)
  //   // printf ("node public_transport=stop_position && tram=yes -> %s\n", fzk_value);
  // }

  // Automobil-Club
  // --------------
  else if ( TAG_EXISTS(amenity, "car_club") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "car_club") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CARCLUB__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=car_club -> %s\n", fzk_value);
  }

  // Autoverleih
  // -----------
  else if ( TAG_EXISTS(amenity, "car_rental") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "car_rental") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CARRENTAL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=car_rental -> %s\n", fzk_value);
  }

  // Carsharing-Station
  // ------------------
  else if ( TAG_EXISTS(amenity, "car_sharing") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__CARSHARING__);
      //   TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node amenity=car_sharing -> %s\n", fzk_value);
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "car_sharing") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CARSHARING__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=car_sharing -> %s\n", fzk_value);
  }

  // Autowaschanlage
  // ---------------
  else if ( TAG_EXISTS(amenity, "car_wash") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__CARWASH__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=car_wash -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "car_wash") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CARWASH__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=car_wash -> %s\n", fzk_value);
  }

  // Faehrterminal
  // -------------
  else if ( TAG_EXISTS(amenity, "ferry_terminal") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__FERRYTERMINAL__);
      //   TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node amenity=ferry_terminal -> %s\n", fzk_value);
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "ferry_terminal") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__FERRYTERMINAL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=ferry_terminal -> %s\n", fzk_value);
  }

  // Tankstelle (Reihenfolge der Benennung: Brand, Operator, Name)
  // ----------
  else if ( TAG_EXISTS(amenity, "fuel") && KEY_EXISTS(brand) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", valp_brand, $__FUELSTATION__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", valp_brand);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=fuel && brand=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "fuel") && KEY_EXISTS(brand) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", valp_brand, $__FUELSTATION__);
    snprintf (fzk_value, sizeof(fzk_value), "%s", valp_brand);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=fuel -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(amenity, "fuel") && KEY_EXISTS(operator) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", valp_operator, $__FUELSTATION__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", valp_operator);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=fuel && operator=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "fuel") && KEY_EXISTS(operator) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", valp_operator, $__FUELSTATION__);
    snprintf (fzk_value, sizeof(fzk_value), "%s", valp_operator);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=fuel && operator=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(amenity, "fuel") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__FUELSTATION__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=fuel && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "fuel") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__FUELSTATION__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=fuel -> %s\n", fzk_value);
  }

  // Ladestation (Reihenfolge der Benennung: Network, Operator, Name)
  // ----------
  else if ( TAG_EXISTS(amenity, "charging_station") && KEY_EXISTS(network) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s", valp_network);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=charging_station && network=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "charging_station") && KEY_EXISTS(network) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", valp_network);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=charging_station -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(amenity, "charging_station") && KEY_EXISTS(operator) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s", valp_operator);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=charging_station && operator=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "charging_station") && KEY_EXISTS(operator) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", valp_operator);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=charging_station && operator=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(amenity, "charging_station") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=charging_station && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "charging_station") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CHARGINGSTATION__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=charging_station -> %s\n", fzk_value);
  }

  // Parkplatz privat (access=private oder access=no).
  // ----------------
  else if ( TAG_EXISTS(amenity, "parking") && (TAG_EXISTS(access, "private") || TAG_EXISTS(access, "no")) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "parken_privat")
      }
      else if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=parking && access=private|no && name=*-> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "parking") && (TAG_EXISTS(access, "private") || TAG_EXISTS(access, "no")) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
     if (strcmp (*keyp, "amenity") == 0) {
       TAG_WRITE(*keyp, "parken_privat")
     }
     else {
       TAG_WRITE(*keyp, *valp)
     }
    END_FOR_ALL_TAGS
    // strcpy (fzk_key, "name");
    // snprintf (fzk_value, sizeof(fzk_value), "%s", $__PRIVATE__);
    // TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=parking && access=private|no -> %s\n", fzk_value);
  }

  // Parkplatz (Parken eingeschraenkt, Stellplaetze angegeben)
  // ---------
  else if ( TAG_EXISTS(amenity, "parking") && KEY_EXISTS(access) && KEY_EXISTS(capacity) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        if (atol (valp_capacity) >= 10) {
          TAG_WRITE(*keyp, "parken_eingeschraenkt_10_9999")
        }
        else {
          TAG_WRITE(*keyp, "parken_eingeschraenkt_1_9")
        }
      }
      else if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%ld, %s %s)", *valp, atol (valp_capacity), $__PARKING__, $__RESTRICTED__);
        snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_capacity));
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=parking && access=* && capacity=* && name=*-> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "parking") && KEY_EXISTS(access) && KEY_EXISTS(capacity) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
     if (strcmp (*keyp, "amenity") == 0) {
       if (atol (valp_capacity) >= 10) {
         TAG_WRITE(*keyp, "parken_eingeschraenkt_10_9999")
       }
       else {
         TAG_WRITE(*keyp, "parken_eingeschraenkt_1_9")
       }
     }
     else {
       TAG_WRITE(*keyp, *valp)
     }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", $__RESTRICTED__, atol (valp_capacity));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=parking && access=* && capacity=* -> %s\n", fzk_value);
  }

  // Parkplatz (Parken eingeschraenkt, Stellplaetze nicht angegeben)
  // ---------
  else if ( TAG_EXISTS(amenity, "parking") && KEY_EXISTS(access) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "parken_eingeschraenkt")
      }
      // else if (strcmp (*keyp, "name") == 0) {
      //   snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__RESTRICTED__);
      //   TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node amenity=parking && access=* && name=*-> %s\n", fzk_value);
      // }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "parking") && KEY_EXISTS(access) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
     if (strcmp (*keyp, "amenity") == 0) {
       TAG_WRITE(*keyp, "parken_eingeschraenkt")
     }
     else {
       TAG_WRITE(*keyp, *valp)
     }
    END_FOR_ALL_TAGS
    // strcpy (fzk_key, "name");
    // snprintf (fzk_value, sizeof(fzk_value), "%s", $__RESTRICTED__);
    // TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=parking && access=* -> %s\n", fzk_value);
  }

  // Parkplatz (Parken uneingeschraenkt, Stellplaetze angegeben)
  // ---------
  else if ( TAG_EXISTS(amenity, "parking") && KEY_EXISTS(capacity) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        if (atol (valp_capacity) >= 10) {
          TAG_WRITE(*keyp, "parken_uneingeschraenkt_10_9999")
        }
        else {
          TAG_WRITE(*keyp, "parken_uneingeschraenkt_1_9")
        }
      }
      else if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%ld, %s)", *valp, atol (valp_capacity), $__PARKING__);
        snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", *valp, atol (valp_capacity));
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=parking && capacity=* && name=*-> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "parking") && KEY_EXISTS(capacity) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
     if (strcmp (*keyp, "amenity") == 0) {
        if (atol (valp_capacity) >= 10) {
          TAG_WRITE(*keyp, "parken_uneingeschraenkt_10_9999")
        }
        else {
          TAG_WRITE(*keyp, "parken_uneingeschraenkt_1_9")
        }
     }
     else {
       TAG_WRITE(*keyp, *valp)
     }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%ld)", $__PARKING__, atol (valp_capacity));
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=parking && capacity=* -> %s\n", fzk_value);
  }

  // Parkplatz (Parken uneingeschraenkt, Stellplaetze nicht angegeben)
  // ---------
  else if ( TAG_EXISTS(amenity, "parking") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "parken_uneingeschraenkt")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "parking") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "parken_uneingeschraenkt")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    // strcpy (fzk_key, "name");
    // snprintf (fzk_value, sizeof(fzk_value), "%s", $__PARKING__);
    // TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=parking -> %s\n", fzk_value);
  }

  // Geldautomat (Reihenfolge: Name, Operator)
  // -----------
  else if ( TAG_EXISTS(amenity, "atm") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "atm") && KEY_EXISTS(operator) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", valp_operator);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=atm && operator=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(amenity, "atm") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__ATM__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=atm -> %s\n", fzk_value);
  }

  // Bank mit Geldautomat (Reihenfolge: Name, Operator)
  // --------------------
  else if ( TAG_EXISTS(amenity, "bank") && TAG_EXISTS(atm, "yes") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "bank_geldautomat")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "bank") && TAG_EXISTS(atm, "yes") && KEY_EXISTS(operator) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "bank_geldautomat")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", valp_operator);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=bank && atm=yes && operator=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(amenity, "bank") && TAG_EXISTS(atm, "yes") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "bank_geldautomat")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__ATM__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=bank && atm=yes -> %s\n", fzk_value);
  }

  // Bank ohne Geldautomat
  // ---------------------
  else if ( TAG_EXISTS(amenity, "bank") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "bank") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__BANK__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=bank -> %s\n", fzk_value);
  }

  // Apotheke
  // --------
  else if ( TAG_EXISTS(amenity, "pharmacy") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "pharmacy") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__PHARMACY__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=pharmacy -> %s\n", fzk_value);
  }

  // Krankenhaus
  // -----------
  else if ( TAG_EXISTS(amenity, "hospital") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__HOSPITAL__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=hospital -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "hospital") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__HOSPITAL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=hospital -> %s\n", fzk_value);
  }

  // Klinik
  // --------
  else if ( TAG_EXISTS(amenity, "clinic") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "clinic") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CLINIC__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=clinic -> %s\n", fzk_value);
  }

  // Arztpraxis
  // --------
  else if ( TAG_EXISTS(amenity, "doctors") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "doctors") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__DOCTOR__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=doctors -> %s\n", fzk_value);
  }

  // Tierarzt
  // --------
  else if ( TAG_EXISTS(amenity, "veterinary") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "veterinary") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__VETERINARY__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=veterinary -> %s\n", fzk_value);
  }

  // Kunstzentrum
  // ------------
  else if ( TAG_EXISTS(amenity, "arts_centre") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "arts_centre") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__ARTSCENTRE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=arts_centre -> %s\n", fzk_value);
  }

  // Casino
  // ------
  else if ( TAG_EXISTS(amenity, "casino") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "casino") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CASINO__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=casino -> %s\n", fzk_value);
  }

  // Kino
  // ----
  else if ( TAG_EXISTS(amenity, "cinema") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "cinema") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CINEMA__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=cinema -> %s\n", fzk_value);
  }

  // Lokale Veranstaltungshalle
  // --------------------------
  else if ( (TAG_EXISTS(amenity, "community_centre") || TAG_EXISTS(amenity, "community_hall")) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__COMMUNITYCENTRE__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=community_centre || amenity=community_hall -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( (TAG_EXISTS(amenity, "community_centre") || TAG_EXISTS(amenity, "community_hall")) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__COMMUNITYCENTRE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=community_centre || amenity=community_hall -> %s\n", fzk_value);
  }

  // Konzerthalle
  // ------------
  else if ( TAG_EXISTS(amenity, "concert_hall") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__CONCERTHALL__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=concert_hall -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "concert_hall") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CONCERTHALL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=concert_hall -> %s\n", fzk_value);
  }

  // Messe, Konferenzcenter
  // ----------------------
  else if ( TAG_EXISTS(amenity, "conference_centre") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__CONFERENCECENTRE__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=conference_centre -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "conference_centre") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__CONFERENCECENTRE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=conference_centre -> %s\n", fzk_value);
  }

  // Nachtclub, Disco
  // ----------------
  else if ( TAG_EXISTS(amenity, "nightclub") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "nightclub") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__NIGHTCLUB__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=nightclub -> %s\n", fzk_value);
  }

  // social_centre (a place for free and not-for-profit activities)
  // -------------
  else if ( TAG_EXISTS(amenity, "social_centre") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__SOCIALCENTRE__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=social_centre -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "social_centre") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__SOCIALCENTRE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=social_centre -> %s\n", fzk_value);
  }

  // social_facility (a place for free and not-for-profit activities)
  // ---------------
  else if ( TAG_EXISTS(amenity, "social_facility") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__SOCIALFACILITY__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=social_facility -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "social_facility") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__SOCIALFACILITY__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=social_facility -> %s\n", fzk_value);
  }

  // Theater, Oper, Schauspielhaus
  // -----------------------------
  else if ( TAG_EXISTS(amenity, "theatre") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "theatre") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__THEATRE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=theatre -> %s\n", fzk_value);
  }

  // Gericht
  // -------
  else if ( TAG_EXISTS(amenity, "courthouse") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "courthouse") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__COURTHOUSE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=courthouse -> %s\n", fzk_value);
  }

  // Trinkwasserstelle (oeffentlich zugaenglich)
  // -----------------
  else if ( TAG_EXISTS(amenity, "drinking_water") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "drinking_water") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__DRINKINGWATER__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=drinking_water -> %s\n", fzk_value);
  }

  // Botschaft, Botschaftsgebaeude
  // -----------------------------
  else if ( TAG_EXISTS(amenity, "embassy") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "embassy") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__EMBASSY__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=embassy -> %s\n", fzk_value);
  }

  // Feuerwehrhaus (auch als Feuerwache, Ruesthaus, Brandwache bezeichnet)
  // -------------
  else if ( TAG_EXISTS(amenity, "fire_station") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "fire_station") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__FIRESTATION__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=fire_station -> %s\n", fzk_value);
  }

  // Anbetungsort (mit Religionsangabe; Kirche, Moschee, Synagoge)
  // ------------
  else if ( TAG_EXISTS(amenity, "place_of_worship") && KEY_EXISTS(religion) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        if (strcmp (valp_religion, "christian") == 0) { TAG_WRITE(*keyp, "anbetungsort_christlich") }
        else if (strcmp (valp_religion, "muslim") == 0) { TAG_WRITE(*keyp, "anbetungsort_muslimisch") }
        else if (strcmp (valp_religion, "jewish") == 0) { TAG_WRITE(*keyp, "anbetungsort_juedisch") }
        else {
          TAG_WRITE(*keyp, "anbetungsort_religion")
        }
      }
      else if (strcmp (*keyp, "name") == 0) {
        if (strcmp (valp_religion, "christian") == 0) {
          // snprintf (fzk_value, sizeof(fzk_value), "%s (%s, %s)", *valp, $__CHRISTIAN__, $__PLACEOFWORSHIP__);
          snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        }
        else if (strcmp (valp_religion, "muslim") == 0) {
          // snprintf (fzk_value, sizeof(fzk_value), "%s (%s, %s)", *valp, $__MUSLIM__, $__PLACEOFWORSHIP__);
          snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        }
        else if (strcmp (valp_religion, "jewish") == 0) {
          snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        }
        else {
          snprintf (fzk_value, sizeof(fzk_value), "%s (%s, %s)", *valp, valp_religion, $__PLACEOFWORSHIP__);
        }
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=place_of_worship && religion=* && name=* -> %s\n", fzk_value);
      }
      else if (strcmp (*keyp, "tourism") == 0) {
        // tourism entfernen (oft zusaetzlich als tourism=attraction getaggt)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "place_of_worship") && KEY_EXISTS(religion) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        if (strcmp (valp_religion, "christian") == 0) { TAG_WRITE(*keyp, "anbetungsort_christlich") }
        else if (strcmp (valp_religion, "muslim") == 0) { TAG_WRITE(*keyp, "anbetungsort_muslimisch") }
        else if (strcmp (valp_religion, "jewish") == 0) { TAG_WRITE(*keyp, "anbetungsort_juedisch") }
        else {
          TAG_WRITE(*keyp, "anbetungsort_religion")
        }
      }
      else if (strcmp (*keyp, "name") == 0) {
        if (strcmp (valp_religion, "christian") == 0) {
          // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__PLACEOFWORSHIP__, $__CHRISTIAN__);
          snprintf (fzk_value, sizeof(fzk_value), "%s", $__PLACEOFWORSHIP__);
        }
        else if (strcmp (valp_religion, "muslim") == 0) {
          // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__PLACEOFWORSHIP__, $__MUSLIM__);
          snprintf (fzk_value, sizeof(fzk_value), "%s", $__PLACEOFWORSHIP__);
        }
        else if (strcmp (valp_religion, "jewish") == 0) {
          // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__PLACEOFWORSHIP__, $__JEWISH__);
          snprintf (fzk_value, sizeof(fzk_value), "%s", $__PLACEOFWORSHIP__);
        }
        else if (strcmp (*keyp, "tourism") == 0) {
          // tourism entfernen (oft zusaetzlich als tourism=attraction getaggt)
        }
        else {
          snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__PLACEOFWORSHIP__, valp_religion);
        }
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=place_of_worship && religion=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Anbetungsort (allgemein, ohne Religionsangabe)
  // ------------
  else if ( TAG_EXISTS(amenity, "place_of_worship") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__PLACEOFWORSHIP__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=place_of_worship -> %s\n", fzk_value);
      }
      else if (strcmp (*keyp, "tourism") == 0) {
        // tourism entfernen (oft zusaetzlich als tourism=attraction getaggt)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "place_of_worship") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__PLACEOFWORSHIP__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=place_of_worship -> %s\n", fzk_value);
  }

  // Polizeistation
  // --------------
  else if ( TAG_EXISTS(amenity, "police") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__POLICE__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node amenity=police -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "police") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__POLICE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=police -> %s\n", fzk_value);
  }

  // Postamt
  // -------
  else if ( TAG_EXISTS(amenity, "post_office") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "post_office") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__POSTOFFICE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=post_office -> %s\n", fzk_value);
  }

  // Oeffentliches Gebaeude
  // ----------------------
  else if ( TAG_EXISTS(amenity, "public_building") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "public_building") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__PUBLICBUILDING__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=public_building -> %s\n", fzk_value);
  }

  // Unterstand (ueberdachter offener Platz als Schutz vor der Witterung)
  // ----------
  else if ( TAG_EXISTS(amenity, "shelter") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "shelter") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // strcpy (fzk_key, "name");
    // snprintf (fzk_value, sizeof(fzk_value), "%s", $__SHELTER__);
    // TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=shelter -> %s\n", fzk_value);
  }

  // Rathaus
  // -------
  else if ( TAG_EXISTS(amenity, "townhall") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "townhall") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__TOWNHALL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=townhall -> %s\n", fzk_value);
  }

  // (Spring-)Brunnen (fuer kulturelle, dekorative oder der Erholung dienende Zwecke)
  // ----------------
  else if ( TAG_EXISTS(amenity, "fountain") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "fountain") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // strcpy (fzk_key, "name");
    // snprintf (fzk_value, sizeof(fzk_value), "%s", $__FOUNTAIN__);
    // TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=fountain -> %s\n", fzk_value);
  }

  // (Oeffentliche) Toilette
  // -----------------------
  else if ( TAG_EXISTS(amenity, "toilets") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(amenity, "toilets") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__TOILETS__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node amenity=toilets -> %s\n", fzk_value);
  }

  // Bunkerruine
  // -----------
  else if ( TAG_EXISTS(military, "bunker") && TAG_EXISTS(ruins, "yes") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s, %s)", *valp, $__RUINS__, $__MILITARYBUNKER__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node military=bunker && ruins=yes -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(military, "bunker") && TAG_EXISTS(ruins, "yes") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", $__RUINS__, $__MILITARYBUNKER__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node military=bunker && ruins=yes -> %s\n", fzk_value);
  }

  // Bunker, Gebaeude mit sehr dicken Mauern
  // ---------------------------------------
  else if ( TAG_EXISTS(military, "bunker") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__MILITARYBUNKER__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node military=bunker -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(military, "bunker") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__MILITARYBUNKER__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node military=bunker -> %s\n", fzk_value);
  }

  // Geschaefte
  // ----------
  else if ( KEY_EXISTS(shop) && KEY_EXISTS(name) ) {
    // Tags schreiben; shop-Value modifizieren (nur bei unbekanntem Shop); name-Value modifizieren
    FOR_ALL_TAGS
      if (strcmp (*keyp, "shop") == 0) {
        if (strcmp (valp_shop, "bag") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "alcohol") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "art") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "bakery") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "beauty") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "bed") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "beverages") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "bicycle") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "books") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "boutique") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "butcher") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "car") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "car_repair") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "carpet") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "chemist") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "chocolate") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "clothes") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "coffee") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "computer") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "confectionery") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "convenience") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "copyshop") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "cosmetics") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "deli") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "department_store") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "doityourself") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "dry_cleaning") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "electronics") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "erotic") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "fabric") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "farm") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "fashion") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "fish") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "fishmonger") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "florist") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "funeral_directors") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "furniture") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "garden_centre") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "general") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "gift") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "greengrocer") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "hairdresser") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "hardware") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "hearing_aids") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "hifi") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "ice_cream") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "insurance") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "interior_decoration") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "jewelry") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "kiosk") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "kitchen") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "laundry") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "mall") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "massage") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "medical_supply") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "mobile_phone") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "motorcycle") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "music") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "musical_instrument") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "newsagent") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "optician") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "outdoor") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "paint") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "pastry") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "perfumery") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "pet") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "photo") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "seafood") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "second_hand") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "shoes") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "solarium") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "sports") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "stationery") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "supermarket") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "tailor") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "tanning") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "tattoo") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "tea") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "tobacco") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "toys") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "trade") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "travel_agency") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "tyres") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "vacant") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "variety_store") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "video") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "watches") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "wine") == 0) { TAG_WRITE(*keyp, *valp) }
        else {
          // alle anderen Geschaefte
          TAG_WRITE(*keyp, "spezialgeschaeft")
        }
      }
      else if (strcmp (*keyp, "name") == 0) {
        if (strcmp (valp_shop, "bag") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "alcohol") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "art") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "bakery") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "beauty") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "bed") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "beverages") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "bicycle") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "books") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "boutique") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "butcher") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "car") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "car_repair") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "carpet") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "chemist") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "chocolate") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "clothes") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "coffee") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "computer") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "confectionery") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "convenience") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "copyshop") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "cosmetics") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "deli") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "department_store") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "doityourself") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "dry_cleaning") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "electronics") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "erotic") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__EROTIC__); }
        else if (strcmp (valp_shop, "fabric") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "farm") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "fashion") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "fish") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "fishmonger") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "florist") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "funeral_directors") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__FUNERALDIRECTORS__); }
        else if (strcmp (valp_shop, "furniture") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "garden_centre") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "general") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "gift") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "greengrocer") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "hairdresser") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "hardware") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "hearing_aids") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "hifi") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "ice_cream") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "insurance") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__INSURANCE__); }
        else if (strcmp (valp_shop, "interior_decoration") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "jewelry") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "kiosk") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "kitchen") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "laundry") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "mall") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "massage") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__MASSAGE__); }
        else if (strcmp (valp_shop, "medical_supply") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "mobile_phone") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "motorcycle") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "music") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "musical_instrument") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "newsagent") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "optician") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "outdoor") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "paint") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "pastry") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "perfumery") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "pet") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "photo") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "seafood") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "second_hand") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "shoes") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "solarium") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__SOLARIUM__); }
        else if (strcmp (valp_shop, "sports") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "stationery") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "supermarket") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "tailor") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "tanning") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__SOLARIUM__); }
        else if (strcmp (valp_shop, "tattoo") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__TATTOO__); }
        else if (strcmp (valp_shop, "tea") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "tobacco") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "toys") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "trade") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "travel_agency") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "tyres") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "vacant") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__VACANT__); }
        else if (strcmp (valp_shop, "variety_store") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "video") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__VIDEO__); }
        else if (strcmp (valp_shop, "watches") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else if (strcmp (valp_shop, "wine") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", *valp); }
        else {
          // alle anderen Geschaefte
          snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, valp_shop);
        }
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node shop=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( KEY_EXISTS(shop) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "shop") == 0) {
        if (strcmp (valp_shop, "bakery") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "beauty") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "cosmetics") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "bicycle") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "butcher") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "car") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "car_repair") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "chemist") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "boutique") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "clothes") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "fashion") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "computer") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "confectionery") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "convenience") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "deli") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "department_store") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "doityourself") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "furniture") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "garden_centre") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "general") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "hairdresser") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "hardware") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "ice_cream") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "mall") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "shoes") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "supermarket") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "trade") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "tyres") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "alcohol") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "art") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "bed") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "beverages") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "books") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "copyshop") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "carpet") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "coffee") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "dry_cleaning") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "electronics") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "erotic") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "fabric") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "farm") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "fish") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "fishmonger") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "florist") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "funeral_directors") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "gift") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "greengrocer") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "hearing_aids") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "hifi") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "insurance") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "jewelry") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "kitchen") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "interior_decoration") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "kiosk") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "laundry") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "massage") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "mobile_phone") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "motorcycle") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "music") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "musical_instrument") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "newsagent") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "optician") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "outdoor") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "paint") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "pet") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "photo") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "seafood") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "second_hand") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "solarium") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "sports") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "stationery") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "tanning") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "tattoo") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "tailor") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "tea") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "tobacco") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "toys") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "travel_agency") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "vacant") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "variety_store") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "video") == 0) { TAG_WRITE(*keyp, *valp) }
        else if (strcmp (valp_shop, "wine") == 0) { TAG_WRITE(*keyp, *valp) }
        else {
          // alle anderen Geschaefte
          TAG_WRITE(*keyp, "spezialgeschaeft")
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    if (strcmp (valp_shop, "bakery") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__BAKERY__); }
    else if (strcmp (valp_shop, "beauty") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__COSMETICS__); }
    else if (strcmp (valp_shop, "cosmetics") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__COSMETICS__); }
    else if (strcmp (valp_shop, "bicycle") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__BICYCLESHOP__); }
    else if (strcmp (valp_shop, "butcher") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__BUTCHER__); }
    else if (strcmp (valp_shop, "car") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__CARSHOP__); }
    else if (strcmp (valp_shop, "car_repair") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__CARREPAIR__); }
    else if (strcmp (valp_shop, "chemist") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__CHEMIST__); }
    else if (strcmp (valp_shop, "boutique") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__BOUTIQUE__); }
    else if (strcmp (valp_shop, "clothes") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__CLOTHES__); }
    else if (strcmp (valp_shop, "fashion") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__FASION__); }
    else if (strcmp (valp_shop, "computer") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__COMPUTER__); }
    else if (strcmp (valp_shop, "confectionery") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__CONFECTIONERY__); }
    else if (strcmp (valp_shop, "convenience") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__CONVENIENCE__); }
    else if (strcmp (valp_shop, "deli") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__DELICACY__); }
    else if (strcmp (valp_shop, "department_store") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__DEPARTMENTSTORE__); }
    else if (strcmp (valp_shop, "doityourself") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__DOITYOURSELF__); }
    else if (strcmp (valp_shop, "furniture") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__FURNITURE__); }
    else if (strcmp (valp_shop, "garden_centre") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__GARDENCENTRE__); }
    else if (strcmp (valp_shop, "general") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__GENERALSHOP__); }
    else if (strcmp (valp_shop, "hairdresser") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__HAIRDRESSER__); }
    else if (strcmp (valp_shop, "hardware") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__HARDWARE__); }
    else if (strcmp (valp_shop, "ice_cream") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__ICECREAMSHOP__); }
    else if (strcmp (valp_shop, "mall") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__MALL__); }
    else if (strcmp (valp_shop, "shoes") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__SHOES__); }
    else if (strcmp (valp_shop, "supermarket") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__SUPERMARKET__); }
    else if (strcmp (valp_shop, "trade") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__TRADESHOP__); }
    else if (strcmp (valp_shop, "tyres") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__TYRES__); }
    else if (strcmp (valp_shop, "alcohol") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__ALCOHOL__); }
    else if (strcmp (valp_shop, "art") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__ARTSHOP__); }
    else if (strcmp (valp_shop, "bed") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__BEDSHOP__); }
    else if (strcmp (valp_shop, "beverages") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__BEVERAGES__); }
    else if (strcmp (valp_shop, "books") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__BOOKS__); }
    else if (strcmp (valp_shop, "copyshop") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__COPYSHOOP__); }
    else if (strcmp (valp_shop, "carpet") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__CARPET__); }
    else if (strcmp (valp_shop, "coffee") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__COFFEESHOP__); }
    else if (strcmp (valp_shop, "dry_cleaning") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__DRYCLEANING__); }
    else if (strcmp (valp_shop, "electronics") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__ELECTRONICS__); }
    else if (strcmp (valp_shop, "erotic") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__EROTIC__); }
    else if (strcmp (valp_shop, "fabric") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__FABRIC__); }
    else if (strcmp (valp_shop, "farm") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__FARMSHOP__); }
    else if (strcmp (valp_shop, "fish") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__FISHMONGER__); }
    else if (strcmp (valp_shop, "fishmonger") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__FISHMONGER__); }
    else if (strcmp (valp_shop, "florist") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__FLORIST__); }
    else if (strcmp (valp_shop, "funeral_directors") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__FUNERALDIRECTORS__); }
    else if (strcmp (valp_shop, "gift") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__GIFTSHOP__); }
    else if (strcmp (valp_shop, "greengrocer") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__GREENGROCER__); }
    else if (strcmp (valp_shop, "hearing_aids") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__HEARINGAIDS__); }
    else if (strcmp (valp_shop, "hifi") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__HIFI__); }
    else if (strcmp (valp_shop, "insurance") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__INSURANCE__); }
    else if (strcmp (valp_shop, "jewelry") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__JEWELRY__); }
    else if (strcmp (valp_shop, "kitchen") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__KITCHEN__); }
    else if (strcmp (valp_shop, "interior_decoration") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__INTERIORDECORATION__); }
    else if (strcmp (valp_shop, "kiosk") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__KIOSK__); }
    else if (strcmp (valp_shop, "laundry") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__LOUNDRY__); }
    else if (strcmp (valp_shop, "massage") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__MASSAGE__); }
    else if (strcmp (valp_shop, "mobile_phone") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__MOBILEPHONE__); }
    else if (strcmp (valp_shop, "motorcycle") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__MOTORCYCLE__); }
    else if (strcmp (valp_shop, "music") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__MUSICSHOP__); }
    else if (strcmp (valp_shop, "musical_instrument") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__MUSICALINSTRUMENT__); }
    else if (strcmp (valp_shop, "newsagent") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__NEWSAGENT__); }
    else if (strcmp (valp_shop, "optician") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__OPTICIAN__); }
    else if (strcmp (valp_shop, "outdoor") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__OUTDOOR__); }
    else if (strcmp (valp_shop, "paint") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__PAINT__); }
    else if (strcmp (valp_shop, "pet") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__PETSHOP__); }
    else if (strcmp (valp_shop, "photo") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__PHOTO__ ); }
    else if (strcmp (valp_shop, "seafood") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__SEAFOODSHOP__); }
    else if (strcmp (valp_shop, "second_hand") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__SECONDHAND__); }
    else if (strcmp (valp_shop, "solarium") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__SOLARIUM__); }
    else if (strcmp (valp_shop, "sports") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__SPORTSSHOP__); }
    else if (strcmp (valp_shop, "stationery") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__STATIONERY__); }
    else if (strcmp (valp_shop, "tanning") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__SOLARIUM__); }
    else if (strcmp (valp_shop, "tattoo") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__TATTOO__); }
    else if (strcmp (valp_shop, "tailor") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__TAILOR__); }
    else if (strcmp (valp_shop, "tea") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__TEASHOP__); }
    else if (strcmp (valp_shop, "tobacco") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__TOBACCO__); }
    else if (strcmp (valp_shop, "toys") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__TOYS__); }
    else if (strcmp (valp_shop, "travel_agency") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__TRAVELAGENCY__); }
    else if (strcmp (valp_shop, "vacant") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__VACANT__); }
    else if (strcmp (valp_shop, "variety_store") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__VARIETYSTORE__); }
    else if (strcmp (valp_shop, "video") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__VIDEO__); }
    else if (strcmp (valp_shop, "wine") == 0) { snprintf (fzk_value, sizeof(fzk_value), "%s", $__WINESHOP__); }
    else {
      // alle anderen Geschaefte
      snprintf (fzk_value, sizeof(fzk_value), "%s", valp_shop);
    }
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node shop=* -> %s\n", fzk_value);
  }

  // Bushaltestelle
  // --------------
  else if ( TAG_EXISTS(highway, "bus_stop") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__BUSSTOP__);
        // TAG_WRITE(*keyp, fzk_value)
        // printf ("node highway=bus_stop -> %s\n", fzk_value);
        TAG_WRITE(*keyp, *valp)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "bus_stop") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__BUSSTOP__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node highway=bus_stop -> %s\n", fzk_value);
  }

  // Autobahnausfahrt (ref sollte die Nummer oder ID der Ausfahrt enthalten)
  // ----------------
  else if ( TAG_EXISTS(highway, "motorway_junction") && KEY_EXISTS(ref) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s, %s)", *valp, valp_ref, $__MOTORWAYJUNCTION__);
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, valp_ref);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node highway=motorway_junction && ref=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "motorway_junction") && KEY_EXISTS(ref) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", valp_ref, $__MOTORWAYJUNCTION__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node highway=motorway_junction && ref=* -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(highway, "motorway_junction") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__MOTORWAYJUNCTION__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node highway=motorway_junction && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "motorway_junction") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__MOTORWAYJUNCTION__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node highway=motorway_junction -> %s\n", fzk_value);
  }

  // Rettungspunkt (ref sollte die Nummer oder ID enthalten)
  // -------------
  else if ( TAG_EXISTS(highway, "emergency_access_point") && KEY_EXISTS(ref) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s", valp_ref);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node highway=emergency_access_point && ref=* && name=* -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "emergency_access_point") && KEY_EXISTS(ref) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", valp_ref);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node highway=emergency_access_point && ref=* -> %s\n", fzk_value);
  }

  // Place where drivers can leave the road to rest, but not refuel
  // --------------------------------------------------------------
  else if ( TAG_EXISTS(highway, "rest_area") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__HIGHWAYRESTAREA__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node highway=rest_area -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "rest_area") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__HIGHWAYRESTAREA__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node highway=rest_area -> %s\n", fzk_value);
  }

  // Raststaette, auch Autobahnraststaette
  // -------------------------------------
  else if ( TAG_EXISTS(highway, "services") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "services") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__HIGHWAYSERVICEAREA__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node highway=services -> %s\n", fzk_value);
  }

  // Beach Resort (Strand mit Dienstleistungsangebot)
  // ------
  else if ( TAG_EXISTS(leisure, "beach_resort") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "beach_resort") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  //  Vogelbeobachtungsturm oder -versteck
  // ------
  else if ( TAG_EXISTS(leisure, "bird_hide") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "bird_hide") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  
  // Tanzen (Oertlichkeit, speziell zum regelmaessigen Tanzen)
  // ------
  else if ( TAG_EXISTS(leisure, "dance") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "dance") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__DANCE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node leisure=dance -> %s\n", fzk_value);
  }

  //  Angelplatz
  // ------
  else if ( TAG_EXISTS(leisure, "fishing") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "fishing") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Eislaufplatz, Eislaufbahn (in Deutschland typischerweise ein Gebaeude)
  // -------------------------
  else if ( TAG_EXISTS(leisure, "ice_rink") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__ICERINK__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node leisure=ice_rink -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "ice_rink") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__ICERINK__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node leisure=ice_rink -> %s\n", fzk_value);
  }

  // Minigolf
  // --------
  else if ( TAG_EXISTS(leisure, "miniature_golf") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "miniature_golf") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__MINIATUREGOLF__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node leisure=miniature_golf -> %s\n", fzk_value);
  }

  // Spielfeld mit Angabe der Sportart (z.B. Fussball, Tennis, Handball, Hockey)
  // ---------------------------------
  else if ( TAG_EXISTS(leisure, "pitch") && KEY_EXISTS(sport) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
 }
  else if ( TAG_EXISTS(leisure, "pitch") && KEY_EXISTS(sport) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Spielfeld ohne Angabe der Sportart
  // ----------------------------------
  else if ( TAG_EXISTS(leisure, "pitch") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__PITCH__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node leisure=pitch -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "pitch") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__PITCH__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node leisure=pitch -> %s\n", fzk_value);
  }

  // Spielplatz fuer Kinder
  // ----------------------
  else if ( TAG_EXISTS(leisure, "playground") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__PLAYGROUND__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node leisure=playground -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "playground") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // strcpy (fzk_key, "name");
    // snprintf (fzk_value, sizeof(fzk_value), "%s", $__PLAYGROUND__);
    // TAG_WRITE(fzk_key, fzk_value)
    // printf ("node leisure=playground -> %s\n", fzk_value);
  }

  // recreation_ground (gibt es nur in den Commenwealth-Staaten - hier eher identisch mit landuse=village_green)
  // -----------------
  else if ( TAG_EXISTS(leisure, "recreation_ground") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__RECREATIONGROUND__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node leisure=recreation_ground -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "recreation_ground") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__RECREATIONGROUND__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node leisure=recreation_ground -> %s\n", fzk_value);
  }

  // Sportzentrum (auch kommerzielle Freizeitsportanbieter, Fitnesscenter)
  // ------------
  else if ( TAG_EXISTS(leisure, "sports_centre") && KEY_EXISTS(name) && KEY_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "leisure") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "sports_centre_flaeche");
        TAG_WRITE(*keyp, fzk_value)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "sports_centre") && KEY_NOT_EXISTS(name) && KEY_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "leisure") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "sports_centre_flaeche");
        TAG_WRITE(*keyp, fzk_value)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__SPORTSCENTRE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node leisure=sports_centre || leisure=sports_center -> %s\n", fzk_value);
  }
  else if ( TAG_EXISTS(leisure, "sports_centre") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__SPORTSCENTRE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node leisure=sports_centre || leisure=sports_center -> %s\n", fzk_value);
  }

  // Stadion mit Angabe der Sportart (groessere Sportanlage mit permanenten, gestaffelten Sitzreihen)
  // -------------------------------
  else if ( TAG_EXISTS(leisure, "stadium") && KEY_EXISTS(sport) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "stadium") && KEY_EXISTS(sport) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Stadion ohne Angabe der Sportart (groessere Sportanlage mit permanenten, gestaffelten Sitzreihen).
  // --------------------------------
  else if ( TAG_EXISTS(leisure, "stadium") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "stadium") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__STADIUM__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node leisure=stadium -> %s\n", fzk_value);
  }

  // Laufbahn oder Rennbahn mit Angabe der Sportart (z.B. fuer Wett-, Rad-, Hunde- oder Pferderennen)
  // ----------------------------------------------
  else if ( TAG_EXISTS(leisure, "track") && KEY_EXISTS(sport) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "track") && KEY_EXISTS(sport) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Laufbahn oder Rennbahn ohne Angabe der Sportart
  // -----------------------------------------------
  else if ( TAG_EXISTS(leisure, "track") && KEY_EXISTS(name)) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        // snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__TRACK__);
        snprintf (fzk_value, sizeof(fzk_value), "%s", *valp);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node leisure=track -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "track") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__TRACK__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node leisure=track -> %s\n", fzk_value);
  }

  // Schwimmbecken mit Angabe der Sportart
  // -------------------------------------
    else if ( TAG_EXISTS(leisure, "swimming_pool") && TAG_EXISTS(access, "private") ) {
    // Private Schwimmbecken / Swimming-Pools umbenennen (ausfiltern)
    FOR_ALL_TAGS
      if (strcmp (*keyp, "leisure") == 0) {
         TAG_WRITE(*keyp, "swimming_pool_private")
         // printf ("node leisure=swimming_pool && access=private -> leisure=swimming_pool_private\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
     END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "swimming_pool")  && KEY_EXISTS(sport) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "swimming_pool") && KEY_EXISTS(sport) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Schwimmbecken ohne Angabe der Sportart
  // --------------------------------------
  else if ( TAG_EXISTS(leisure, "swimming_pool") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "swimming_pool")  && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__SWIMMINGPOOL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node leisure/amenity=swimming_pool -> %s\n", fzk_value);
  }

  // Wasserpark mit Angabe der Sportart (Vergnuegnungsstaette mit Wasserrutschen, Schwimmbecken und Umkleiden)
  // ----------------------------------
  else if ( TAG_EXISTS(leisure, "water_park") && KEY_EXISTS(sport) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "water_park") && KEY_EXISTS(sport) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Wasserpark ohne Angabe der Sportart (Vergnuegnungsstaette mit Wasserrutschen, Schwimmbecken und Umkleiden)
  // -----------------------------------
  else if ( TAG_EXISTS(leisure, "water_park") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(leisure, "water_park") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__WATERPARK__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node leisure=water_park -> %s\n", fzk_value);
  }

  // Fitness-Center/Studio (nur benannte Objekte übernehmen)
  else if ( TAG_EXISTS(leisure, "fitness_centre") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Golfanlage (nur benannte Objekte übernehmen)
  else if ( TAG_EXISTS(leisure, "golf_course") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Reitanlage (nur benannte Objekte übernehmen)
  else if ( TAG_EXISTS(leisure, "horse_riding") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Fossillagerstaette: Sedimentgesteine die sich durch eine aussergewoehnlich hohe Anzahl von Fossilien und
  // ------------------- Fossilienvielfalt und/oder durch eine besonders vollstaendige Fossilerhaltung auszeichnen.
  //
  else if ( TAG_EXISTS(geological, "palaeontological_site") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__PALEONTOLOGICALSITE__);
      //   TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node geological=palaeontological_site -> %s\n", fzk_value);
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(geological, "palaeontological_site") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__PALEONTOLOGICALSITE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node geological=palaeontological_site -> %s\n", fzk_value);
  }

  // Outcrop: A outcrop site is a place where is absent in the coverage of alteration due to exogenous agents or vegetation,
  // -------- and where, therefore, the rock macroscopic surfaces by being directly accessible to analysis.
  //
  else if ( TAG_EXISTS(geological, "outcrop") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__OUTCROP__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node geological=outcrop -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(geological, "outcrop") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__OUTCROP__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node geological=outcrop -> %s\n", fzk_value);
  }

  // Sport allgemein
  // ---------------
  // else if ( KEY_EXISTS(sport) && KEY_EXISTS(name) ) {
  //   // erster Durchlauf: Sportart ermitteln (da Tagreihenfolge alpabetisch und somit "name" vor "sport" kommt)
  //   FOR_ALL_TAGS
  //     if (strcmp (*keyp, "sport") == 0) {
  //       // Sportart merken, erster Buchstabe gross
  //       strcpy (fzk_temp, *valp);
  //       fzk_temp[0] = toupper(fzk_temp[0]);
  //     }
  //   END_FOR_ALL_TAGS
  //   // zweiter Durchlauf: sport-Tag verallgemeinern, name-Tag modifizieren
  //   FOR_ALL_TAGS
  //     if (strcmp (*keyp, "sport") == 0) {
  //       // sport-Tag verallgemeinern
  //       TAG_WRITE(*keyp, "sport_allgemein")
  //     }
  //     else if (strcmp (*keyp, "name") == 0) {
  //       snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, fzk_temp);
  //       TAG_WRITE(*keyp, fzk_value)
  //     }
  //     else {
  //       TAG_WRITE(*keyp, *valp)
  //     }
  //   END_FOR_ALL_TAGS
  //   // printf ("node sport=* -> sport=sport_allgemein + name=%s\n", fzk_value);
  // }
  // else if ( KEY_EXISTS(sport) && KEY_NOT_EXISTS(name) ) {
  //   FOR_ALL_TAGS
  //     if (strcmp (*keyp, "sport") == 0) {
  //       // Sportart merken; erster Buchstabe gross; sport-Tag verallgemeinern
  //       strcpy (fzk_temp, *valp);
  //       fzk_temp[0] = toupper(fzk_temp[0]);
  //       TAG_WRITE(*keyp, "sport_allgemein")
  //     }
  //     else {
  //       TAG_WRITE(*keyp, *valp)
  //     }
  //   END_FOR_ALL_TAGS
  //   // neues Tag (mit Wert aus "sport") erzeugen
  //   strcpy (fzk_key, "name");
  //   TAG_WRITE(fzk_key, fzk_temp)
  //   // printf ("node sport=* -> sport=sport_allgemein + name=%s\n", fzk_temp);
  // }

  // Wasserfall
  // ----------
  else if ( TAG_EXISTS(waterway, "waterfall") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__WATERFALL__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node waterway=waterfall -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(waterway, "waterfall") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__WATERFALL__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node waterway=waterfall -> %s\n", fzk_value);
  }

  // Wehr (abgeleitete POIs ignorieren)
  // ----
  else if ( TAG_EXISTS(waterway, "weir") && KEY_EXISTS(name) && KEY_NOT_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      // if (strcmp (*keyp, "name") == 0) {
      //   snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__WEIR__);
      //   TAG_WRITE(*keyp, fzk_value)
      //   // printf ("node waterway=weir -> %s\n", fzk_value);
      // }
      // else {
      //   TAG_WRITE(*keyp, *valp)
      // }
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(waterway, "weir") && KEY_NOT_EXISTS(name) && KEY_NOT_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__WEIR__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node waterway=weir -> %s\n", fzk_value);
  }

  // Local authorities, administrative and supervising institutions (that are not related to state government)
  // --------------------------------------------------------------
  else if ( TAG_EXISTS(office, "administrative") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__ADMINISTRATIVE__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node office=administrative -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(office, "administrative") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__ADMINISTRATIVE__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node office=administrative -> %s\n", fzk_value);
  }

  // Job-Center / Arbeitsvermittlung)
  // -------------------------------
  else if ( TAG_EXISTS(office, "employment_agency") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__EMPLOYMENTAGENCY__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node office=employment_agency -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(office, "employment_agency") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__EMPLOYMENTAGENCY__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node office=employment_agency -> %s\n", fzk_value);
  }

  // Buero einer Regierung / Behoerde / Regierungseinrichtung)
  // --------------------------------------------------------
  else if ( TAG_EXISTS(office, "government") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__GOVERNMENT__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node office=government -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(office, "government") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__GOVERNMENT__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node office=government -> %s\n", fzk_value);
  }

  // Buero einer Nichtregierungsorganisation
  // ---------------------------------------
  else if ( TAG_EXISTS(office, "ngo") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__NGO__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node office=ngo -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(office, "ngo") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__NGO__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node office=ngo -> %s\n", fzk_value);
  }

  // Buero oder Sitz einer halbstaatlichen Organisation
  // --------------------------------------------------
  else if ( TAG_EXISTS(office, "quango") && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "name") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, $__QUANGO__);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("node office=quango -> %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(office, "quango") && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    strcpy (fzk_key, "name");
    snprintf (fzk_value, sizeof(fzk_value), "%s", $__QUANGO__);
    TAG_WRITE(fzk_key, fzk_value)
    // printf ("node office=quango -> %s\n", fzk_value);
  }

  // Benannte Grabstellen bedeutender Persoenlichkeiten (d.h. Personen mit Wikipedia-Eintrag)
  // --------------------------------------------------
  else if ( TAG_EXISTS(cemetery, "grave") && (KEY_EXISTS(wikipedia_de) || KEY_EXISTS(wikipedia) || KEY_EXISTS(website)) && KEY_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "cemetery") == 0) {
        TAG_WRITE(*keyp, "grabstaette_bedeutende_person")
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Node der nur Adressdaten ([Strasse +] Hausnummer) enthaelt (abgeleitete POIs nicht beruecksichtigen)
  // ----------------------------------------------------------
  else if ( KEY_EXISTS(addr_housenumber) && KEY_NOT_EXISTS(bBoxWeight) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    TAG_WRITE("fzk_adresse", "yes")
    // printf ("node addr:street=* && addr:housenumber=* -> fzk_adresse=yes\n");
  }

  // (Feuer-) Hydrant
  // ----------------
  else if ( TAG_EXISTS(emergency, "fire_hydrant") && KEY_NOT_EXISTS(name) ) {
    strcpy (fzk_temp, "??");
    FOR_ALL_TAGS
      if (strcmp (*keyp, "fire_hydrant:diameter") == 0) {
        // Durchmesser merken
        strcpy (fzk_temp, *valp);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
    // name Tag erzeugen
    strcpy (fzk_key, "name");
    TAG_WRITE(fzk_key, fzk_temp)
    // printf ("node emergency=fire_hydrant (without name) -> name=%s\n", fzk_temp);
  }

  //------------------------------------------------------------
  // hier weitere Regeln gemaess folgendem Schema einfuegen:
  // else if ( ... ) {
  //   ...
  // }
  //------------------------------------------------------------
  else {
    if ( KEY_EXISTS(bBoxWeight) ) {
      // alle abgeleiteten und bislang noch nicht betrachteten Objekte verwerfen
    }
    else {
      // Objekt unveraendert uebernehmen
      FOR_ALL_TAGS
        TAG_WRITE(*keyp, *valp)
      END_FOR_ALL_TAGS
    }
  }
}
