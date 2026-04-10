"""
plot_sim.py — plot the CSV output from quad_sim

Usage:
    python3 plot_sim.py run.csv
    python3 plot_sim.py              (defaults to run.csv)
"""

import sys
import csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

def main():
    path = sys.argv[1] if len(sys.argv) > 1 else "run.csv"

    time_s, angle, ang_vel, pid_out, setpoint = [], [], [], [], []

    with open(path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            time_s.append(float(row["time_s"]))
            angle.append(float(row["angle_deg"]))
            ang_vel.append(float(row["angular_vel_dps"]))
            pid_out.append(float(row["pid_output"]))
            setpoint.append(float(row["setpoint"]))

    fig, axes = plt.subplots(3, 1, figsize=(10, 8), sharex=True)

    axes[0].plot(time_s, angle, label="angle (deg)", linewidth=1.5)
    axes[0].plot(time_s, setpoint, "--", label="setpoint", linewidth=1, color="gray")
    axes[0].set_ylabel("Angle (deg)")
    axes[0].legend()
    axes[0].set_title("PID Single-Axis Simulation")
    axes[0].grid(True, alpha=0.3)

    axes[1].plot(time_s, ang_vel, label="angular velocity (deg/s)",
                 linewidth=1.5, color="orange")
    axes[1].set_ylabel("Angular vel (deg/s)")
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)

    axes[2].plot(time_s, pid_out, label="PID output", linewidth=1.5, color="green")
    axes[2].set_ylabel("PID output")
    axes[2].set_xlabel("Time (s)")
    axes[2].legend()
    axes[2].grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig("sim_plot.png", dpi=150)
    print("Saved sim_plot.png — open it to see the plots")
    try:
        plt.show()
    except Exception:
        pass

if __name__ == "__main__":
    main()
