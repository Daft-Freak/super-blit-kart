# messy script for parsing metadata out of maps

import math
import os
from pathlib import Path
import sys
import textwrap
from xml.etree import ElementTree as ET

tracks = ['rainbow']

src_dir = sys.argv[1]
build_dir = sys.argv[2]

def parse_points(points_str):
    return [[int(x) for x in p.split(',')] for p in points_str.split(' ')]

tileset_friction = {}

track_routes = []
track_structs = []

for name in tracks:

    tmx_filename = f'{src_dir}/assets/{name}.tmx'

    root = ET.parse(tmx_filename)

    tileset_filename = root.findall("tileset")[0].get('source')
    tileset_filename = Path(tmx_filename).parent / tileset_filename

    objects = root.findall('.//objectgroup[@name="meta"]/object')

    map_name = Path(tmx_filename).stem
    tileset_name = Path(tileset_filename).stem

    finish_list = ""
    route_list = ""

    for obj in objects:
        name = obj.get('name')

        x = int(obj.get('x'))
        y = int(obj.get('y'))

        if name == 'finish':
            line = obj.find("polyline")
            points = parse_points(line.get("points"))
            
            points = [[p[0] + x, p[1] + y] for p in points]
            finish_list = ', '.join([f'{{{p[0]}, {p[1]}}}' for p in points])

        elif name == 'route':
            line = obj.find("polygon")
            points = parse_points(line.get("points"))

            points.append(points[0]) # close the loop
            
            points = [[p[0] + x, p[1] + y] for p in points]
            route_list = ', '.join([f'{{{p[0]}, {p[1]}}}' for p in points])

    # now the tileset
    if tileset_name not in tileset_friction:
        root = ET.parse(tileset_filename)
        tiles = root.findall('tile')

        tile_friction = [0.0] + [1.0] * 255 # defaults

        for tile in tiles:
            tid = int(tile.get('id'))

            prop = tile.find('.//property[@name="friction"]')
            if prop is not None:
                tile_friction[tid] = float(prop.get('value'))

        while tile_friction[-1] == 1.0:
            tile_friction.pop()

        friction_list = ', '.join([f'{x}f' for x in tile_friction])
        tileset_friction[tileset_name] = f'static const float {tileset_name}_friction[]{{{friction_list}}};'

    # build arrays/structs
    track_routes.append(f'static const blit::Point {map_name}_route[]{{{route_list}}};')

    track_structs.append(textwrap.dedent('''
        {{
            {{{finish_list}}}, // finish line
            {map_name}_route, std::size({map_name}_route), // route
            {tileset_name}_friction, std::size({tileset_name}_friction), // tile meta
            asset_{map_name}_map, asset_{tileset_name}_tiles // assets
        }}'''.format(finish_list=finish_list, map_name=map_name, tileset_name=tileset_name)))


indented_tracks = textwrap.indent(','.join(track_structs), '    ')
joined_routes = '\n'.join(track_routes)

joined_friction = '\n'.join(tileset_friction.values())

out_f = open(f'{build_dir}/track-info.cpp', 'w')

rel_to_src = os.path.relpath(src_dir, build_dir)

out_f.write('''
// generated file!
#include <array>

#include "{rel_to_src}/track.hpp"
#include "assets.hpp"

#include "types/point.hpp"

{joined_friction}

{joined_routes}

extern const TrackInfo track_info[] {{{indented_tracks}
}};

extern const int num_tracks = std::size(track_info);
'''.format(rel_to_src=rel_to_src, joined_routes=joined_routes, indented_tracks=indented_tracks, joined_friction=joined_friction))