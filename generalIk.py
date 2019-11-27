
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

debugOn = 0
def debug(n=None, var=None):
	if debugOn:
		if not n and not var:
			print
		else:
			print("{} {}".format(n, var))



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
			globalWeight = pData.inputValue(generalIk.aGlobalWeight).asDouble()

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
			jointData = [None] * inLength # ARRAY OF STRUCTS REEEEEEEE
			for i in range(inLength):
				inJntArrayDH.jumpToPhysicalElement(i)
				childCompDH = inJntArrayDH.inputValue()
				worldInputs[i] = childCompDH.child(
					generalIk.aJntMat).asMatrix()
				ups[i] = childCompDH.child(
					generalIk.aJntUpMat).asMatrix()

				orients[i] = om.MEulerRotation(childCompDH.child(
					generalIk.aOrientRot).asDouble3())

				# extra data
				weight = childCompDH.child(generalIk.aJntWeight).asDouble()
				upDir = childCompDH.child(generalIk.aJntUpDir).asDouble3()
				jointData[i] = {
					"weight" : weight,
					"upDir" : upDir,
				}


			# from world inputs, reconstruct localised chain
			# remove joint orients, then reapply
			chainData = buildChains(
				worldInputs, orients, ups, length=inLength)
			localMatrices = chainData["localMatrices"]
			localUpMatrices = chainData["localUpMatrices"]
			ikSpaceMatrices = chainData["ikSpaceMatrices"]
			ikSpaceUpMatrices = chainData["ikSpaceUpMatrices"]
			# print("localMatrices {}".format(localMatrices))
			# print("ikMatrices {}".format(ikSpaceMatrices))
			# ikSpace and local matrices are correct

			# extract cached matrices from previous graph evaluation
			cacheMatrices = pData.inputValue(generalIk.aCacheMatrices)
			cacheArray = om.MFnMatrixArrayData(cacheMatrices.data()).array()

			if len(cacheArray) != inLength:
				print "cache length different, invalid"
				cacheArray.clear()
				for n in localMatrices : cacheArray.append(n)
			# for now check translations are valid -
			# this will need more complexity
			if any( positionFromMatrix(j) != positionFromMatrix(k)
			        for j, k in zip(localMatrices, cacheArray)):
				cacheArray.clear()
				for n in localMatrices : cacheArray.append(n)


			""" we cannot just check if the input matrices are dirty - 
			since they all take the world matrix, dirty does not mean modified """

			""" I don't think there is any point in treating the end
			matrix separately - it only adds a special case at every turn
			
			refactor to include it in normal local matrices
			
			"""

			# main loop
			n = 0
			tol = 100

			#endIkSpace = worldInputs[0].inverse() * endMat
			endIkSpace = endMat * worldInputs[0].inverse()
			# print "initial endIkSpace {}".format(
			# 	(endIkSpace[12], endIkSpace[13], endIkSpace[14]) )
			# endIkSpace is correct

			targetMat = neutraliseRotations(targetMat)
			endLocalSpace = endMat * worldInputs[-1].inverse()

			""" localise target into ikSpace """
			#targetIkSpace = worldInputs[0].inverse() * targetMat
			targetIkSpace =  targetMat * worldInputs[0].inverse()
			#print("targetIkSpace {}".format( (
			# 	targetIkSpace[12], targetIkSpace[13], targetIkSpace[14])
			# ))
			# targetIkSpace is correct

			results = localMatrices
			while n < maxIter and tol > tolerance:
				debug()
				debug("n", n)
				data = iterateChainCCD(
					worldMatrices=worldInputs,
					ikSpaceMatrices=ikSpaceMatrices,
					localMatrices=localMatrices,
					length=inLength,
					targetMat=targetIkSpace,
					endMat=endIkSpace,
					localEndMat=endLocalSpace,
					upMatrices=ups,
					jointData=jointData,
					globalWeight=globalWeight,
					ikSpaceUpMatrices=ikSpaceUpMatrices,
					tolerance=tolerance,
				)
				localMatrices = data["results"]

				debug("results", localMatrices)

				tol = data["tolerance"]
				endIkSpace = data["end"]
				targetMat = data["target"]

				n += 1

				if tol < tolerance:
					print("found peace on pass {}".format(n))
					break

			results = localMatrices

			#worldSpaceTarget = worldInputs[0] * targetIkSpace # correct
			worldSpaceTarget = worldInputs[0] * targetMat

			ikSpaceOutputs = [
				multiplyMatrices(localMatrices[:i + 1]) for i in range(inLength)]
			#print("ikSpaceOutputs {}".format(ikSpaceOutputs))
			endLocalSpace = endIkSpace * ikSpaceOutputs[-1].inverse()

			# print("endLocalSpace {}".format(
			# 	( endLocalSpace[12], endLocalSpace[13], endLocalSpace[14] ) ))

			# restore world space root position
			results[0] = results[0] * worldInputs[0]


			# outputs

			spaceConstant = 1

			outDebugDH = pData.outputValue(generalIk.aDebugTarget)
			outDebugDH.setMMatrix(worldSpaceTarget)
			outDebugOffsetDH = pData.outputValue(generalIk.aDebugOffset)
			outDebugOffsetDH.setDouble(tol)

			# end transform
			endTfMat = om.MTransformationMatrix(endLocalSpace)
			outEndTransDH = pData.outputValue(generalIk.aOutEndTrans)
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

			# save cached matrices for next evaluation
			cacheArray.clear()
			for i in localMatrices:
				cacheArray.append(i)

			outArrayDH.setAllClean()

			pData.setClean(pPlug)

def buildChains(worldChain, orients, ups, length=1):
	""" reconstruct a chain of ordered local matrices
	from random world inputs"""

	chain = [None] * length # root to tip
	localUps = [None] * length
	ikSpaceChain = [None] * length
	ikSpaceUps = [None] * length
	for i in range(length):

		inMat = worldChain[i] # world matrices already account for orient

		if i:
			#localMat = worldChain[ i - 1].inverse() * inMat
			localMat = inMat * worldChain[ i - 1].inverse()
		else:
			localMat = om.MMatrix()
		localUps[i] = ups[i] #* inMat.inverse()
		chain[i] = localMat # matrix TO index FROM previous
		ikSpaceChain[i] = inMat * worldChain[0].inverse()
		ikSpaceUps[i] = ups[i] * worldChain[0].inverse()

	return {
		"localMatrices" : chain,
		"localUpMatrices" : localUps,
		"ikSpaceMatrices" : ikSpaceChain,
		"ikSpaceTarget" : None,
		"ikSpaceUpMatrices" : ikSpaceUps
	}


def multiplyMatrices(mats, reverse=False):
	out = om.MMatrix()
	for i in mats:
		if reverse:
			out = out * i
		else:
			out = i * out
	return out

def iterateChainCCD(worldMatrices=None,
                    ikSpaceMatrices=None,
                    localMatrices=None,
                    tolerance=None, length=1,
                    targetMat=None, endMat=None,
                    localEndMat=None, upMatrices=None,
                    jointData=None, globalWeight=None,
                    ikSpaceUpMatrices=None):
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

	localEnd = localEndMat


	debug("localEnd", localEnd)
	debug("endMat", endMat)

	step = 0 # check iteration order
	d = 1000

	#print("localMatrices {}".format(localMatrices))

	for i in range(length):  # i from TIP TO ROOT
		step += 1
		index = length - 1 - i
		data = jointData[index]

		# print
		debug("index {}, step {}".format(index, step))


		# matrices from root to index
		toIndex = localMatrices[ :index + 1 ]
		#print( "toIndex {}".format(toIndex))
		activeMat = multiplyMatrices( toIndex )

		#print( "activeMat {}".format(activeMat))

		# matrices to end from index
		toEnd = localMatrices[ index+1: ]
		toEndMat = localEnd * multiplyMatrices( toEnd, reverse=False )

		# # info from previous iteration
		oldMat = om.MMatrix(localMatrices[index])
		# oldRot = oldMat
		oldRot = neutraliseTranslations(oldMat)
		# previous quaternion
		oldQuat = om.MQuaternion()
		oldQuat.setValue(oldRot)
		# print "oldQuat {}".format(oldQuat)


		# localise end, target and upMatrix
		#activeEnd = endMat * activeMat.inverse()
		activeEnd = toEndMat
		activeTarget = targetMat * activeMat.inverse()
		activeUp = ikSpaceUpMatrices[ index ] * activeMat.inverse()

		debug("activeEnd", positionFromMatrix(activeEnd))
		debug("activeTarget", positionFromMatrix(activeTarget))
		# # indentity quat is being set, not multiplied

		# # process upVector
		# upDir = jointData[index]["upDir"]
		# if upDir == (0, 0, 0):
		# 	upDir = (0, 1, 0)
		# upVector = positionFromMatrix(activeUp)
		# #dot = upVector * (activeMat * om.MVector(upDir) )
		# dot = upVector * ( om.MVector(upDir) * activeMat )


		# aim from end to target
		aimQuat = testLookAt( baseMat=om.MMatrix(),
		                      #baseMat=activeMat,
		                       endMat=activeEnd,
		                       targetMat=activeTarget,
								factor=1.0,
		                    )
		#aimQuat = aimQuat * oldQuat
		aimQuat = oldQuat * aimQuat
		#debug("aimQuat", aimQuat)

		# don't breathe this
		weight = min(data["weight"] * globalWeight, 0.999)
		#weight = data["weight"]

		# print("oldQuat {}".format(oldQuat))
		# print("weight {}".format(weight))


		outQuat = om.MQuaternion.slerp( aimQuat, aimQuat, weight, spin=0)

		weightMat = outQuat.asMatrix()

		debug("weightMat", weightMat)



		""" HERE is where we apply constraints, weight blending etc"""



		#orientMat = outQuat.asMatrix() * oldRot
		#orientMat = outQuat.asMatrix()
		orientMat = weightMat# * oldRot


		# # transfer original translate attributes to new matrix
		for n in range(12, 15):
			orientMat[n] = localMatrices[index][n]


		localMatrices[index] = orientMat


		ikSpaceEnd = toEndMat * orientMat #* activeEnd


		# print("ikSpaceEnd final {}".format(ikSpaceEnd))

		""" with end and target now in ikSpace, calculate offset """

		endTargetVec = vectorBetweenMatrices(ikSpaceEnd, targetMat)
		d = endTargetVec.length()
		#print("tolerance {}".format(tolerance))
		endMat = ikSpaceEnd

		ikChainEnd = multiplyMatrices(localMatrices)

		#localEndMat = ikChainEnd.inverse() * endMat
		localEndMat = endMat * ikChainEnd.inverse()

		if d < tolerance:
			break

	tolerance = d

	return {
		"results" : localMatrices,
		"tolerance" : tolerance,
		"end" : endMat,
		"target" : targetMat,
		"localEnd" : localEndMat
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

def neutraliseTranslations(mat):
	newMat = om.MMatrix(mat)
	for i in range(12, 15):
		newMat[i] = 0
	return newMat

def testLookAt(baseMat, endMat, targetMat, factor=1.0):
	toEnd = vectorBetweenMatrices(baseMat, endMat).normalize()
	toTarget = vectorBetweenMatrices(baseMat, targetMat).normalize()
	return om.MQuaternion(toEnd, toTarget, factor) # .asMatrix()

def vectorBetweenMatrices(fromMat, toMat):
	return om.MVector( toMat[12] - fromMat[12],
	                   toMat[13] - fromMat[13],
	                   toMat[14] - fromMat[14] )


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
	                                              om.MFnNumericData.kDouble, 0.1)
	toleranceAttrFn.storable = True
	toleranceAttrFn.keyable = True
	toleranceAttrFn.readable = False
	toleranceAttrFn.writable = True
	toleranceAttrFn.setMin(0)
	om.MPxNode.addAttribute(generalIk.aTolerance)

	# weight of the world
	globalWeightAttrFn = om.MFnNumericAttribute()
	generalIk.aGlobalWeight = globalWeightAttrFn.create("globalWeight", "globalWeight",
	                                              om.MFnNumericData.kDouble, 0.8)
	globalWeightAttrFn.writable = True
	globalWeightAttrFn.keyable = True
	om.MPxNode.addAttribute(generalIk.aGlobalWeight)

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
	generalIk.aEndMat = endMatAttrFn.create("inputEndMatrix", "endMat", 1)
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

	orientRyAttrFn = om.MFnUnitAttribute()
	generalIk.aOrientRy = orientRyAttrFn.create("orientY", "orientY", 1, 0.0)

	orientRzAttrFn = om.MFnUnitAttribute()
	generalIk.aOrientRz = orientRzAttrFn.create("orientZ", "orientZ", 1, 0.0)

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

	# but which way is up
	jntUpDirAttrFn = om.MFnNumericAttribute()
	generalIk.aJntUpDir = jntUpDirAttrFn.create("upDir", "upDir",
	                                            om.MFnNumericData.k3Double)

	# who is the heftiest boi
	jntWeightAttrFn = om.MFnNumericAttribute()
	generalIk.aJntWeight = jntWeightAttrFn.create("weight", "jntWeight",
	                                              om.MFnNumericData.kDouble, 1)
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
	                                      om.MFnNumericData.kDouble, 0)
	# how low can you go
	rxMinAttrFn = om.MFnNumericAttribute()
	generalIk.aRxMin = rxMinAttrFn.create("minRotateX", "minRx",
	                                      om.MFnNumericData.kDouble, 0)
	limitAttrFn.addChild(generalIk.aRxMax)
	limitAttrFn.addChild(generalIk.aRxMin)



	## there is more to be done here

	# you will never break the chain
	jntArrayAttrFn = om.MFnCompoundAttribute()
	generalIk.aJnts = jntArrayAttrFn.create("inputJoints", "inputJoints")
	jntArrayAttrFn.array = True
	jntArrayAttrFn.usesArrayDataBuilder = True
	jntArrayAttrFn.addChild(generalIk.aJntMat)
	jntArrayAttrFn.addChild(generalIk.aJntUpMat)
	jntArrayAttrFn.addChild(generalIk.aJntUpDir)
	jntArrayAttrFn.addChild(generalIk.aJntWeight)
	jntArrayAttrFn.addChild(generalIk.aOrientRot)
	jntArrayAttrFn.addChild(generalIk.aRotOrder)
	jntArrayAttrFn.addChild(generalIk.aLimits)
	# add limits later
	om.MPxNode.addAttribute(generalIk.aJnts)

	# fruits of labour
	outRxAttrFn = om.MFnUnitAttribute()
	generalIk.aOutRx = outRxAttrFn.create("rotateX", "outRx", 1, 0.0)
	outRxAttrFn.writable = False
	outRxAttrFn.keyable = False
	# om.MPxNode.addAttribute(generalIk.aOutRx)


	outRyAttrFn = om.MFnUnitAttribute()
	generalIk.aOutRy = outRyAttrFn.create("rotateY", "outRy", 1, 0.0)
	outRyAttrFn.writable = False
	outRyAttrFn.keyable = False
	# om.MPxNode.addAttribute(generalIk.aOutRy)

	outRzAttrFn = om.MFnUnitAttribute()
	generalIk.aOutRz = outRzAttrFn.create("rotateZ", "outRz", 1, 0.0)
	outRzAttrFn.writable = False
	outRzAttrFn.keyable = False
	# om.MPxNode.addAttribute(generalIk.aOutRz)

	outRotAttrFn = om.MFnCompoundAttribute()
	# generalIk.aOutRot = outRotAttrFn.create("outputRotate", "outRot",
	#     om.MFnNumericData.k3Double)
	generalIk.aOutRot = outRotAttrFn.create("rotate", "outRot")
	outRotAttrFn.storable = False
	outRotAttrFn.writable = False
	outRotAttrFn.keyable = False
	outRotAttrFn.addChild(generalIk.aOutRx)
	outRotAttrFn.addChild(generalIk.aOutRy)
	outRotAttrFn.addChild(generalIk.aOutRz)
	om.MPxNode.addAttribute(generalIk.aOutRot)



	# # add smooth jazz

	outTransAttrFn = om.MFnNumericAttribute()
	generalIk.aOutTrans = outTransAttrFn.create("translate", "outTrans",
	                                            om.MFnNumericData.k3Double)
	outTransAttrFn.storable = False
	outTransAttrFn.writable = False
	outTransAttrFn.keyable = False
	om.MPxNode.addAttribute(generalIk.aOutTrans)


	# all that the sun touches
	outArrayAttrFn = om.MFnCompoundAttribute()
	generalIk.aOutArray = outArrayAttrFn.create("outputJoints", "out")
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


	# debug
	debugTargetFn = om.MFnMatrixAttribute()
	generalIk.aDebugTarget = debugTargetFn.create("debugTarget", "debugTarget", 1)
	om.MPxNode.addAttribute(generalIk.aDebugTarget)

	debugOffset = om.MFnNumericAttribute()
	generalIk.aDebugOffset = debugOffset.create("debugOffset", "debugOffset",
	                                            om.MFnNumericData.kDouble, 0)
	om.MPxNode.addAttribute(generalIk.aDebugOffset)

	# caching results to persist across graph evaluations
	cacheMatricesFn = om.MFnTypedAttribute()
	matrixArrayData = om.MFnMatrixArrayData().create()
	generalIk.aCacheMatrices = cacheMatricesFn.create(
		"cacheMatrices", "cacheMatrices", 12, matrixArrayData ) # matrix array
	cacheMatricesFn.writable = True
	cacheMatricesFn.readable = True
	cacheMatricesFn.cached = True
	om.MPxNode.addAttribute( generalIk.aCacheMatrices )



	# everyone's counting on you
	drivers = [generalIk.aTargetMat, generalIk.aEndMat, generalIk.aJnts,
	           generalIk.aMaxIter, generalIk.aGlobalWeight, generalIk.aTolerance,
	           generalIk.aJntWeight]
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
