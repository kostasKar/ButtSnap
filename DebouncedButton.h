#ifndef DEBOUNCEDBUTTON_H
#define DEBOUNCEDBUTTON_H

#include <inttypes.h>


class DebouncedButton{

    public:
    /*
     *  Arguments: pin number
     *  number of consecutive high samples to determine a low to high transition
     *  number of consecutive low samples to determine a high to low transition
     *  if not used or 0, the same number as lowToHighSamples will be used
     */
    DebouncedButton(uint8_t pin, uint8_t lowToHighSamples, uint8_t highToLowSamples = 0);

    /*
     *  samples the digital input. Needs to be run periodically inside a timer ISR
     */
    void sample();

    /*
     *  returns the debounced state of the input
     */
    uint8_t getState();
    
    /*
     *  returns true if a transition from low to high has happened. 
     */
    bool changedToHigh();

    /*
     *  returns true if a transition from high to low has happened. 
     */
    bool changedToLow();

        /*
     *  returns true if a transition from low to high has happened. 
     *  Also clears the internal transition flag, so another call or a call to changedToHigh will read as false
     */
    bool changedToHighAndClear();

    /*
     *  returns true if a transition from high to low has happened. 
     *  Also clears the internal transition flag, so another call or a call to changedToLigh will read as false
     */
    bool changedToLowAndClear();

    /*
     *  Clears any transitions that have happened so as to monitor new ones
     */
    void clearChanges();

    private:
    uint8_t _pin;
    uint8_t _highToLowSamples;
    uint8_t _lowToHighSamples;
    uint8_t _debouncedState;
    uint8_t _counter;
    bool _changedToHigh;
    bool _changedToLow;


};








#endif
