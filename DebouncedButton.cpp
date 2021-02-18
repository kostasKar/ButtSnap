#include "DebouncedButton.h"
#include "Arduino.h"


DebouncedButton::DebouncedButton(uint8_t pin, uint8_t lowToHighSamples, uint8_t highToLowSamples):
    _pin(pin),
    _lowToHighSamples(lowToHighSamples),
    _counter(0),
    _changedToHigh(false),
    _changedToLow(false)
{
    _highToLowSamples = (highToLowSamples)? highToLowSamples : lowToHighSamples;
    pinMode(_pin, INPUT_PULLUP);
    _debouncedState = digitalRead(_pin);
}

void DebouncedButton::sample(){
    if (_debouncedState == HIGH){
        if (digitalRead(_pin) == HIGH){
            _counter = 0;
        } else {
            _counter++;
            if (_counter == _highToLowSamples){
                _changedToLow = true;
                _debouncedState = LOW;
                _counter = 0;
            }
        }
    } else {
        if (digitalRead(_pin) == LOW){
            _counter = 0;
        } else {
            _counter++;
            if (_counter == _lowToHighSamples){
                _changedToHigh = true;
                _debouncedState = HIGH;
                _counter = 0;
            }
        }
    }
}

uint8_t DebouncedButton::getState(){
    return _debouncedState;
}

bool DebouncedButton::changedToHigh(){
    return _changedToHigh;
}

bool DebouncedButton::changedToLow(){
    return _changedToLow;
}

bool DebouncedButton::changedToHighAndClear(){
    if (_changedToHigh){
        _changedToHigh = false;
        return true;
    }
    return false;
}

bool DebouncedButton::changedToLowAndClear(){
    if (_changedToLow){
        _changedToLow = false;
        return true;
    }
    return false;
}

void DebouncedButton::clearChanges(){
    _changedToLow = false;
    _changedToHigh = false;
}
