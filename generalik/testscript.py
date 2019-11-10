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
cmds.setAttr(end + ".translateY", 5)
cmds.parent(end, base)
#cmds.makeIdentity(base, apply=True, jo=True)
cmds.joint(base, e=True, oj="xyz", secondaryAxisOrient="zup")

output = cmds.duplicate(base, n="output", rc=True)[0]
for i in "XYZ":
	cmds.setAttr(output + ".jointOrient" + i, 0)
generalIk = cmds.createNode("generalIk")

cmds.connectAttr( base + ".worldMatrix[0]", generalIk + ".joints[0].matrix")
cmds.connectAttr( base + ".jointOrient", generalIk + ".joints[0].orient")
cmds.connectAttr( end + ".worldMatrix[0]", generalIk + ".endMatrix")
cmds.connectAttr( target + ".worldMatrix[0]", generalIk + ".targetMatrix")
cmds.setAttr( generalIk + ".maxIterations", 1)

cmds.connectAttr( generalIk + ".outputArray[0].outputRotate",
                  output + ".rotate")
cmds.connectAttr( generalIk + ".outputArray[0].outputTranslate",
                  output + ".translate")
cmds.hide(base)

# template = None
# output = None
# for i in range(1): # start
# 	template = cmds.createNode("joint", n="template{}_jnt".format(i))
# 	output = cmds.createNode("joint", n="output{}_jnt".format(i))