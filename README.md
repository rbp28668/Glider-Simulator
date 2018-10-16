# Glider-Simulator
Software for virtual instruments, add-ons to P3D and joystick interfacing for the CGC simulator.  There are a number of sub-projects here.

## CondorInstrumentsJFX
This uses Java JFX to draw virtual glider instruments on a screen.  The instruments are photographs of real K21 instruments (KFY and HTV at Cambridge Gliding Centre) with the needles cut out with the GIMP and re-inserted afterwards according to what the instrument should be reading.  This uses an extended Condor data format so can work with the Condor simulator or a P3D plugin that generates the same format.   This uses JFX as it's possible to get this running on a Raspberry Pi with hardware acceleration.  

## CondorInstrumentsController
This allows control messages to be sent to CondorInstrumentsJFX to change panel.  Typically used to switch between VFR and cloud flying panels.
