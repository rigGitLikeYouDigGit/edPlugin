
from maya import cmds

def sureReloadPlugin(path):
	cmds.loadPlugin(path)
	cmds.file(new=True, f=True)

	cmds.unloadPlugin(path, f=True)
	#cmds.file(new=True, f=True)
	cmds.loadPlugin(path)
	#cmds.file(new=True, f=True)