# Intellivision_USB_adapter
This code allows a Arduino Micro (I believe a Leonardo could be used with minor pin assignments) to detect as a USB keyboard on a PC with bindings set to the Intellivision controller. Of note is that the directional pad properly resolves as a 12-way pad like it does on the actual console.

This is based on spinal's Intellivision Controller Adapter from socoder.net (https://socoder.net/?Blogs=55773)
I removed conflicting reference to another type of micro controller board.
I also added three more additional buttons. They are intended to be used to pause/reset a game in the jzintv emulator and also exit back to an emulator dashboard like RetroPie. These can easily be mapped for other uses.

This program uses an actual ground pin and does not pull a gpio pin low like spinal describes in his original project.
