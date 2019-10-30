
# augmenting pointOnCurve with a rotation-minimising frame,
# using the double reflection method
"""SORT the statpoint instability by internally extending the curve backwards
along its starting tangent, so that there is always a base that does not change
 - then compute the frame as normal"""
maya_useNewAPI = True


def maya_useNewAPI():
	pass


import sys
import maya.api.OpenMaya as om
# import maya.api.OpenMayaMPx as omMPx
import math
import maya.cmds as cmds
from edPlugin.lib.python import nodeio

kPluginNodeName = "rmfPci"
kPluginNodeId = om.MTypeId(0xDBD1)  # find way to replace with eyyyy


class rmfPci(om.MPxNode):
	# define everything
	id = om.MTypeId(0xDBD1)

	def __init__(self):
		om.MPxNode.__init__(self)
		self.internalCurve = om.MFnNurbsCurveData()

	def compute(self, pPlug, pData):

		bind = pData.inputValue(rmfPci.aBind).asShort()
		curveDH = pData.inputValue(rmfPci.aCurve)
		curveFn = om.MFnNurbsCurve(curveDH.asNurbsCurve())
		maxIter = int(pData.inputValue(rmfPci.aIter).asInt())
		u = pData.inputValue(rmfPci.aU).asDouble()
		# print "u is {}".format(u)
		# always turn on percentage
		curveU = curveFn.numSpans * u

		outPos = curveFn.getPointAtParam(curveU)
		# there is currently mismatch between point and orient because of
		# different curve lengths

		internalCurveData = om.MFnNurbsCurveData(
			pData.outputValue(rmfPci.aIntCurve).asNurbsCurve())

		internalDH = pData.outputValue(rmfPci.aIntCurve)
		intFn = om.MFnNurbsCurve(internalCurveData.object())

		# check that vector arrays are not calculated before doing anything
		obj = self.thisMObject()
		nodeFn = om.MFnDependencyNode(obj)
		normalPlug = nodeFn.findPlug("normals", False)  # don't want networked plugs
		tangentPlug = nodeFn.findPlug("tangents", False)

		if normalPlug.connectedTo(True, False) and \
				tangentPlug.connectedTo(True, False):  # asDest, not asSource
			# get vector array inputs
			print ""
			print "we are connected"
			normalDH = pData.inputValue(rmfPci.aNormals)
			normalData = om.MFnVectorArrayData(normalDH.data())
			normals = normalData.array()
			print "received normals {}".format(normals)
			# not receiving networked normals properly for some reason - oh well

			tangentDH = pData.inputValue(rmfPci.aTangents)
			tangentData = om.MFnVectorArrayData(tangentDH.data())
			tangents = tangentData.array()

		else:
			# do the full calculation

			# if bind and curvePlug.isDestination:
			# print "bind is {}".format(bind)
			if bind == 1:
				internalCurveDataObj = self.bind(pData, curveFn, internalCurveData)
				# print "int curve data is {}".format(internalCurveData)
				intFn = om.MFnNurbsCurve(internalCurveDataObj)

			# if not curveDH.isClean():
			# how to check if input is dirty or not?

			# update the internal curve
			self.updateInternal(pData, curveFn, intFn)

			# make the frame
			normals, tangents, binormals = self.makeRmf(maxIter, intFn)

		# print "normals are {}".format(normals)
		# print "tangents are {}".format(tangents)

		# set vector array values before we mutilate them
		normalDH = pData.outputValue(rmfPci.aNormals)
		normalData = om.MFnVectorArrayData(normalDH.data())
		normalData.set(normals)
		normalDH.setClean()

		tangentDH = pData.outputValue(rmfPci.aTangents)
		tangentData = om.MFnVectorArrayData(tangentDH.data())
		tangentData.set(tangents)
		tangentDH.setClean()

		# # cull the vector values lying before the true curve?
		realFraction = curveFn.numSpans / float(intFn.numSpans)
		cullFraction = (1 - realFraction) * maxIter
		for i in [normals, tangents]:
			for n in range(int(cullFraction) - 1):
				i.remove(0)

		normalValue = om.MVector(self.vecArrayInterpolate(normals, u))
		tangentValue = om.MVector(self.vecArrayInterpolate(tangents, u))
		binormalValue = normalValue ^ tangentValue

		resultMat = self.matrixFromVectors(tangentValue, normalValue, binormalValue)
		# set output plugs to matrix rotation
		outRot = om.MTransformationMatrix(resultMat).rotation()

		# # if vector arrays are connected, don't compute anything
		# # apart from interpolation between values
		outRotAll = pData.outputValue(rmfPci.aOutRot)
		# outRotAll.set3Float(*outRot)
		outRotAll.set3Double(*outRot)
		outRotAll.setClean()

		outRotX = pData.outputValue(rmfPci.aOutRx)
		outRotX.setMAngle(om.MAngle(outRot[0], 1))
		outRotY = pData.outputValue(rmfPci.aOutRy)
		outRotY.setMAngle(om.MAngle(outRot[1], 1))
		outRotZ = pData.outputValue(rmfPci.aOutRz)
		outRotZ.setMAngle(om.MAngle(outRot[2], 1))
		for i in [outRotX, outRotY, outRotZ]:
			i.setClean()

		outTAll = pData.outputValue(rmfPci.aOutTrans)

		# print "outPos is {}".format(outPos)
		# outTAll.set3Float(outPos[0], outPos[1], outPos[2])

		outTx = pData.outputValue(rmfPci.aOutTx)
		# outTx.setFloat(outPos[0])
		outTx.setDouble(outPos[0])
		outTx.setClean()

		outTy = pData.outputValue(rmfPci.aOutTy)
		# outTy.setFloat(outPos[1])
		outTy.setDouble(outPos[1])
		outTy.setClean()

		outTz = pData.outputValue(rmfPci.aOutTz)
		# outTz.setFloat(outPos[2])
		outTz.setDouble(outPos[2])
		outTz.setClean()

		pData.setClean(pPlug)

	def updateInternal(self, datablock, inputCrvFn, internalFn):
		# update the second three cvs of the internal curve
		# to match the live input
		# internalFn = om.MFnNurbsCurve(self.internalCurve.object())
		# internalData = datablock.outputValue(rmfPci.aIntCurve)
		inputBase, inputTan = inputCrvFn.getDerivativesAtParam(0)  # MPoint, MVector
		inputVec = om.MVector(inputBase)
		inputDegree = inputCrvFn.degree
		for i in range(inputDegree):
			point = -(i + 1.0) / 10.0 * inputTan + inputVec
			internalFn.setCVPosition(inputDegree + i, om.MPoint(point))

	# this should account properly for any degree

	def makeRmf(self, iterations, curveFn):
		"""returns samples of tangents, normals and binormals along a curve
		as MVectorArrays
		currently uses the double-projection method"""
		# curveFn = om.MFnNurbsCurve(curveData.object())
		# print "curveFn is {}".format(curveFn)
		# print "cvs are {}".format(curveFn.cvPositions())
		# print "iterations is {}".format(iterations)
		us = []
		# normals = []
		normals = om.MVectorArray()
		# tangents = []
		tangents = om.MVectorArray()
		binormals = []
		prevPos = None
		prevTan = None
		prevNormal = None
		intSpans = curveFn.numSpans
		intDegree = curveFn.degree

		for i in range(iterations):

			# print "i is {}".format(i)
			u = (float(i) / iterations) * intSpans  # + (2 * intDegree - 1)
			us.append(u)
			pos, tan = curveFn.getDerivativesAtParam(u)

			# print "u is {}".format(u)
			#
			# print "pos is {}".format(
			# 	curveFn.getPointAtParam(u, 2) #kObject
			# )
			# print "is param on curve? {}".format(
			# 	curveFn.isParamOnCurve(float(u))
			# )
			# print "normal is {}".format(
			# 	curveFn.normal(u)
			# )
			# derivs = curveFn.getDerivativesAtParam(u, 2, dUU=False) #kObject
			# print "derivs are {}".format(derivs)
			if i == 0:
				normal = tan ^ om.MVector(0, 1, 0)
			else:
				"""from the paper:
				r is normal
				v1 is vector from point i to point i+1
				c1 is dot product of v1 with itself??

				"""

				# first reflection is from previous position to current
				planeNormal = om.MVector(pos - prevPos)
				strangeC1 = planeNormal * planeNormal  # what is this??
				# results in left-handed coord frame
				leftNormal = prevNormal - (2.0 / strangeC1) * (planeNormal * prevNormal) * planeNormal
				leftTan = prevTan - (2.0 / strangeC1) * (planeNormal * prevTan) * planeNormal

				# second reflection
				plane2Normal = tan - leftTan
				strangeC2 = plane2Normal * plane2Normal
				normal = leftNormal - (2.0 / strangeC2) * (plane2Normal * leftNormal) * plane2Normal

				pass

			# mat = self.matrixFromVectors(tan, normal, binormal)
			binormal = tan ^ normal
			normals.append(normal)
			tangents.append(tan)
			binormals.append(binormal)

			prevPos = pos
			prevTan = tan
			prevNormal = normal
		return normals, tangents, binormals

	@staticmethod
	def matrixFromVectors(tangent, normal, binormal): # works
		return om.MMatrix([tangent.x, tangent.y, tangent.z, 0.0,
		                   normal.x, normal.y, normal.z, 0.0,
		                   binormal.x, binormal.y, binormal.z, 0.0,
		                   0.0, 0.0, 0.0, 0.0])

	@staticmethod
	def arrayInterpolate(array, lookup, vec=False):
		"""interpolates value from lookup fraction"""
		# print ""
		# print "array is {}".format(array)
		# print "lookup is {}".format(lookup)
		# print "len array is {}".format(len(array))
		if lookup >= 1.0:
			lookup = lookup / len(array)
		if lookup <= 0.0:
			result = array[0]
		elif lookup >= 1.0:
			result = array[-1]
		else:
			arrayFraction = len(array) * lookup
			# print "arrayFraction is {}".format(arrayFraction)

			lowest = int(math.floor(arrayFraction))
			# print "lowest is {}".format(lowest)
			highest = lowest + 1
			if highest > len(array) - 1:
				highest = len(array) - 1

			# print "highest is {}".format(highest)
			leftover = arrayFraction - lowest
			# print "leftover is {}".format(leftover)
			realLowest = array[lowest]
			# print "realLowest is {}".format(realLowest)
			realHighest = array[highest]
			# print "realHighest is {}".format(realHighest)
			result = realLowest + (realHighest - realLowest) * leftover
		# print "result is {}".format(result)
		return result

	@staticmethod
	def vecArrayInterpolate(vecArray, lookup):
		""""assumes vectors of dimension 3"""
		xs, ys, zs = [], [], []
		for i in range(len(vecArray)):
			xs.append(vecArray[i].x)
			ys.append(vecArray[i].y)
			zs.append(vecArray[i].z)

		xVal = rmfPci.arrayInterpolate(xs, lookup)
		yVal = rmfPci.arrayInterpolate(ys, lookup)
		zVal = rmfPci.arrayInterpolate(zs, lookup)

		return xVal, yVal, zVal

	def bind(self, dataBlock, curveFn, internalCurveData):
		"""extends current curve start back along its tangent, ensuring
		a static base to the curve regardless of user modification
		waste a small amount of computation, but it's trivial"""

		# get tangent at start of curve in order to extend properly
		basePoint, baseTan = curveFn.getDerivativesAtParam(0)  # MPoint, MVector

		curveCvs = curveFn.cvPositions()  # MPointArray of starting points
		curveDegree = curveFn.degree  # eventually take this into account for point generation
		curveSpans = curveFn.numSpans

		# add new points to cv array in direction of tangent
		addCvs = []
		for i in range(2 * curveDegree):
			addCvs.insert(0, basePoint + baseTan * -((i + 1) * 0.1))  # MVectors
		addPoints = om.MPointArray()
		for i in addCvs:
			point = om.MPoint(i)
			addPoints.append(point)

		newCvs = addPoints + curveCvs

		# knots
		curveKnots = curveFn.knots()  # MFloatArray
		targetLen = (curveSpans + 2 * curveDegree - 1)
		# print "current knots are {}".format(curveKnots)
		# print "current number knots is {}".format(len(curveKnots))
		# print "target number is {}".format(targetLen)

		# add 4 to existing array, and insert [0,0,0,1,2,3] at beginning
		for i in range(len(curveKnots)):
			curveKnots[i] += float(2 * curveDegree)

		for i in range(curveDegree):
			# chop off first zeroes
			curveKnots.remove(0)

		# for i in [3.0,2.0,1.0,0.0,0.0]:
		# 	curveKnots.insert(i, 0)
		endKnots = [0.0 for i in range(curveDegree)]
		# print "endZeroes are {}".format(endKnots)
		for i in range(2 * curveDegree):
			endKnots.insert(0, float(i + 1))

		for i in endKnots:
			curveKnots.insert(i, 0)

		# print "new knots are {}".format(curveKnots)
		# print "new length is {}".format(len(curveKnots))

		# we now have new points extending away from base of curve
		# second half of these will move with tangent, first half will
		# stay static

		test = om.MFnNurbsCurve()
		return test.create(newCvs, curveKnots, 3, 1, False,
		                   False, parent=internalCurveData.object())

# self.internalCurve = om.MFnNurbsCurveData(curveFn.object())


def nodeInitializer():
	# create attributes

	# don't take too much now
	iterAttrFn = om.MFnNumericAttribute()
	rmfPci.aIter = iterAttrFn.create("iterations", "i",
	                                 om.MFnNumericData.kLong, 30)
	iterAttrFn.storable = True
	iterAttrFn.keyable = True
	iterAttrFn.readable = True
	iterAttrFn.writable = True
	iterAttrFn.setMin(3)
	om.MPxNode.addAttribute(rmfPci.aIter)

	# HELLO MY BABY
	curveAttrFn = om.MFnTypedAttribute()
	rmfPci.aCurve = curveAttrFn.create("curve", "crv",
	                                   om.MFnNurbsCurveData.kNurbsCurve)
	om.MPxNode.addAttribute(rmfPci.aCurve)

	# internal extended curve, stored on master node
	intCurveAF = om.MFnTypedAttribute()
	intCurveData = om.MFnNurbsCurveData()
	# intCurveAF.storable = True
	# intCurveAF.readable = True
	rmfPci.aIntCurve = intCurveAF.create("intCurve", "intCrv",
	                                     om.MFnNurbsCurveData.kNurbsCurve, intCurveData.create())
	intCurveAF.writable = False
	intCurveAF.storable = True
	om.MPxNode.addAttribute(rmfPci.aIntCurve)

	# bind switch to create the internal curve
	nodeio.makeBindAttr(rmfPci, extras=None)

	# switch to use percentage for u lookup
	percentAttrFn = om.MFnNumericAttribute()
	rmfPci.aPercent = percentAttrFn.create(
		"turnOnPercentage", "top", om.MFnNumericData.kBoolean, 1)
	percentAttrFn.keyable = True
	om.MPxNode.addAttribute(rmfPci.aPercent)

	# create vector arrays to transmit and receive rmf function
	vAttrFn = om.MFnTypedAttribute()
	vArrayData = om.MFnVectorArrayData()
	rmfPci.aNormals = vAttrFn.create("normals", "ns", om.MFnData.kVectorArray,
	                                 vArrayData.create())
	om.MPxNode.addAttribute(rmfPci.aNormals)

	# create vector arrays to transmit and receive rmf function
	vTanArrayData = om.MFnVectorArrayData()
	rmfPci.aTangents = vAttrFn.create("tangents", "ts", om.MFnData.kVectorArray,
	                                  vTanArrayData.create())
	om.MPxNode.addAttribute(rmfPci.aTangents)

	# u lookup
	uAttrFn = om.MFnNumericAttribute()
	rmfPci.aU = uAttrFn.create("u", "u",
	                           om.MFnNumericData.kDouble, 0)
	uAttrFn.writable = True
	uAttrFn.keyable = True
	uAttrFn.setMin(0.0)
	uAttrFn.setMax(1.0)
	om.MPxNode.addAttribute(rmfPci.aU)

	# basic attributes returning the actual point information
	# rotate
	outRxAttrFn = om.MFnUnitAttribute()
	rmfPci.aOutRx = outRxAttrFn.create("outputRotateX", "outRx", 1, 0.0)
	outRxAttrFn.writable = False
	outRxAttrFn.keyable = False

	outRyAttrFn = om.MFnUnitAttribute()
	rmfPci.aOutRy = outRyAttrFn.create("outputRotateY", "outRy", 1, 0.0)
	outRyAttrFn.writable = False
	outRyAttrFn.keyable = False

	outRzAttrFn = om.MFnUnitAttribute()
	rmfPci.aOutRz = outRzAttrFn.create("outputRotateZ", "outRz", 1, 0.0)
	outRzAttrFn.writable = False
	outRzAttrFn.keyable = False

	outRotAttrFn = om.MFnCompoundAttribute()
	rmfPci.aOutRot = outRotAttrFn.create("outputRotate", "outRot")
	outRotAttrFn.storable = False
	outRotAttrFn.writable = False
	outRotAttrFn.keyable = False
	outRotAttrFn.addChild(rmfPci.aOutRx)
	outRotAttrFn.addChild(rmfPci.aOutRy)
	outRotAttrFn.addChild(rmfPci.aOutRz)
	om.MPxNode.addAttribute(rmfPci.aOutRot)

	# translate
	outTxAttrFn = om.MFnNumericAttribute()
	rmfPci.aOutTx = outTxAttrFn.create("outputTranslateX", "outTx",
	                                   om.MFnNumericData.kDouble, 0)

	outTyAttrFn = om.MFnNumericAttribute()
	rmfPci.aOutTy = outTyAttrFn.create("outputTranslateY", "outTy",
	                                   om.MFnNumericData.kDouble, 0)

	outTzAttrFn = om.MFnNumericAttribute()
	rmfPci.aOutTz = outTzAttrFn.create("outputTranslateZ", "outTz",
	                                   om.MFnNumericData.kDouble, 0)

	outTransAttrFn = om.MFnCompoundAttribute()
	rmfPci.aOutTrans = outTransAttrFn.create("outputTranslate", "outTrans")
	outTransAttrFn.storable = False
	outTransAttrFn.writable = False
	outTransAttrFn.keyable = False
	outTransAttrFn.addChild(rmfPci.aOutTx)
	outTransAttrFn.addChild(rmfPci.aOutTy)
	outTransAttrFn.addChild(rmfPci.aOutTz)
	om.MPxNode.addAttribute(rmfPci.aOutTrans)

	rmfPci.attributeAffects(rmfPci.aCurve, rmfPci.aOutTrans)
	rmfPci.attributeAffects(rmfPci.aCurve, rmfPci.aOutRot)
	rmfPci.attributeAffects(rmfPci.aCurve, rmfPci.aIntCurve)

	rmfPci.attributeAffects(rmfPci.aU, rmfPci.aOutTrans)
	rmfPci.attributeAffects(rmfPci.aU, rmfPci.aOutTx)
	rmfPci.attributeAffects(rmfPci.aU, rmfPci.aOutTy)
	rmfPci.attributeAffects(rmfPci.aU, rmfPci.aOutTz)
	rmfPci.attributeAffects(rmfPci.aU, rmfPci.aOutRot)
	rmfPci.attributeAffects(rmfPci.aU, rmfPci.aOutRx)
	rmfPci.attributeAffects(rmfPci.aU, rmfPci.aOutRy)
	rmfPci.attributeAffects(rmfPci.aU, rmfPci.aOutRz)

	rmfPci.attributeAffects(rmfPci.aBind, rmfPci.aOutTrans)
	rmfPci.attributeAffects(rmfPci.aBind, rmfPci.aOutRot)


def nodeCreator():
	# creates node, returns to maya as pointer
	return rmfPci()


def initializePlugin(mobject):
	mplugin = om.MFnPlugin(mobject)
	try:
		mplugin.registerNode(kPluginNodeName, kPluginNodeId,
		                     nodeCreator, nodeInitializer)
	except:
		sys.stderr.write("Failed to register node:" + kPluginNodeName)
		raise


def uninitializePlugin(mobject):
	mPlugin = om.MFnPlugin(mobject)
	try:
		mPlugin.deregisterNode(kPluginNodeId)
	except:
		sys.stderr.write("failed to unregister node, you're stuck with rmfPci forever lol")
		raise


"""reusable convenience methods for any plugin"""


def getOwnFn(mpx):
	"""returns a function set attached to the specific node being computed
	pass self from within compute"""
	return om.MFnDependencyNode(mpx.thisMObject())


def getOwnAttrObject(mpx, attr=""):
	"""returns attribute MObject for specific node being computed
	pass self from within compute"""
	fn = getOwnFn(mpx)
	return fn.findPlug(attr)  # MPlug


"""test script




pluginName = "rmfPci"
nodeName = "rmfPci"

cmds.file("F:/all projects desktop/common/edCode/edPlugin/{}TestScene.ma".format(nodeName), 
	o=True, f=True)

matAims = cmds.ls(type=nodeName)
cmds.delete(matAims)
cmds.flushUndo()
cmds.unloadPlugin(pluginName)
cmds.loadPlugin(pluginName)
aim = cmds.createNode(nodeName)

cmds.connectAttr("crv.local", aim+".curve")
for i in "XYZ":
    cmds.connectAttr(aim+".outputRotate"+i, "out.rotate"+i)
    cmds.connectAttr(aim+".outputTranslate"+i, "out.translate"+i)
cmds.connectAttr(aim+".intCurve", "intOut.create")

slave = cmds.createNode(nodeName)
cmds.connectAttr("crv.local", slave+".curve")
cmds.connectAttr(aim+".normals", slave+".normals")
cmds.connectAttr(aim+".tangents", slave+".tangents")
cmds.connectAttr(slave+".outputRotate", "child.rotate")
cmds.connectAttr(slave+".outputTranslate", "child.translate")



"""
