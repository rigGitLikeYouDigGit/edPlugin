
""" trying to automate a bit of the code generation for plugin nodes
this is a basic approach, not live - something like cog would be more
powerful and make it more manageable to update all nodes at once,
but hopefully that won't be a frequent situation """

import sys, shutil, os, string

from edPlugin.templates import nodecpp, nodeh
from edPlugin import ROOT_PATH

def makeFiles( nodeName="newNode", nodeParentType="MPxNode", dirpath=None):

	nodeNameCaps = "".join( [i.capitalize() for i in nodeName] )
	nodeNameTitle = nodeName.capitalize()

	if nodeParentType == "MPxDeformerNode":
		mainMethod = nodeh.deformMethod
	else:
		mainMethod = nodeh.computeMethod


	hFormat = nodeh.baseH.format(nodeName=nodeName, nodeNameCaps=nodeNameCaps,
	                       nodeNameTitle=nodeNameTitle,
	                       nodeParent=nodeParentType,
	                       mainMethod=mainMethod)

	hPath = dirpath + "/" + nodeName + ".h"
	with open(hPath, mode="w+") as hFile:
		hFile.write( hFormat )


if __name__ == "__main__":

	""" run test backup """
	nodeTypes = (  # examples
		"MPxNode",
		"MPxDeformerNode",
	)
	outputPath = ROOT_PATH

	makeFiles( "testNode", nodeParentType="MPxNode", dirpath=outputPath)

