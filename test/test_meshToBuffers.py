


from maya import cmds

import math
from edPlugin.test import lib

class TestMeshToBuffers(lib.MayaTest):
	nodeTypes = ["meshToBuffers"]


	def test_meshToBuffers(self):
		""" check that buffers are working correctly
		"""
		node = cmds.createNode("meshToBuffers")

		cube = cmds.polyCube(n="testCube", ch=0)[0]

		shape = cube + "Shape"
		# cmds.connectAttr(shape + ".outMesh", node + ".inMesh")
		cmds.connectAttr(shape + ".worldMesh[0]", node + ".inMesh")

		cmds.setAttr(node + ".bind", 1)

		result = cmds.getAttr(node + ".pointPositionsRaw")
		expected = [-0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5]
		self.assertEqual(result, expected, msg=
		                 "MTB raw positions not correct in neutral - \n"
		                 f"expected {expected}, \n"
		                 f"got {result}")
		#print(result)

		cmds.xform(cube, t=(1, 1, 1), rotation=(20, 50, 30))
		cmds.makeIdentity(cube, apply=1)
		
		
		# attach locator for easier viewing
		loc = cmds.spaceLocator()[0]


		aim = cmds.createNode("aimConstraint")
		cmds.connectAttr(node + ".pointNormal[0]",
		                 aim + ".target[0].targetTranslate")
		cmds.connectAttr(aim + ".constraintRotate", loc + ".rotate")



		cmds.connectAttr(node + ".point[0]", loc + ".translate")
		cmds.connectAttr(cube + ".matrix", loc + ".offsetParentMatrix")
		print(loc)


		expected = [1.2403451204299927, 0.39876610040664673, 1.5751104354858398, 1.7970155477523804, 0.7201598882675171, 0.8090659976005554, 0.9973996877670288, 1.3435651063919067, 1.7949568033218384, 1.5540701150894165, 1.6649589538574219, 1.0289123058319092, 0.20298445224761963, 1.279840111732483, 1.1909339427947998, 0.7596548795700073, 1.6012338399887085, 0.42488956451416016, 0.4459298849105835, 0.3350411057472229, 0.9710876941680908, 1.0026001930236816, 0.6564348936080933, 0.2050432562828064]
		result = cmds.getAttr(node + ".pointPositionsRaw")
	
		self.assertEqual(result, expected, msg=
		"MTB raw positions not correct in transform - \n"
		f"expected {expected}, \n"
		f"got {result}")

		bindResult = cmds.getAttr(node + ".bind")
		self.assertEqual(bindResult, 2, msg=
		                 "MTB bind attribute not updating to bound")

		# vector attributes
		plugSize = cmds.getAttr(node + ".point", size=1)
		floats = []
		vectors = []
		for i in range(plugSize):
			plug = node + f".point[{i}]"
			vectors.append(cmds.getAttr(plug)[0])
			plug = plug + ".pointX"
			floats.append(cmds.getAttr(plug))

		expectedFloats = [1.2403451204299927, 1.7970155477523804, 0.9973996877670288, 1.5540701150894165, 0.20298445224761963, 0.7596548795700073, 0.4459298849105835, 1.0026001930236816]
		expectedVectors = [(1.2403451204299927, 0.39876610040664673, 1.5751104354858398), (1.7970155477523804, 0.7201598882675171, 0.8090659976005554), (0.9973996877670288, 1.3435651063919067, 1.7949568033218384), (1.5540701150894165, 1.6649589538574219, 1.0289123058319092), (0.20298445224761963, 1.279840111732483, 1.1909339427947998), (0.7596548795700073, 1.6012338399887085, 0.42488956451416016), (0.4459298849105835, 0.3350411057472229, 0.9710876941680908), (1.0026001930236816, 0.6564348936080933, 0.2050432562828064)]
		# print(floats)
		# print(vectors)
		self.assertEqual(vectors, expectedVectors, msg=
		"MTB vector positions not correct in transform - \n"
		f"expected {expectedVectors}, \n"
		f"got {vectors}")

		self.assertEqual(floats, expectedFloats, msg=
		"MTB vector positions float elements not correct in transform - \n"
		f"expected {expectedFloats}, \n"
		f"got {floats}")

		
		
		



def test(*args, **kwargs):
	node = cmds.createNode("meshToBuffers")

	cube = cmds.polyCube(n="testCube", ch=0)[0]

	shape = cube + "Shape"
	# cmds.connectAttr(shape + ".outMesh", node + ".inMesh")
	cmds.connectAttr(shape + ".worldMesh[0]", node + ".inMesh")

	cmds.setAttr(node + ".bind", 1)

	result = cmds.getAttr(node + ".pointPositions")
	print(result)

	cmds.xform(cube, t=(1, 1, 1), rotation=(20, 50, 30))
	cmds.makeIdentity(cube, apply=1)

	result = cmds.getAttr(node + ".pointPositions")
	print(result)
	
	bindResult = cmds.getAttr(node + ".bind")
	print(bindResult) # should be 2, bound


