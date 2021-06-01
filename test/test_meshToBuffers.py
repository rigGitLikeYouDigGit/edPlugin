


from maya import cmds

def test(*args, **kwargs):
	node = cmds.createNode("meshToBuffers")

	cube = cmds.polyCube(n="testCube", ch=0)[0]

	shape = cube + "Shape"
	cmds.connectAttr(shape + ".outMesh", node + ".inMesh")

	cmds.setAttr(node + ".bind", 1)

	result = cmds.getAttr(node + ".pointPositions")
	print(result)


