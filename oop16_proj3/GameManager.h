#pragma once
//Include
#include "d3dUtility.h"
#include <string>

class GameManager
{
private:
	int m_life;
	int m_score;
	int m_bonus;
	int m_balls;
	bool m_isOver;

public:
	GameManager(); //constructor
	/*
	Default value:
		- life: 3
		- score: 0
		- bonus: 25
		- isOver: false
		- balles : 20
	*/
	int getLife();
	int getScore();
	int getBonus();
	int getBalls();
	bool getIsOver();
	void addScore(int numberOfBallDestruct);
	void addLife(int lifeAdd = 1); //default 1
	void removeLife(int lifeLose = 1); //default 1
	void removeBalle();
	const char* gameOver();
	const char* victory();
	const char* printScore();
	const char* printLife();
};

