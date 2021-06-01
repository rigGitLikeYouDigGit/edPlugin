
import functools, itertools



testMap = {}

def testNodes(nodeNames):
	"""decorator to add test to test map"""
	
	def _wrap(fn):
		@functools.wraps(fn)
		def _inner(*args, **kwargs):
			result = fn(*args, **kwargs)
			return result
		return _inner
	return _wrap


