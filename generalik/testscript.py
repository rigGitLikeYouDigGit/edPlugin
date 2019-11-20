from maya import cmds
import sys
for i in sys.modules:
	del i

path = "F:\\all_projects_desktop\common\edCode\edPlugin\generalIk.py"
path = "generalIk.py"
cmds.loadPlugin( path )
cmds.file(new=True, f=True)

cmds.unloadPlugin(path, f=True)

cmds.file(new=True, f=True)

cmds.loadPlugin( path )

cmds.file(new=True, f=True)
# setup basic ik scene


def connectInputJoint(joint, ik, index):
	cmds.connectAttr(joint + ".worldMatrix[0]",
	                 ik + ".inputJoints[{}].matrix".format(index))
	cmds.connectAttr(joint + ".jointOrient",
	                 ik + ".inputJoints[{}].orient".format(index))

def connectOutputJoint(ik, joint, index):
	cmds.connectAttr(ik + ".outputJoints[{}].translate".format(index),
	                 joint + ".translate")
	cmds.connectAttr(ik + ".outputJoints[{}].rotate".format(index),
	                 joint + ".rotate")

target = cmds.createNode("joint", n="effector")
cmds.setAttr(target + ".translateY", 2)
cmds.setAttr(target + ".translateX", 2)
cmds.setAttr(target + ".translateZ", 3)


generalIk = cmds.createNode("generalIk")
cmds.setAttr( generalIk + ".maxIterations", 2)


chainLength = 4
baseChain = []
outputChain = []
for i in range(chainLength):
	base = cmds.createNode("joint", n="base_{}_jnt".format(i))
	cmds.setAttr(base + ".translateY", i * 5)

	out = cmds.duplicate(base, n="output_{}_jnt".format(i))[0]
	cmds.connectAttr(base + ".jointOrient", out + ".jointOrient")
	cmds.connectAttr(base + ".rotateOrder", out + ".rotateOrder")
	baseChain.append(base)
	outputChain.append(out)

	if i:
		cmds.parent(base, baseChain[i-1])
		cmds.parent(out, outputChain[i-1])


cmds.setAttr(baseChain[0] + ".translateX", 3)
#cmds.setAttr(baseChain[0] + ".rotateZ", 20)

for i in range(chainLength):
	if i < chainLength - 1:
		connectInputJoint(baseChain[i], generalIk, i)
		connectOutputJoint(generalIk, outputChain[i], i)
	else:
		# connect ends
		cmds.connectAttr(baseChain[i] + ".worldMatrix[0]",
		                 generalIk + ".inputEndMatrix")
		cmds.connectAttr(generalIk + ".outputEndTranslate",
		                 outputChain[i] + ".translate")
		cmds.connectAttr(generalIk + ".outputEndRotate",
		                 outputChain[i] + ".rotate")




up = cmds.spaceLocator(n="up")[0]
cmds.setAttr(up + ".translateZ", 5)


debugTarget = cmds.spaceLocator(n="debugOut")[0]
decomp = cmds.createNode("decomposeMatrix")
cmds.connectAttr(generalIk + ".debugTarget", decomp + ".inputMatrix")
cmds.connectAttr(decomp + ".outputTranslate", debugTarget + ".translate")
cmds.connectAttr(decomp + ".outputRotate", debugTarget + ".rotate")


debugOffset = cmds.polyCube(ch=0)[0]
cmds.setAttr(debugOffset + ".translateZ", -2)



cmds.connectAttr(generalIk + ".debugOffset", debugOffset + ".translateY")

cmds.connectAttr( target + ".worldMatrix[0]", generalIk + ".targetMatrix")
cmds.connectAttr( up + ".worldMatrix[0]", generalIk + ".inputJoints[0].upMatrix")


cmds.dgdirty(generalIk)

