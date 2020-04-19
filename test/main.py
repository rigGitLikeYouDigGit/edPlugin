
""" tests for each plugin node """
from maya import cmds

from edPlugin import MLL_PATH, PLUGIN_ID

from edPlugin.test import meshanalysis


def unloadPlugin(path=None):
	""" forces new scene and unloads pplugin for recompilation """
	path = path or PLUGIN_ID
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
	return cube

def testDeformers(mesh):
	""" test uberDeformer system """
	deformer = cmds.deformer(mesh, type="uberDeformer")[0]
	baseNotion = cmds.createNode("deformerNotion", n="testNotion")

	cmds.connectAttr(baseNotion + ".masterConnection",
	                 deformer + ".notions[0]")




