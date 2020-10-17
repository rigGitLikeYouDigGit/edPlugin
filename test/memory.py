
import random

from maya import cmds

def con(*args):
	cmds.connectAttr(*args, f=1)

def testMemory():
	""" set up 2 3-body problems, one with random inputs,
	one with the stable solution - see how long it stays stable"""

	source = cmds.createNode("memorySource")
	sink = cmds.createNode("memorySink")
	con(source + ".sink", sink + ".source")

	startTfs = [cmds.createNode("transform",
	                            n="start{}_pos".format(i)) for i in range(3)]
	orbs = [cmds.polySphere(n="orb{}_ply".format(i))[0] for i in range(3)]

	# check if it's frame zero
	cond = cmds.createNode("condition")
	cmds.setAttr(cond + ".secondTerm", 2.0)
	cmds.setAttr(cond + ".operation", 4)
	con("time1.outTime", cond + ".firstTerm")

	# before reset frame, take starting positions from outside solve
	# after, solve values take over
	for i in range(3):

		# random start for now
		cmds.setAttr(startTfs[i] + ".translateX", random.random() * 5)
		cmds.setAttr(startTfs[i] + ".translateY", random.random() * 5)

		# position switch
		posSwitch = cmds.createNode("choice")
		con(cond + ".outColorR", posSwitch + ".selector")
		con(startTfs[i] + ".translate", posSwitch + ".input[0]")
		con(posSwitch + ".output", sink + ".data[{}]".format(i))

		# output
		con(posSwitch + ".output", orbs[i] + ".translate")

		# test
		sourcePlugs = [source + ".frame[{}].data[{}]".format(
			n, i) for n in range(3)]
		add = cmds.createNode("plusMinusAverage")
		con(sourcePlugs[0], add + ".input3D[0]")
		con(sourcePlugs[1], add + ".input3D[1]")
		con(add + ".output3D", posSwitch + ".input[1]")




	# stable solution first


