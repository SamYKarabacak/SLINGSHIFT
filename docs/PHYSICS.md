# Physics notes

Distances and times use game units. A spacecraft is a radius-5 disk; levels occupy a 1200 by 540 chart. Bodies are stationary circles with a gravitational parameter `mu`. Rocks have `mu = 0` and remain collision hazards.

## Force and integration

For a body at `q` and spacecraft at `p`, acceleration is

```text
d = q - p
a = mu * d / (dot(d,d) + epsilon^2)^(3/2)
epsilon = 12
```

Accelerations from all bodies add. Softening keeps the field finite at a center and is reflected in the potential used by the orbital energy test. Actual ships collide long before reaching a planet's center.

Velocity Verlet advances at 120 fixed steps per second:

```text
p_next = p + v * dt + 0.5 * a(p) * dt^2
v_next = v + 0.5 * (a(p) + a(p_next)) * dt
```

The renderer uses an accumulator and interpolates between the last two simulated positions. Frame time is capped at 0.1 seconds, so a badly stalled frame slows the game instead of attempting an unbounded catch-up. While aiming a midflight burn, simulated time runs at 12% speed; the physics timestep stays unchanged. Pausing advances neither the accumulator nor the spacecraft.

`predict()` copies a flight and calls the exact same `advance()` function until an event or the 22-second flight limit. Prediction is cached for roughly 35 milliseconds while input changes. A staged burn is applied to the predicted copy. Once a burn is released, its impulse is applied to the real flight. At most one impulse is available, with magnitude capped at 82 distance units per second.

## Events and safeguards

Each step checks the swept position segment against each body expanded by spacecraft radius, the destination gate and the chart boundary. The earliest event wins; hazards take priority on exact ties. This prevents fast ships from jumping through small rocks or a gate. Impact position and elapsed time are interpolated to the first contact.

The swept path is a straight approximation to the small curved Verlet step, rather than an exact curved collision solver. Fixed `dt`, surface collisions, bounded launches and the softened field keep that approximation useful for these levels.

Non-finite positions, velocities, clocks and time steps are contained. Invalid steps, negative flight time and implausible speeds end the flight rather than propagating NaNs into rendering. A zero-length or invalid correction consumes no burn. Terminal flights do not advance.

## What the model leaves out

- Celestial bodies do not orbit, collide with one another or react to the spacecraft.
- A stationary planet bends a path but cannot provide the net energy transfer of a real moving-planet gravity assist.
- There are no physical units, relativity, thrust mass, fuel mass, tidal forces, atmosphere, drag or real docking constraints.
- Destination capture requires position only, at any speed.
- Determinism is tested within a build. Different compilers and math libraries may round differently; bit-identical results across platforms are not promised.

## Verification

Tests check force direction and superposition, softened inverse-square falloff, a circular orbit's closure, energy drift and angular momentum, zero-force motion, collision ordering, tunneling, tangencies, initial overlaps, chart exits, invalid input, timeout, one-burn limits, scoring, deterministic replay and equality between predicted and actual trajectories.

Every level stores a tested three-star launch vector. The test also samples small velocity perturbations to ensure those routes tolerate aiming error. The browser suite separately plays the full campaign through the real input and fixed-step game loop. That second layer caught an input quantization problem on a difficult route and led to the empty-space refinement control.
