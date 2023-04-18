let a = 2

def b():
	return a

b()

def c(b):
	return b()

c():
	print("hey")

a = 3

b()

b = 1