//------------------------------------------------------------
// Freizeitkarte-PlugIn fuer das Programm "osmfilter" von Markus Weber.
//
// Version : 1.0.0 - 02.02.2014
// Version : 1.1.0 - 25.04.2014: Beruecksichtigung von Wadis
// Version : 1.2.0 - 14.07.2014: Beruecksichtigung von Ruinen; Umbenennung privater Swimming-Pools
// Version : 1.3.0 - 03.10.2014: Beruecksichtigung von Garagen; weitere Differenzierung von Parkplaetzen; Logik fuer Radfusswege ergaenzt
// Version : 1.4.0 - 01.01.2015: Beruecksichtigung von Servicegleisen; Differenzierung der Bergwege in die Kategorien 1 bis 6
//                               Differenzierung von Feuchtgebieten (natural = schilf, feuchtwiese, sumpfgebiet, moorgebiet, watt, saline)
//                               Beruecksichtigung von landuse=salt_pond (Saline)
// Version : 1.5.0 - 06.02.2015: Beruecksichtigung von landuse=reservoir
// Version : 1.6.0 - 21.03.2015: Beruecksichtigung von natural=shingle
//                               Beruecksichtigung von amenity=parking_space
//                               Unterstuetzung fuer railway=abandoned auskommentiert
// Version : 1.7.0 - 25.04.2015: Unterstuetzung der Tags leaf_type=broadleaved / needleleaved / mixed
// Version : 1.8.0 - 10.01.2016: Tagtransform implementiert
// Version : 1.8.1 - 24.01.2016: Logikfehler bei "highway=footway && bicycle=yes|designated|official" korrigiert
// Version : 1.8.2 - 30.01.2016: Flaechengroessen fuer landuse=commercial und leisure=park beruecksichtigt
// Version : 1.8.3 - 31.01.2016: Beruecksichtigung von "schutzgebiet=strict_nature_reserve"
// Version : 1.8.4 - 06.02.2016: Vorverarbeitung von "aeroway=terminal|tower"
// Version : 1.8.5 - 12.02.2016: Vorverarbeitung von "indoor=yes"
// Version : 1.8.6 - 25.02.2016: Sonderbehandlung von building=train_station und building=roof
// Version : 1.8.7 - 05.03.2016: Sonderbehandlung von layer<0 geaendert, Logik fuer indoor-Wege geaendert
// Version : 1.9.0 - 31.03.2016: Defekt bei der Behandlung von railway korrigiert
//                               Tags für high=service_small, natural=shoal, natural=reef eingefuehrt
//                               highway=*_link werden jetzt gesondert behandelt; building=train_station wie building=station
//                               Sonderbehandlung fuer military=* entfernt; type=boundary wird zu type=multipolygon
// Version : 1.9.1 - 26.04.2016: Sonderbehandlung fuer surface=* eingefuehrt
// Version : 1.10.0 - 19.06.2016: Beruecksichtigung von railway=construction
// Version : 1.11.0 - 13.10.2016: Verarbeitung von via_ferrata ueberarbeitet
// Version : 1.12.0 - 27.12.2016: mtb_scale und mtb_scale_uphill Tagtransform
// Version : 17.09 - 31.08.2017: trail_visibility Tagtransform
// Version : 17.12 - 09.11.2017: bus_guideway (tunnel + bridge) und allotments Tagtransform
//                               Größenbewertung für tourism=zoo/theme_park/museum
//                               light_rain- + subway-Servicegleise berücksichtigt
// Version : 18.03 - 07.03.2018: Vorverarbeitung von cycleway lanes
// Version : 19.03 - 10.03.2019: Beruecksichtigung von tunnel:name, Korrektur cycleway lanes
// Version : 19.09 - 19.08.2019: Sonderbehandlung für Nationalparkgrenzen (type=boundary && boundary=national_park)
// Version : 19.12 - 11.11.2019: Beruecksichtigung von railway=miniature
// Version : 20.09 - 08.08.2020: natural=* (z.B. natural=water): mögliche layer-Angabe = "0"
//                               Sonderbehandlung fuer ford=*
// Version : 24.09 - 07.07.2024: Beruecksichtigung von natural=bare_rock
//                               Sonderbehandlung admin_level nur für administrative Grenzen verwenden
//
// Autor   : Klaus Tockloth
//
// Copyright (C) 2013-2024 FZK-Projekt <freizeitkarte@googlemail.com>
//
// Anmerkungen:
// - Neue TAGs sind auch in der Datei "fzk_globals.c" zu definieren.
// - Auf den Key des aktuellen TAGs kann via "*keyp" zugegriffen werden.
// - Auf den Value des aktuellen TAGs kann via "*valp" zugegriffen werden.
// - Auf den Key eines speziellen TAGs kann via "keyp_<tagname>" zugegriffen werden.
// - Auf den Value eines speziellen TAGs kann via "valp_<tagname>" zugegriffen werden.
// - Das Objekt "fzk_key" kann zum Erstellen eines neuen Keys benutzt werden.
// - Das Objekt "fzk_value" kann zum Erstellen eines neuen Values benutzt werden.
//
// Um ein spezielles TAG einzufuehren und zu verarbeiten sind folgende Schritte erforderlich:
// - TAG definieren (in "fzk_globals.c")
// - TAG initialisieren (TAG_INIT)
// - TAG speichern (TAG_STORE)
// - TAG logisch verarbeiten
//
// Der Wert "bBoxWeight" gibt die grobe Groessenordnung eines Polygons an.
// Die Wert der umgebenden Bounding-Box wird angegeben: log2(Quadartmeter)
// n (2 hoch n)
// 1 2
// 2 4
// 3 8
// 4 16
// 5 32
// 6 64
// 7 128
// 8 256
// 9 512
// 10 1.024 (Kibi)
// 11 2.048
// 12 4.096
// 13 8.192
// 14 16.384
// 15 32.768
// 16 65.536
// 17 131.072
// 18 262.144
// 19 524.288
// 20 1.048.576 (Mebi)
// 21 2.097.152
// 22 4.194.304
// 23 8.388.608
// 24 16.777.216
// 25 33.554.432
// 26 67.108.864
// 27 134.217.728
// 28 268.435.456
// 29 536.870.912
// 30 1.073.741.824 (Gibi)
// 31 2.147.483.648
// 32 4.294.967.296
// 33 8.589.934.592
// 34 17.179.869.184
// 35 34.359.738.368
// 36 68.719.476.736
// 37 137.438.953.472
// 38 274.877.906.944
// 39 549.755.813.888
// 40 1.099.511.627.776 (Tebi)
//------------------------------------------------------------

//------------------------------------------------------------
// Initialisierung der TAGs und Merker
//------------------------------------------------------------

// alle speziellen TAGs (key/value) initialisieren die nachfolgend verwendet werden
TAG_INIT(abandoned)
TAG_INIT(access)
TAG_INIT(admin_level)
TAG_INIT(aeroway)
TAG_INIT(allotments)
TAG_INIT(amenity)
TAG_INIT(attraction)
TAG_INIT(bBoxWeight)
TAG_INIT(bicycle)
TAG_INIT(boundary)
TAG_INIT(bridge)
TAG_INIT(building)
TAG_INIT(contour)
TAG_INIT(cycleway)
TAG_INIT(cycleway_left)
TAG_INIT(cycleway_right)
TAG_INIT(ele)
TAG_INIT(foot)
TAG_INIT(ford)
TAG_INIT(highway)
TAG_INIT(horse)
TAG_INIT(indoor)
TAG_INIT(intermittent)
TAG_INIT(landuse)
TAG_INIT(layer)
TAG_INIT(leisure)
TAG_INIT(man_made)
TAG_INIT(motorcar)
TAG_INIT(motor_vehicle)
TAG_INIT(mtb_scale)
TAG_INIT(mtb_scale_uphill)
TAG_INIT(name)
TAG_INIT(natural)
TAG_INIT(protect_class)
TAG_INIT(railway)
TAG_INIT(religion)
TAG_INIT(ruins)
TAG_INIT(sac_scale)
TAG_INIT(service)
TAG_INIT(shop)
TAG_INIT(sport)
TAG_INIT(surface)
TAG_INIT(tracktype)
TAG_INIT(tourism)
TAG_INIT(trail_visibility)
TAG_INIT(tunnel)
TAG_INIT(tunnel_name)
TAG_INIT(type)
TAG_INIT(vehicle)
TAG_INIT(via_ferrata_scale)
TAG_INIT(waterway)
TAG_INIT(wetland)
TAG_INIT(zoo)

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
  else if TAG_STORE(admin_level, "admin_level")
  else if TAG_STORE(aeroway, "aeroway")
  else if TAG_STORE(allotments, "allotments")
  else if TAG_STORE(amenity, "amenity")
  else if TAG_STORE(attraction, "attraction")
  else if TAG_STORE(bBoxWeight, "bBoxWeight")
  else if TAG_STORE(bicycle, "bicycle")
  else if TAG_STORE(boundary, "boundary")
  else if TAG_STORE(bridge, "bridge")
  else if TAG_STORE(building, "building")
  else if TAG_STORE(contour, "contour")
  else if TAG_STORE(cycleway, "cycleway")
  else if TAG_STORE(cycleway_left, "cycleway:left")
  else if TAG_STORE(cycleway_right, "cycleway:right")
  else if TAG_STORE(ele, "ele")
  else if TAG_STORE(foot, "foot")
  else if TAG_STORE(ford, "ford")
  else if TAG_STORE(highway, "highway")
  else if TAG_STORE(horse, "horse")
  else if TAG_STORE(indoor, "indoor")
  else if TAG_STORE(intermittent, "intermittent")
  else if TAG_STORE(landuse, "landuse")
  else if TAG_STORE(layer, "layer")
  else if TAG_STORE(leisure, "leisure")
  else if TAG_STORE(man_made, "man_made")
  else if TAG_STORE(motorcar, "motorcar")
  else if TAG_STORE(motor_vehicle, "motor_vehicle")
  else if TAG_STORE(mtb_scale, "mtb:scale")
  else if TAG_STORE(mtb_scale_uphill, "mtb:scale:uphill")
  else if TAG_STORE(name, "name")
  else if TAG_STORE(natural, "natural")
  else if TAG_STORE(protect_class, "protect_class")
  else if TAG_STORE(railway, "railway")
  else if TAG_STORE(religion, "religion")
  else if TAG_STORE(ruins, "ruins")
  else if TAG_STORE(sac_scale, "sac_scale")
  else if TAG_STORE(service, "service")
  else if TAG_STORE(shop, "shop")
  else if TAG_STORE(surface, "surface")
  else if TAG_STORE(tourism, "tourism")
  else if TAG_STORE(tracktype, "tracktype")
  else if TAG_STORE(trail_visibility, "trail_visibility")
  else if TAG_STORE(tunnel, "tunnel")
  else if TAG_STORE(tunnel_name, "tunnel:name")
  else if TAG_STORE(type, "type")
  else if TAG_STORE(vehicle, "vehicle")
  else if TAG_STORE(via_ferrata_scale, "via_ferrata_scale")
  else if TAG_STORE(waterway, "waterway")
  else if TAG_STORE(wetland, "wetland")
  else if TAG_STORE(zoo, "zoo")
END_FOR_ALL_TAGS

//------------------------------------------------------------
// TAGs (gegebenenfalls) modifizieren und schreiben
//------------------------------------------------------------

if (fzk_found == 0) {
  // keine spezielle Bearbeitung erforderlich (da kein Spezial-TAG gefunden wurde)
  // printf ("Keine spezielle Bearbeitung ...\n");
  FOR_ALL_TAGS
    TAG_WRITE(*keyp, *valp)
    // printf ("way/rel %s = %s\n", *keyp, *valp);
  END_FOR_ALL_TAGS
}
else {
  // spezielle Bearbeitung pruefen und durchfuehren (da mindestens ein Spezial-TAG gefunden wurde)

  // Objekte umtaggen (tag transform)
  // Vorbemerkungen:
  // TAG_DEFINE(NN) legt zwei Zeiger an (NN = Platzhalter):
  // - *keyp_NN
  // - *valp_NN
  // Der Zeiger *keyp_NN fließt ein in die Abfragen:
  // - KEY_EXISTS()
  // - KEY_NOT_EXISTS()
  // Die Zeigen *keyp_NN und *valp_NN fließen ein in die Abfrage:
  // - TAG_EXISTS()

  // Sonderbehandlung fuer alle Relationen type=boundary:
  // - mapwriter loest nur Objekte type=multipolygon auf
  // - type=boundary in type=multipolygon umwandeln
  // administrative Grenzen
  if ( KEY_EXISTS(admin_level) && TAG_EXISTS(type, "boundary") && TAG_EXISTS(boundary, "administrative") ) {
    // printf ("\n");
    FOR_ALL_TAGS
      // printf ("%s = %s\n", *keyp, *valp);
      if (strcmp (*keyp, "type") == 0) {
        valp_type = "multipolygon";
        *valp = "multipolygon";
        // printf ("--> type = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }
  // Nationalparkgrenzen
  if ( TAG_EXISTS(type, "boundary") && TAG_EXISTS(boundary, "national_park") ) {
    // printf ("\n");
    FOR_ALL_TAGS
      // printf ("%s = %s\n", *keyp, *valp);
      if (strcmp (*keyp, "type") == 0) {
        valp_type = "multipolygon";
        *valp = "multipolygon";
        // printf ("--> type = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }
  // Sondernaturschutzgebiete (z.B. Naturwaldreservate; eher kleinere Gebiete)
  if ( TAG_EXISTS(type, "boundary") && TAG_EXISTS(boundary, "protected_area") && (TAG_EXISTS(protect_class, "1") || TAG_EXISTS(protect_class,"1a") || TAG_EXISTS(protect_class, "1b")) ) {
    // printf ("\n");
    FOR_ALL_TAGS
      // printf ("%s = %s\n", *keyp, *valp);
      if (strcmp (*keyp, "type") == 0) {
        valp_type = "multipolygon";
        *valp = "multipolygon";
        // printf ("--> type = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }
  
  // admin_level nur für administrative Grenzen  
  if ( KEY_EXISTS(admin_level) ) {
    if ( TAG_EXISTS(boundary, "administrative") ) {
       // nop
    }
    else {
      FOR_ALL_TAGS
        if (strcmp (*keyp, "admin_level") == 0) {
         // Tag auf 0 setzen (wird nicht verwendet)
         *valp = "0";
         // printf ("way admin_level=0\n");
          break;
        }
      END_FOR_ALL_TAGS
    }
  }

  // Sonderbehandlung fuer surface wegen Common-Value-Bug: natural=beach + surface=sand kollidiert mit natural=sand
  if ( KEY_EXISTS(surface) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "surface") == 0) {
        snprintf (fzk_surface, sizeof(fzk_highway), "surface_%s", *valp );
        valp_surface = fzk_surface;
        *valp = fzk_surface;
        // printf ("surface = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }

  // Sonderbehandlung fuer alle indoor-Wege die keine Tunnel sind:
  // - Indoor-Fusswege und Treppen bleiben unveraendert
  // - alle anderen indoor-Objekte (highway) werden umbenannt
  // - nachgelagert kann die Uebernahme der umbenannten Objekte unterdrueckt werden
  if ( KEY_EXISTS(highway) && TAG_EXISTS(indoor, "yes") ) {
    if ( TAG_EXISTS(highway, "footway") || TAG_EXISTS(highway, "steps") ) {
       // nop
    }
    else {
      FOR_ALL_TAGS
        if (strcmp (*keyp, "highway") == 0) {
          snprintf (fzk_highway, sizeof(fzk_highway), "indoor_%s", *valp );
          valp_highway = fzk_highway;
          *valp = fzk_highway;
          // printf ("highway = %s (nach tag transform)\n", *valp);
          break;
        }
      END_FOR_ALL_TAGS
    }
  }

  // Sonderbehandlung fuer Objekte die mit layer<0 getaggt sind:
  // - Nachgelagert koennen alle Objekte mit layer<0 aus dem Datenbestand entfernt werden.
  if ( KEY_EXISTS(waterway) && KEY_EXISTS(layer) ) {
    // layer-Tag auf den Wert 0 setzen (waterways sind oft inkorrekterweise mit layer=-1 erfasst)
    FOR_ALL_TAGS
      if (strcmp (*keyp, "layer") == 0) {
        if ( atol (*valp) < 0 ) {
          *valp = "0";
          // printf ("waterway = layer=%s (nach tag transform)\n", *valp);
        }
        break;
      }
    END_FOR_ALL_TAGS
  }
  else if ( KEY_EXISTS(natural) && KEY_EXISTS(layer) ) {
    // layer-Tag auf den Wert 0 setzen (water-Flächen sind oft inkorrekterweise mit layer=-1 erfasst)
    FOR_ALL_TAGS
      if (strcmp (*keyp, "layer") == 0) {
        if ( atol (*valp) < 0 ) {
          *valp = "0";
          // printf ("natural = layer=%s (nach tag transform)\n", *valp);
        }
        break;
      }
    END_FOR_ALL_TAGS
  }
  else if ( KEY_EXISTS(highway) && KEY_EXISTS(layer) ) {
    // layer-Tag auf den Wert 0 setzen (Treppen (U-Bahn) und (offene) Unterfuehrungen)
    FOR_ALL_TAGS
      if (strcmp (*keyp, "layer") == 0) {
        if ( atol (*valp) < 0 ) {
          *valp = "0";
          // printf ("highway = layer=%s (nach tag transform)\n", *valp);
        }
        break;
      }
    END_FOR_ALL_TAGS
  }
  else if ( KEY_EXISTS(railway) && KEY_EXISTS(layer) ) {
    // layer-Tag auf den Wert 0 setzen (Streckenabschnitte in Tunnel oder Unterfuehrungen)
    if (   TAG_EXISTS(railway, "abandoned")  || TAG_EXISTS(railway, "construction") || TAG_EXISTS(railway, "disused")      || TAG_EXISTS(railway, "funicular")
        || TAG_EXISTS(railway, "light_rail") || TAG_EXISTS(railway, "miniature")    || TAG_EXISTS(railway, "monorail")     || TAG_EXISTS(railway, "narrow_gauge") 
        || TAG_EXISTS(railway, "preserved")  || TAG_EXISTS(railway, "rail")         || TAG_EXISTS(railway, "subway")       || TAG_EXISTS(railway, "tram") ) {
      // nur auf Schienenobjekte anwenden
      FOR_ALL_TAGS
        if (strcmp (*keyp, "layer") == 0) {
          if ( atol (*valp) < 0 ) {
            *valp = "0";
            // printf ("railway = layer=%s (nach tag transform)\n", *valp);
          }
          break;
        }
      END_FOR_ALL_TAGS
    }
  }
  else if (KEY_EXISTS(tunnel) && KEY_EXISTS(layer) ) {
    // layer-Tag auf den Wert 0 setzen (verbliebene tunnel-Objekte, z.B. area=yes, power=cable, tunnel=yes/building_passage, ...)
    FOR_ALL_TAGS
      if (strcmp (*keyp, "layer") == 0) {
        if ( atol (*valp) < 0 ) {
          *valp = "0";
          // printf ("--> tunnel = layer=%s (nach tag transform)\n", *valp);
        }
        break;
      }
    END_FOR_ALL_TAGS
  }

  // Sonderbehandlung fuer Zugangsbeschraenkungen
  if ( KEY_EXISTS(access) ) {
    // Beispiel: access=no -> access=access_no
    FOR_ALL_TAGS
      if (strcmp (*keyp, "access") == 0) {
        snprintf (fzk_access, sizeof(fzk_access), "%s_%s", *keyp, *valp);
        valp_access = fzk_access;
        *valp = fzk_access;
        // printf ("access = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }
  if ( KEY_EXISTS(vehicle) ) {
    // Beispiel: vehicle=destination -> vehicle=vehicle_destination
    FOR_ALL_TAGS
      if (strcmp (*keyp, "vehicle") == 0) {
        snprintf (fzk_vehicle, sizeof(fzk_vehicle), "%s_%s", *keyp, *valp);
        valp_vehicle = fzk_vehicle;
        *valp = fzk_vehicle;
        // printf ("vehicle = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }
  if ( KEY_EXISTS(motor_vehicle) ) {
    // Beispiel: motor_vehicle=destination -> motor_vehicle=motor_vehicle_destination
    FOR_ALL_TAGS
      if (strcmp (*keyp, "motor_vehicle") == 0) {
        snprintf (fzk_motor_vehicle, sizeof(fzk_motor_vehicle), "%s_%s", *keyp, *valp);
        valp_motor_vehicle = fzk_motor_vehicle;
        *valp = fzk_motor_vehicle;
        // printf ("motor_vehicle = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }
  if ( KEY_EXISTS(motorcar) ) {
    // Beispiel: motorcar=destination -> motorcar=motorcar_destination
    FOR_ALL_TAGS
      if (strcmp (*keyp, "motorcar") == 0) {
        snprintf (fzk_motorcar, sizeof(fzk_motorcar), "%s_%s", *keyp, *valp);
        valp_motorcar = fzk_motorcar;
        *valp = fzk_motorcar;
        // printf ("motorcar = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }
  if ( KEY_EXISTS(foot) ) {
    // Beispiel: foot=yes -> foot=foot_yes
    FOR_ALL_TAGS
      if (strcmp (*keyp, "foot") == 0) {
        snprintf (fzk_foot, sizeof(fzk_foot), "%s_%s", *keyp, *valp);
        valp_foot = fzk_foot;
        *valp = fzk_foot;
        // printf ("foot = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }
  if ( KEY_EXISTS(bicycle) ) {
    // Beispiel: bicycle=yes -> bicycle=bicycle_yes
    FOR_ALL_TAGS
      if (strcmp (*keyp, "bicycle") == 0) {
        snprintf (fzk_bicycle, sizeof(fzk_bicycle), "%s_%s", *keyp, *valp);
        valp_bicycle = fzk_bicycle;
        *valp = fzk_bicycle;
        // printf ("bicycle = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }

  // Sonderbehandlung für MTB Scale und MTB Scale Uphill
  if ( KEY_EXISTS(mtb_scale) ) {
    // Beispiel: mtb:scale=0 -> mtb:scale=mtbs_0
    FOR_ALL_TAGS
      if (strcmp (*keyp, "mtb:scale") == 0) {
        snprintf (fzk_mtb_scale, sizeof(fzk_mtb_scale), "%s_%s", "mtbs", *valp);
        valp_mtb_scale = fzk_mtb_scale;
        *valp = fzk_mtb_scale;
        // printf ("mtb:scale = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }
  if ( KEY_EXISTS(mtb_scale_uphill) ) {
    // Beispiel: mtb:scale:uphill=0 -> mtb:scale:uphill=mtbu_0
    FOR_ALL_TAGS
      if (strcmp (*keyp, "mtb:scale:uphill") == 0) {
        snprintf (fzk_mtb_scale_uphill, sizeof(fzk_mtb_scale_uphill), "%s_%s", "mtbu", *valp);
        valp_mtb_scale_uphill = fzk_mtb_scale_uphill;
        *valp = fzk_mtb_scale_uphill;
        // printf ("mtb:scale:uphill = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }

  // Sonderbehandlung für trail_visibility
  if ( KEY_EXISTS(trail_visibility) ) {
    // Beispiel: trail_visibility=horrible -> trail_visibility=trail_visibility_horrible
    FOR_ALL_TAGS
      if (strcmp (*keyp, "trail_visibility") == 0) {
        snprintf (fzk_trail_visibility, sizeof(fzk_trail_visibility), "%s_%s", "trail_visibility", *valp);
        valp_trail_visibility = fzk_trail_visibility;
        *valp = fzk_trail_visibility;
        // printf ("trail_visibility = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }

  // Sonderbehandlung für ford
  if ( KEY_EXISTS(ford) ) {
    // Beispiel: ford=yes -> ford=ford_yes
    FOR_ALL_TAGS
      if (strcmp (*keyp, "ford") == 0) {
        snprintf (fzk_ford, sizeof(fzk_ford), "%s_%s", "ford", *valp);
        valp_ford = fzk_ford;
        *valp = fzk_ford;
        // printf ("ford = %s (nach tag transform)\n", *valp);
        break;
      }
    END_FOR_ALL_TAGS
  }

  // Sonderbehandlung für Radstreifen
  // cycleway=lane|track
  // cycleway:left=lane|track, cycleway:left=opposite_lane, 
  // cycleway:right=lane|track, cycleway:right=opposite_lane
  if ( TAG_EXISTS(cycleway, "lane") || TAG_EXISTS(cycleway, "track") ) {
    // cycleway_lane-Tag (key/value) erzeugen
    TAG_WRITE("cycleway_lane", "cycleway_lane_both")
    // printf ("way/rel cycleway=lane|track -> cycleway_lane=cycleway_lane_both\n");
  }
  else if ( ( TAG_EXISTS(cycleway_left, "lane") || TAG_EXISTS(cycleway_left, "track") || TAG_EXISTS(cycleway_left, "opposite_lane") ) && ( TAG_EXISTS(cycleway_right, "lane") || TAG_EXISTS(cycleway_right, "track") || TAG_EXISTS(cycleway_right, "opposite_lane") ) ) {
    // cycleway_lane-Tag (key/value) erzeugen
    TAG_WRITE("cycleway_lane", "cycleway_lane_both")
    // printf ("way/rel cycleway:left=* and cycleway:right -> cycleway_lane=cycleway_lane_both\n");
  }
  else if ( TAG_EXISTS(cycleway_left, "lane") || TAG_EXISTS(cycleway_left, "track") || TAG_EXISTS(cycleway_left, "opposite_lane") ) {
    // cycleway_lane-Tag (key/value) erzeugen
    TAG_WRITE("cycleway_lane", "cycleway_lane_left")
    // printf ("way/rel cycleway:left=lane|track|opposite_lane -> cycleway_lane=cycleway_lane_left\n");
  }
  else if ( TAG_EXISTS(cycleway_right, "lane") || TAG_EXISTS(cycleway_right, "track") || TAG_EXISTS(cycleway_right, "opposite_lane") ) {
    // cycleway_lane-Tag (key/value) erzeugen
    TAG_WRITE("cycleway_lane", "cycleway_lane_right")
    // printf ("way/rel cycleway:right=lane|track|opposite_lane -> cycleway_lane=cycleway_lane_right\n");
  }
  
  // Sonderbehandlung für Tunnelnamen
  if ( KEY_EXISTS(tunnel_name) && KEY_NOT_EXISTS(name) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "tunnel:name") == 0) {
        strcpy (fzk_key, "name");
        TAG_WRITE(fzk_key, *valp)
		// printf ("Tunnelname: %s = %s\n", *keyp, *valp);
		break;
      }
    END_FOR_ALL_TAGS
  }
  
  if ( TAG_EXISTS(contour, "elevation") && KEY_EXISTS(ele) ) {
    // Datenbeispiel: <tag k="ele" v="550"/>  <tag k="contour" v="elevation"/>  <tag k="contour_ext" v="elevation_minor"/>
    // printf ("way/rel contour=elevation: ele = %s\n", valp_ele);
    if ( atol (valp_ele) > 0 ) {
      // Tags schreiben; name-Value mit Hoehenangabe erzeugen
      FOR_ALL_TAGS
        TAG_WRITE(*keyp, *valp)
      END_FOR_ALL_TAGS
      // name-Tag (key/value) erzeugen (Beispiel: "275")
      TAG_WRITE("name", valp_ele)
    }
    else {
      // Hoehenlinien "<= 0" verwerfen
    }
  }
  // else if ( TAG_EXISTS(leisure, "garden") && TAG_EXISTS(access, "access_private") ) {
  //   FOR_ALL_TAGS
  //     if (strcmp (*keyp, "leisure") == 0) {
  //       // Tag umdefinieren
  //       TAG_WRITE(*keyp, "garden_private")
  //       printf ("way/rel leisure=garden && access=private = garden_private\n");
  //     }
  //     else {
  //       TAG_WRITE(*keyp, *valp)
  //     }
  //   END_FOR_ALL_TAGS
  // }

  // Achterbahn (Feature nicht überzeugend; bei Reaktivierung Brücke und Tunnel berücksichtigen)
  // else if ( KEY_EXISTS(railway) && TAG_EXISTS(attraction, "roller_coaster") ) {
  //   FOR_ALL_TAGS
  //     // printf ("way/rel railway=* && attraction=roller_coaster = %s=%s\n", *keyp, *valp);
  //     if (strcmp (*keyp, "railway") == 0) {
  //       // Tag umdefinieren
  //       TAG_WRITE(*keyp, "fzk_roller_coaster")
  //       // printf ("way/rel railway=* && attraction=roller_coaster = railway=fzk_roller_coaster\n");
  //     }
  //     else {
  //       TAG_WRITE(*keyp, *valp)
  //     }
  //   END_FOR_ALL_TAGS
  // }

  // (Flughafen-) Terminal
  else if ( TAG_EXISTS(aeroway, "terminal") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        // Tag entfernen
        // printf ("way/rel aeroway=terminal -> building removed\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Gebaeude die als Ruinen getaggt sind
  else if ( KEY_EXISTS(building) && TAG_EXISTS(ruins, "yes") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "ruine")
        // printf ("way/rel building=* + ruins=yes -> building=ruine\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Bei Gebaeuden wird wie folgt verallgemeinert:
  // - building = anbetungsort (Kirche, Kapelle, Mosche, Synagoge, ...)
  // - building = sportanlage (Turnhalle, Sportzentrum, Schwimmbad/-halle, Gebaeude mit sportartspezifischer Nutzung, ...)
  // - building = bahnhof (Bahnhof)
  // - building = dach (Daecher, Ueberdachungen))
  // - building = bildungseinrichtung (Kindergarten, Schule, Universitaet, ...)
  // - building = krankenhaus (Krankenhaus, Hospital, ...)
  // - building = gewaechshaus (Gewaechshaus, Treibhaus, ...)
  // - building = gebaeude_oeffentlich (Gericht, Rathaus, Polizei, Feuerwache, ...)
  // - building = gebaeude_allgemein (alle anderen Gebaeude)
  //
  // Generalisierung aller Anbetungsorte (Kirchen, Kapellen, ... ):
  // building=cathedral : Gebaeude, das als Kathedrale erbaut wurde. Wird in Verbindung mit amenity=place_of_worship, religion=* und denomination=* benutzt.
  // building=chapel    : A building that was built as a chapel. Used in conjunction with amenity=place_of_worship, religion=* and denomination=* for the chapel grounds where it is in current use.
  // building=church    : Ein Gebaeude, das als Kirche erbaut wurde. Dies sollte mit amenity=place_of_worship, religion=* and denomination=* vervollstaendigt werden.
  // Alle Anbetungsorte werden nachfolgend ueber das Tag "religion" generalisiert.
  // else if ( KEY_EXISTS(building) && KEY_EXISTS(religion) ) {
  else if ( KEY_EXISTS(building) && TAG_EXISTS(amenity, "place_of_worship") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "anbetungsort")
        // printf ("way/rel building=* + religion=* -> building=anbetungsort\n");
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s)", *valp, "anbetungsort");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* + religion=* + amenity=* -> amenity=%s\n", fzk_value);
      }
      else if (strcmp (*keyp, "name") == 0) {
        // name entfernen (wird als abgeleiteter POI ausgegeben)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(building, "cathedral") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "anbetungsort")
        // printf ("way/rel building=cathedral -> building=anbetungsort\n");
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s)", *valp, "anbetungsort");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=cathedral + amenity=* -> amenity=%s\n", fzk_value);
      }
      else if (strcmp (*keyp, "name") == 0) {
        // name entfernen (wird als abgeleiteter POI ausgegeben)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(building, "chapel") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "anbetungsort")
        // printf ("way/rel building=chapel -> building=anbetungsort\n");
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s)", *valp, "anbetungsort");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=chapel + amenity=* -> amenity=%s\n", fzk_value);
      }
      else if (strcmp (*keyp, "name") == 0) {
        // name entfernen (wird als abgeleiteter POI ausgegeben)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(building, "church") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "anbetungsort")
        // printf ("way/rel building=church -> building=anbetungsort\n");
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s)", *valp, "anbetungsort");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=church + amenity=* -> amenity=%s\n", fzk_value);
      }
      else if (strcmp (*keyp, "name") == 0) {
        // name entfernen (wird als abgeleiteter POI ausgegeben)
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Gebaeude im Zusammenhang mit sportlichen Aktivitaeten:
  // - als Turnhalle / Sportzentrum / Sportanlage
  // - als Schwimmbad / -halle
  // - mit spezifischer Sportnutzung
  else if ( KEY_EXISTS(building) && TAG_EXISTS(leisure, "sports_centre") && KEY_EXISTS(sport) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "sportanlage")
      }
      else if (strcmp (*keyp, "leisure") == 0) {
        TAG_WRITE(*keyp, "sports_centre_gebaeude")
        // printf ("way/rel building=* && leisure=sports_centre = building=sportanlage && leisure=sports_centre_gebaeude\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( KEY_EXISTS(building) && TAG_EXISTS(leisure, "sports_centre") && KEY_NOT_EXISTS(sport) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "sportanlage")
      }
      else if (strcmp (*keyp, "leisure") == 0) {
        TAG_WRITE(*keyp, "sports_centre_gebaeude")
        // printf ("way/rel building=* && leisure=sports_centre = building=sportanlage && leisure=sports_centre_gebaeude\n");
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
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "sportanlage")
      }
      else if (strcmp (*keyp, "leisure") == 0) {
        TAG_WRITE(*keyp, "water_park_gebaeude")
        // printf ("way/rel building=* && leisure=water_park = building=sportanlage && leisure=water_park_gebaeude\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( KEY_EXISTS(building) && TAG_EXISTS(leisure, "water_park") && KEY_NOT_EXISTS(sport) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "sportanlage")
      }
      else if (strcmp (*keyp, "leisure") == 0) {
        TAG_WRITE(*keyp, "water_park_gebaeude")
        // printf ("way/rel building=* && leisure=water_park = building=sportanlage && leisure=water_park_gebaeude\n");
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
  else if ( KEY_EXISTS(building) && KEY_EXISTS(sport) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "sportanlage")
        // printf ("way/rel building=* && sport=* = building=sportanlage\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(building, "sport") || TAG_EXISTS(building, "sports_hall")) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "sportanlage")
        // printf ("way/rel building=sport || building=sports_hall = building=sportanlage\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Bahnhoefe (oftmals komplex, koennen z.B. transparent dargestellt werden)
  else if ( TAG_EXISTS(building, "train_station") ||TAG_EXISTS(building, "station") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "bahnhof")
        // printf ("way/rel building=train_station|station= building=bahnhof\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Daecher (koennen z.B. transparent dargestellt werden)
  else if ( TAG_EXISTS(building, "roof") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "dach")
        // printf ("way/rel building=roof = building=dach\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Generalisierung aller Bildungseinrichtungen:
  // building = school       : Schulgebaeude; vervollstaendigt mit dem Tag amenity=school.
  // building = university   : Ein Universitaetsgebaeude; vervollstaendigt mit dem Tag amenity=university.
  // building = kindergarten : Ein Kindergartengebaeude; vervollstaendigt mit dem Tag amenity=kindergarten.
  // building=yes + amenity=kindergarten
  // building=yes + amenity=school
  // building=yes + amenity=college
  // building=yes + amenity=university
  // building=yes + amenity=library
  else if ( KEY_EXISTS(building) && TAG_EXISTS(amenity, "kindergarten") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "bildungseinrichtung")
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "kindergarten_gebaeude")
        // printf ("way/rel building=* && amenity=kindergarten = building=bildungseinrichtung && amenity=kindergarten_gebaeude\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( KEY_EXISTS(building) && TAG_EXISTS(amenity, "school") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "bildungseinrichtung")
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "school_gebaeude")
        // printf ("way/rel building=* && amenity=school = building=bildungseinrichtung && amenity=school_gebaeude\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( KEY_EXISTS(building) && TAG_EXISTS(amenity, "college") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "bildungseinrichtung")
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "college_gebaeude")
        // printf ("way/rel building=* && amenity=college = building=bildungseinrichtung && amenity=college_gebaeude\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( KEY_EXISTS(building) && TAG_EXISTS(amenity, "university") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "bildungseinrichtung")
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "university_gebaeude")
        // printf ("way/rel building=* && amenity=university = building=bildungseinrichtung && amenity=university_gebaeude\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( KEY_EXISTS(building) && TAG_EXISTS(amenity, "library") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "bildungseinrichtung")
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "library_gebaeude")
        // printf ("way/rel building=* && amenity=library = building=bildungseinrichtung && amenity=library_gebaeude\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(building, "kindergarten") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "bildungseinrichtung")
        // printf ("way/rel building=kindergarten = building=bildungseinrichtung\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(building, "school") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "bildungseinrichtung")
        // printf ("way/rel building=school = building=bildungseinrichtung\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(building, "university") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "bildungseinrichtung")
        // printf ("way/rel building=university = building=bildungseinrichtung\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Gebaeude als Krankenhaus
  else if ( KEY_EXISTS(building) && TAG_EXISTS(amenity, "hospital") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "krankenhaus")
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        TAG_WRITE(*keyp, "hospital_gebaeude")
        // printf ("way/rel building=* && amenity=hospital = building=krankenhaus && amenity=hospital_gebaeude\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(building, "hospital") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "krankenhaus")
        // printf ("way/rel building=hospital = building=krankenhaus\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  } 

  // Gebaeude als Gewaechs- / Treibhaus
  else if ( KEY_EXISTS(building) && TAG_EXISTS(landuse, "greenhouse_horticulture") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "gewaechshaus")
      }
      else if (strcmp (*keyp, "landuse") == 0) {
        TAG_WRITE(*keyp, "greenhouse_horticulture_gebaeude")
        // printf ("way/rel building=* && landuse=greenhouse_horticulture = building=gewaechshaus && landuse=greenhouse_horticulture_gebaeude\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(building, "greenhouse") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "gewaechshaus")
        // printf ("way/rel building=greenhouse = building=gewaechshaus\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Generalisierung der oeffentlichen Gebaeude (ohne Bildungseinrichtungen):

  // Gericht
  else if ( KEY_EXISTS(building) && TAG_EXISTS(amenity, "courthouse") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "gebaeude_oeffentlich")
        // printf ("way/rel building && amenity=courthouse = building=gebaeude_oeffentlich\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Rathaus
  else if ( KEY_EXISTS(building) && TAG_EXISTS(amenity, "townhall") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "gebaeude_oeffentlich")
        // printf ("way/rel building && amenity=townhall = building=gebaeude_oeffentlich\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Polizeistation
  else if ( KEY_EXISTS(building) && TAG_EXISTS(amenity, "police") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "gebaeude_oeffentlich")
        // printf ("way/rel building && amenity=police = building=gebaeude_oeffentlich\n");
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
      }
      else if (strcmp (*keyp, "name") == 0) {
        // "name" wegen Doppelausgabe verwerfen
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Feuerwache
  else if ( KEY_EXISTS(building) && TAG_EXISTS(amenity, "fire_station") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "gebaeude_oeffentlich")
        // printf ("way/rel building && amenity=fire_station = building=gebaeude_oeffentlich\n");
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
      }
      else if (strcmp (*keyp, "name") == 0) {
        // "name" wegen Doppelausgabe verwerfen
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Theater
  else if ( KEY_EXISTS(building) && TAG_EXISTS(amenity, "theatre") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "gebaeude_oeffentlich")
        // printf ("way/rel building && amenity=theatre = building=gebaeude_oeffentlich\n");
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
      }
      else if (strcmp (*keyp, "name") == 0) {
        // "name" wegen Doppelausgabe verwerfen
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // nicht naeher spezifiziertes oeffentliches Gebaeude (deprecated)
  else if ( KEY_EXISTS(building) && TAG_EXISTS(amenity, "public_building") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "gebaeude_oeffentlich")
        // printf ("way/rel building && amenity=public_building = building=gebaeude_oeffentlich\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(building, "garage")) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "gebaeude_garage")
        // printf ("way/rel building=garage = building=gebaeude_garage\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(building, "public") || TAG_EXISTS(building, "civic") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "gebaeude_oeffentlich")
        // printf ("way/rel building=public | civic = building=gebaeude_oeffentlich\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Generalisierung aller restlichen Gebaeude:
  // Bei einigen Objekten ist insbesondere auf eine geeignete Differenzierung zu achten - Beispiel:
  // - Grundflaeche einer Universitaet (Campus) getaggt als "amenity=university"
  // - Universitaetsgebaeude getaggt als "building=yes + amenity=university"
  // Aus allen Flaechenobjekten werden (generisch) (tag-)identische POIs erzeugt.
  // Um die doppelte Benennung der Objekte (1x POI + 1x Gebaeude) zu vermeiden, wird das name-Tag entfernt.
  // Dies gilt fuer alle Objekte fuer die ein zusaetzliches Tag "shop" oder "tourism" erfasst ist.
  // Sowie fuer Objekte fuer die zusaetzlich bestimmte key/value-Paare vom Typ "amenity" erfasst sind.
  else if ( KEY_EXISTS(building) &&
            (   KEY_EXISTS(shop) || KEY_EXISTS(tourism)
             || TAG_EXISTS(amenity, "bar")             || TAG_EXISTS(amenity, "biergarten")      || TAG_EXISTS(amenity, "cafe")
             || TAG_EXISTS(amenity, "fast_food")       || TAG_EXISTS(amenity, "ice_cream")       || TAG_EXISTS(amenity, "pub")
             || TAG_EXISTS(amenity, "restaurant")      || TAG_EXISTS(amenity, "social_centre")   || TAG_EXISTS(amenity, "toilets")
             || TAG_EXISTS(amenity, "fuel")            || TAG_EXISTS(amenity, "bank")            || TAG_EXISTS(amenity, "bicycle_rental")
             || TAG_EXISTS(amenity, "car_rental")      || TAG_EXISTS(amenity, "car_sharing")     || TAG_EXISTS(amenity, "car_wash")
             || TAG_EXISTS(amenity, "pharmacy")        || TAG_EXISTS(amenity, "arts_centre")     || TAG_EXISTS(amenity, "cinema")
             || TAG_EXISTS(amenity, "nightclub")
            )
          ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "gebaeude_allgemein")
        // printf ("way/rel building=* -> building=gebaeude_allgemein\n");
      }
      // Flaechentags modifizieren
      else if (strcmp (*keyp, "landuse") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* -> %s = %s\n", *keyp, fzk_value);
      }
      else if (strcmp (*keyp, "natural") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* -> %s = %s\n", *keyp, fzk_value);
      }
      else if (strcmp (*keyp, "power") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* -> %s = %s\n", *keyp, fzk_value);
      }
      else if (strcmp (*keyp, "leisure") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* -> %s = %s\n", *keyp, fzk_value);
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* -> %s = %s\n", *keyp, fzk_value);
      }
      else if (strcmp (*keyp, "tourism") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* -> %s = %s\n", *keyp, fzk_value);
      }
      else if (strcmp (*keyp, "name") == 0) {
        // "name" wegen Doppelausgabe verwerfen
        // printf ("way/rel building=* && (shop=* || tourism=* || amenity=*) -> %s = %s (verworfen)\n", *keyp, *valp);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
      // printf ("way/rel building=* && (shop=* || tourism=* || amenity=*) -> %s = %s\n", *keyp, *valp);
    END_FOR_ALL_TAGS
    // printf ("\n");
  }
  else if ( KEY_EXISTS(building) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "building") == 0) {
        TAG_WRITE(*keyp, "gebaeude_allgemein")
        // printf ("way/rel building=* -> building=gebaeude_allgemein\n");
      }
      // Flaechentags modifizieren
      else if (strcmp (*keyp, "landuse") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* -> %s = %s\n", *keyp, fzk_value);
      }
      else if (strcmp (*keyp, "natural") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* -> %s = %s\n", *keyp, fzk_value);
      }
      else if (strcmp (*keyp, "power") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* -> %s = %s\n", *keyp, fzk_value);
      }
      else if (strcmp (*keyp, "leisure") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* -> %s = %s\n", *keyp, fzk_value);
      }
      else if (strcmp (*keyp, "amenity") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* -> %s = %s\n", *keyp, fzk_value);
      }
      else if (strcmp (*keyp, "tourism") == 0) {
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* -> %s = %s\n", *keyp, fzk_value);
      }
      else if (strcmp (*keyp, "man_made") == 0) {
        // Objekte ggf. dedizierter umbenennnen (water_works, wastewater_plant, ...)
        snprintf (fzk_value, sizeof(fzk_value), "%s_%s", *valp, "gebaeude");
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel building=* -> %s = %s\n", *keyp, fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Größe der Flächen in einem Tag ergänzen

  else if ( (TAG_EXISTS(tourism, "zoo") || TAG_EXISTS(tourism, "museum") || TAG_EXISTS(tourism, "theme_park")) && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    // printf ("\nway/rel tourism=zoo/museum/theme_park: bBoxWeight = %ld\n", my_bBoxWeight);
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
      // printf ("way/rel tourism=zoo/museum/theme_park: %s = %s\n", *keyp, *valp);
    END_FOR_ALL_TAGS
    // Flächengröße hinzufügen
    if (my_bBoxWeight <= 15) {
      TAG_WRITE("area_size", "small")
    }
    else {
      TAG_WRITE("area_size", "large")
    }
  }

  else if ( TAG_EXISTS(landuse, "reservoir") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "reservoir_huge")
          // printf ("way/rel landuse=reservoir >= 26 = reservoir_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "reservoir_large")
          // printf ("way/rel landuse=reservoir >= 24 = reservoir_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "reservoir_medium")
          // printf ("way/rel landuse=reservoir >= 22 = reservoir_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "reservoir_small")
          // printf ("way/rel landuse=reservoir >= 20 = reservoir_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "reservoir_tiny")
          // printf ("way/rel landuse=reservoir >= 18 = reservoir_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "reservoir_micro")
          // printf ("way/rel landuse=reservoir >= 16 = reservoir_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "reservoir_nano")
          // printf ("way/rel landuse=reservoir >= 14 = reservoir_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=reservoir < 14 = reservoir\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "vineyard") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "vineyard_huge")
          // printf ("way/rel landuse=vineyard >= 26 = vineyard_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "vineyard_large")
          // printf ("way/rel landuse=vineyard >= 24 = vineyard_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "vineyard_medium")
          // printf ("way/rel landuse=vineyard >= 22 = vineyard_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "vineyard_small")
          // printf ("way/rel landuse=vineyard >= 20 = vineyard_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "vineyard_tiny")
          // printf ("way/rel landuse=vineyard >= 18 = vineyard_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "vineyard_micro")
          // printf ("way/rel landuse=vineyard >= 16 = vineyard_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "vineyard_nano")
          // printf ("way/rel landuse=vineyard >= 14 = vineyard_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=vineyard < 14 = vineyard\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "mud") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "mud_huge")
          // printf ("way/rel natural=mud >= 26 = mud_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "mud_large")
          // printf ("way/rel natural=mud >= 24 = mud_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "mud_medium")
          // printf ("way/rel natural=mud >= 22 = mud_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "mud_small")
          // printf ("way/rel natural=mud >= 20 = mud_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "mud_tiny")
          // printf ("way/rel natural=mud >= 18 = mud_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "mud_micro")
          // printf ("way/rel natural=mud >= 16 = mud_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "mud_nano")
          // printf ("way/rel natural=mud >= 14 = mud_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=mud < 14 = mud\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "shingle") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "shingle_huge")
          // printf ("way/rel natural=shingle >= 26 = shingle_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "shingle_large")
          // printf ("way/rel natural=shingle >= 24 = shingle_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "shingle_medium")
          // printf ("way/rel natural=shingle >= 22 = shingle_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "shingle_small")
          // printf ("way/rel natural=shingle >= 20 = shingle_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "shingle_tiny")
          // printf ("way/rel natural=shingle >= 18 = shingle_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "shingle_micro")
          // printf ("way/rel natural=shingle >= 16 = shingle_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "shingle_nano")
          // printf ("way/rel natural=shingle >= 14 = shingle_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=shingle < 14 = shingle\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "shoal") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "shoal_huge")
          // printf ("way/rel natural=shoal >= 26 = shoal_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "shoal_large")
          // printf ("way/rel natural=shoal >= 24 = shoal_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "shoal_medium")
          // printf ("way/rel natural=shoal >= 22 = shoal_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "shoal_small")
          // printf ("way/rel natural=shoal >= 20 = shoal_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "shoal_tiny")
          // printf ("way/rel natural=shoal >= 18 = shoal_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "shoal_micro")
          // printf ("way/rel natural=shoal >= 16 = shoal_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "shoal_nano")
          // printf ("way/rel natural=shoal >= 14 = shoal_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=shoal < 14 = shoal\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "reef") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "reef_huge")
          // printf ("way/rel natural=reef >= 26 = reef_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "reef_large")
          // printf ("way/rel natural=reef >= 24 = reef_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "reef_medium")
          // printf ("way/rel natural=reef >= 22 = reef_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "reef_small")
          // printf ("way/rel natural=reef >= 20 = reef_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "reef_tiny")
          // printf ("way/rel natural=reef >= 18 = reef_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "reef_micro")
          // printf ("way/rel natural=reef >= 16 = reef_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "reef_nano")
          // printf ("way/rel natural=reef >= 14 = reef_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=reef < 14 = reef\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "residential") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "residential_huge")
          // printf ("way/rel landuse=residential >= 26 = residential_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "residential_large")
          // printf ("way/rel landuse=residential >= 24 = residential_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "residential_medium")
          // printf ("way/rel landuse=residential >= 22 = residential_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "residential_small")
          // printf ("way/rel landuse=residential >= 20 = residential_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "residential_tiny")
          // printf ("way/rel landuse=residential >= 18 = residential_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "residential_micro")
          // printf ("way/rel landuse=residential >= 16 = residential_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "residential_nano")
          // printf ("way/rel landuse=residential >= 14 = residential_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=residential < 14 = residential\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "farm") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "farm_huge")
          // printf ("way/rel landuse=farm >= 26 = farm_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "farm_large")
          // printf ("way/rel landuse=farm >= 24 = farm_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "farm_medium")
          // printf ("way/rel landuse=farm >= 22 = farm_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "farm_small")
          // printf ("way/rel landuse=farm >= 20 = farm_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "farm_tiny")
          // printf ("way/rel landuse=farm >= 18 = farm_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "farm_micro")
          // printf ("way/rel landuse=farm >= 16 = farm_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "farm_nano")
          // printf ("way/rel landuse=farm >= 14 = farm_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=farm < 14 = farm\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "farmland") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "farmland_huge")
          // printf ("way/rel landuse=farmland >= 26 = farmland_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "farmland_large")
          // printf ("way/rel landuse=farmland >= 24 = farmland_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "farmland_medium")
          // printf ("way/rel landuse=farmland >= 22 = farmland_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "farmland_small")
          // printf ("way/rel landuse=farmland >= 20 = farmland_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "farmland_tiny")
          // printf ("way/rel landuse=farmland >= 18 = farmland_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "farmland_micro")
          // printf ("way/rel landuse=farmland >= 16 = farmland_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "farmland_nano")
          // printf ("way/rel landuse=farmland >= 14 = farmland_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=farmland < 14 = farmland\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "field") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "field_huge")
          // printf ("way/rel landuse=field >= 26 = field_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "field_large")
          // printf ("way/rel landuse=field >= 24 = field_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "field_medium")
          // printf ("way/rel landuse=field >= 22 = field_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "field_small")
          // printf ("way/rel landuse=field >= 20 = field_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "field_tiny")
          // printf ("way/rel landuse=field >= 18 = field_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "field_micro")
          // printf ("way/rel landuse=field >= 16 = field_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "field_nano")
          // printf ("way/rel landuse=field >= 14 = field_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=field < 14 = field\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "meadow") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "meadow_huge")
          // printf ("way/rel landuse=meadow >= 26 = meadow_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "meadow_large")
          // printf ("way/rel landuse=meadow >= 24 = meadow_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "meadow_medium")
          // printf ("way/rel landuse=meadow >= 22 = meadow_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "meadow_small")
          // printf ("way/rel landuse=meadow >= 20 = meadow_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "meadow_tiny")
          // printf ("way/rel landuse=meadow >= 18 = meadow_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "meadow_micro")
          // printf ("way/rel landuse=meadow >= 16 = meadow_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "meadow_nano")
          // printf ("way/rel landuse=meadow >= 14 = meadow_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=meadow < 14 = meadow\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "grass") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "grass_huge")
          // printf ("way/rel landuse=grass >= 26 = grass_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "grass_large")
          // printf ("way/rel landuse=grass >= 24 = grass_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "grass_medium")
          // printf ("way/rel landuse=grass >= 22 = grass_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "grass_small")
          // printf ("way/rel landuse=grass >= 20 = grass_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "grass_tiny")
          // printf ("way/rel landuse=grass >= 18 = grass_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "grass_micro")
          // printf ("way/rel landuse=grass >= 16 = grass_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "grass_nano")
          // printf ("way/rel landuse=grass >= 14 = grass_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=grass < 14 = grass\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "quarry") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "quarry_huge")
          // printf ("way/rel landuse=quarry >= 26 = quarry_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "quarry_large")
          // printf ("way/rel landuse=quarry >= 24 = quarry_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "quarry_medium")
          // printf ("way/rel landuse=quarry >= 22 = quarry_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "quarry_small")
          // printf ("way/rel landuse=quarry >= 20 = quarry_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "quarry_tiny")
          // printf ("way/rel landuse=quarry >= 18 = quarry_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "quarry_micro")
          // printf ("way/rel landuse=quarry >= 16 = quarry_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "quarry_nano")
          // printf ("way/rel landuse=quarry >= 14 = quarry_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=quarry < 14 = quarry\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "industrial") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "industrial_huge")
          // printf ("way/rel landuse=industrial >= 26 = industrial_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "industrial_large")
          // printf ("way/rel landuse=industrial >= 24 = industrial_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "industrial_medium")
          // printf ("way/rel landuse=industrial >= 22 = industrial_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "industrial_small")
          // printf ("way/rel landuse=industrial >= 20 = industrial_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "industrial_tiny")
          // printf ("way/rel landuse=industrial >= 18 = industrial_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "industrial_micro")
          // printf ("way/rel landuse=industrial >= 16 = industrial_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "industrial_nano")
          // printf ("way/rel landuse=industrial >= 14 = industrial_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=industrial < 14 = industrial\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "commercial") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "commercial_huge")
          // printf ("way/rel landuse=commercial >= 26 = commercial_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "commercial_large")
          // printf ("way/rel landuse=commercial >= 24 = commercial_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "commercial_medium")
          // printf ("way/rel landuse=commercial >= 22 = commercial_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "commercial_small")
          // printf ("way/rel landuse=commercial >= 20 = commercial_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "commercial_tiny")
          // printf ("way/rel landuse=commercial >= 18 = commercial_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "commercial_micro")
          // printf ("way/rel landuse=commercial >= 16 = commercial_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "commercial_nano")
          // printf ("way/rel landuse=commercial >= 14 = commercial_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=commercial < 14 = commercial\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(leisure, "park") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "leisure") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "park_huge")
          // printf ("way/rel leisure=park >= 26 = park_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "park_large")
          // printf ("way/rel leisure=park >= 24 = park_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "park_medium")
          // printf ("way/rel leisure=park >= 22 = park_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "park_small")
          // printf ("way/rel leisure=park >= 20 = park_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "park_tiny")
          // printf ("way/rel leisure=park >= 18 = park_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "park_micro")
          // printf ("way/rel leisure=park >= 16 = park_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "park_nano")
          // printf ("way/rel leisure=park >= 14 = park_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel leisure=park < 14 = park\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "fell") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "fell_huge")
          // printf ("way/rel natural=fell >= 26 = fell_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "fell_large")
          // printf ("way/rel natural=fell >= 24 = fell_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "fell_medium")
          // printf ("way/rel natural=fell >= 22 = fell_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "fell_small")
          // printf ("way/rel natural=fell >= 20 = fell_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "fell_tiny")
          // printf ("way/rel natural=fell >= 18 = fell_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "fell_micro")
          // printf ("way/rel natural=fell >= 16 = fell_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "fell_nano")
          // printf ("way/rel natural=fell >= 14 = fell_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=fell < 14 = fell\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "forest") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "forest_huge")
          // printf ("way/rel landuse=forest >= 26 = forest_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "forest_large")
          // printf ("way/rel landuse=forest >= 24 = forest_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "forest_medium")
          // printf ("way/rel landuse=forest >= 22 = forest_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "forest_small")
          // printf ("way/rel landuse=forest >= 20 = forest_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "forest_tiny")
          // printf ("way/rel landuse=forest >= 18 = forest_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "forest_micro")
          // printf ("way/rel landuse=forest >= 16 = forest_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "forest_nano")
          // printf ("way/rel landuse=forest >= 14 = forest_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=forest < 14 = forest\n");
        }
      }
      else if ( (strcmp (*keyp, "wood") == 0) && (strcmp (*valp, "deciduous") == 0) ) {
        // Tag umdefinieren
        TAG_WRITE("leaf_type", "broadleaved")
        // printf ("way/rel landuse=forest + wood=deciduous -> leaf_type=broadleaved\n");
      }
      else if ( (strcmp (*keyp, "wood") == 0) && (strcmp (*valp, "coniferous") == 0) ) {
        // Tag umdefinieren
        TAG_WRITE("leaf_type", "needleleaved")
        // printf ("way/rel landuse=forest + wood=coniferous -> leaf_type=needleleaved\n");
      }
      else if ( (strcmp (*keyp, "wood") == 0) && (strcmp (*valp, "mixed") == 0) ) {
        // Tag umdefinieren
        TAG_WRITE("leaf_type", "mixed")
        // printf ("way/rel landuse=forest + wood=mixed -> leaf_type=mixed\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "wood") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "wood_huge")
          // printf ("way/rel natural=wood >= 26 = wood_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "wood_large")
          // printf ("way/rel natural=wood >= 24 = wood_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "wood_medium")
          // printf ("way/rel natural=wood >= 22 = wood_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "wood_small")
          // printf ("way/rel natural=wood >= 20 = wood_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "wood_tiny")
          // printf ("way/rel natural=wood >= 18 = wood_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "wood_micro")
          // printf ("way/rel natural=wood >= 16 = wood_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "wood_nano")
          // printf ("way/rel natural=wood >= 14 = wood_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=wood < 14 = forest\n");
        }
      }
      else if ( (strcmp (*keyp, "wood") == 0) && (strcmp (*valp, "deciduous") == 0) ) {
        // Tag umdefinieren
        TAG_WRITE("leaf_type", "broadleaved")
        // printf ("way/rel natural=wood + wood=deciduous -> leaf_type=broadleaved\n");
      }
      else if ( (strcmp (*keyp, "wood") == 0) && (strcmp (*valp, "coniferous") == 0) ) {
        // Tag umdefinieren
        TAG_WRITE("leaf_type", "needleleaved")
        // printf ("way/rel natural=wood + wood=coniferous -> leaf_type=needleleaved\n");
      }
      else if ( (strcmp (*keyp, "wood") == 0) && (strcmp (*valp, "mixed") == 0) ) {
        // Tag umdefinieren
        TAG_WRITE("leaf_type", "mixed")
        // printf ("way/rel natural=wood + wood=mixed -> leaf_type=mixed\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "landfill") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "landfill_huge")
          // printf ("way/rel landuse=landfill >= 26 = landfill_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "landfill_large")
          // printf ("way/rel landuse=landfill >= 24 = landfill_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "landfill_medium")
          // printf ("way/rel landuse=landfill >= 22 = landfill_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "landfill_small")
          // printf ("way/rel landuse=landfill >= 20 = landfill_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "landfill_tiny")
          // printf ("way/rel landuse=landfill >= 18 = landfill_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "landfill_micro")
          // printf ("way/rel landuse=landfill >= 16 = landfill_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "landfill_nano")
          // printf ("way/rel landuse=landfill >= 14 = landfill_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=landfill < 14 = landfill\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "grassland") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "grassland_huge")
          // printf ("way/rel natural=grassland >= 26 = grassland_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "grassland_large")
          // printf ("way/rel natural=grassland >= 24 = grassland_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "grassland_medium")
          // printf ("way/rel natural=grassland >= 22 = grassland_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "grassland_small")
          // printf ("way/rel natural=grassland >= 20 = grassland_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "grassland_tiny")
          // printf ("way/rel natural=grassland >= 18 = grassland_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "grassland_micro")
          // printf ("way/rel natural=grassland >= 16 = grassland_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "grassland_nano")
          // printf ("way/rel natural=grassland >= 14 = grassland_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=grassland < 14 = grassland\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "heath") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "heath_huge")
          // printf ("way/rel natural=heath >= 26 = heath_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "heath_large")
          // printf ("way/rel natural=heath >= 24 = heath_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "heath_medium")
          // printf ("way/rel natural=heath >= 22 = heath_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "heath_small")
          // printf ("way/rel natural=heath >= 20 = heath_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "heath_tiny")
          // printf ("way/rel natural=heath >= 18 = heath_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "heath_micro")
          // printf ("way/rel natural=heath >= 16 = heath_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "heath_nano")
          // printf ("way/rel natural=heath >= 14 = heath_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=heath < 14 = heath\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "scree") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "scree_huge")
          // printf ("way/rel natural=scree >= 26 = scree_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "scree_large")
          // printf ("way/rel natural=scree >= 24 = scree_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "scree_medium")
          // printf ("way/rel natural=scree >= 22 = scree_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "scree_small")
          // printf ("way/rel natural=scree >= 20 = scree_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "scree_tiny")
          // printf ("way/rel natural=scree >= 18 = scree_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "scree_micro")
          // printf ("way/rel natural=scree >= 16 = scree_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "scree_nano")
          // printf ("way/rel natural=scree >= 14 = scree_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=scree < 14 = scree\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "bare_rock") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "bare_rock_huge")
          // printf ("way/rel natural=bare_rock >= 26 = bare_rock_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "bare_rock_large")
          // printf ("way/rel natural=bare_rock >= 24 = bare_rock_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "bare_rock_medium")
          // printf ("way/rel natural=bare_rock >= 22 = bare_rock_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "bare_rock_small")
          // printf ("way/rel natural=bare_rock >= 20 = bare_rock_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "bare_rock_tiny")
          // printf ("way/rel natural=bare_rock >= 18 = bare_rock_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "bare_rock_micro")
          // printf ("way/rel natural=bare_rock >= 16 = bare_rock_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "bare_rock_nano")
          // printf ("way/rel natural=bare_rock >= 14 = bare_rock_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=bare_rock < 14 = bare_rock\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "sand") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "sand_huge")
          // printf ("way/rel natural=sand >= 26 = sand_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "sand_large")
          // printf ("way/rel natural=sand >= 24 = sand_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "sand_medium")
          // printf ("way/rel natural=sand >= 22 = sand_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "sand_small")
          // printf ("way/rel natural=sand >= 20 = sand_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "sand_tiny")
          // printf ("way/rel natural=sand >= 18 = sand_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "sand_micro")
          // printf ("way/rel natural=sand >= 16 = sand_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "sand_nano")
          // printf ("way/rel natural=sand >= 14 = sand_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=sand < 14 = sand\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(aeroway, "aerodrome") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "aeroway") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "aerodrome_huge")
          // printf ("way/rel aeroway=aerodrome >= 26 = aerodrome_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "aerodrome_large")
          // printf ("way/rel aeroway=aerodrome >= 24 = aerodrome_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "aerodrome_medium")
          // printf ("way/rel aeroway=aerodrome >= 22 = aerodrome_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "aerodrome_small")
          // printf ("way/rel aeroway=aerodrome >= 20 = aerodrome_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "aerodrome_tiny")
          // printf ("way/rel aeroway=aerodrome >= 18 = aerodrome_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "aerodrome_micro")
          // printf ("way/rel aeroway=aerodrome >= 16 = aerodrome_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "aerodrome_nano")
          // printf ("way/rel aeroway=aerodrome >= 14 = aerodrome_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel aeroway=aerodrome < 14 = aerodrome\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "glacier") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "glacier_huge")
          // printf ("way/rel natural=glacier >= 26 = glacier_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "glacier_large")
          // printf ("way/rel natural=glacier >= 24 = glacier_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "glacier_medium")
          // printf ("way/rel natural=glacier >= 22 = glacier_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "glacier_small")
          // printf ("way/rel natural=glacier >= 20 = glacier_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "glacier_tiny")
          // printf ("way/rel natural=glacier >= 18 = glacier_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "glacier_micro")
          // printf ("way/rel natural=glacier >= 16 = glacier_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "glacier_nano")
          // printf ("way/rel natural=glacier >= 14 = glacier_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=glacier < 14 = glacier\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "scrub") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "scrub_huge")
          // printf ("way/rel natural=scrub >= 26 = scrub_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "scrub_large")
          // printf ("way/rel natural=scrub >= 24 = scrub_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "scrub_medium")
          // printf ("way/rel natural=scrub >= 22 = scrub_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "scrub_small")
          // printf ("way/rel natural=scrub >= 20 = scrub_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "scrub_tiny")
          // printf ("way/rel natural=scrub >= 18 = scrub_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "scrub_micro")
          // printf ("way/rel natural=scrub >= 16 = scrub_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "scrub_nano")
          // printf ("way/rel natural=scrub >= 14 = scrub_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=scrub < 14 = scrub\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "cemetery") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "cemetery_huge")
          // printf ("way/rel landuse=cemetery >= 26 = cemetery_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "cemetery_large")
          // printf ("way/rel landuse=cemetery >= 24 = cemetery_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "cemetery_medium")
          // printf ("way/rel landuse=cemetery >= 22 = cemetery_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "cemetery_small")
          // printf ("way/rel landuse=cemetery >= 20 = cemetery_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "cemetery_tiny")
          // printf ("way/rel landuse=cemetery >= 18 = cemetery_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "cemetery_micro")
          // printf ("way/rel landuse=cemetery >= 16 = cemetery_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "cemetery_nano")
          // printf ("way/rel landuse=cemetery >= 14 = cemetery_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=cemetery < 14 = cemetery\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(natural, "water") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "water_huge")
          // printf ("way/rel natural=water >= 26 = water_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "water_large")
          // printf ("way/rel natural=water >= 24 = water_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "water_medium")
          // printf ("way/rel natural=water >= 22 = water_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "water_small")
          // printf ("way/rel natural=water >= 20 = water_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "water_tiny")
          // printf ("way/rel natural=water >= 18 = water_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "water_micro")
          // printf ("way/rel natural=water >= 16 = water_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "water_nano")
          // printf ("way/rel natural=water >= 14 = water_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=water < 14 = water\n");
        }
      }
      // else if (strcmp (*keyp, "name") == 0) {
      //   TAG_WRITE(*keyp, *valp)
      //   printf ("way/rel natural=water = name = %s\n", *valp);
      // }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // veraltet: natural=marsh -> natural=wetland + wetland=marsh
  else if ( TAG_EXISTS(natural, "marsh") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "marsh_huge")
          // printf ("way/rel natural=marsh (veraltet) >= 26 = marsh_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "marsh_large")
          // printf ("way/rel natural=marsh (veraltet) >= 24 = marsh_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "marsh_medium")
          // printf ("way/rel natural=marsh (veraltet) >= 22 = marsh_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "marsh_small")
          // printf ("way/rel natural=marsh (veraltet) >= 20 = marsh_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "marsh_tiny")
          // printf ("way/rel natural=marsh (veraltet) >= 18 = marsh_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "marsh_micro")
          // printf ("way/rel natural=marsh (veraltet) >= 16 = marsh_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "marsh_nano")
          // printf ("way/rel natural=marsh (veraltet) >= 14 = marsh_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=marsh (veraltet) < 14 = marsh\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Roehricht, Schilf
  else if ( TAG_EXISTS(natural, "wetland") && TAG_EXISTS(wetland, "reedbed") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "schilf_huge")
          // printf ("way/rel natural=wetland && wetland=reedbed >= 26 = natural=schilf_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "schilf_large")
          // printf ("way/rel natural=wetland && wetland=reedbed >= 24 = natural=schilf_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "schilf_medium")
          // printf ("way/rel natural=wetland && wetland=reedbed >= 22 = natural=schilf_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "schilf_small")
          // printf ("way/rel natural=wetland && wetland=reedbed >= 20 = natural=schilf_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "schilf_tiny")
          // printf ("way/rel natural=wetland && wetland=reedbed >= 18 = natural=schilf_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "schilf_micro")
          // printf ("way/rel natural=wetland && wetland=reedbed >= 16 = natural=schilf_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "schilf_nano")
          // printf ("way/rel natural=wetland && wetland=reedbed >= 14 = natural=schilf_nano\n");
        }
        else {
          TAG_WRITE(*keyp, "schilf")
          // printf ("way/rel natural=wetland && wetland=reedbed < 14 = natural=schilf\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Salz- oder Feuchtwiese, Marsch
  else if ( TAG_EXISTS(natural, "wetland") && (TAG_EXISTS(wetland, "marsh") || TAG_EXISTS(wetland, "saltmarsh") || TAG_EXISTS(wetland, "wet_meadow")) && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "feuchtwiese_huge")
          // printf ("way/rel natural=wetland && wetland=marsh|saltmarsh|wet_meadow >= 26 = natural=feuchtwiese_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "feuchtwiese_large")
          // printf ("way/rel natural=wetland && wetland=marsh|saltmarsh|wet_meadow >= 24 = natural=feuchtwiese_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "feuchtwiese_medium")
          // printf ("way/rel natural=wetland && wetland=marsh|saltmarsh|wet_meadow >= 22 = natural=feuchtwiese_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "feuchtwiese_small")
          // printf ("way/rel natural=wetland && wetland=marsh|saltmarsh|wet_meadow >= 20 = natural=feuchtwiese_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "feuchtwiese_tiny")
          // printf ("way/rel natural=wetland && wetland=marsh|saltmarsh|wet_meadow >= 18 = natural=feuchtwiese_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "feuchtwiese_micro")
          // printf ("way/rel natural=wetland && wetland=marsh|saltmarsh|wet_meadow >= 16 = natural=feuchtwiese_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "feuchtwiese_nano")
          // printf ("way/rel natural=wetland && wetland=marsh|saltmarsh|wet_meadow >= 14 = natural=feuchtwiese_nano\n");
        }
        else {
          TAG_WRITE(*keyp, "feuchtwiese")
          // printf ("way/rel natural=wetland && wetland=marsh|saltmarsh|wet_meadow < 14 = natural=feuchtwiese\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // offener Sumpf oder Sumpf
  else if ( TAG_EXISTS(natural, "wetland") && TAG_EXISTS(wetland, "swamp") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "sumpfgebiet_huge")
          // printf ("way/rel natural=wetland && wetland=swamp >= 26 = natural=sumpfgebiet_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "sumpfgebiet_large")
          // printf ("way/rel natural=wetland && wetland=swamp >= 24 = natural=sumpfgebiet_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "sumpfgebiet_medium")
          // printf ("way/rel natural=wetland && wetland=swamp >= 22 = natural=sumpfgebiet_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "sumpfgebiet_small")
          // printf ("way/rel natural=wetland && wetland=swamp >= 20 = natural=sumpfgebiet_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "sumpfgebiet_tiny")
          // printf ("way/rel natural=wetland && wetland=swamp >= 18 = natural=sumpfgebiet_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "sumpfgebiet_micro")
          // printf ("way/rel natural=wetland && wetland=swamp >= 16 = natural=sumpfgebiet_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "sumpfgebiet_nano")
          // printf ("way/rel natural=wetland && wetland=swamp >= 14 = natural=sumpfgebiet_nano\n");
        }
        else {
          TAG_WRITE(*keyp, "sumpfgebiet")
          // printf ("way/rel natural=wetland && wetland=swamp < 14 = natural=sumpfgebiet\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Hoch-, Nieder- oder Strangmoor
  else if ( TAG_EXISTS(natural, "wetland") && (TAG_EXISTS(wetland, "bog") || TAG_EXISTS(wetland, "fen") || TAG_EXISTS(wetland, "string_bog")) && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "moorgebiet_huge")
          // printf ("way/rel natural=wetland && wetland=bog|fen|string_bog >= 26 = natural=moorgebiet_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "moorgebiet_large")
          // printf ("way/rel natural=wetland && wetland=bog|fen|string_bog >= 24 = natural=moorgebiet_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "moorgebiet_medium")
          // printf ("way/rel natural=wetland && wetland=bog|fen|string_bog >= 22 = natural=moorgebiet_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "moorgebiet_small")
          // printf ("way/rel natural=wetland && wetland=bog|fen|string_bog >= 20 = natural=moorgebiet_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "moorgebiet_tiny")
          // printf ("way/rel natural=wetland && wetland=bog|fen|string_bog >= 18 = natural=moorgebiet_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "moorgebiet_micro")
          // printf ("way/rel natural=wetland && wetland=bog|fen|string_bog >= 16 = natural=moorgebiet_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "moorgebiet_nano")
          // printf ("way/rel natural=wetland && wetland=bog|fen|string_bog >= 14 = natural=moorgebiet_nano\n");
        }
        else {
          TAG_WRITE(*keyp, "moorgebiet")
          // printf ("way/rel natural=wetland && wetland=bog|fen|string_bog < 14 = natural=moorgebiet\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Watt
  else if ( TAG_EXISTS(natural, "wetland") && TAG_EXISTS(wetland, "tidalflat") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "watt_huge")
          // printf ("way/rel natural=wetland && wetland=tidalflat >= 26 = natural=watt_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "watt_large")
          // printf ("way/rel natural=wetland && wetland=tidalflat >= 24 = natural=watt_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "watt_medium")
          // printf ("way/rel natural=wetland && wetland=tidalflat >= 22 = natural=watt_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "watt_small")
          // printf ("way/rel natural=wetland && wetland=tidalflat >= 20 = natural=watt_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "watt_tiny")
          // printf ("way/rel natural=wetland && wetland=tidalflat >= 18 = natural=watt_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "watt_micro")
          // printf ("way/rel natural=wetland && wetland=tidalflat >= 16 = natural=watt_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "watt_nano")
          // printf ("way/rel natural=wetland && wetland=tidalflat >= 14 = natural=watt_nano\n");
        }
        else {
          TAG_WRITE(*keyp, "watt")
          // printf ("way/rel natural=wetland && wetland=tidalflat < 14 = natural=watt\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // (Salzwasser-) Saline
  else if ( TAG_EXISTS(natural, "wetland") && TAG_EXISTS(wetland, "saltern") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "saline_huge")
          // printf ("way/rel natural=wetland && wetland=saltern >= 26 = natural=saline_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "saline_large")
          // printf ("way/rel natural=wetland && wetland=saltern >= 24 = natural=saline_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "saline_medium")
          // printf ("way/rel natural=wetland && wetland=saltern >= 22 = natural=saline_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "saline_small")
          // printf ("way/rel natural=wetland && wetland=saltern >= 20 = natural=saline_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "saline_tiny")
          // printf ("way/rel natural=wetland && wetland=saltern >= 18 = natural=saline_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "saline_micro")
          // printf ("way/rel natural=wetland && wetland=saltern >= 16 = natural=saline_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "saline_nano")
          // printf ("way/rel natural=wetland && wetland=saltern >= 14 = natural=saline_nano\n");
        }
        else {
          TAG_WRITE(*keyp, "saline")
          // printf ("way/rel natural=wetland && wetland=saltern < 14 = natural=saline\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // (Salzwasser-) Saline
  else if ( TAG_EXISTS(landuse, "salt_pond") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "salt_pond_huge")
          // printf ("way/rel landuse=salt_pond >= 26 = landuse=salt_pond_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "salt_pond_large")
          // printf ("way/rel landuse=salt_pond >= 24 = landuse=salt_pond_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "salt_pond_medium")
          // printf ("way/rel landuse=salt_pond >= 22 = landuse=salt_pond_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "salt_pond_small")
          // printf ("way/rel landuse=salt_pond >= 20 = landuse=salt_pond_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "salt_pond_tiny")
          // printf ("way/rel landuse=salt_pond >= 18 = landuse=salt_pond_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "salt_pond_micro")
          // printf ("way/rel landuse=salt_pond >= 16 = landuse=salt_pond_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "salt_pond_nano")
          // printf ("way/rel landuse=salt_pond >= 14 = landuse=salt_pond_nano\n");
        }
        else {
          TAG_WRITE(*keyp, "salt_pond")
          // printf ("way/rel landuse=salt_pond < 14 = landuse=salt_pond\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // allgemeines Feuchtgebiet (ohne Angabe von wetland=*)
  else if ( TAG_EXISTS(natural, "wetland") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "natural") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "wetland_huge")
          // printf ("way/rel natural=wetland (ohne wetland=*) >= 26 = wetland_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "wetland_large")
          // printf ("way/rel natural=wetland (ohne wetland=*) >= 24 = wetland_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "wetland_medium")
          // printf ("way/rel natural=wetland (ohne wetland=*) >= 22 = wetland_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "wetland_small")
          // printf ("way/rel natural=wetland (ohne wetland=*) >= 20 = wetland_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "wetland_tiny")
          // printf ("way/rel natural=wetland (ohne wetland=*) >= 18 = wetland_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "wetland_micro")
          // printf ("way/rel natural=wetland (ohne wetland=*) >= 16 = wetland_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "wetland_nano")
          // printf ("way/rel natural=wetland (ohne wetland=*) >= 14 = wetland_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel natural=wetland (ohne wetland=*) < 14 = wetland\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  else if ( TAG_EXISTS(landuse, "military") && KEY_EXISTS(bBoxWeight) ) {
    long my_bBoxWeight = atol (valp_bBoxWeight);
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        if (my_bBoxWeight >= 26) {
          TAG_WRITE(*keyp, "military_huge")
          // printf ("way/rel landuse=military >= 26 = military_huge\n");
        }
        else if (my_bBoxWeight >= 24) {
          TAG_WRITE(*keyp, "military_large")
          // printf ("way/rel landuse=military >= 24 = military_large\n");
        }
        else if (my_bBoxWeight >= 22) {
          TAG_WRITE(*keyp, "military_medium")
          // printf ("way/rel landuse=military >= 22 = military_medium\n");
        }
        else if (my_bBoxWeight >= 20) {
          TAG_WRITE(*keyp, "military_small")
          // printf ("way/rel landuse=military >= 20 = military_small\n");
        }
        else if (my_bBoxWeight >= 18) {
          TAG_WRITE(*keyp, "military_tiny")
          // printf ("way/rel landuse=military >= 18 = military_tiny\n");
        }
        else if (my_bBoxWeight >= 16) {
          TAG_WRITE(*keyp, "military_micro")
          // printf ("way/rel landuse=military >= 16 = military_micro\n");
        }
        else if (my_bBoxWeight >= 14) {
          TAG_WRITE(*keyp, "military_nano")
          // printf ("way/rel landuse=military >= 14 = military_nano\n");
        }
        else {
          TAG_WRITE(*keyp, *valp)
          // printf ("way/rel landuse=military < 14 = military\n");
        }
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Behandlung von nur gelegentlich wasserfuehrenden Baechen
  else if ( TAG_EXISTS(waterway, "stream") && TAG_EXISTS(intermittent, "yes") && KEY_EXISTS(tunnel)) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "waterway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "wadi_tunnel")
        // printf ("way/rel waterway stream + intermittent + tunnel -> wadi_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(waterway, "stream") && TAG_EXISTS(intermittent, "yes") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "waterway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "wadi")
        // printf ("way/rel waterway stream + intermittent -> wadi\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Behandlung von kuenstlich unter die Erde gelegten Wasserlaeufen (tunnel=yes/culvert)
  else if ( TAG_EXISTS(waterway, "canal") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "waterway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "canal_tunnel")
        // printf ("way/rel waterway canal + tunnel -> canal_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(waterway, "ditch") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "waterway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "ditch_tunnel")
        // printf ("way/rel waterway ditch + tunnel -> ditch_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(waterway, "drain") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "waterway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "drain_tunnel")
        // printf ("way/rel waterway drain + tunnel -> drain_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(waterway, "river") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "waterway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "river_tunnel")
        // printf ("way/rel waterway river + tunnel -> river_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(waterway, "stream") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "waterway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "stream_tunnel")
        // printf ("way/rel waterway stream + tunnel -> stream_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(waterway, "wadi") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "waterway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "wadi_tunnel")
        // printf ("way/rel waterway wadi + tunnel -> wadi_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "motorway") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "motorway_tunnel")
        // printf ("way/rel highway motorway + tunnel -> motorway_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "motorway") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "motorway_bridge")
        // printf ("way/rel highway motorway + bridge -> motorway_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "motorway_link") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "motorway_link_tunnel")
        // printf ("way/rel highway motorway_link + tunnel -> motorway_link_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "motorway_link") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "motorway_link_bridge")
        // printf ("way/rel highway motorway_link + bridge -> motorway_link_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "trunk") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "trunk_tunnel")
        // printf ("way/rel highway trunk + tunnel -> trunk_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "trunk") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "trunk_bridge")
        // printf ("way/rel highway trunk + bridge -> trunk_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "trunk_link") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "trunk_link_tunnel")
        // printf ("way/rel highway trunk_link + tunnel -> trunk_link_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "trunk_link") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "trunk_link_bridge")
        // printf ("way/rel highway trunk_link + bridge -> trunk_link_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "primary") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "primary_tunnel")
        // printf ("way/rel highway primary + tunnel -> primary_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "primary") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "primary_bridge")
        // printf ("way/rel highway primary + bridge -> primary_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "primary_link") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "primary_link_tunnel")
        // printf ("way/rel highway primary_link + tunnel -> primary_link_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "primary_link") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "primary_link_bridge")
        // printf ("way/rel highway primary_link + bridge -> primary_link_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "secondary") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "secondary_tunnel")
        // printf ("way/rel highway secondary + tunnel -> secondary_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "secondary") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "secondary_bridge")
        // printf ("way/rel highway secondary + bridge -> secondary_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "secondary_link") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "secondary_link_tunnel")
        // printf ("way/rel highway secondary_link + tunnel -> secondary_link_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "secondary_link") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "secondary_link_bridge")
        // printf ("way/rel highway secondary_link + bridge -> secondary_link_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "tertiary") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "tertiary_tunnel")
        // printf ("way/rel highway tertiary + tunnel -> tertiary_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "tertiary") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "tertiary_bridge")
        // printf ("way/rel highway tertiary + bridge -> tertiary_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "tertiary_link") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "tertiary_link_tunnel")
        // printf ("way/rel highway tertiary_link + tunnel -> tertiary_link_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "tertiary_link") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "tertiary_link_bridge")
        // printf ("way/rel highway tertiary_link + bridge -> tertiary_link_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "living_street") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "living_street_tunnel")
        // printf ("way/rel highway living_street + tunnel -> living_street_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "living_street") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "living_street_bridge")
        // printf ("way/rel highway living_street + bridge -> living_street_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "pedestrian") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "pedestrian_tunnel")
        // printf ("way/rel highway pedestrian + tunnel -> pedestrian_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "pedestrian") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "pedestrian_bridge")
        // printf ("way/rel highway pedestrian + bridge -> pedestrian_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "residential") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "residential_tunnel")
        // printf ("way/rel highway residential + tunnel -> residential_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "residential") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "residential_bridge")
        // printf ("way/rel highway residential + bridge -> residential_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "unclassified") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "unclassified_tunnel")
        // printf ("way/rel highway unclassified + tunnel -> unclassified_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "unclassified") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "unclassified_bridge")
        // printf ("way/rel highway unclassified + bridge -> unclassified_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "service") && KEY_EXISTS(service) && KEY_EXISTS(tunnel)) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "service_small_tunnel")
        // printf ("way/rel highway service + service + tunnel -> service_small_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "service") && KEY_EXISTS(service) && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "service_small_bridge")
        // printf ("way/rel highway service + service + bridge -> service_small_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "service") && KEY_EXISTS(service) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "service_small")
        // printf ("way/rel highway service + service -> service_small\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "service") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "service_tunnel")
        // printf ("way/rel highway service + tunnel -> service_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "service") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "service_bridge")
        // printf ("way/rel highway service + bridge -> service_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }


  else if ( TAG_EXISTS(highway, "road") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "road_tunnel")
        // printf ("way/rel highway road + tunnel -> road_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "road") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "road_bridge")
        // printf ("way/rel highway road + bridge -> road_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "construction") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "construction_tunnel")
        // printf ("way/rel highway construction + tunnel -> construction_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "construction") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "construction_bridge")
        // printf ("way/rel highway construction + bridge -> construction_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "steps") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "steps_tunnel")
        // printf ("way/rel highway steps + tunnel -> steps_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "steps") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "steps_bridge")
        // printf ("way/rel highway steps + bridge -> steps_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(abandoned, "yes") && KEY_EXISTS(highway) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = abandoned" umdefinieren
        TAG_WRITE(*keyp, "abandoned")
        // printf ("way/rel highway * + abandoned -> abandoned\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "via_ferrata") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = via_ferrata_tunnel" umdefinieren
        TAG_WRITE(*keyp, "via_ferrata_tunnel")
        // printf ("way/rel highway via_ferrata + tunnel -> via_ferrata_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "via_ferrata") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = via_ferrata_bridge" umdefinieren
        TAG_WRITE(*keyp, "via_ferrata_bridge")
        // printf ("way/rel highway via_ferrata + bridge -> via_ferrata_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "via_ferrata") && KEY_EXISTS(via_ferrata_scale) ) {
    FOR_ALL_TAGS
      if  (strcmp (*keyp, "name") == 0) {
        // via ferrata scale an name anhaengen
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, valp_via_ferrata_scale);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel highway via_ferrata -> name = %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && KEY_EXISTS(via_ferrata_scale) && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = via_ferrata_tunnel" umdefinieren
        TAG_WRITE(*keyp, "via_ferrata_tunnel")
        // printf ("way/rel highway path + via_ferrata_scale + tunnel -> via_ferrata_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && KEY_EXISTS(via_ferrata_scale) && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = via_ferrata_bridge" umdefinieren
        TAG_WRITE(*keyp, "via_ferrata_bridge")
        // printf ("way/rel highway path + via_ferrata_scale + bridge -> via_ferrata_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && KEY_EXISTS(via_ferrata_scale) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = via_ferrata" umdefinieren
        TAG_WRITE(*keyp, "via_ferrata")
        // printf ("way/rel highway path + via_ferrata_scale -> via_ferrata\n");
      }
      else if  (strcmp (*keyp, "name") == 0) {
        // via ferrata scale an name anhaengen
        snprintf (fzk_value, sizeof(fzk_value), "%s (%s)", *valp, valp_via_ferrata_scale);
        TAG_WRITE(*keyp, fzk_value)
        // printf ("way/rel highway path + via_ferrata_scale -> via_ferrata -> name = %s\n", fzk_value);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "hiking") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg1_tunnel" umdefinieren (entspricht T1 gelb)
        TAG_WRITE(*keyp, "bergweg1_tunnel")
        // printf ("way/rel highway path + hiking + tunnel -> bergweg1_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "hiking") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg1_bridge" umdefinieren (entspricht T1 gelb)
        TAG_WRITE(*keyp, "bergweg1_bridge")
        // printf ("way/rel highway path + hiking + bridge -> bergweg1_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "hiking") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg1" umdefinieren (entspricht T1 gelb)
        TAG_WRITE(*keyp, "bergweg1")
        // printf ("way/rel highway path + hiking -> bergweg1\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "mountain_hiking") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg2_tunnel" umdefinieren (entspricht T2 rot)
        TAG_WRITE(*keyp, "bergweg2_tunnel")
        // printf ("way/rel highway path + mountain_hiking + tunnel -> bergweg2_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "mountain_hiking") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg2_bridge" umdefinieren (entspricht T2 rot)
        TAG_WRITE(*keyp, "bergweg2_bridge")
        // printf ("way/rel highway path + mountain_hiking + bridge -> bergweg2_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "mountain_hiking") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg2" umdefinieren (entspricht T2 rot)
        TAG_WRITE(*keyp, "bergweg2")
        // printf ("way/rel highway path + mountain_hiking -> bergweg2\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "demanding_mountain_hiking") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg3_tunnel" umdefinieren (entspricht T3 rot)
        TAG_WRITE(*keyp, "bergweg3_tunnel")
        // printf ("way/rel highway path + demanding_mountain_hiking + tunnel -> bergweg3_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "demanding_mountain_hiking") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg3_bridge" umdefinieren (entspricht T3 rot)
        TAG_WRITE(*keyp, "bergweg3_bridge")
        // printf ("way/rel highway path + demanding_mountain_hiking + bridge -> bergweg3_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "demanding_mountain_hiking") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg3" umdefinieren (entspricht T3 rot)
        TAG_WRITE(*keyp, "bergweg3")
        // printf ("way/rel highway path + demanding_mountain_hiking -> bergweg3\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "alpine_hiking") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg4_tunnel" umdefinieren (entspricht T4 blau)
        TAG_WRITE(*keyp, "bergweg4_tunnel")
        // printf ("way/rel highway path + alpine_hiking + tunnel -> bergweg4_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "alpine_hiking") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg4_bridge" umdefinieren (entspricht T4 blau)
        TAG_WRITE(*keyp, "bergweg4_bridge")
        // printf ("way/rel highway path + alpine_hiking + bridge -> bergweg4_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "alpine_hiking") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg4" umdefinieren (entspricht T4 blau)
        TAG_WRITE(*keyp, "bergweg4")
        // printf ("way/rel highway path + alpine_hiking -> bergweg4\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "demanding_alpine_hiking") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg5_tunnel" umdefinieren (entspricht T5 blau)
        TAG_WRITE(*keyp, "bergweg5_tunnel")
        // printf ("way/rel highway path + demanding_alpine_hiking + tunnel -> bergweg5_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "demanding_alpine_hiking") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg5_bridge" umdefinieren (entspricht T5 blau)
        TAG_WRITE(*keyp, "bergweg5_bridge")
        // printf ("way/rel highway path + demanding_alpine_hiking + bridge -> bergweg5_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "demanding_alpine_hiking") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg5" umdefinieren (entspricht T5 blau)
        TAG_WRITE(*keyp, "bergweg5")
        // printf ("way/rel highway path + demanding_alpine_hiking -> bergweg5\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "difficult_alpine_hiking") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg6_tunnel" umdefinieren (entspricht T6 blau)
        TAG_WRITE(*keyp, "bergweg6_tunnel")
        // printf ("way/rel highway path + difficult_alpine_hiking + tunnel -> bergweg6_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "difficult_alpine_hiking") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg6_bridge" umdefinieren (entspricht T6 blau)
        TAG_WRITE(*keyp, "bergweg6_bridge")
        // printf ("way/rel highway path + difficult_alpine_hiking + bridge -> bergweg6_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(sac_scale, "difficult_alpine_hiking") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bergweg6" umdefinieren (entspricht T6 blau)
        TAG_WRITE(*keyp, "bergweg6")
        // printf ("way/rel highway path + difficult_alpine_hiking -> bergweg6\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "track") && TAG_EXISTS(tracktype, "grade1") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = feldweg1_tunnel" umdefinieren
        TAG_WRITE(*keyp, "feldweg1_tunnel")
        // printf ("way/rel highway track + tracktype 1 + tunnel -> feldweg1_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "track") && TAG_EXISTS(tracktype, "grade1") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = feldweg1_bridge" umdefinieren
        TAG_WRITE(*keyp, "feldweg1_bridge")
        // printf ("way/rel highway track + tracktype 1 + bridge -> feldweg1_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "track") && TAG_EXISTS(tracktype, "grade1") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = feldweg1" umdefinieren
        TAG_WRITE(*keyp, "feldweg1")
        // printf ("way/rel highway track + tracktype 1 -> feldweg1\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "track") && (TAG_EXISTS(tracktype, "grade2") || TAG_EXISTS(tracktype, "grade3")) && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = feldweg023_tunnel" umdefinieren
        TAG_WRITE(*keyp, "feldweg023_tunnel")
        // printf ("way/rel highway track + tracktype 2/3 + tunnel -> feldweg023_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "track") && (TAG_EXISTS(tracktype, "grade2") || TAG_EXISTS(tracktype, "grade3")) && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = feldweg023_bridge" umdefinieren
        TAG_WRITE(*keyp, "feldweg023_bridge")
        // printf ("way/rel highway track + tracktype 2/3 + bridge -> feldweg023_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "track") && (TAG_EXISTS(tracktype, "grade2") || TAG_EXISTS(tracktype, "grade3")) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = feldweg023" umdefinieren
        TAG_WRITE(*keyp, "feldweg023")
        // printf ("way/rel highway track + tracktype 2/3 -> feldweg023\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "track") && (TAG_EXISTS(tracktype, "grade4") || TAG_EXISTS(tracktype, "grade5")) && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = feldweg45_tunnel" umdefinieren
        TAG_WRITE(*keyp, "feldweg45_tunnel")
        // printf ("way/rel highway track + tracktype 4/5 + tunnel -> feldweg45_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "track") && (TAG_EXISTS(tracktype, "grade4") || TAG_EXISTS(tracktype, "grade5")) && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = feldweg45_bridge" umdefinieren
        TAG_WRITE(*keyp, "feldweg45_bridge")
        // printf ("way/rel highway track + tracktype 4/5 + bridge -> feldweg45_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "track") && (TAG_EXISTS(tracktype, "grade4") || TAG_EXISTS(tracktype, "grade5")) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = feldweg45" umdefinieren
        TAG_WRITE(*keyp, "feldweg45")
        // printf ("way/rel highway track + tracktype 4/5 -> feldweg45\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "track") && KEY_NOT_EXISTS(tracktype) && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = feldweg023_tunnel" umdefinieren
        TAG_WRITE(*keyp, "feldweg023_tunnel")
        // printf ("way/rel highway track + tracktype missing + tunnel -> feldweg023_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "track") && KEY_NOT_EXISTS(tracktype) && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = feldweg023_bridge" umdefinieren
        TAG_WRITE(*keyp, "feldweg023_bridge")
        // printf ("way/rel highway track + tracktype missing + bridge -> feldweg023\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "track") && KEY_NOT_EXISTS(tracktype) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = feldweg023" umdefinieren
        TAG_WRITE(*keyp, "feldweg023")
        // printf ("way/rel highway track + tracktype missing -> feldweg023\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "rail") && KEY_EXISTS(service) && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = rail_service_tunnel" umdefinieren
        TAG_WRITE(*keyp, "rail_service_tunnel")
        // printf ("way/rel railway rail + service + tunnel -> rail_service_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "rail") && KEY_EXISTS(service) && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = rail_service_bridge" umdefinieren
        TAG_WRITE(*keyp, "rail_service_bridge")
        // printf ("way/rel railway rail + service + bridge -> rail_service_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "rail") && KEY_EXISTS(service) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = rail_service" umdefinieren
        TAG_WRITE(*keyp, "rail_service")
        // printf ("way/rel railway rail + service  -> rail_service\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "rail") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = rail_tunnel" umdefinieren
        TAG_WRITE(*keyp, "rail_tunnel")
        // printf ("way/rel railway rail + tunnel -> rail_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "rail") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = rail_bridge" umdefinieren
        TAG_WRITE(*keyp, "rail_bridge")
        // printf ("way/rel railway rail + bridge -> rail_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "light_rail") && KEY_EXISTS(service) && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = light_rail_service_tunnel" umdefinieren
        TAG_WRITE(*keyp, "light_rail_service_tunnel")
        // printf ("way/rel railway light_rail + service + tunnel -> light_rail_service_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "light_rail") && KEY_EXISTS(service) && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = light_rail_service_bridge" umdefinieren
        TAG_WRITE(*keyp, "light_rail_service_bridge")
        // printf ("way/rel railway light_rail + service + bridge -> light_rail_service_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "light_rail") && KEY_EXISTS(service) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = light_rail_service" umdefinieren
        TAG_WRITE(*keyp, "light_rail_service")
        // printf ("way/rel railway light_rail + service  -> light_rail_service\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "light_rail") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = light_rail_tunnel" umdefinieren
        TAG_WRITE(*keyp, "light_rail_tunnel")
        // printf ("way/rel railway light_rail + tunnel -> light_rail_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "light_rail") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = light_rail_bridge" umdefinieren
        TAG_WRITE(*keyp, "light_rail_bridge")
        // printf ("way/rel railway light_rail + bridge -> light_rail_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "subway") && KEY_EXISTS(service) && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = subway_tunnel_service" umdefinieren
        TAG_WRITE(*keyp, "subway_service_tunnel")
        // printf ("way/rel railway subway + service + tunnel -> subway_tunnel_service\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "subway") && KEY_EXISTS(service) && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = subway_service_bridge" umdefinieren
        TAG_WRITE(*keyp, "subway_service_bridge")
        // printf ("way/rel railway subway + service + bridge -> subway_service_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "subway") && KEY_EXISTS(service) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = subway_service" umdefinieren
        TAG_WRITE(*keyp, "subway_service")
        // printf ("way/rel railway subway + service -> subway_service\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "subway") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = subway_tunnel" umdefinieren
        TAG_WRITE(*keyp, "subway_tunnel")
        // printf ("way/rel railway subway + tunnel -> subway_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "subway") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = subway_bridge" umdefinieren
        TAG_WRITE(*keyp, "subway_bridge")
        // printf ("way/rel railway subway + bridge -> subway_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "tram") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = tram_tunnel" umdefinieren
        TAG_WRITE(*keyp, "tram_tunnel")
        // printf ("way/rel railway tram + tunnel -> tram_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "tram") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = tram_bridge" umdefinieren
        TAG_WRITE(*keyp, "tram_bridge")
        // printf ("way/rel railway tram + bridge -> tram_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "bus_guideway") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = bus_guideway_tunnel" umdefinieren
        TAG_WRITE(*keyp, "bus_guideway_tunnel")
        // printf ("way/rel highway bus_guideway + tunnel -> bus_guideway_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "bus_guideway") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "railway = bus_guideway_bridge" umdefinieren
        TAG_WRITE(*keyp, "bus_guideway_bridge")
        // printf ("way/rel highway bus_guideway + bridge -> bus_guideway_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "miniature") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = miniature_tunnel" umdefinieren
        TAG_WRITE(*keyp, "miniature_tunnel")
        // printf ("way/rel railway miniature + tunnel -> miniature_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "miniature") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = miniature_bridge" umdefinieren
        TAG_WRITE(*keyp, "miniature_bridge")
        // printf ("way/rel railway miniature + bridge -> miniature_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "monorail") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = monorail_tunnel" umdefinieren
        TAG_WRITE(*keyp, "monorail_tunnel")
        // printf ("way/rel railway monorail + tunnel -> monorail_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "monorail") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = monorail_bridge" umdefinieren
        TAG_WRITE(*keyp, "monorail_bridge")
        // printf ("way/rel railway monorail + bridge -> monorail_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "disused") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = disused_tunnel" umdefinieren
        TAG_WRITE(*keyp, "disused_tunnel")
        // printf ("way/rel railway disused + tunnel -> disused_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "disused") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = disused_bridge" umdefinieren
        TAG_WRITE(*keyp, "disused_bridge")
        // printf ("way/rel railway disused + bridge -> disused_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "preserved") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = rail_tunnel" umdefinieren
        TAG_WRITE(*keyp, "rail_tunnel")
        // printf ("way/rel railway preserved + tunnel -> rail_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "preserved") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = rail_bridge" umdefinieren
        TAG_WRITE(*keyp, "rail_bridge")
        // printf ("way/rel railway preserved + bridge -> rail_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "preserved") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = rail" umdefinieren
        TAG_WRITE(*keyp, "rail")
        // printf ("way/rel railway preserved -> rail\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "narrow_gauge") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = narrow_gauge_tunnel" umdefinieren
        TAG_WRITE(*keyp, "narrow_gauge_tunnel")
        // printf ("way/rel railway narrow_gauge + tunnel -> narrow_gauge_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "narrow_gauge") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = narrow_gauge_bridge" umdefinieren
        TAG_WRITE(*keyp, "narrow_gauge_bridge")
        // printf ("way/rel railway narrow_gauge + bridge -> narrow_gauge_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "funicular") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = rail_tunnel" umdefinieren
        TAG_WRITE(*keyp, "rail_tunnel")
        // printf ("way/rel railway funicular + tunnel -> rail_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "funicular") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = rail_bridge" umdefinieren
        TAG_WRITE(*keyp, "rail_bridge")
        // printf ("way/rel railway funicular + bridge -> rail_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "funicular") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = rail" umdefinieren
        TAG_WRITE(*keyp, "rail")
        // printf ("way/rel railway funicular -> rail\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "construction") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = construction_tunnel" umdefinieren
        TAG_WRITE(*keyp, "construction_tunnel")
        // printf ("way/rel railway construction + tunnel -> construction_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(railway, "construction") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "railway") == 0) {
        // zu "railway = construction_bridge" umdefinieren
        TAG_WRITE(*keyp, "construction_bridge")
        // printf ("way/rel railway construction + bridge -> construction_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  // else if ( TAG_EXISTS(railway, "abandoned") && KEY_EXISTS(tunnel) ) {
  //   FOR_ALL_TAGS
  //     if (strcmp (*keyp, "railway") == 0) {
  //       // zu "railway = abandoned_tunnel" umdefinieren
  //       TAG_WRITE(*keyp, "abandoned_tunnel")
  //       // printf ("way/rel railway abandoned + tunnel -> abandoned_tunnel\n");
  //     }
  //     else {
  //       TAG_WRITE(*keyp, *valp)
  //     }
  //   END_FOR_ALL_TAGS
  // }
  // else if ( TAG_EXISTS(railway, "abandoned") && KEY_EXISTS(bridge) ) {
  //   FOR_ALL_TAGS
  //     if (strcmp (*keyp, "railway") == 0) {
  //       // zu "railway = abandoned_bridge" umdefinieren
  //       TAG_WRITE(*keyp, "abandoned_bridge")
  //       // printf ("way/rel railway abandoned + bridge -> abandoned_bridge\n");
  //     }
  //     else {
  //       TAG_WRITE(*keyp, *valp)
  //     }
  //   END_FOR_ALL_TAGS
  // }

  // RADFUSSWEG (benutzungspflichtig) gemaess Vorschlag von CHKR (um highway=footway erweitert 2014/10):
  // "highway=cycleway foot=yes|designated|official"
  // "higway=path bicycle=official|designated foot=designated|official"
  // "highway=footway bicycle=official|designated"
  // RADFUSSWEG (nicht benutzungspflichtig, z.B. Wege in Parks) gemaess Vorschlag von CHKR:
  // "highway=path foot=yes bicycle=yes"
  // "highway=footway bicycle=yes"
  else if ( TAG_EXISTS(highway, "cycleway") && (TAG_EXISTS(foot, "foot_yes") || TAG_EXISTS(foot, "foot_designated") || TAG_EXISTS(foot, "foot_official")) && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radfussweg_tunnel" umdefinieren
        TAG_WRITE(*keyp, "radfussweg_tunnel")
        // printf ("way/rel highway=cycleway foot=yes|designated|official + tunnel -> radfussweg_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "cycleway") && (TAG_EXISTS(foot, "foot_yes") || TAG_EXISTS(foot, "foot_designated") || TAG_EXISTS(foot, "foot_official")) && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radfussweg_bridge" umdefinieren
        TAG_WRITE(*keyp, "radfussweg_bridge")
        // printf ("way/rel highway=cycleway foot=yes|designated|official + bridge -> radfussweg_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "cycleway") && (TAG_EXISTS(foot, "foot_yes") || TAG_EXISTS(foot, "foot_designated") || TAG_EXISTS(foot, "foot_official")) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radfussweg" umdefinieren
        TAG_WRITE(*keyp, "radfussweg")
        // printf ("way/rel highway=cycleway foot=yes|designated|official -> radfussweg\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && (TAG_EXISTS(bicycle, "bicycle_designated") || TAG_EXISTS(bicycle, "bicycle_official")) && (TAG_EXISTS(foot, "foot_designated") || TAG_EXISTS(foot, "foot_official")) && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radfussweg_tunnel" umdefinieren
        TAG_WRITE(*keyp, "radfussweg_tunnel")
        // printf ("way/rel higway=path bicycle=official|designated foot=designated|official + tunnel -> radfussweg_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && (TAG_EXISTS(bicycle, "bicycle_designated") || TAG_EXISTS(bicycle, "bicycle_official")) && (TAG_EXISTS(foot, "foot_designated") || TAG_EXISTS(foot, "foot_official")) && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radfussweg_bridge" umdefinieren
        TAG_WRITE(*keyp, "radfussweg_bridge")
        // printf ("way/rel higway=path bicycle=official|designated foot=designated|official + bridge -> radfussweg_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && (TAG_EXISTS(bicycle, "bicycle_designated") || TAG_EXISTS(bicycle, "bicycle_official")) && (TAG_EXISTS(foot, "foot_designated") || TAG_EXISTS(foot, "foot_official")) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radfussweg" umdefinieren
        TAG_WRITE(*keyp, "radfussweg")
        // printf ("way/rel higway=path bicycle=official|designated foot=designated|official -> radfussweg\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(foot, "foot_yes") && TAG_EXISTS(bicycle, "bicycle_yes") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radfussweg_tunnel" umdefinieren
        TAG_WRITE(*keyp, "radfussweg_tunnel")
        // printf ("way/rel highway=path foot=yes bicycle=yes + tunnel-> radfussweg_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(foot, "foot_yes") && TAG_EXISTS(bicycle, "bicycle_yes") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radfussweg_tunnel" umdefinieren
        TAG_WRITE(*keyp, "radfussweg_bridge")
        // printf ("way/rel highway=path foot=yes bicycle=yes + bridge-> radfussweg_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && TAG_EXISTS(foot, "foot_yes") && TAG_EXISTS(bicycle, "bicycle_yes") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radfussweg" umdefinieren
        TAG_WRITE(*keyp, "radfussweg")
        // printf ("way/rel highway=path foot=yes bicycle=yes -> radfussweg\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "footway") && (TAG_EXISTS(bicycle, "bicycle_yes") || TAG_EXISTS(bicycle, "bicycle_designated") || TAG_EXISTS(bicycle, "bicycle_official")) && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radfussweg_tunnel" umdefinieren
        TAG_WRITE(*keyp, "radfussweg_tunnel")
        // printf ("way/rel highway=footway bicycle=yes|designated|official + tunnel -> radfussweg_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "footway") && (TAG_EXISTS(bicycle, "bicycle_yes") || TAG_EXISTS(bicycle, "bicycle_designated") || TAG_EXISTS(bicycle, "bicycle_official")) && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radfussweg_bridge" umdefinieren
        TAG_WRITE(*keyp, "radfussweg_bridge")
        // printf ("way/rel highway=footway bicycle=yes|designated|official + bridge -> radfussweg_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "footway") && (TAG_EXISTS(bicycle, "bicycle_yes") || TAG_EXISTS(bicycle, "bicycle_designated") || TAG_EXISTS(bicycle, "bicycle_official")) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radfussweg" umdefinieren
        TAG_WRITE(*keyp, "radfussweg")
        // printf ("way/rel highway=footway bicycle=yes|designated|official -> radfussweg\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // RADWEG (benutzungspflichtig) gemaess Vorschlag von CHKR:
  // alle verbleibenden "highway=cycleway"
  // "highway=path bicycle=official|designated foot=no"
  else if ( TAG_EXISTS(highway, "cycleway") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radweg_tunnel" umdefinieren
        TAG_WRITE(*keyp, "radweg_tunnel")
        // printf ("way/rel highway=cycleway + tunnel-> radweg_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "cycleway") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radweg_bridge" umdefinieren
        TAG_WRITE(*keyp, "radweg_bridge")
        // printf ("way/rel highway=cycleway + bridge-> radweg_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "cycleway") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radweg" umdefinieren
        TAG_WRITE(*keyp, "radweg")
        // printf ("way/rel highway=cycleway -> radweg\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && (TAG_EXISTS(bicycle, "bicycle_official") || TAG_EXISTS(bicycle, "bicycle_designated")) && TAG_EXISTS(foot, "foot_no") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radweg_tunnel" umdefinieren
        TAG_WRITE(*keyp, "radweg_tunnel")
        // printf ("way/rel highway=path bicycle=official|designated foot=no + tunnel -> radweg_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && (TAG_EXISTS(bicycle, "bicycle_official") || TAG_EXISTS(bicycle, "bicycle_designated")) && TAG_EXISTS(foot, "foot_no") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radweg_bridge" umdefinieren
        TAG_WRITE(*keyp, "radweg_bridge")
        // printf ("way/rel highway=path bicycle=official|designated foot=no + bridge -> radweg_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && (TAG_EXISTS(bicycle, "bicycle_official") || TAG_EXISTS(bicycle, "bicycle_designated")) && TAG_EXISTS(foot, "foot_no") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = radweg" umdefinieren
        TAG_WRITE(*keyp, "radweg")
        // printf ("way/rel highway=path bicycle=official|designated foot=no -> radweg\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // FUSSWEG (benutzungspflichtig) analog RADWEG:
  // alle verbleibenden "highway=footway"
  // "highway=path foot=official|designated bicycle=no"
  else if ( TAG_EXISTS(highway, "footway") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = fussweg_tunnel" umdefinieren
        TAG_WRITE(*keyp, "fussweg_tunnel")
        // printf ("way/rel highway=footway + tunnel -> fussweg_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "footway") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = fussweg_tunnel" umdefinieren
        TAG_WRITE(*keyp, "fussweg_bridge")
        // printf ("way/rel highway=footway + bridge -> fussweg_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "footway") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = fussweg" umdefinieren
        TAG_WRITE(*keyp, "fussweg")
        // printf ("way/rel highway=footway -> fussweg\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && (TAG_EXISTS(foot, "foot_official") || TAG_EXISTS(foot, "foot_designated")) && TAG_EXISTS(bicycle, "bicycle_no") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = fussweg_tunnel" umdefinieren
        TAG_WRITE(*keyp, "fussweg_tunnel")
        // printf ("way/rel highway=path foot=official|designated bicycle=no + tunnel -> fussweg_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && (TAG_EXISTS(foot, "foot_official") || TAG_EXISTS(foot, "foot_designated")) && TAG_EXISTS(bicycle, "bicycle_no") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = fussweg_bridge" umdefinieren
        TAG_WRITE(*keyp, "fussweg_bridge")
        // printf ("way/rel highway=path foot=official|designated bicycle=no + bridge -> fussweg_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && (TAG_EXISTS(foot, "foot_official") || TAG_EXISTS(foot, "foot_designated")) && TAG_EXISTS(bicycle, "bicycle_no") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = fussweg" umdefinieren
        TAG_WRITE(*keyp, "fussweg")
        // printf ("way/rel highway=path foot=official|designated bicycle=no -> fussweg\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = path_tunnel" umdefinieren
        TAG_WRITE(*keyp, "path_tunnel")
        // printf ("way/rel highway=path + tunnel -> path_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = path_bridge" umdefinieren
        TAG_WRITE(*keyp, "path_bridge")
        // printf ("way/rel highway=path + bridge -> path_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // REITWEG - ausgeschilderter Reitweg (nur Reiter erlaubt)
  // "highway=path horse=official|designated"
  // alle verbleibenden "highway=bridleway"
  else if ( TAG_EXISTS(highway, "path") && (TAG_EXISTS(horse, "official") || TAG_EXISTS(horse, "designated")) && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = reitweg_tunnel" umdefinieren
        TAG_WRITE(*keyp, "reitweg_tunnel")
        // printf ("way/rel highway=path horse=official|designated + tunnel -> reitweg_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && (TAG_EXISTS(horse, "official") || TAG_EXISTS(horse, "designated")) && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = reitweg_bridge" umdefinieren
        TAG_WRITE(*keyp, "reitweg_bridge")
        // printf ("way/rel highway=path horse=official|designated + bridge -> reitweg_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "path") && (TAG_EXISTS(horse, "official") || TAG_EXISTS(horse, "designated")) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = reitweg" umdefinieren
        TAG_WRITE(*keyp, "reitweg")
        // printf ("way/rel highway=path horse=official|designated -> reitweg\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "bridleway") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = reitweg_tunnel" umdefinieren
        TAG_WRITE(*keyp, "reitweg_tunnel")
        // printf ("way/rel highway=bridleway + tunnel -> reitweg_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "bridleway") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = reitweg_bridge" umdefinieren
        TAG_WRITE(*keyp, "reitweg_bridge")
        // printf ("way/rel highway=bridleway + bridge -> reitweg_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(highway, "bridleway") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "highway") == 0) {
        // zu "highway = reitweg" umdefinieren
        TAG_WRITE(*keyp, "reitweg")
        // printf ("way/rel highway=bridleway -> reitweg\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(aeroway, "taxiway") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "aeroway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "taxiway_tunnel")
        // printf ("way/rel aeroway taxiway + tunnel -> taxiway_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(aeroway, "taxiway") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "aeroway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "taxiway_bridge")
        // printf ("way/rel aeroway taxiway + bridge -> taxiway_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(aeroway, "runway") && KEY_EXISTS(tunnel) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "aeroway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "runway_tunnel")
        // printf ("way/rel aeroway runway + tunnel -> runway_tunnel\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  else if ( TAG_EXISTS(aeroway, "runway") && KEY_EXISTS(bridge) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "aeroway") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "runway_bridge")
        // printf ("way/rel aeroway runway + bridge -> runway_bridge\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Nationalparks (siehe auch Sonderbehandlung für Nationalparkgrenzen (type=boundary && boundary=national_park))
  else if ( TAG_EXISTS(boundary, "national_park") ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // schutzgebiet-Tag (key/value) erzeugen
    TAG_WRITE("schutzgebiet", "national_park")
    // printf ("way/rel boundary=national_park -> schutzgebiet=national_park\n");
  }
  else if ( TAG_EXISTS(boundary, "protected_area") && TAG_EXISTS(protect_class, "2") ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // schutzgebiet-Tag (key/value) erzeugen
    TAG_WRITE("schutzgebiet", "national_park")
    // printf ("way/rel boundary=protected_area + protect_class=2 -> schutzgebiet=national_park\n");
  }

  // Naturschutzgebiete mit Betretungsverbot (z.B. Naturwaldreservate; eher kleinere Gebiete)
  else if ( TAG_EXISTS(boundary, "protected_area") && (TAG_EXISTS(protect_class, "1") || TAG_EXISTS(protect_class, "1a") || TAG_EXISTS(protect_class, "1b")) ) {
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
    // schutzgebiet-Tag (key/value) erzeugen
    TAG_WRITE("schutzgebiet", "strict_nature_reserve")
    // printf ("way/rel boundary=protected_area + protect_class=1|1a|1b -> schutzgebiet=strict_nature_reserve\n");
  }

  // Private Parkplaetze (access=private oder access=no)
  else if ( TAG_EXISTS(amenity, "parking") && (TAG_EXISTS(access, "access_private") || TAG_EXISTS(access, "access_no")) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "parken_privat")
        // printf ("way/rel amenity=parking && access=private|no -> amenity=parken_privat\n");
      }
      else if (strcmp (*keyp, "name") == 0) {
        TAG_WRITE(*keyp, *valp)
        // printf ("name = %s\n", *valp);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Private Parkflaechen (access=private oder access=no)
  else if ( TAG_EXISTS(amenity, "parking_space") && (TAG_EXISTS(access, "access_private") || TAG_EXISTS(access, "access_no")) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "parkflaeche_privat")
        // printf ("way/rel amenity=parking_space && access=private|no -> amenity=parkflaeche_privat\n");
      }
      else if (strcmp (*keyp, "name") == 0) {
        TAG_WRITE(*keyp, *valp)
        // printf ("name = %s\n", *valp);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Oeffentliche Parkplaetze die zusaetzlich mit access=yes getaggt sind
  else if ( TAG_EXISTS(amenity, "parking") && ( TAG_EXISTS(access, "access_yes") || TAG_EXISTS(access, "access_public") ) ) {
    FOR_ALL_TAGS
      // Objekt unveraendert uebernehmen
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Oeffentliche Parkflaechen die zusaetzlich mit access=yes getaggt sind
  else if ( TAG_EXISTS(amenity, "parking_space") && ( TAG_EXISTS(access, "access_yes") || TAG_EXISTS(access, "access_public") ) ) {
    FOR_ALL_TAGS
      // Objekt unveraendert uebernehmen
      TAG_WRITE(*keyp, *valp)
    END_FOR_ALL_TAGS
  }

  // Parkplaetze mit eingeschraenkter Nutzung (access=*) (s.a. vorherige Regel die 'access=yes/public' filtert)
  else if ( TAG_EXISTS(amenity, "parking") && KEY_EXISTS(access) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "parken_eingeschraenkt")
        // printf ("way/rel amenity=parking && access=* -> amenity=parken_eingeschraenkt\n");
      }
      else if (strcmp (*keyp, "name") == 0) {
        TAG_WRITE(*keyp, *valp)
        // printf ("name = %s\n", *valp);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Parkflaechen mit eingeschraenkter Nutzung (access=*) (s.a. vorherige Regel die 'access=yes/public' filtert)
  else if ( TAG_EXISTS(amenity, "parking_space") && KEY_EXISTS(access) ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "amenity") == 0) {
        // eigenes Objekt erzeugen
        TAG_WRITE(*keyp, "parkflaeche_eingeschraenkt")
        // printf ("way/rel amenity=parking_space && access=* -> amenity=parkflaeche_eingeschraenkt\n");
      }
      else if (strcmp (*keyp, "name") == 0) {
        TAG_WRITE(*keyp, *valp)
        // printf ("name = %s\n", *valp);
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }

  // Private Schwimmbecken / Swimming-Pools umbenennen
  else if ( (TAG_EXISTS(leisure, "swimming_pool") || TAG_EXISTS(amenity, "swimming_pool") ) && TAG_EXISTS(access, "access_private") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "leisure") == 0) {
         TAG_WRITE(*keyp, "swimming_pool_private")
         // printf ("way/rel leisure=swimming_pool && access=private -> leisure=swimming_pool_private\n", fzk_value);
      }
      else if (strcmp (*keyp, "amenity") == 0) {
         TAG_WRITE(*keyp, "swimming_pool_private")
         // printf ("way/rel amenity=swimming_pool && access=private -> amenity=swimming_pool_private\n", fzk_value);
      }
      else if (strcmp (*keyp, "sport") == 0) {
         // Tag entfernen
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
     END_FOR_ALL_TAGS
  }

  // Parzellen in Kleingärten
  else if ( TAG_EXISTS(landuse, "allotments") && TAG_EXISTS(allotments, "plot") ) {
    FOR_ALL_TAGS
      if (strcmp (*keyp, "landuse") == 0) {
        TAG_WRITE(*keyp, "allotments_plot")
        // printf ("way/rel landuse=allotments + allotments=plot -> landuse=allotments_plot\n");
      }
      else {
        TAG_WRITE(*keyp, *valp)
      }
    END_FOR_ALL_TAGS
  }
  //------------------------------------------------------------
  // hier weitere Regeln gemaess folgendem Schema einfuegen:
  // else if ( ... ) {
  //   ...
  // }
  //------------------------------------------------------------
  else {
    // keine der vorstehenden, speziellen Bearbeitungsregeln hat gezogen
    FOR_ALL_TAGS
      TAG_WRITE(*keyp, *valp)
      // printf ("Ohne Bearbeitungsregel: %s = %s\n", *keyp, *valp);
    END_FOR_ALL_TAGS
    // printf ("\n");
  }
}
