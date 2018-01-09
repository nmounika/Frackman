#include "StudentWorld.h"
#include "GraphObject.h"
#include "Actor.h"
#include <vector>
#include <queue>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <cassert>
#include <string>
using namespace std;

StudentWorld *StudentWorld::instance = nullptr;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}


StudentWorld::StudentWorld(std::string assetDir)
	: GameWorld(assetDir)
{
	instance = this;
	m_numBarrels = 0;
	m_frackman = nullptr;
}


int StudentWorld::init()
{
	m_frackman = new FrackMan();

	m_goodieinterval = getLevel() * 25 + 300;
	m_protestorinterval = std::min(25, 200 - static_cast<int>(getLevel()));

	Protestor* protestor = new Protestor();
	m_actors.push_back(protestor);

	srand(static_cast<unsigned int>(time(NULL)));

	createDirt();

	m_numBarrels = std::min(static_cast<int>(getLevel()) + 2, 20);

	createInitObjects(std::min(static_cast<int>(getLevel()) / 2 + 2, 6), eBoulder);
	createInitObjects(std::max(5 - static_cast<int>(getLevel()) / 2, 2), eGoldNugget);
	createInitObjects(m_numBarrels, eBarrel);

	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::getNumProtestors() const
{
	int numprotestors = 0;
	for (unsigned int i = 0; i < m_actors.size(); i++)
	{
		if (m_actors[i]->canbeAnnoyed())
			numprotestors++;
	}
	return numprotestors;
}

int StudentWorld::move()
{
	if (!m_frackman->isAlive())
		return GWSTATUS_PLAYER_DIED;

	if (m_numBarrels == 0)
	{
		playSound(SOUND_FINISHED_LEVEL);
		return GWSTATUS_FINISHED_LEVEL;
	}

	setGameStatText(formatString());
	m_frackman->doSomething();

	if (m_goodieinterval == 0)
	{
		int r = rand() % 5;
		if (r == 0)
		{
			SonarKit* s = new SonarKit(std::min(100, 300 - 10 * static_cast<int>(getLevel())));
			m_actors.push_back(s);
		}
		else
		{
			int x;
			int y;

			getRandCoordForPool(x, y);
			while (!isClear(x, y))
			{
				getRandCoordForPool(x, y);
			}

			WaterPool* w = new WaterPool(x, y, std::min(100, 300 - 10 * static_cast<int>(getLevel())));
			m_actors.push_back(w);
		}
		m_goodieinterval = getLevel() * 25 + 300;
	}
	else
		m_goodieinterval--;

	if (m_protestorinterval == 0 && getNumProtestors() < std::min(15, static_cast<int>(ceil(2 + getLevel() * 1.5))))
	{
		int probabilityOfHardcore = min(90, static_cast<int>(getLevel()) * 10 + 30);
		int r = rand() % 100;
		Protestor* p = nullptr;

		if (r < probabilityOfHardcore)
			p = new Hardcore();
		else
			p = new Protestor();
		m_actors.push_back(p);

		m_protestorinterval = std::min(25, 200 - static_cast<int>(getLevel()));
	}
	else
		m_protestorinterval--;

	for (unsigned int i = 0; i < m_actors.size(); i++)
	{
		m_actors[i]->processTicks();
	}

	cleanUpAfterMove();

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::createDirt()
{
	for (int i = 0; i < VIEW_WIDTH; i++)
	{
		for (int j = 0; j < VIEW_HEIGHT; j++)
			m_dirt[i][j] = nullptr;
	}

	for (int i = 0; i < 30; i++)
	{
		for (int j = 0; j < VIEW_HEIGHT - 4; j++)
			m_dirt[i][j] = new Dirt(i, j);
	}

	for (int i = 30; i < 34; i++)
	{
		for (int j = 0; j < 4; j++)
			m_dirt[i][j] = new Dirt(i, j);
	}

	for (int i = 34; i < VIEW_WIDTH; i++)
	{
		for (int j = 0; j < VIEW_HEIGHT - 4; j++)
			m_dirt[i][j] = new Dirt(i, j);
	}
}

//clears 4x4 for goodies/boulder
bool StudentWorld::clearDirt(int x, int y)
{
	bool didClear = false;
	for (int i = x; i < x + 4; i++)
	{
		for (int j = y; j < y + 4; j++)
		{
			if (m_dirt[i][j] != nullptr)
			{
				delete m_dirt[i][j];
				m_dirt[i][j] = nullptr;
				didClear = true;
			}
		}
	}
	return didClear;
}

void StudentWorld::getRandCoordForPool(int &x, int &y)
{
	x = rand() % (VIEW_WIDTH - 4);
	y = rand() % (VIEW_HEIGHT - 4);
}

//produces random x, y corrdinates that enable an object of size 4x4 to reside
void StudentWorld::getRandCoord(int &x, int &y)
{
	x = rand() % (VIEW_WIDTH - 4);
	y = 20 + rand() % (VIEW_HEIGHT - 28);
}

void StudentWorld::createOneObject(ObjectType o)
{
	int x, y;
	getRandCoord(x, y);

	while (!isFilled(x, y) || isActorWithinDistance(x, y, 6))
	{
		getRandCoord(x, y);
	}

	Actor* object = nullptr;
	bool clearD = true;

	switch (o)
	{
	case (eBoulder):
		object = new Boulder(x, y);
		break;
	case (eGoldNugget):
		object = new GoldNugget(x, y);
		clearD = false;
		break;
	case (eBarrel):
		object = new Barrel(x, y);
		clearD = false;
		break;
	}

	if (object != nullptr)
		m_actors.push_back(object);
	
	if (clearD)
		clearDirt(x, y);
}

void StudentWorld::createInitObjects(int num, ObjectType o)
{
	for (int i = 0; i < num; i++)
	{
		createOneObject(o);
	}
}

bool StudentWorld::isActorWithinDistance(int x, int y, double distance)
{
	for (unsigned int i = 0; i < m_actors.size(); i++)
	{
		double eDist = m_actors[i]->distanceFrom(x, y);
		if (eDist < distance)
			return true;
	}
	return false;
}

//checks if 4x4 square at x, y is filled with dirt = for putting in boulders
bool StudentWorld::isFilled(int x, int y) //
{
	for (int i = x; i < x + 4; i++)
	{
		for (int j = 0; j < y + 4; j++)
		{
			if (m_dirt[i][j] == nullptr)
				return false;
		}
	}
	return true;
}

void StudentWorld::makeObjectsVisible(int x, int y, double radius)
{
	for (unsigned int i = 0; i < m_actors.size(); i++)
	{
		double distance = m_actors[i]->distanceFrom(x, y);
		if (distance <= radius)
		{
			m_actors[i]->setVisible(true);
		}
	}
}

void StudentWorld::createSquirt(int x, int y, GraphObject::Direction dir)
{
	Squirt* sq = new Squirt(x, y, dir);
	m_actors.push_back(sq);
}

void StudentWorld::createTempGoldNugget(int x, int y)
{
	TempGoldNugget* tgn = new TempGoldNugget(x, y, 100);
	m_actors.push_back(tgn);
}

Actor* StudentWorld::findClosestProtestor(int x, int y) const
{
	for (unsigned int i = 0; i < m_actors.size(); i++)
	{
		if (m_actors[i]->canbeAnnoyed())
		{
			Actor* prot = m_actors[i];
			double distance = prot->distanceFrom(x, y); //find distance from squirt object
			if (distance <= 3)
				return prot;
		}
	}
	return nullptr;
}

int StudentWorld::moveinMaze(int startX, int startY, int endX, int endY, int &nextX, int &nextY, GraphObject::Direction &newD)
{
	class Coord
	{
	public:
		Coord(int xx, int yy, int steps) : m_x(xx), m_y(yy), m_steps(steps) {}
		void setSteps(int steps) { m_steps = steps; }
		int getSteps() const {return m_steps;}
		int x() const { return m_x; }
		int y() const { return m_y; }
	private:
		int m_x;
		int m_y;
		int m_steps;
	};

	queue<Coord> coordMaze;     // declare a queque of Coords

	Coord start(endX, endY, 0);
	Coord end(startX, startY, 0);
	coordMaze.push(start);

	char maze[VIEW_WIDTH][VIEW_HEIGHT];
	for (int i = 0; i < VIEW_WIDTH; i++)
	{
		for (int j = 0; j < VIEW_HEIGHT; j++)
		{
			maze[i][j] = '0';
		}
	}

	while (!coordMaze.empty())
	{
		Coord front = coordMaze.front();
		coordMaze.pop();
		int curx = front.x();
		int cursteps = front.getSteps() + 1;
		int beforeX = front.x();
		int cury = front.y();
		int beforeY = front.y();

		if (maze[curx][cury] == '0')
		{
			maze[curx][cury] = 'X';
			if (canMove(curx, cury, false, GraphObject::up))		// UP
			{
				if (curx == end.x() && cury == end.y())
				{
					nextX = beforeX;
					nextY = beforeY;
					newD = GraphObject::down;
					return cursteps;
				}
				else
				{
					coordMaze.push(Coord(curx, cury, cursteps));
				}
			}

			curx = beforeX;
			cury = beforeY;
			if (canMove(curx, cury, false, GraphObject::left))		// LEFT
			{
				if (curx == end.x() && cury == end.y())
				{
					nextX = beforeX;
					nextY = beforeY;
					newD = GraphObject::right;
					return cursteps;
				}
				else
				{
					coordMaze.push(Coord(curx, cury, cursteps));
				}
			}

			curx = beforeX;
			cury = beforeY;
			if (canMove(curx, cury, false, GraphObject::down))		// DOWN
			{
				if (curx == end.x() && cury == end.y())
				{
					nextX = beforeX;
					nextY = beforeY;
					newD = GraphObject::up;
					return cursteps;
				}
				else
				{
					coordMaze.push(Coord(curx, cury, cursteps));
				}
			}

			curx = beforeX;
			cury = beforeY;
			if (canMove(curx, cury, false, GraphObject::right))		// RIGHT			
			{
				if (curx == end.x() && cury == end.y())
				{
					nextX = beforeX;
					nextY = beforeY;
					newD = GraphObject::left;
					return cursteps;
				}
				else
				{
					coordMaze.push(Coord(curx, cury, cursteps));
				}
			}
		}
	}

	return 99999;
}

bool StudentWorld::isClear(int x, int y)
{
	for (int i = x; i < x + 4; i++)
	{
		for (int j = y; j < y + 4; j++)
		{
			if (m_dirt[i][j] != nullptr)
				return false;
		}
	}
	return true;
}

bool StudentWorld::canMove(int &x, int &y, bool canDig, GraphObject::Direction dir, Actor* ignore)
{
	int newX = x;
	int newY = y;

	switch (dir)
	{
	case GraphObject::up:
		newY++;
		break;
	case GraphObject::down:
		newY--;
		break;
	case GraphObject::left:
		newX--;
		break;
	case GraphObject::right:
		newX++;
		break;
	}

	if (newX < 0 || newX > VIEW_WIDTH - 4 || newY < 0 || newY > VIEW_HEIGHT - 4)
		return false;

	//check protestor case
	if (!canDig && !isClear(newX, newY))
		return false;
	
	for (unsigned int i = 0; i < m_actors.size(); i++)
	{
		if (m_actors[i] != ignore && m_actors[i]->willBlock(newX, newY, canDig)) //checks boulder case
			return false;
	}

	x = newX;
	y = newY;

	return true;
}

StudentWorld::~StudentWorld()
{
	cleanUp();
	delete m_frackman;
}

string StudentWorld::formatString()
{
	int score = getScore();
	int level = getLevel();
	int lives = getLives();
	int health = m_frackman->getHealth();
	int squirts = m_frackman->getNumSquirts();
	int gold = m_frackman->getGoldNuggets();
	int sonar = m_frackman->getSonar();
	int barrels = m_numBarrels;
	char stats[200];

	sprintf_s(stats, 200, "Scr: %06d  Lvl: %2d  Lives: %1d  Hlth: %3d%%  Wtr: %2d  Gld: %2d  Sonar: %2d  Oil Left: %2d", score, level, lives, health, squirts, gold, sonar, barrels);
	
	return stats;
}

void StudentWorld::cleanUp()
{
	delete m_frackman;
	m_frackman = nullptr;

	for (unsigned int i = 0; i < m_actors.size(); i++)
	{
		delete m_actors[i];
	}
	m_actors.clear();

	for (int i = 0; i < VIEW_WIDTH; i++)
	{
		for (int j = 0; j < VIEW_HEIGHT - 4; j++)
		{
			if (m_dirt[i][j] != nullptr)
			{
				delete m_dirt[i][j];
				m_dirt[i][j] = nullptr;
			}
		}
	}
}

bool deleteifObjDead(Actor* a)
{
	if (!a->isAlive())
	{
		delete a;
		return true;
	}
	
	return false;
}

void StudentWorld::cleanUpAfterMove()
{
	if (!m_frackman->isAlive())
		return;
	m_actors.erase(remove_if(m_actors.begin(), m_actors.end(), deleteifObjDead), m_actors.end());
}