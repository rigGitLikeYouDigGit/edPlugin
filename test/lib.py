
import unittest
cmds = None
try:
	from maya import cmds

except:
	pass


class MayaTest(unittest.TestCase):
	"""base class for tests within maya"""

	# which plugin nodes does this test relate to?
	nodeTypes = []
	clearScene = True

	def setUp(self):
		# from maya import standalone
		# standalone.initialize()
		if self.clearScene:
			cmds.file(new=1, f=1)

	def test_nodeCreation(self):
		""" check that all nodes can be created successfully in scene
		"""
		for typeName in self.nodeTypes:
			try:
				print("creating", typeName)
				node = cmds.createNode(typeName)
				print("created", node)
				# check name is correct
				self.assertIn(typeName, node,
				              msg=f"""Node {node} unable to be created""")

				self.assertEqual(typeName, cmds.nodeType(node),
					msg=f"""Node {node} type is not correct""")


			except:
				self.fail(msg=f"""Creating node {node} raised error""")
