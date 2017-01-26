#include <stdio.h>
#include "../../simdebug.h"
#include "../../utils/simstring.h"

#include "../weg_besch.h"
#include "../intro_dates.h"
#include "../../bauer/wegbauer.h"

#include "way_reader.h"
#include "../obj_node_info.h"
#include "../../network/pakset_info.h"


void way_reader_t::register_obj(obj_desc_t *&data)
{
    way_desc_t *desc = static_cast<way_desc_t *>(data);

    way_builder_t::register_desc(desc);
//    printf("...Weg %s geladen\n", desc->get_name());
	obj_for_xref(get_type(), desc->get_name(), data);

	checksum_t *chk = new checksum_t();
	desc->calc_checksum(chk);
	pakset_info_t::append(desc->get_name(), chk);
}


bool way_reader_t::successfully_loaded() const
{
    return way_builder_t::successfully_loaded();
}


obj_desc_t * way_reader_t::read_node(FILE *fp, obj_node_info_t &node)
{
	ALLOCA(char, desc_buf, node.size);

	way_desc_t *desc = new way_desc_t();
	// DBG_DEBUG("way_reader_t::read_node()", "node size = %d", node.size);

	// Hajo: Read data
	fread(desc_buf, node.size, 1, fp);
	char * p = desc_buf;

	// Hajo: old versions of PAK files have no version stamp.
	// But we know, the higher most bit was always cleared.
	int version = 0;

	if(node.size == 0) {
		// old node, version 0, compatibility code
		desc->cost = 10000;
		desc->maintenance = 800;
		desc->topspeed = 999;
		desc->axle_load = 9999;
		desc->intro_date = DEFAULT_INTRO_DATE*12;
		desc->obsolete_date = DEFAULT_RETIRE_DATE*12;
		desc->wt = road_wt;
		desc->styp = type_flat;
		desc->draw_as_obj = false;
		desc->number_seasons = 0;
	}
	else {

		const uint16 v = decode_uint16(p);
		version = v & 0x7FFF;

		// Whether the read file is from Simutrans-Experimental
		//@author: jamespetts

		way_constraints_of_way_t way_constraints;
		const bool experimental = version > 0 ? v & EXP_VER : false;
		uint16 experimental_version = 0;
		if(experimental)
		{
			// Experimental version to start at 0 and increment.
			version = version & EXP_VER ? version & 0x3FFF : 0;
			while(version > 0x100)
			{
				version -= 0x100;
				experimental_version ++;
			}
			experimental_version -=1;
		}

		if(version==6) {
			// version 6, now with axle load
			desc->cost = decode_uint32(p);
			desc->maintenance = decode_uint32(p);
			desc->topspeed = decode_uint32(p);
			desc->intro_date = decode_uint16(p);
			desc->obsolete_date = decode_uint16(p);
			desc->axle_load = decode_uint16(p);	// new
			desc->wt = decode_uint8(p);
			desc->styp = decode_uint8(p);
			desc->draw_as_obj = decode_uint8(p);
			desc->number_seasons = decode_sint8(p);
			if(experimental)
			{
				way_constraints.set_permissive(decode_uint8(p));
				way_constraints.set_prohibitive(decode_uint8(p));
				if(experimental_version >= 1)
				{
					desc->topspeed_gradient_1 = decode_sint32(p);
					desc->topspeed_gradient_2 = decode_sint32(p);
					desc->max_altitude = decode_sint8(p);
					desc->max_vehicles_on_tile = decode_uint8(p);
					desc->wear_capacity = decode_uint32(p);
					desc->way_only_cost = decode_uint32(p);
					desc->upgrade_group = decode_uint8(p);
					desc->monthly_base_wear = decode_uint32(p); 
				}
				if(experimental_version > 1)
				{
					dbg->fatal( "way_reader_t::read_node()","Incompatible pak file version for Simutrans-E, number %i", experimental_version );
				}
			}
		}
		else if(version==4  ||  version==5) {
			// Versioned node, version 4+5
			desc->cost = decode_uint32(p);
			desc->maintenance = decode_uint32(p);
			desc->topspeed = decode_sint32(p);
			desc->axle_load = decode_uint32(p);
			desc->intro_date = decode_uint16(p);
			desc->obsolete_date = decode_uint16(p);
			desc->wt = decode_uint8(p);
			desc->styp = decode_uint8(p);
			desc->draw_as_obj = decode_uint8(p);
			desc->number_seasons = decode_sint8(p);
			if(experimental)
			{
				way_constraints.set_permissive(decode_uint8(p));
				way_constraints.set_prohibitive(decode_uint8(p));
				if(experimental_version == 1)
				{
					desc->topspeed_gradient_1 = decode_sint32(p);
					desc->topspeed_gradient_2 = decode_sint32(p);
					desc->max_altitude = decode_sint8(p);
					desc->max_vehicles_on_tile = decode_uint8(p);
					desc->wear_capacity = decode_uint32(p);
					desc->way_only_cost = decode_uint32(p);
					desc->upgrade_group = decode_uint8(p);
				}
				if(experimental_version > 1)
				{
					dbg->fatal("way_reader_t::read_node()","Incompatible pak file version for Simutrans-Ex, number %i", experimental_version);
				}
			}

		}
		else if(version==3) {
			// Versioned node, version 3
			desc->cost = decode_uint32(p);
			desc->maintenance = decode_uint32(p);
			desc->topspeed = decode_uint32(p);
			desc->axle_load = decode_uint32(p);
			desc->intro_date = decode_uint16(p);
			desc->obsolete_date = decode_uint16(p);
			desc->wt = decode_uint8(p);
			desc->styp = decode_uint8(p);
			desc->draw_as_obj = decode_uint8(p);
			desc->number_seasons = 0;
		}
		else if(version==2) {
			// Versioned node, version 2
			desc->cost = decode_uint32(p);
			desc->maintenance = decode_uint32(p);
			desc->topspeed = decode_uint32(p);
			desc->axle_load = decode_uint32(p);
			desc->intro_date = decode_uint16(p);
			desc->obsolete_date = decode_uint16(p);
			desc->wt = decode_uint8(p);
			desc->styp = decode_uint8(p);
			desc->draw_as_obj = false;
			desc->number_seasons = 0;
		}
		else if(version == 1) {
			// Versioned node, version 1
			desc->cost = decode_uint32(p);
			desc->maintenance = decode_uint32(p);
			desc->topspeed = decode_uint32(p);
			desc->axle_load = decode_uint32(p);
			uint32 intro_date= decode_uint32(p);
			desc->intro_date = (intro_date/16)*12 + (intro_date%16);
			desc->wt = decode_uint8(p);
			desc->styp = decode_uint8(p);
			desc->obsolete_date = DEFAULT_RETIRE_DATE*12;
			desc->draw_as_obj = false;
			desc->number_seasons = 0;
		}
		else {
			dbg->fatal("way_reader_t::read_node()","Invalid version %d", version);
		}
		desc->set_way_constraints(way_constraints);
		if(experimental_version < 1 || !experimental)
		{
			desc->topspeed_gradient_1 = desc->topspeed_gradient_2 = desc->topspeed;
			desc->max_altitude = 0;
			desc->max_vehicles_on_tile = 251;
			desc->wear_capacity = desc->get_waytype() == road_wt ? 100000000 : 4000000000;
			desc->way_only_cost = desc->cost;
			desc->upgrade_group = 0;
		}

		if(version < 6 || experimental_version < 1 || !experimental)
		{
			desc->monthly_base_wear = desc->wear_capacity / 600;
		}
	}

	// some internal corrections to pay for previous confusion with two waytypes
	if(desc->wt==tram_wt) {
		desc->styp = type_tram;
		desc->wt = track_wt;
	}
	else if(desc->styp==5  &&  desc->wt==track_wt) {
		desc->wt = monorail_wt;
		desc->styp = type_flat;
	}
	else if(desc->wt==128) {
		desc->wt = powerline_wt;
	}
	
	if(version<=2  &&  desc->wt==air_wt  &&  desc->topspeed>=250) {
		// runway!
		desc->styp = 1;
	}

	// front images from version 5 on
	desc->front_images = version > 4;

	desc->base_cost = desc->cost;
	desc->base_maintenance = desc->maintenance;
	desc->base_way_only_cost = desc->way_only_cost;

	DBG_DEBUG("way_reader_t::read_node()",
		"version=%d, price=%d, maintenance=%d, topspeed=%d, max_weight=%d, "
		"wtype=%d, styp=%d, intro_year=%i, axle_load=%d, wear_capacity=%d, monthly_base_wear=%d, "
		"way_constraints_permissive = %d, way_constraints_prohibitive = %d",
		version,
		desc->cost,
		desc->maintenance,
		desc->topspeed,
		desc->wt,
		desc->styp,
		desc->intro_date/12,
		desc->axle_load,
		desc->wear_capacity,
		desc->monthly_base_wear,
		desc->get_way_constraints().get_permissive(),
		desc->get_way_constraints().get_prohibitive());

  return desc;
}
