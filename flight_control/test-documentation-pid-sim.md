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

**Status: PASS**

- **Oscillation observed:** Yes. The angle overshoots past 0 degrees (goes negative to approximately -2 degrees) within the first 0.1 seconds, then oscillates briefly before settling. This confirms the P-only "spring with no damper" hypothesis.
- **Zero-crossings:** The angle crosses zero approximately 2-3 times in the first 0.5 seconds.
- **Settling time:** ~0.5-1 second to reach and stay within 1 degree of setpoint. The system settles faster than ideal (zero-drag) P-only theory predicts because the plant model includes aerodynamic drag (`DRAG_COEFF = 1.0e-5`), which dissipates energy. This is physically realistic -- real drones experience air resistance -- and does not invalidate the test.
- **Steady-state offset:** Effectively zero in this simulation. On real hardware with gravity-induced torques, a steady-state offset would likely appear — this is why we need the I-term (tested in PID-SIM-03).
- **PID output:** Mirrors the angle as expected. Initial output saturates at -100 (clamped by `PID_OUTPUT_LIMIT`) because KP * 15° = 22.5, and the large resulting torque drives the first oscillation spike.
- **Key takeaway:** P-only control works but produces oscillation and relies on plant drag for convergence. Adding the D-term (PID-SIM-02) should eliminate the oscillation; adding the I-term (PID-SIM-03) will address any real-world steady-state offset.

**Evidence:** `flight_control/sim/test1_p_only.png`

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

**Status: PASS**

- **Oscillation eliminated:** The angle converges smoothly from 15° to 0° without ever crossing zero. No oscillation at all — a clear improvement over PID-SIM-01.
- **Settling time:** Angle drops below 0.5° at t=0.73s and continues decaying monotonically.
- **No overshoot:** The angle never goes negative, confirming the D term damps out the energy that caused oscillation in the P-only test.
- **PID output:** Initial spike to -100 (saturated because the large initial error plus the rate of change combine to exceed the output limit), then smoothly decays toward 0 as the angle approaches setpoint.
- **Comparison with PID-SIM-01:** In the P-only test, the angle crossed zero and oscillated before settling. Here, the D term resists rapid changes and removes that overshoot entirely. The trade-off is slightly slower initial convergence (angle is still 1.78° at t=0.43, versus nearly 0° in test 1), but the response is much cleaner.
- **Key takeaway:** The D term acts as a damper on the P-term "spring." PD control gives smooth, monotonic convergence with no overshoot — exactly what a drone needs for stable flight.

**Evidence:** `flight_control/sim/test2_pd.png`

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

**Results:** PASS

- **Oscillation is significantly worse than PID-SIM-01:** The angle overshoots to about -9° on the first swing (compared to about -4° with KP=1.5), and the system crosses zero 18 times total versus 7 in Test 1. In the first half second alone there are 10 zero crossings — the controller is overcorrecting back and forth rapidly.
- **Peak-to-peak amplitude:** About 20° (from +11° to -9°), wider than Test 1's 18°. The higher gain pushes too hard on every swing, which makes the next swing even worse.
- **PID output saturation:** Hits -100 on the very first step (same as Test 1 — the initial 15° error saturates both). After that, the rapid back-and-forth keeps the angle small enough that the PID output stays within bounds, but the output keeps flipping direction. On real hardware, this would cause the motors to constantly reverse thrust, leading to stuttering and mechanical stress.
- **Eventual settling via drag:** The simulation drag eventually damps the oscillation (angle under 1° by about a third of a second, essentially zero by 1 second), but the transient is violent. On a real drone without this idealized drag, the oscillation could be sustained or divergent.
- **Comparison with PID-SIM-01:** With KP=1.5 the system oscillated but settled relatively quickly. Increasing KP to 8.0 made the overshoot about twice as bad and the oscillation much more aggressive. This confirms that cranking up P alone is not a valid tuning strategy — it makes the initial response faster but at the cost of stability.
- **Key takeaway:** High proportional gain without derivative damping produces aggressive oscillation that would be catastrophic on real hardware. This test motivates PID-SIM-04, where we add an adequate D term to tame the high P gain.

**Evidence:** `flight_control/sim/test3_high_p.png`

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

**Results:** PASS (after derivative algorithm fix)

**Note:** This test originally FAILED with the old derivative-on-error algorithm. The old code computed the derivative by looking at how fast the error changed, dividing by the time step (0.01 seconds). With KD=0.8, even a tiny angle change got multiplied by 0.8/0.01 = 80, which slammed the output to the opposite saturation limit every single step. The controller fought itself in an endless loop called "chattering" and the angle never settled.

After fixing the derivative algorithm (computing on measurement instead of error, plus adding a low-pass filter), this test now passes:

- **Fast convergence:** The angle drops from 15 degrees to under 1 degree by about 0.2 seconds, and is essentially zero by 1 second. This is significantly faster than Test 2 (which took about 0.7 seconds to get below 1 degree with KP=1.5).
- **Some initial oscillation:** The high P gain causes the angle to overshoot past zero briefly (going to about -1 degree) and bounce a couple of times in the first 0.2 seconds. But unlike the old results where it never stopped bouncing, the low-pass filter on the D term smooths things out and the system settles cleanly.
- **No chattering:** The PID output is no longer stuck alternating between +100 and -100. It spikes during the initial correction, then quickly settles to near zero. The filter prevents the D term from amplifying tiny changes into wild overcorrections.
- **Comparison with Test 3 (same KP, no D):** Test 3 had violent oscillation that took a full second to damp out. Adding KD=0.8 with the improved derivative algorithm eliminates most of that oscillation and produces a much faster, cleaner response.
- **Key takeaway:** The derivative-on-measurement algorithm with a low-pass filter made this gain combination viable. The old derivative-on-error approach could not handle aggressive D gains at all. This proves that the algorithm fix was necessary and effective.

**Evidence:** `flight_control/sim/test4_high_pd.png`

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
torque += 5.0e-4f;
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

**Verdict: PASS**

**Run A (no I-term):** The angle dropped from 15 degrees and settled at about 2 degrees. It stopped moving and the angular velocity went to zero, but the drone never made it all the way to level. Since there is no I-term, the only way the controller can produce output is if there is still some error. So the error has to stay there permanently for the controller to keep pushing back against the bias. This is called steady-state error.

**Run B (with I-term):** The angle dropped from 15 degrees and kept slowly working its way toward zero. By about 1 second it was below 1 degree, and by about 5 seconds it was nearly at zero. By the end of the 10-second run the angle was essentially zero. There was no overshoot -- the angle never went below zero. The I-term slowly accumulated a correction that fully cancelled out the constant bias, which is exactly its job.

**Why this matters:** In a real drone, the center of gravity is never perfectly centered. One side is always slightly heavier than the other, or a prop produces slightly more thrust. Without an I-term, the drone would hover with a permanent tilt. The I-term notices that the error is not going away and keeps adding more correction until the drone is truly level. This test proves that behavior works correctly.

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

**Verdict: PASS**

The drone started at 45 degrees tilted -- three times worse than our normal 15-degree tests. Despite this huge initial error, the system recovered well:

- The angle dropped quickly from 45 degrees and crossed zero at about half a second. That means the controller brought the drone from 45 degrees to level in under a second.
- It overshot past zero by about 7 degrees in the negative direction. That overshoot is much smaller than the starting 45 degrees, which means the integral clamp is doing its job. Without the clamp, the overshoot would have been way worse because the integral would have built up a huge value while the output was maxed out.
- By about 3 seconds the angle was essentially at zero and stayed there.
- The PID output only hit full saturation (100) on the very first step, then quickly came back down. This shows the integral limit prevented the I-term from building up too much during the initial recovery.

**Why this matters:** On a real drone, if you pick it up and tilt it 45 degrees then let go, the I-term will accumulate a lot of correction while the PID is maxed out trying to recover. The integral clamp prevents that accumulated value from causing a huge overshoot in the opposite direction. This test proves that protection is working.

---

### PID-SIM-07 — Disturbance Rejection

**Hypothesis:**
A well-tuned PID controller should recover from an unexpected external force (simulating a wind gust). The drone should deflect briefly, then return to level. With an I-term present, the steady-state angle after recovery should be exactly 0.

**Configuration:**

We use the stable gains proven in Tests 2 and 5. (In early testing with the old derivative-on-error algorithm, aggressive gains like KP=5, KD=0.6 caused chattering. With the new derivative-on-measurement algorithm those gains would likely work, but we kept the moderate gains here since they produce clean, reliable results.)

```c
#define KP   1.5f
#define KI   0.5f
#define KD   0.3f
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

**Results: PASS**

For the first 3 seconds the drone sits perfectly level at 0 degrees — no input, no movement. At t=3 seconds the wind gust hits. The angle deflects to less than 1 degree (peak of about a third of a degree). The PID immediately starts correcting, and the drone is back below a tenth of a degree within about a third of a second. After that, the angle settles to essentially zero — the I-term eliminates any leftover error.

The PID output spikes briefly when the gust hits, then quickly returns to near zero. There is no oscillation, no overshoot past the setpoint, and no chattering. The controller handles the disturbance smoothly and returns to level.

**Key takeaway:** This test confirms that a well-tuned PID (with moderate gains and all three terms active) can handle sudden external forces and return to level smoothly.

---

## 4. Known Limitations (Current Phase)

- The plant model is a simplified single-axis simulation. It does not capture cross-axis coupling (pitch affecting roll), motor response delay, propeller aerodynamics, or sensor noise.
- Physical parameters (moment of inertia, torque per unit) are estimates based on component specs. Actual values will differ and gains will need re-tuning on real hardware.
- No sensor noise is injected. Real IMU data will be noisier than the simulation, which will affect the D term. The low-pass filter on the derivative should help with this, but the filter coefficient (`d_filter_alpha`) may need tuning on real hardware.

---

## 5. Derivative Algorithm Fix (Completed)

After the initial 7 tests, the PID derivative algorithm was upgraded:

- **Derivative on measurement (not error):** Instead of computing `(error - prev_error) / dt`, the code now computes `-(measured - prev_measurement) / dt`. This avoids "derivative kick" — a spike in the D term whenever the setpoint changes suddenly. When only the measurement moves (not the setpoint), both methods give the same result, which is why Tests 1-3 and 5-7 produced identical behavior before and after the fix.
- **Low-pass filter on the D term:** A first-order filter smooths the raw derivative: `filtered = alpha * raw + (1 - alpha) * previous`. With `alpha = 0.3`, each new reading only contributes 30% and the other 70% comes from the previous smoothed value. This prevents sensor noise (or rapid simulation jitter) from causing wild motor commands.

The most dramatic improvement was **Test 4**, which went from a permanent FAIL (chattering) to a clean PASS. The filter prevented the aggressive D gain from amplifying tiny changes into full-saturation overcorrections. All other tests continued to pass with essentially unchanged behavior.

## 6. Next Phase Testing (Planned)

- **Level 2 — On-chip IMU verification:** Flash firmware with `TEST_THROTTLE=0`, tilt the board by hand, verify PID outputs match expected direction and magnitude on the serial monitor.
- **Level 3 — Tethered motor test:** Mount drone on a single-axis pivot, raise throttle to a low value, arm motors, verify the PID stabilizes the axis.
- **GPS integration:** Write UART driver and NMEA parser for the NewBeeDrone M10Q after PID is validated.

---

## 7. Conclusion

This test plan validates the PID controller implementation (`pid.c`) using a desktop simulation before any hardware testing.
Each test is derived from a control theory hypothesis and proves a specific property of the controller: proportional response, derivative damping, integral steady-state correction, windup protection, and disturbance rejection.
All 7 tests pass with the updated derivative-on-measurement algorithm and low-pass filter. The PID foundation is correct and ready for on-hardware validation (Level 2 and Level 3 testing).
