
"""
test python shader setup
"""
import maya.api.OpenMaya as om
import maya.api.OpenMayaRender as omr
import maya.cmds as cmds

# Using the Maya Python API 2.0.
def maya_useNewAPI():
    pass

pathToPlugin = 'F:/all_projects_desktop/common/edCode/edPlugin/'

def initializePlugin(mobject):
    # NOTICE:  Please set the path to the plugin before running




    plugin = om.MFnPlugin(mobject, "Autodesk", "3.0", "Any")
    plugin.registerNode(RenderOverrideOptions.kTypeName, RenderOverrideOptions.kTypeId, RenderOverrideOptions.creator, RenderOverrideOptions.initializer)

    fragmentRenderer = FragmentRenderOverride('GroundReflectionsOverride')
    omr.MRenderer.registerOverride(fragmentRenderer)

def uninitializePlugin(mobject):
    plugin = om.MFnPlugin(mobject)
    plugin.deregisterNode(apiMeshCreator.id)
    omr.MRenderer.unregisterOverride(fragmentRenderer)
    fragmentRenderer = None

