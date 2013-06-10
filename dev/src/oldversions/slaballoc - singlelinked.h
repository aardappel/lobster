#ifdef _DEBUG
    //#define PASSTHRUALLOC
#endif

class SlabAlloc
{
    // tweakables:
    enum { MAXBUCKETS = 32 };   // must be ^2.
                                // lower means more blocks have to go thru the traditional allocator (slower)
                                // higher means you may get pages with only few allocs of that unique size (memory wasted)
    enum { PAGESATONCE = 101 }; // depends on how much you want to take from the OS at once: PAGEATONCE*PAGESIZE
                                // You will waste 1 page to alignment
                                // with MAXBUCKETS at 32 on a 32bit system, PAGESIZE is 2048, so this is 202k

    // derived:
    enum { PTRBITS = sizeof(char *)==4 ? 2 : 3 };  // "64bit should be enough for everyone"
                                                   // everything is twice as big on 64bit: alignment, memory blocks, and pages
    enum { ALIGNBITS = PTRBITS+1 };                // must fit 2 pointers in smallest block for doubly linked list
    enum { ALIGN = 1<<ALIGNBITS };
    enum { ALIGNMASK = ALIGN-1 };

    enum { MAXREUSESIZE = (MAXBUCKETS-1)*ALIGN };

	enum { PAGESIZE = MAXBUCKETS*ALIGN*8 }; // meaning the largest block will fit almost 8 times

    enum { PAGEMASK = (~(PAGESIZE-1)) };
	enum { PAGEBLOCKSIZE = PAGESIZE*PAGESATONCE };

	inline size_t bucket(size_t s)
    {
        return (s+ALIGNMASK)>>ALIGNBITS;
    }

	inline size_t *ppage(void *p)
    {
        return (size_t *)(((size_t)p)&PAGEMASK);
    }

	void *reuse[MAXBUCKETS];
	void **pages;
    void **blocks;

    long long stats[MAXBUCKETS];
    long long statbig;

	void putinbuckets(char *start, char *end, size_t bsize)
	{
		size_t size = bsize*ALIGN;        
		for(end -= size; start<=end; start += size)
		{
			*((void **)start) = reuse[bsize];
			reuse[bsize] = start;
		}
	}

	void newpageblocks()
	{
		void **b = (void **)malloc(PAGEBLOCKSIZE+sizeof(void *)); // if we could get page aligned memory here, that would be even better
        *b = (void *)blocks;
        blocks = b;
        b++;

        char *first = ((char *)ppage(b))+PAGESIZE;
		for(int i = 0; i<PAGESATONCE-1; i++)
		{
			void **p = (void **)(first+i*PAGESIZE);
			*p = pages;
			pages = p;
		}
        /*
		if(first-b>minimum_useful)          available_memory(b, first-b); 
		if(b-first+PAGESIZE>minimum_useful) available_memory(first+PAGEBLOCKSIZE-PAGESIZE, b-first+PAGESIZE);
        */
	}

	void *newpage(size_t bsize)
	{
		if(!pages) newpageblocks();
		void **page = pages;
		pages = (void **)*pages;
		*page = 0;
		putinbuckets((char *)(page+1), ((char *)page)+PAGESIZE, bsize);
		return alloc(bsize*ALIGN);
	}

	void freepage(size_t *page, size_t bsize) // worst case if very large amounts of objects get deallocated in random order from when they were allocated
	{
		for(void **r = &reuse[bsize]; *r; )
		{
			if(page == ppage(*r)) *r = *((void **)*r);
			else r = (void **)*r;
		}
		void **p = (void **)page;
		*p = pages;
		pages = p;
	}

public:

	SlabAlloc() : blocks(NULL), pages(NULL), statbig(0)
	{
		for(int i = 0; i<MAXBUCKETS; i++)
        {
            reuse[i] = NULL;
            stats[i] = 0;
        }
	}

    ~SlabAlloc()
    {
        for(int i = 0; i<MAXBUCKETS; i++) if(reuse[i])
        {
            // TODO: can report unfreed memory here
        }

        while(blocks)
        {
            void *next = *blocks;
            free(blocks);
            blocks = (void **)next;
        }
    }

	void *alloc(size_t size)
	{
        #ifdef PASSTHRUALLOC
            return malloc(size);
        #else
            if(size>MAXREUSESIZE) { statbig++; return malloc(size); }
            assert(size);
		    size = bucket(size);
            #ifdef _DEBUG
                stats[size]++;
            #endif
		    void **r = (void **)reuse[size];
		    if(!r) return newpage(size);
		    reuse[size] = *r;
		    size_t *page = ppage(r);
		    (*page)++;
		    return (void *)r;
        #endif
	}

	void dealloc(void *p, size_t size)
	{
        #ifdef PASSTHRUALLOC
            free(p);
        #else
            #ifdef _DEBUG     
                memset(p, 0xBA, size); 
            #endif

		    if(size>MAXREUSESIZE)
		    {
			    free(p);
		    }
		    else
		    {
			    size = bucket(size);
			    *((void **)p) = reuse[size];
			    reuse[size] = p;
			    size_t *page = ppage(p);
			    if(!--(*page)) freepage(page, size);
		    }
        #endif
	}

    void *resize(void *p, size_t oldsize, size_t size)
    {
        void *np = alloc(size);
        memcpy(np, p, size>oldsize ? oldsize : size);
        dealloc(p, oldsize);
        return np;
    }

    // versions of the above functions that track size for you

    void *alloc_sized(size_t size)
    {
        size_t *p = (size_t *)alloc(size+sizeof(size_t));
        *p++ = size;  // stores 2 sizes for big objects!
        return p;
    }

    void dealloc_sized(void *p) 
    {
        size_t *t = (size_t *)p;
        size_t size = *--t;	
        size += sizeof(size_t);
        dealloc(t, size);
    }

    void *resize_sized(void *p, size_t size)
    {
        void *np = alloc_sized(size);
        size_t oldsize = ((size_t *)p)[-1];
        memcpy(np, p, size>oldsize ? oldsize : size);
        dealloc_sized(p);
        return np;
    }

    // typed helpers

    template<class T> T *allocobj()
    {
        return (T *)alloc(sizeof(T));
    }

    /*
    template<class T> T *create()
    {
        return new (allocobj<T>()) T();
    }
    */

    template<class T> void destruct(T *obj)
    {
        obj->~T();
        dealloc(obj, sizeof(T));
    }

	void printstats()
	{
		size_t totalwaste = 0;
        long long totalallocs = 0;
		char buf[100];
		for(int i = 0; i<MAXBUCKETS; i++)
		{
			size_t n = 0;
			for(void **r = (void **)reuse[i]; r; r = (void **)*r) n++;
			if(n || stats[i])
			{
				size_t waste = (i*ALIGN*n+512)/1024;
				totalwaste += waste;
                totalallocs += stats[i];
				sprintf(buf, "bucket %d -> %d (%d k), %lld allocs\n", i*ALIGN, n, waste, stats[i]);
				::OutputDebugStringA(buf); 
			}
		}

        int nump = 0;
        while(pages) { nump++; pages = (void **)*pages; }

        sprintf(buf, "totalwaste %d k, %d pages empty, %lld total allocs, %d big allocs\n", totalwaste, nump, totalallocs, statbig);
		::OutputDebugStringA(buf); 
	}
};
