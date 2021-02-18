#ifndef GAMERESULT_H
#define GAMERESULT_H


enum Player {
    NONE,
    PLAYER_1,
    PLAYER_2,
};

class GameResult {
  public:
  Player winner;
  uint32_t responseTime;
  GameResult(Player p, unsigned int t): winner(p), responseTime(t) {};
};




#endif