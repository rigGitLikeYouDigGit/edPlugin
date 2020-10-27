
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


	ctl = cmds.createNode("transform", n="CTL_GRP")
	cmds.addAttr(ctl, ln="exponent", dv=2)
	cmds.addAttr(ctl, ln="timeScale", dv=0.2)
	# how many data elements will each item pass through the simulation?
	nDataElements = 2
	for i in range(3):

		posPlug = "data[{}]".format(nDataElements * i)
		inPosPlug = source + ".frame[0]." + posPlug

		vPlug = "data[{}]".format(nDataElements * i + 1)

		# get distances to other bodies
		nSum = 0
		forceSum = cmds.createNode("plusMinusAverage", n="forceSum")
		#cmds.setAttr(forceSum + ".operation", 3) # average

		forceNegate = cmds.createNode("multiplyDivide", n="forceNegate")
		for ax in "XYZ":
			cmds.setAttr(forceNegate + ".input1" + ax, 3)
		cmds.connectAttr(forceSum + ".output3D", forceNegate + ".input2")
		forceOutPlug = forceNegate + ".output"

		for n in range(3):
			if i == n:
				continue

			# vector to body
			vector = cmds.createNode("plusMinusAverage",
			                         n="vec{}to{}".format(i, n))
			cmds.setAttr(vector + ".operation", 2) # subtract
			con(source + ".frame[0].data[{}]".format(nDataElements * n),
			    vector + ".input3D[0]")
			con(inPosPlug, vector + ".input3D[1]")

			# distance
			distance = cmds.createNode("distanceBetween", n="vecLength")
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
			con(vector + ".output3D", norm + ".input1")

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
		""" velocity verlet integration :
		x2 += v * dt + 0.5 * a * dt * dt
		v += a * dt

		position verlet
		x1 = x2
		x2 = x2 * 2 - x0 + a * dt * dt
		x0 = x1
		"""

		dt = cmds.createNode("multDoubleLinear", n="dtMdl")
		cmds.connectAttr(ctl + ".timeScale", dt + ".input1")
		cmds.connectAttr(source + ".delta[0]", dt + ".input2")
		dtPlug = dt + ".output"

		vTMult = cmds.createNode("multiplyDivide", n="vTMult")
		con(source + ".frame[0].data[{}]".format(nDataElements * i + 1),
		    vTMult + ".input1")
		for ax in "XYZ":
			con(dtPlug, vTMult + ".input2" + ax)

		tSquared = cmds.createNode("multDoubleLinear", n="tSquared")
		con(dtPlug, tSquared + ".input1")
		con(dtPlug, tSquared + ".input2")

		halfMult = cmds.createNode("multDoubleLinear")
		con(tSquared + ".output", halfMult + ".input1")
		cmds.setAttr(halfMult + ".input2", 0.5)

		aPosMult = cmds.createNode("multiplyDivide", n="aPosMult")
		con( forceOutPlug, aPosMult + ".input1")
		for ax in "XYZ":
			con(halfMult + ".output", aPosMult + ".input2" + ax)

		aPosAdd = cmds.createNode("plusMinusAverage", n="aPosAdd")
		con(vTMult + ".output", aPosAdd + ".input3D[0]")
		con(aPosMult + ".output", aPosAdd + ".input3D[1]")

		prevPosAdd = cmds.createNode("plusMinusAverage", n="prevPosAdd")
		con(source + ".frame[0].data[{}]".format(i * nDataElements),
		    prevPosAdd + ".input3D[0]")
		con(aPosAdd + ".output3D", prevPosAdd + ".input3D[1]")

		#posOutPlug = aPosAdd + ".output3D"
		posOutPlug = prevPosAdd + ".output3D"

		# velocity
		sourceVPlug = source + ".frame[0].data[{}]".format(i * nDataElements + 1)
		aVMult = cmds.createNode("multiplyDivide", n="aVMult")
		con(sourceVPlug, aVMult + ".input1")
		for ax in "XYZ":
			con(dtPlug, aVMult + ".input2" + ax)

		prevVAdd = cmds.createNode("plusMinusAverage", n="prevVAdd")
		con(sourceVPlug, prevVAdd + ".input3D[0]")
		#con(aVMult + ".output", prevVAdd + ".input3D[1]")

		#vOutPlug = aVMult + ".output"
		vOutPlug = prevVAdd + ".output3D"



		# random start for now
		cmds.setAttr(startTfs[i] + ".translateX", random.random() * 5)
		cmds.setAttr(startTfs[i] + ".translateY", random.random() * 5)

		# position switch
		posSwitch = cmds.createNode("choice", n="pos{}_switch".format(i))
		con(cond + ".outColorR", posSwitch + ".selector")
		con(startTfs[i] + ".translate", posSwitch + ".input[0]")
		con(posSwitch + ".output",
		    sink + ".data[{}]".format(i * nDataElements))
		con( posOutPlug, posSwitch + ".input[1]")

		# velocity switch
		vSwitch = cmds.createNode("choice", n="v{}_switch".format(i))
		con(cond + ".outColorR", vSwitch + ".selector")
		#con(startTfs[i] + ".translate", vSwitch + ".input[0]")
		vSource = cmds.createNode("transform", n="vSource")
		con(vSource + ".translate", vSwitch + ".input[0]")
		con( vOutPlug, vSwitch + ".input[1]")
		con(vSwitch + ".output",
		    sink + ".data[{}]".format(i * nDataElements + 1))

		con(sink + ".data[{}]".format(i * nDataElements), orbs[i] + ".translate")



		# random start velocity too?


	# stable solution first


