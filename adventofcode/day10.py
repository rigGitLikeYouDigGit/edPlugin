
from edPlugin.adventofcode.inputs import day10

import math
import fractions


"""
on an integer grid, two points are aligned if the rational numbers
formed by their coordinates are multiples of each other
i think

we actually only care about each point's vector from the station

"""



def processPoints():
	pass


class ListGrid(object):
	""" wish i had this on day 3 """
	def __init__(self, rows, columns, entries=None):
		self.rows = rows
		self.columns = columns
		self.entries = entries
		# ROW LEFT TO RIGHT, THEN COLUMN TOP TO BOTTOM

	def point(self, x, y):
		return self.entries[ y * self.columns + x ]

if __name__ == '__main__':
	rows = day10.split("\n")
	nRows = len(rows)
	print nRows # 34
	nColumns = len( rows[0] )
	print nColumns
	points = [] # list of fractions

	for row in range(nRows):  # y
		for column in range(nColumns): # x
			points.append( (column, row) )

	#grid = ListGrid(nRows, nColumns, points)

	# check each point
	mostSeen = 0
	for row in range(nRows):  # y
		for column in range(nColumns): # x
			pos = (column, row)
			seenSet = set()
			for i in points:
				vec = (i[0] - pos[0], i[1] - pos[1])
				if not vec[0]:
					seenSet.add(10)
					continue
				fract = fractions.Fraction(vec[1], vec[0])
				seenSet.add( fract )
			nSeen = len(seenSet)
			mostSeen = mostSeen if mostSeen > nSeen else nSeen
			#print(mostSeen)
	print mostSeen
	print seenSet



