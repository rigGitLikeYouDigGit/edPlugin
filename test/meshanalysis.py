
from maya import cmds
from edPlugin.test import sureReloadPlugin

path = "edPush.mll"

def test():

	sureReloadPlugin(path)

	ball = cmds.polySphere()[0]
	analysis = cmds.createNode("meshAnalysis", n="testAnalysis")