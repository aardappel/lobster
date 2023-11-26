struct foo:
	bar: int
constructor foo(bar: int = 1):
	return foo{bar}

def baz():
	return foo(5)

baz()
