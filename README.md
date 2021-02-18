# ButtSnap


ButtSnap is the simplest competitive speed game. It challenges the players speed and reflex response time

Made as an Arduino sketch on a small custom PCB featuring the ATMega328P processor

[Website](https://buttsnap.com/)

[Tindie listing](https://www.tindie.com/products/studiobelow/buttsnap/)

![DSC_6568](https://user-images.githubusercontent.com/13304797/108424890-6900cc00-7242-11eb-881f-2b82a7b07c9d.jpg)

## How to play

- 2 players. Each one has a button and a win indicating LED. 1 central trigger LED.
When the trigger LED pulses on, the fastest player to press his button wins the point!

- Every once in a while, randomly, a bonus round is launched, where the trigger LED flashes rapidly for a fixed amount of time. The players have to push their buttons as many times as possible within that period. The one that manages the most clicks wins the point.

- Each point won is indicated by some short flashes of the winner's LED. The number of flashes are correlated with the reaction time of the player. Faster responses are "applauded" accordingly

## Points / Sets

- A number of points make up a set. This number defaults to 10 but can be easily configured. To configure it, switch the device on while holding Player 1 button pressed and then press the button as many times as the number of set points you wish to have. After 1 second, the device boots normally with the applied configuration. The configuration is permanently stored in the device

- When a player wins a set, his LED is held on steadily for 2 seconds

## Checking the score

- At any time during the game, the players can check the current score by both pressing and holding their buttons. Then each players score will be indicated by flashes of his LED. First the sets are indicated and then the points of the currently played set, with a different pulse frequency

- The device also permanently stores one high score for each player position. It is the maximum number of button presses during the bonus rounds. To check it, you have to switch on the device while holding both buttons pressed. The high scores will be indicated by flashes of the players LEDs

## Features

- The delay between the trigger rounds is truly random, fed by atmospheric noise. The duration of the round pulses is also random.

- The board is small enough to carry in your pocket (25mm by 60mm), yet very comfortable for holding. It also has a hole for keychain.

- It is powered by a CR2032 battery

>Tip: The flashing frequency of trigger LED during the bonus round is the speed of the current World Record for button pressing! So if you manage to cope with that, you should start making some phone calls :) Also don't worry, ButtSnap will store your high score to show your friends

>Another Tip: Sure you can just keep pressing the button like crazy while waiting for the trigger LED to come up and hope for a good response time. Just, you know, no one will like you. If there are complaints I will add a penalty in future versions