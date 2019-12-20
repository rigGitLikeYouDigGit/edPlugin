from edPlugin.adventofcode.inputs import day4
import math # :/ hoped to get through this all without importing libs

"""I'm going to brute force this because I am but a brutish basic boi"""


"""
--- Day 4: Secure Container ---
You arrive at the Venus fuel depot only to discover it's protected by a password. The Elves had written the password on a sticky note, but someone threw it out.

However, they do remember a few key facts about the password:

It is a six-digit number.
The value is within the range given in your puzzle input.
Two adjacent digits are the same (like 22 in 122345).
Going from left to right, the digits never decrease; they only ever increase or stay the same (like 111123 or 135679).
Other than the range rule, the following are true:

111111 meets these criteria (double 11, never decreases).
223450 does not meet these criteria (decreasing pair of digits 50).
123789 does not meet these criteria (no double).
How many different passwords within the range given in your puzzle input meet these criteria?

Your puzzle input is 284639-748759.
"""

def check(digits, number, results, length=6, groups=False):
	"""digits as list of digits
	results : list """

	digitSet = set(digits)
	groups = {} # oof ouch my cpu cycles
	if len(digitSet) == length: # no duplicates
		return
	for i in range(length):
		d = digits[ i ]
		if i:
			if digits[ i ] < digits[ i-1 ]:
				return # nope
		groups[ d ] = groups[ d ] + 1 if d in groups else 1

	# looking for at least one two-digit set
	if not any([n == 2 for n in groups.values()]):
		return

	results.append(number)




	pass


def findRange(min, max):
	""" find the actual range from random starting bounds
	for our purpose : 284639 goes to 288888
	748759 goes to 699999"""
	# min = str(min)
	# newMin = []
	# for i in range( len(min) ):
	# 	if i:
	# 		if int(min[i]) < int(min[i-1]):
	# 			newMin[i] = min[i-1]
	# 	else:
	# 		newMin[i] = min[1]
	# whatever
	return 288888, 699999

""" wait a second - let's try do a big brain think 

	288888 - 288889 : only one possible
	for our purposes, how many viable lie between 288888 and 699999?
	
	digits = --k-- i --m--
	where 0 <= i <= 9
	( 9 - i + 1 ) * m ** m
	
	if i is 9, all subsequent digits must be 9 : 
		(9 - 9 + 1) * m ** 2 ? no
		(9 - 9)
		
	9xx --> 999
	8xx --> 888, 889, 899, 999 
	7xx --> 777, 778, 788, 888, #889, 899, 999 
	6xx --> 666, 667, 677, 777, 
	nViable = (9 - i) * (m + 1) + 1?
	
	8xxx --> we predict ( 9 - 8 ) * 4 + 1 = 5
	8xxx --> 8888, 8889, 8899, 8999, 9999
	
	# this would be easy if i weren't a fool
	9xx --> 999
	8xx --> 888, 889, 899, 999 
	7xx --> 777, 778, 788, 888, # 779, 789 
	6xx --> 666, 667, 677, 777, # 669, 699, 689, 688, 678, 679
	# this feels symmetrical
		
	
	
	for any 3 digits, how many must we discount due to no pairs?
	3 = xxx # imagine in binary, digitSpace = 2
	000, 001, 011, 010, 100, 101, 110, 111
	impossible in binary, if m > digitSpace, all results are valid
	aaa, aab, aba, abb, # bba, bab, baa, bbb, 
	# again this feels like reflection, almost like a Gray code
	
	there is an analytical solution here, but i'm literally too stupid and dull
	to see it. sorry if you were expecting something interesting or elegant, i don't
	like being this any more than you like being stuck with it. stop reading this
	and go watch numberphile, do something that will actually help you. apologies
	for wasting your time

	
	
	
	
"""


if __name__ == '__main__':
	# find bounds
	lowBound, upperBound = day4
	lowBound, upperBound = findRange(lowBound, upperBound)

	valid = []
	for i in range(lowBound, upperBound + 1):
		# digitising operation from stackoverflow
		# digits = [ (i // ( 10 ** i)) % 10 for n in range(
		# 	int(math.ceil( math.log( i, 10) ) - 1), -1,  -1	) ]
		digits = [int(n) for n in str(i)]
		# faster than stringifying apparently
		# NOT ON MY PC lol, took like a second per iteration
		check( digits, i, valid, length=6 )
		if not i % 100:
			#print(i, len(valid), valid)
			pass
	print(len(valid))
	""" the answer is 895 """






