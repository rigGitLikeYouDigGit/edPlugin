
"""test plugins from commandline """
import sys, os, subprocess, multiprocessing

def enclose(s, char="\""):
	return char + s + char


def createMayaProcess(exe, scriptPath):
	""" command args for maya """
	#flags = subprocess.CREATE_NEW_CONSOLE
	DETACHED_PROCESS = 0x00000008
	CREATE_NEW_PROCESS_GROUP = 0x00000200
	#flags = DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP

	# maya script to execute in console
	# clientPath = os.sep.join(__file__.split(os.sep)[:-2] +
	#                          ["clients", "maya.py"])
	cmd = [
		exe,
		"-command",
		"""python("execfile('{}')")""".format(
			#os.path.normpath(clientPath)
			scriptPath
		),
	]

	return subprocess.Popen(
		cmd,
		shell=True,
		stdin=subprocess.PIPE,
		stdout=subprocess.PIPE,
		stderr=subprocess.PIPE,
		creationflags=DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP
		#creationflags= CREATE_NEW_PROCESS_GROUP
	)

def main():

	startIn = "C:/Program Files/Autodesk/Maya2022/"
	#startIn = "C:/Program Files/Autodesk/Maya2022/bin/"

	exePath = '"C:/Program Files/Autodesk/Maya2022/bin/mayapy.exe"'

	scriptPath = "F:/all_projects_desktop/common/edCode/edPlugin/test/standalone.py"

	os.chdir(startIn)

	cmd = [exePath, scriptPath]
	cmd = " ".join(cmd)
	cmd = enclose(cmd)

	print(cmd)
	os.system(cmd)
	return


if __name__ == '__main__':
	main()




