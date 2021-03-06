#include "Room.h"
#include "HelpfulData.h"
#include "v2d.h"
#include <fstream>
#include <stdio.h>
#include "GameEngine.h"
using namespace std;

// Default constructor
Room::Room()
{
	// set visited to false
	visited = false;
}

// Default destructor
// make sure safer release is called
Room::~Room()
{ 
	release(); 
}

// safetly release all data
void Room::release()
{
	m_floorTex.release();
	m_wallBaseTex.release();
	m_edgeTex.release();
	m_connectTex.release();
	m_doorTex.release();
	m_lockedTex.release();
	m_enemies.release();
	m_rocks.release();
}

// load all textures and set up the room
void Room::initialize(char* roomFile, ROOM_TYPE type, GameEngine *ref, Player* thePlayer)
{
	// set graphics engine reference
	pGEngine = ref;

	// set room type
	m_type = type;

	// used late in graph construction, keeps track of the number of "free" 
	int freeSize = 0;

	// first load textures
	m_floorTex.initialize(L"images/floor_dirt.png");
	m_wallBaseTex.initialize(L"images/wall_base.png");
	m_edgeTex.initialize(L"images/wall_edge.png");
	m_connectTex.initialize(L"images/wall_connection.png");
	m_doorTex.initialize(L"images/door.png");
	m_lockedTex.initialize(L"images/locked.png");

	// create fstream opject to load map from file
	int y = 0;
	FILE* file;
	fopen_s(&file, roomFile, "r");
	char temp = 0;
	bool additionalData = false;

	// generate room placing doors where appropriate
	for( int y = 0; y < ROOM_HEIGHT; ++y )
	{
		for(int x = 0; x < ROOM_WIDTH; ++x)
		{
			// get current line of from the file
			temp = fgetc(file);
			// check if a line has ended. if so make sure to grab the next char
			if(temp == '\n')
				temp = fgetc(file);
			// make sure temp is not eof
			if(temp == EOF)
				break;
			bool createRock = false;
			// now parse this line to create the room
			switch(temp)
			{
			case 'R':
				createRock = true;
			case'.': // floor
				m_layout[y][x].m_type = FLOOR;
				m_layout[y][x].m_graphNodeID = -1;
				// increment 
				freeSize++;
				break;
			case '#': // wall
				m_layout[y][x].m_type = WALL;
				m_layout[y][x].m_graphNodeID = -1;
				break;
			case 'D': // door
				m_layout[y][x].m_door.locked = false;
				m_layout[y][x].m_type = DOOR;
				m_layout[y][x].m_graphNodeID = -1;
				break;
			case -1:
				break;
			default:
				if(temp == '\n')
					break;
				// generate world position
				V2DF position( (x * GRID_SIZE) + BORDER + HALF_GRID, (y * GRID_SIZE) + BORDER + HALF_GRID );
				// load correct enemy
				Enemy *tEnemy = new Enemy();
				tEnemy->initialize(temp, position, thePlayer, this);
				m_enemies.add(tEnemy);
				// create floor
				m_layout[y][x].m_type = FLOOR;
				m_layout[y][x].m_graphNodeID = -1;
				// increment 
				freeSize++;
				break;
			}

			// create a rock if told to
			if(createRock)
			{
				V2DF Rpos(x * GRID_SIZE + HALF_GRID + BORDER,y * GRID_SIZE + HALF_GRID + BORDER);
				FRect Rrect; Rrect.top = -8; Rrect.left = -8; Rrect.right = 8; Rrect.bottom = 8;
				Entity Rrock(Rpos, L"images/gem.png", Rrect, 0.0f, 0.5f);
				m_rocks.add(Rrock);
			}
		}
		// make sure temp is not eof
		if(temp == EOF)
			break;
	}
	// make sure to close the file
	fclose(file);

	// once the room has been constructed create a graph from it
	// be sure there is free space to make a graph
	if(freeSize > 0)
	{
		m_graph.initialize(freeSize);
	}

	// now create a node for each free space
	for(int y = 0; y < ROOM_HEIGHT; ++y)
	{
		for(int x = 0; x < ROOM_WIDTH; ++x)
		{
			// if this is a free space make a graph node for it
			if(m_layout[y][x].m_type == FLOOR)
			{
				// calculate the screen position
				V2DF temp;
				temp.x = (x * GRID_SIZE) + BORDER + HALF_GRID;
				temp.y = (y * GRID_SIZE) + BORDER + HALF_GRID;
				// create the node
				m_layout[y][x].m_graphNodeID = m_graph.createNode(temp);
			}
		}
	}

	// now create connections for the graph
	for(int y = 0; y < ROOM_HEIGHT; ++y)
	{
		for(int x = 0; x < ROOM_WIDTH; ++x)
		{
			int nodeA = m_layout[y][x].m_graphNodeID;
			// see if there is a node here
			if(m_layout[y][x].m_graphNodeID >= 0)
			{
				// now look at all adjacent nodes
				for(int yD = -1; yD < 2; ++yD)
				{
					// make sure this is within the room
					if(yD + y >= 0 && yD + y < ROOM_HEIGHT)
					{
						for(int xD = -1; xD < 2; ++xD)
						{
							// make sure not to check the same node
							if( !(xD == 0 && yD == 0) )
							{
								// make sure this is within the room
								// and that the node is in the graph
								// make sure not to iclude itself
								if(xD + x >= 0 && xD + x < ROOM_WIDTH &&
									m_layout[yD+y][xD+x].m_graphNodeID >= 0 )
								{
									// if diagonal it costs 20
									int cost = 20;
									// otherwise cose is 10
									if( xD == 0 || yD == 0 )
										cost = 10;
									int nodeB = m_layout[yD+y][xD+x].m_graphNodeID;
									// create a oneway connection
									m_graph.setOneWayConnection(nodeA, nodeB, cost);
								}
							}
						}
					}
				}
			}
		}
	}
}

// draw the room
// and all its contents
void Room::render()
{
	// draw each tile of the room
	for(int y = 0; y < ROOM_HEIGHT; y++)
	{
		for(int x = 0; x < ROOM_WIDTH; x++)
		{
			V2DF temp;
			temp.x = (x * GRID_SIZE) + BORDER + HALF_GRID;
			temp.y = (y * GRID_SIZE) + BORDER + HALF_GRID;
			// now draw the correct texture based on the tile type
			switch( m_layout[y][x].m_type )
			{
			case WALL:
				// first draw the base of the wall
				m_wallBaseTex.draw(temp, 0.0f, 1.0f);
				// draw the top side
				// if there is another wall connect them
				if( y-1 >= 0 && y-1 < ROOM_HEIGHT && m_layout[y-1][x].m_type == WALL )
					m_connectTex.draw(temp, 0.0f, 1.0f);
				else // if there isn't draw the edge
					m_edgeTex.draw(temp, 0.0f, 1.0f);
				// draw the bottom side
				// if there is another wall connect them
				if( y+1 >= 0 && y+1 < ROOM_HEIGHT && m_layout[y+1][x].m_type == WALL )
					m_connectTex.draw(temp, 180.0f, 1.0f);
				else // if there isn't draw the edge
					m_edgeTex.draw(temp, 180.0f, 1.0f);
				// draw the right side
				// if there is another wall connect them
				if( x+1 >= 0 && x+1 < ROOM_WIDTH && m_layout[y][x+1].m_type == WALL )
					m_connectTex.draw(temp, 90.0f, 1.0f);
				else // if there isn't draw the edge
					m_edgeTex.draw(temp, 90.0f, 1.0f);
				// draw the left side
				// if there is another wall connect them
				if( x-1 >= 0 && x-1 < ROOM_WIDTH && m_layout[y][x-1].m_type == WALL )
					m_connectTex.draw(temp, -90.0f, 1.0f);
				else // if there isn't draw the edge
					m_edgeTex.draw(temp, -90.0f, 1.0f);
				break;
			case FLOOR:
				// draw the floor texture
				m_floorTex.draw(temp, 0.0f, 1.0f);
				break;
			case DOOR:
				// check if this door is locked
				if(m_layout[y][x].m_door.locked)
					m_lockedTex.draw(temp, 0.0f, 1.0f);
				else
					m_doorTex.draw(temp, 0.0f, 1.0f);
				break;
			}
		}
	}
	
	for(int i = 0; i < m_enemies.size(); ++i)
		m_enemies.get(i)->render();

	for(int i = 0; i < m_rocks.size(); ++i)
		m_rocks.get(i).render();

//	m_graph.render();
}

// check the given location. if it is enterable return true else return false
Node Room::checkLocation(int x, int y)
{
	// first check and make sure the given x and y are within bounds of the room if not return false
	if(x < 0 || x >= ROOM_WIDTH || y < 0 || y >= ROOM_HEIGHT)
	{
		Node temp;
		temp.m_type = BAD;
		return temp;
	}
	// if within the room return the node
	return m_layout[y][x];
}

void Room::setDoors(Room* a_doors[4])
{
	// find the north and south doors
	for(int x = 0; x < ROOM_WIDTH; x++)
	{
		// north door
		if (m_layout[0][x].m_type == DOOR)
		{
			m_layout[0][x].m_door.nextRoom = a_doors[NORTH];
			m_layout[0][x].m_door.posNew = V2DF(x, ROOM_HEIGHT - 2);
		}
		// sout door
		if (m_layout[ROOM_HEIGHT - 1][x].m_type == DOOR)
		{
			m_layout[ROOM_HEIGHT - 1][x].m_door.nextRoom = a_doors[SOUTH];
			m_layout[ROOM_HEIGHT - 1][x].m_door.posNew = V2DF(x, 1);
		}
	}
	// find the east and west doors
	for(int y = 0; y < ROOM_HEIGHT; y++)
	{
		// west door
		if (m_layout[y][0].m_type == DOOR)
		{
			m_layout[y][0].m_door.nextRoom = a_doors[WEST];
			m_layout[y][0].m_door.posNew = V2DF(ROOM_WIDTH - 2, y);
		}
		// east door
		if (m_layout[y][ROOM_WIDTH - 1].m_type == DOOR)
		{
			m_layout[y][ROOM_WIDTH - 1].m_door.nextRoom = a_doors[EAST];
			m_layout[y][ROOM_WIDTH - 1].m_door.posNew = V2DF(1, y);
		}
	}
}

void Room::enterRoom(Player* thePlayer)
{
}

void Room::update(float dT)
{
	// update enemies
	for(int i = 0; i < m_enemies.size(); ++i)
		m_enemies.get(i)->update(dT);
	// check if any enemies are dead
	for(int i = 0; i < m_enemies.size(); ++i)
	{
		// check this enemy's health
		if(m_enemies.get(i)->checkHealth() <= 0)
		{
			// on a random chance spawn a gem
			if(randomInt(0,100) >= 50)
			{
				FRect rect; rect.top = -8; rect.left = -8; rect.right = 8; rect.bottom = 8;
				Entity gem(m_enemies.get(i)->getPosition(), L"images/gem.png", rect, 0.0f, 0.5f);
				m_rocks.add(gem);
			}
			// this enemy is dead remove it from the list
			m_enemies.removeFast(i);
		}
	}
}

// check if the given entity is colliding with any walls in the room
// return true if any collisions occured and false otherwise
bool Room::coll_walls(Entity* entity)
{
	// hold the result of whether a collision occured
	bool result = false;
	// go through the entire room and check against each door/wall
	for(int y = 0; y < ROOM_HEIGHT; y++)
	{
		for(int x = 0; x < ROOM_WIDTH; x++)
		{
			// first make sure that the space is a wall
			if( m_layout[y][x].m_type == WALL )
			{
				// calculate this tile's rect
				V2DF temp;
				temp.x = (x * GRID_SIZE) + BORDER + HALF_GRID;
				temp.y = (y * GRID_SIZE) + BORDER + HALF_GRID;
				FRect bRect;
				bRect.top = temp.y - 16.0f;
				bRect.left = temp.x - 16.0f;
				bRect.right = temp.x + 16.0f;
				bRect.bottom = temp.y + 16.0f;
				// now check if this wall collides with the given entity
				SIDE check = entity->collisionCheck(bRect, true);
				if (  check != NONE )
					result = true;
			}
		}
	}
	// the given rect collides with no walls or doors return false
	return result;
}

Room* Room::collOpenDoors(FRect a_rect, V2DF& posNew)
{
	for(int y = 0; y < ROOM_HEIGHT; ++y)
	{
		for(int x = 0; x < ROOM_WIDTH; ++x)
		{
			// calculate this tile's rect
			V2DF temp;
			temp.x = (x * GRID_SIZE) + BORDER + HALF_GRID;
			temp.y = (y * GRID_SIZE) + BORDER + HALF_GRID;
			FRect bRect;
			bRect.top = temp.y - 16.0f;
			bRect.left = temp.x - 16.0f;
			bRect.right = temp.x + 16.0f;
			bRect.bottom = temp.y + 16.0f;
			// if this space is an unlocked door and the player is on it return true
			if(m_layout[y][x].m_type == DOOR && !m_layout[y][x].m_door.locked &&
				colliding(a_rect, bRect) )
			{
				posNew = m_layout[y][x].m_door.posNew;
				return m_layout[y][x].m_door.nextRoom;
			}
		}
	}
	return 0;
}

// returns a node of the given location if it exists
// if the location doesn't have a node null is returned
GraphNode* Room::getNode(int x, int y)
{
	// make sure x and y are within the room...
	// ..and has a node
	if(x < 0 || x >= ROOM_WIDTH || y < 0 || y >= ROOM_HEIGHT ||
		m_layout[y][x].m_graphNodeID < 0)
		return 0;

	// the location exists return a reference to it
	return m_graph.getNode( m_layout[y][x].m_graphNodeID );
}

// returns a node of the given location if it exists
// if the location doesn't have a node null is returned
GraphNode* Room::getNode(V2DF pos)
{
	for(int y = 0; y < ROOM_HEIGHT; ++y)
	{
		for(int x = 0; x < ROOM_WIDTH; ++x)
		{
			// only check if it is a floor tile
			if( m_layout[y][x].m_type == FLOOR )
			{
				// calculate this tile's rect
				V2DF temp;
				temp.x = (x * GRID_SIZE) + BORDER + HALF_GRID;
				temp.y = (y * GRID_SIZE) + BORDER + HALF_GRID;
				FRect bRect;
				bRect.top = temp.y - 16.0f;
				bRect.left = temp.x - 16.0f;
				bRect.right = temp.x + 16.0f;
				bRect.bottom = temp.y + 16.0f;
				
				// check if the player is in this node
				if( colliding(pos, bRect) )
					return getNode(x,y);
			}
		}
	}
	// no node reutrn null
	return 0;
}

// returns the current number of enemies in this room
int Room::EnemyCount() { return m_enemies.size(); }

// returns a pointer to the given enemy
// if the enemy doesn't exist or the index is out of bounds a null pointer is returned
Enemy* Room::getEnemy(int a_index)
{
	// make sure index is within range
	if( a_index < 0 || a_index >= m_enemies.size() )
		return NULL;

	// index is in range return enemy reference
	return m_enemies.get(a_index);
}

// returns a reference to the graph node that the player occupies
GraphNode* Room::getPlayerNode(Player* thePlayer)
{
	// check which floor tile the player is in
	V2DF playerPos = thePlayer->getPosition();
	for(int y = 0; y < ROOM_HEIGHT; ++y)
	{
		for(int x = 0; x < ROOM_WIDTH; ++x)
		{
			// only check if it is a floor tile
			if( m_layout[y][x].m_type == FLOOR )
			{
				// calculate this tile's rect
				V2DF temp;
				temp.x = (x * GRID_SIZE) + BORDER + HALF_GRID;
				temp.y = (y * GRID_SIZE) + BORDER + HALF_GRID;
				FRect bRect;
				bRect.top = temp.y - 16.0f;
				bRect.left = temp.x - 16.0f;
				bRect.right = temp.x + 16.0f;
				bRect.bottom = temp.y + 16.0f;
				
				// check if the player is in this node
				if( colliding(playerPos, bRect) )
					return getNode(x,y);
			}
		}
	}
	// no node reutrn null
	return 0;
}

// collide with all enemies on the map
// return true if an enemy has been collided with
bool Room::coll_enemies(Entity* entity)
{
	bool col = false;
	// check against all enemies
	for(int i = 0; i < m_enemies.size(); ++i)
	{
		// check if this enemy is colliding with the give entity
		if( entity->collisionCheck( m_enemies.get(i), true ) != NONE )
			col = true;
	}
	// collisions occured return false
	return col;
}

// same as overload although the enemy of the given index is ignored
bool Room::coll_enemies(Entity* entity, int ignoreIndex)
{
	bool col = false;
	// check against all enemies
	for(int i = 0; i < m_enemies.size(); ++i)
	{
		// check if this enemy is colliding with the give entity
		// if the enemy being checked is the given index ignore it
		if( entity->collisionCheck( m_enemies.get(i), true ) != NONE && i != ignoreIndex)
			col = true;
	}
	// collisions occured return false
	return col;
}

int Room::coll_gems(Entity* entity)
{
	int collNum = 0;
	// check each rock
	for(int i = 0; i < m_rocks.size(); ++i)
	{
		if( m_rocks.get(i).collisionCheck(entity,false) != NONE)
		{
			// remove it fromt the list
			m_rocks.removeFast(i);
			i--;
			collNum++;
		}
	}
	return collNum;
}