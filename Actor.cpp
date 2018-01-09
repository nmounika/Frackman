#include "Actor.h"
#include "StudentWorld.h"
#include "GameController.h"
#include <algorithm>
using namespace std;

#define PI 3.14159265

/////////////////////////////////////////////////////////////////////////////////////////////////////

FrackMan::FrackMan()
	:Actor(IID_PLAYER, 30, 60, right, 1, 0, true)
{
	moveTo(30, 60);
	m_needToPlough = false;
	m_hitpoints = 10;
	m_waterunits = 5;
	m_sonarkits = 1;
	m_goldnuggets = 0;
}

bool FrackMan::canbeAnnoyed() const
{
	return true;
}

void FrackMan::hitByBoulder()
{
	setDead();
}

void FrackMan::doSomething()
{
	ploughDirt();

	int m;
	bool success = false;

	if (StudentWorld::getWorld()->getKey(m) == true)
	{
		// user hit a key this tick!
		switch (m)
		{
		case KEY_PRESS_LEFT:
			success = stepMove(left);
			break;
		case KEY_PRESS_RIGHT:
			success = stepMove(right);
			break;
		case KEY_PRESS_UP:
			success = stepMove(up);
			break;
		case KEY_PRESS_DOWN:
			success = stepMove(down);
			break;
		case KEY_PRESS_SPACE:
			success = doSquirt();
			break;
		case KEY_PRESS_TAB:
			success = bribeGold();
			break;
		case 'Z':
		case 'z':
			success = useSonar();
			break;
		case KEY_PRESS_ESCAPE:
			success = handleEscape();
			break;
		}
	}

	if (success == false)
		return;
}

bool FrackMan::handleEscape()
{
	setDead();
	return true;
}

bool FrackMan::bribeGold()
{
	if (m_goldnuggets > 0)
	{
		StudentWorld::getWorld()->createTempGoldNugget(getX(), getY());
		m_goldnuggets--;
		return true;
	}
	return false;
}

bool FrackMan::useSonar()
{
	if (m_sonarkits > 0)
	{
		StudentWorld::getWorld()->makeObjectsVisible(getX(), getY(), 12);
		m_sonarkits--;
		return true;
	}
	return false;
}

bool FrackMan::doSquirt()
{
	if (m_waterunits <= 0)
		return false;
	else
	{
		GameController::getInstance().playSound(SOUND_PLAYER_SQUIRT);
		m_waterunits--;

		int x = getX();
		int y = getY();

		for (unsigned int i = 0; i < 4; i++)
		{
			if (!StudentWorld::getWorld()->canMove(x, y, false, getDirection()))
				return false;
		}
		
		StudentWorld::getWorld()->createSquirt(x, y, getDirection());
		return true;
	}
}

void FrackMan::ploughDirt()
{
	if (m_needToPlough)
	{
		m_needToPlough = false;
		int x = getX();
		int y = getY();
		if (StudentWorld::getWorld()->clearDirt(x, y))
		{
			GameController::getInstance().playSound(SOUND_DIG);
		}
	}
}

void FrackMan::getGoodie(GoodieType goodie)
{
	switch (goodie)
	{
	case eG_GoldNugget:
		StudentWorld::getWorld()->increaseScore(10);
		m_goldnuggets++;
		break;
	case eG_Barrel:
		StudentWorld::getWorld()->increaseScore(1000);
		StudentWorld::getWorld()->gotBarrel();
		break;
	case eG_SonarKit:
		StudentWorld::getWorld()->increaseScore(75);
		m_sonarkits++;
		break;
	case eG_WaterPool:
		StudentWorld::getWorld()->increaseScore(100);
		m_waterunits += 5;
		break;
	}
}

bool FrackMan::stepMove(Direction dir)
{
	if (getDirection() != dir)
	{
		setDirection(dir);
		return true;
	}

	int x = getX();
	int y = getY();

	if (StudentWorld::getWorld()->canMove(x, y, true, dir))
	{
		m_needToPlough = true;
		moveTo(x, y);
		return true;
	}
	return false;
}

void FrackMan::annoy()
{
	m_hitpoints = m_hitpoints - 2;
	GameController::getInstance().playSound(SOUND_PLAYER_ANNOYED);
	if (m_hitpoints <= 0)
	{
		GameController::getInstance().playSound(SOUND_PLAYER_GIVE_UP);
		setDead();
	}
}

FrackMan::~FrackMan()
{}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

Boulder::Boulder(int startX, int startY)
	:Actor(IID_BOULDER, startX, startY, down, 1, 1, true)
{
	m_state = eStable;
}

void Boulder::doSomething()
{
	int x = getX();
	int y = getY();

	switch (m_state)
	{
	case eStable:
	{
		if (!StudentWorld::getWorld()->canMove(x, y, false, GraphObject::down, this))
			return;
		else
		{
			m_state = eWaiting;
			setSleepTicks(30);
		}
		break;
	}
	case eWaiting:
	{
		m_state = eFalling;
		GameController::getInstance().playSound(SOUND_FALLING_ROCK);
		break;
	}
	case eFalling:
	{
		Actor* prot = nullptr;
		do
		{
			prot = StudentWorld::getWorld()->findClosestProtestor(x, y);
			if (prot != nullptr)
				prot->hitByBoulder();
		} while (prot != nullptr);

		FrackMan* fm = StudentWorld::getWorld()->getPlayer();
		double distance = fm->distanceFrom(x, y); //find distance from squirt object
		if (distance <= 3)
			fm->hitByBoulder();

		if (StudentWorld::getWorld()->canMove(x, y, false, down, this))
			moveTo(x, y);
		else
			setDead();

		break;
	}
	}
}

bool Boulder::willBlock(int x, int y, bool canDig)
{
	if (distanceFrom(x, y) <= 3)
		return true;
	else
		return false;
}

Boulder::~Boulder()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

Squirt::Squirt(int startX, int startY, Direction dir)
	:Actor(IID_WATER_SPURT, startX, startY, dir, 1, 1, true)
{
	m_traveldist = 4;
	m_dir = dir;
}

void Squirt::doSomething()
{
	Actor* prot = StudentWorld::getWorld()->findClosestProtestor(getX(), getY());
	if (prot != nullptr)
	{
		prot->annoy();
		setDead();
	}

	int x = getX();
	int y = getY();

	if (StudentWorld::getWorld()->canMove(x, y, false, m_dir))
		moveTo(x, y);
	else
		setDead();

	m_traveldist--;
	if (m_traveldist <= 0)
		setDead();
}

Squirt::~Squirt()
{}

/////////////////////////////////////////////////////////////////////////////////////////////////////

Goodie::Goodie(int imageID, int startX, int startY, bool visible, GoodieType goodie)
	:Actor(imageID, startX, startY, right, 1, 2, visible)
{
	m_goodie = goodie;
}

void Goodie::doSomething()
{
	FrackMan* fm = StudentWorld::getWorld()->getPlayer();
	if (fm != nullptr)
	{
		int x = getX();
		int y = getY();

		double distance = fm->distanceFrom(x, y);

		if (!isVisible() && distance <= 4)
		{
			setVisible(true);
			return;
		}
		else if (distance <= 3)
		{
			setDead();
			switch (m_goodie)
			{
			case eG_GoldNugget:
				GameController::getInstance().playSound(SOUND_GOT_GOODIE);
				fm->getGoodie(eG_GoldNugget);
				break;
			case eG_Barrel:
				GameController::getInstance().playSound(SOUND_FOUND_OIL);
				fm->getGoodie(eG_Barrel);
				break;
			case eG_SonarKit:
				GameController::getInstance().playSound(SOUND_GOT_GOODIE);
				fm->getGoodie(eG_SonarKit);
				break;
			case eG_WaterPool:
				GameController::getInstance().playSound(SOUND_GOT_GOODIE);
				fm->getGoodie(eG_WaterPool);
				break;
			}

		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Barrel::Barrel(int startX, int startY)
	:Goodie(IID_BARREL, startX, startY, false, eG_Barrel)
{}

Barrel::~Barrel()
{}

/////////////////////////////////////////////////////////////////////////////////////////////////////

GoldNugget::GoldNugget(int startX, int startY)
	:Goodie(IID_GOLD, startX, startY, false, eG_GoldNugget)
{}

GoldNugget::~GoldNugget()
{}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

SonarKit::SonarKit(int lifetime)
	:TempGoodie(IID_SONAR, 0, 60, lifetime, eG_SonarKit)
{}

SonarKit::~SonarKit()
{}

/////////////////////////////////////////////////////////////////////////////////////////////////////

WaterPool::WaterPool(int startX, int startY, int lifetime)
	:TempGoodie(IID_WATER_POOL, startX, startY, lifetime, eG_WaterPool)
{}

WaterPool::~WaterPool()
{}

////////////////////////////////////////////////////////////////////////////////////////////////////

TempGoodie::TempGoodie(int imageID, int startX, int startY, int lifetime, GoodieType goodie)
	:Goodie(imageID, startX, startY, true, goodie)
{
	m_lifetime = lifetime;
}

void TempGoodie::doSomething()
{
	Goodie::doSomething();
	decLifetime();
}

void TempGoodie::decLifetime()
{
	m_lifetime--;

	if (m_lifetime <= 0)
		setDead();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

TempGoldNugget::TempGoldNugget(int startX, int startY, int lifetime)
	:TempGoodie(IID_GOLD, startX, startY, lifetime, eG_GoldNugget)
{
}

void TempGoldNugget::doSomething()
{
	Actor* prot = StudentWorld::getWorld()->findClosestProtestor(getX(), getY());
	if (prot != nullptr)
	{
		GameController::getInstance().playSound(SOUND_PROTESTER_FOUND_GOLD);
		prot->bribeWithGold();
		setDead();
	}

	decLifetime();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

Protestor::Protestor(int annoyscore, int hitpoints, int imageID)
	:Actor(imageID, 60, 60, left, 1, 0, true)
{
	m_numSquaresToMoveInCurrentDirection = 8 + (rand() % 53);
	setSleepTicks(std::max(0,static_cast<int>(3 - StudentWorld::getWorld()->getLevel() / 4)));
	m_hitpoints = hitpoints;
	m_gaveUp = false;
	m_shoutticks = 0;
	m_annoyscore = annoyscore;
	m_turnticks = 200;
}

void Protestor::hitByBoulder()
{
	m_gaveUp = true;
	GameController::getInstance().playSound(SOUND_PROTESTER_GIVE_UP);
	setSleepTicks(0);
	StudentWorld::getWorld()->increaseScore(500);
}

bool Protestor::isFacingFrackMan() //used only when fm & prot are <=4 units away
{
	GraphObject::Direction protD = getDirection();
	int protX = getX();
	int protY = getY();
	FrackMan* fm = StudentWorld::getWorld()->getPlayer();
	int fmX = fm->getX();
	int fmY = fm->getY();

	double angle = atan2(static_cast<double>(fmY - protY), static_cast<double>(fmX - protX)) * 180 / PI;

	if (angle <= 45.0 && angle > 315.0 && protD == GraphObject::right)
		return true;
	if (angle > 45.0 && angle <= 135.0 && protD == GraphObject::up)
		return true;
	if (angle > 135.0 && angle <= 225.0 && protD == GraphObject::left)
		return true;
	if (angle > 225.0 && angle <= 315.0 && protD == GraphObject::down)
		return true;

	return false;
}

bool Protestor::isFrackManInLineOfSight(GraphObject::Direction &dir, int &newX, int &newY)
{
	int x = getX();
	int initX = getX();
	int y = getY();
	int initY = getY();
	FrackMan* fm = StudentWorld::getWorld()->getPlayer();

	//UP
	while (StudentWorld::getWorld()->canMove(x, y, false, up))
	{
		if (fm->getX() == x && fm->getY() == y)
		{
			dir = up;
			newX = initX;
			newY = initY + 1;
			return true;
		}
	}
	x = initX;
	y = initY;

	//DOWN
	while (StudentWorld::getWorld()->canMove(x, y, false, down))
	{
		if (fm->getX() == x && fm->getY() == y)
		{
			newX = initX;
			newY = initY - 1;
			dir = down;
			return true;
		}
	}
	x = initX;
	y = initY;

	//LEFT
	while (StudentWorld::getWorld()->canMove(x, y, false, left))
	{
		if (fm->getX() == x && fm->getY() == y)
		{
			newX = initX - 1;
			newY = initY;
			dir = left;
			return true;
		}
	}
	x = initX;
	y = initY;

	//RIGHT
	while (StudentWorld::getWorld()->canMove(x, y, false, right))
	{
		if (fm->getX() == x && fm->getY() == y)
		{
			newX = initX + 1;
			newY = initY;
			dir = right;
			return true;
		}
	}
	x = initX;
	y = initY;

	return false;
}

bool Protestor::turnAtIntersection(GraphObject::Direction &dir)
{
	if (m_turnticks > 0)
		return false;

	GraphObject::Direction curdir = getDirection();
	GraphObject::Direction newDir1, newDir2;
	if (curdir == up || curdir == down)
	{
		newDir1 = left;
		newDir2 = right;
	}
	else if (curdir == left || curdir == right)
	{
		newDir1 = up;
		newDir2 = down;
	}

	int x = getX();
	int prevX = getX();
	int y = getY();
	int prevY = getY();

	bool canMove1 = StudentWorld::getWorld()->canMove(x, y, false, newDir1);
	x = prevX;
	y = prevY;

	bool canMove2 = StudentWorld::getWorld()->canMove(x, y, false, newDir2);
	x = prevX;
	y = prevY;

	if (canMove1 && canMove2)
	{
		int r = rand() % 2;
		if (r == 0)
			dir = newDir1;
		else
			dir = newDir2;
	}
	else if (canMove1 && !canMove2)
	{
		dir = newDir1;
	}
	else if (!canMove1 && canMove2)
	{
		dir = newDir2;
	}
	else
		return false;

	return true;
}

void Protestor::doSomething()
{
	if (m_shoutticks > 0)
		m_shoutticks--;

	if (m_turnticks > 0)
		m_turnticks--;

	setSleepTicks(std::max(0, static_cast<int>(3 - StudentWorld::getWorld()->getLevel() / 4)));

	if (m_gaveUp)
	{
		if (getX() == 60 && getY() == 60)
		{
			setDead();
			return;
		}

		GraphObject::Direction newD;
		int newX;
		int newY;

		// could have been hit by a boulder which could be blocking the way temporarily
		if (StudentWorld::getWorld()->moveinMaze(getX(), getY(), 60, 60, newX, newY, newD) != 99999)
		{
			setDirection(newD);
			moveTo(newX, newY);
		}
		return;
	}

		/*********************SHOUT AT FRACKMAN******************************/
		FrackMan* fm = StudentWorld::getWorld()->getPlayer();
		double distance = fm->distanceFrom(getX(), getY());

		if (distance <= 4 && isFacingFrackMan() && m_shoutticks == 0)
		{
			GameController::getInstance().playSound(SOUND_PROTESTER_YELL);
			m_shoutticks = 15;
			fm->annoy();
			return;
		}

		/***********************FOLLOW FRACKMAN**********************/
		GraphObject::Direction d;
		int x, y;
		bool isHe = isFrackManInLineOfSight(d, x, y);
		int posX = getX();
		int posY = getY();

		if (distance > 4 && quickSeekFrackMan())
		{
			return;
		}

		if (distance > 4 && isHe)
		{
			setDirection(d);
			moveTo(x, y); //move one step in direction d
			m_numSquaresToMoveInCurrentDirection = 0;
			return;
		}

		/***********************CHANGE DIRECTIONS***********************/
		if (!isHe) //frackman is not in vertical/horizontal line of sight
		{
			m_numSquaresToMoveInCurrentDirection--;
			if (m_numSquaresToMoveInCurrentDirection <= 0)
			{
				GraphObject::Direction newD = getDirection();
				int curX;
				int initX = getX();
				int curY;
				int initY = getY();

				// the protester could be in the way of a falling boulder
				// and there may be no direction in which he could move
				// detect that case and let the protester die
				bool canMove[4];

				for (int i = 0; i < 4; i++)
					canMove[i] = false;

				curX = initX;
				curY = initY;
				canMove[0] = StudentWorld::getWorld()->canMove(curX, curY, false, up);
				curX = initX;
				curY = initY;
				canMove[1] = StudentWorld::getWorld()->canMove(curX, curY, false, down);
				curX = initX;
				curY = initY;
				canMove[2] = StudentWorld::getWorld()->canMove(curX, curY, false, left);
				curX = initX;
				curY = initY;
				canMove[3] = StudentWorld::getWorld()->canMove(curX, curY, false, right);

				if (!canMove[0] && !canMove[1] && !canMove[2] & !canMove[3])
				{
					m_numSquaresToMoveInCurrentDirection = 0;
					return;    // don't do anything this tick
				}

				int curDir = 0;

				switch (getDirection())
				{
				case up: curDir = 0; break;
				case down: curDir = 1; break;
				case left: curDir = 2; break;
				case right: curDir = 3; break;
				}

				bool canMoveInOther = false;
				for (int i = 0; i < 4; i++)
				{
					if (i != curDir && canMove[i])
						canMoveInOther = true;
				}

				if (canMoveInOther)
				{
					int r = 0;
					do
					{
						r = rand() % 4;
					} while (!canMove[r]);

					switch (r)
					{
					case 0: newD = up; break;
					case 1: newD = down; break;
					case 2: newD = left; break;
					case 3: newD = right; break;
					}
				}

				setDirection(newD);
				m_numSquaresToMoveInCurrentDirection = 8 + rand() % 53;
			}
		}
		/***************************TURNING AT INTERSECTION***********************/
		if (turnAtIntersection(d))
		{
			setDirection(d);
			m_numSquaresToMoveInCurrentDirection = 8 + rand() % 53;
			m_turnticks = 200;
		}

		/***************************CAN'T TRAVEL IN SAME DIRECTION*************************/
		int thisX = getX();
		int thisY = getY();
		if (!StudentWorld::getWorld()->canMove(thisX, thisY, false, getDirection()))
		{
			m_numSquaresToMoveInCurrentDirection = 0;
			return;
		}

		/***************************CONTINUE IN SAME DIRECTION****************************/
		int currentX = getX();
		int currentY = getY();
		StudentWorld::getWorld()->canMove(currentX, currentY, false, getDirection());
		moveTo(currentX, currentY);

}

void Protestor::bribeWithGold()
{
	StudentWorld::getWorld()->increaseScore(25);
	m_gaveUp = true;
	setSleepTicks(0);
}

bool Protestor::canbeBribed() const
{
	return (isAlive() && !m_gaveUp);
}

bool Protestor::canbeAnnoyed() const
{
	return (isAlive() && !m_gaveUp);
}

void Protestor::annoy() //with squirt
{
	m_hitpoints -= 2;
	if (m_hitpoints <= 0)
	{
		m_gaveUp = true;
		GameController::getInstance().playSound(SOUND_PROTESTER_GIVE_UP);
		setSleepTicks(0);
		StudentWorld::getWorld()->increaseScore(m_annoyscore);
	}
	else
	{
		GameController::getInstance().playSound(SOUND_PROTESTER_ANNOYED);
		setSleepTicks(min(50, 100 - static_cast<int>(StudentWorld::getWorld()->getLevel()) * 10));
	}
}

bool Protestor::quickSeekFrackMan()
{
	return false;
}

Protestor::~Protestor()
{}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

Hardcore::Hardcore()
	:Protestor(250, 20, IID_HARD_CORE_PROTESTER)
{
}

bool Hardcore::quickSeekFrackMan() //already found >4 
{
	int M = 16 + StudentWorld::getWorld()->getLevel() * 2;
	int protX = getX();
	int protY = getY();
	FrackMan* fm = StudentWorld::getWorld()->getPlayer();
	int fmX = fm->getX();
	int fmY = fm->getY();
	int newX, newY;
	GraphObject::Direction newD;

	int steps = StudentWorld::getWorld()->moveinMaze(protX, protY, fmX, fmY, newX, newY, newD);
	if (steps <= M)
	{
		setDirection(newD);
		moveTo(newX, newY);
		return true;
	}

	return false;
}

void Hardcore::bribeWithGold()
{
	StudentWorld::getWorld()->increaseScore(50);
	setSleepTicks(min(50, 100 - static_cast<int>(StudentWorld::getWorld()->getLevel()) * 10));
}

Hardcore::~Hardcore()
{}

//////////////////////////////////////////////////////////////////////////////////////////////////////////