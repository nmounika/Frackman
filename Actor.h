#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include <string>

enum GoodieType {eG_GoldNugget = 0, eG_Barrel = 1, eG_WaterPool = 2, eG_SonarKit = 3};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Actor: public GraphObject
{
public:
	Actor(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth, bool visible)
		:GraphObject(imageID, startX, startY, dir, size, depth)
	{
		setVisible(visible);
		m_alive = true;
		m_sleepticks = 0;
	}

	virtual void bribeWithGold()
	{}

	virtual void annoy()
	{}

	virtual void hitByBoulder()
	{}

	void processTicks()
	{
		if (!isAlive())
			return;
		if (m_sleepticks > 0)
			m_sleepticks--;
		else if (m_sleepticks <= 0)
			doSomething();
	}

	virtual void doSomething()
	{}

	virtual bool canbeBribed() const
	{
		return false;
	}

	virtual bool canbeAnnoyed() const
	{
		return false;
	}

	virtual bool willBlock(int x, int y, bool canDig)
	{
		return false;
	}

	double distanceFrom(int x, int y)
	{
		double eDist = sqrt(pow(getX() - x, 2) + pow(getY() - y, 2));
		return eDist;
	}

	void setSleepTicks(int n)
	{
		m_sleepticks = n;
	}

	bool isAlive() const
	{
		return m_alive;
	}

	void setDead()
	{
		m_alive = false;
	}

	//functions from base class you should use
	//int getX(), int getY(), moveTo(x, y), Direction getDirection(), setDirection(Direction d), double getSize()

	virtual ~Actor()
	{}

private:
	bool m_alive;
	int m_sleepticks;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Dirt : public Actor
{
public:
	Dirt(int startX, int startY)
		:Actor(IID_DIRT, startX, startY, right, 0.25, 3, true)
	{}

	virtual bool willBlock(int x, int y, bool canDig)
	{
		return !canDig;
	}

	virtual ~Dirt()
	{}

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

class FrackMan : public Actor
{
public:
	FrackMan();

	void ploughDirt();

	bool stepMove(Direction dir);

	bool doSquirt();

	bool useSonar();

	void getGoodie(GoodieType goodie);

	bool bribeGold();
	
	void hitByBoulder();

	bool handleEscape();

	virtual void doSomething();

	virtual bool canbeAnnoyed() const;

	int getGoldNuggets() const
	{
		return m_goldnuggets;
	}

	int getNumSquirts() const
	{
		return m_waterunits;
	}

	int getHealth() const
	{
		if (m_hitpoints <= 10)
			return (m_hitpoints * 10);
		else
			return 100;
	}

	int getSonar() const
	{
		return m_sonarkits;
	}

	virtual void annoy();

	virtual ~FrackMan();

private:
	bool m_needToPlough;
	int m_hitpoints;
	int m_waterunits;
	int m_sonarkits;
	int m_goldnuggets;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Boulder :public Actor
{
public:
	Boulder(int startX, int startY);

	enum State { eStable, eWaiting, eFalling };

	virtual void doSomething();

	virtual bool willBlock(int x, int y, bool canDig);

	virtual ~Boulder();

private:
	State m_state;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Squirt :public Actor
{
public:
	Squirt(int startX, int startY, Direction dir);

	virtual void doSomething();

	virtual ~Squirt();

private:
	int m_traveldist;
	GraphObject::Direction m_dir;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Goodie :public Actor
{
public:
	Goodie(int imageID, int startX, int startY, bool visible, GoodieType goodie);

	virtual void doSomething();

	virtual ~Goodie()
	{}

private:
	GoodieType m_goodie;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Barrel :public Goodie
{
public: 
	Barrel(int startX, int startY);

	virtual ~Barrel();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GoldNugget :public Goodie
{
public:
	GoldNugget(int startX, int startY);

	virtual ~GoldNugget();

private:
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TempGoodie :public Goodie
{
public: 
	TempGoodie(int imageID, int startX, int startY, int lifetime, GoodieType goodie);

	virtual void doSomething();

	void decLifetime();

private:
	int m_lifetime;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////

class SonarKit :public TempGoodie
{
public:
	SonarKit(int lifetime);

	virtual ~SonarKit();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WaterPool :public TempGoodie
{
public:
	WaterPool(int startX, int startY, int lifetime);

	virtual ~WaterPool();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TempGoldNugget :public TempGoodie
{
public:
	TempGoldNugget(int startX, int startY, int lifetime);

	virtual void doSomething();
};

class Protestor :public Actor
{
public:
	Protestor(int annoyscore = 100, int hitpoints = 5, int imageID = IID_PROTESTER);

	virtual void doSomething();

	virtual void hitByBoulder();
	
	virtual bool canbeBribed() const;
	
	virtual void bribeWithGold();

	virtual bool quickSeekFrackMan();

	bool isFacingFrackMan();

	virtual bool canbeAnnoyed() const;

	bool isFrackManInLineOfSight(GraphObject::Direction &dir, int &newX, int &newY);

	bool turnAtIntersection(GraphObject::Direction &dir);

	virtual void annoy();

	virtual ~Protestor();

private:
	int m_numSquaresToMoveInCurrentDirection;
	int m_hitpoints;
	bool m_gaveUp;
	int m_shoutticks;
	int m_turnticks;
	int m_annoyscore;
};

class Hardcore :public Protestor
{
public:
	Hardcore();

	virtual void bribeWithGold();

	virtual bool quickSeekFrackMan();

	virtual ~Hardcore();
private: 
};

#endif // ACTOR_H_
