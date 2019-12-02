
"""attribute and data boilerplate likely to be common across nodes"""
from __future__ import division, print_function
import maya.api.OpenMaya as om


def makeBindAttr(nodeClass, name="bind", extras=None):
	""" create the default 'off, bind, bound' attr
	found on most nodes
	:param nodeClass : the node class to receive the attribute
	:type nodeClass : om.MPxNode
	:param extras : string list of bind options -
		'off, bind, bound' are always default and first
	"""
	bindFn = om.MFnEnumAttribute()
	nodeClass.aBind = bindFn.create(
		name, name, 1)
	extras = extras or []
	for i, val in enumerate(["off", "bind", "bound"] + extras):
		bindFn.addField( val, i)
	bindFn.keyable = True
	bindFn.hidden = False
	return nodeClass.aBind

def setAttributeAffects( driverList, drivenList, nodeObject ):
	for i in driverList:
		for j in drivenList:
			nodeObject.attributeAffects(i, j)
