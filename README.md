# DIY Cue Light System

This Repo contains all of the neccesary hardware and software to implement a 10 station cue light system for use in a theater or other semi-permanent installation. As of March 2020, you can put this system together for well under $500.00 which is exceptional value when compared with competing commercial products.

For videos and photos of the finished system (and proof that it works...) visit the [project page](https://chrismarkus.me/cue-lights) on my website.

DISCLAIMER: *This project is intended for ADVANCED users only! That is, you need advanced soldering skills and an in-depth understanding of circuits and electrical systems is recommended to put it together. I am not responsible for any damages should you choose to put this system together yourself - use at your own risk...*

## Contents

**/controller:** Contains Arduino code that runs on the Arduino Mega that acts as the controller for the system.
**/reciever:** Contains Arduino code that runs on the arduino nano inside of each cue light box.
**/hardware_controller:** Contains schematics and guides for building the controller box.
**/hardware_receiver:** Contains schematics, guides, and PCB (Printed Circuit Board) manufacturing files to order the circuit boards within each cue light box.

## Getting Started

If you would like to replicate this project, I recommend you do a few things first:

1. Read the entirety of these instructions (there are a few subtleties that could make or break the project for you)!
2. Make sure you can source all of the parts needed to build the final product or suitable replacements (See a parts list [here](https://www.google.com)).
3. Make sure you have the neccesary tools:
    - Soldering Iron, Solder, Flux, tweezers (there is 1 surface mount component)
    - Access to a laser cutter **or** a drill, files, and lots of patience.
    - A rotary tool or other plastic-cutting solution if you are using the same boxes for the clue light stations as I was (you will have to make some square holes in the boxes for the ethernet jacks - more on that later).
