"""
Your flight departs in a few days from the coastal airport; the easiest way down to the coast from here is via toboggan.

The shopkeeper at the North Pole Toboggan Rental Shop is having a bad day. "Something's wrong with our computers; we can't log in!" You ask if you can take a look.

Their password database seems to be a little corrupted: some of the passwords wouldn't have been allowed by the Official Toboggan Corporate Policy that was in effect when they were chosen.

To try to debug the problem, they have created a list (your puzzle input) of passwords (according to the corrupted database) and the corporate policy when that password was set.

For example, suppose you have the following list:

1-3 a: abcde
1-3 b: cdefg
2-9 c: ccccccccc
Each line gives the password policy and then the password. The password policy indicates the lowest and highest number of times a given letter must appear for the password to be valid. For example, 1-3 a means that the password must contain a at least 1 time and at most 3 times.

In the above example, 2 passwords are valid. The middle password, cdefg, is not; it contains no instances of b, but needs at least 1. The first and third passwords are valid: they contain one a or nine c, both within the limits of their respective policies.

How many passwords are valid according to their policies?
"""


from edPlugin.adventofcode.a2020.inputs import day2

day2 = day2.split("\n")

def passAIsValid(char, minN, maxN, seq=""):
	""" given seq, find if seq contains at least minN
	and at most maxN character"""
	nOccurences = len(seq) - len(seq.translate(None, char))
	return minN <= nOccurences <=maxN

def passBIsValid(char, minN, maxN, seq=""):
	""" given seq, find if seq contains char at EITHER
	minN OR at maxN - minN=1 maps to seq index 0 """
	# minBool = (seq[minN - 1] == char)
	# maxBool = (seq[maxN - 1] == char)
	# result = (seq[minN - 1] == char) != (seq[maxN - 1] == char)
	# if not result:
	# 	print( seq[minN - 1], minBool, seq[maxN - 1], maxBool)
	# return result

	return (seq[minN - 1] == char) != (seq[maxN - 1] == char)


if __name__ == '__main__':
	validA = []
	validB = []
	for i in day2:
		# stage 1
		data, seq = i.split(":")
		seq = seq.strip()
		char = data[-1]
		minN, maxN = [int(n) for n in data[:-2].split("-")]

		if passAIsValid(char, minN, maxN, seq):
			validA.append(i)
		print(char, minN, maxN, seq)
		if passBIsValid(char, minN, maxN, seq):
			print("validB")
			validB.append(i)
	print len(validA)
	print len(validB)

