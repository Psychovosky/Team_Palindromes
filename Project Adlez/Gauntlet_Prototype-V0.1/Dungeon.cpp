#include "Dungeon.h"
#include "GameEngine.h"

// set layout to 0
Dungeon::Dungeon()
{
	start = 0;
	end = 0;
	// go through and set each room to 0
	for(int y = 0; y < FLR_SIZE; ++y)
	{
		for(int x = 0; x < FLR_SIZE; ++x)
		{
			m_layout[y][x] = 0;
		}
	}
}

// make sure safe release is called
Dungeon::~Dungeon()
{
	release();
}

// safetly delete all data
void Dungeon::release()
{
	// go through and safely delete each room
	for(int y = 0; y < FLR_SIZE; ++y)
	{
		for(int x = 0; x < FLR_SIZE; ++x)
		{
			// first check if the room exist
			if(m_layout[y][x])
			{
				// now release and then delete the room
				m_layout[y][x]->release();
			}
		}
	}
}

// load the given floor file
void Dungeon::initialise(char* floorName, GameEngine* ref, Player* thePlayer)
{
	// first load the given file
	FILE* file;
	fopen_s(&file, floorName, "r");
	char tempChar;
	char tempLayout[FLR_SIZE][FLR_SIZE] = {0};
	// go through the file and generate the floor based on it
	for(int y = 0; y < FLR_SIZE; ++y)
	{
		for(int x = 0; x < FLR_SIZE; ++x)
		{
			// get the current character from the file
			tempChar = fgetc(file);
			// first make sure if the end of the line has been reached if so get the next character
			if(tempChar == '\n')
				tempChar = fgetc(file);
			// now check if the end of the file has been reached
			if(tempChar == EOF)
				break;
			// the character is good store it
			tempLayout[y][x] = tempChar;
		}
		// now check if the end of the file has been reached
		if(tempChar == EOF)
			break;
	}
	// make sure to close the file
	fclose(file);

	char roomName [LEN] = {0};

	char north = 0, south = 0, east = 0, west = 0;

	// generate the map based on the file info
	for(int y = 0; y < FLR_SIZE; ++y)
	{
		for(int x = 0; x < FLR_SIZE; ++x)
		{
			// go through each location and decide how many doors each room should have
			switch(tempLayout[y][x])
			{
			case '#': // Basic room
				// now decide the type of room to be loaded
				// check north side
				if(y - 1 < 0 || tempLayout[y - 1][x] == '.')
					north = BLANK;
				else
					north = N_DOOR;
				// check south side
				if(y + 1 >= FLR_SIZE || tempLayout[y + 1][x] == '.')
					south = BLANK;
				else
					south = S_DOOR;
				// check east side
				if(x + 1 >= FLR_SIZE || tempLayout[y][x + 1] == '.')
					east = BLANK;
				else
					east = E_DOOR;
				// check west side
				if(x - 1 < 0 || tempLayout[y][x - 1] == '.')
					west = BLANK;
				else
					west = W_DOOR;

				sprintf_s(roomName, "rooms\\%c%c%c%c.txt", north, south, east, west);

				// now load the given room file
				m_layout[y][x] = new Room(); 
				m_layout[y][x]->initialize(roomName, BASIC, ref, thePlayer);
				break;
			case 'S': // start
				// just load the start room
				m_layout[y][x] = new Room();
				m_layout[y][x]->initialize("rooms\\start.txt", START, ref, thePlayer);
				start = m_layout[y][x];
				break;
			case 'E': // end
				m_layout[y][x] = new Room();
				m_layout[y][x]->initialize("rooms\\end.txt", END, ref, thePlayer);
				end = m_layout[y][x];
				break;
			case '.': // blank
				// load nothing blank space
				m_layout[y][x] = 0;
				break;
			}
		}
	}

	// temp doors
	Room* a_doors[4] = {0};
	// asign rooms to doors
	for(int y = 0; y < FLR_SIZE; ++y)
	{
		// reset
		for(int i = 0; i < 4; ++i)
			a_doors[i] = 0;

		for(int x = 0; x < FLR_SIZE; ++x)
		{
			switch(tempLayout[y][x])
			{
			case '.': // blank do nothing
				break;
			case '#': // standard room
				// west door
				if( !(x - 1 < 0) && m_layout[y][x-1])
					a_doors[WEST] = m_layout[y][x-1];
				// east door
				if( !(x + 1 >= ROOM_WIDTH) && m_layout[y][x+1])
					a_doors[EAST] = m_layout[y][x+1];
				// north door
				if( !(y - 1 < 0) && m_layout[y-1][x])
					a_doors[NORTH] = m_layout[y-1][x];
				// south door
				if( !(y + 1 < 0) && m_layout[y+1][x])
					a_doors[SOUTH] = m_layout[y+1][x];
				break;
			case 'E': // end room
				// south door
				if( !(y + 1 >= ROOM_WIDTH) && m_layout[y+1][x])
					a_doors[SOUTH] = m_layout[y+1][x];
				break;
			case 'S': // start room
				// north door
				if( !(y - 1 < 0) && m_layout[y-1][x])
					a_doors[NORTH] = m_layout[y-1][x];
				break;
			}
			// set doors to the current room
			if(m_layout[y][x])
				m_layout[y][x]->setDoors(a_doors);
		}
	}
}

Room* Dungeon::getStart()
{
	return start;
}

bool Dungeon::allDead()
{
	int count = 0;
	for(int y = 0; y < FLR_SIZE; y++)
	{
		for(int x = 0; x < FLR_SIZE; x++)
		{
			if(m_layout[y][x])
				count += m_layout[y][x]->EnemyCount();
		}
	}
	return (count <= 0);
}