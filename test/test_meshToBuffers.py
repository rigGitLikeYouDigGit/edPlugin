


from maya import cmds

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


