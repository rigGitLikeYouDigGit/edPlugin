


"""file read directly on commandline startup"""


import sys, os
import logging

from edPlugin.test import main

if __name__ == '__main__':
	print("batchtest main")

	from maya import standalone

	standalone.initialize()

	from maya import cmds
	plugins = cmds.pluginInfo(q=1, pluginsInUse=1)
	# print("starting plugins:")
	# print(plugins)

	main.loadPlugin()
	main.runTests()




