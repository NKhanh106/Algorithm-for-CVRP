

import os
import sys
import math
import random
import copy
import time
from collections import namedtuple

# ------------------------- Configurable parameters -------------------------
PARAMS = {
    'num_ants': 20,
    'num_iterations': 200,
    'alpha': 1.0,
    'beta': 3.0,
    'rho': 0.1,                # evaporation
    'Q': 1.0,                  # pheromone factor
    'pheromone_init': 1.0,
    'candidate_list_size': 15, # nearest neighbors
    'no_improve_restart': 80,  # restart pheromones when stuck
    'elitist_weight': 1.0      # extra deposit from global best
}

# ------------------------- IO: load CVRP (TSPLIB-like) -------------------------

def load_cvrp(path):
    coords = {}
    demand = {}
    depot = None
    capacity = None

    section = None
    with open(path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            if line.startswith('CAPACITY'):
                # supports both 'CAPACITY : 100' and 'CAPACITY: 100'
                parts = line.split(':')
                capacity = int(parts[1])
                continue
            if line.startswith('NODE_COORD_SECTION'):
                section = 'coord'
                continue
            if line.startswith('DEMAND_SECTION'):
                section = 'demand'
                continue
            if line.startswith('DEPOT_SECTION'):
                section = 'depot'
                continue
            if line.startswith('EOF'):
                break

            if section == 'coord':
                parts = line.split()
                idx = int(parts[0])
                coords[idx] = (float(parts[1]), float(parts[2]))

            elif section == 'demand':
                parts = line.split()
                demand[int(parts[0])] = int(parts[1])

            elif section == 'depot':
                val = int(line)
                if val == -1:
                    continue
                depot = val

    return coords, demand, depot, capacity

# ------------------------- Utilities -------------------------

def euclidean(a, b):
    return math.hypot(a[0]-b[0], a[1]-b[1])

# ------------------------- Local search operators -------------------------

def two_opt_route(route, dist):
    # route is [depot, a, b, ..., depot]
    best = route[:]
    improved = True
    while improved:
        improved = False
        n = len(best)
        for i in range(1, n-3):
            for j in range(i+1, n-1):
                if j - i == 1:
                    continue
                new = best[:i] + best[i:j][::-1] + best[j:]
                if route_length(new, dist) < route_length(best, dist):
                    best = new
                    improved = True
        route = best
    return best


def route_length(route, dist):
    s = 0.0
    for i in range(len(route)-1):
        s += dist[route[i]][route[i+1]]
    return s


def intra_route_ls(routes, dist):
    return [two_opt_route(r, dist) if len(r) > 3 else r for r in routes]


def relocate(routes, demand, CAPACITY, dist):
    # greedy relocate: try moving a customer from route i to j improving total length
    best = copy.deepcopy(routes)
    improved = True
    while improved:
        improved = False
        for i in range(len(best)):
            for a in range(1, len(best[i]) - 1):
                node = best[i][a]
                load_i = sum(demand[n] for n in best[i] if n != -1)
                for j in range(len(best)):
                    if i == j:
                        continue
                    load_j = sum(demand[n] for n in best[j] if n != -1)
                    if load_j + demand[node] > CAPACITY:
                        continue
                    # try insert at every position in route j
                    for b in range(1, len(best[j])):
                        new_routes = copy.deepcopy(best)
                        new_routes[i] = best[i][:a] + best[i][a+1:]
                        new_routes[j] = best[j][:b] + [node] + best[j][b:]
                        # remove empty route if necessary
                        if len(new_routes[i]) == 2:
                            new_routes.pop(i)
                        if total_length(new_routes, dist) < total_length(best, dist):
                            best = new_routes
                            improved = True
                            break
                    if improved:
                        break
                if improved:
                    break
            if improved:
                break
    return best


def swap(routes, demand, CAPACITY, dist):
    # greedy 1-1 swap between routes
    best = copy.deepcopy(routes)
    improved = True
    while improved:
        improved = False
        for i in range(len(best)):
            for j in range(i+1, len(best)):
                for a in range(1, len(best[i]) - 1):
                    for b in range(1, len(best[j]) - 1):
                        node_i = best[i][a]
                        node_j = best[j][b]
                        load_i = sum(demand[n] for n in best[i] if n != -1) - demand[node_i] + demand[node_j]
                        load_j = sum(demand[n] for n in best[j] if n != -1) - demand[node_j] + demand[node_i]
                        if load_i > CAPACITY or load_j > CAPACITY:
                            continue
                        new_routes = copy.deepcopy(best)
                        new_routes[i] = best[i][:a] + [node_j] + best[i][a+1:]
                        new_routes[j] = best[j][:b] + [node_i] + best[j][b+1:]
                        if total_length(new_routes, dist) < total_length(best, dist):
                            best = new_routes
                            improved = True
                            break
                    if improved:
                        break
                if improved:
                    break
            if improved:
                break
    return best

# ------------------------- ACO core -------------------------
AntResult = namedtuple('AntResult', ['routes', 'length'])


def build_candidate_list(dist, CL_size):
    N = len(dist) - 1
    CL = {i: [] for i in range(1, N+1)}
    for i in range(1, N+1):
        neighbors = sorted(range(1, N+1), key=lambda x: dist[i][x] if x != i else 1e12)
        CL[i] = [n for n in neighbors if n != i][:CL_size]
    return CL


def construct_solution(pheromone, eta, dist, customers, demand, CAPACITY, alpha, beta, CL):
    unvisited = set(customers)
    routes = []
    while unvisited:
        route = [DEPOT]
        load = 0
        current = DEPOT
        while True:
            feasible = [c for c in unvisited if demand[c] + load <= CAPACITY]
            if not feasible:
                break
            # use candidate list first
            cand = [c for c in CL[current] if c in feasible]
            candidates = cand if cand else feasible
            weights = []
            for j in candidates:
                tau = pheromone[current][j] ** alpha
                hij = eta[current][j] ** beta
                weights.append(tau * hij)
            total = sum(weights)
            if total == 0:
                chosen = random.choice(candidates)
            else:
                r = random.random() * total
                cum = 0.0
                chosen = candidates[-1]
                for idx, w in enumerate(weights):
                    cum += w
                    if r <= cum:
                        chosen = candidates[idx]
                        break
            route.append(chosen)
            load += demand[chosen]
            unvisited.remove(chosen)
            current = chosen
        route.append(DEPOT)
        routes.append(route)
    length = total_length(routes, dist)
    return AntResult(routes=routes, length=length)


def total_length(routes, dist):
    s = 0.0
    for r in routes:
        for i in range(len(r)-1):
            s += dist[r[i]][r[i+1]]
    return s


def update_pheromone_mmas(pheromone, ants, rho, Q, tau_min, tau_max, global_best):
    N = len(pheromone) - 1
    # evaporation
    for i in range(1, N+1):
        for j in range(1, N+1):
            pheromone[i][j] *= (1.0 - rho)
            if pheromone[i][j] < tau_min:
                pheromone[i][j] = tau_min
    # deposit: iteration-best
    iter_best = min(ants, key=lambda a: a.length)
    delta = Q / iter_best.length
    for r in iter_best.routes:
        for i in range(len(r)-1):
            a = r[i]
            b = r[i+1]
            pheromone[a][b] += delta
            pheromone[b][a] += delta
            if pheromone[a][b] > tau_max:
                pheromone[a][b] = tau_max
            if pheromone[b][a] > tau_max:
                pheromone[b][a] = tau_max
    # elitist deposit from global best (if exists)
    if global_best is not None:
        delta_g = Q * PARAMS['elitist_weight'] / global_best.length
        for r in global_best.routes:
            for i in range(len(r)-1):
                a = r[i]
                b = r[i+1]
                pheromone[a][b] += delta_g
                pheromone[b][a] += delta_g
                if pheromone[a][b] > tau_max:
                    pheromone[a][b] = tau_max
                if pheromone[b][a] > tau_max:
                    pheromone[b][a] = tau_max

# ------------------------- Main ACO loop -------------------------

def run_aco_instance(coords, demand, DEPOT, CAPACITY, params):
    # prepare instance-specific data
    N = len(coords)
    customers = [i for i in range(1, N+1) if i != DEPOT]
    # build dist
    dist = [[0.0]*(N+1) for _ in range(N+1)]
    for i in range(1, N+1):
        for j in range(1, N+1):
            dist[i][j] = euclidean(coords[i], coords[j])
    # candidate list
    CL = build_candidate_list(dist, params['candidate_list_size'])
    # initialize pheromone and heuristic
    pheromone = [[params['pheromone_init']]*(N+1) for _ in range(N+1)]
    eta = [[0.0]*(N+1) for _ in range(N+1)]
    for i in range(1, N+1):
        for j in range(1, N+1):
            if i != j:
                eta[i][j] = 1.0 / (dist[i][j] + 1e-6)
    # MMAS bounds
    tau_max = params['pheromone_init']
    tau_min = 1e-4

    best_global = None
    best_global_iter = 0
    no_improve = 0

    start = time.time()
    for it in range(1, params['num_iterations']+1):
        ants = []
        for k in range(params['num_ants']):
            ant = construct_solution(pheromone, eta, dist, customers, demand, CAPACITY, params['alpha'], params['beta'], CL)
            # local search: intra-route 2-opt
            ls_routes = intra_route_ls(ant.routes, dist)
            # inter-route LS: relocate then swap
            ls_routes = relocate(ls_routes, demand, CAPACITY, dist)
            ls_routes = swap(ls_routes, demand, CAPACITY, dist)
            valid, msg = validate_solution(ls_routes, demand, CAPACITY, DEPOT)
            if not valid:
                print(f"[WARNING] Invalid solution at iteration {it}, ant {k}: {msg}")
                print("Routes:", ls_routes)
    # Có thể skip nghiệm, hoặc sửa nhẹ
                continue
            
            ant = AntResult(routes=ls_routes, length=total_length(ls_routes, dist))
            ants.append(ant)

        # update pheromone with MMAS-like scheme
        update_pheromone_mmas(pheromone, ants, params['rho'], params['Q'], tau_min, tau_max, best_global)

        # track bests
        iter_best = min(ants, key=lambda a: a.length)
        if best_global is None or iter_best.length < best_global.length:
            best_global = copy.deepcopy(iter_best)
            best_global_iter = it
            no_improve = 0
        else:
            no_improve += 1

        # update tau_max/tau_min adaptively (optional simple schedule)
        tau_max = max(tau_max, params['pheromone_init'])
        tau_min = tau_max * 0.001

        if it % 50 == 0:
            print(f"Iter {it}/{params['num_iterations']} - iter_best: {iter_best.length:.3f} - best_global: {best_global.length:.3f}")

        # restart pheromone if stuck
        if no_improve >= params['no_improve_restart']:
            pheromone = [[params['pheromone_init']]*(N+1) for _ in range(N+1)]
            no_improve = 0
            print(f"Pheromone restarted at iter {it}")

    elapsed = time.time() - start
    print(f"Finished {params['num_iterations']} iters in {elapsed:.2f}s. Best: {best_global.length:.3f} at iter {best_global_iter}")
    return best_global, best_global_iter
def validate_solution(routes, demand, CAPACITY, DEPOT):
    """
    Validate a CVRP solution.
    Checks:
      - Each route begins/ends with DEPOT
      - No depot appears in the middle of a route
      - Capacity constraint satisfied
      - No duplicated customers
      - All customers visited exactly once
    """

    seen = set()
    all_customers = set(demand.keys()) - {DEPOT}

    # --- Check each route ---
    for idx, r in enumerate(routes, 1):

        # 1. Route must start and end at depot
        if len(r) < 2 or r[0] != DEPOT or r[-1] != DEPOT:
            return False, f"Route {idx} does not start/end at depot: {r}"

        # 2. No depot allowed inside the route
        for n in r[1:-1]:
            if n == DEPOT:
                return False, f"Route {idx} contains depot inside: {r}"

        # 3. Capacity check
        load = sum(demand[n] for n in r[1:-1])
        if load > CAPACITY:
            return False, f"Route {idx} exceeds capacity ({load} > {CAPACITY}): {r}"

        # 4. Duplicate customers
        for n in r[1:-1]:
            if n in seen:
                return False, f"Duplicate customer {n} found in route {idx}"
            seen.add(n)

    # 5. Missing customers
    missing = all_customers - seen
    if missing:
        return False, f"Missing customers: {sorted(list(missing))}"

    # 6. Extra unexpected nodes (should never happen)
    extra = seen - all_customers
    if extra:
        return False, f"Extra invalid customer IDs: {sorted(list(extra))}"

    return True, "OK"

# ------------------------- Batch runner -------------------------
if __name__ == '__main__':
    random.seed(0)

    input_dir = 'Vrp-Set-A-input'
    output_dir = 'Vrp-Set-A-output'
    os.makedirs(output_dir, exist_ok=True)

    files = [f for f in os.listdir(input_dir) if f.endswith('.vrp') and not f.endswith('.sol')]
    if not files:
        print('No .vrp files found in', input_dir)
        sys.exit(1)

    summary = []
    for fname in files:
        path = os.path.join(input_dir, fname)
        print('Processing', fname)
        coords, demand, DEPOT, CAPACITY = load_cvrp(path)
        best, best_iter = run_aco_instance(coords, demand, DEPOT, CAPACITY, PARAMS)
        
        out_path = os.path.join(output_dir, fname.replace('.vrp', '.sol'))
        with open(out_path, 'a') as f:
            f.write("\n")
            f.write("==============ACO===============")
            f.write(f"Instance: {fname}\n")
            f.write(f"Best iter: {best_iter}\n")
            f.write(f"Solution: {best.length:.3f}\n")
            f.write(f"Vehicles: {len(best.routes)}\n")
            for i, r in enumerate(best.routes, 1):
                load = sum(demand[n] for n in r if n != DEPOT)
                f.write(f"Route {i}: {r} | load: {load}\n")
        print('Wrote result to', out_path)
        summary.append((fname, best.length, len(best.routes), best_iter))

    # print summary
    print('Summary:\n')
    for s in summary:
        print(s)
