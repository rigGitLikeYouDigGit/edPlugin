


"""file read directly on commandline startup"""


import sys, os
import logging

from edPlugin.test import main
from edPlugin.test.lib import MayaTest
from edPlugin.test.test_meshToBuffers import TestMeshToBuffers
import unittest


dirPath = "F:/all_projects_desktop/common/edCode/edPlugin/test"

if __name__ == '__main__':
	print("batchtest main")

	from maya import standalone

	standalone.initialize()

	from maya import cmds
	plugins = cmds.pluginInfo(q=1, pluginsInUse=1)

	main.loadPlugin()

	loader = unittest.TestLoader()
	suite = loader.discover(dirPath)
	runner = unittest.TextTestRunner()
	runner.run(suite)






