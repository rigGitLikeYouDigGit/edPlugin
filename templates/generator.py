
""" trying to automate a bit of the code generation for plugin nodes
this is a basic approach, not live - something like cog would be more
powerful and make it more manageable to update all nodes at once,
but hopefully that won't be a frequent situation

stuff this WON'T do yet:
 - update cMakeLists.txt
 - regenerate cmake solutions, build files etc
 - update pluginMain.cpp


"""

import sys, shutil, os, string

from edPlugin.templates import nodecpp, nodeh
from edPlugin import ROOT_PATH

def makeFiles( nodeName="newNode", nodeParentType="MPxNode", dirpath=None,
               hObjectLines="", cObjectLines="", nodeDescription=""):

	nodeNameCaps = "".join( [i.capitalize() for i in nodeName] )
	nodeNameTitle = nodeName[0].upper() + nodeName[1:]

	# h file
	if nodeParentType == "MPxDeformerNode":
		mainMethod = nodeh.deformMethod
	else:
		mainMethod = nodeh.computeMethod
	# TEMP
	mTypeId = nodeh.MTypeId

	hFormat = nodeh.baseH.format(nodeName=nodeName, nodeNameCaps=nodeNameCaps,
	                       nodeNameTitle=nodeNameTitle,
	                       nodeParent=nodeParentType,
	                       mainMethod=mainMethod,
	                        MObjects=hObjectLines)

	hPath = dirpath + "/" + nodeName + ".h"
	with open(hPath, mode="w+") as hFile:
		hFile.write( hFormat )

	# cpp file
	cFormat = nodecpp.baseCpp.format(nodeName=nodeName, nodeNameCaps=nodeNameCaps,
	                             nodeNameTitle=nodeNameTitle,
	                             nodeParent=nodeParentType,
	                             mainMethod=mainMethod,
	                                 MTypeId=mTypeId,
	                                 MObjects=cObjectLines,
	                                 nodeDescription=nodeDescription,
	                                 )

	cPath = dirpath + "/" + nodeName + ".cpp"
	with open(cPath, mode="w+") as cFile:
		cFile.write(cFormat)


class NodeCode(object):
	""" small class to wrap code generation process """
	nodeTypes = (  # examples
		"MPxNode",
		"MPxDeformerNode",
	)
	dirpath = ROOT_PATH + "/src"

	def __init__(self,
	             nodeName="camelCaseName",
	             nodeType="MPxNode",
	             dirpath=None):
		assert nodeType in self.nodeTypes
		self.name = nodeName
		self.nodeType = nodeType
		self.dirpath = dirpath or self.dirpath

		self.attributeNames = []

		self.description = ""

	def addAttribute(self, name="aInput"):
		self.attributeNames.append(name)

	@property
	def nodeNameTitle(self):
		return self.name[0].upper() + self.name[1:]
	@property
	def nodeNameCaps(self):
		return "".join([i.capitalize() for i in self.name])

	# actual code gen
	def makeMObjectHLines(self):
		""" return the MObject lines as they should appear in .h file """
		lines = []
		template = nodeh.MObjectTemplate
		"""static MObject {MObjectName};
						"""
		for i in self.attributeNames:
			lines.append( template.format(MObjectName=i) )
		return "".join(lines)

	def makeMObjectCLines(self):
		""" code duplication in code generator lol """
		lines = []
		template = nodecpp.MObjectTemplate
		"""MObject {nodeNameTitle}::{MObjectName};
					"""
		for i in self.attributeNames:
			lines.append( template.format(
				nodeNameTitle=self.nodeNameTitle,
				MObjectName=i) )
		return "".join(lines)

	def write(self):
		""" makes new files
		in future protect the areas we know will never change against being
		overridden """
		makeFiles(nodeName=self.name,
		          nodeParentType=self.nodeType,
		          dirpath=self.dirpath,
		          hObjectLines=self.makeMObjectHLines(),
		          cObjectLines=self.makeMObjectCLines(),
		          nodeDescription=self.description
		          )

def build():
	project = """F:\all_projects_desktop\common\edCode\edPlugin\build\edPlugin.vcxproj"""
	# process = subprocess.Popen( cmd.split(" "),
	#                             stdout=subprocess.PIPE,
	#                             stderr=subprocess.PIPE)
	# while True:
	# 	output = process.stdout.readline().strip()
	# 	if output:
	# 		print(output)
	# 	returnCode = process.poll()
	# 	if returnCode is not None: # process finished
	# 		break
	# return returnCode


if __name__ == "__main__":

	""" run test backup """

	outputPath = ROOT_PATH

	#makeFiles( "uberDeformer", nodeParentType="MPxDeformerNode", dirpath=outputPath)
	#makeFiles( "meshToBuffers", nodeParentType="MPxNode", dirpath=outputPath)

	# meshToBuffers = NodeCode("meshToBuffers", nodeType="MPxNode")
	# meshToBuffers.description = """
	# converts maya mesh to raw float and int buffers of position and topo data
	# """
	# meshToBuffers.write()

	# uberDeformer = NodeCode("uberDeformer", nodeType="MPxDeformerNode")
	# uberDeformer.description = """
	# build deformation scheme by iterating over deformer notions
	# """
	# uberDeformer.write()

	deformerNotion = NodeCode("deformerNotion", nodeType="MPxNode")
	deformerNotion.description = """
	individual component deformation of uberDeformer
	"""
	deformerNotion.write()


