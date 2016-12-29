#pragma once

enum Tile_Type
{
    TILE_TYPE_NONE,
    TILE_TYPE_ROOM,
    TILE_TYPE_CORRIDOR,
    
    TILE_TYPE_COUNT
};

struct Rect
{
    v2i min, max;

    b32 collides(Rect *other_rect, i32 min_dist);
};

inline b32 Rect::collides(Rect *other_rect, i32 min_dist = 0)
{
    b32 h_overlap = max.x > (other_rect->min.x - min_dist) && min.x < (other_rect->max.x + min_dist);
    b32 v_overlap = max.y > (other_rect->min.y - min_dist) && min.y < (other_rect->max.y + min_dist);
    return h_overlap && v_overlap;
}

#define TILE_SIZE 10
#define MAX_ROOM 100
struct Tile_Map
{
    i32 x_count, y_count;
    u8 *tiles;
    v2i *position_stack;
    
    u8 &get_tile(v2i tile_position);
    b32 is_tile_valid(v2i tile_position);
    b32 is_tile_walkable(v2i tile_position);
    
    void flood_fill(v2i tile_position, v2i flood_direction);
    void init(i32 x_count, i32 y_count);
    void render();
};

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
            if (!is_tile_valid({x, y}) ||
                get_tile({x, y}) != TILE_TYPE_NONE)
            {
                is_walkable = false;
                break;
            }
        }
    }
    
    return is_walkable;
}

inline u8 &Tile_Map::get_tile(v2i tile_position)
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
    
    if (flood_direction == v2i_zero())
    {
        if (is_tile_walkable(tile_position))
        {
            get_tile(tile_position) = TILE_TYPE_CORRIDOR;
            for_each(i, directions)
            {
                flood_fill(tile_position + directions[i], directions[i]);
            }
        }
    }
    else
    {
        //check if current tile position is walkable for wanderer
        b32 tile_position_is_walkable = true;
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
                    get_tile(check_positions[i]) != TILE_TYPE_NONE)
                {
                    tile_position_is_walkable = false;
                    break;
                }
            }
        }
        
        if (tile_position_is_walkable)
        {
            get_tile(tile_position) = TILE_TYPE_CORRIDOR;
            for_each(i, directions)
            {
                flood_fill(tile_position + directions[i], directions[i]);
            }
        }
    }
}

void Tile_Map::init(i32 x_count, i32 y_count)
{
    this->x_count = x_count;
    this->y_count = y_count;
    if (tiles)
    {
        free(tiles);
        tiles = 0;
    }
    tiles = (u8 *)calloc(x_count * y_count, sizeof(u8));
    
    Rect rooms[MAX_ROOM] = {};
    i32 room_cursor = 0;
    
    //fill up with rooms
    {
        i32 room_fill_count = 100;
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
            for (i32 room_index = 0; room_index < room_cursor; ++room_index)
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
                rooms[room_cursor++] = room;
                for (i32 x = room.min.x; x < room.max.x; ++x)
                {
                    for (i32 y = room.min.y; y < room.max.y; ++y)
                    {
                        get_tile({x, y}) = TILE_TYPE_ROOM;
                    }
                }
            }
        }
    }
    
    //flood-fill corridors
    {
        loop_for(x, x_count)
        {
            loop_for(y, y_count)
            {
                if (is_tile_walkable({x, y}))
                {
                    flood_fill({x, y}, v2i_zero());
                }
            }
        }
    }
}

void Tile_Map::render()
{
    SDL_SetRenderDrawColor(global_renderer, 0x10, 0x10, 0x10, 0xff);
    {
        SDL_Rect fill_rect = {0, WINDOW_HEIGHT - y_count*TILE_SIZE, x_count*TILE_SIZE, y_count*TILE_SIZE};
        SDL_RenderFillRect(global_renderer, &fill_rect);
    }
    
    loop_for(x, x_count)
    {
        loop_for(y, y_count)
        {
            if (get_tile({x, y}) != TILE_TYPE_NONE)
            {
                if (get_tile({x, y}) == TILE_TYPE_ROOM)
                {
                    SDL_SetRenderDrawColor(global_renderer, 0xff, 0, 0, 0xff);
                }
                if (get_tile({x, y}) == TILE_TYPE_CORRIDOR)
                {
                    SDL_SetRenderDrawColor(global_renderer, 0, 0xff, 0, 0xff);
                }
                SDL_Rect tile_rect = {x*TILE_SIZE, WINDOW_HEIGHT - y*TILE_SIZE, TILE_SIZE, TILE_SIZE};
                SDL_RenderFillRect(global_renderer, &tile_rect);
            }
        }
    }
}
