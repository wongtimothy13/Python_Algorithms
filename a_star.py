import copy
import heapq
import math
import matplotlib.pyplot as plt
import numpy as np
import time
from heapq import *

# car state = (x,y)
# state tuple (f,g,(x,y), [(x1,y1),(x2,y2)...])
# total cost f(n) = actual cost g(n) + heuristic cost h(n)
# obstacles = [(x,y), ...]
# min_x, max_x, min_y, max_y are the boundaries of the environment

class Node():
    def __init__(self, parent=None, pos=None):
        self.parent = parent 
        self.pos = pos
        self.g = 0
        self.h = 0
        self.f = 0

        


class a_star:
    def __init__(self, min_x, max_x, min_y, max_y, \
            obstacle=[], resolution=1, robot_size=1):
        self.obstacle = obstacle
        self.grid = np.zeros((max_y-min_y+1,max_x-min_x+1))
        self.min_x = min_x 
        self.max_x = max_x
        self.min_y = min_y
        self.max_y = max_y
        # print(self.grid.shape)
    ## TODO     

    # state: (total cost f, previous cost g, current position (x,y), \
    # previous motion id, path[(x1,y1),...])
    # start = (sx, sy)
    # end = (gx, gy)
    # sol_path = [(x1,y1),(x2,y2), ...]
    def find_path(self, start, end):
        sol_path = []
    # https://code.activestate.com/recipes/578919-python-a-pathfinding-with-binary-heap/
        ##TODO
        g = {start:0}
        heur = np.sqrt((end[0]-start[0])**2 + (end[1]-start[1])**2)
        f = {start:heur}
        
        open_ls = []
        parent_set = {}
        closed_set = set()
        # push as a tuple
        heappush(open_ls, (f[start], start))

        flag = 0
        while open_ls:
            current = heappop(open_ls)[1]
            # finished and return the reverse path
            if current == end:
                while current in parent_set:
                    sol_path.append(current)
                    current = parent_set[current]
                sol_path.append(start)
                print("TEST")
                print(sol_path[::-1])
                return sol_path[::-1]
            closed_set.add(current)

            # go search around the neighbors
            #       tl       tm    tr     ml    mr      bl      bm     br
            nbrs = [(-1,1),(0,1),(1,1), (-1,0),(1,0), (-1,-1),(0,-1),(1,-1)]
            for nbr in nbrs:
                nbr_x = current[0]+nbr[0]
                nbr_y = current[1]+nbr[1]

                # do not include things out of bounds.
                if (nbr_x >= self.max_x) and (nbr_x <= self.min_x) and (nbr_y >= self.max_y) and (nbr_y <= self.min_y):
                    continue
                # do not include obstacles
                if (nbr_x,nbr_y) in self.obstacle:
                    continue
                heur = np.sqrt((nbr_x-current[0])**2 + (nbr_y-current[1])**2)
                tt_g_score = g[current] + heur

                if (nbr_x,nbr_y) in closed_set and tt_g_score >= g.get((nbr_x,nbr_y), 0):
                    continue
                # print("strnge")
                # print(g.get((nbr_x,nbr_y)))

                if  tt_g_score < g.get((nbr_x,nbr_y), 0) or (nbr_x,nbr_y) not in [i[1]for i in open_ls]:
                    parent_set[(nbr_x,nbr_y)] = current
                    g[(nbr_x,nbr_y)] = tt_g_score
                    heur = np.sqrt((nbr_x-end[0])**2 + (nbr_y-end[1])**2)
                    f[(nbr_x,nbr_y)] = tt_g_score + heur
                    heappush(open_ls, (f[(nbr_x,nbr_y)], (nbr_x,nbr_y)))
         


def main():
    print(__file__ + " start!!")

    grid_size = 1  # [m]
    robot_size = 1.0  # [m]

    sx, sy = -10, -10
    gx, gy = 10, 10
    obstacle = []
    for i in range(30):
        obstacle.append((i-15, -15))
        obstacle.append((i-14, 15))
        obstacle.append((-15, i-14))
        obstacle.append((15, i-15))

    for i in range(10):
        # obstacle.append((0,i))
        # obstacle.append((0,-i))
        obstacle.append((5,0+i))
        obstacle.append((6,0+i))
        obstacle.append((5,9+i))
        obstacle.append((6,9+i))
        obstacle.append((3+i,0))
        obstacle.append((3+i,0))
        # obstacle.append((-i,0))
    plt.plot(sx, sy, "xr")
    plt.plot(gx, gy, "xb")
    plt.grid(True)
    plt.axis("equal")

    simple_a_star = a_star(-15, 15, -15, 15, obstacle=obstacle, \
        resolution=grid_size, robot_size=robot_size)
    path = simple_a_star.find_path((sx,sy), (gx,gy))
    print (path)

    rx, ry = [], []
    for node in path:
        rx.append(node[0])
        ry.append(node[1])

    plt.plot(rx, ry, "-r")
    plt.show()


if __name__ == '__main__':
    main()
