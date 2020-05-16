
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
	cmds.connectAttr(cube + ".translateY", sink + ".data[0]")
	cmds.setAttr(cube + ".translateX", 1.2)
	cmds.connectAttr("time1.outTime", source + ".time")

	#cmds.setAttr(source + ".resetFrame", 1)

	adl = cmds.createNode("addDoubleLinear")
	cmds.connectAttr(source + ".data[0]", adl + ".input1")
	#cmds.connectAttr(source + ".floatData", adl + ".input2")

	outputCube = cmds.duplicate(cube, n="outputCube")[0]
	midCube = cmds.duplicate(cube, n="midCube")[0]
	cmds.setAttr(midCube + ".translateZ", 5)
	cmds.connectAttr(adl + ".output", midCube + ".translateY")

	# test chaining memory cells
	sink2 = cmds.createNode("memorySink", n="secondSink")
	source2 = cmds.createNode("memorySource", n="secondSource")
	cmds.connectAttr(source2 + ".sink", sink2 + ".source")
	cmds.connectAttr( source + ".data[0]", sink2 + ".data[0]")
	cmds.connectAttr("time1.outTime", source2 + ".time")


	cmds.connectAttr(source2 + ".data[0]", outputCube + ".translateY")
	cmds.setAttr(outputCube + ".translateZ", 10)

	cmds.setKeyframe(cube, at="translateY", time=1, value=0)
	cmds.setKeyframe(cube, at="translateY", time=20, value=10)
	cmds.setKeyframe(cube, at="translateY", time=40, value=0)

	return cube

def testDeformers(mesh):
	""" test uberDeformer system """
	deformer = cmds.deformer(mesh, type="uberDeformer")[0]
	baseNotion = cmds.createNode("deformerNotion", n="testNotion")

	cmds.connectAttr(baseNotion + ".masterConnection",
	                 deformer + ".notions[0]")




