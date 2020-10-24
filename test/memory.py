
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

	followA = cmds.duplicate(orbs[0])[0]
	cmds.setAttr(followA + ".scale", 0.5, 0.5, 0.5)
	followB = cmds.duplicate(orbs[0])[0]
	cmds.setAttr(followB + ".scale", 0.2, 0.2, 0.2)

	# check if it's frame zero
	cond = cmds.createNode("condition")
	cmds.setAttr(cond + ".secondTerm", 2.0)
	cmds.setAttr(cond + ".operation", 4)
	con("time1.outTime", cond + ".firstTerm")

	cmds.connectAttr(source + ".frame[1].data[0]", followA + ".translate")
	cmds.connectAttr(source + ".frame[2].data[0]", followB + ".translate")

	# # janky multi-frame solver setup with individual cache nodes
	# sources = [source]
	# sinks = [sink]
	# for i in "BC":
	# 	sourceN = cmds.createNode("memorySource", n="source" + i)
	# 	sinkN = cmds.createNode("memorySink", n="sink" + i)
	# 	cmds.connectAttr(sourceN + ".sink", sinkN + ".source")
	# 	sources.append(sourceN)
	# 	sinks.append(sinkN)
	# for i in range(3):
	# 	cmds.connectAttr(sources[0] + ".frame[0].data[{}]".format(i),
	# 	                 sinks[1] + ".data[{}]".format(i))
	# 	cmds.connectAttr(sources[1] + ".frame[0].data[{}]".format(i),
	# 	                 sinks[2] + ".data[{}]".format(i))
	# cmds.connectAttr(sources[1] + ".frame[0].data[0]", followA + ".translate", f=1)
	# cmds.connectAttr(sources[2] + ".frame[0].data[0]", followB + ".translate", f=1)

	# before reset frame, take starting positions from outside solve
	# after, solve values take over

	""" velocity verlet integration :
	x2 = v * dt + 0.5 * a * dt * dt
	v = a * dt
	
	position verlet
	x1 = x2
	x2 = x2 * 2 - x0 + a * dt * dt
	x0 = x1
	
	"""
	ctl = cmds.createNode("transform", n="CTL_GRP")
	cmds.addAttr(ctl, ln="exponent", dv=2)
	# how many data elements will each item pass through the simulation?
	nDataElements = 2
	for i in range(3):

		posPlug = "data[{}]".format(nDataElements * i)
		inPosPlug = source + ".frame[0]." + posPlug

		vPlug = "data[{}]".format(nDataElements * i + 1)

		# get distances to other bodies
		nSum = 0
		forceSum = cmds.createNode("plusMinusAverage")
		cmds.setAttr(forceSum + ".operation", 3) # average
		for n in range(3):
			if i == n:
				continue

			# vector to body
			vector = cmds.createNode("plusMinusAverage")
			cmds.setAttr(vector + ".operation", 2) # subtract
			con(source + ".frame[0].data[{}]".format(nDataElements * n),
			    vector + ".input3D[0]")
			con(inPosPlug, vector + ".input3D[1]")

			# distance
			distance = cmds.createNode("distanceBetween")
			con(vector + ".output3D", distance + ".point1")

			# squared
			dSquared = cmds.createNode("multiplyDivide")
			cmds.setAttr(dSquared + ".operation", 3) # power
			con(ctl + ".exponent", dSquared + ".input2X")
			con( distance + ".distance", dSquared + ".input1X")

			# normalise vector
			norm = cmds.createNode("vectorProduct")
			cmds.setAttr(norm + ".operation", 0)
			cmds.setAttr(norm + ".normalizeOutput", 1)
			con(dSquared + ".output", norm + ".input1")

			# divide force
			div = cmds.createNode("multiplyDivide")
			cmds.setAttr(div + ".operation", 2) # divide
			con(norm + ".output", div + ".input1")
			for ax in "XYZ":
				con(dSquared + ".outputX", div + ".input2" + ax)

			# connect to sum
			con(div + ".output", forceSum + ".input3D[{}]".format(nSum))
			nSum += 1


		# integrate


		# random start for now
		cmds.setAttr(startTfs[i] + ".translateX", random.random() * 5)
		cmds.setAttr(startTfs[i] + ".translateY", random.random() * 5)

		# position switch
		posSwitch = cmds.createNode("choice")
		con(cond + ".outColorR", posSwitch + ".selector")
		con(startTfs[i] + ".translate", posSwitch + ".input[0]")
		con(posSwitch + ".output",
		    sink + ".data[{}]".format(i * nDataElements))
		# random start velocity too?
		# con(posSwitch + ".output",
		#     sink + ".data[{}]".format(i * nDataElements + 1))
		# output

		con(sink + ".data[{}]".format(i), orbs[i] + ".translate")

		# test
		sourcePlugs = [source + ".frame[{}].data[{}]".format(
			n, i) for n in range(3)]
		add = cmds.createNode("plusMinusAverage")
		con(sourcePlugs[0], add + ".input3D[0]")
		con(sourcePlugs[1], add + ".input3D[1]")
		con(add + ".output3D", posSwitch + ".input[1]")





	# stable solution first


