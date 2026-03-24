# ACO-Multiway: Ant Colony Optimization for Dynamic Pickup and Delivery with LIFO Constraints.
We present an ant colony optimization framework for dynamic pickup and delivery problems (DPDPs) under capacity, time‑window, dock, and last‑in‑first‑out (LIFO) loading constraints. A distinctive feature of this work is the encoding of LIFO‑compliant routes as multiway trees. This representation shifts the constraint‑handling burden from post‑hoc feasibility checking to the construction process itself, allowing the ant colony to explore the solution space without generating infeasible candidates.

Core Idea
We model each vehicle’s route as a rooted tree. A node corresponds to a pickup operation, and its children represent the deliveries that must be completed before the pickup’s own delivery can occur. A post‑order traversal of the tree yields a linear route that inherently respects LIFO ordering. This encoding eliminates the need for separate constraint‑repair mechanisms during solution construction.

The algorithm operates in a rolling‑horizon fashion. At each decision interval, we:

- allocate orders to vehicles using a pheromone‑guided roulette selection,

- insert each order into the vehicle’s multiway tree at a position selected by a probability distribution that combines distance‑based heuristics with pheromone trails,

- extract the final routes via tree traversal,

- refine the solution using four local search operators (couple/block exchange and couple/block relocate) within a tabu search framework,

- update pheromone matrices at both the order‑assignment and node‑transition levels, reinforcing high‑quality routes.
