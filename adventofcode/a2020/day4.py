
"""

You arrive at the airport only to realize that you grabbed your North Pole Credentials instead of your passport. While these documents are extremely similar, North Pole Credentials aren't issued by a country and therefore aren't actually valid documentation for travel in most of the world.

It seems like you're not the only one having problems, though; a very long line has formed for the automatic passport scanners, and the delay could upset your travel itinerary.

Due to some questionable network security, you realize you might be able to solve both of these problems at the same time.

The automatic passport scanners are slow because they're having trouble detecting which passports have all required fields. The expected fields are as follows:

byr (Birth Year)
iyr (Issue Year)
eyr (Expiration Year)
hgt (Height)
hcl (Hair Color)
ecl (Eye Color)
pid (Passport ID)
cid (Country ID)
Passport data is validated in batch files (your puzzle input). Each passport is represented as a sequence of key:value pairs separated by spaces or newlines. Passports are separated by blank lines.

Here is an example batch file containing four passports:

ecl:gry pid:860033327 eyr:2020 hcl:#fffffd
byr:1937 iyr:2017 cid:147 hgt:183cm

iyr:2013 ecl:amb cid:350 eyr:2023 pid:028048884
hcl:#cfa07d byr:1929

hcl:#ae17e1 iyr:2013
eyr:2024
ecl:brn pid:760753108 byr:1931
hgt:179cm

hcl:#cfa07d eyr:2025 pid:166559648
iyr:2011 ecl:brn hgt:59in
The first passport is valid - all eight fields are present. The second passport is invalid - it is missing hgt (the Height field).

The third passport is interesting; the only missing field is cid, so it looks like data from North Pole Credentials, not a passport at all! Surely, nobody would mind if you made the system temporarily ignore missing cid fields. Treat this "passport" as valid.

The fourth passport is missing two fields, cid and byr. Missing cid is fine, but missing any other field is not, so this passport is invalid.

According to the above rules, your improved system would report 2 valid passports.

Count the number of valid passports - those that have all required fields. Treat cid as optional. In your batch file, how many passports are valid?
"""

from edPlugin.adventofcode.a2020.inputs import day4

data = day4.split("\n")
data = " ".join(data).split("  ")

keys = ["byr", "iyr", "eyr", "hgt", "hcl", "ecl", "pid", "cid"]

def passIsValid(data):
	if not all([key in data for key in keys[:-1]]):
		return 0
	b = 1920 <= int(data["byr"]) <= 2002
	i = 2010 <= int(data["iyr"]) <= 2020
	eyr = 2020 <= int(data["eyr"]) <= 2030
	if data["hgt"].endswith("cm"):
		hgt = 150 <= int(data["hgt"][:-2]) <= 193
	elif data["hgt"].endswith("in"):
		hgt = 59 <= int(data["hgt"][:-2]) <= 76
	else:
		hgt = False
	hcls = data["hcl"]
	hcl = len(hcls) == 7 and hcls.startswith("#")
	ecl = data["ecl"] in ["amb", "blu", "brn", "gry", "grn", "hzl", "oth"]
	pid = len(data["pid"]) == 9 and int(data["pid"])

	return all([b, i, eyr, hgt, hcl, ecl, pid])

if __name__ == '__main__':
	print data
	# maps = [[{key: val for key, val in entry.split(":")} for entry in i] for i in [group.split(" ") for group in data]]
	maps = [group.split(" ") for group in data]
	maps = [[group.split(":") for group in n] for n in maps]
	print maps
	results = []
	for group in maps:
		d = {}
		for tie in group:
			d[tie[0]] = tie[1]
		results.append(d)
	#maps = [{k : v} for k, v in [n for n in [i for i in maps]]]
	#maps = [[{k : v for k, v in i.split(":")} for i in n] for n in maps]

	print results

	valid = 0
	for i in results:
		# validKeys = list(keys)
		# validKeys.remove("cid")
		# if all([key in i for key in validKeys]):
		# 	valid += 1
		if passIsValid(i):
			valid += 1
	print( valid)








