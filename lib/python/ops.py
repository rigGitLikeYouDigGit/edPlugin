""" common maths operations on maya datatypes """

import maya.api.OpenMaya as om
import math


def interpolateMMatrices(baseMat, targetMat, weight):
	""" blends between a base and target matrix by weight factor """
