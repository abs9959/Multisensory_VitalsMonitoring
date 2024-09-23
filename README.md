Loadcell_functioning_3measurements requirements:
- HX711 Library (https://steemit.com/utopian-io/@drencolha/making-a-weight-scale-with-the-hx711-module-hx711-arduino-library-and-a-load-cell-tutorial)


# Combined_Motor_LoadCell

## Serial Commands

### Motor Control
- **`h`**: High-speed counter-clockwise motor rotation.
- **`l`**: Low-speed clockwise motor rotation.
- **`t`**: Tighten the motor (counter-clockwise for a fixed duration).
- **`ls`**: Loosen the motor (clockwise for a fixed duration).
- **`n`**: Stop the motor (no rotation).

### Load Cell Commands
- **`m`**: Take 3 force measurements using the load cell and display the results in Newtons.
- **`r`**: Recalibrate the load cell with a known mass.
- **`f`**: Set a target force (in Newtons) and adjust the motor automatically to reach this force.

## How to Use

1. **Motor Operation**: 
   - Send motor control commands (`h`, `l`, `t`, `ls`, `n`) through the serial monitor to control the motor.  Note, in this version only does one cycle

2. **Take Force Measurements**: 
   - Use the `m` command to take 3 force readings from the load cell, which will be displayed in Newtons.

3. **Adjust Motor to Target Force**: 
   - Send the `f` command, then input your target force value when prompted. The motor will automatically adjust to reach the specified force.

4. **Recalibrate Load Cell**:
   - Use the `r` command to recalibrate the load cell. Ensure you have a known mass on the load cell for accurate calibration.

## To do
- Add a feature to log time vs force vs motor command for monitoring purposes.
-  **GPIO Pin 18** being used for both the encoder and the load cell ?
- Figure out why load cell part does not work
