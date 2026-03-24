# An-ant-colony-optimization-algorithm-for-dynamic-pickup-and-delivery-problems-with-LIFO-constraints
This repository implements an ant colony optimization framework for DPDP with LIFO constraints. I encode feasible routes as binary trees, enabling constraint satisfaction during construction. A dual‑level pheromone structure guides order allocation and node transitions. Four local search operators embedded in tabu search refine solutions. 
# ACO-Multiway: An Ant Colony Optimization Framework for Dynamic Pickup and Delivery with LIFO Constraints

## Algorithm Description

We address dynamic pickup and delivery problems (DPDPs) where orders arrive in real time and vehicles must operate under capacity limits, time windows, dock availability, and last‑in‑first‑out (LIFO) loading. The LIFO constraint is particularly restrictive: the most recently loaded goods must be the first unloaded, which imposes a nested structure on each vehicle’s route. Conventional algorithms treat LIFO as a feasibility condition to be checked after constructing a route, leading to wasted effort on infeasible solutions and a restricted search space. We instead embed the constraint directly into the solution representation.

**Multiway Tree Encoding.** We represent a vehicle’s route as a rooted tree. Each node corresponds to a pickup operation. Its children are the deliveries that must be completed before the pickup’s own delivery can take place. This structure precisely captures the nesting relationships demanded by LIFO loading. To obtain a linear route from the tree, we perform a post‑order traversal of its equivalent binary representation. The resulting node sequence automatically satisfies LIFO, eliminating the need for separate feasibility checks during construction.

**Ant Colony Construction.** The algorithm maintains two layers of pheromone matrices. The first layer, of dimension `(vehicle count) × (order count)`, guides the assignment of orders to vehicles. The second layer, of dimension `(node count) × (node count)`, directs transitions between nodes within a vehicle’s route. Each ant constructs a solution as follows:

1. **Order allocation.** The ant sequentially assigns each order to a vehicle using a roulette‑wheel selection that combines pheromone intensity and a heuristic based on the distance between the vehicle’s current location and the order’s pickup point.
2. **Tree insertion.** For each allocated order, the ant selects an insertion position within the vehicle’s existing multiway tree. The decision respects both capacity constraints and the nested LIFO structure. A probability distribution over candidate positions is computed from the product of heuristic information (distance‑based) and pheromone values from the second matrix.
3. **Route extraction.** After inserting all orders, the ant performs a post‑order traversal of each vehicle’s tree to generate a concrete route.

**Local Search with Tabu.** Once a population of ants completes construction, we apply four local search operators, each designed to preserve LIFO feasibility:
- *Couple exchange*: swaps two complete order pairs either within the same vehicle or across different vehicles.
- *Block exchange*: swaps two contiguous sequences of nodes that form feasible blocks.
- *Couple relocate*: moves an order pair to a different position.
- *Block relocate*: moves a block of nodes to a new location.

These operators are executed within a tabu search framework. The tabu list records recently applied moves to prevent cycling. When a move leads to a solution with lower cost, we update the global best and reinforce the corresponding pheromone trails.

**Pheromone Update.** We employ two‑stage evaporation and reinforcement. After each iteration, we apply a local update that slightly decreases pheromone on the edges used by the constructing ants, promoting exploration. At the end of the iteration, the global best solution receives a stronger reinforcement: the pheromone values on its edges are increased proportionally to the inverse of its total cost. This dual mechanism balances diversification and intensification throughout the search.

**Experimental Validation.** We evaluate the algorithm on the HW benchmark suite from the ICAPS 2021 DPDP competition. The benchmark contains 64 instances with varying numbers of vehicles (5 to 100) and orders (50 to 4000). For small‑scale instances (5 vehicles, 50–300 orders), we compare against the state‑of‑the‑art MOEAD-TS method (Cai et al., 2024). Results over 20 independent runs show that our approach achieves consistently lower mean total cost and smaller standard deviations. For instance, on HW1, the mean cost is 116.7 (std 3.48) compared to 117.5 for MOEAD-TS. The improvement stems from the multiway‑tree encoding, which focuses the search on feasible regions and reduces wasted computational effort on infeasible intermediates.

**Implementation.** The code is written in C++17 and organized into several modules. `lifo_tree.h` implements the multiway‑tree and binary‑tree data structures with insertion and traversal operations. `node.h`, `oderitem.h`, and `veichle.h` define the core problem entities. The main algorithm resides in `aco_pdp.cpp`, which orchestrates the ant colony construction, local search, and pheromone updates. Input data follows the format specified by the Huawei benchmark, and output solutions are written to CSV files for analysis.

**Future Directions.** While the current implementation demonstrates the promise of structure‑aware encoding, several extensions are worth exploring. We plan to integrate the tree‑based representation with deep reinforcement learning to learn insertion policies from historical data. Adaptive parameter control could adjust pheromone evaporation rates based on search progress. Parallelization of the ant colony across multiple cores would allow handling larger instances within the same time budget. Finally, extending the framework to explicitly optimize multiple objectives—such as tardiness and travel distance—remains an open and promising direction.
