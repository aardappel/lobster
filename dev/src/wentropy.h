// Wouter's Entropy Coder
//
// identical compression performance to huffman, and absolutely tiny code, one function for both compression and decompression
// adaptive, so should work well even for tiny buffers
// depending on the "compress" boolean, one of the vectors passed in is the input, the other the output (which should be empty initially)
// not the fastest possible implementation (40MB/sec for either compression or decompression on a modern pc), but should be sufficient for many uses
//
// by design, it is limited to compress max 4GB at once.
// replace uint in the code below by size_t to allow it to compress larger files in 64bit mode, at the cost of incompatability of the data format with 32bit builds
// the vector<uint> may require endian-swapping if used cross platform

void WEntropyCoder(vector<uchar> &uncompressed, vector<uint> &compressed, bool compress)
{
    assert(uncompressed.size() <= 0xFFFFFFFF);  // just in case, for 64bit builds
    const int NSYM = 256;   // to change this, we'd have to read from something other than vector<uchar>
    uint freq[NSYM];
    int ch[NSYM], si[NSYM];
    for (int i = 0; i < NSYM; i++) { freq[i] = 1; ch[i] = si[i] = i; }

    uint len = compress ? (uint)uncompressed.size() : compressed[0];
    if (compress) compressed.push_back(len);    // original size always stored as first element
    uint ci = 0;
    uint bits = 0;
    int nbits = 0;

    for (uint i = 0; i < len; i++)
    {
        int start = 0, size = NSYM;
        uint tf = i + NSYM;
        while (size > 1)
        {
            uint af = 0;
            int j = start;
            do
            {
                af += freq[j++];
            }
            while (af + freq[j] / 2 < tf / 2);
            uint bit = 0;
            if (compress)
            {
                if (si[uncompressed[i]] < j) bit = 1;
            }
            else
            {
                if (!nbits) { bits = compressed[++ci]; nbits = sizeof(uint) * 8; }
                bit = bits & 1;
            }
            if (bit) { tf = af; assert(j - start < size); size = j - start; }
            else     { tf -= af; size -= j - start; start = j; }
            if (compress)
            {
                bits |= bit << nbits;
                if (++nbits == sizeof(uint) * 8) { compressed.push_back(bits); bits = 0; nbits = 0; }
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

