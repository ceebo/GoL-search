# Use LifeHistory. Indicate the single channel lane by
# drawing a single NE glider in state 4 (red).
# The output cell list will have your pattern roughly
# centered on (0,0)
import golly as g

cells = g.getcells(g.getrect())
sm = smy = cnt = 0
lane = -999
for i in range(0, len(cells)-2, 3):
    if cells[i+2] == 4:
        lane = max(lane, cells[i] + cells[i+1])
    elif cells[i+2] % 2:
        cnt += 1
        sm += cells[i] - cells[i+1]

sm //= 2 * cnt

dx = lane // 2 + sm - 1
dy = lane - lane // 2 - sm

outstr = "./elbow "
for i in range(0, len(cells)-2, 3):
    if cells[i+2] % 2:
        outstr += "%d %d " % (cells[i] - dx, cells[i+1] - dy)

g.show(outstr)
g.setclipstr(outstr)
