-- Create route_hiking function
CREATE OR REPLACE FUNCTION route_hiking() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'routehiking';
  network text;
BEGIN
  -- lwn local hiking network
  network := 'lwn';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='hiking' OR tags->'route'='foot') AND (tags->'network'='lwn' OR NOT tags ? 'network') AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'local hiking network finished';

  -- rwn regional hiking network
  network := 'rwn';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='hiking' OR tags->'route'='foot') AND tags->'network'='rwn' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'regional hiking network finished';
  
  -- nwn national hiking network
  network := 'nwn';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='hiking' OR tags->'route'='foot') AND tags->'network'='nwn' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'national hiking network finished';
  
  -- iwn international hiking network
  network := 'iwn';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='hiking' OR tags->'route'='foot') AND tags->'network'='iwn' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'international hiking network finished';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 100000000000;
    INSERT INTO ways 
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes 
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New hiking ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION route_hiking() IS 'Add tags from hiking relations';


-- Create route_mtb function
CREATE OR REPLACE FUNCTION route_mtb() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'routemtb';
  network text;
BEGIN
  -- mountain bike network
  network := 'mtb';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='mtb' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'mountain bike network finished';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 110000000000;
    INSERT INTO ways 
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes 
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New mtb ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION route_mtb() IS 'Add tags from mtb relations';


-- Create route_bicycle function
CREATE OR REPLACE FUNCTION route_bicycle() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'routebicycle';
  network text;
BEGIN
  -- lcn local bicycle network
  network := 'lcn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='bicycle' AND (tags->'network'='lcn' OR NOT tags ? 'network') AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'local bicycle network finished';

  -- rcn regional bicycle network
  network := 'rcn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='bicycle' AND tags->'network'='rcn' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'regional bicycle network finished';
  
  -- ncn national bicycle network
  network := 'ncn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='bicycle' AND tags->'network'='ncn' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'national bicycle network finished';
  
  -- icn international bicycle network
  network := 'icn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='bicycle' AND tags->'network'='icn' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'international bicycle network finished';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 120000000000;
    INSERT INTO ways 
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes 
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New bicycle ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION route_bicycle() IS 'Add tags from bicycle relations';


-- Create route_inline function
CREATE OR REPLACE FUNCTION route_inline() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'routeinline';
  network text;
BEGIN
  -- lcn local inline_skates network
  network := 'lin';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='inline_skates' AND (tags->'network'='local' OR tags->'network'='lin' OR NOT tags ? 'network') AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'local inline_skates network finished';

  -- rcn regional inline_skates network
  network := 'rin';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='inline_skates' AND (tags->'network'='regional' OR tags->'network'='rin') AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'regional inline_skates network finished';
  
  -- ncn national inline_skates network
  network := 'nin';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='inline_skates' AND (tags->'network'='national' OR tags->'network'='nin') AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'national inline_skates network finished';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 130000000000;
    INSERT INTO ways 
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes 
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New inline_skates ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION route_inline() IS 'Add tags from inline_skates relations';


-- Create route_horse function
CREATE OR REPLACE FUNCTION route_horse() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'routehorse';
  network text;
BEGIN
  -- lhn local horse network
  network := 'lhn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='horse' AND (tags->'network'='lhn' OR NOT tags ? 'network') AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'local horse network finished';

  -- rhn regional horse network
  network := 'rhn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='horse' AND tags->'network'='rhn' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'regional horse network finished';
  
  -- nhn national horse network
  network := 'nhn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='horse' AND tags->'network'='nhn' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'national horse network finished';
  
  -- ihn international horse network
  network := 'ihn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='horse' AND tags->'network'='ihn' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'international horse network finished';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 140000000000;
    INSERT INTO ways 
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes 
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New horse ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION route_horse() IS 'Add tags from horse relations';


-- Create route_canoe function
CREATE OR REPLACE FUNCTION route_canoe() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'routecanoe';
  network text;
BEGIN
  -- lpn local canoe network
  network := 'lpn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='canoe' AND (tags->'network'='lpn' OR NOT tags ? 'network') AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'local canoe network finished';

  -- rpn regional canoe network
  network := 'rpn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='canoe' AND tags->'network'='rpn' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'regional canoe network finished';
  
  -- npn national canoe network
  network := 'nhn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='canoe' AND tags->'network'='npn' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'national canoe network finished';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 150000000000;
    INSERT INTO ways 
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes 
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New canoe ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION route_canoe() IS 'Add tags from canoe relations';


-- Create route_motorboat function
CREATE OR REPLACE FUNCTION route_motorboat() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'routemotorboat';
  network text;
BEGIN
  -- lmn local motorboat network
  network := 'lmn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='motorboat' AND (tags->'network'='lmn' OR NOT tags ? 'network') AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'local motorboat network finished';

  -- rmn regional motorboat network
  network := 'rmn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='motorboat' AND tags->'network'='rmn' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'regional motorboat network finished';
  
  -- nmn national motorboat network
  network := 'nmn';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='motorboat' AND tags->'network'='nmn' AND NOT (tags@>'network:type=>node_network') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'national motorboat network finished';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 160000000000;
    INSERT INTO ways 
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes 
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New motorboat ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION route_motorboat() IS 'Add tags from motorboat relations';


-- Create node_network_hiking function
CREATE OR REPLACE FUNCTION node_network_hiking() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'node_network_hiking';
  network text;
BEGIN
  -- all node_network hiking networks
  network := 'nn_hiking';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='hiking' OR tags->'route'='foot') AND tags->'network:type'='node_network' LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'node_network hiking finished';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 170000000000;
    INSERT INTO ways 
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes 
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New node_network hiking ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION node_network_hiking() IS 'Add tags from node_network hiking relations';


-- Create node_network_bicycle function
CREATE OR REPLACE FUNCTION node_network_bicycle() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'node_network_bicycle';
  network text;
BEGIN
  -- all node_network bicycle networks
  network := 'nn_bicycle';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='bicycle' OR tags->'route'='mtb') AND tags->'network:type'='node_network' LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'node_network bicycle finished';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 180000000000;
    INSERT INTO ways 
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes 
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New node_network bicycle ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION node_network_bicycle() IS 'Add tags from node_network bicycle/mtb relations';


-- Create node_network_inline function
CREATE OR REPLACE FUNCTION node_network_inline() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'node_network_inline_skates';
  network text;
BEGIN
  -- all node_network hiking networks
  network := 'nn_inline_skates';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='inline_skates' AND tags->'network:type'='node_network' LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'node_network inline_skates finished';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 190000000000;
    INSERT INTO ways 
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes 
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New node_network inline_skates ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION node_network_inline() IS 'Add tags from node_network inline_skates relations';


-- Create node_network_horse function
CREATE OR REPLACE FUNCTION node_network_horse() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'node_network_horse';
  network text;
BEGIN
  -- all node_network horse networks
  network := 'nn_horse';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='horse' AND tags->'network:type'='node_network' LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'node_network horse finished';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 200000000000;
    INSERT INTO ways 
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes 
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New node_network horse ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION node_network_horse() IS 'Add tags from node_network horse relations';


-- Create node_network_canoe function
CREATE OR REPLACE FUNCTION node_network_canoe() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'node_network_canoe';
  network text;
BEGIN
  -- all node_network canoe networks
  network := 'nn_canoe';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='canoe' AND tags->'network:type'='node_network' LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'node_network canoe finished';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 210000000000;
    INSERT INTO ways 
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes 
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New node_network canoe ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION node_network_canoe() IS 'Add tags from node_network canoe relations';


-- Create node_network_motorboat function
CREATE OR REPLACE FUNCTION node_network_motorboat() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'node_network_motorboat';
  network text;
BEGIN
  -- all node_network motorboat networks
  network := 'nn_motorboat';
  FOR crel IN SELECT id, tags FROM relations WHERE tags->'route'='motorboat' AND tags->'network:type'='node_network' LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'node_network motorboat finished';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 220000000000;
    INSERT INTO ways 
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes 
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New node_network motorboat ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION node_network_motorboat() IS 'Add tags from node_network motorboat relations';


-- Create piste_downhill function
CREATE OR REPLACE FUNCTION piste_downhill() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'pistedownhill';
  network text;
BEGIN
  -- expert and advanced difficulty downhill piste
  network := 'advanced';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='piste' OR tags->'route'='ski') AND tags->'piste:type'='downhill' AND NOT (tags@>'area=>yes') AND (tags->'piste:difficulty'='expert' OR tags->'piste:difficulty'='advanced') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'expert and advanced difficulty downhill piste finished';

  -- intermediate difficulty downhill piste
  network := 'intermediate';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='piste' OR tags->'route'='ski') AND tags->'piste:type'='downhill' AND NOT (tags@>'area=>yes') AND tags->'piste:difficulty'='intermediate' LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'intermediate difficulty downhill piste finished';
  
  -- easy difficulty downhill piste
  network := 'easy';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='piste' OR tags->'route'='ski') AND tags->'piste:type'='downhill' AND NOT (tags@>'area=>yes') AND (tags->'piste:difficulty'='easy' OR NOT tags ? 'piste:difficulty') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'easy difficulty downhill piste finished';
  
  -- novice difficulty downhill piste
  network := 'novice';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='piste' OR tags->'route'='ski') AND tags->'piste:type'='downhill' AND NOT (tags@>'area=>yes')AND tags->'piste:difficulty'='novice' LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'novice difficulty downhill piste finished';
  
  -- tagging piste which are not part of a relation
  UPDATE ways
    SET tags = hstore(ways.tags || merge_piste_tags(ways.tags, ''::hstore, tagprefix, 'advanced'))
    WHERE tags->'piste:type'='downhill' AND NOT tags ? tagprefix AND NOT (tags@>'area=>yes') AND (tags->'piste:difficulty'='expert' OR tags->'piste:difficulty'='advanced');
  UPDATE ways
    SET tags = hstore(ways.tags || merge_piste_tags(ways.tags, ''::hstore, tagprefix, 'intermediate'))
    WHERE tags->'piste:type'='downhill' AND NOT tags ? tagprefix AND NOT (tags@>'area=>yes') AND tags->'piste:difficulty'='intermediate';
  UPDATE ways
    SET tags = hstore(ways.tags || merge_piste_tags(ways.tags, ''::hstore, tagprefix, 'easy'))
    WHERE tags->'piste:type'='downhill' AND NOT tags ? tagprefix AND NOT (tags@>'area=>yes') AND (tags->'piste:difficulty'='easy' OR NOT tags ? 'piste:difficulty');
  UPDATE ways
    SET tags = hstore(ways.tags || merge_piste_tags(ways.tags, ''::hstore, tagprefix, 'novice'))
    WHERE tags->'piste:type'='downhill' AND NOT tags ? tagprefix AND NOT (tags@>'area=>yes') AND tags->'piste:difficulty'='novice';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 230000000000;
    INSERT INTO ways
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New downhill piste ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION piste_downhill() IS 'Add tags from piste downhill relations';


-- Create piste_nordic function
CREATE OR REPLACE FUNCTION piste_nordic() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'pistenordic';
  network text;
BEGIN
  -- expert and advanced difficulty nordic piste
  network := 'advanced';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='piste' OR tags->'route'='ski') AND tags->'piste:type'='nordic' AND NOT (tags@>'area=>yes') AND (tags->'piste:difficulty'='expert' OR tags->'piste:difficulty'='advanced') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'expert and advanced difficulty nordic piste finished';

  -- intermediate difficulty nordic piste
  network := 'intermediate';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='piste' OR tags->'route'='ski') AND tags->'piste:type'='nordic' AND NOT (tags@>'area=>yes') AND tags->'piste:difficulty'='intermediate' LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'intermediate difficulty nordic piste finished';
  
  -- easy difficulty nordic piste
  network := 'easy';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='piste' OR tags->'route'='ski') AND tags->'piste:type'='nordic' AND NOT (tags@>'area=>yes') AND (tags->'piste:difficulty'='easy' OR NOT tags ? 'piste:difficulty') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'easy difficulty nordic piste finished';
  
  -- novice difficulty nordic piste
  network := 'novice';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='piste' OR tags->'route'='ski') AND tags->'piste:type'='nordic' AND NOT (tags@>'area=>yes') AND tags->'piste:difficulty'='novice' LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'novice difficulty nordic piste finished';
  
  -- tagging piste which are not part of a relation
  UPDATE ways
    SET tags = hstore(ways.tags || merge_piste_tags(ways.tags, ''::hstore, tagprefix, 'advanced'))
    WHERE tags->'piste:type'='nordic' AND NOT tags ? tagprefix AND NOT (tags@>'area=>yes') AND (tags->'piste:difficulty'='expert' OR tags->'piste:difficulty'='advanced');
  UPDATE ways
    SET tags = hstore(ways.tags || merge_piste_tags(ways.tags, ''::hstore, tagprefix, 'intermediate'))
    WHERE tags->'piste:type'='nordic' AND NOT tags ? tagprefix AND NOT (tags@>'area=>yes') AND tags->'piste:difficulty'='intermediate';
  UPDATE ways
    SET tags = hstore(ways.tags || merge_piste_tags(ways.tags, ''::hstore, tagprefix, 'easy'))
    WHERE tags->'piste:type'='nordic' AND NOT tags ? tagprefix AND NOT (tags@>'area=>yes') AND (tags->'piste:difficulty'='easy' OR NOT tags ? 'piste:difficulty');
  UPDATE ways
    SET tags = hstore(ways.tags || merge_piste_tags(ways.tags, ''::hstore, tagprefix, 'novice'))
    WHERE tags->'piste:type'='nordic' AND NOT tags ? tagprefix AND NOT (tags@>'area=>yes') AND tags->'piste:difficulty'='novice';
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 240000000000;
    INSERT INTO ways
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New nordic piste ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION piste_nordic() IS 'Add tags from piste nordic relations';


-- Create piste_sled function
CREATE OR REPLACE FUNCTION piste_sled() RETURNS float4 AS $$
DECLARE
  crel RECORD;
  relcount float4 := 0;
  tagprefix text := 'pistesled';
  network text;
BEGIN
  -- advanced difficulty sled piste
  network := 'advanced';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='piste' OR tags->'route'='ski') AND tags->'piste:type'='sled' AND NOT (tags@>'area=>yes') AND tags->'piste:difficulty'='advanced' LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'advanced difficulty sled piste finished';

  -- intermediate difficulty sled piste
  network := 'intermediate';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='piste' OR tags->'route'='ski') AND tags->'piste:type'='sled' AND NOT (tags@>'area=>yes') AND tags->'piste:difficulty'='intermediate' LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'intermediate difficulty sled piste finished';
  
  -- easy difficulty sled piste
  network := 'easy';
  FOR crel IN SELECT id, tags FROM relations WHERE (tags->'route'='piste' OR tags->'route'='ski') AND tags->'piste:type'='sled' AND NOT (tags@>'area=>yes') AND (tags->'piste:difficulty'='easy' OR NOT tags ? 'piste:difficulty') LOOP
    relcount := relcount + 1;
    PERFORM tagging_ways(crel.id, crel.tags, tagprefix, network);
  END LOOP;
  RAISE NOTICE 'easy difficulty sled piste finished';
  
  -- tagging piste which are not part of a relation
  UPDATE ways
    SET tags = hstore(ways.tags || merge_piste_tags(ways.tags, ''::hstore, tagprefix, 'advanced'))
    WHERE tags->'piste:type'='sled' AND NOT tags ? tagprefix AND NOT (tags@>'area=>yes') AND tags->'piste:difficulty'='advanced';
  UPDATE ways
    SET tags = hstore(ways.tags || merge_piste_tags(ways.tags, ''::hstore, tagprefix, 'intermediate'))
    WHERE tags->'piste:type'='sled' AND NOT tags ? tagprefix AND NOT (tags@>'area=>yes') AND tags->'piste:difficulty'='intermediate';
  UPDATE ways
    SET tags = hstore(ways.tags || merge_piste_tags(ways.tags, ''::hstore, tagprefix, 'easy'))
    WHERE tags->'piste:type'='sled' AND NOT tags ? tagprefix AND NOT (tags@>'area=>yes') AND (tags->'piste:difficulty'='easy' OR NOT tags ? 'piste:difficulty');
  
  -- create new ways
  CREATE SEQUENCE wayid MINVALUE 250000000000;
    INSERT INTO ways
      SELECT nextval('wayid'), 1, ways.user_id, now(), 1, new_tags(ways.tags, tagprefix), ways.nodes
        FROM ways WHERE ways.tags->tagprefix='yes';
  DROP SEQUENCE wayid;
  RAISE NOTICE 'New sled piste ways created';
  
  RETURN relcount;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION piste_sled() IS 'Add tags from piste sled relations';



-- Create tagging ways function
CREATE OR REPLACE FUNCTION tagging_ways(crel_id bigint, crel_tags hstore, tagprefix text, network text) RETURNS int AS $$
BEGIN
  WITH RECURSIVE WAYCTE (relation_id, member_id, member_type) AS
  (
    SELECT relation_id, member_id, member_type
      FROM relation_members
      WHERE relation_id = crel_id
    UNION ALL
    SELECT d.relation_id, d.member_id, d.member_type
      FROM relation_members AS d
      INNER JOIN WAYCTE ON WAYCTE.member_id = d.relation_id
      WHERE WAYCTE.member_type = 'R'
  )
  UPDATE ways
    SET tags = hstore(ways.tags || merge_tags(ways.tags, crel_tags, tagprefix, network))
    WHERE ways.id IN (SELECT member_id FROM WAYCTE WHERE member_type = 'W');
  RETURN NULL;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION tagging_ways(bigint, hstore, text, text) IS 'Tagging ways with route tags';	  
	  

-- Create merge tags function for current ways
CREATE OR REPLACE FUNCTION merge_tags(ways_tags hstore, crel_tags hstore, tagprefix text, network text) RETURNS hstore AS $$
DECLARE
  tagroute hstore;
  tagnet hstore;
  tagref hstore;
  tagname hstore;
  tagrefkey text;
BEGIN
  tagroute := hstore(tagprefix, 'yes');
  tagnet := hstore(tagprefix || 'net', network);
  -- merge ref if exist without duplicates
  tagrefkey := tagprefix || 'ref';
  CASE
    WHEN (crel_tags->'ref') IS NULL THEN
      tagref := ''::hstore;
    WHEN (ways_tags->tagrefkey) IS NULL THEN
      tagref := hstore(tagrefkey, (crel_tags->'ref'));
    ELSE
      IF (crel_tags->'ref') = ANY(string_to_array((ways_tags->tagrefkey), '/')) THEN
        tagref := ''::hstore;
      ELSE
        tagref := hstore(tagrefkey, (ways_tags->tagrefkey) || '/' || (crel_tags->'ref'));
      END IF;
  END CASE;
  -- replace name if exist
  CASE
    WHEN (crel_tags->'name') IS NULL THEN
      tagname := ''::hstore;
    ELSE
      tagname := hstore(tagprefix || 'name', (crel_tags->'name'));
  END CASE;
  RETURN hstore(tagroute || tagnet || tagref || tagname);
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION merge_tags(hstore, hstore, text, text) IS 'Merge tags for current ways';

-- Create merge tags function for current piste ways
CREATE OR REPLACE FUNCTION merge_piste_tags(ways_tags hstore, crel_tags hstore, tagprefix text, network text) RETURNS hstore AS $$
DECLARE
  tagroute hstore;
  tagnet hstore;
  tagref hstore;
  tagname hstore;
  tagrefkey text;
  tagrefway text;
BEGIN
  tagroute := hstore(tagprefix, 'yes');
  tagnet := hstore(tagprefix || 'net', network);
  -- merge ref if exist without duplicates
  tagrefkey := tagprefix || 'ref';
  CASE
    WHEN (crel_tags->'ref') IS NULL THEN
      tagref := ''::hstore;
      -- merge ref/piste:ref from way if exist without duplicates
      IF defined(ways_tags, 'piste:ref') THEN
        tagrefway := ways_tags->'piste:ref';
      ELSIF defined(ways_tags, 'ref') AND NOT ways_tags ? 'highway' THEN
        tagrefway := ways_tags->'ref';
      END IF;
      IF tagrefway <> '' AND (ways_tags->tagrefkey) IS NULL THEN
        tagref := hstore(tagrefkey, tagrefway);
      ELSIF tagrefway <> '' THEN
        IF NOT ((tagrefway) = ANY(string_to_array((ways_tags->tagrefkey), '/'))) THEN
          tagref := hstore(tagrefkey, (ways_tags->tagrefkey) || '/' || tagrefway);
        END IF;
      END IF;
      -- end merge ef/piste:ref from way
    WHEN (ways_tags->tagrefkey) IS NULL THEN
      tagref := hstore(tagrefkey, (crel_tags->'ref'));
    ELSE
      IF (crel_tags->'ref') = ANY(string_to_array((ways_tags->tagrefkey), '/')) THEN
        tagref := ''::hstore;
      ELSE
        tagref := hstore(tagrefkey, (ways_tags->tagrefkey) || '/' || (crel_tags->'ref'));
      END IF;
  END CASE;
  -- replace name if exist
  CASE
    WHEN (crel_tags->'name') IS NULL THEN
      IF defined(ways_tags, 'piste:name') THEN
        tagname := hstore(tagprefix || 'name', (ways_tags->'piste:name'));
      ELSIF defined(ways_tags, 'name') AND NOT ways_tags ? 'highway' THEN
        tagname := hstore(tagprefix || 'name', (ways_tags->'name'));
      ELSE
        tagname := ''::hstore;
      END IF;
    ELSE
      tagname := hstore(tagprefix || 'name', (crel_tags->'name'));
  END CASE;
  RETURN hstore(tagroute || tagnet || tagref || tagname);
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION merge_piste_tags(hstore, hstore, text, text) IS 'Merge tags for current piste ways';


-- Create new tags function for new ways
CREATE OR REPLACE FUNCTION new_tags(ways_tags hstore, tagprefix text) RETURNS hstore AS $$
DECLARE
  taghighway hstore;
  tagnet hstore;
  tagref hstore;
  tagname hstore;
  tagsize hstore;
  taglayer hstore;
  tagnetkey text;
  tagrefkey text;
  tagnamekey text;
BEGIN
  taghighway := hstore('highway', tagprefix);
  
  tagnetkey := tagprefix || 'net';
  tagnet := hstore('routenetwork', ways_tags->tagnetkey);
  
  tagrefkey := tagprefix || 'ref';
  IF defined(ways_tags, tagrefkey) THEN
    tagref := hstore('ref', ways_tags->tagrefkey);
  ELSE
    tagref := ''::hstore;
  END IF;
  
  tagnamekey := tagprefix || 'name';
  IF defined(ways_tags, tagnamekey) THEN
    tagname := hstore('name', ways_tags->tagnamekey);
  ELSE
    tagname := ''::hstore;
  END IF;
  
  CASE
    WHEN ways_tags->'highway' IN ('motorway','trunk','primary','secondary','tertiary','motorway_link','trunk_link','primary_link','secondary_link','tertiary_link') THEN
      tagsize := 'routesize=>large'::hstore;
    WHEN ways_tags->'highway' IN ('unclassified','residential','road','pedestrian','living_street') THEN
      tagsize := 'routesize=>medium'::hstore;
    ELSE
      tagsize := 'routesize=>small'::hstore;
  END CASE;
  
  IF ways_tags->'layer'~E'^\\d+$' THEN
	taglayer := hstore('layer', ways_tags->'layer');
  ELSE
    taglayer := ''::hstore;
  END IF;
  
  RETURN hstore(taghighway || tagnet || tagref || tagname || tagsize || taglayer);
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION new_tags(hstore, text) IS 'New tags for new ways';


-- Create relation cleanup function
-- This function breaks up cycles, but without knowing the intention of the author
CREATE OR REPLACE FUNCTION relation_cleanup() RETURNS Int AS $$
BEGIN
  WITH RECURSIVE search_graph (relation_id, member_id, path, hasCycle, isCycle) AS (
    SELECT relation_id, member_id, ARRAY[relation_id], FALSE, FALSE
      FROM relation_members g
      WHERE member_type = 'R'
    UNION ALL
    SELECT g.relation_id, g.member_id, path || g.relation_id, g.member_id = ANY(path), g.member_id = path[1]
      FROM relation_members g, search_graph sg
      WHERE g.relation_id = sg.member_id AND NOT hasCycle AND g.member_type = 'R'
  )
  DELETE FROM relation_members 
    WHERE (relation_id, member_id) IN
    (SELECT relation_id, member_id FROM search_graph WHERE isCycle);

  RETURN NULL;
END;
$$ LANGUAGE plpgsql;
COMMENT ON FUNCTION relation_cleanup() IS 'Delete relation cycles';
