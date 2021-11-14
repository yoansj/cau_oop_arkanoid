#include "GameManager.h"

GameManager::GameManager() {
	this->m_life = 3;
	this->m_score = 0;
	this->m_bonus = 25;
	this->m_isOver = false;
	this->m_balls = 20;
}

int GameManager::getLife() {
	return m_life;
}

int GameManager::getScore() {
	return m_score;
}

int GameManager::getBonus() {
	return m_bonus;
}

int GameManager::getBalls() {
	return m_balls;
}

bool GameManager::getIsOver() {
	if (m_life <= 0)
	{
		m_isOver = true;
	}
	else {
		m_isOver = false;
	}
	return m_isOver;
}

void GameManager::addScore(int numberOfBallDestruct) {
	for (size_t i = 0; i < numberOfBallDestruct; i++)
	{
		m_score += 100 + m_bonus * i;
	}
}

void GameManager::addLife(int lifeAdd) {
	m_life += lifeAdd;
}

void GameManager::removeLife(int lifeLose) {
	m_life -= lifeLose;
}

void GameManager::removeBalle() {
	m_balls -= 1;
}

const char* GameManager::gameOver() {
	if (m_isOver) {
		return "GAMEOVER !";
	}
}

const char* GameManager::victory() {
	if (!m_isOver && m_balls <=0) {
		return "Victory !";
	}
}

const char *  GameManager::printScore() {
	std::string temp = std::to_string(m_score);
	char const *result = temp.c_str();
	return result;
}

const char* GameManager::printLife() {
	std::string temp = std::to_string(m_life);
	char const* result = temp.c_str();
	return result;
}