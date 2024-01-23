// General purpose dictionary implemented using "open addressing" hashing.

// Key can be anything there is a hash() function for (which is all types except
// structs of reference types), though if you use classes or vectors as keys
// you must use the same instances to look up.

// Values can be any type (including any structs). To be able to pass nil to the
// default val for get, the value type must be nillable.

// Initialize with a size that is near the amount of elements you are expecting
// (or want it to be efficient for), e.g.: dictionary<string, float> { 1009 }
// (prime numbers are most efficient: 67, 127, 251, 503, 1009, 2003, 4001,..).

import std

private class chain<K,V>:
    next:chain<K,V>?
    key:K
    value:V

class dictionary<K,V>:
    private buckets:[chain<K,V>?]

    private def lookup(key:K):
        let h = hash(key) % buckets.length
        var c = buckets[h]
        while c:
            if c.key == key:
                return c, h
            c = c.next
        return nil, h

    def set(key:K, value:V):
        let c, h = lookup(key)
        if c:
            c.value = value
        else:
            buckets[h] = chain<K,V> { buckets[h], key, value }

    def del(key:K):
        def delete(c) -> chain<K,V>?:
            if not c: return c
            if c.key == key:
                return c.next
            c.next = delete(c.next)
            return c
        let h = hash(key) % buckets.length
        buckets[h] = delete(buckets[h])

    def clear():
        for (buckets.length) i:
            buckets[i] = nil

    // Useful if the value is something that supports the + operator, you can use it
    // to increment/append.
    def add(key:K, value:V):
        let c, h = lookup(key)
        if c:
            c.value += value
        else:
            buckets[h] = chain<K,V> { buckets[h], key, value }

    // Use this if the value type is a reference type, such that if `get`
    // fails to find an element, it returns nil.
    def get(key:K) -> V?:
        let c = lookup(key)
        return c and c.value or nil

    // Use this with non-reference type, and specify the default or sentinal value to return,
    // e.g. for int use 0 or -1.
    // Also useful for string to return "" as default instead of nil.
    def get(key:K, default_val:V) -> V:
        let c = lookup(key)
        return if c: c.value else: default_val

    def get_keys() -> [K]:
        let keys = []
        for(buckets) c:
            while c:
                keys.push(c.key)
                c = c.next
        return keys

    def get_values() -> [V]:
        let vals = []
        for(buckets) c:
            while c:
                vals.push(c.value)
                c = c.next
        return vals

    def forkv(fun):
        for(buckets) c:
            while c:
                fun(c.key, c.value)
                c = c.next

constructor dictionary<K,V>(size:int):
    return dictionary<K,V> { map(size): nil }
