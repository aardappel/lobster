// Copyright 2014 Wouter van Oortmerssen. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// doubly linked list
// DLNodeRaw does not initialize nor assumes initialization, so can be used in
// situations where memory is already allocated DLNodeBase is meant to be a base
// class for objects that want to be held in a DLList. Unlike DLNodeRaw it is always
// initialized and checks that it's being used sensibly
// These are better than std::list which doesn't have a node to inherit from,
// so causes 2 allocations rather than 1 for object hierarchies

struct DLNodeRaw
{
    DLNodeRaw *prev, *next;
    
    void Remove()
    {
        prev->next = next;
        next->prev = prev;
    }
    
    void InsertAfterThis(DLNodeRaw *o)
    {
        o->next = next;
        o->prev = this;
        next->prev = o;
        next = o;
    }

    void InsertBeforeThis(DLNodeRaw *o)
    {
        o->prev = prev;
        o->next = this;
        prev->next = o;
        prev = o;
    }
};

struct DLNodeBase : DLNodeRaw
{
    DLNodeBase() { prev = next = nullptr; }   

    bool Connected() { return next && prev; }
    
    virtual ~DLNodeBase()
    {
        if (Connected()) Remove();
    }
    
    void Remove()
    {
        assert(Connected());
        DLNodeRaw::Remove();
        next = prev = nullptr;
    }

    void InsertAfterThis(DLNodeBase *o)
    {
        assert(Connected() && !o->Connected());
        DLNodeRaw::InsertAfterThis(o);
    }

    void InsertBeforeThis(DLNodeBase *o)
    {
        assert(Connected() && !o->Connected());
        DLNodeRaw::InsertBeforeThis(o);
    }
};

template<typename T> struct DLList : DLNodeRaw
{
    typedef T nodetype;

    DLList() { next = prev = this; }
    
    bool Empty() { return next==this; }

    T *Get()
    {
        assert(!Empty());
        DLNodeRaw *r = next;
        r->Remove();
        return (T *)r;
    }

    T *Next() { return (T *)next; }
    T *Prev() { return (T *)prev; }
};

template<typename T> T *Next(T *n) { return (T *)n->next; }
template<typename T> T *Prev(T *n) { return (T *)n->prev; }

// safe Remove on not self
#define loopdllistother(L, n)   for (auto n = (L).Next();              n != (void *)&(L);          n = Next(n))
// safe Remove on self
#define loopdllist(L, n)        for (auto n = (L).Next(), p = Next(n); n != (void *)&(L); (n = p),(p = Next(n)))
// safe Remove on self reverse
#define loopdllistreverse(L, n) for (auto n = (L).Prev(), p = Prev(n); n != (void *)&(L); (n = p),(p = Prev(n)))

inline uchar *loadfile(const char *fn, size_t *lenret = nullptr)
{
    FILE *f = fopen(fn, "rb");
    if (!f) return nullptr;
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, 0, SEEK_SET);
    uchar *buf = (uchar *)malloc(len+1);
    if (!buf) { fclose(f); return nullptr; }
    buf[len] = 0;
    size_t rlen = fread(buf, 1, len, f);
    fclose(f);
    if (len!=rlen || len<=0) { free(buf); return nullptr; }
    if (lenret) *lenret = len;
    return buf;
}

class MersenneTwister         
{
    const static uint N = 624;             
    const static uint M = 397;               
    const static uint K = 0x9908B0DFU;   
        
    uint hiBit(uint u)  { return u & 0x80000000U; }  
    uint loBit(uint u)  { return u & 0x00000001U; }  
    uint loBits(uint u) { return u & 0x7FFFFFFFU; }  
    
    uint mixBits(uint u, uint v) { return hiBit(u)|loBits(v); } 

    uint state[N+1];     
    uint *next;          
    int left;   
    
    public:
    
    MersenneTwister() : left(-1) {}  

    void SeedMT(uint seed)
    {
        uint x = (seed | 1U) & 0xFFFFFFFFU, *s = state;
        int j;
        for(left=0, *s++=x, j=N; --j; *s++ = (x*=69069U) & 0xFFFFFFFFU);
    }

    uint ReloadMT()
    {
        uint *p0=state, *p2=state+2, *pM=state+M, s0, s1;
        int j;
        if(left < -1) SeedMT(4357U);
        left=N-1, next=state+1;
        for(s0=state[0], s1=state[1], j=N-M+1; --j; s0=s1, s1=*p2++)
            *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
        for(pM=state, j=M; --j; s0=s1, s1=*p2++)
            *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
        s1=state[0], *p0 = *pM ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
        s1 ^= (s1 >> 11);
        s1 ^= (s1 <<  7) & 0x9D2C5680U;
        s1 ^= (s1 << 15) & 0xEFC60000U;
        return(s1 ^ (s1 >> 18));
    }

    uint RandomMT()
    {
        uint y;
        if(--left < 0) return(ReloadMT());
        y  = *next++;
        y ^= (y >> 11);
        y ^= (y <<  7) & 0x9D2C5680U;
        y ^= (y << 15) & 0xEFC60000U;
        return(y ^ (y >> 18));
    }
    
    int operator()(int max) { return RandomMT()%max; }

    double rnddouble() { return RandomMT() * (1.0 / 4294967296.0); }
    float rndfloat() { return (float)rnddouble(); } // FIXME: performance?
    float rndfloatsigned() { return (float)(rnddouble() * 2 - 1); }

    void ReSeed(uint seed)
    {
        SeedMT(seed);
        left = 0;
        ReloadMT();
    }
};

struct Serializer;

struct Serializable
{
    virtual void Serialize(Serializer &ser) = 0;
};

struct Serializer
{
    vector<uchar> wbuf;
    uchar *rbuf;

    Serializer(uchar *_rbuf) : rbuf(_rbuf) {}

    template<typename T> void integer(T &x)
    {
        if (rbuf)
        {
            x = 0;
            for (int j = 0; j < int(sizeof(T)); j++)
            {
                x |= *rbuf++ << (j * 8);
            }
        }
        else
        {
            for (int j = 0; j < int(sizeof(T)); j++)
            {
                wbuf.push_back(x & 0xFF);
                x >>= 8;
            }
        }
    }

    template<typename T> void operator()(T *&x)
    {
        if (rbuf) x = new T();
        (*this)(*x);
    }

    void operator()(Serializable &x)
    {
        x.Serialize(*this);
    }

    void operator()(int    &x) { integer(x); }
    void operator()(size_t &x) { integer(x); }
    
    void operator()(bool &b)
    {
        if (rbuf)
            b = *rbuf++ != 0;
        else
            wbuf.push_back(b);
    }

    void operator()(string &s)
    {
        if (rbuf)
        {
            s = (char *)rbuf;
            rbuf += strlen((char *)rbuf) + 1;
        }
        else
        {
            wbuf.insert(wbuf.end(), s.c_str(), s.c_str() + s.size() + 1);
        }
    }

    template<typename T> void operator()(vector<T> &v)
    {
        size_t sz = v.size();
        (*this)(sz);
        if (rbuf) v.resize(sz);
        for (auto &e : v) (*this)(e);
    }
};

#ifdef WIN32
#define snprintf _snprintf
#endif

inline char *inttoa(int i)
{
    static char _buf[100];
    snprintf(_buf, 100, "%d", i);
    return _buf;
}

inline char *flttoa(double f, int decimals = -1)
{
    static char _buf[100];
    if (decimals < 0) snprintf(_buf, 100, "%f", f);
    else              snprintf(_buf, 100, "%.*f", decimals, f);
    return _buf; 
}

/* Accumulator: a container that is great for accumulating data like std::vector,
   but without the reallocation/copying and unused memory overhead.
   Instead stores elements as a 2-way growing list of blocks.
   Loses the O(1) random access time, but instead has fast block-wise iteration.
   Optimized to append/prepend many of T at once.
   Can specify a minimum growth (block size) such that small appends are also efficient
*/

template <typename T> class Accumulator
{
    struct Buf
    {
        Buf *next;
        size_t size, unused;

        T *Elems() { return this + 1; }
    };

    Buf *first, *last, *iterator;
    size_t mingrowth;
    size_t totalsize;

    Buf *NewBuf(size_t _size, size_t numelems, T *elems, Buf *_next)
    {
        Buf *buf = (Buf *)malloc(sizeof(Buf) + sizeof(T) * _size);
        buf->size = _size;
        buf->unused = _size - numelems;
        buf->next = _next;
        memcpy(buf->Elems(), elems, sizeof(T) * numelems);
        return buf;
    }

    Accumulator(const Accumulator &);  // don't copy, create instances of this class with new preferably
    Accumulator &operator=(const Accumulator &);

    public:

    Accumulator(size_t _mingrowth = 1024)
        : first(nullptr), last(nullptr), iterator(nullptr),
          mingrowth(_mingrowth), totalsize(0)
          {}

    ~Accumulator()
    {
        while (first)
        {
            Buf *buf = first->next;
            free(first);
            first = buf;
        }
    }

    size_t Size() { return totalsize; }

    void Append(const T &elem) { Append(&elem, 1); }

    void Append(const T *newelems, size_t amount)
    {
        totalsize += amount;

        // first fill up any unused space in the last block, this the common path for small appends
        if (last && last->unused)
        {
            size_t fit = min(amount, last->unused);
            // note: copy constructor skipped, if any
            memcpy(last->Elems() + (last->size - last->unused), newelems, sizeof(T) * fit);
            last->unused -= fit;
            amount -= fit;
        }

        // if there are more elements left, create a new block of mingrowth or bigger size
        if (amount)
        {
            size_t allocsize = max(mingrowth, amount);
            Buf *buf = NewBuf(allocsize, min(amount, allocsize), newelems, nullptr);
            if (last) last->next = buf;
            else last = first = buf;
        }
    }

    void Prepend(const T *newelems, size_t amount)
    {
        totalsize += amount;

        // since Prepend is a less common operation, we don't respect mingrowth here and just allocate
        // a single block every time we could support mingrowth if needed, at the cost of complicating
        // tracking where the unused space lives
        first = NewBuf(amount, amount, newelems, first);
        if (!last) last = first;
    }

    // custom iterator, because it is more efficient to do this a block at a time for clients wishing to process
    // multiple elements at once. limitation of one iterator at a time seems reasonable.

    void ResetIterator() { iterator = first; }

    size_t Iterate(T *&buf)
    {
        if (!iterator) return 0;
        size_t size = iterator->size - iterator->unused;
        buf = iterator->Elems();
        iterator = iterator->next;
        return size;
    }

    // example of iterator usage: Copy into a single linear buffer
    // the size of dest must equal what Size() returns

    void CopyTo(T *dest)
    {
        T *buf;
        size_t size;
        ResetIterator();
        while((size = Iterate(buf)))
        {
            memcpy(dest, buf, size * sizeof(T));
            dest += size;
        }
    }
};

typedef Accumulator<uchar> ByteAccumulator;




// Easy "dynamic scope" helper: replace any variable by a new value, and at the end of the scope put the old value back

template <typename T> struct DS
{
    T temp;
    T &dest;
    
    DS(T &_dest, const T &val) : dest(_dest)
    {
        temp = dest;
        dest = val;
    }
    
    ~DS()
    {
        dest = temp;
    }
};

// Container that turns pointers into integers, with O(1) add/delete/get.
// Robust: passing invalid integers will just return a nullptr pointer / ignore the delete
// Cannot store nullptr pointers (will assert on Add)
// conveniently, index 0 is never used, so can be used by the client to indicate invalid index

// TODO: can change IntResourceManager to takes T's instead of T* by making the next field have a special value for
// in-use (e.g. -2, or 0 if you skip the first field)

template <typename T> class IntResourceManager
{
    struct Elem
    {
        T *t;
        size_t nextfree;

        Elem() : t(nullptr), nextfree(size_t(-1)) {}
    };

    vector<Elem> elems;
    size_t firstfree;

    public:

    IntResourceManager() : firstfree(size_t(-1))
    {
        elems.push_back(Elem());    // a nullptr item at index 0 that can never be allocated/deleted
    }

    ~IntResourceManager()
    {
        for (auto &e : elems)
            if (e.t)
                delete e.t;
    }

    size_t Add(T *t)
    {
        // we can't store nullptr pointers as elements, because we wouldn't be able to distinguish them from unallocated
        // slots
        assert(t);
        size_t i = elems.size();
        if (firstfree < i)
        {
            i = firstfree;
            firstfree = elems[i].nextfree;
        }
        else
        {
            elems.push_back(Elem());
        }
        elems[i].t = t;
        return i;
    }

    T *Get(size_t i)
    {
        return i < elems.size() ? elems[i].t : nullptr;
    }

    void Delete(size_t i)
    {
        T *e = Get(i);
        if (e)
        {
            delete e;
            elems[i].t = nullptr;
            elems[i].nextfree = firstfree;
            firstfree = i;
        }
    }

    size_t Range() { return elems.size(); }     // if you wanted to iterate over all elements
};

// same as IntResourceManager, but now uses pointer tagging to store the free list in-place.
// Uses half the memory. Access is slightly slower, but if memory bound could still be faster overall.
// Can store nullptr pointers, but not pointers with the lowest bit set (e.g. char * pointing inside of another allocation,
// will assert on Add)

template <typename T> class IntResourceManagerCompact
{
    vector<T *> elems;
    size_t firstfree;
    const function<void(T *e)> deletefun;

    // free slots have their lowest bit set, and represent an index (shifted by 1)
    bool IsFree(T *e) { return ((size_t)e) & 1; }
    size_t GetIndex(T *e) { return ((size_t)e) >> 1; }
    T *CreateIndex(size_t i) { return (T *)((i << 1) | 1); }
    bool ValidSlot(size_t i) { return i < elems.size() && !IsFree(elems[i]); }

    public:

    IntResourceManagerCompact(const function<void(T *e)> &_df) : firstfree(SIZE_MAX), deletefun(_df)
    {
        elems.push_back(nullptr);  // slot 0 is permanently blocked, so can be used to denote illegal index
    }

    ~IntResourceManagerCompact()
    {
        ForEach(deletefun);
    }

    void ForEach(const function<void(T *e)> &f)
    {
        for (auto e : elems)
            if (!IsFree(e) && e)
                f(e);
    }

    size_t Add(T *e)
    {
        assert(!IsFree(e)); // can't store pointers with their lowest bit set

        size_t i = elems.size();
        if (firstfree < i)
        {
            i = firstfree;
            firstfree = GetIndex(elems[i]);
            elems[i] = e;
        }
        else
        {
            elems.push_back(e);
        }
        return i;
    }

    T *Get(size_t i)
    {
        return ValidSlot(i) ? elems[i] : nullptr;
    }

    void Delete(size_t i)
    {
        if (ValidSlot(i) && i)
        {
            T *&e = elems[i];
            if (e) deletefun(e);
            e = CreateIndex(firstfree);
            firstfree = i;
        }
    }

    size_t Range() { return elems.size(); }     // if you wanted to iterate over all elements
};


/*

// from: http://average-coder.blogspot.com/2012/07/python-style-range-loops-in-c.html

template<class T>
class range_iterator : public std::iterator<std::input_iterator_tag, T>{
public:
    range_iterator(const T &item) : item(item) {}
    
    // Dereference, returns the current item.
    const T &operator*() {
        return item;
    }
    
    // Prefix.
    range_iterator<T> &operator++() {
        ++item;
        return *this;
    }
    
    // Postfix.
    range_iterator<T> &operator++(int) {
        range_iterator<T> range_copy(*this);
        ++item;
        return range_copy;
    }
    
    // Compare internal item
    bool operator==(const range_iterator<T> &rhs) {
        return item == rhs.item;
    }
    
    // Same as above
    bool operator!=(const range_iterator<T> &rhs) {
        return !(*this == rhs);
    }
private:
    T item;
};

template<class T>
class range_wrapper {
public:
    range_wrapper(const T &r_start, const T &r_end)
    : r_start(r_start), r_end(r_end) {}

    range_iterator<T> begin() {
        return range_iterator<T>(r_start);
    }
    
    range_iterator<T> end() {
        return range_iterator<T>(r_end);
    }
private:
    T r_start, r_end;
};

// Returns a range_wrapper<T> containing the range [start, end)
template<class T>
range_wrapper<T> range(const T &start, const T &end) {
    return range_wrapper<T>(start, end);
}

// Returns a range_wrapper<T> containing the range [T(), end)
template<class T>
range_wrapper<T> range(const T &end) {
    return range_wrapper<T>(T(), end);
}

*/
