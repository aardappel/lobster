// Wouter's Entropy Coder
//
// identical compression performance to huffman, and absolutely tiny code, one function for both compression and decompression
// adaptive, so should work well even for tiny buffers
// one of the vectors passed in is the input, the other the output (and exactly one of the two should be empty initially)
// not the fastest possible implementation (only 40MB/sec for either compression or decompression on a modern pc), but should be sufficient for many uses
//
// uses std::vector and std::swap as its only external dependencies for simplicity, but could made to not rely on them relatively easily.
//
// by design, it is limited to compress max 4GB at once.
// replace unsigned int in the code below by size_t to allow it to compress larger files in 64bit mode, at the cost of incompatability of the data format with 32bit builds
// the vector<unsigned int> may require endian-swapping if used cross platform (hey, didn't little endian win? :P)

void WEntropyCoder(vector<uchar> &uncompressed, vector<unsigned int> &compressed)
{
    bool compress = uncompressed.size() != 0;
    assert(uncompressed.size() <= 0xFFFFFFFF);  // just in case, for 64bit builds
    const int NSYM = 256;   // to change this, we'd have to read from something other than vector<uchar>
    unsigned int freq[NSYM];
    int ch[NSYM], si[NSYM];
    for (int i = 0; i < NSYM; i++) { freq[i] = 1; ch[i] = si[i] = i; }

    unsigned int len = compress ? (unsigned int)uncompressed.size() : compressed[0];
    if (compress) compressed.push_back(len);    // original size always stored as first element
    unsigned int ci = 0;
    unsigned int bits = 0;
    int nbits = 0;

    for (unsigned int i = 0; i < len; i++)
    {
        int start = 0, size = NSYM;
        unsigned int tf = i + NSYM;
        while (size > 1)
        {
            unsigned int af = 0;
            int j = start;
            do af += freq[j++]; while (af + freq[j] / 2 < tf / 2);
            unsigned int bit = 0;
            if (compress)
            {
                if (si[uncompressed[i]] < j) bit = 1;
            }
            else
            {
                if (!nbits) { bits = compressed[++ci]; nbits = sizeof(unsigned int) * 8; }
                bit = bits & 1;
            }
            if (bit) { tf = af; assert(j - start < size); size = j - start; }
            else     { tf -= af; size -= j - start; start = j; }
            if (compress)
            {
                bits |= bit << nbits;
                if (++nbits == sizeof(unsigned int) * 8) { compressed.push_back(bits); bits = 0; nbits = 0; }
            }
            else
            {
                bits >>= 1;
                nbits--;
            }
        }
        if (!compress) uncompressed.push_back(ch[start]);
        assert(size == 1 && uncompressed[i] == ch[start]);   
        freq[start]++;
        while (start && freq[start - 1] < freq[start])
        {
            swap(si[ch[start - 1]], si[ch[start]]);
            swap(freq[start - 1], freq[start]);
            swap(ch[start - 1], ch[start]);
            start--;
        }
    }
    if (compress) compressed.push_back(bits);
}

