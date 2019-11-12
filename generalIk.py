
maya_useNewAPI = True

def maya_useNewAPI():
	pass

import sys
import maya.api.OpenMaya as om

import math
import maya.cmds as cmds
from collections import namedtuple

kPluginNodeName = "generalIk"
kPluginNodeId = om.MTypeId( 0xDAA1 )

# ChainData = namedtuple("ChainData", ["matrices"])

class generalIk(om.MPxNode):
	# define everything
	id = om.MTypeId( 0xDAA1)

	def __init__(self):
		om.MPxNode.__init__(self)

	def compute(self, pPlug, pData):

		# only compute if output is in out array
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

				# orient = childCompDH.child(
				# 	generalIk.aOrientRot).asDouble3()
				# orients[i] = om.MEulerRotation(
				# 	[math.degrees(j) for j in orient] )
				# seems to convert automatically, makes no difference

					# list of euler transforms
					# add rotateOrder support here

				orients[i] = om.MEulerRotation(childCompDH.child(
					generalIk.aOrientRot).asDouble3())



			# from world inputs, reconstruct localised chain
			# remove joint orients, then reapply
			localMatrices = buildChain(worldInputs, orients, length=inLength)

			# main loop
			n = 0
			tol = 100
			# localise end first
			endLocalMat = endMat * worldInputs[-1].inverse()
			#endLocalMat = endMat

			targetLocalMat = targetMat * worldInputs[0].inverse()
			#targetLocalMat = targetMat


			results = localMatrices
			while n < maxIter and tol > tolerance:
				results = iterateChain(results, length=inLength,
				             targetMat=targetLocalMat, endMat=endLocalMat,
				                       upMatrices=ups)

				n += 1


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
				# outMat = om.MTransformationMatrix(
				# 	results[i] ).rotateBy( orients[i].inverse(), 4 )

				""" NB : jointOrient does not distort the
				current world matrix - 
				it distorts its *children*
				"""

				outMat = om.MTransformationMatrix(results[i])

				outRotVals = outMat.rotation()
				# unitConversions bring SHAME on family
				xAngle = om.MAngle(outRotVals[0])
				yAngle = om.MAngle(outRotVals[1])
				zAngle = om.MAngle(outRotVals[2])

				outRxDH.setMAngle( xAngle )
				outRyDH.setMAngle( yAngle )
				outRzDH.setMAngle( zAngle )

			outArrayDH.setAllClean()


			pData.setClean(pPlug)

def buildChain(worldChain, orients, length=1):
	""" reconstruct a chain of ordered local matrices
	from random world inputs"""

	chain = [None] * length # root to tip
	for i in range(length):

		# inMat = om.MTransformationMatrix(
		# 	worldChain[i] ).rotateBy( orients[i], 1 ).asMatrix()
		inMat = worldChain[i] # world matrices already account for orient

		if i == 0:
			localMat = inMat
		else:
			print("calculating inverse")
			localMat = inMat * worldChain[ i-1 ].inverse()
		chain[i] = localMat
	return chain

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

	for i in range(length):  # i from TIP TO ROOT
		index = length - 1 - i
		inMat = localChain[ index ]
		upMat = upMatrices[ index ]

		endMat = endMat * inMat.inverse()

		# find rotation of active joint to end, THEN
		# rotation from that to target
		#print "endMat {}".format(endMat)
		endOrient = lookAt(inMat, endMat, upMat=upMat)


		orientMat = endOrient.inverse() *\
		            lookAt(inMat, targetMat, upMat=upMat) \

		# orientMat = lookAt(inMat, targetMat).inverse() * endOrient
		# orientMat = endOrient * lookAt(inMat, targetMat)
		# orientMat = lookAt(inMat, targetMat, upMat=upMat)

		# orientMat = testLookAt(baseMat=inMat, endMat=endMat,
		#                        targetMat=targetMat)

		#orientMat = endOrient
		# this all now works, taking account of end and target position

		localChain[index] = orientMat
	return localChain

def testLookAt(baseMat, endMat, targetMat, factor=1.0):
	toEnd = vectorBetweenMatrices(baseMat, endMat)
	toTarget = vectorBetweenMatrices(baseMat, targetMat)
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
		up = om.MVector( upMat[12] - base[12],
		                 upMat[13] - base[13],
		                 upMat[14] - base[14]).normalize()

	# x is vector between base and target
	x = om.MVector(target[12 ] -base[12],
	               target[13 ] -base[13],
	               target[14 ] -base[14]).normalize()


	z = x ^ om.MVector(up)
	z.normalize()
	y = x ^ z
	y.normalize()

	aim = om.MMatrix([
		#x.x, x.y, x.z, 0,
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
	targetMatAttrFn.writable = True
	targetMatAttrFn.cached = True
	om.MPxNode.addAttribute(generalIk.aTargetMat)

	# compare and contrast
	endMatAttrFn = om.MFnMatrixAttribute()
	generalIk.aEndMat = endMatAttrFn.create("endMatrix", "endMat", 1)
	endMatAttrFn.storable = True
	endMatAttrFn.readable = False
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

	# everyone's counting on you
	generalIk.attributeAffects(generalIk.aTargetMat, generalIk.aOutArray)
	generalIk.attributeAffects(generalIk.aMaxIter, generalIk.aOutArray)
	generalIk.attributeAffects(generalIk.aTolerance, generalIk.aOutArray)


	# following are reference chain - trigger rebuild only when these change
	# generalIk.attributeAffects(generalIk.aRootMat, generalIk.aOutArray)
	generalIk.attributeAffects(generalIk.aEndMat, generalIk.aOutArray)
	generalIk.attributeAffects(generalIk.aJnts, generalIk.aOutArray)

#     # TRY THIS OUT LATER:
#     refMatArrayFn = om.MFnMatrixAttribute()
#     generalIk.aRefArray = refMatArrayFn.create("refMatArray")
#     refMatArrayFn.array = True
#     refMatArrayFn.internal = True
#     refMatArrayFn.cached = True
#     refMatArrayFn.storable = True
#     # do we need arrayDataBuilder?
#     om.MPxNode.addAttribute(generalIk.aRefArray)
# this would be constructed of the joints' relative matrices, and then store the outputs
# of the node across iterations - basically its memory
# if the reference skeleton changes RELATIVE TO THE ROOT, this would need to be recalculated
# a battle for another day

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

# roadmap:
# get it working
# get it working with constraints
# find way to cache matrix chain unless reference chain rebuilds
# get different solvers working - maybe fabrik, but i want to try the quat splice
# rebuild in c++ if it really needs it?
# make cmd to attach joints automatically