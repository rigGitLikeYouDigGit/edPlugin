
maya_useNewAPI = True

def maya_useNewAPI():
	pass

import sys
import maya.api.OpenMaya as om

import math
import maya.cmds as cmds
from collections import namedtuple
from edPlugin.lib.python import nodeio

kPluginNodeName = "generalIk"
kPluginNodeId = om.MTypeId( 0xDAA1 )

# ChainData = namedtuple("ChainData", ["matrices"])

""" STUFF TO BE AWARE OF:
for the most part this is robust to dynamically changing proportions
and positions in the start chain - the 'outputEnd' translate and rotate
attributes are convenience only, and only pass through local transformations
of the original end matrix

- get inputs
- check if matrix chain is dirty
	- if yes: 
		- rebuild local chain from worlds
		- localise target and up matrices into ik space through the
		root world matrix
	- if no, reload cached local chain from inputs

- process local chain from root to tip
	- at tip joint, active space matrix is found through multiplying
	all precedent joints from root to tip
		- local target is ikSpace target, multiplied by inverse of this


terminology:
 - ikSpace: space with root joint of input chain as origin
 - activeSpace: space with currently mobile joint link as origin

"""


class generalIk(om.MPxNode):
	# define everything
	id = om.MTypeId( 0xDAA1)

	def __init__(self):
		om.MPxNode.__init__(self)

	def compute(self, pPlug, pData):

		# only compute if output is in out array
		if not pPlug.isChild:
			return
		if(pPlug.parent() == generalIk.aOutArray):
			# descend into coordinated cycles
			# inputs

			solver = pData.inputValue(generalIk.aSolver).asInt()
			maxIter = pData.inputValue(generalIk.aMaxIter).asInt()
			tolerance = pData.inputValue(generalIk.aTolerance).asDouble()

			# target
			targetMat = pData.inputValue(generalIk.aTargetMat).asMatrix()

			# end
			endMat = pData.inputValue(generalIk.aEndMat).asMatrix()

			# extract input world matrices from array then leave it alone
			inJntArrayDH = pData.inputArrayValue(generalIk.aJnts)
			inLength = inJntArrayDH.__len__()
			worldInputs = [None] * inLength
			orients = [None] * inLength
			ups = [None] * inLength
			for i in range(inLength):
				inJntArrayDH.jumpToPhysicalElement(i)
				childCompDH = inJntArrayDH.inputValue()
				worldInputs[i] = childCompDH.child(
					generalIk.aJntMat).asMatrix()
				ups[i] = childCompDH.child(
					generalIk.aJntUpMat).asMatrix()

				orients[i] = om.MEulerRotation(childCompDH.child(
					generalIk.aOrientRot).asDouble3())


			# from world inputs, reconstruct localised chain
			# remove joint orients, then reapply
			localMatrices, localUpMatrices = buildChain(
				worldInputs, orients, ups, length=inLength)

			# main loop
			n = 0
			tol = 100

			activeEnd = endMat

			targetMat = neutraliseRotations(targetMat)

			""" localise target into ikSpace """

			targetIkSpace = worldInputs[0].inverse() * targetMat
			targetIkSpace = targetMat

			results = localMatrices
			while n < maxIter and tol > tolerance:
				data = iterateChain(results, length=inLength,
				             targetMat=targetIkSpace, endMat=activeEnd,
				                       upMatrices=ups)
				results = data["results"]
				tol = data["tolerance"]
				activeEnd = data["end"]

				n += 1

			#activeEnd = activeEnd * worldInputs[0].inverse()

			spaceConstant = 1

			worldSpaceTarget = targetIkSpace * worldInputs[0]
			worldSpaceTarget = targetMat

			# outputs
			outDebugDH = pData.outputValue(generalIk.aDebugTarget)
			outDebugDH.setMMatrix(worldSpaceTarget)
			outDebugOffsetDH = pData.outputValue(generalIk.aDebugOffset)
			outDebugOffsetDH.setDouble(tol)

			# end transform
			endTfMat = om.MTransformationMatrix(activeEnd)
			#print("activeEnd {}".format(activeEnd))
			outEndTransDH = pData.outputValue(generalIk.aOutEndTrans)
			# translate = endTfMat.translation( spaceConstant )
			# print("translate {}".format(translate))
			outEndTransDH.set3Double( *endTfMat.translation( spaceConstant ) )

			# convert jntArray of matrices to useful rotation values
			outArrayDH = pData.outputArrayValue(generalIk.aOutArray)


			for i in range(inLength):
				outArrayDH.jumpToPhysicalElement(i)
				outCompDH = outArrayDH.outputValue()

				outRotDH = outCompDH.child(generalIk.aOutRot)
				outRxDH = outRotDH.child(generalIk.aOutRx)
				outRyDH = outRotDH.child(generalIk.aOutRy)
				outRzDH = outRotDH.child(generalIk.aOutRz)
				#

				# apply jointOrient
				outMat = om.MTransformationMatrix(
					results[i] ).rotateBy( orients[i].inverse(), spaceConstant )


				outRot = outMat.rotation()
				# unitConversions bring SHAME on family
				xAngle = om.MAngle(outRot[0])
				yAngle = om.MAngle(outRot[1])
				zAngle = om.MAngle(outRot[2])
				outRxDH.setMAngle( xAngle )
				outRyDH.setMAngle( yAngle )
				outRzDH.setMAngle( zAngle )

				outTranslate = (results[i][12], results[i][13], results[i][14])
				outTransDH = outCompDH.child(generalIk.aOutTrans)
				outTransDH.set3Double( *outTranslate )

			outArrayDH.setAllClean()


			pData.setClean(pPlug)

def buildChain(worldChain, orients, ups, length=1):
	""" reconstruct a chain of ordered local matrices
	from random world inputs"""

	chain = [None] * length # root to tip
	localUps = [None] * length
	for i in range(length):

		inMat = worldChain[i] # world matrices already account for orient


		""" we don't need local matrices
		we just need EVERYTHING ELSE localised into each world matrix
		then later we do need to construct the local chain though
		BUT world matrix will not be valid across iterations, 
		needs to be regenerated
		"""
		localMat = inMat
		localUps[i] = ups[i] #* inMat.inverse()
		chain[i] = localMat

	return chain, localUps

def iterateChain(localChain, tolerance=None, length=1,
                 targetMat=None, endMat=None, upMatrices=None):
	"""performs one complete iteration of the chain,
	may be possible to keep this solver-agnostic"""
	"""
	welcome to the bone zone
	# reconstruct hierarchy with matrices from previous iteration
	# currently target is known in rootspace, and end isn't known at all
	# backwards to get target and forwards to get end, both in active joint space
	#
	#                  +(target)
	#               .
	#             .
	#           O
	#         /   \       X(end)
	#       /       \   /
	#     /           O
	#   /
	# (root)
	# this works by getting vector from active joint to end, from active joint to target,
	# then aligning one to the other. joints are assumed to have direct parent-child
	# hierarchy, so all rotations are inherited rigidly
	"""
	# HERE we need knowledge of the live end position
	endMat = neutraliseRotations(endMat)

	for i in range(length):  # i from TIP TO ROOT
		index = - 1 - i
		inMat = localChain[ index ]
		upMat = upMatrices[ index ]

		localEnd = endMat * inMat.inverse()

		# find rotation of active joint to end, THEN
		# rotation from that to target


		quatMat = testLookAt( baseMat=inMat,
		                       endMat=endMat,
		                       targetMat=targetMat,
								factor=1.0)

		orientMat = inMat * quatMat
		#orientMat = inMat * quatMat

		rawMat = om.MMatrix(orientMat)


		# this all now works, taking account of end and target position
		# transfer original translate attributes to new matrix
		for i in range(12, 15):
			orientMat[i] = inMat[i]
		localChain[index] = orientMat

		"""output translation values for end must match exactly ref chain
		end must be multiplied out to ik space to find span to target"""

		# find current offset
		ikSpaceEnd = quatMat * localEnd
		ikSpaceTarget = quatMat * targetMat
		#ikSpaceEnd = localEnd
		endTargetVec = vectorBetweenMatrices(ikSpaceEnd, ikSpaceTarget)
		tolerance = endTargetVec.length()
		#print("tolerance {}".format(tolerance))
		endMat = localEnd


	return {
		"results" : localChain,
		"tolerance" : tolerance,
		"end" : endMat
	}

def neutraliseRotations(mat):
	""" return mmatrix containing only its original translations"""
	newMat = om.MMatrix()
	for i in range(16):
		if any( i == n for n in range(12, 16)):
			newMat[i] = mat[i]

	return newMat

def positionFromMatrix(mat):
	""" return only translation component of matrix as MVector"""
	return om.MVector( mat[12], mat[13], mat[14])

def testLookAt(baseMat, endMat, targetMat, factor=1.0):
	toEnd = vectorBetweenMatrices(baseMat, endMat).normalize()
	toTarget = vectorBetweenMatrices(baseMat, targetMat).normalize()
	return om.MQuaternion(toEnd, toTarget, factor).asMatrix()

def vectorBetweenMatrices(fromMat, toMat):
	return om.MVector( toMat[12] - fromMat[12],
	                   toMat[13] - fromMat[13],
	                   toMat[14] - fromMat[14] )

def lookAt(base, target, up = (0, 0.5, 0.5), upMat=None):
	""" NB : takes no account of initial base orientation
	only depends on vector from base to target"""
	# axes one by one, code shamelessly copied from somewhere
	# convert to quat someday?
	if upMat:
		up = positionFromMatrix(upMat).normalize()

	# x is vector between base and target
	x = vectorBetweenMatrices(base, target).normalize()

	z = x ^ om.MVector(up)
	z.normalize()
	y = x ^ z
	y.normalize()

	aim = om.MMatrix([
		y.x, y.y, y.z, 0,
		x.x, x.y, x.z, 0,
		z.x, z.y, z.z, 0,
		0, 0, 0, 1
	])
	return aim

def nodeInitializer():
	# create attributes

	# pick your pointy poison
	solverAttrFn = om.MFnEnumAttribute()
	generalIk.aSolver = solverAttrFn.create("solver", "sol", 0)
	solverAttrFn.addField("CCD", 0)
	solverAttrFn.addField("FABRIK (not yet implemented)", 1)
	solverAttrFn.storable = True
	solverAttrFn.keyable = True
	solverAttrFn.readable = False
	solverAttrFn.writable = True
	om.MPxNode.addAttribute(generalIk.aSolver)

	iterAttrFn = om.MFnNumericAttribute()
	generalIk.aMaxIter = iterAttrFn.create("maxIterations", "mi",
	                                       om.MFnNumericData.kLong, 30)
	iterAttrFn.storable = True
	iterAttrFn.keyable = True
	iterAttrFn.readable = False
	iterAttrFn.writable = True
	iterAttrFn.setMin(0)
	om.MPxNode.addAttribute(generalIk.aMaxIter)

	# how far will you go for perfection
	toleranceAttrFn = om.MFnNumericAttribute()
	generalIk.aTolerance = toleranceAttrFn.create("tolerance", "tol",
	                                              om.MFnNumericData.kDouble, 0.01)
	toleranceAttrFn.storable = True
	toleranceAttrFn.keyable = True
	toleranceAttrFn.readable = False
	toleranceAttrFn.writable = True
	toleranceAttrFn.setMin(0)
	om.MPxNode.addAttribute(generalIk.aTolerance)

	# what are your goals in life
	targetMatAttrFn = om.MFnMatrixAttribute()
	generalIk.aTargetMat = targetMatAttrFn.create("targetMatrix",
	                                              "targetMat", 1)
	targetMatAttrFn.storable = True
	targetMatAttrFn.readable = False
	targetMatAttrFn.keyable = False
	targetMatAttrFn.writable = True
	targetMatAttrFn.cached = True
	om.MPxNode.addAttribute(generalIk.aTargetMat)

	# compare and contrast
	endMatAttrFn = om.MFnMatrixAttribute()
	generalIk.aEndMat = endMatAttrFn.create("endMatrix", "endMat", 1)
	endMatAttrFn.storable = True
	endMatAttrFn.readable = False
	endMatAttrFn.keyable = False
	endMatAttrFn.writable = True
	endMatAttrFn.cached = True
	om.MPxNode.addAttribute(generalIk.aEndMat)


	# once i built a tower
	jntMatAttrFn = om.MFnMatrixAttribute()
	generalIk.aJntMat = jntMatAttrFn.create("matrix",
	                                        "jntMat", 1)
	jntMatAttrFn.storable = False
	jntMatAttrFn.writable = True
	jntMatAttrFn.cached = False # prevent ghost influences from staying
	# om.MPxNode.addAttribute(generalIk.aJntMat)

	# joint orients
	orientRxAttrFn = om.MFnUnitAttribute()
	generalIk.aOrientRx = orientRxAttrFn.create("orientX", "orientX", 1, 0.0)
	orientRxAttrFn.writable = True
	orientRxAttrFn.keyable = False

	orientRyAttrFn = om.MFnUnitAttribute()
	generalIk.aOrientRy = orientRyAttrFn.create("orientY", "orientY", 1, 0.0)
	orientRyAttrFn.writable = True
	orientRyAttrFn.keyable = False

	orientRzAttrFn = om.MFnUnitAttribute()
	generalIk.aOrientRz = orientRzAttrFn.create("orientZ", "orientZ", 1, 0.0)
	orientRzAttrFn.writable = True
	orientRzAttrFn.keyable = False

	orientRotAttrFn = om.MFnCompoundAttribute()
	generalIk.aOrientRot = orientRotAttrFn.create("orient", "orient")
	orientRotAttrFn.storable = False
	orientRotAttrFn.writable = True
	orientRotAttrFn.keyable = False
	orientRotAttrFn.addChild(generalIk.aOrientRx)
	orientRotAttrFn.addChild(generalIk.aOrientRy)
	orientRotAttrFn.addChild(generalIk.aOrientRz)

	# debug
	debugTargetFn = om.MFnMatrixAttribute()
	generalIk.aDebugTarget = debugTargetFn.create("debugTarget", "debugTarget", 1)
	om.MPxNode.addAttribute(generalIk.aDebugTarget)

	debugOffset = om.MFnNumericAttribute()
	generalIk.aDebugOffset = debugOffset.create("debugOffset", "debugOffset",
	                                            om.MFnNumericData.kDouble, 0)
	om.MPxNode.addAttribute(generalIk.aDebugOffset)

	# rotate order
	rotOrderAttrFn = om.MFnNumericAttribute()
	generalIk.aRotOrder = rotOrderAttrFn.create("rotateOrder", "rotateOrder",
	                                            om.MFnNumericData.kLong, 0)

	# eye on the sky
	jntUpMatAttrFn = om.MFnMatrixAttribute()
	generalIk.aJntUpMat = jntUpMatAttrFn.create("upMatrix",
	                                            "jntUpMat", 1)
	jntUpMatAttrFn.storable = True
	jntUpMatAttrFn.writable = True
	jntUpMatAttrFn.cached = True
	# om.MPxNode.addAttribute(generalIk.aJntUpMat)

	# who is the heftiest boi
	jntWeightAttrFn = om.MFnNumericAttribute()
	generalIk.aJntWeight = jntWeightAttrFn.create("weight", "jntWeight",
	                                              om.MFnNumericData.kFloat, 1)
	jntWeightAttrFn.storable = True
	jntWeightAttrFn.keyable = True
	jntWeightAttrFn.writable = True
	jntWeightAttrFn.setMin(0)
	# om.MPxNode.addAttribute(generalIk.aJntWeight)

	limitAttrFn = om.MFnCompoundAttribute()
	generalIk.aLimits = limitAttrFn.create("limits", "limits")

	# like really know them
	rxMaxAttrFn = om.MFnNumericAttribute()
	generalIk.aRxMax = rxMaxAttrFn.create("maxRotateX", "maxRx",
	                                      om.MFnNumericData.kFloat, 0)
	# how low can you go
	rxMinAttrFn = om.MFnNumericAttribute()
	generalIk.aRxMin = rxMinAttrFn.create("minRotateX", "minRx",
	                                      om.MFnNumericData.kFloat, 0)
	limitAttrFn.addChild(generalIk.aRxMax)
	limitAttrFn.addChild(generalIk.aRxMin)



	## there is more to be done here

	# you will never break the chain
	jntArrayAttrFn = om.MFnCompoundAttribute()
	generalIk.aJnts = jntArrayAttrFn.create("joints", "joints")
	jntArrayAttrFn.array = True
	jntArrayAttrFn.usesArrayDataBuilder = True
	jntArrayAttrFn.addChild(generalIk.aJntMat)
	jntArrayAttrFn.addChild(generalIk.aJntUpMat)
	jntArrayAttrFn.addChild(generalIk.aJntWeight)
	jntArrayAttrFn.addChild(generalIk.aOrientRot)
	jntArrayAttrFn.addChild(generalIk.aRotOrder)
	jntArrayAttrFn.addChild(generalIk.aLimits)
	# add limits later
	om.MPxNode.addAttribute(generalIk.aJnts)

	# fruits of labour
	outRxAttrFn = om.MFnUnitAttribute()
	generalIk.aOutRx = outRxAttrFn.create("outputRotateX", "outRx", 1, 0.0)
	outRxAttrFn.writable = False
	outRxAttrFn.keyable = False
	# om.MPxNode.addAttribute(generalIk.aOutRx)


	outRyAttrFn = om.MFnUnitAttribute()
	generalIk.aOutRy = outRyAttrFn.create("outputRotateY", "outRy", 1, 0.0)
	outRyAttrFn.writable = False
	outRyAttrFn.keyable = False
	# om.MPxNode.addAttribute(generalIk.aOutRy)

	outRzAttrFn = om.MFnUnitAttribute()
	generalIk.aOutRz = outRzAttrFn.create("outputRotateZ", "outRz", 1, 0.0)
	outRzAttrFn.writable = False
	outRzAttrFn.keyable = False
	# om.MPxNode.addAttribute(generalIk.aOutRz)

	outRotAttrFn = om.MFnCompoundAttribute()
	# generalIk.aOutRot = outRotAttrFn.create("outputRotate", "outRot",
	#     om.MFnNumericData.k3Double)
	generalIk.aOutRot = outRotAttrFn.create("outputRotate", "outRot")
	outRotAttrFn.storable = False
	outRotAttrFn.writable = False
	outRotAttrFn.keyable = False
	outRotAttrFn.addChild(generalIk.aOutRx)
	outRotAttrFn.addChild(generalIk.aOutRy)
	outRotAttrFn.addChild(generalIk.aOutRz)
	om.MPxNode.addAttribute(generalIk.aOutRot)


	# # add smooth jazz

	outTransAttrFn = om.MFnNumericAttribute()
	generalIk.aOutTrans = outTransAttrFn.create("outputTranslate", "outTrans",
	                                            om.MFnNumericData.k3Double)
	outTransAttrFn.storable = False
	outTransAttrFn.writable = False
	outTransAttrFn.keyable = False
	om.MPxNode.addAttribute(generalIk.aOutTrans)


	# all that the sun touches
	outArrayAttrFn = om.MFnCompoundAttribute()
	generalIk.aOutArray = outArrayAttrFn.create("outputArray", "out")
	outArrayAttrFn.array = True
	outArrayAttrFn.usesArrayDataBuilder = True
	outArrayAttrFn.storable = False
	outArrayAttrFn.writable = False
	outArrayAttrFn.keyable = False
	outArrayAttrFn.addChild(generalIk.aOutRot)
	outArrayAttrFn.addChild(generalIk.aOutTrans)
	om.MPxNode.addAttribute(generalIk.aOutArray)
	# investigate rolling this into the input hierarchy

	# convenience end attributes for babies
	outEndTransFn = om.MFnNumericAttribute()
	generalIk.aOutEndTrans = outEndTransFn.create(
		"outputEndTranslate", "outputEndTranslate", om.MFnNumericData.k3Double)
	outEndTransFn.writable = False
	om.MPxNode.addAttribute(generalIk.aOutEndTrans)

	outEndRotFn = om.MFnCompoundAttribute()
	generalIk.aOutEndRot = outEndRotFn.create(
		"outputEndRotate", "outputEndRotate"	)
	outEndRotFn.writable = False
	outEndRxAttrFn = om.MFnUnitAttribute()
	generalIk.aOutEndRx = outEndRxAttrFn.create("outputEndRotateX", "outEndRx", 1, 0.0)
	outEndRotFn.addChild(generalIk.aOutEndRx)
	outEndRyAttrFn = om.MFnUnitAttribute()
	generalIk.aOutEndRy = outEndRyAttrFn.create("outputEndRotateY", "outEndRy", 1, 0.0)
	outEndRotFn.addChild(generalIk.aOutEndRy)
	outEndRzAttrFn = om.MFnUnitAttribute()
	generalIk.aOutEndRz = outEndRzAttrFn.create("outputEndRotateZ", "outEndRz", 1, 0.0)
	outEndRotFn.addChild(generalIk.aOutEndRz)
	om.MPxNode.addAttribute(generalIk.aOutEndRot)


	# everyone's counting on you
	drivers = [generalIk.aTargetMat, generalIk.aEndMat, generalIk.aJnts]
	driven = [generalIk.aOutArray, generalIk.aOutEndTrans, generalIk.aOutEndRot,
	          generalIk.aDebugTarget, generalIk.aDebugOffset]

	nodeio.setAttributeAffects(drivers, driven, generalIk)



def nodeCreator():
	# creates node, returns to maya as pointer
	return generalIk()

def initializePlugin(mobject):
	mplugin = om.MFnPlugin(mobject)
	try:
		mplugin.registerNode( kPluginNodeName, kPluginNodeId,
		                      nodeCreator, nodeInitializer)
	except:
		sys.stderr.write("Failed to register node:" + kPluginNodeName)
		raise

def uninitializePlugin( mobject ):
	mPlugin = om.MFnPlugin(mobject)
	# try:
	# 	mPlugin.deregisterNode(kPluginNodeId)
	#
	# except:
	# 	sys.stderr.write("failed to unregister node, you're stuck with generalIk forever lol")
	# 	raise
	mPlugin.deregisterNode(kPluginNodeId)
