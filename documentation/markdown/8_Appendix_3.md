### Appendix 3 – Manufacturing Test Plan & Results

This section defines a generic manufacturing and verification test plan for future engineers who build or maintain production versions of the autonomous drone.

---

## 1. Scope & Administrative Details

### Scope

- **System under test:** Fully assembled production drones and subassemblies (flight controller PCB, power system, motors).
- **Goal of testing:** Verify that manufactured product meets the project's functional and safety objectives before being accepted for use (mechanical integrity, power behavior, electronics health, communications, motor mapping, and basic flight/failsafe behavior).
- **Parameters and justification:** Tests focus on parameters that directly impact safety and reliability.
- **Expectations (hypothesis):** Units built to the documented design and assembly process will pass all six manufacturing tests without rework.

### Administrative Details

- **Date and location of testing:** To be filled in per production run
- **Client or organization:** Future owner of the autonomous drone design
- **Conducting the test:** Manufacturing / Quality Assurance engineers or technicians following this plan.

---

## 2. Manufacturing Test Definitions

### TEST ID: MECH-01 — Visual Mechanical & Assembly Inspection

**Scope & goal**

- **System under test**: Fully assembled autonomous drone.
- **Goal**: Confirm each assembled drone matches mechanical drawings and is safe to power.

**Test design (variables, sampling, apparatus)**

- **Type**: Qualitative inspection (yes/no, acceptable/unacceptable).
- **Independent variable**: Individual production unit.
- **Dependent variables**: Pass/fail for each checklist item.
- **Sampling**: Every unit (100% inspection); performed before the unit is powered on for the first time.
- **Apparatus**: Mechanical drawings, good lighting.

**Procedure (outline)**

1. Place powered-off unit on inspection bench and compare it to the latest mechanical drawing.
2. Verify they are the same.
3. Confirm each propeller is installed in the correct position and orientation; rotate slowly by hand to check for interference.
4. Inspect wiring for pinched insulation, unsecured leads, and exposed conductors; verify connectors are fully seated.
5. Record pass/fail against the checklist; photograph any defects.

**Safety & external factors**

- No power applied; battery disconnected during inspection.
- Be cautious of sharp edges on printed parts.
- Note any unusual lighting or visibility conditions that could affect inspection quality.

**Data collection**

- Complete a digital or paper checklist per unit.
- Attach photographs of any defects or borderline conditions.

**Pass criteria**

- 0 cracked, warped, or obviously damaged structural parts.
- 0 mis-oriented or rubbing propellers.
- 0 exposed conductors or unsafe wiring.

---

### TEST ID: PWR-01 — Power-On & Idle Current Verification

**Scope & goal**

- **System under test**: Complete drone with production power electronics and firmware.
- **Goal**: Verify power rails, boot behavior, and idle current are within specification.

**Test design (variables, sampling, apparatus)**

- **Type**: Quantitative electrical test.
- **Independent variable**: Input supply (battery vs. bench supply).
- **Dependent variables**: Boot success, peak inrush current, steady-state idle current, supply voltage.
- **Sampling**: Every unit or defined batch sampling.
- **Apparatus**: Bench supply or production battery, inline current measurement, voltmeter, safe test stand/mat.

**Procedure (outline)**

1. Connect drone to bench supply (with current limit) or a known-good production battery.
2. Measure and record input voltage and current limit settings.
3. Power on the drone and observe peak inrush current and steady-state idle current once boot is complete.
4. Confirm status indicators or telemetry show that firmware booted successfully (no repeating resets).

**Safety & external factors**

- Use current limits on bench supply; keep flammable materials away.
- Ensure adequate ventilation; monitor for abnormal heating or smell.
- Record ambient temperature if it may affect current draw.

**Data collection**

- Log input voltage, current limit, peak inrush current, and steady-state idle current for each unit.
- Record pass/fail plus any anomalies in a shared spreadsheet or database.

**Pass criteria**

- Drone boots from power-on to ready state.
- Peak inrush current and idle current fall within defined allowable ranges.
- No abnormal heating, smell, or cycling resets during the observation period.

---

### TEST ID: PCB-01 — Electronics Self-Test & Sensor Sanity Check

**Scope & goal**

- **System under test**: Assembled flight controller PCB with production firmware.
- **Goal**: Confirm that the microcontroller and critical sensors (e.g., IMU, barometer, magnetometer) are present and functional.

**Test design (variables, sampling, apparatus)**

- **Type**: Mixed qualitative/quantitative functional test.
- **Independent variable**: Unit under test.
- **Dependent variables**: Self-test result code, sensor responsiveness, and basic value ranges.
- **Sampling**: 100 % of PCBs before integration into airframes.
- **Apparatus**: Test jig or debug interface, host PC/machine, self-test script or tool.

**Procedure (outline)**

1. Connect PCB or fully assembled drone to the test jig or debug port.
2. Trigger the firmware self-test routine and read back the summary status.
3. Query each required sensor at rest and record a short window of data for each.
4. Check that values are within plausible ranges.

**Safety & external factors**

- Follow Electrostatic Discharge precautions when handling bare PCBs.
- Ensure board is mechanically supported to avoid stressing solder joints.
- Write down the room temperature and local pressure if they change the sensor readings.

**Data collection**

- Store self-test status codes and raw sensor snapshots with unit serial numbers.
- Keep logs or CSV exports from the test jig software for later analysis.

**Pass criteria**

- Self-test completes with an "OK" or equivalent status code.
- All required sensors respond without communication errors.
- All measured values fall within predefined acceptable ranges for a unit at rest.

---

### TEST ID: COMMS-01 — Control Link & Protocol Verification

**Scope & goal**

- **System under test**: Communication link between ground station (app/controller) and drone, including command/ACK protocol.
- **Goal**: Verify that control commands are correctly received, acknowledged, and constrained by safety rules.

**Test design (variables, sampling, apparatus)**

- **Type**: Functional communications test.
- **Independent variables**: Command type (e.g., ARM, DISARM, ESTOP, basic movement or throttle commands).
- **Dependent variables**: ACK success/failure codes, internal state transitions, latency.
- **Sampling**: 100 % of units.
- **Apparatus**: Ground station or scripted test client, log capture, safe bench or fixture.

**Procedure (outline)**

1. Establish a control link between the ground station and the drone on a bench or fixture.
2. Send a scripted sequence of commands including ARM, small thrust changes, ESTOP, DISARM, and at least one "illegal" command (such as thrust while disarmed).
3. Capture command/ACK logs at both ends and record any internal state reported by telemetry.

**Safety & external factors**

- Perform on a secured bench or fixture; do not allow free-flight for this test.
- Either remove props or limit motor outputs to low test values.
- Minimize radio-frequency interference sources nearby when evaluating latency behavior.

**Data collection**

- Save timestamped command and ACK logs from both ground station and drone.
- Record any protocol errors, timeouts, and observed latencies per command type.

**Pass criteria**

- Every test command receives an ACK with matching ID and correct success/error status.
- Illegal commands (e.g., motor command while disarmed) are rejected and do not change motor state.
- ESTOP forces all motor outputs to a safe value within the specified time and requires a deliberate re-arm before further motion.

---

### TEST ID: SYS-01 — Motor Mapping & Spin-Up Test (Tethered / No Lift-Off)

**Scope & goal**

- **System under test**: Fully assembled drone with motors and ESCs, on a tether or fixture.
- **Goal**: Ensure each logical motor drives the correct physical motor, in the correct direction, with smooth low-power response.

**Test design (variables, sampling, apparatus)**

- **Type**: Functional system-level test.
- **Independent variables**: Commanded motor channel and throttle level.
- **Dependent variables**: Observed motor spin, direction, and current draw.
- **Sampling**: Every unit before first free-flight.
- **Apparatus**: Mechanical fixture or tether, propellers, eye and hearing protection.

**Procedure (outline)**

1. Secure the drone in a test fixture or tether so it cannot lift off.
2. ensure propellers command each motor individually to low and then medium test speeds.
3. Observe which physical motor spins and confirm rotation direction; listen and feel for abnormal vibration or noise.
4. Record current draw per motor channel if available.

**Safety & external factors**

- Always secure the drone in a fixture and keep hands clear of rotating parts.
- Use eye and hearing protection when motors are spinning (if needed).
- Note any unusual ambient vibration or mechanical noise in the test area.

**Data collection**

- Record pass/fail for motor ID and direction per channel.
- Log current draw and any abnormal observations for units that need rework.

**Pass criteria**

- Logical motor IDs match the correct physical motors for all channels.
- All motors spin in the intended direction with no abnormal vibration or noise.
- Measured current per motor lies within expected limits for the test speeds.

---

### TEST ID: FLT-01 — Production Acceptance Hover & Failsafe Test (Sample Units)

**Scope & goal**

- **System under test**: Representative production drones in a safe test environment.
- **Goal**: Confirm that units can maintain a stable hover and execute basic failsafe behavior (e.g., link loss response) as specified.

**Test design (variables, sampling, apparatus)**

- **Type**: System-level flight test.
- **Independent variables**: Unit sample, commanded altitude, control link state.
- **Dependent variables**: Position/altitude deviation during hover, behavior when link is lost or degraded.
- **Sampling**: Any unit that passes the previous tests moves onto this one.
- **Apparatus**: Safe test area or netted flight cage, ground station, means to log telemetry and link state.

**Procedure (outline)**

1. In a designated test area, take off and climb to a fixed test altitude (for example, 2–3 m above ground).
2. Command a hover and maintain it for a defined time window while recording position and altitude deviations.
3. Intentionally drop or degrade the control link according to the documented failsafe scenario and observe behavior until landing or timeout.

**Safety & external factors**

- Conduct tests only in a safe, controlled area (e.g., netted flight cage or open test range) with observers briefed on emergency procedures.
- Comply with all relevant safety and airspace regulations.
- Record environmental conditions such as wind speed, temperature, and lighting.

**Data collection**

- Log telemetry (position, altitude, link status) for the full duration of the test.
- Capture notes or video for any unexpected behaviors during hover or failsafe response.

**Pass criteria**

- During hover, position and altitude remain within defined tolerances/ranges for the duration of the test.
- When the link is lost or degraded, the drone follows the documented failsafe policy (e.g., hover then land, or return-to-home then land) and does not exhibit uncontrolled motion or fly-away behavior.


### TEST ID: SENS-01 — IMU Orientation & Complementary Filter Verification


**Scope & goal**

- **System under test**: Assembled flight controller running `flight_control` firmware with the `main_imu_test.c` build (or `main.c` attitude telemetry).
- **Goal**: Confirm the IMU is detected (WHO_AM_I), produces plausible 6-axis data, and the complementary filter outputs correct, stable pitch/roll.

**Test design (variables, sampling, apparatus)**

- **Type**: Mixed quantitative/qualitative.
- **Independent variable**: Physical orientation of the board (level, pitched ±, rolled ±).
- **Dependent variables**: WHO_AM_I ID, raw accel/gyro values, fused pitch/roll angles.
- **Sampling**: 100 % of flight controller boards before integration.
- **Apparatus**: Bench, USB serial console, a known-flat surface and a reference angle (e.g., 90° block or inclinometer).

**Procedure (outline)**

1. Flash the `main_imu_test` build and connect the USB serial console.
2. Confirm IMU init / WHO_AM_I matches the expected ICM-42670-P ID with no I2C errors.
3. Place the board flat; verify pitch/roll are ~0° and gyro ~0°/s at rest.
4. Tilt to a known pitch (e.g., ~90°) and roll; verify the filter output tracks the angle and direction with correct sign per the mount-rotation config.
5. Return to flat; verify the angle settles back near 0° with no long-term drift over ~30 s.

**Safety & external factors**

- Follow Electrostatic Discharge precautions; support the board to avoid stressing solder joints.
- Keep the board still during the rest check; note any nearby vibration.

**Data collection**

- Record WHO_AM_I, rest pitch/roll/gyro, tracked angles vs. reference, and drift over the hold window.

**Pass criteria**

- WHO_AM_I matches the expected ID; no repeated I2C read errors.
- At rest, pitch/roll within a small tolerance of 0° and gyro near 0°/s.
- Fused angles track applied tilt in the correct direction and approximate magnitude.
- Angle returns to ~0° at rest with negligible drift over the observation window.

---

### TEST ID: SENS-02 — GPS, Compass & Barometer Telemetry and Calibration Verification

**What we implemented & why**

- `wifi_gps_softap` integrates a UART GPS (NMEA parser → `gps_fix_t`), a magnetometer (QMC5883/HMC5883L, auto-detected) with runtime hard-iron calibration and GOOD/PARTIAL/POOR quality reporting, and a BMP280/BME280 barometer that reads factory calibration coefficients and establishes a relative-altitude baseline.
- These feed the navigation/telemetry the phone uses for distance and heading.
- We added runtime hard-iron calibration because magnetometers are sensitive to nearby ferrous material and wiring that differ per build, so per-unit calibration is needed for trustworthy heading.

**Scope & goal**

- **System under test**: Drone running `wifi_gps_softap` firmware.
- **Goal**: Verify each environmental sensor initializes, calibrates, and reports plausible telemetry over `GET /gps` and the WSS feed.

**Test design (variables, sampling, apparatus)**

- **Type**: Mixed quantitative/qualitative.
- **Independent variables**: Physical heading (rotate the unit through known headings), known outdoor location (for GPS), vertical displacement (for baro).
- **Dependent variables**: GPS fix validity/lat/lon, compass heading + calibration quality, baro relative altitude.
- **Sampling**: 100 % of integrated units (GPS verified at a sky-view location).
- **Apparatus**: Open-sky test spot, host joined to the drone SoftAP, browser/WSS client, a reference heading (compass app) and a known height change (e.g., lift 1 m).

**Procedure (outline)**

1. Power the unit, join its SoftAP, and confirm `GET /` health responds.
2. Perform the compass calibration routine (rotate through a full circle); read back calibration quality.
3. Outdoors, wait for a GPS fix; `GET /gps` and confirm a valid fix with plausible lat/lon.
4. Rotate the unit to known headings; compare reported heading to the reference.
5. Raise the unit a known height; confirm baro relative altitude changes in the right direction and approximate magnitude.
6. Open the WSS feed; confirm JSON fields (`droneGpsValid`, `droneLat`/`droneLon`, `droneHeadingDeg`, `altM`, `droneBaroOk`) update at ~5 Hz.

**Safety & external factors**

- Outdoor GPS testing: keep clear of traffic and obstacles; propellers off.
- Note nearby metal/magnetic interference and weather conditions that affect GPS or compass.

**Data collection**

- Log calibration quality, GPS fix and coordinates, heading error vs. reference, baro delta, and WSS sample rate.

**Pass criteria**

- Compass calibration reaches GOOD (or documented acceptable) quality; heading within tolerance of the reference.
- GPS reports a valid fix with coordinates consistent with the test location.
- Baro reports plausible relative altitude that tracks the known height change; `droneBaroOk` is true.
- WSS telemetry streams the expected JSON fields at ~5 Hz without dropping the link.

---

### TEST ID: CTRL-01 — Tethered Closed-Loop Stabilization (PID) Verification

**Scope & goal**

- **System under test**: Fully assembled drone, `flight_control` firmware, secured on a tether/gimbal fixture, props on, `TETHER_BRINGUP_MODE` enabled.
- **Goal**: Confirm the closed-loop controller actively corrects attitude disturbances and holds level without oscillation or runaway.

**Test design (variables, sampling, apparatus)**

- **Type**: Functional system-level test.
- **Independent variables**: Manually induced pitch/roll disturbances.
- **Dependent variables**: Motor response (which corner spins up), recovery toward level, oscillation/overshoot.
- **Sampling**: Every unit before first free-flight.
- **Apparatus**: Tether/gimbal that allows tilt but prevents lift-off/translation, eye and hearing protection, telemetry/serial for attitude and motor outputs.

**Procedure (outline)**

1. Secure the drone in the tether fixture; confirm props are clear; enable tether mode.
2. ARM; confirm motors spin to the capped hover baseline.
3. Manually pitch the airframe nose-down; verify the controller increases the appropriate motors to push back toward level.
4. Repeat for nose-up, roll-left, and roll-right.
5. Release from a small tilt; verify it returns to level without sustained oscillation.
6. DISARM; confirm the smooth ramp-down.

**Safety & external factors**

- The drone must be physically restrained; keep hands clear of props; use eye and hearing protection; keep ESTOP within reach.
- Note unusual fixture vibration or play that could affect the response.

**Data collection**

- Log attitude and per-motor outputs during disturbances; note any oscillation, overshoot, or wrong-direction response.

**Pass criteria**

- For each disturbance, the correct motors respond to drive the airframe back toward level (correct sign).
- The airframe returns to ~level after release without sustained or growing oscillation.
- Throttle stays at or below the tether cap; no runaway.
- DISARM produces the documented ~900 ms ramp-down to idle.

---

### TEST ID: AUTO-01 — Autonomy Command & Decision Verification (Phone Commands + Drone Terminal)



**Scope & goal**

- **System under test**: Mobile app follow controller + drone running `wifi_gps_softap` with a UART console attached.
- **Goal**: Confirm correct state transitions/thresholds and that the matching NAV intent is transmitted over BLE and observed on the drone.

**Test design (variables, sampling, apparatus)**

- **Type**: Functional integration + unit test.
- **Independent variables**: Simulated or real phone-vs-drone distance and yaw error; Start/Stop.
- **Dependent variables**: Follow state, NAV opcode sent, drone-side log of the received command.
- **Sampling**: Per firmware/app release.
- **Apparatus**: Phone with the app, drone on bench (props off) with USB/UART console, BLE link; Jest for unit-level threshold checks.

**Procedure (outline)**

1. Run the Jest follow suite (`follow-mock.test.ts`, `use-follow-mock-controller.test.tsx`); confirm threshold/transition cases pass (arrival 3.0 m, standoff 1.5 m, rotate 10°/25°, IDLE on stale/unhealthy fix).
2. On hardware: connect the app to the drone over BLE; open the drone UART console.
3. Start follow and vary distance so the phone crosses thresholds: far → FORWARD, within 3 m → HOLD, under 1.5 m → RETREAT, large yaw → ROTATE.
4. Confirm each phase change emits the matching NAV_* opcode and the drone logs receiving it.
5. Stop follow (or drop the GPS fix / let the snapshot go stale > 1.5 s); confirm NAV_IDLE is sent and the state returns to IDLE.

**Safety & external factors**

- Props off / motors not driven; this test verifies command and decision logic only.
- Minimize radio interference when evaluating BLE delivery.

**Data collection**

- Jest results; app state log; drone UART log of received opcodes; a mapping table of distance/yaw → state → opcode.

**Pass criteria**

- The Jest suite passes.
- State transitions occur at the documented distance/yaw thresholds (with hysteresis, no flicker).
- Each phase change sends the correct NAV intent and the drone logs the matching command.
- Turning off follow or losing the fix returns to IDLE and emits NAV_IDLE.

---

### TEST ID: AUTO-02 — Full Autonomous Follow Flight (Physical)

**Scope & goal**

- **System under test**: Drone with autonomy-enabled flight firmware, in a safe flight area.
- **Goal**: Confirm the drone physically follows the user, maintaining the configured standoff band.

**Test design (variables, sampling, apparatus)**

- **Type**: System-level autonomous flight test.
- **Independent variables**: User (phone) position and motion.
- **Dependent variables**: Drone-to-user distance over time, response to user approach/retreat, behavior on stop/link loss.
- **Sampling**: Sample units that pass CTRL-01, SAFE-01, and AUTO-01.
- **Apparatus**: Netted flight cage or open test range, phone, telemetry logging, observers briefed on emergency procedures.

**Procedure (outline)**

1. In a safe area, arm and bring the drone to a stable hover; enable follow.
2. Walk away from the drone; verify it approaches to re-establish the arrival radius (3.0 m).
3. Walk toward the drone past the minimum standoff (1.5 m); verify it backs off to maintain standoff.
4. Stand still within range; verify it holds position.
5. Turn off follow (and separately, cut the link); verify it stops following and executes the documented safe behavior.

**Safety & external factors**

- Controlled area only; comply with airspace and safety regulations; keep ESTOP ready; keep observers clear of the flight path.
- Record environmental conditions such as wind, temperature, and lighting.

**Data collection**

- Telemetry of distance/position over the full test; video; notes on overshoot or instability.

**Pass criteria**

- The drone maintains distance within the configured standoff band (≈1.5–3.0 m) as the user moves.
- It approaches when the user moves away, retreats when the user gets too close, and holds when in range.
- On follow-off or link loss, it stops following and performs the documented failsafe (no uncontrolled motion or fly-away).

---

### TEST ID: SAFE-01 — Failsafe & Link-Loss Behavior Verification


**Scope & goal**

- **System under test**: Assembled drone, `flight_control` firmware, tether fixture, props on (or motor outputs observed).
- **Goal**: Verify each failsafe forces a safe state under its specified condition.

**Test design (variables, sampling, apparatus)**

- **Type**: Functional safety test.
- **Independent variables**: Triggered fault (ESTOP command, heartbeat stop, BLE disconnect, IMU bus fault).
- **Dependent variables**: Resulting motor state, time-to-safe, re-arm requirement.
- **Sampling**: 100 % of units before free-flight.
- **Apparatus**: Tether, ground station/app, ability to cut the BLE link and (on bench) interrupt the IMU bus, serial/telemetry logging.

**Procedure (outline)**

1. ARM on the tether (tether mode). Send ESTOP; confirm motors go to a safe state immediately and re-arm is required before further motion.
2. ARM; stop sending heartbeat/commands; confirm auto-disarm after the link-timeout window.
3. ARM; force a BLE disconnect; confirm immediate auto-disarm.
4. (Bench, carefully) ARM and induce repeated IMU read failures; confirm disarm after the consecutive-failure threshold.
5. Observe that each disarm uses the smooth ramp-down.

**Safety & external factors**

- Tethered, props guarded, hands clear, with a hardware ESTOP/power cutoff ready.
- Be prepared for an unexpected motor response when inducing the IMU fault.

**Data collection**

- Log trigger time, time-to-safe, and re-arm behavior for each scenario.

**Pass criteria**

- ESTOP forces a safe state and blocks motion until deliberate re-arm.
- The no-heartbeat condition auto-disarms within the documented timeout.
- BLE disconnect auto-disarms.
- The IMU failure threshold triggers a disarm.
- Each disarm follows the documented ramp-down; no uncommanded spin-up.

---

### TEST ID: LINK-01 — Wi-Fi SoftAP, Secure Provisioning & Telemetry Link Verification

**Scope & goal**

- **System under test**: Drone running `wifi_gps_softap`; phone or laptop client.
- **Goal**: Verify the AP, secure endpoints, telemetry stream, and provisioning/rotation/reset all function.

**Test design (variables, sampling, apparatus)**

- **Type**: Functional communications/security test.
- **Independent variables**: Client actions (join, provision, rotate, factory reset).
- **Dependent variables**: Endpoint responses, telemetry stream health, password change behavior.
- **Sampling**: 100 % of integrated units.
- **Apparatus**: WSS/HTTPS-capable client, the documented factory SSID/password (matching mobile `drone-defaults`), log capture.

**Procedure (outline)**

1. Power the unit; confirm the SoftAP advertises the expected SSID; join with the factory password.
2. `GET /` (health) and `GET /gps`; confirm valid responses over TLS.
3. Open WSS `/ws`; confirm telemetry JSON at ~5 Hz.
4. `POST /wifi/provision` (or `/wifi/rotate-password`); confirm the password changes and the new credentials are required on reconnect.
5. `POST /wifi/factory-reset`; confirm it returns to the factory password.

**Safety & external factors**

- Bench test; no motors involved.
- Note nearby Wi-Fi congestion that could affect throughput or join reliability.

**Data collection**

- Log endpoint responses, WSS rate, and provisioning/rotation/reset outcomes.

**Pass criteria**

- The SoftAP joins with factory credentials; TLS endpoints respond correctly.
- WSS telemetry streams the expected fields at ~5 Hz.
- Provision/rotate changes the password and old credentials no longer work.
- Factory reset restores the factory password.

---

### TEST ID: APP-01 — Mobile App End-to-End Command/Telemetry Integration


**Scope & goal**

- **System under test**: Mobile app built against a real drone (`flight_control` for commands and/or `wifi_gps_softap` for telemetry).
- **Goal**: Confirm the app connects, sends correct commands with ACKs, displays live telemetry, maintains heartbeat, and that global ESTOP works.

**Test design (variables, sampling, apparatus)**

- **Type**: Functional end-to-end integration + unit test.
- **Independent variables**: User actions (connect, ARM/DISARM/ESTOP, motor steps, takeoff/land), transport state.
- **Dependent variables**: Command bytes + ACKs, telemetry fields shown, heartbeat cadence, link indicators.
- **Sampling**: Per app release / representative units.
- **Apparatus**: Phone (real-comms build, not the Expo Go mock), drone on bench (props off), BLE + Wi-Fi, Jest.

**Procedure (outline)**

1. Run `npm test` (Jest); confirm the comms/telemetry/nav/autonomy suites pass.
2. On device: scan and connect over BLE; join Wi-Fi/SoftAP for telemetry.
3. Send ARM, a SET_MOTOR step (per firmware bench semantics), and DISARM; confirm correct command frames and matching ACKs.
4. Confirm telemetry (link/RSSI, GPS, heading) updates on the dashboard from the WSS feed.
5. Confirm the BLE heartbeat fires roughly every 800 ms.
6. Press the global ESTOP; confirm an ESTOP command is sent and reflected in drone state.

**Safety & external factors**

- Props off; motors not flown; bench only.
- Note RF congestion that could affect BLE/Wi-Fi reliability.

**Data collection**

- Jest results; captured command/ACK logs; telemetry screenshots; heartbeat timing.

**Pass criteria**

- The Jest suite passes.
- The app connects on both transports; commands produce correct frames and matching ACKs.
- Telemetry fields display and update from the live feed.
- Heartbeat cadence is ~800 ms; link indicators reflect actual state.
- Global ESTOP reliably sends ESTOP and the drone enters a safe state.
