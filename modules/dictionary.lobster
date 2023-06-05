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
    private buckets:[chain<K,V>?] = []
    private size:int

    private def lookup(key:K):
        if not buckets.length:
            buckets = map(size): nil
        let h = hash(key) % size
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

    def add(key:K, value:V):
        let c, h = lookup(key)
        if c:
            c.value += value
        else:
            buckets[h] = chain<K,V> { buckets[h], key, value }

    def get(key:K, default_val:V) -> V:
        let c = lookup(key)
        return if c: c.value else: default_val

    def get(key:K) -> V?:
        let c = lookup(key)
        return c and c.value or nil

    def get_all() -> [K], [V]:
        let keys = []
        let vals = []
        for(buckets) c:
            while c:
                keys.push(c.key)
                vals.push(c.value)
                c = c.next
        return keys, vals
