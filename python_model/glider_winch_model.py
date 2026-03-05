"""
Skylaunch Glider Winch Model
============================

Steady-state model of a Skylaunch-type glider winch drivetrain:

    Engine  -->  Torque Converter  -->  TH400 3-speed Auto  -->  Final Drive  -->  Drum  -->  Cable

Engine:  GM 7.4 L (454 ci) Big Block V8, LPG fuelled
         250 kW @ 4 600 RPM, 597 Nm @ 3 400 RPM  (Skylaunch published data)

Transmission:  Turbo-Hydramatic 400  (2.48 / 1.48 / 1.00)

The glider controls cable speed — the winch reacts to the load.
For a given cable speed and throttle position the model solves for the
engine equilibrium RPM, then propagates torque through the drivetrain
to give cable tension.

Usage:
    python glider_winch_model.py              # prints examples + saves plots
    python glider_winch_model.py --no-plot    # text output only
"""

import argparse
import sys

import numpy as np
from scipy.interpolate import CubicSpline
from scipy.optimize import brentq


# ---------------------------------------------------------------------------
#  Engine
# ---------------------------------------------------------------------------

class Engine:
    """GM 7.4 L V8 on LPG — Skylaunch 3 rated figures.

    Published:  250 kW @ 4 600 RPM,  597 Nm @ 3 400 RPM.
    The full-throttle torque curve is a cubic spline through
    measured-style anchor points that honour those two values.

    Partial throttle is modelled with a butterfly-valve characteristic
    (roughly quadratic at small openings).
    """

    def __init__(self):
        self.idle_rpm = 700
        self.max_rpm = 5000

        # Full-throttle torque anchors  (RPM, Nm) — LPG, as installed
        rpm_pts = np.array([700, 1000, 1500, 2000, 2500, 3000, 3400,
                            4000, 4600, 5000])
        nm_pts = np.array([300, 380, 455, 515, 560, 588, 597,
                           570, 519, 470])
        #   Check:  250 kW @ 4600  →  519 Nm  ✓
        #           Peak torque 597 Nm @ 3400  ✓

        self._wot = CubicSpline(rpm_pts, nm_pts)

    def torque(self, rpm, throttle):
        """Torque (Nm) at *rpm* and *throttle* (0–1)."""
        rpm = np.clip(rpm, self.idle_rpm, self.max_rpm)
        wot = float(self._wot(rpm))

        # Butterfly-valve flow ≈ throttle^1.5 at small openings
        eff = float(np.clip(throttle, 0.0, 1.0)) ** 1.5

        # Even at closed throttle the engine produces some idle torque
        idle_frac = 0.25
        idle_nm = float(self._wot(self.idle_rpm)) * idle_frac
        return idle_nm + (wot - idle_nm) * eff

    def power_kw(self, rpm, throttle):
        return self.torque(rpm, throttle) * rpm * np.pi / 30_000


# ---------------------------------------------------------------------------
#  Torque converter
# ---------------------------------------------------------------------------

class TorqueConverter:
    """Generic TH400-style fluid torque converter.

    Characterised by two curves that depend only on speed ratio
    SR = N_turbine / N_pump:

        Torque ratio   TR(SR)  — multiplication from ~2.1 at stall to 1.0
                                  at the coupling point.
        Capacity factor K(SR)  — relates pump speed to pump torque:
                                  T_pump = (N_pump / K)^2   [Nm, RPM]

    K-factor at stall is back-calculated so the engine stalls at
    approximately the expected converter stall speed.
    """

    def __init__(self, stall_torque_ratio=2.1, coupling_point=0.87):
        self.coupling_point = coupling_point
        cp = coupling_point

        # --- Torque-ratio curve ---
        sr = [0.00, 0.10, 0.20, 0.30, 0.40, 0.50,
              0.60, 0.70, 0.80, cp,   0.93, 0.97, 1.00]
        tr = [stall_torque_ratio,
              2.05, 1.97, 1.86, 1.73, 1.58,
              1.41, 1.24, 1.10, 1.00, 1.00, 1.00, 1.00]
        self._tr = CubicSpline(sr, tr)

        # --- K-factor curve  (RPM / sqrt(Nm)) ---
        # Stall K chosen so a 597 Nm-peak engine stalls ≈ 1 800 RPM:
        #   T_pump(stall) ≈ engine torque at 1800 RPM ≈ 430 Nm
        #   K_stall = 1800 / sqrt(430) ≈ 86.8
        kf = [86.8, 88, 91, 95, 100, 107,
              117, 132, 155, 195, 240, 310, 420]
        self._kf = CubicSpline(sr, kf)

    def torque_ratio(self, sr):
        return float(self._tr(np.clip(sr, 0.0, 1.0)))

    def k_factor(self, sr):
        return float(self._kf(np.clip(sr, 0.0, 1.0)))

    def pump_torque(self, pump_rpm, sr):
        """Torque absorbed by the pump (= engine output) in Nm."""
        k = self.k_factor(sr)
        return (pump_rpm / k) ** 2

    def turbine_torque(self, pump_rpm, sr):
        """Torque delivered to the turbine (= trans input) in Nm."""
        return self.pump_torque(pump_rpm, sr) * self.torque_ratio(sr)


# ---------------------------------------------------------------------------
#  TH400  transmission
# ---------------------------------------------------------------------------

class Transmission:
    """GM Turbo-Hydramatic 400 — three forward speeds.

    Ratios from GM specifications.  No lock-up clutch
    (the TH400 predates that feature).
    """

    RATIOS = {1: 2.48, 2: 1.48, 3: 1.00}
    EFFICIENCY = 0.95          # per-gear mesh efficiency

    def output_torque(self, input_torque, gear):
        return input_torque * self.RATIOS[gear] * self.EFFICIENCY

    def input_speed(self, output_rpm, gear):
        """Turbine RPM for a given gearbox output RPM."""
        return output_rpm * self.RATIOS[gear]


# ---------------------------------------------------------------------------
#  Winch drum
# ---------------------------------------------------------------------------

class Drum:
    """Cable drum with variable effective radius.

    Skylaunch drums hold up to 2 000 m of 4.6 mm cable.
    The description is "large diameter, narrow width" (no published
    dimensions), so reasonable estimates are used.

    As cable pays out the winding radius decreases → higher tension,
    lower cable speed for the same drum RPM.
    """

    def __init__(self, inner_radius=0.35, width=0.20,
                 cable_diameter=0.0046, cable_length=2000):
        self.inner_radius = inner_radius          # m
        self.width = width                        # m
        self.cable_diameter = cable_diameter       # m
        self.cable_length = cable_length           # m
        self.wraps_per_layer = int(width / cable_diameter)

    def effective_radius(self, cable_out):
        """Radius (m) at the cable departure point."""
        on_drum = self.cable_length - cable_out
        if on_drum <= 0:
            return self.inner_radius

        remaining = on_drum
        layer = 0
        while remaining > 0:
            r_centre = self.inner_radius + (layer + 0.5) * self.cable_diameter
            layer_cap = self.wraps_per_layer * 2 * np.pi * r_centre
            if remaining <= layer_cap:
                frac = remaining / layer_cap
                return self.inner_radius + (layer + frac) * self.cable_diameter
            remaining -= layer_cap
            layer += 1

        return self.inner_radius + layer * self.cable_diameter

    def max_layers(self):
        return int(np.ceil(
            (self.effective_radius(0) - self.inner_radius) / self.cable_diameter
        ))

    def rpm(self, cable_speed, cable_out):
        r = self.effective_radius(cable_out)
        return cable_speed / r * 30 / np.pi          # rad/s → RPM


# ---------------------------------------------------------------------------
#  Complete winch model
# ---------------------------------------------------------------------------

class GliderWinch:
    """Combines all drivetrain components and solves for equilibrium.

    For a given cable speed (set by the glider) and throttle position
    the solver finds the engine RPM where:

        T_engine(RPM, throttle)  =  T_pump(RPM, SR)

    where SR = N_turbine / N_pump and N_turbine is fixed by the cable
    speed working back through the drum, final drive, and gearbox.
    """

    def __init__(self, final_drive_ratio=4.0, final_drive_efficiency=0.97):
        self.engine = Engine()
        self.converter = TorqueConverter()
        self.transmission = Transmission()
        self.drum = Drum()
        self.final_drive_ratio = final_drive_ratio
        self.final_drive_efficiency = final_drive_efficiency

    # ---- gear selection heuristic ----

    def _best_gear(self, cable_speed, throttle, cable_out):
        """Pick the gear that keeps the engine in a productive RPM band."""
        best = None
        for g in (1, 2, 3):
            r = self._solve_gear(g, cable_speed, throttle, cable_out)
            if r is None:
                continue
            rpm = r["engine_rpm"]
            if not (self.engine.idle_rpm <= rpm <= self.engine.max_rpm):
                continue
            if best is None:
                best = r
            # prefer highest gear that keeps RPM above 1 400
            elif rpm >= 1400 and g > best["gear"]:
                best = r
        return best

    # ---- per-gear solver ----

    def _solve_gear(self, gear, cable_speed, throttle, cable_out):
        r_eff = self.drum.effective_radius(cable_out)
        drum_rpm = self.drum.rpm(cable_speed, cable_out)
        trans_out_rpm = drum_rpm * self.final_drive_ratio
        turbine_rpm = self.transmission.input_speed(trans_out_rpm, gear)

        def balance(engine_rpm):
            sr = turbine_rpm / engine_rpm if engine_rpm > 0 else 0.0
            sr = np.clip(sr, 0.0, 0.999)
            return (self.engine.torque(engine_rpm, throttle)
                    - self.converter.pump_torque(engine_rpm, sr))

        rpm_lo = max(self.engine.idle_rpm, turbine_rpm + 1)
        rpm_hi = self.engine.max_rpm
        if rpm_lo >= rpm_hi:
            return None

        # look for a sign change
        pts = np.linspace(rpm_lo, rpm_hi, 40)
        vals = [balance(p) for p in pts]
        root = None
        for i in range(len(vals) - 1):
            if vals[i] * vals[i + 1] < 0:
                root = brentq(balance, pts[i], pts[i + 1])
                break
        if root is None:
            return None

        engine_rpm = root
        sr = np.clip(turbine_rpm / engine_rpm, 0.0, 0.999)
        t_eng = self.engine.torque(engine_rpm, throttle)
        t_turb = self.converter.turbine_torque(engine_rpm, sr)
        t_trans = self.transmission.output_torque(t_turb, gear)
        t_drum = t_trans * self.final_drive_ratio * self.final_drive_efficiency
        tension = t_drum / r_eff

        p_eng = self.engine.power_kw(engine_rpm, throttle)
        p_cable = tension * cable_speed / 1000

        return {
            "cable_speed_ms":      cable_speed,
            "cable_speed_kt":      cable_speed * 1.94384,
            "cable_speed_kmh":     cable_speed * 3.6,
            "throttle":            throttle,
            "cable_out":           cable_out,
            "gear":                gear,
            "engine_rpm":          engine_rpm,
            "engine_torque_nm":    t_eng,
            "engine_power_kw":     p_eng,
            "speed_ratio":         sr,
            "torque_ratio":        self.converter.torque_ratio(sr),
            "turbine_rpm":         turbine_rpm,
            "turbine_torque_nm":   t_turb,
            "drum_rpm":            self.drum.rpm(cable_speed, cable_out),
            "drum_radius_m":       r_eff,
            "cable_tension_n":     tension,
            "cable_tension_kgf":   tension / 9.807,
            "cable_tension_dan":   tension / 9.807,   # daN ≈ kgf
            "cable_power_kw":      p_cable,
            "efficiency":          p_cable / p_eng if p_eng > 0 else 0,
        }

    # ---- public interface ----

    def solve(self, cable_speed, throttle, cable_out=1000, gear=None):
        """Find the steady-state operating point.

        Parameters
        ----------
        cable_speed : float   m/s  (set by the glider)
        throttle    : float   0–1
        cable_out   : float   metres of cable paid out
        gear        : int|None  force gear (1/2/3) or None for auto

        Returns
        -------
        dict  or  None if no valid equilibrium exists.
        """
        if cable_speed <= 0:
            return None
        if gear is not None:
            return self._solve_gear(gear, cable_speed, throttle, cable_out)
        return self._best_gear(cable_speed, throttle, cable_out)


# ---------------------------------------------------------------------------
#  Formatted output
# ---------------------------------------------------------------------------

def print_point(r):
    if r is None:
        print("  (no valid operating point)")
        return
    print(f"  Cable speed     {r['cable_speed_ms']:6.1f} m/s   "
          f"({r['cable_speed_kt']:.0f} kt / {r['cable_speed_kmh']:.0f} km/h)")
    print(f"  Throttle        {r['throttle']*100:6.0f} %")
    print(f"  Gear                 {r['gear']}")
    print(f"  Engine RPM      {r['engine_rpm']:6.0f}")
    print(f"  Engine torque   {r['engine_torque_nm']:6.0f} Nm")
    print(f"  Engine power    {r['engine_power_kw']:6.1f} kW")
    print(f"  Speed ratio     {r['speed_ratio']:6.3f}")
    print(f"  Torque ratio    {r['torque_ratio']:6.2f}")
    print(f"  Drum radius     {r['drum_radius_m']*100:6.1f} cm")
    print(f"  Cable tension   {r['cable_tension_kgf']:6.0f} kgf  "
          f"({r['cable_tension_n']:.0f} N)")
    print(f"  Cable power     {r['cable_power_kw']:6.1f} kW")
    print(f"  Efficiency      {r['efficiency']*100:6.1f} %")


# ---------------------------------------------------------------------------
#  Plotting
# ---------------------------------------------------------------------------

def _import_mpl():
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    return plt


def plot_engine(winch, save="engine_curves.png"):
    plt = _import_mpl()
    rpms = np.linspace(winch.engine.idle_rpm, winch.engine.max_rpm, 300)
    tq = [winch.engine.torque(r, 1.0) for r in rpms]
    pw = [winch.engine.power_kw(r, 1.0) for r in rpms]

    fig, ax1 = plt.subplots(figsize=(10, 6))
    ax1.plot(rpms, tq, "b-", lw=2, label="Torque (Nm)")
    ax1.set_xlabel("Engine RPM")
    ax1.set_ylabel("Torque  (Nm)", color="b")
    ax1.tick_params(axis="y", labelcolor="b")
    ax1.grid(True, alpha=0.3)

    ax2 = ax1.twinx()
    ax2.plot(rpms, pw, "r-", lw=2, label="Power (kW)")
    ax2.set_ylabel("Power  (kW)", color="r")
    ax2.tick_params(axis="y", labelcolor="r")

    h1, l1 = ax1.get_legend_handles_labels()
    h2, l2 = ax2.get_legend_handles_labels()
    ax1.legend(h1 + h2, l1 + l2, loc="upper left")
    ax1.set_title("GM 7.4 L V8 on LPG — full throttle")
    fig.tight_layout()
    fig.savefig(save, dpi=150)
    plt.close(fig)
    print(f"  saved {save}")


def plot_converter(winch, save="converter_curves.png"):
    plt = _import_mpl()
    sr = np.linspace(0, 1, 300)
    tr = [winch.converter.torque_ratio(s) for s in sr]
    kf = [winch.converter.k_factor(s) for s in sr]

    fig, ax1 = plt.subplots(figsize=(10, 6))
    ax1.plot(sr, tr, "b-", lw=2, label="Torque ratio")
    ax1.axvline(winch.converter.coupling_point, color="grey", ls="--",
                alpha=0.5, label=f"Coupling (SR={winch.converter.coupling_point})")
    ax1.set_xlabel("Speed ratio  N_turbine / N_pump")
    ax1.set_ylabel("Torque ratio", color="b")
    ax1.tick_params(axis="y", labelcolor="b")
    ax1.grid(True, alpha=0.3)

    ax2 = ax1.twinx()
    ax2.plot(sr, kf, "r-", lw=2, label="K-factor")
    ax2.set_ylabel("K-factor  (RPM/√Nm)", color="r")
    ax2.tick_params(axis="y", labelcolor="r")

    h1, l1 = ax1.get_legend_handles_labels()
    h2, l2 = ax2.get_legend_handles_labels()
    ax1.legend(h1 + h2, l1 + l2, loc="center right")
    ax1.set_title("TH400 torque converter characteristics")
    fig.tight_layout()
    fig.savefig(save, dpi=150)
    plt.close(fig)
    print(f"  saved {save}")


def plot_drum(winch, save="drum_radius.png"):
    plt = _import_mpl()
    co = np.linspace(0, winch.drum.cable_length, 300)
    radii = [winch.drum.effective_radius(c) * 100 for c in co]

    fig, ax = plt.subplots(figsize=(10, 5))
    ax.plot(co, radii, "b-", lw=2)
    ax.set_xlabel("Cable paid out  (m)")
    ax.set_ylabel("Effective drum radius  (cm)")
    ax.set_title("Drum radius vs cable payout  "
                 f"(4.6 mm × {winch.drum.cable_length} m)")
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(save, dpi=150)
    plt.close(fig)
    print(f"  saved {save}")


def plot_tension_vs_speed(winch, cable_out=1000,
                          save="tension_vs_speed.png"):
    """Cable tension vs cable speed at several throttle settings."""
    plt = _import_mpl()
    speeds = np.linspace(0.5, 38, 150)
    throttles = [0.25, 0.50, 0.75, 1.00]
    colours = ["#2196F3", "#4CAF50", "#FF9800", "#F44336"]

    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    fig.suptitle(
        f"Skylaunch winch characteristics  —  cable {cable_out} m out\n"
        "7.4 L LPG  /  TH400  /  torque converter",
        fontsize=13,
    )

    for thr, col in zip(throttles, colours):
        vs, tens, rpms, pws, effs, grs = [], [], [], [], [], []
        for v in speeds:
            r = winch.solve(v, thr, cable_out)
            if r is None:
                continue
            vs.append(r["cable_speed_kt"])
            tens.append(r["cable_tension_kgf"])
            rpms.append(r["engine_rpm"])
            pws.append(r["cable_power_kw"])
            effs.append(r["efficiency"] * 100)
            grs.append(r["gear"])
        if not vs:
            continue
        lbl = f"{int(thr*100)} % throttle"
        axes[0, 0].plot(vs, tens, color=col, lw=2, label=lbl)
        axes[0, 1].plot(vs, rpms, color=col, lw=2, label=lbl)
        axes[1, 0].plot(vs, pws,  color=col, lw=2, label=lbl)
        axes[1, 1].plot(vs, effs, color=col, lw=2, label=lbl)

    # Weak-link reference lines on the tension plot
    for wl, style, lbl in [(450, ":", "450 kgf  (light)"),
                            (750, "--", "750 kgf  (medium)"),
                            (1000, "-.", "1 000 kgf  (heavy)")]:
        axes[0, 0].axhline(wl, color="grey", ls=style, alpha=0.5, label=lbl)

    titles = ["Cable tension", "Engine RPM",
              "Power delivered to cable", "Overall drivetrain efficiency"]
    ylabels = ["Tension  (kgf)", "RPM", "Power  (kW)", "Efficiency  (%)"]
    for ax, t, yl in zip(axes.flat, titles, ylabels):
        ax.set_xlabel("Cable speed  (knots)")
        ax.set_ylabel(yl)
        ax.set_title(t)
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.3)
    axes[0, 0].set_ylim(bottom=0)
    axes[1, 0].set_ylim(bottom=0)
    axes[1, 1].set_ylim(0, 100)

    fig.tight_layout()
    fig.savefig(save, dpi=150)
    plt.close(fig)
    print(f"  saved {save}")


def plot_cable_out_effect(winch, save="cable_out_effect.png"):
    """Full throttle tension for different cable payout lengths."""
    plt = _import_mpl()
    cable_outs = [200, 500, 1000, 1500, 1900]
    colours = ["#9C27B0", "#2196F3", "#4CAF50", "#FF9800", "#F44336"]
    speeds = np.linspace(0.5, 38, 150)

    fig, ax = plt.subplots(figsize=(10, 6))
    for co, col in zip(cable_outs, colours):
        vs, ts = [], []
        for v in speeds:
            r = winch.solve(v, 1.0, co)
            if r is None:
                continue
            vs.append(r["cable_speed_kt"])
            ts.append(r["cable_tension_kgf"])
        if vs:
            r_cm = winch.drum.effective_radius(co) * 100
            ax.plot(vs, ts, color=col, lw=2,
                    label=f"{co} m out  (R = {r_cm:.1f} cm)")

    for wl, style, lbl in [(450, ":", "450 kgf"),
                            (750, "--", "750 kgf"),
                            (1000, "-.", "1 000 kgf")]:
        ax.axhline(wl, color="grey", ls=style, alpha=0.5, label=lbl)

    ax.set_xlabel("Cable speed  (knots)")
    ax.set_ylabel("Cable tension  (kgf)")
    ax.set_title("Effect of cable payout on tension  (full throttle)")
    ax.legend(fontsize=9)
    ax.grid(True, alpha=0.3)
    ax.set_ylim(bottom=0)
    fig.tight_layout()
    fig.savefig(save, dpi=150)
    plt.close(fig)
    print(f"  saved {save}")


def plot_per_gear(winch, cable_out=1000, save="per_gear.png"):
    """Show each gear's envelope at full throttle."""
    plt = _import_mpl()
    speeds = np.linspace(0.5, 38, 200)

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    fig.suptitle(f"Per-gear characteristics  —  full throttle, "
                 f"cable {cable_out} m out", fontsize=13)

    gear_col = {1: "#F44336", 2: "#FF9800", 3: "#4CAF50"}
    for g in (1, 2, 3):
        vs, ts, rpms = [], [], []
        for v in speeds:
            r = winch.solve(v, 1.0, cable_out, gear=g)
            if r is None:
                continue
            if not (winch.engine.idle_rpm <= r["engine_rpm"]
                    <= winch.engine.max_rpm):
                continue
            vs.append(r["cable_speed_kt"])
            ts.append(r["cable_tension_kgf"])
            rpms.append(r["engine_rpm"])
        if vs:
            ax1.plot(vs, ts, color=gear_col[g], lw=2, label=f"Gear {g}")
            ax2.plot(vs, rpms, color=gear_col[g], lw=2, label=f"Gear {g}")

    for wl, style in [(450, ":"), (750, "--"), (1000, "-.")]:
        ax1.axhline(wl, color="grey", ls=style, alpha=0.5)

    ax1.set_xlabel("Cable speed  (knots)")
    ax1.set_ylabel("Cable tension  (kgf)")
    ax1.set_title("Tension")
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    ax1.set_ylim(bottom=0)

    ax2.set_xlabel("Cable speed  (knots)")
    ax2.set_ylabel("Engine RPM")
    ax2.set_title("Engine RPM")
    ax2.legend()
    ax2.grid(True, alpha=0.3)

    fig.tight_layout()
    fig.savefig(save, dpi=150)
    plt.close(fig)
    print(f"  saved {save}")


# ---------------------------------------------------------------------------
#  Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Skylaunch winch model")
    parser.add_argument("--no-plot", action="store_true",
                        help="skip chart generation")
    args = parser.parse_args()

    winch = GliderWinch(final_drive_ratio=4.0)

    hdr = ("=" * 62 + "\n"
           "  SKYLAUNCH GLIDER WINCH MODEL\n"
           "  GM 7.4 L V8 LPG  /  TH400  /  torque converter\n"
           "=" * 62)
    print(hdr)

    drum = winch.drum
    print(f"\nDrum  inner radius {drum.inner_radius*100:.0f} cm,  "
          f"width {drum.width*100:.0f} cm,  "
          f"cable {drum.cable_diameter*1000:.1f} mm × {drum.cable_length} m")
    print(f"      wraps/layer {drum.wraps_per_layer},  "
          f"radius range {drum.inner_radius*100:.1f} – "
          f"{drum.effective_radius(0)*100:.1f} cm")
    print(f"Final drive ratio  {winch.final_drive_ratio}")

    # --- sample operating points ---
    cases = [
        ( 5, 0.50, 1500, "Slow / half throttle / early launch"),
        (15, 0.75,  800, "Mid speed / ¾ throttle"),
        (25, 1.00,  600, "Cruise / full throttle"),
        (30, 1.00,  400, "Fast / full throttle"),
        (35, 1.00,  200, "Near max / full throttle / late launch"),
    ]
    print("\n--- Sample operating points ---")
    for spd, thr, co, desc in cases:
        print(f"\n  {desc}  ({spd} m/s,  {thr*100:.0f}%,  {co} m out)")
        r = winch.solve(spd, thr, co)
        print_point(r)

    # --- plots ---
    if not args.no_plot:
        print("\nGenerating plots …")
        plot_engine(winch)
        plot_converter(winch)
        plot_drum(winch)
        plot_tension_vs_speed(winch, cable_out=1000)
        plot_cable_out_effect(winch)
        plot_per_gear(winch, cable_out=1000)
        print("Done.")


if __name__ == "__main__":
    main()
