#!/usr/bin/env python

'''
Artificial Horizon instrument for flight simulation display.
Compatible with pyglet 2.0+
'''

from math import pi, sin, cos, radians
import pyglet
from pyglet import shapes
from pyglet.gl import glEnable, glDisable, GL_SCISSOR_TEST, glScissor


class ArtificialHorizon:
    """Draws an artificial horizon instrument showing pitch and roll."""

    def __init__(self, x, y, radius=150):
        self.x = x
        self.y = y
        self.radius = radius
        self.pitch = 0.0  # degrees
        self.roll = 0.0   # degrees

    def update(self, pitch, roll):
        """Update pitch and roll angles in degrees."""
        self.pitch = pitch
        self.roll = roll

    def draw(self):
        """Draw the artificial horizon."""
        # Draw outer circle (bezel)
        self._draw_circle_outline(self.x, self.y, self.radius, (76, 76, 76))

        # Enable scissor test to clip horizon to circle
        glEnable(GL_SCISSOR_TEST)
        scissor_size = int(self.radius * 2)
        glScissor(int(self.x - self.radius), int(self.y - self.radius),
                  scissor_size, scissor_size)

        # Calculate roll rotation
        # Bank right (positive roll) -> horizon appears to roll left from pilot's view
        # So LEFT side of horizon goes UP on display
        roll_rad = radians(self.roll)
        cos_roll = cos(roll_rad)
        sin_roll = sin(roll_rad)

        # Calculate pitch offset (pixels per degree)
        # Positive pitch (nose up) should move horizon DOWN (show more sky)
        pitch_scale = 3.0
        pitch_offset = -self.pitch * pitch_scale

        # Draw sky and ground as rotated quads
        self._draw_horizon_background(cos_roll, sin_roll, pitch_offset)

        # Draw horizon line
        self._draw_rotated_line(0, pitch_offset, self.radius * 2,
                                cos_roll, sin_roll, (255, 255, 255))

        # Draw pitch ladder
        for pitch_line in [-20, -10, 10, 20]:
            y_pos = pitch_offset + pitch_line * pitch_scale
            if abs(y_pos) < self.radius * 1.5:
                line_width = 40 if pitch_line % 20 == 0 else 30
                self._draw_rotated_line(0, y_pos, line_width * 2,
                                        cos_roll, sin_roll, (255, 255, 255))

        glDisable(GL_SCISSOR_TEST)

        # Draw fixed reference airplane symbol (not rotated)
        # Center dot
        center_dot = shapes.Circle(self.x, self.y, 3, color=(255, 255, 0))
        center_dot.draw()

        # Wings - left
        left_wing = shapes.Line(self.x - 60, self.y, self.x - 10, self.y,
                                color=(255, 255, 0))
        left_wing.draw()
        left_tip = shapes.Line(self.x - 60, self.y, self.x - 60, self.y - 8,
                               color=(255, 255, 0))
        left_tip.draw()

        # Wings - right
        right_wing = shapes.Line(self.x + 10, self.y, self.x + 60, self.y,
                                 color=(255, 255, 0))
        right_wing.draw()
        right_tip = shapes.Line(self.x + 60, self.y, self.x + 60, self.y - 8,
                                color=(255, 255, 0))
        right_tip.draw()

        # Draw roll indicator at top (rotated with roll)
        self._draw_roll_pointer(cos_roll, sin_roll)

    def _draw_horizon_background(self, cos_roll, sin_roll, pitch_offset):
        """Draw the sky and ground backgrounds rotated by roll angle."""
        size = self.radius * 3

        # Sky quad vertices (above horizon)
        sky_vertices = [
            (-size, pitch_offset),
            (size, pitch_offset),
            (size, size),
            (-size, size)
        ]

        # Ground quad vertices (below horizon)
        ground_vertices = [
            (-size, -size),
            (size, -size),
            (size, pitch_offset),
            (-size, pitch_offset)
        ]

        # Rotate and translate vertices, then draw
        sky_rotated = [self._rotate_point(x, y, cos_roll, sin_roll)
                       for x, y in sky_vertices]
        ground_rotated = [self._rotate_point(x, y, cos_roll, sin_roll)
                          for x, y in ground_vertices]

        # Draw sky (blue)
        self._draw_quad(sky_rotated, (51, 102, 204))

        # Draw ground (brown)
        self._draw_quad(ground_rotated, (102, 64, 25))

    def _rotate_point(self, x, y, cos_a, sin_a):
        """Rotate a point around the origin."""
        return (x * cos_a - y * sin_a + self.x,
                x * sin_a + y * cos_a + self.y)

    def _draw_quad(self, vertices, color):
        """Draw a filled quadrilateral using two triangles."""
        # Triangle 1: vertices 0, 1, 2
        tri1 = shapes.Triangle(
            vertices[0][0], vertices[0][1],
            vertices[1][0], vertices[1][1],
            vertices[2][0], vertices[2][1],
            color=color
        )
        tri1.draw()

        # Triangle 2: vertices 0, 2, 3
        tri2 = shapes.Triangle(
            vertices[0][0], vertices[0][1],
            vertices[2][0], vertices[2][1],
            vertices[3][0], vertices[3][1],
            color=color
        )
        tri2.draw()

    def _draw_rotated_line(self, x, y, length, cos_roll, sin_roll, color):
        """Draw a horizontal line rotated by roll angle."""
        half_len = length / 2
        x1, y1 = self._rotate_point(-half_len, y, cos_roll, sin_roll)
        x2, y2 = self._rotate_point(half_len, y, cos_roll, sin_roll)

        line = shapes.Line(x1, y1, x2, y2, color=color)
        line.draw()

    def _draw_circle_outline(self, x, y, radius, color):
        """Draw an unfilled circle using line segments."""
        segments = 50
        for i in range(segments):
            angle1 = 2 * pi * i / segments
            angle2 = 2 * pi * (i + 1) / segments
            x1 = x + radius * cos(angle1)
            y1 = y + radius * sin(angle1)
            x2 = x + radius * cos(angle2)
            y2 = y + radius * sin(angle2)
            line = shapes.Line(x1, y1, x2, y2, color=color)
            line.draw()

    def _draw_roll_pointer(self, cos_roll, sin_roll):
        """Draw the roll indicator triangle at the top."""
        # Triangle vertices relative to center
        v1 = (0, self.radius - 5)
        v2 = (-5, self.radius - 15)
        v3 = (5, self.radius - 15)

        # Rotate vertices
        v1_rot = self._rotate_point(*v1, cos_roll, sin_roll)
        v2_rot = self._rotate_point(*v2, cos_roll, sin_roll)
        v3_rot = self._rotate_point(*v3, cos_roll, sin_roll)

        triangle = shapes.Triangle(
            v1_rot[0], v1_rot[1],
            v2_rot[0], v2_rot[1],
            v3_rot[0], v3_rot[1],
            color=(255, 255, 0)
        )
        triangle.draw()


class AOAIndicator:
    """Angle of Attack indicator gauge."""

    def __init__(self, x, y, width=30, height=150):
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.aoa = 0.0  # degrees
        self.min_aoa = -10.0
        self.max_aoa = 20.0
        self.critical_aoa = 15.0  # stall warning

    def update(self, aoa):
        """Update angle of attack in degrees."""
        self.aoa = aoa

    def draw(self):
        """Draw the AOA indicator."""
        # Background
        bg = shapes.Rectangle(
            self.x - self.width / 2, self.y - self.height / 2,
            self.width, self.height, color=(40, 40, 40)
        )
        bg.draw()

        # Draw scale markings
        for aoa_mark in range(-10, 21, 5):
            y_pos = self._aoa_to_y(aoa_mark)
            # Tick mark
            line = shapes.Line(
                self.x - self.width / 2, y_pos,
                self.x - self.width / 2 + 8, y_pos,
                color=(255, 255, 255)
            )
            line.draw()

        # Critical AOA zone (red)
        critical_y = self._aoa_to_y(self.critical_aoa)
        top_y = self._aoa_to_y(self.max_aoa)
        if critical_y < top_y:
            danger_zone = shapes.Rectangle(
                self.x - self.width / 2 + 2, critical_y,
                self.width - 4, top_y - critical_y,
                color=(180, 0, 0)
            )
            danger_zone.draw()

        # Current AOA pointer
        pointer_y = self._aoa_to_y(self.aoa)
        pointer_y = max(self.y - self.height / 2, min(self.y + self.height / 2, pointer_y))

        # Pointer color based on AOA
        if self.aoa >= self.critical_aoa:
            color = (255, 0, 0)
        elif self.aoa >= self.critical_aoa - 3:
            color = (255, 255, 0)
        else:
            color = (0, 255, 0)

        pointer = shapes.Triangle(
            self.x + self.width / 2, pointer_y,
            self.x + self.width / 2 + 12, pointer_y - 6,
            self.x + self.width / 2 + 12, pointer_y + 6,
            color=color
        )
        pointer.draw()

        # Border
        self._draw_rect_outline(
            self.x - self.width / 2, self.y - self.height / 2,
            self.width, self.height, (100, 100, 100)
        )

    def _aoa_to_y(self, aoa):
        """Convert AOA value to Y position."""
        range_aoa = self.max_aoa - self.min_aoa
        normalized = (aoa - self.min_aoa) / range_aoa
        return self.y - self.height / 2 + normalized * self.height

    def _draw_rect_outline(self, x, y, width, height, color):
        """Draw rectangle outline."""
        lines = [
            shapes.Line(x, y, x + width, y, color=color),
            shapes.Line(x + width, y, x + width, y + height, color=color),
            shapes.Line(x + width, y + height, x, y + height, color=color),
            shapes.Line(x, y + height, x, y, color=color),
        ]
        for line in lines:
            line.draw()


class AirspeedIndicator:
    """Airspeed Indicator (ASI) - circular dial."""

    def __init__(self, x, y, radius=80):
        self.x = x
        self.y = y
        self.radius = radius
        self.airspeed = 0.0  # m/s
        self.min_speed = 0.0
        self.max_speed = 60.0  # m/s
        self.vne = 55.0  # never exceed
        self.vs0 = 18.0  # stall speed

    def update(self, airspeed):
        """Update airspeed in m/s."""
        self.airspeed = airspeed

    def draw(self):
        """Draw the airspeed indicator."""
        # Background circle
        bg = shapes.Circle(self.x, self.y, self.radius, color=(30, 30, 30))
        bg.draw()

        # Draw speed arc zones
        self._draw_arc_zone(self.vs0, self.max_speed, (0, 200, 0))  # Green - normal
        self._draw_arc_zone(self.vne, self.max_speed, (200, 0, 0))  # Red - VNE

        # Draw scale markings
        for speed in range(0, int(self.max_speed) + 1, 10):
            angle = self._speed_to_angle(speed)
            cos_a = cos(angle)
            sin_a = sin(angle)

            # Tick marks
            inner_r = self.radius - 15
            outer_r = self.radius - 5
            line = shapes.Line(
                self.x + inner_r * cos_a, self.y + inner_r * sin_a,
                self.x + outer_r * cos_a, self.y + outer_r * sin_a,
                color=(255, 255, 255)
            )
            line.draw()

        # Draw needle
        angle = self._speed_to_angle(min(self.airspeed, self.max_speed))
        needle_len = self.radius - 20
        cos_a = cos(angle)
        sin_a = sin(angle)

        needle = shapes.Line(
            self.x, self.y,
            self.x + needle_len * cos_a, self.y + needle_len * sin_a,
            color=(255, 255, 255)
        )
        needle.draw()

        # Center cap
        cap = shapes.Circle(self.x, self.y, 8, color=(80, 80, 80))
        cap.draw()

        # Outer ring
        self._draw_circle_outline(self.x, self.y, self.radius, (100, 100, 100))

    def _speed_to_angle(self, speed):
        """Convert speed to angle (radians). 0 speed at bottom-left."""
        normalized = speed / self.max_speed
        # Arc from 225° to -45° (270° sweep)
        start_angle = radians(225)
        sweep = radians(270)
        return start_angle - normalized * sweep

    def _draw_arc_zone(self, start_speed, end_speed, color):
        """Draw a colored arc zone."""
        segments = 20
        start_angle = self._speed_to_angle(start_speed)
        end_angle = self._speed_to_angle(end_speed)
        arc_r = self.radius - 10

        angle_step = (end_angle - start_angle) / segments
        for i in range(segments):
            a1 = start_angle + i * angle_step
            a2 = start_angle + (i + 1) * angle_step
            line = shapes.Line(
                self.x + arc_r * cos(a1), self.y + arc_r * sin(a1),
                self.x + arc_r * cos(a2), self.y + arc_r * sin(a2),
                color=color
            )
            line.draw()

    def _draw_circle_outline(self, x, y, radius, color):
        """Draw an unfilled circle using line segments."""
        segments = 50
        for i in range(segments):
            angle1 = 2 * pi * i / segments
            angle2 = 2 * pi * (i + 1) / segments
            x1 = x + radius * cos(angle1)
            y1 = y + radius * sin(angle1)
            x2 = x + radius * cos(angle2)
            y2 = y + radius * sin(angle2)
            line = shapes.Line(x1, y1, x2, y2, color=color)
            line.draw()


class VerticalSpeedIndicator:
    """Vertical Speed Indicator (VSI) / Variometer."""

    def __init__(self, x, y, radius=80):
        self.x = x
        self.y = y
        self.radius = radius
        self.vspeed = 0.0  # m/s (positive = climb)
        self.max_vspeed = 10.0  # m/s

    def update(self, vspeed):
        """Update vertical speed in m/s (positive = climb, negative = descent)."""
        self.vspeed = vspeed

    def draw(self):
        """Draw the VSI."""
        # Background circle
        bg = shapes.Circle(self.x, self.y, self.radius, color=(30, 30, 30))
        bg.draw()

        # Draw scale markings (-10 to +10 m/s)
        for vs in range(-10, 11, 2):
            angle = self._vspeed_to_angle(vs)
            cos_a = cos(angle)
            sin_a = sin(angle)

            # Tick marks
            inner_r = self.radius - 15
            outer_r = self.radius - 5
            line = shapes.Line(
                self.x + inner_r * cos_a, self.y + inner_r * sin_a,
                self.x + outer_r * cos_a, self.y + outer_r * sin_a,
                color=(255, 255, 255)
            )
            line.draw()

        # Zero line (horizontal)
        zero_line = shapes.Line(
            self.x - self.radius + 20, self.y,
            self.x + self.radius - 20, self.y,
            color=(100, 100, 100)
        )
        zero_line.draw()

        # Draw needle
        clamped_vs = max(-self.max_vspeed, min(self.max_vspeed, self.vspeed))
        angle = self._vspeed_to_angle(clamped_vs)
        needle_len = self.radius - 20
        cos_a = cos(angle)
        sin_a = sin(angle)

        # Color based on climb/descent
        if self.vspeed > 1:
            color = (0, 255, 0)  # Green for climb
        elif self.vspeed < -1:
            color = (255, 100, 100)  # Red-ish for descent
        else:
            color = (255, 255, 255)  # White for level

        needle = shapes.Line(
            self.x, self.y,
            self.x + needle_len * cos_a, self.y + needle_len * sin_a,
            color=color
        )
        needle.draw()

        # Center cap
        cap = shapes.Circle(self.x, self.y, 8, color=(80, 80, 80))
        cap.draw()

        # Outer ring
        self._draw_circle_outline(self.x, self.y, self.radius, (100, 100, 100))

    def _vspeed_to_angle(self, vspeed):
        """Convert vertical speed to angle. 0 is horizontal (right), positive is up."""
        # Map -10 to +10 m/s to angle range
        # 0 m/s = 0° (pointing right), +10 = +90° (up), -10 = -90° (down)
        normalized = vspeed / self.max_vspeed
        return normalized * (pi / 2)  # ±90 degrees

    def _draw_circle_outline(self, x, y, radius, color):
        """Draw an unfilled circle using line segments."""
        segments = 50
        for i in range(segments):
            angle1 = 2 * pi * i / segments
            angle2 = 2 * pi * (i + 1) / segments
            x1 = x + radius * cos(angle1)
            y1 = y + radius * sin(angle1)
            x2 = x + radius * cos(angle2)
            y2 = y + radius * sin(angle2)
            line = shapes.Line(x1, y1, x2, y2, color=color)
            line.draw()


class SideslipIndicator:
    """Sideslip indicator (slip/skid ball)."""

    def __init__(self, x, y, width=120, height=30):
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.sideslip = 0.0  # degrees
        self.max_slip = 15.0  # degrees for full deflection

    def update(self, sideslip):
        """Update sideslip angle in degrees."""
        self.sideslip = sideslip

    def draw(self):
        """Draw the sideslip indicator."""
        # Tube background
        tube = shapes.Rectangle(
            self.x - self.width / 2, self.y - self.height / 2,
            self.width, self.height, color=(20, 20, 20)
        )
        tube.draw()

        # Center reference marks
        mark_width = 3
        mark_offset = 15
        left_mark = shapes.Rectangle(
            self.x - mark_offset - mark_width / 2, self.y - self.height / 2,
            mark_width, self.height, color=(255, 255, 0)
        )
        left_mark.draw()

        right_mark = shapes.Rectangle(
            self.x + mark_offset - mark_width / 2, self.y - self.height / 2,
            mark_width, self.height, color=(255, 255, 0)
        )
        right_mark.draw()

        # Ball position based on sideslip
        normalized = self.sideslip / self.max_slip
        normalized = max(-1, min(1, normalized))
        ball_x = self.x + normalized * (self.width / 2 - 15)
        ball_radius = self.height / 2 - 3

        # Ball
        ball = shapes.Circle(ball_x, self.y, ball_radius, color=(40, 40, 40))
        ball.draw()

        # Ball highlight
        highlight = shapes.Circle(
            ball_x - ball_radius / 3, self.y + ball_radius / 3,
            ball_radius / 3, color=(80, 80, 80)
        )
        highlight.draw()

        # Tube outline
        self._draw_rect_outline(
            self.x - self.width / 2, self.y - self.height / 2,
            self.width, self.height, (100, 100, 100)
        )

    def _draw_rect_outline(self, x, y, width, height, color):
        """Draw rectangle outline."""
        lines = [
            shapes.Line(x, y, x + width, y, color=color),
            shapes.Line(x + width, y, x + width, y + height, color=color),
            shapes.Line(x + width, y + height, x, y + height, color=color),
            shapes.Line(x, y + height, x, y, color=color),
        ]
        for line in lines:
            line.draw()


class ControlPositionIndicator:
    """Control position indicator showing aileron, elevator, rudder and spoiler."""

    def __init__(self, x, y, size=120):
        self.x = x
        self.y = y
        self.size = size
        self.aileron = 0.0   # -1 to 1 (left to right)
        self.elevator = 0.0  # -1 to 1 (down to up)
        self.rudder = 0.0    # -1 to 1 (left to right)
        self.spoiler = 0.0   # 0 to 1 (retracted to extended)
        self.bar_width = 20
        self.bar_gap = 15

    def update(self, aileron, elevator, rudder, spoiler):
        """Update control positions. All values -1 to 1 except spoiler 0 to 1."""
        self.aileron = max(-1, min(1, aileron))
        self.elevator = max(-1, min(1, elevator))
        self.rudder = max(-1, min(1, rudder))
        self.spoiler = max(0, min(1, spoiler))

    def draw(self):
        """Draw the control position indicator."""
        half_size = self.size / 2

        # Main box background (aileron/elevator)
        bg = shapes.Rectangle(
            self.x - half_size, self.y - half_size,
            self.size, self.size, color=(30, 30, 30)
        )
        bg.draw()

        # Center crosshair lines
        h_line = shapes.Line(
            self.x - half_size, self.y,
            self.x + half_size, self.y,
            color=(60, 60, 60)
        )
        h_line.draw()

        v_line = shapes.Line(
            self.x, self.y - half_size,
            self.x, self.y + half_size,
            color=(60, 60, 60)
        )
        v_line.draw()

        # Control position dot
        # Aileron: right stick = right on display
        # Elevator: pull back (nose up) = down on display (matches stick position)
        dot_x = self.x + self.aileron * (half_size - 10)
        dot_y = self.y - self.elevator * (half_size - 10)
        dot = shapes.Circle(dot_x, dot_y, 8, color=(0, 255, 0))
        dot.draw()

        # Main box outline
        self._draw_rect_outline(
            self.x - half_size, self.y - half_size,
            self.size, self.size, (100, 100, 100)
        )

        # Rudder bar (horizontal, underneath center box)
        rudder_y = self.y - half_size - self.bar_gap - self.bar_width
        self._draw_horizontal_bar(
            self.x, rudder_y, self.size, self.bar_width,
            self.rudder, color=(100, 150, 255)
        )

        # Spoiler bar (right side)
        spoiler_x = self.x + half_size + self.bar_gap
        self._draw_vertical_bar(
            spoiler_x, self.y, self.bar_width, self.size,
            self.spoiler, centered=False, color=(255, 150, 100)
        )

    def _draw_vertical_bar(self, x, y, width, height, value, centered, color):
        """Draw a vertical bar indicator."""
        half_height = height / 2

        # Background
        bg = shapes.Rectangle(
            x, y - half_height, width, height, color=(30, 30, 30)
        )
        bg.draw()

        # Value indicator
        if centered:
            # Centered bar (for rudder): value -1 to 1, bar extends from center
            bar_height = abs(value) * half_height
            if value >= 0:
                bar_y = y
            else:
                bar_y = y - bar_height
            indicator = shapes.Rectangle(
                x + 2, bar_y, width - 4, bar_height, color=color
            )
            indicator.draw()

            # Center line
            center_line = shapes.Line(
                x, y, x + width, y, color=(100, 100, 100)
            )
            center_line.draw()
        else:
            # Bottom-up bar (for spoiler): value 0 to 1
            bar_height = value * height
            indicator = shapes.Rectangle(
                x + 2, y - half_height, width - 4, bar_height, color=color
            )
            indicator.draw()

        # Outline
        self._draw_rect_outline(x, y - half_height, width, height, (100, 100, 100))

    def _draw_horizontal_bar(self, x, y, width, height, value, color):
        """Draw a horizontal bar indicator (centered, for rudder)."""
        half_width = width / 2

        # Background
        bg = shapes.Rectangle(
            x - half_width, y, width, height, color=(30, 30, 30)
        )
        bg.draw()

        # Value indicator: value -1 to 1, bar extends from center
        bar_width = abs(value) * half_width
        if value >= 0:
            bar_x = x
        else:
            bar_x = x - bar_width
        indicator = shapes.Rectangle(
            bar_x, y + 2, bar_width, height - 4, color=color
        )
        indicator.draw()

        # Center line
        center_line = shapes.Line(
            x, y, x, y + height, color=(100, 100, 100)
        )
        center_line.draw()

        # Outline
        self._draw_rect_outline(x - half_width, y, width, height, (100, 100, 100))

    def _draw_rect_outline(self, x, y, width, height, color):
        """Draw rectangle outline."""
        lines = [
            shapes.Line(x, y, x + width, y, color=color),
            shapes.Line(x + width, y, x + width, y + height, color=color),
            shapes.Line(x + width, y + height, x, y + height, color=color),
            shapes.Line(x, y + height, x, y, color=color),
        ]
        for line in lines:
            line.draw()
