# RZR_CAN_HACKS
Hacks to make an electric Polaris RZR work well!

This project uses an Arduino Uno with a SeeedStudio CAN-bus shield to send and receive CAN messages that do things such as keep power steering running, turn off the check-engine light (even though there is no longer and engine), and restore the speedometer and tachometer on the dash.

### CAN Messages

#### Read:

* RPM from Curtis Controller. Address: `0x601`, byte 0 = RPM high byte, byte 1 = RPM low byte.

* Gear shifter position from RZR CAN-bus. Address: `0x18F00500`, byte 5 = shifter postion. `0x4C` = low gear drive, `0x48` = high gear drive, `0x4E` = neutral, `0x52` = reverse, `0x50` = park, `0x2D` = error.

#### Sent:

* Speed to RZR CAN-bus. Address: `0x18FEF100`, message = `{ 0xFF, SPEED LSB, SPEED MSB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }`, where `SPEED` is the speed in mph times 500.

* RPM to RZR CAN-bus. Address: `0x18FF6600`, message = `{ 0x00, RPM LSB, RPM MSB, 0x00, 0x00, 0x00, 0x00, 0x00 }`, where `RPM` is the raw RPM of the motor.

* Engine OK signal to RZR CAN-bus. Address: `0x18FECA00`, message = `{ 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF }`. This turns the check-engine light off.

Note that as long as an RPM of 500 or more is sent to the RZR CAN-bus every 5 minutes, the power steering will stay on.
