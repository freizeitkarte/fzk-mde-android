-- Create additional indexes
-- Index on way tags for export - not neccessay because osmois export everything first
-- CREATE INDEX idx_ways_tags ON ways USING GIN(tags);
-- Index on relation tags for route function
CREATE INDEX idx_relations_tags ON relations USING GIN(tags);

-- Optimize
VACUUM ANALYZE;

-- Run relation_cleanup
SELECT relation_cleanup();

-- Run route_hiking
SELECT route_hiking();

-- Run route_mtb
SELECT route_mtb();

-- Run route_bicycle
SELECT route_bicycle();

-- Run route_inline
SELECT route_inline();

-- Run route_horse
SELECT route_horse();

-- Run route_canoe
SELECT route_canoe();

-- Run route_motorboat
SELECT route_motorboat();

-- Run node_network_hiking
SELECT node_network_hiking();

-- Run node_network_bicycle
SELECT node_network_bicycle();

-- Run node_network_inline
SELECT node_network_inline();

-- Run node_network_horse
SELECT node_network_horse();

-- Run node_network_canoe
SELECT node_network_canoe();

-- Run node_network_motorboat
SELECT node_network_motorboat();

-- Run piste_downhill
SELECT piste_downhill();

-- Run piste_nordic
SELECT piste_nordic();

-- Run piste_sled
SELECT piste_sled();
