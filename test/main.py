
""" tests for each plugin node """
from maya import cmds, mel
import maya.api.OpenMaya as om

import sys, os

from importlib import reload
import edPlugin
reload(edPlugin)
from edPlugin import MLL_PATH, MLL_RELEASE_PATH, PLUGIN_ID, MLL_DIR

from edPlugin.test import meshanalysis, memory
reload(memory)

from edPlugin.test import test_meshToBuffers
reload(test_meshToBuffers)

testMap = {
	"meshToBuffers" : test_meshToBuffers,
}

def unloadPlugin(path=None):
	""" forces new scene and unloads plugin for recompilation
	this is
	so
	fucking
	annoying
	"""
	path = path or PLUGIN_ID
	path = PLUGIN_ID
	path = MLL_PATH
	path = "edPlugin"
	paths = [MLL_PATH, MLL_RELEASE_PATH, "edPlugin", "edPlugin.mll"]
	cmds.file(new=1, f=1)

	pluginPath = os.environ["MAYA_PLUG_IN_PATH"]
	#print("pluginPath", pluginPath)
	# cmds.unloadPlugin(MLL_RELEASE_PATH, f=1)
	cmds.unloadPlugin("edPlugin.mll", f=1)


def loadPlugin(path=None):
	pluginDir = ";" + MLL_DIR + ";"
	if not pluginDir in os.environ["MAYA_PLUG_IN_PATH"]:
		os.environ["MAYA_PLUG_IN_PATH"] += pluginDir
	#print(os.environ["MAYA_PLUG_IN_PATH"])
	cmds.loadPlugin( MLL_RELEASE_PATH, "edPlugin.mll", quiet=True )


def runTests():
	""" loads plugin and runs tests """
	cmds.file(new=1, f=1)
	loadPlugin()
	print("running edPlugin tests")

	nodes = cmds.pluginInfo(PLUGIN_ID, q=1, dependNode=1)
	print(nodes)

	for node in nodes:
		if testMap.get(node):
			cmds.file(new=1, f=1)
			testMap[node].test()


	#createPluginNodes()
	#cube = baseTest()
	#testDeformers(cube)
	#testDdm()
	#memory.testMemory()

def getMObject(name):
	sel = om.MSelectionList()
	sel.add(name)
	return sel.getDependNode(0)

def getMPlug(nodeObj, plugName):
	return om.MFnDependencyNode(nodeObj).findPlug(plugName, False)


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
	#testMemory(cube)

	return cube


def testDdm():

	# test for skin
	mesh = cmds.polyCylinder(r=1, h=10, sx=8, sy=40, sz=8, ax=(0, 0, 1), ch=0)[0]
	# skin cluster to transfer base weights
	skcMesh = cmds.duplicate(mesh, n="skcMesh")[0]
	refMesh = cmds.duplicate(mesh, n="refDdmMesh")[0]
	joints = []
	refJoints = []

	for i in range(3):
		jnt = cmds.createNode("joint", n="mainJnt_{}".format(i))
		cmds.setAttr(jnt + ".translateZ", i * 5 - 5)
		joints.append(jnt)
		if i: cmds.parent(jnt, joints[i-1])

	skin = cmds.skinCluster(joints, skcMesh)[0]
	ddm = cmds.deformer(mesh, type="directDeltaMush")[0]
	refDdm = cmds.deformer(refMesh, type="refDDM")[0]

	for mush in [ddm, refDdm]:

		# copy matrix connections
		for i in range(3):
			cmds.connectAttr(joints[i] + ".worldMatrix[0]", mush + ".matrix[{}]".format(i))
			#cmds.connectAttr(joints[i] + ".worldMatrix[0]", refDdm + ".matrix[{}]".format(i))
			pass


		# copy plug connections and weights to deltamush
		copyPlugs = ("weightList", "bindPreMatrix")

		size = cmds.getAttr(skin + ".weightList", size=1)
		cmds.setAttr(mush + ".weightList", size=size)

		cmds.select(cl=1)
		for i in copyPlugs:
			sourcePlug = getMPlug(getMObject(skin), i)
			sourceDH = om.MArrayDataHandle( sourcePlug.asMDataHandle())
			sinkPlug = getMPlug(getMObject(mush), i)
			sinkDH = om.MArrayDataHandle( sinkPlug.asMDataHandle())

			sinkDH.copy(sourceDH)

			sourceDH = sourcePlug.asMDataHandle()
			sinkPlug.setMDataHandle(om.MDataHandle(sourceDH))

			cmds.select(mush)

		# direct weight connections from skincluster to allow live weight editing
		for i in range( cmds.getAttr(skin + ".weightList", size=1)):
			array = skin + ".weightList[{}]".format(i)
			# for n in range( cmds.getAttr(array + ".weights", size=1 )):
			# 	srcPlug = array + ".weights[{}]".format(n)

			srcPlug = array
			dstPlug = srcPlug.replace(skin, mush)
			cmds.connectAttr(srcPlug, dstPlug, f=1)

		cmds.setAttr(ddm + ".iterations", 10)
		cmds.setAttr(ddm + ".alpha", 0.5)
		cmds.setAttr(ddm + ".smoothTranslation", 10.0)
		cmds.setAttr(ddm + ".smoothRotation", 10.0)

	# move skc mesh off to side
	group = cmds.group(skcMesh, n="skcOffsetGrp")
	cmds.setAttr(group + ".translateX", 5)

	group = cmds.group(refMesh, n="refOffsetGrp")
	cmds.setAttr(group + ".translateX", -5)



def testDeformers(mesh):
	""" test uberDeformer system """
	deformer = cmds.deformer(mesh, type="uberDeformer")[0]
	baseNotion = cmds.createNode("deformerNotion", n="testNotion")

	cmds.connectAttr(baseNotion + ".masterConnection",
	                 deformer + ".notions[0]")



def testMemory(cube):
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



