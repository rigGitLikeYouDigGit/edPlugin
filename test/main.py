
""" tests for each plugin node """
from maya import cmds

from edPlugin import MLL_PATH, PLUGIN_ID

from edPlugin.test import meshanalysis


def unloadPlugin(path=None):
	""" forces new scene and unloads plugin for recompilation """
	path = path or PLUGIN_ID
	path = PLUGIN_ID
	path = MLL_PATH
	path = "edPlugin"
	cmds.file(new=1, f=1)
	cmds.unloadPlugin(path, f=True)

def loadPlugin(path=None):
	cmds.loadPlugin( PLUGIN_ID )


def runTests():
	""" loads plugin and runs tests """
	cmds.file(new=1, f=1)
	loadPlugin()
	print("running edPlugin tests")

	createPluginNodes()
	cube = baseTest()
	testDeformers(cube)


def createPluginNodes():
	""" basic: can we instantiate each node? """
	for i in cmds.pluginInfo("edPlugin", q=1, dependNode=1):
		cmds.createNode(i)

def baseTest():
	""" temp trash for nodes that don't need indepth testing """

	cube = cmds.polyCube()[0]

	buffers = cmds.createNode("meshToBuffers")
	cmds.connectAttr( cube + ".outMesh", buffers + ".inMesh", f=True)
	cmds.setAttr( buffers + ".bind", 3) #live, bind system works fine
	print("testsRun")

	# memory sources and sinks
	source = cmds.createNode("memorySource")
	sink = cmds.createNode("memorySink")
	cmds.connectAttr(source + ".sink", sink + ".source")
	cmds.connectAttr(cube + ".translateY", sink + ".data")
	cmds.connectAttr(cube + ".translateX", sink + ".floatData")
	cmds.setAttr(cube + ".translateX", 1.2)
	cmds.connectAttr("time1.outTime", sink + ".time")
	cmds.connectAttr("time1.outTime", source + ".time")

	adl = cmds.createNode("addDoubleLinear")
	cmds.connectAttr(source + ".data", adl + ".input1")
	cmds.connectAttr(source + ".floatData", adl + ".input2")

	outputCube = cmds.duplicate(cube, n="outputCube")[0]
	cmds.setAttr(outputCube + ".translateZ", 5)
	cmds.connectAttr(adl + ".output", outputCube + ".translateY")

	return cube

def testDeformers(mesh):
	""" test uberDeformer system """
	deformer = cmds.deformer(mesh, type="uberDeformer")[0]
	baseNotion = cmds.createNode("deformerNotion", n="testNotion")

	cmds.connectAttr(baseNotion + ".masterConnection",
	                 deformer + ".notions[0]")




