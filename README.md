# arduino-midi-to-cv
This arduino project accsepts MIDI input and translates it into various control voltages for a synthesiser.

## Features

 * 0 to 5 volt v/octave output, with 0V corresponding to MIDI note 36 (C2).
 * Monophonic, highest note priority
 * Gate Output
 
### Work in progress (planned features):

 * Pitch bend support (on the v/octave output)
 * Mod wheel output
 * Velocity output
 * OLED screen with options menu
 * Note priority option
 * Gate retrigger/legato option


## Installation requirements

 * if one desires this to work over usb, then flashing the [mocoLUFA](https://github.com/kuwatay/mocolufa) firware to the arduino is required. I personaly found this [tutorial](https://youtu.be/-bCz2I9SMAA) helpfull. Otherwise use a 5 pin MIDI connector circuit(not shown).
 * Upload with arduino IDE or comand line utility
