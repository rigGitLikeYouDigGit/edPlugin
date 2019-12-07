from edPlugin.adventofcode.inputs import day2
"""
--- Day 2: 1202 Program Alarm ---
On the way to your gravity assist around the Moon, your ship computer beeps angrily about a "1202 program alarm". On the radio, an Elf is already explaining how to handle the situation: "Don't worry, that's perfectly norma--" The ship computer bursts into flames.

You notify the Elves that the computer's magic smoke seems to have escaped. "That computer ran Intcode programs like the gravity assist program it was working on; surely there are enough spare parts up there to build a new Intcode computer!"

An Intcode program is a list of integers separated by commas (like 1,0,0,3,99). To run one, start by looking at the first integer (called position 0). Here, you will find an opcode - either 1, 2, or 99. The opcode indicates what to do; for example, 99 means that the program is finished and should immediately halt. Encountering an unknown opcode means something went wrong.

Opcode 1 adds together numbers read from two positions and stores the result in a third position. The three integers immediately after the opcode tell you these three positions - the first two indicate the positions from which you should read the input values, and the third indicates the position at which the output should be stored.

For example, if your Intcode computer encounters 1,10,20,30, it should read the values at positions 10 and 20, add those values, and then overwrite the value at position 30 with their sum.

Opcode 2 works exactly like opcode 1, except it multiplies the two inputs instead of adding them. Again, the three integers after the opcode indicate where the inputs and outputs are, not their values.

Once you're done processing an opcode, move to the next one by stepping forward 4 positions.

For example, suppose you have the following program:

1,9,10,3,2,3,11,0,99,30,40,50
For the purposes of illustration, here is the same program split into multiple lines:

1,9,10,3,
2,3,11,0,
99,
30,40,50
The first four integers, 1,9,10,3, are at positions 0, 1, 2, and 3. Together, they represent the first opcode (1, addition), the positions of the two inputs (9 and 10), and the position of the output (3). To handle this opcode, you first need to get the values at the input positions: position 9 contains 30, and position 10 contains 40. Add these numbers together to get 70. Then, store this value at the output position; here, the output position (3) is at position 3, so it overwrites itself. Afterward, the program looks like this:

1,9,10,70,
2,3,11,0,
99,
30,40,50
Step forward 4 positions to reach the next opcode, 2. This opcode works just like the previous, but it multiplies instead of adding. The inputs are at positions 3 and 11; these positions contain 70 and 50 respectively. Multiplying these produces 3500; this is stored at position 0:

3500,9,10,70,
2,3,11,0,
99,
30,40,50
Stepping forward 4 more positions arrives at opcode 99, halting the program.

Here are the initial and final states of a few more small programs:

1,0,0,0,99 becomes 2,0,0,0,99 (1 + 1 = 2).
2,3,0,3,99 becomes 2,3,0,6,99 (3 * 2 = 6).
2,4,4,5,99,0 becomes 2,4,4,5,99,9801 (99 * 99 = 9801).
1,1,1,4,99,5,6,0,99 becomes 30,1,1,4,2,5,6,0,99.
Once you have a working computer, the first step is to restore the gravity assist program (your puzzle input) to the "1202 program alarm" state it had just before the last computer caught fire. To do this, before running the program, replace position 1 with the value 12 and replace position 2 with the value 2. What value is left at position 0 after the program halts?
"""

ops = {
	1 : ("inAddress1", "inAddress2", "outAddress"),
	2 : ("inAddress1", "inAddress2", "outAddress"),
	99 : (),
	3 : ("inputAddress"), # takes input and stores it at address
	4 : ("outputAddress"), # takes value from address and outputs it

}

def intCode(program, inputs=None,):
	""" 4-cel words: opcode, input1, input2, output
	op1 - add
	op2 - multiply """
	outputs = []
	length = len(program)
	index = 0
	#for i in range(numWords):
	while index < length:
		op = program[ index ]
		params = ops[ op ]
		args = [op]
		for i in range(len(params)):
			index += 1
			args.append( program[ index ])
		index += 1
		result = execute(args, memory=program, inputs=inputs, outputs=outputs)
		if result < 0: # exit
			#print("halting program")
			break
	return outputs

def execute( args,  memory=None, inputs=None, outputs=None):
	"""program is the full intcode program"""
	# op = args[0]
	opData = processOpCode(args)
	op = opData["op"]
	paramModes = opData["paramModes"]

	# gather values for calculation -------
	vals = []
	for i, j in zip(paramModes, args[1:] ):
		if i == 0: # position mode
			vals.append( memory[j] )
		elif i == 1: # literal, "immediate" mode
			vals.append( j )

	# calculate and assign -----------------
	# we don't need to check output params, they're always position mode

	if op == 99:
		return -1

	if op == 1:
		#outVal = val1 + val2
		outVal = vals[0] + vals[1]
		memory[args[3]] = outVal
	elif op == 2:
		#outVal = val1 * val2
		outVal = vals[0] + vals[1]
		memory[args[3]] = outVal
	elif op == 3:
		memory[ args[1] ] = inputs[0] # assigns input to memory index
	elif op == 4:
		#outputs.append( memory[ args[1] ] ) # outputs value at index
		outputs.append( vals[0] )

	else:
		raise Exception("illegal opcode {}".format(op))

	return 0

def processOpCode( args ):
	""" pulls useful info out of the n-digit opcode
	expects the full operation sequence
	from the source:
	ABCDE
	 1002

	DE - two-digit opcode,      02 == opcode 2
	 C - mode of 1st parameter,  0 == position mode
	 B - mode of 2nd parameter,  1 == immediate mode
	 A - mode of 3rd parameter,  0 == position mode,
                                  omitted due to being a leading zero"""
	opCode = str(args[0])
	op = int(opCode[-3:]) # last two digits
	paramModes = []
	for i in range(len(args) - 1):
		if i >= len(opCode) - 2:
			paramModes.append(0)
		else:
			# reverse opcode to find correct param order
			paramModes.append( int( opCode[::-1] [i + 2] ) )
	print(" opCode {}, op {}, paramModes {}".format(opCode, op, paramModes))
	return {"op" : op, "paramModes" : paramModes}




if __name__ == '__main__':

	program = list(day2)
	# modify data as directed
	program[1] = 12
	program[2] = 2
	program[1] = 82
	program[2] = 50
	intCode( program )
	result = program[0]
	print result # 3562672

	# # find inputs to program to make 19690720
	# found = None
	# for i in range(100):
	# 	for n in range(100):
	# 		memory = list(program)
	# 		memory[1] = i
	# 		memory[2] = n
	# 		intCode(memory)
	# 		result = memory[0]
	# 		#print result is 19690720
	# 		if result == 19690720:
	# 			found = i, n
	# 			break
	#
	# if found:
	# 	print found
	# 	#print "result {}, input1 {}, input2 {}".format(result, i, n)
	# else:
	# 	print "the search continues"
	# 	print found

	# 82, 50
	print 100 * 82 + 50



