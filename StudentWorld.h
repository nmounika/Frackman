#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include <string>
#include <vector>

class Actor;
class Dirt;
class FrackMan;

enum ObjectType {eBoulder = 0, eGoldNugget = 1, eBarrel = 2, ePool = 3, eSonarKit = 4};

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir);

	bool clearDirt(int x, int y);

	virtual int init();

	FrackMan* getPlayer() const
	{
		return m_frackman;
	}

	void gotBarrel()
	{
		m_numBarrels--;
	}

	static StudentWorld* getWorld()
	{
		return instance;
	}

	virtual int move();

	int moveinMaze(int startX, int startY, int endX, int endY, int &nextX, int &nextY, GraphObject::Direction &newD);

	void makeObjectsVisible(int x, int y, double radius);

	Actor* findClosestProtestor(int x, int y) const;

	bool canMove(int &x, int &y, bool canDig, GraphObject::Direction dir, Actor* ignore = nullptr);

	virtual void cleanUp();
	
	void createSquirt(int x, int y, GraphObject::Direction dir);

	void createTempGoldNugget(int x, int y);

	virtual ~StudentWorld();

private:
	void createInitObjects(int num, ObjectType o);
	int getNumProtestors() const;
	void createOneObject(ObjectType o);
	bool isClear(int x, int y);
	void createDirt();
	void getRandCoord(int &x, int &y);
	bool isFilled(int x, int y);
	void getRandCoordForPool(int &x, int &y);
	std::string formatString();
	bool isActorWithinDistance(int x, int y, double distance);
	void cleanUpAfterMove();
	Dirt* m_dirt[VIEW_WIDTH][VIEW_HEIGHT];
	std::vector<Actor*> m_actors;
	FrackMan* m_frackman;
	static StudentWorld* instance;
	int m_numBarrels;
	int m_goodieinterval;
	int m_protestorinterval;
};

#endif // STUDENTWORLD_H_
