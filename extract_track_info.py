# messy script for parsing metadata out of maps

import math
import os
from pathlib import Path
import sys
import textwrap
from xml.etree import ElementTree as ET

tracks = ['rainbow', 'desert']

src_dir = sys.argv[1]
build_dir = sys.argv[2]

def parse_points(points_str):
    return [[int(x) for x in p.split(',')] for p in points_str.split(' ')]

def get_template_properties(template_path):
    root = ET.parse(template_path)
    props = root.findall('.//property')

    return {x.get('name'): x.get('value') for x in props}

tileset_friction = {}

track_routes = []
track_collisions = []
track_sprites = []
track_structs = []

for name in tracks:

    tmx_filename = f'{src_dir}/assets/{name}.tmx'

    root = ET.parse(tmx_filename)

    tileset_filename = root.findall("tileset")[0].get('source')
    tileset_filename = Path(tmx_filename).parent / tileset_filename

    objects = root.findall('.//objectgroup[@name="meta"]/object')
    collision_objects = root.findall('.//objectgroup[@name="collisions"]/object')

    map_name = Path(tmx_filename).stem
    tileset_name = Path(tileset_filename).stem

    finish_list = ""
    route_list = ""

    background_col = root.getroot().get('backgroundcolor')

    if background_col is None:
        background_col = (0, 0, 0, 255)
    else:
        r = int(background_col[1:3], 16)
        g = int(background_col[3:5], 16)
        b = int(background_col[5:7], 16)
        background_col = (r, g, b, 255)

    sprites = []

    # route/finish objects
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

        # template/sprites
        elif obj.get('template'):
            props = get_template_properties(Path(tmx_filename).parent / obj.get('template'))

            if 'sprite' in props:
                sprite_x, sprite_y, sprite_w, sprite_h = (int(x) for x in props['sprite'].split(','))

                type = 'Static'
                scale = 1.0
                origin_x = 0
                origin_y = 0

                if 'scale' in props:
                    scale = float(props['scale'])
                
                if 'type' in props:
                    type = props['type']

                if 'origin' in props:
                    origin_x, origin_y = (int(x) for x in props['origin'].split(','))

                scale = int(scale * 16)

                sprites.append(f'{{ObjectType::{type}, {scale}, {x}, {y}, {sprite_x}, {sprite_y}, {sprite_w}, {sprite_h}, {origin_x}, {origin_y}}}')

    # collisions (all rects)
    collsion_rects = []
    for obj in collision_objects:
        x = int(obj.get('x'))
        y = int(obj.get('y'))
        w = int(obj.get('width'))
        h = int(obj.get('height'))

        collsion_rects.append(f'{{{x}, {y}, {w}, {h}}}')

    if not collsion_rects:
        collsion_rects.append('{0, 0, 0, 0}')

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

    joined = ', '.join(collsion_rects)
    track_collisions.append(f'static const blit::Rect {map_name}_collisions[]{{{joined}}};')

    sprites_ptr = 'nullptr'
    sprites_len = '0'
    if sprites:
        joined = ', '.join(sprites)
        track_sprites.append(f'static const TrackObjectInfo {map_name}_sprites[]{{{joined}}};')
        sprites_ptr = f'{map_name}_sprites'
        sprites_len = f'std::size({sprites_ptr})'

    bg_pen = ", ".join([str(x) for x in background_col])

    track_structs.append(textwrap.dedent('''
        {{
            "{map_name}",
            {{{finish_list}}}, // finish line
            {map_name}_route, std::size({map_name}_route), // route
            {map_name}_collisions, std::size({map_name}_collisions), // collision rects
            {tileset_name}_friction, std::size({tileset_name}_friction), // tile meta
            {sprites_ptr}, {sprites_len}, // sprites
            asset_{map_name}_map, asset_{tileset_name}_tiles, // assets
            {{{bg_pen}}}
        }}'''.format(finish_list=finish_list, map_name=map_name, tileset_name=tileset_name, sprites_ptr=sprites_ptr, sprites_len=sprites_len, bg_pen=bg_pen)))


indented_tracks = textwrap.indent(','.join(track_structs), '    ')
joined_routes = '\n'.join(track_routes)

joined_collisions = '\n'.join(track_collisions)

joined_sprites = '\n'.join(track_sprites)

joined_friction = '\n'.join(tileset_friction.values())

out_f = open(f'{build_dir}/track-info.cpp', 'w')

rel_to_src = os.path.relpath(src_dir, build_dir)

out_f.write('''
// generated file!
#include <array>

#include "{rel_to_src}/track.hpp"
#include "assets.hpp"

#include "types/point.hpp"
#include "types/rect.hpp"

{joined_friction}

{joined_collisions}

{joined_routes}

{joined_sprites}

extern const TrackInfo track_info[] {{{indented_tracks}
}};

extern const int num_tracks = std::size(track_info);
'''.format(
    rel_to_src=rel_to_src, joined_routes=joined_routes, indented_tracks=indented_tracks,
    joined_friction=joined_friction, joined_collisions=joined_collisions, joined_sprites=joined_sprites
))