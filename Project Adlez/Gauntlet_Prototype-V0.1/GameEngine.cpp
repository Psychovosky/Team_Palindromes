#include "GameEngine.h"

GameEngine::GameEngine()
{
}

// Make sure release is called
GameEngine::~GameEngine()
{
	release();
}

// set up all game components
void GameEngine::initialize()
{
	m_state = MENU;
	running = true;
	// seed rand
	seedRand();
	// setup GUI
	guiTest.initialize(L"gui2.png");
	// setup dungeon
	Dungeon1.initialise("dungeons\\Dungeon1.txt", this);

	// set up buttons
	// button rect
	FRect btemp;
	btemp.top = -32;
	btemp.bottom = 32;
	btemp.left = -64;
	btemp.right = 64;
	// menu
	start.initialize(btemp, V2DF(400,GRID_SIZE*7) );
	start.setText("START",5);
	quit.initialize(btemp, V2DF(400,GRID_SIZE*10) );
	quit.setText("QUIT", 4);
	// class
	warrior.initialize(btemp, V2DF(400,GRID_SIZE*7) );
	warrior.setText("WARRIOR", 7);
	mage.initialize(btemp, V2DF(400,GRID_SIZE*10) );
	mage.setText("MAGE", 4);
	// death
	main.initialize(btemp, V2DF(400,GRID_SIZE*13) );
	main.setText("MAIN\nMENU", 9);
}

// update all game components
void GameEngine::update(float dT)
{
	switch(m_state)
	{
	case MENU:
		// start 
		start.update(dT);
		if(start.clicked())
			m_state = CLASS;
		// quit
		quit.update(dT);
		if(quit.clicked())
			running = false;
		break;
	case CLASS:
		FRect temp;
		temp.bottom = 8;
		temp.top = -8;
		temp.left = -8;
		temp.right = 8;
		// main menu
		main.update(dT);
		if(main.clicked())
			m_state = MENU;
		// select mage class/ start game
		mage.update(dT);
		if(mage.clicked())
		{
			thePlayer.initialize(temp);
			m_state = GAME;
			// TODO:: remove this following line used for debug/test only
			thePlayer.setCurrentRoom(Dungeon1.getStart());
			thePlayer.setPosition( V2DF( GRID_SIZE * 8 + HALF_GRID + BORDER, GRID_SIZE * 6 + HALF_GRID + BORDER ) );
		}
		// select warrior class / start game
		warrior.update(dT);
		if(warrior.clicked())
		{
			thePlayer.initialize(temp);
			m_state = GAME;
			// TODO:: remove this following line used for debug/test only
			thePlayer.setCurrentRoom(Dungeon1.getStart());
			thePlayer.setPosition( V2DF( GRID_SIZE * 8 + HALF_GRID + BORDER, GRID_SIZE * 6 + HALF_GRID + BORDER ) );
		}
		break;
	case GAME:
		thePlayer.getCurrentRoom()->update(dT);
		thePlayer.Update(dT,this);
		break;
	case VICTORY:
	case DEAD:
		// quit
		quit.update(dT);
		if(quit.clicked())
			running = false;
		// main menu
		main.update(dT);
		if(main.clicked())
			m_state = MENU;
		break;
	}
}

// render all game components
void GameEngine::render()
{
	// render the given objects for each state
	switch(m_state)
	{
	case MENU:
		start.render();
		quit.render();
		break;
	case CLASS:
		mage.render();
		warrior.render();
		main.render();
		break;
	case GAME:
		// render GUI background
		for(int y = 0; y < 18; y++)
		{
			for(int x = 0; x < 25; x++)
			{
				float drawX = x * GRID_SIZE + HALF_GRID;
				float drawY = y * GRID_SIZE + HALF_GRID;
				guiTest.draw(V2DF(drawX,drawY),0.0,1.0f);
			}
		}
		// render the room the player is currently in
		thePlayer.getCurrentRoom()->render();
		// render the players
		thePlayer.render();
		break;
	case VICTORY:
	case DEAD:
		main.render();
		quit.render();
		break;
	}
}

void GameEngine::drawText()
{
	RECT screen;
	screen.left = screen.top = 0;
	screen.right = 800;
	screen.bottom = 300;
	switch(m_state)
	{
	case MENU:
		DX2DEngine::getInstance()->writeCenterText("Project Adlez",screen);
		start.drawText();
		quit.drawText();
		break;
	case CLASS:
		mage.drawText();
		warrior.drawText();
		main.drawText();
		break;
	case GAME:
		thePlayer.drawText();
		break;
	case VICTORY:
		DX2DEngine::getInstance()->writeCenterText("YOU WIN!",screen);
	case DEAD:
		if(m_state == DEAD) ///redundant
			DX2DEngine::getInstance()->writeCenterText("DEFEATED!",screen);
		main.drawText();
		quit.drawText();
		break;
	}
}

// safetly release all game components
void GameEngine::release()
{
	guiTest.release();
	thePlayer.release();
	Dungeon1.release();
}

bool GameEngine::isRunning()
{
	return running;
}

void GameEngine::setRunning(bool a_run)
{
	running = a_run;
}

void GameEngine::setState(G_State state)
{
	m_state = state;
}