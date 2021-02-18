#include "RandomGenerator.h"
#include "Arduino.h"


void RandomGenerator::setSeed(uint32_t seed){
	randomSeed(seed);
}

void RandomGenerator::setAdcNoiseSeed(int analogChannel){
	uint32_t ret = 0;  
  	for (int i = 0; i < 32; i++){
    	ret <<= 1;
    	ret |= (analogRead(analogChannel) & 1);
    	delay(1);
  	}
  	RandomGenerator::setSeed(ret);
}

uint32_t RandomGenerator::get(uint32_t minimum, uint32_t maximum){
  return random(minimum, maximum);
}


RandomGenerator::RandomGenerator(): _min(0), _max(RAND_MAX) {}

RandomGenerator::RandomGenerator(uint32_t maximum): _min(0), _max(maximum) {}

RandomGenerator::RandomGenerator(uint32_t minimum, uint32_t maximum): _min(minimum), _max(maximum) {}

uint32_t RandomGenerator::get(){
	return RandomGenerator::get(_min, _max);
}





WeightedRandomGenerator::WeightedRandomGenerator(unsigned int size, const int* values, const unsigned int* weights): _size(size), _values(values), _weights(weights) {
  
	uint32_t sum = 0;
	for(int i = 0; i< _size; i++){
		sum += weights[i];
	}

	_min = 0;
	_max = sum;
}


int WeightedRandomGenerator::get(){
	uint32_t r = RandomGenerator::get();
	uint32_t weightSum = 0;
	for(int i = 0; i < _size; i++){
		weightSum += _weights[i];
		if(r < weightSum){
			return _values[i];
		}
	}
}





TriangularRandomGenerator::TriangularRandomGenerator(int minimum, int peak, int maximum) : _min(minimum), _max(maximum), _peak(peak) {
  uint32_t peakProbability = ((uint32_t)RAND_MAX * 2) / (maximum - minimum);
  _risingSideProbStep = peakProbability / (peak - minimum);
  _fallingSideProbStep = peakProbability / (maximum - peak) + 1; // + 1 to avoid cumulative error because of truncation

  _correctedRandMax = 0;
  uint32_t stepProbability = 0;
  for (int i = _min; i < _max; i++){
    if (i == _min){
      stepProbability += _risingSideProbStep / 2;
    } else if (i < _peak){
      stepProbability += _risingSideProbStep;  
    } else if (i == _peak){
      stepProbability += (_risingSideProbStep / 2) - (_fallingSideProbStep / 2);
    } else {
      stepProbability -= _fallingSideProbStep;
    }
    _correctedRandMax += stepProbability;    
  }

}

int TriangularRandomGenerator::get(){

  uint32_t r = RandomGenerator::get(0, _correctedRandMax);
  
  uint32_t stepProbability = 0;
  uint32_t cumulativeProbability = 0;
  
  for (int i = _min; i < _max; i++){

    if (i == _min){
      stepProbability += _risingSideProbStep / 2;
    } else if (i < _peak){
      stepProbability += _risingSideProbStep;  
    } else if (i == _peak){
      stepProbability += (_risingSideProbStep / 2) - (_fallingSideProbStep / 2);
    } else {
      stepProbability -= _fallingSideProbStep;
    }
    
    cumulativeProbability += stepProbability;
    if (r < cumulativeProbability){
      return i;
    }
  }
  return _max;
}


void TriangularRandomGenerator::debug(){

  Serial.print("rising step: ");
  Serial.println(_risingSideProbStep);
  Serial.print("falling step: ");
  Serial.println(_fallingSideProbStep);
  Serial.print(" corrected Rand Max: ");
  Serial.println(_correctedRandMax);

  uint32_t stepProbability = 0;
  uint32_t cumulativeProbability = 0;
  
  for (int i = _min; i < _max; i++){
    if (i == _min){
      stepProbability += _risingSideProbStep / 2;
    } else if (i < _peak){
      stepProbability += _risingSideProbStep;  
    } else if (i == _peak){
      stepProbability += (_risingSideProbStep / 2) - (_fallingSideProbStep / 2);
    } else {
      stepProbability -= _fallingSideProbStep;
    }
    cumulativeProbability += stepProbability;
    Serial.print("step probability: ");
    Serial.print(stepProbability);
    Serial.print(" cumulative probability: ");
    Serial.println(cumulativeProbability);
  }
}
