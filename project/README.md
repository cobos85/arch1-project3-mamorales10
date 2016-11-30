# arch1-project3-mamorales10

About Project:
My task is to develop a game for the LCD screen of the MSP430. I should utilize the knowledge
that I have gained in working with the buttons and the buzzer. In addition, a series of libraries
have been found to assist me in creating my game.

This directory contains:
* a file (shapemotion.c) which implements the game for the toy.
* a file (wdt_handler.s) which handles the interrupts of the watchdog timer



To install:

First compile from the main project directory:
~~~
$ make
~~~
Load from the directory named "project":
~~~
$ make load
~~~
To delete binaries: (from project directory)
~~~
$ make clean
