
"""towards a point-based iterative solver for arbitrarily-connected transforms,
later adding support for goals, curve connections and sdf collision
template in python before c++
with a suitably-built array you could also use this for ik
took me longer than i'm proud of to think of the name
"""
from __future__ import division, print_function
import maya.api.OpenMaya as om


maya_useNewAPI = True
def maya_useNewAPI():
	pass

kPluginNodeName = "tectonic"
kPluginNodeId = om.MTypeId(0xDBD1)  # find way to replace with eyyyy


class tectonic(om.MPxNode):
	# define everything
	id = om.MTypeId(0xDBD1)

