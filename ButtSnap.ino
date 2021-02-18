#include <stdlib.h>
#include <avr/interrupt.h>
#include "RandomGenerator.h"
#include "GameResult.h"
#include "DebouncedButton.h"
#include <EEPROM.h>



/*
 * This is a speed game for 2 players
 * The board has one central LED (Trigger)
 * Each player has a button and an LED to indicate if he is winning the game
 * 
 * When the trigger led lights, the players have to rush to press their buttons.
 * The first one to press it, wins a point. 
 * To indicate the point won, the winners LED flashes a number of times
 * The number of flashes indicates how fast he was. Very fast responses are applauded by many led flashes.
 * Throughout the game, the player leds indicate the player with the most points
 * 
 * The beginning of each new round delays a random amount of time.
 * The duration of the trigger pulse is also random. 
 * 
 * Every once in a while after winnong a point, a bonus round is launched. 
 * During the bonus round, the trigger led flashes rapidly.
 * The players must press their buttons as many times as possible, whithin the duration of the bonus round
 * The point goes to the player who pressed his button the most times
 */



/*
 *  Consts
 */

#define BUTTON_DEBOUNCE_COUNTER_MS  25  
 
#define ROUND_DELAY_MS_MIN          300
#define ROUND_DELAY_MS_MAX          8000
#define PULSE_DURATION_MS_MIN       200
#define PULSE_DURATION_MS_MAX       2000

//Bonus round
#define BONUS_ROUND_INV_PROBABILITY 20
#define BONUS_ROUND_DURATION_MS     4000
#define BONUS_ROUND_FLASH_PERIOD_MS 31


//Point winner indication 
#define WINNER_LED_FLASH_PERIOD_MS  50
#define WINNER_LED_FLASH_TIMES_MIN  3
#define WINNER_LED_FLASH_TIMES_MAX  100
#define GOOD_TIME_THRESHOLD_MS      500

//Startup signal Ready Set Go
#define STARTUP_SIGNAL_PERIOD_MS    400

//Show total score
#define SHOW_SCORE_DELAY_MS         1500
#define SCORE_SETS_PERIOD_MS        500
#define SCORE_POINTS_PERIOD_MS      250


//set points
#define DEFAULT_SET_POINTS          10
#define SET_SETPOINTS_DELAY         2000
#define SET_SETPOINTS_FLASH_DELAY   50
#define SET_WINNER_FLASH_DELAY      2000

//highScore
#define HIGH_SCORE_FLASH_DELAY      200


//Random Generator seed 
#define ANALOG_CHANNEL_FOR_SEED     0

//Hardware pins
const int CENTRAL_LED      = 9;
const int PLAYER_1_LED     = 5;
const int PLAYER_2_LED     = 6;
const int PLAYER_1_BUTTON  = 2; //ext int0
const int PLAYER_2_BUTTON  = 3; //ext int1


/*
 * Global variables
 */
uint8_t player1Points, player2Points;
uint8_t player1Sets, player2Sets;
uint8_t setPoints;


uint32_t roundDelayTimer;
uint32_t bothButtonsPressedTimer;

RandomGenerator roundDelayGen(ROUND_DELAY_MS_MIN, ROUND_DELAY_MS_MAX);
RandomGenerator pulseDurationGen(PULSE_DURATION_MS_MIN, PULSE_DURATION_MS_MAX);
const int outcomes[] = {true,false};
const unsigned int outcomeWeights[] = {1, BONUS_ROUND_INV_PROBABILITY - 1};
WeightedRandomGenerator playBonusRoundGen(2, outcomes, outcomeWeights);

DebouncedButton player1Button (PLAYER_1_BUTTON, BUTTON_DEBOUNCE_COUNTER_MS, 1);
DebouncedButton player2Button (PLAYER_2_BUTTON, BUTTON_DEBOUNCE_COUNTER_MS, 1);


/*
 * ISRs
 */
ISR(TIMER1_COMPA_vect) {
  player1Button.sample();
  player2Button.sample();
} 

/*
 * Functions
 */
GameResult playRound(uint32_t pulseDuration){
  digitalWrite(CENTRAL_LED, HIGH);
  Player faster = NONE;
  player1Button.clearChanges();
  player2Button.clearChanges();
  uint32_t timer = millis();
  
  while ((millis() - timer < pulseDuration)){
    if(player1Button.changedToLow() && player2Button.changedToLow()){
      break;
    }
    if (player1Button.changedToLow() && !player2Button.changedToLow()){
      faster = PLAYER_1;
      break;
    }
    if (player2Button.changedToLow() && !player1Button.changedToLow()){
      faster = PLAYER_2;
      break;
    }
  }
  
  digitalWrite(CENTRAL_LED, LOW);
  return GameResult(faster, millis() - timer);
}

GameResult playBonusRound(){
  uint8_t player1Clicks = 0, player2Clicks = 0;
  uint32_t timer = millis();
  player1Button.clearChanges();
  player2Button.clearChanges();
  
  while (millis() - timer < BONUS_ROUND_DURATION_MS){
    
    //keep the central led flashing
    if ((millis() / BONUS_ROUND_FLASH_PERIOD_MS) & 1){
      digitalWrite(CENTRAL_LED, HIGH);
    } else {
      digitalWrite(CENTRAL_LED, LOW);
    }

    if (player1Button.changedToLowAndClear()){
      player1Clicks++;
    } 
    if (player2Button.changedToLowAndClear()){
      player2Clicks++;
    } 
    
  }
  
  digitalWrite(CENTRAL_LED, LOW);

  if ( player1Clicks > EEPROM.read(1)){
    EEPROM.write(1, player1Clicks);
  }

  if ( player2Clicks > EEPROM.read(2)){
    EEPROM.write(2, player2Clicks);
  }

  if (player1Clicks > player2Clicks){
    return GameResult(PLAYER_1, BONUS_ROUND_DURATION_MS / player1Clicks);
  } else if (player2Clicks > player1Clicks){
    return GameResult(PLAYER_2, BONUS_ROUND_DURATION_MS / player2Clicks);
  } else {
    return GameResult(NONE, 0);
  }
}


void flashPlayerLed(int ledNumber, int numberOfFlashes){
  digitalWrite(ledNumber, LOW);
  delay(WINNER_LED_FLASH_PERIOD_MS);
  for (int i = 0; i < numberOfFlashes; i++){
    digitalWrite(ledNumber, HIGH);
    delay(WINNER_LED_FLASH_PERIOD_MS);
    digitalWrite(ledNumber, LOW);
    delay(WINNER_LED_FLASH_PERIOD_MS);
  }
}

//The number of times that the winner led will flash depends on how quick the winner's response was
int getNumberOfWinningLedFlashes(uint32_t responseTime){
  if (responseTime == 0){
    responseTime = 1;
  }
  int ret = (GOOD_TIME_THRESHOLD_MS / responseTime) + WINNER_LED_FLASH_TIMES_MIN;
  return min(ret, WINNER_LED_FLASH_TIMES_MAX);
}

void handleRoundResult(GameResult r){
  switch(r.winner){
    case PLAYER_1:
    flashPlayerLed(PLAYER_1_LED, getNumberOfWinningLedFlashes(r.responseTime));
    player1Points++;
    break;
    case PLAYER_2:
    flashPlayerLed(PLAYER_2_LED, getNumberOfWinningLedFlashes(r.responseTime));
    player2Points++;
    break;
    case NONE:
    break;
  }

  if (player1Points == setPoints){
    player1Sets++;
    player1Points = 0;
    player2Points = 0;
    digitalWrite(PLAYER_1_LED, HIGH);
    delay(SET_WINNER_FLASH_DELAY);
    digitalWrite(PLAYER_1_LED, LOW);
  }

  if (player2Points == setPoints){
    player2Sets++;
    player1Points = 0;
    player2Points = 0;
    digitalWrite(PLAYER_2_LED, HIGH);
    delay(SET_WINNER_FLASH_DELAY);
    digitalWrite(PLAYER_2_LED, LOW);
  }
}

void doReadySetGoSignal(){
  for (int i = 0; i < 3; i++){
    digitalWrite(PLAYER_1_LED, HIGH);
    digitalWrite(PLAYER_2_LED, HIGH);
    delay(STARTUP_SIGNAL_PERIOD_MS);
    digitalWrite(PLAYER_1_LED, LOW);
    digitalWrite(PLAYER_2_LED, LOW);
    delay(STARTUP_SIGNAL_PERIOD_MS);
  }
}



void showTotalScore(){

  //first blink to show the sets
  for (int i = 0; i < max(player1Sets, player2Sets); i++){
    if (i <  player1Sets){
      digitalWrite(PLAYER_1_LED, HIGH);
    }
    if (i <  player2Sets){
      digitalWrite(PLAYER_2_LED, HIGH);
    }
    delay(SCORE_SETS_PERIOD_MS);
    digitalWrite(PLAYER_1_LED, LOW);
    digitalWrite(PLAYER_2_LED, LOW);
    delay(SCORE_SETS_PERIOD_MS);
  }
  delay(SCORE_SETS_PERIOD_MS);

  //then blink to show the points of the current set
  for (int i = 0; i < max(player1Points, player2Points); i++){
    if (i <  player1Points){
      digitalWrite(PLAYER_1_LED, HIGH);
    }
    if (i <  player2Points){
      digitalWrite(PLAYER_2_LED, HIGH);
    }
    delay(SCORE_POINTS_PERIOD_MS);
    digitalWrite(PLAYER_1_LED, LOW);
    digitalWrite(PLAYER_2_LED, LOW);
    delay(SCORE_POINTS_PERIOD_MS);
  }
}

bool bothButtonsPressed(){
  return ((digitalRead(PLAYER_1_BUTTON) == LOW) && (digitalRead(PLAYER_2_BUTTON) == LOW));
}

void setTimer1At1kHz(){
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = (F_CPU / 1000) - 1; // we want 1ms intervals
  TCCR1B |= (1<<WGM12); //CTC mode, top value OCR1A
  TCCR1B |= (1<<CS10); //Run with no prescaler
  TIMSK1 |= (1 <<  OCIE1A); //enable interrupt at compare match
}

void readSetPointsFromEEPROM(){
  setPoints = EEPROM.read(0);
  if (setPoints == 255){
    setPoints = DEFAULT_SET_POINTS;
  }
}

bool writeSetPointsToEEPROMIfRequested(){

  uint8_t counter = 0;
  uint32_t timer;
  if ((digitalRead(PLAYER_1_BUTTON) == LOW)){
    player1Button.clearChanges();
    timer = millis();
    while(millis() - timer < SET_SETPOINTS_DELAY){
      if (player1Button.changedToLowAndClear()){
        counter++;
        digitalWrite(PLAYER_1_LED, HIGH);
        delay(SET_SETPOINTS_FLASH_DELAY);
        digitalWrite(PLAYER_1_LED, LOW);
        delay(SET_SETPOINTS_FLASH_DELAY);
        timer = millis();
      }
    }
    if(counter){
      EEPROM.write(0, counter);
    }
    return true;
  }
  return false;
}

bool showHighScoreFromEEPROMIfRequested(){

  uint8_t player1HighScore = EEPROM.read(1);
  if (player1HighScore == 255){
    player1HighScore = 0;
    EEPROM.write(1,0);
  }
  
  uint8_t player2HighScore = EEPROM.read(2);
  if (player2HighScore == 255){
    player2HighScore = 0;
    EEPROM.write(2,0);
  }
  
  if (bothButtonsPressed()){
    
    for (uint8_t i = 0; i< max(player1HighScore, player2HighScore); i++){
      if (i <  player1HighScore){
        digitalWrite(PLAYER_1_LED, HIGH);
      }
      if (i <  player2HighScore){
        digitalWrite(PLAYER_2_LED, HIGH);
      }
      delay(HIGH_SCORE_FLASH_DELAY);
      digitalWrite(PLAYER_1_LED, LOW);
      digitalWrite(PLAYER_2_LED, LOW);
      delay(HIGH_SCORE_FLASH_DELAY);
    }
    delay(1000);

//  //if both buttons are still pressed, delete the high score
//    if (bothButtonsPressed()){
//      EEPROM.write(1, 0);
//      EEPROM.write(2, 0);
//    }
    return true;
  }
  
  return false;
}



/*
 * Setup code
 */
void setup() {
  // hardware initialization
  pinMode(CENTRAL_LED, OUTPUT);
  pinMode(PLAYER_1_LED, OUTPUT);
  pinMode(PLAYER_2_LED, OUTPUT);
  pinMode(PLAYER_1_BUTTON, INPUT_PULLUP);
  pinMode(PLAYER_2_BUTTON, INPUT_PULLUP);
  setTimer1At1kHz();
  RandomGenerator::setAdcNoiseSeed(ANALOG_CHANNEL_FOR_SEED);
  if (!showHighScoreFromEEPROMIfRequested()){
    writeSetPointsToEEPROMIfRequested();
  }
  readSetPointsFromEEPROM();
  doReadySetGoSignal();
  roundDelayTimer = millis() + roundDelayGen.get();
}


/*
 * Main loop
 */
void loop() {
  if (millis() > roundDelayTimer){
    handleRoundResult(playRound(pulseDurationGen.get()));
    if (playBonusRoundGen.get()){
      handleRoundResult(playBonusRound());
    }
    roundDelayTimer = millis() + roundDelayGen.get();
  }
  
  if (bothButtonsPressed()){
    bothButtonsPressedTimer = millis();
    while (bothButtonsPressed()){
      if (millis() -  bothButtonsPressedTimer > SHOW_SCORE_DELAY_MS){
        showTotalScore();
        roundDelayTimer = millis() + roundDelayGen.get();
        break;
      }
    }
  }
  
}
