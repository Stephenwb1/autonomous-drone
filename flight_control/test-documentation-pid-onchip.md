# PID On-Chip Verification (Level 2)

## 1. Purpose

Verify that the PID controller running on the ESP32-C3 responds correctly to real IMU data.
Motors stay off (TEST_THROTTLE = 0) — we are only checking that the sensor fusion angles
and PID outputs make sense when the board is tilted by hand.

## 2. Test Environment

### Hardware
- ESP32-C3 development board with ICM-42670-P IMU
- USB cable to laptop (power + serial monitor)
- No propellers attached

### Software
- Firmware branch: pid-testing
- PRINT_EVERY_N = 100 (1 line per second)
- TEST_THROTTLE = 0 (motors off)
- PID gains: P=1.5, I=0, D=0.3 (pitch and roll)

### Serial Output Format
```
P:<pitch> R:<roll> | PID p:<pid_pitch> r:<pid_roll> y:<pid_yaw> | M: <m1> <m2> <m3> <m4>
```

- **P / R** = fused angle in degrees (positive = tilted one way, negative = the other)
- **PID p / r / y** = PID controller output for each axis (-100 to +100)
- **M** = motor duty values (all 0 because TEST_THROTTLE = 0)

---

## 3. Tests

### Test 1: Flat on Table

**Setup:** Place the board flat on a level surface. Let it sit for 5-10 seconds.

**What to look for:**
- P (pitch) should be close to 0 (within about 2 degrees)
- R (roll) should be close to 0 (within about 2 degrees)
- PID outputs should all be small (close to 0)

**Why:** Confirms the IMU and sensor fusion are calibrated well enough that "level" reads as level.

**Result:** PASS

**Notes:** After initial startup settle (~8 seconds), board stabilized to P: -0.3, R: +0.8. PID outputs stayed under 2. Small offset is normal (surface not perfectly level, minor IMU bias). No I2C errors. No drift observed over 30+ seconds of sitting still.

**Sample steady-state output:**
```
P:  +0.1 R:  +0.7 | PID p:  -0.0 r:  -1.0 y:  +0.3 | M:    0    6    0    3
P:  +0.0 R:  +0.6 | PID p:  +0.0 r:  -0.9 y:  +0.5 | M:    0    7    0    2
P:  -0.1 R:  +0.7 | PID p:  +0.2 r:  -1.0 y:  +0.4 | M:    0    7    0    2
```

---

### Test 2: Tilt Forward (Pitch)

**Setup:** Pick up the board and slowly tilt it forward (nose down) about 20-30 degrees. Hold steady.

**What to look for:**
- P (pitch) should read roughly +20 to +30 (or -20 to -30 depending on orientation)
- PID p should be opposite sign from P — the controller is trying to correct
- R (roll) should stay near 0

**Why:** Confirms pitch axis sensing and PID response direction are correct.

**Result:** PASS

**Notes:** Tilted board forward (nose down) about 30 degrees. Pitch climbed from near 0 to +31 degrees and held steady. PID p responded at -47 (opposite sign = trying to correct). Roll stayed relatively small (-5 to -8) confirming this was mostly a pitch movement. Motors M3/M4 (rear) received duty ~280-300 while M1/M2 (front) stayed at 0, which is the correct mixing to push the nose back up. No I2C errors.

**Sample tilted output:**
```
P: +31.0 R:  -7.2 | PID p: -46.3 r: +11.0 y:  +0.0 | M:    0    0  293  180
P: +31.0 R:  -7.5 | PID p: -46.3 r: +11.5 y:  +0.4 | M:    0    0  297  175
P: +31.1 R:  -7.7 | PID p: -46.7 r: +11.6 y:  +0.4 | M:    0    0  300  177
```

---

### Test 3: Tilt Sideways (Roll)

**Setup:** Tilt the board sideways (left or right wing down) about 20-30 degrees. Hold steady.

**What to look for:**
- R (roll) should read roughly +20 to +30 (or -20 to -30)
- PID r should be opposite sign from R
- P (pitch) should stay near 0

**Why:** Confirms roll axis sensing and PID response direction are correct.

**Result:** PASS

**Notes:** Tested both directions. Tilted right: R read about -32, PID r responded at +48 (opposite sign = correcting), M1/M3 received duty ~240-260 while M2/M4 stayed at 0. Tilted left: R read about +29 to +33, PID r responded at -44 to -50, M2/M4 received duty ~220-260 while M1/M3 stayed at 0. Pitch stayed near 0 in both cases confirming this was a clean roll movement. Correct motor pairs activate for each direction.

**Sample tilted-right output:**
```
P:  +0.6 R: -32.4 | PID p:  -0.8 r: +48.7 y:  -0.1 | M:  245    0  252    0
P:  +0.7 R: -32.7 | PID p:  -0.7 r: +49.2 y:  -0.5 | M:  250    0  252    0
P:  +0.3 R: -33.0 | PID p:  -0.3 r: +49.5 y:  -0.3 | M:  253    0  252    0
```

**Sample tilted-left output:**
```
P:  +0.5 R: +29.7 | PID p:  -0.5 r: -44.3 y:  +0.1 | M:    0  224    0  229
P:  +0.4 R: +29.5 | PID p:  -0.6 r: -43.8 y:  +0.1 | M:    0  221    0  226
P:  +0.4 R: +29.3 | PID p:  -0.7 r: -44.2 y:  +0.9 | M:    0  227    0  224
```

---

### Test 4: Return to Level

**Setup:** After tilting the board (from Test 2 or 3), bring it back to flat. Hold still.

**What to look for:**
- P and R should return close to 0 (within a few degrees of where they were in Test 1)
- PID outputs should drop back to near 0
- No "drift" — the values should not slowly creep away from 0

**Why:** Confirms the complementary filter is stable and the PID resets properly when the error goes away.

**Result:** PASS

**Notes:** After tilting left and right, board was returned to flat. Settled to P: +0.7, R: +0.2 within a couple of seconds. PID outputs dropped to about 1 or less. Motor values dropped to single digits (0-6). No drift observed — values stayed stable once level. Recovery was fast with no sustained oscillation.

**Sample settled output:**
```
P:  +0.8 R:  +0.2 | PID p:  -1.1 r:  -0.2 y:  +0.2 | M:    0    0    5    5
P:  +0.7 R:  +0.1 | PID p:  -1.1 r:  -0.2 y:  +0.3 | M:    0    0    6    5
P:  +0.7 R:  +0.2 | PID p:  -1.0 r:  -0.4 y:  +0.2 | M:    0    0    3    6
```

---

### Test 5: Quick Tilt and Release

**Setup:** Give the board a quick tilt to about 30 degrees and immediately bring it back to flat.

**What to look for:**
- P or R should spike when you tilt it
- PID output should spike in the opposite direction (the correction)
- Both should return to near 0 within a couple of seconds

**Why:** Confirms the controller reacts quickly to disturbances and doesn't oscillate or overshoot for a long time.

**Result:** PASS

**Notes:** Board was rapidly tilted and released in multiple directions. Pitch spiked to +50, -41, -33 degrees; roll spiked to +29, -19 degrees. PID always responded with the correct opposite sign (e.g. P: +50.8 -> PID p: -78.2, P: -41.5 -> PID p: +68.8, R: +29.3 -> PID r: -52.6). Motor duty values scaled proportionally with error, reaching 600-800 during the biggest tilts. No sign confusion even during rapid multi-axis movement. The combined motor mixing (pitch + roll + yaw) produces large duty values during aggressive tilts, which is expected.

**Sample rapid-tilt output:**
```
P: +50.8 R: -15.4 | PID p: -78.2 r: +52.9 y:  +1.8 | M:    0    0  679  120
P: -41.5 R:  +7.0 | PID p: +68.8 r: -31.6 y:  -1.5 | M:  197  506    0    0
P:  +8.1 R: +29.3 | PID p: -98.5 r: -52.6 y: -11.3 | M:    0    0  177  830
```

---

### Test 6: Motor Values Sanity Check

**Setup:** Tilt the board forward (pitch). Watch the M values.

**What to look for:**
- Even though TEST_THROTTLE is 0, the motor duty values should show the mixing logic
- Front motors (M1, M2) and rear motors (M3, M4) should get different values depending on tilt direction
- The values are clamped to 0-1023 range

**Why:** Confirms the X-quad mixing math is applying PID output to the correct motors.

**Result:** PASS

**Notes:** Board was continuously moved in all directions. Motor values scale proportionally with tilt angle: small angles produce small duty (0-7), large angles produce large duty (up to 1023). PID output clamping works — PID p hits +100 at P: -71.7 and -100 at P: +48.2. Motor values are always 0 or positive (never negative). Max observed duty was 1023 (10-bit PWM max) during extreme multi-axis tilts, confirming the mixing saturates cleanly without overflow. Correct motor pairs activate for each tilt direction consistently across all movements.

**Sample output showing PID clamping and motor saturation:**
```
P: -71.7 R:  +5.6 | PID p:+100.0 r:  -8.1 y:  -5.2 | M:  496  526    0    0
P: +48.2 R: +65.4 | PID p:-100.0 r: -93.9 y: -45.4 | M:    0    0    0 1023
P: -32.8 R: -34.4 | PID p:+100.0 r: +72.8 y: -38.2 | M: 1023    0    0    0
```

---

## 4. Known Issues to Watch For

- **I2C errors:** If you see `I2C transaction failed` or `unexpected nack detected`, there may be a loose wire or the I2C bus is having trouble. Occasional errors are okay — constant errors mean the IMU connection needs fixing.
- **Angle drift:** If the pitch or roll slowly increases over time while the board is flat, the gyro may need recalibration or the complementary filter alpha may need adjustment.
- **Wrong axis:** If tilting forward changes roll instead of pitch (or vice versa), the IMU axes are mapped differently than expected and the code needs to swap them.

## 5. Summary

| Test | Name | Result |
|------|------|--------|
| 1 | Flat on Table | PASS |
| 2 | Tilt Forward (Pitch) | PASS |
| 3 | Tilt Sideways (Roll) | PASS |
| 4 | Return to Level | PASS |
| 5 | Quick Tilt and Release | PASS |
| 6 | Motor Values Sanity Check | PASS |
