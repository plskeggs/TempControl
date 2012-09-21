TempControl code 9/20/2012
- changes by Pete Skeggs to allow switching to control using thermocouple (e.g., with a toaster oven) in addition to a hot plate with IR thermometer.
- changed heater updates to occur between calling every main loop handler; this results in snappier response
- changed timebase and power range to 500ms from 1000ms for faster response; results in tighter temperature control

9/21/2012
- added use of modified version of TimerOne library to perform pwm in the background (original is from here:http://arduino.cc/playground/Code/Timer1, but I added ability to do pwm to any pin)

Scott's original readme:

The current versions of the Eagle files, .hex files and source code can be found at http://dorkbotpdx.org/blog/scott_d/temperature_controller_board_final_design
To install the temperature controller software on a new ATMega328 you can either load the .hex files or build from the Arduino source code.
If you choose to load the .hex files, you should load both the temp_control.hex which is the program code which should be loaded into flash memory and also eeprom.hex, which contains some parameters for the controler which reside in eeprom memory.

If you want to build the Arduino source code, there are a few steps.
1) (Optional) Program the Arduino boot loader into your ATMega328.  
2) Install the required libraries:  RotaryEncoder and max6675.  The procedure for installing libraries varies with your platform and version of the Arduino software.  It is easiest if you use Arduino version 17 or higher.  In that case, you can unzip the libraries into a libraries subdiretory in the directory where your sketches reside.  In earlier versions of of the Arduino software, you need to unzip the libraries in the main Arduino library directory.  Check the documentation at http://arduino.cc for further details.
3)  Create a directory called tempcontrol in your Arduino sketch directory.  Then unpack main.zip in that directory.  When you start up the Arduino software you will see a new sketch called tempcontrol with a subfile called main.  If you open tempcontrol/main you will see a number of  tabs which contain the various source code files.  You should be able to select "Arduino Duemilanove or Nano w/ ATmega328" as the target board and compile and load the code into the temperature controller.
4) You still need to set the eeprom parameters the first time you run the temperature controller.  To do this, you could use and AVR programmer to write the eeprom.hex file into the eeprom memory of the temperature controller.  Or you can use a serial connection to the temperature controller to set the parameters from a Processing sketch as detailed in this <a href="http://www.instructables.com/id/Closing-the-Loop-on-Surface-Mount-Soldering/step3/Hooking-it-all-up/">Instructable</a>.  The Processing sketch is also useful for graphing the temperature of the hotplate over time.

