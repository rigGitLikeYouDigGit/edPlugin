
from edPlugin.adventofcode.inputs import day6


class OrbitNode(object):

	def __init__(self, id=None):
		self.id = id
		self.parent=None
		self.children=set()

		# undirected capability
		self.neighbours = set()
		self.distances = {}

	@property
	def depth(self):
		i = self
		depth = 0
		while i.parent:
			depth += 1
			i = i.parent
		#print "{} depth {}".format( self.id, depth)
		return depth


def assignDistanceFrom(register, nodeId):

	node = register[nodeId]
	node.distances[nodeId] = 0
	passed = { node }
	traverse(node, 0, nodeId, passed=passed)


def traverse(node, distance, fromId, passed=None):
	distance += 1
	print("node {}".format(node.id))

	for i in node.neighbours - passed:
		passed.add(i)

		i.distances[fromId] = distance
		traverse(i, distance, fromId, passed)



if __name__ == '__main__':

	nodes = {}

	tokens = [i for i in day6.split("\n") if i]

	for i in tokens:
		parent, child = i.split(")")
		for n in (parent, child):
			if not nodes.get(n): nodes[n] = OrbitNode(n)
		parent = nodes[parent]
		child = nodes[child]

		child.parent = parent
		parent.children.add(child)

		parent.neighbours.add(child)
		child.neighbours.add(parent)

	root = nodes["COM"]

	depth = 0
	for k, v in nodes.iteritems():
		depth += v.depth
	#print depth

	# find distance from YOU to SAN
	assignDistanceFrom(nodes, "YOU")
	print nodes["SAN"].distances
	print nodes["SAN"].neighbours
	print nodes["YOU"].distances
	print nodes["YOU"].neighbours

	print nodes["SAN"].distances["YOU"] - 2


