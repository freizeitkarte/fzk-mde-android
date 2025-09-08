//------------------------------------------------------------
// Freizeitkarte-PlugIn fuer das Programm "osmfilter" von Markus Weber.
//
// Version : 1.0.0 - 02.02.2014
// Version : 1.1.0 - 24.05.2014: Tags intermittent und disused ergaenzt
// Version : 1.2.0 - 24.05.2014: Tags ruins ergaenzt
// Version : 1.3.0 - 03.10.2014: Tags zoo ergaenzt
// Version : 1.4.0 - 01.01.2015: Tags service ergaenzt
// Version : 1.5.0 - 12.02.2016: Puffer fuer tagtransform eingefuehrt
// Version : 1.6.0 - 24.02.2016: Tag station ergaenzt
// Version : 1.7.0 - 31.03.2016: Tags layer, type ergaenzt
// Version : 1.8.0 - 26.04.2016: Tag surface ergaenzt
// Version : 1.9.0 - 20.06.2016: Tag car ergaenzt
// Version : 1.10.0 - 09.11.2016: Tag information ergaenzt
// Version : 1.11.0 - 27.12.2016: Tag mtb_scale und mtb_scale_uphill ergaenzt
// Version : 17.09 - 31.08.2017: Tags trail_visibility und emergency ergaenzt
// Version : 17.12 - 18.11.2017: Tags allotments und attraction ergänzt, Puffer für sport-Tags ergänzt
// Version : 18.03 - 24.02.2018: Tags cycleway* ergänzt
// Version : 19.03 - 01.01.2019: Tag tunnel_name ergänzt
// Version : 19.09 - 19.08.2019: protect_id entfernt
// Version : 19.12 - 22.09.2019: Tags für node_network, charging_station ergänzt
// Version : 20.09 - 08.08.2020: Tags für ford und Puffer ergänzt
// Version : 20.12 - 04.09.2020: Tag 'network_type' entfernt
//
// Autor   : Klaus Tockloth
//
// Copyright (C) 2013-2019 FZK-Projekt <freizeitkarte@googlemail.com>
//------------------------------------------------------------

//------------------------------------------------------------
// im nachfolgenden Makro-Bereich bitte nichts aendern
//------------------------------------------------------------

#define TAG_DEFINE(keyname) \
static char *keyp_##keyname = NULL; static char *valp_##keyname = NULL;

#define TAG_INIT(keyname) \
keyp_##keyname = NULL; valp_##keyname = NULL;

#define TAG_STORE(keyname, keystring) \
(strcmp (*keyp, keystring) == 0) { keyp_##keyname = *keyp; valp_##keyname = *valp; fzk_found = 1; }

#define KEY_EXISTS(keyname) \
(keyp_##keyname != NULL)

#define KEY_NOT_EXISTS(keyname) \
(keyp_##keyname == NULL)

#define TAG_EXISTS(keyname, valuestring) \
( (keyp_##keyname != NULL) && (strcmp (valp_##keyname, valuestring) == 0) )

#define FOR_ALL_TAGS \
keyp = key; valp = val; while (keyp < keye) {

#define END_FOR_ALL_TAGS \
keyp++; valp++; }

#define TAG_WRITE(keypointer, valuepointer) \
if (!fil_activeo[otype] || fil_check2(otype,keypointer,valuepointer)) { wo_keyval(keypointer,valuepointer); }

//------------------------------------------------------------
// TAGs die im Rahmen einer Transformation benoetigt werden
//------------------------------------------------------------

TAG_DEFINE(abandoned)
TAG_DEFINE(access)
TAG_DEFINE(addr_housenumber)
TAG_DEFINE(addr_street)
TAG_DEFINE(admin_level)
TAG_DEFINE(aerialway)
TAG_DEFINE(aeroway)
TAG_DEFINE(allotments)
TAG_DEFINE(amenity)
TAG_DEFINE(atm)
TAG_DEFINE(attraction)
TAG_DEFINE(bBoxWeight)
TAG_DEFINE(bicycle)
TAG_DEFINE(boundary)
TAG_DEFINE(brand)
TAG_DEFINE(bridge)
TAG_DEFINE(building)
TAG_DEFINE(bus)
TAG_DEFINE(capacity)
TAG_DEFINE(car)
TAG_DEFINE(cemetery)
TAG_DEFINE(contour)
TAG_DEFINE(cuisine)
TAG_DEFINE(cycleway)
TAG_DEFINE(cycleway_left)
TAG_DEFINE(cycleway_right)
TAG_DEFINE(denotation)
TAG_DEFINE(disused)
TAG_DEFINE(ele)
TAG_DEFINE(emergency)
TAG_DEFINE(foot)
TAG_DEFINE(ford)
TAG_DEFINE(generator_source)
TAG_DEFINE(geological)
TAG_DEFINE(highway)
TAG_DEFINE(historic)
TAG_DEFINE(horse)
TAG_DEFINE(icn_ref)
TAG_DEFINE(indoor)
TAG_DEFINE(information)
TAG_DEFINE(intermittent)
TAG_DEFINE(iwn_ref)
TAG_DEFINE(landuse)
TAG_DEFINE(layer)
TAG_DEFINE(lcn_ref)
TAG_DEFINE(leisure)
TAG_DEFINE(lwn_ref)
TAG_DEFINE(man_made)
TAG_DEFINE(memorial_type)
TAG_DEFINE(military)
TAG_DEFINE(motor_vehicle)
TAG_DEFINE(motorcar)
TAG_DEFINE(mountain_pass)
TAG_DEFINE(mtb_scale)
TAG_DEFINE(mtb_scale_uphill)
TAG_DEFINE(name)
TAG_DEFINE(natural)
TAG_DEFINE(ncn_ref)
TAG_DEFINE(network)
TAG_DEFINE(nwn_ref)
TAG_DEFINE(office)
TAG_DEFINE(operator)
TAG_DEFINE(place)
TAG_DEFINE(population)
TAG_DEFINE(power)
TAG_DEFINE(power_source)
TAG_DEFINE(protect_class)
TAG_DEFINE(public_transport)
TAG_DEFINE(railway)
TAG_DEFINE(rcn_ref)
TAG_DEFINE(ref)
TAG_DEFINE(religion)
TAG_DEFINE(rhn_ref)
TAG_DEFINE(rin_ref)
TAG_DEFINE(rmn_ref)
TAG_DEFINE(rpn_ref)
TAG_DEFINE(ruins)
TAG_DEFINE(rwn_ref)
TAG_DEFINE(sac_scale)
TAG_DEFINE(service)
TAG_DEFINE(shelter_type)
TAG_DEFINE(shop)
TAG_DEFINE(sport)
TAG_DEFINE(stars)
TAG_DEFINE(station)
TAG_DEFINE(surface)
TAG_DEFINE(tourism)
TAG_DEFINE(tower_type)
TAG_DEFINE(tracktype)
TAG_DEFINE(trail_visibility)
TAG_DEFINE(train)
TAG_DEFINE(tram)
TAG_DEFINE(tunnel)
TAG_DEFINE(tunnel_name)
TAG_DEFINE(type)
TAG_DEFINE(vehicle)
TAG_DEFINE(via_ferrata_scale)
TAG_DEFINE(waterway)
TAG_DEFINE(website)
TAG_DEFINE(wetland)
TAG_DEFINE(wikipedia)
TAG_DEFINE(wikipedia_de)
TAG_DEFINE(zoo)
// hier weitere TAGs definieren

//------------------------------------------------------------
// die nachfolgende Sektion bitte nicht veraendern
//------------------------------------------------------------

// fzk buffer
static char fzk_key [128];
static char fzk_value [512];
static char fzk_temp [512];

// fzk merker
static int fzk_found = 0;

// fzk tag transform
static char fzk_access [512];
static char fzk_amenity [512];
static char fzk_bicycle [512];
static char fzk_car [512];
static char fzk_foot [512];
static char fzk_ford [512];
static char fzk_highway [512];
static char fzk_motor_vehicle [512];
static char fzk_motorcar [512];
static char fzk_mtb_scale [512];
static char fzk_mtb_scale_uphill [512];
static char fzk_sport [512];
static char fzk_surface [512];
static char fzk_trail_visibility [512];
static char fzk_vehicle [512];
