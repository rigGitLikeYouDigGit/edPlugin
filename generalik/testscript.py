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


target = cmds.createNode("joint", n="effector")
cmds.setAttr(target + ".translateY", 2)
cmds.setAttr(target + ".translateX", 2)
cmds.setAttr(target + ".translateZ", 3)

base = cmds.createNode("joint", n="templateBase")
end = cmds.createNode("joint", n="templateEnd")
up = cmds.spaceLocator(n="up")[0]
cmds.setAttr(up + ".translateZ", 5)
cmds.setAttr(end + ".translateY", 5)
cmds.parent(end, base)

# cmds.joint(base, e=True, oj="xyz", secondaryAxisOrient="zup", zso=True)


output = cmds.duplicate(base, n="output", rc=True)[0]
outputEnd = cmds.listRelatives(output, children=True)[0]
for i in base, output:
	continue
	#cmds.joint(i, e=True, oj="xyz", secondaryAxisOrient="zup", zso=True)
#cmds.makeIdentity(base, apply=False, jo=True)

for i in "XYZ":
	cmds.setAttr(output + ".jointOrient" + i, 0)
	cmds.connectAttr(end + ".translate" + i,
	                 outputEnd + ".translate" + i)
generalIk = cmds.createNode("generalIk")

cmds.connectAttr( base + ".worldMatrix[0]", generalIk + ".joints[0].matrix")
cmds.connectAttr( base + ".jointOrient", generalIk + ".joints[0].orient")
cmds.connectAttr( base + ".jointOrient", output + ".jointOrient")
cmds.connectAttr( base + ".rotateOrder", output + ".rotateOrder")
cmds.connectAttr( base + ".rotateOrder", generalIk + ".joints[0].rotateOrder")
cmds.connectAttr( end + ".worldMatrix[0]", generalIk + ".endMatrix")
cmds.connectAttr( target + ".worldMatrix[0]", generalIk + ".targetMatrix")
cmds.connectAttr( up + ".worldMatrix[0]", generalIk + ".joints[0].upMatrix")
cmds.setAttr( generalIk + ".maxIterations", 1)

cmds.connectAttr( generalIk + ".outputArray[0].outputRotate",
                  output + ".rotate")
cmds.connectAttr( generalIk + ".outputArray[0].outputTranslate",
                  output + ".translate")

debugTarget = cmds.spaceLocator(n="debugOut")[0]
decomp = cmds.createNode("decomposeMatrix")
cmds.connectAttr(generalIk + ".debugTarget", decomp + ".inputMatrix")
cmds.connectAttr(decomp + ".outputTranslate", debugTarget + ".translate")
cmds.connectAttr(decomp + ".outputRotate", debugTarget + ".rotate")

#cmds.parent(end, world=True)
#cmds.hide(base)
cmds.setAttr(base + ".translateX", 1)
cmds.setAttr(end + ".translateZ", 2)


