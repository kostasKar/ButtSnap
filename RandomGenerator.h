#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <inttypes.h>


//RAND_MAX = 32767



class RandomGenerator{
protected:
	uint32_t _min, _max;
public:
	static void setSeed(uint32_t seed);
	static void setAdcNoiseSeed(int analogChannel);
  static uint32_t get(uint32_t minimum, uint32_t maximum);

	RandomGenerator();
	RandomGenerator(uint32_t maximum);
	RandomGenerator(uint32_t minimum, uint32_t maximum);
	uint32_t get();
};




class WeightedRandomGenerator : protected RandomGenerator{

	const int* _values;
	const unsigned int* _weights;
	unsigned int _size;

public:
	WeightedRandomGenerator(unsigned int size, const int* values, const unsigned int* weights);
	int get();
};



class TriangularRandomGenerator {

  int _min, _peak, _max;
  uint32_t _correctedRandMax;
  uint32_t _risingSideProbStep, _fallingSideProbStep;

  public:
  TriangularRandomGenerator(int minimum, int peak, int maximum);
  int get();
  void debug();
   
};




#endif
