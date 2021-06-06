

from maya import cmds
import unittest
from edPlugin.test.lib import MayaTest

scenePath = "F:/all_projects_desktop/common/edCode/edPlugin/scenes/tectonic_cube_test_v001.ma"


class TestTectonic(MayaTest):
	nodeTypes = ["tectonic", "tectonicConstraint"]


	def test_tectonic(self):
		"""hook up single constraint and mesh"""

		cmds.file(scenePath, open=1, f=1)

		solver = cmds.createNode("tectonic")

		for i, s in enumerate(("A", "B")):

			constraint = cmds.createNode("tectonicConstraint",
			                             n="constraint" + s)
			goalBase = "goal" + s + "_base"
			goalEnd = "goal" + s + "_end"


			cmds.connectAttr(goalBase + ".translate",
				constraint + ".goal[0].basePos")
			cmds.connectAttr(goalEnd + ".translate",
			                 constraint + ".goal[0].endPos")

			cmds.connectAttr(constraint + ".solver",
			                 solver + f".constraint[{i}]", f=1)

		baseMesh = "base_mesh"
		outMesh = "result_mesh"
		cutMesh = "cut_mesh"

		cmds.connectAttr(baseMesh + "Shape.outMesh",
		                 solver + ".inMesh")

		# test disconnection and reconnection
		cmds.disconnectAttr(baseMesh + "Shape.outMesh",
		                 solver + ".inMesh")
		cmds.connectAttr(baseMesh + "Shape.outMesh",
		                 solver + ".inMesh")

		cmds.connectAttr(baseMesh + "Shape.outMesh",
		                 solver + ".baseMesh")
		cmds.connectAttr(solver + ".outMesh", outMesh + "Shape.inMesh")
		cmds.connectAttr(solver + ".outCutMesh", cutMesh + "Shape.inMesh")

		# bind solver
		cmds.setAttr(solver + ".bind", 1)
		cmds.setAttr(solver + ".splitMode", 0)
		print(cmds.getAttr(solver + ".bind"))
		cmds.dgdirty(a=1)
		cmds.setAttr(solver + ".splitMode", 1)
		cmds.dgdirty(a=1)
		print(cmds.getAttr(solver + ".bind"))
		print(cmds.getAttr(solver + ".complete"))





		print("tectonic test finished")

		#cmds.delete(solver)


