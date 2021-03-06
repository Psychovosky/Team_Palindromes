#pragma once
#include "Entity.h"
#include "HelpfulData.h"
#include "Animation.h"
extern class Player;

enum ITEM {SWORD,BOOMERANG,BOMB,SHIELD};

#define SWCD 0.5 //sword cooldown
#define SHCD 1 //shield cooldown
#define BMCD 2 //boomerang cooldown
#define BMBCD 3.2 //Bomb cooldown

class Items {
public:
	Items();
	Items(Player *a_player);
	~Items();

	float I1Cldwn;
	float I2Cldwn;
	
	void useItem1(ITEM item);
	void useItem2(ITEM item);
	
	void useSword();
	void useShield();
	void useBoomerang();
	void useBomb();
	void updateBomb(float dT);
	void updateBoom(float dT);
	void update(float dT);
	FRect createRect(float width, float height);
	Player* player;
	
	Entity boom;
	FRect boomRect;
	ClDwn boomCldwn;
	
	Entity bomb;
	ClDwn bombCldwn;
	FRect bombRect;
	// Explosion animation controls
//	ClDwn explosion;
//	Texture explosionTexture;
//	Animation explosionAnimation;
	
	int bombState;
	int boomState;
	void render();
	//bool checkCollision(Entity *item,Entity *other);

	void boomSeek(V2DF target);
	void boomCollision();
	void release();
};