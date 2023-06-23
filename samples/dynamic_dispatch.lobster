abstract class Animal:
    alive = true
    // No implementation of "hello" needed, because abstract.

class Cat : Animal
    def hello():
        print "meow"

class Dog : Animal
    barked = 0

def hello(d::Dog):
    print "bark!"
    barked++

let d = Dog {}
d.hello()

let a:Animal = d
a.hello()
