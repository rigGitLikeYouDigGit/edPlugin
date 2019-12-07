
from edPlugin.adventofcode.inputs import day3

# dynamic grid with nested dicts
grid = { 0 : { 0 : 0}}
# x { y : passed } }

# use dict of id : signal path

class Turtle(object):
	"""receives world-space directions of up, down, left, right"""
	dirMap = {
		"U" : (0, 1),
		"D" : (0, -1),
		"L" : (-1, 0),
		"R" : (1, 0),
	}
	def __init__(self, id):
		self.xy = [0, 0]
		self.id = id
		self.pathDelay = 0

	def feedWholeInput(self, directions, grid=None, intersections=None):
		tokens = directions.split(",")
		for i in tokens:
			intersections = self.move(i, grid, intersections)
		return intersections

	def move(self, token, grid=None, intersections=None):
		""" bit verbose but i was going for functionality"""
		direction = token[0]
		d = int(token[1:])
		axes = self.dirMap[ direction ]
		x = abs(axes[0]) * d
		y = abs(axes[1]) * d
		dx = axes[0]
		dy = axes[1]

		intersections = intersections or []

		for i in range(d):
			self.xy[0] += dx
			self.xy[1] += dy
			self.pathDelay += 1
			if grid:
				# crawl through grid
				try:
					check = grid[self.xy[0]][self.xy[1]]
					if self.id not in check:
						check[self.id] = self.pathDelay
						intersections.append( ( (self.xy[0], self.xy[1]), check) )
						print "found"
				except:
					grid[self.xy[0]] = grid.get(self.xy[0]) or {}
					grid[self.xy[0]][self.xy[1]] = \
						grid[self.xy[0]].get(self.xy[1]) or {self.id : self.pathDelay}
				finally:
					grid[self.xy[0]][self.xy[1]][self.id] = self.pathDelay
		return intersections



if __name__ == '__main__':

	intersections = []
	grid = {0 : {0 : 0} }

	turtleA = Turtle(id="ey")
	directions = day3[0]
	intersections = turtleA.feedWholeInput(
		directions, grid=grid, intersections=intersections)

	print intersections

	turtleB = Turtle(id="yo")
	directions = day3[1]
	intersections = turtleB.feedWholeInput(
		directions, grid=grid, intersections=intersections)

	print intersections

	# manhattan distance
	distance = min([abs(i[0][0] + i[0][1]) for i in intersections])
	pathDistance = min([sum(i[1].values()) for i in intersections])
	print distance
	print pathDistance

	pass



