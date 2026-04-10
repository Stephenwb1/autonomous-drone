# Autonomous Drone – PID Controller Simulation Test Plan

## 1. Purpose

This document defines the simulation-based test plan for the PID controller implementation (`pid.c`).
The goal of this phase is to validate:

- Correct proportional, integral, and derivative behavior in isolation and combination
- Integral anti-windup protection
- Output clamping
- Disturbance rejection capability

Each test states a hypothesis derived from control theory **before** execution.
Results either confirm the implementation is correct or reveal a bug.

---

## 2. Test Environment

### Hardware
- No hardware required — all tests run as a desktop simulation on the development machine.

### Software
- Simulation source: `flight_control/sim/quad_sim.c`
- PID under test: `flight_control/main/pid.c` (compiled directly into the simulation)
- Plotting: `flight_control/sim/plot_sim.py` (Python 3 + matplotlib)
- Compiler: `gcc` (Apple Clang or equivalent)

### Build & Run Commands
```bash
cd flight_control/sim
gcc -O2 -o quad_sim quad_sim.c ../main/pid.c -I../main -lm
./quad_sim > testN.csv
python3 plot_sim.py testN.csv
open sim_plot.png
```

### Simulation Plant Model
- Single rotational axis (pitch or roll)
- Moment of inertia: `3.0e-5 kg·m²` (estimated for ~60 g micro quad)
- Torque per PID unit: `2.2e-4 N·m` (estimated for 8520 brushed motors, 55 mm props, ~40 mm arm)
- Drag coefficient: `1.0e-5` (friction proportional to angular velocity)
- Loop rate: 100 Hz (`DT = 0.01 s`), matching the firmware
- Simulation duration: 10 seconds

---

## 3. Test Cases

---

### PID-SIM-01 — P-Only Control (Oscillation Expected)

**Hypothesis:**
With only the proportional term active, the controller acts as a spring with no damper. Control theory predicts the system will oscillate around the setpoint because there is no mechanism to dissipate energy.

**Configuration:**
```c
#define KP   1.5f
#define KI   0.0f
#define KD   0.0f
#define INITIAL_ANGLE_DEG  15.0f
```
Disable the disturbance by setting `DISTURBANCE_TORQUE` to `0.0f`.

**Steps:**
1. Apply the configuration values above in `quad_sim.c`.
2. Compile and run: `./quad_sim > test1_p_only.csv`
3. Plot: `python3 plot_sim.py test1_p_only.csv`
4. Rename output: `mv sim_plot.png test1_p_only.png`

**Pass Criteria:**
- The angle oscillates around 0 degrees (crosses zero multiple times).
- Oscillation decays slowly (due to plant drag) or persists — it does **not** settle quickly and smoothly.
- PID output is a mirror image of the angle (positive angle → negative output).
- No steady, smooth convergence to 0 within the first 2 seconds.

**Evidence to Capture:**
- `test1_p_only.png` — three-panel plot showing angle, angular velocity, and PID output.
- Note the number of zero-crossings in the first 5 seconds.
- Note whether a steady-state offset remains.

**Results:**
_(fill in after running)_

---

### PID-SIM-02 — P+D Control (Damped Response)

**Hypothesis:**
Adding the derivative term should eliminate the oscillation observed in PID-SIM-01. The D term resists changes in error, acting as a damper on the spring. The angle should converge smoothly to 0 with no oscillation or at most one small overshoot.

**Configuration:**
```c
#define KP   1.5f
#define KI   0.0f
#define KD   0.3f
#define INITIAL_ANGLE_DEG  15.0f
```
Disable the disturbance by setting `DISTURBANCE_TORQUE` to `0.0f`.

**Steps:**
1. Apply the configuration values above in `quad_sim.c`.
2. Compile and run: `./quad_sim > test2_pd.csv`
3. Plot: `python3 plot_sim.py test2_pd.csv`
4. Rename output: `mv sim_plot.png test2_pd.png`

**Pass Criteria:**
- The angle converges to 0 smoothly — no sustained oscillation.
- At most one minor overshoot past 0 degrees.
- Settling time is within a few seconds.
- Direct comparison with PID-SIM-01: the oscillation is gone.

**Evidence to Capture:**
- `test2_pd.png` — three-panel plot.
- Note the settling time (time to reach and stay within ±0.5 degrees of 0).
- Compare side-by-side with `test1_p_only.png`.

**Results:**
_(fill in after running)_

---

### PID-SIM-03 — High P, No D (Instability)

**Hypothesis:**
Increasing the proportional gain without derivative damping should make the oscillation from PID-SIM-01 significantly worse. The system may become marginally stable or unstable, with the PID output hitting the ±100 clamp repeatedly.

**Configuration:**
```c
#define KP   8.0f
#define KI   0.0f
#define KD   0.0f
#define INITIAL_ANGLE_DEG  15.0f
```
Disable the disturbance by setting `DISTURBANCE_TORQUE` to `0.0f`.

**Steps:**
1. Apply the configuration values above in `quad_sim.c`.
2. Compile and run: `./quad_sim > test3_high_p.csv`
3. Plot: `python3 plot_sim.py test3_high_p.csv`
4. Rename output: `mv sim_plot.png test3_high_p.png`

**Pass Criteria:**
- The angle oscillates with larger amplitude and/or higher frequency than PID-SIM-01.
- The PID output saturates at ±100 repeatedly (output clamp is hit).
- The system does **not** settle quickly or smoothly.
- This demonstrates that increasing P alone is not a valid tuning strategy.

**Evidence to Capture:**
- `test3_high_p.png` — three-panel plot.
- Note peak-to-peak oscillation amplitude.
- Note how many times the PID output hits ±100 in the first 2 seconds.

**Results:**
_(fill in after running)_

---

### PID-SIM-04 — High P + Adequate D (Fast and Stable)

**Hypothesis:**
Adding sufficient derivative gain to the high P from PID-SIM-03 should produce a fast, well-damped response. The high P provides aggressive correction, and the high D prevents overshoot. Settling time should be significantly shorter than PID-SIM-02.

**Configuration:**
```c
#define KP   8.0f
#define KI   0.0f
#define KD   0.8f
#define INITIAL_ANGLE_DEG  15.0f
```
Disable the disturbance by setting `DISTURBANCE_TORQUE` to `0.0f`.

**Steps:**
1. Apply the configuration values above in `quad_sim.c`.
2. Compile and run: `./quad_sim > test4_high_pd.csv`
3. Plot: `python3 plot_sim.py test4_high_pd.csv`
4. Rename output: `mv sim_plot.png test4_high_pd.png`

**Pass Criteria:**
- The angle converges to 0 faster than PID-SIM-02 (settling time under 1 second).
- At most a small overshoot (less than 2 degrees past 0).
- No sustained oscillation.
- PID output is large initially but settles quickly.
- Direct comparison with PID-SIM-03: the violent oscillation is gone.

**Evidence to Capture:**
- `test4_high_pd.png` — three-panel plot.
- Note settling time and compare with PID-SIM-02 and PID-SIM-03.
- Note peak overshoot if any.

**Results:**
_(fill in after running)_

---

### PID-SIM-05 — I-Term Eliminates Steady-State Error

**Hypothesis:**
When a constant external bias (simulating an off-center center of gravity) is applied, P+D control alone will settle to a non-zero angle because the P term output approaches zero as the error shrinks. Adding an I-term should accumulate the small persistent error over time and drive the angle to exactly 0.

**Configuration (run A — without I-term):**
```c
#define KP   1.5f
#define KI   0.0f
#define KD   0.3f
#define INITIAL_ANGLE_DEG  15.0f
```

**Configuration (run B — with I-term):**
```c
#define KP   1.5f
#define KI   0.5f
#define KD   0.3f
#define INITIAL_ANGLE_DEG  15.0f
```

**For both runs,** add a constant torque bias in `quad_sim.c` to simulate CG offset. Locate the line:
```c
float torque = output * TORQUE_PER_UNIT;
```
And add immediately after it:
```c
torque += 1.0e-5f;
```

**Steps:**
1. Add the bias line. Set gains for run A (KI=0.0). Compile and run: `./quad_sim > test5a_no_i.csv`
2. Plot: `python3 plot_sim.py test5a_no_i.csv` → rename to `test5a_no_i.png`
3. Change to run B gains (KI=0.5). Compile and run: `./quad_sim > test5b_with_i.csv`
4. Plot: `python3 plot_sim.py test5b_with_i.csv` → rename to `test5b_with_i.png`
5. **Remove the bias line** after the test so it doesn't affect other tests.

**Pass Criteria:**
- **Run A (no I):** Angle settles to a small but non-zero steady-state offset (approximately 0.5–2 degrees). It gets close to level but never reaches exactly 0.
- **Run B (with I):** Angle converges all the way to 0.0 degrees. May overshoot slightly before settling. The integral term accumulates until it fully counteracts the bias.
- Comparing A and B directly proves the I-term's purpose.

**Evidence to Capture:**
- `test5a_no_i.png` and `test5b_with_i.png` — side-by-side comparison.
- Note the steady-state angle in run A.
- Note the settling time and overshoot in run B.

**Results:**
_(fill in after running)_

---

### PID-SIM-06 — Integral Windup Protection

**Hypothesis:**
With a high I-gain and a large initial error, the integral term will accumulate rapidly while the PID output is saturated. The `integral_limit` clamp in `pid.c` should prevent the integral from growing unboundedly. Without this protection, the overshoot past 0 degrees would be catastrophic; with it, overshoot should be moderate and bounded.

**Configuration:**
```c
#define KP   1.5f
#define KI   2.0f
#define KD   0.3f
#define INITIAL_ANGLE_DEG  45.0f
#define PID_INTEGRAL_LIMIT  50.0f
```
Disable the disturbance by setting `DISTURBANCE_TORQUE` to `0.0f`.

**Steps:**
1. Apply the configuration values above in `quad_sim.c`.
2. Compile and run: `./quad_sim > test6_windup.csv`
3. Plot: `python3 plot_sim.py test6_windup.csv`
4. Rename output: `mv sim_plot.png test6_windup.png`

**Pass Criteria:**
- The PID output saturates at ±100 during the initial recovery (output clamp working).
- The angle converges to 0, with some overshoot.
- The overshoot past 0 degrees is **bounded** (less than ~15 degrees) — not comparable to the initial 45-degree displacement.
- This confirms the `integral_limit` clamp in `pid.c` is functioning.

**What failure would look like (for reference):**
If the integral clamp were removed, the overshoot would be severe (potentially exceeding the initial 45-degree tilt) because the integral would accumulate to a very large value during the saturated recovery period.

**Evidence to Capture:**
- `test6_windup.png` — three-panel plot.
- Note the peak overshoot past 0 degrees.
- Note how long the PID output remains saturated at ±100.

**Results:**
_(fill in after running)_

---

### PID-SIM-07 — Disturbance Rejection

**Hypothesis:**
A well-tuned PID controller should recover from an unexpected external force (simulating a wind gust). The drone should deflect briefly, then return to level. With an I-term present, the steady-state angle after recovery should be exactly 0.

**Configuration:**
```c
#define KP   5.0f
#define KI   0.2f
#define KD   0.6f
#define INITIAL_ANGLE_DEG  0.0f
#define DISTURBANCE_TIME_S    3.0f
#define DISTURBANCE_TORQUE    5.0e-4f
#define DISTURBANCE_DURATION  0.05f
```

**Steps:**
1. Apply the configuration values above in `quad_sim.c`.
2. Compile and run: `./quad_sim > test7_disturbance.csv`
3. Plot: `python3 plot_sim.py test7_disturbance.csv`
4. Rename output: `mv sim_plot.png test7_disturbance.png`

**Pass Criteria:**
- The angle remains at 0 for the first 3 seconds (no input, no movement).
- At t=3.0s the angle deflects due to the disturbance impulse.
- The PID corrects the deflection and the angle returns to 0.
- Recovery time (back to within ±0.5 degrees) is under 1 second.
- Steady-state after recovery is 0.0 degrees (I-term corrects any residual).

**Evidence to Capture:**
- `test7_disturbance.png` — three-panel plot.
- Note peak deflection angle.
- Note recovery time.
- Note steady-state angle after recovery.

**Results:**
_(fill in after running)_

---

## 4. Known Limitations (Current Phase)

- The plant model is a simplified single-axis simulation. It does not capture cross-axis coupling (pitch affecting roll), motor response delay, propeller aerodynamics, or sensor noise.
- Physical parameters (moment of inertia, torque per unit) are estimates based on component specs. Actual values will differ and gains will need re-tuning on real hardware.
- The simulation uses the PID's current derivative-on-error implementation. A known improvement (derivative-on-measurement) has been identified and will be implemented before hardware testing.
- No sensor noise is injected. Real IMU data will be noisier than the simulation, which will affect the D term.

---

## 5. Next Phase Testing (Planned)

- **PID code improvements:** Fix derivative kick (compute derivative on measurement instead of error) and add a low-pass filter on the D term. Re-run all 7 tests to confirm behavior does not regress.
- **Level 2 — On-chip IMU verification:** Flash firmware with `TEST_THROTTLE=0`, tilt the board by hand, verify PID outputs match expected direction and magnitude on the serial monitor.
- **Level 3 — Tethered motor test:** Mount drone on a single-axis pivot, raise throttle to a low value, arm motors, verify the PID stabilizes the axis.
- **GPS integration:** Write UART driver and NMEA parser for the NewBeeDrone M10Q after PID is validated.

---

## 6. Conclusion

This test plan validates the PID controller implementation (`pid.c`) using a desktop simulation before any hardware testing.
Each test is derived from a control theory hypothesis and proves a specific property of the controller: proportional response, derivative damping, integral steady-state correction, windup protection, and disturbance rejection.
Passing all 7 tests confirms the PID foundation is correct and ready for on-hardware validation (Level 2 and Level 3 testing).
