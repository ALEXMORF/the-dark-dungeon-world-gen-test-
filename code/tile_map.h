#pragma once

struct Rect
{
    v2i min, max;

    b32 collides(Rect *other_rect, i32 min_dist);
};

inline b32 Rect::collides(Rect *other_rect, i32 min_dist = 0)
{
    b32 h_overlap = max.x >= (other_rect->min.x - min_dist) && min.x <= (other_rect->max.x + min_dist);
    b32 v_overlap = max.y >= (other_rect->min.y - min_dist) && min.y <= (other_rect->max.y + min_dist);
    return h_overlap && v_overlap;
}

#define MAX_ROOM 150
#define TILE_VALUE_FILLER 1
struct Tile_Map
{
    i32 x_count, y_count;
    i32 *tiles;

    i32 room_count;
    i32 region_id;
    
    i32 &get_tile(v2i tile_position);
    b32 is_tile_valid(v2i tile_position);
    b32 is_tile_walkable(v2i tile_position);

    void init(i32 x_count, i32 y_count);

    void unify_region_id(v2i tile_position, i32 region_id);
    void flood_fill(v2i tile_position, v2i flood_direction);
    void uncarve(v2i tile_position);
};

void Tile_Map::init(i32 x_count, i32 y_count)
{
    region_id = TILE_VALUE_FILLER + 1;
    this->x_count = x_count;
    this->y_count = y_count;
    if (tiles)
    {
        free(tiles);
        tiles = 0;
    }
    tiles = (i32 *)calloc(x_count * y_count, sizeof(i32));
    
    Rect rooms[MAX_ROOM] = {};
    i32 room_count = 0;

    //fill up with rooms
    {   
        i32 room_fill_count = MAX_ROOM;
        i32 room_max_width = 10;
        i32 room_min_width = 5;
        i32 room_max_height = 10;
        i32 room_min_height = 5;
        i32 room_min_gap = 1;
        
        loop_for(i, room_fill_count)
        {
            v2i room_size = {ranged_rand(room_min_width, room_max_width),
                             ranged_rand(room_min_height, room_max_height)};
            v2i room_lowerleft = {ranged_rand(0, x_count - room_size.x - 1),
                                  ranged_rand(0, y_count - room_size.y - 1)};
            Rect room = {room_lowerleft, room_lowerleft + room_size};
            
            b32 room_overlaps = false;
            loop_for(room_index, room_count)
            {
                if (rooms[room_index].collides(&room, room_min_gap))
                {
                    room_overlaps = true;
                    break;
                }
            }
            
            if (!room_overlaps)
            {
                //add room
                rooms[room_count++] = room;
                for (i32 x = room.min.x; x <= room.max.x; ++x)
                {
                    for (i32 y = room.min.y; y <= room.max.y; ++y)
                    {
                        get_tile({x, y}) = region_id;
                    }
                }
                ++region_id;
            }
        }
    }
    this->room_count = room_count;
    
    //flood-fill corridors
    loop_for(x, x_count)
    {
        loop_for(y, y_count)
        {
            if (is_tile_walkable({x, y}))
            {
                flood_fill({x, y}, v2i_zero());
                ++region_id;
            }
        }
    }
    
    //carving ports to rooms and corridors
    {
        loop_for(room_index, room_count)
        {
            Rect *room = &rooms[room_index];
            i32 room_width = room->max.x - room->min.x;
            i32 room_height = room->max.y - room->min.y;
            i32 current_room_region_id = get_tile(room->min);
                
            b32 has_ports = true;
            while (has_ports)
            {
                v2i *possible_ports = (v2i *)calloc((room_width-2)*2 + (room_height-2)*2, sizeof(v2i));
                i32 ports_count = 0;
            
                //bottom & top row check
                for (i32 x = room->min.x + 1; x < room->max.x - 1; ++x)
                {
                    i32 bot_y = room->min.y - 1;
                    i32 top_y = room->max.y + 1;
                    v2i top_tile_position = {x, room->max.y};
                    v2i bot_tile_position = {x, room->min.y};

                    //check bot
                    if (is_tile_valid({x, bot_y-1}) && get_tile({x, bot_y-1}) != 0 &&
                        get_tile({x, bot_y-1}) != get_tile(bot_tile_position))
                    {
                        possible_ports[ports_count++] = make_v2i(x, bot_y);
                    }
                    //check top
                    if (is_tile_valid({x, top_y+1}) && get_tile({x, top_y+1}) != 0 &&
                        get_tile({x, top_y+1}) != get_tile(top_tile_position))
                    {
                        possible_ports[ports_count++] = make_v2i(x, top_y);
                    }
                }
                
                //left & right row check
                for (i32 y = room->min.y + 1; y < room->max.y - 1; ++y)
                {
                    i32 left_x = room->min.x - 1;
                    i32 right_x = room->max.x + 1;
                    v2i left_tile_position = {room->min.x, y};
                    v2i right_tile_position = {room->max.x, y};
                    
                    //check left
                    if (is_tile_valid({left_x-1, y}) && get_tile({left_x-1, y}) != 0 &&
                        get_tile({left_x-1, y}) != get_tile(left_tile_position))
                    {
                        possible_ports[ports_count++] = make_v2i(left_x, y);
                    }
                    //check right
                    if (is_tile_valid({right_x+1, y}) && get_tile({right_x+1, y}) != 0 &&
                        get_tile({right_x+1, y}) != get_tile(right_tile_position))
                    {
                        possible_ports[ports_count++] = make_v2i(right_x, y);
                    }
                }

                //pick a random port and carve it 
                if (ports_count != 0)
                {
                    i32 random_index = ranged_rand(0, ports_count);
                    v2i port_position = possible_ports[random_index];
                    
                    get_tile(port_position) = TILE_VALUE_FILLER;
                    unify_region_id(port_position, current_room_region_id);
                }
                else
                {
                    has_ports = false;
                }
                
                free(possible_ports);
            }
        }
    }
    
    //uncarving deadends
    loop_for(x, x_count)
    {
        loop_for(y, y_count)
        {
            v2i tile_position = {x, y};
            if (get_tile(tile_position) != 0)
            {
                i32 sides_connected = 0;
                v2i side_offsets[] = {{-1, 0}, {1, 0}, {0, 1}, {0, -1}};
                for_each(i, side_offsets)
                {
                    if (is_tile_valid(tile_position + side_offsets[i]) &&
                        get_tile(tile_position + side_offsets[i]) != 0)
                    {
                        ++sides_connected;
                    }
                }

                if (sides_connected == 0)
                {
                    get_tile(tile_position) = 0;
                }
                else if (sides_connected == 1)
                {
                    uncarve(tile_position);
                }
            }
        }
    }
}

void Tile_Map::uncarve(v2i tile_position)
{
    if (get_tile(tile_position) == 0)
    {
        return;
    }
    
    i32 sides_connected = 0;
    v2i side_offsets[] = {{-1, 0}, {1, 0}, {0, 1}, {0, -1}};
    v2i sole_side_offset = {};
    for_each(i, side_offsets)
    {
        if (is_tile_valid(tile_position + side_offsets[i]) &&
            get_tile(tile_position + side_offsets[i]) != 0)
        {
            ++sides_connected;
            sole_side_offset = side_offsets[i];
        }
    }
    
    if (sides_connected == 1)
    {
        get_tile(tile_position) = 0;
        uncarve(tile_position + sole_side_offset);
    }
}

void Tile_Map::unify_region_id(v2i tile_position, i32 region_id)
{
    if (is_tile_valid(tile_position) && get_tile(tile_position) != 0 &&
        get_tile(tile_position) != region_id)
    {
        get_tile(tile_position) = region_id;
        
        unify_region_id(tile_position + make_v2i(1, 0), region_id);
        unify_region_id(tile_position + make_v2i(-1, 0), region_id);
        unify_region_id(tile_position + make_v2i(0, 1), region_id);
        unify_region_id(tile_position + make_v2i(0, -1), region_id);
    }
}

b32 Tile_Map::is_tile_valid(v2i tile_position)
{
    b32 x_is_valid = tile_position.x >= 0 && tile_position.x < x_count;
    b32 y_is_valid = tile_position.y >= 0 && tile_position.y < y_count;
    return x_is_valid && y_is_valid;
}

b32 Tile_Map::is_tile_walkable(v2i tile_position)
{
    b32 is_walkable = true;
    for (i32 x = tile_position.x - 1; x <= tile_position.x + 1; ++x)
    {
        if (!is_walkable)
        {
            break;
        }
        for (i32 y = tile_position.y - 1; y <= tile_position.y + 1; ++y)
        {
            if (!is_tile_valid({x, y}) || get_tile({x, y}) != 0)
            {
                is_walkable = false;
                break;
            }
        }
    }
    
    return is_walkable;
}

inline i32 &Tile_Map::get_tile(v2i tile_position)
{
    assert(tile_position.x >= 0 && tile_position.x < x_count &&
           tile_position.y >= 0 && tile_position.y < y_count);
    return tiles[tile_position.x + tile_position.y * x_count];
}

void Tile_Map::flood_fill(v2i tile_position, v2i flood_direction)
{
    if (!is_tile_valid(tile_position))
    {
        return;
    }
    
    //randomly shuffle next directions
    v2i directions[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (i32 i = array_count(directions)-1; i >= 0; --i)
    {
        i32 random_index = ranged_rand(0, array_count(directions));
        
        //swap
        if (random_index != i)
        {
            v2i temp = directions[i];
            directions[i] = directions[random_index];
            directions[random_index] = temp;
        }
    }

    //check if current tile position is fillable
    b32 tile_position_is_walkable = true;
    if (flood_direction == v2i_zero())
    {
        tile_position_is_walkable = is_tile_walkable(tile_position);
    }
    else
    {
        v2i check_positions[6];
        if (flood_direction.x)
        {
            check_positions[0] = tile_position;
            check_positions[1] = {tile_position.x, tile_position.y+1};
            check_positions[2] = {tile_position.x, tile_position.y-1};
            check_positions[3] = tile_position + flood_direction;
            check_positions[4] = check_positions[3] + make_v2i(0, -1);
            check_positions[5] = check_positions[3] + make_v2i(0, 1);
        }
        else if (flood_direction.y)
        {
            check_positions[0] = tile_position;
            check_positions[1] = {tile_position.x+1, tile_position.y};
            check_positions[2] = {tile_position.x-1, tile_position.y};
            check_positions[3] = tile_position + flood_direction;
            check_positions[4] = check_positions[3] + make_v2i(1, 0);
            check_positions[5] = check_positions[3] + make_v2i(-1, 0);
        }
        else
        {
            assert(!"what the fuck floor_direction is nulled???");
        }
        for_each(i, check_positions)
        {
            if (!is_tile_valid(check_positions[i]) ||
                get_tile(check_positions[i]) != 0)
            {
                tile_position_is_walkable = false;
                break;
            }
        }
    }
    
    //fill tile and flood surrounding tiles
    if (tile_position_is_walkable)
    {
        get_tile(tile_position) = region_id;
        for_each(i, directions)
        {
            flood_fill(tile_position + directions[i], directions[i]);
        }
    }
}
