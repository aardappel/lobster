/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2026 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

// Functions to discard MP3 tags -
// written by O.Sezer <sezero@users.sourceforge.net>, put into public domain.

// Functions to parse some of MP3 tags -
// written by V.Novichkov <admin@wohlnet.ru>, put into public domain.

#include "SDL_mixer_internal.h"

#define METADATA_TAGS_DEBUG_LOGGING 0
#if METADATA_TAGS_DEBUG_LOGGING
#define DBGLOG SDL_Log
#else
#define DBGLOG(...)
#endif

static Sint32 read_sint32le(const Uint8 *data)
{
    Uint32 result = (Uint32)data[0];
    result |= (Uint32)data[1] << 8;
    result |= (Uint32)data[2] << 16;
    result |= (Uint32)data[3] << 24;
    return (Sint32)result;
}

static Sint32 read_sint24be(const Uint8 *data)
{
    Uint32 result = (Uint32)data[2];
    result |= (Uint32)data[1] << 8;
    result |= (Uint32)data[0] << 16;
    return (Sint32)result;
}

static Sint32 read_sint32be(const Uint8 *data)
{
    Uint32 result = (Uint32)data[3];
    result |= (Uint32)data[2] << 8;
    result |= (Uint32)data[1] << 16;
    result |= (Uint32)data[0] << 24;
    return (Sint32)result;
}


#define TAGS_INPUT_BUFFER_SIZE 128

/********************************************************
 *                  ID3v1                               *
 ********************************************************/

#define ID3v1_TAG_SIZE           128
#define ID3v1_SIZE_OF_FIELD      30
#define ID3v1_SIZE_OF_YEAR_FIELD 4

#define ID3v1_FIELD_TITLE       3
#define ID3v1_FIELD_ARTIST      33
#define ID3v1_FIELD_ALBUM       63
#define ID3v1_FIELD_YEAR        93
#define ID3v1_FIELD_COPYRIGHT   97
#define ID3v1_FIELD_TRACK       125
#define ID3v1_FIELD_GENRE       127

static bool is_id3v1(const Uint8 *data, size_t length)
{
    // https://id3.org/ID3v1 :  3 bytes "TAG" identifier and 125 bytes tag data
    return ((length >= ID3v1_TAG_SIZE) && (SDL_memcmp(data, "TAG", 3) == 0));
}

// Parse ISO-8859-1 string and convert it into UTF-8
static char *parse_id3v1_ansi_string(const Uint8 *buffer, size_t src_len)
{
    char *src_buffer = (char*)SDL_malloc(src_len + 1);
    if (!src_buffer) {
        return NULL; // Out of memory
    }
    SDL_memcpy(src_buffer, buffer, src_len);
    src_buffer[src_len] = '\0';

    // trim whitespace from end (some id3v1 tags pad out with space instead of nulls).
    if (src_len > 0) {
        size_t i = src_len;
        do {
            --i;
            if (src_buffer[i] == ' ') {
                src_buffer[i] = '\0';
                --src_len;
            } else {
                break;
            }
        } while(i > 0);
    }

    char *ret = SDL_iconv_string("UTF-8", "ISO-8859-1", src_buffer, src_len + 1);
    SDL_free(src_buffer);
    return ret;
}

static void id3v1_set_tag(SDL_PropertiesID props, const char *key, const Uint8 *buffer, size_t len)
{
    if (!SDL_HasProperty(props, key)) {  // in case there are multiple ID3v1 tags appended to a file, we'll take the last one, since we parse backwards from the end of file.
        char *src_buf = parse_id3v1_ansi_string(buffer, len);
        if (src_buf && *src_buf) {
            SDL_SetStringProperty(props, key, src_buf);
            SDL_free(src_buf);
        }
    }
}

// Parse content of ID3v1 tag
static void parse_id3v1(SDL_PropertiesID props, const Uint8 *buffer)
{
    // ID3v1.1: if the second-to-last byte of the comment field is a zero (null terminator), treat the last byte as the track number (which should also be zero in ID3v1.0).
    const bool has_tracknum = (buffer[ID3v1_FIELD_TRACK] == 0) && (buffer[ID3v1_FIELD_TRACK+1] != 0);

    id3v1_set_tag(props, "SDL_mixer.metadata.id3v1.title",   buffer + ID3v1_FIELD_TITLE,     ID3v1_SIZE_OF_FIELD);
    id3v1_set_tag(props, "SDL_mixer.metadata.id3v1.artist",  buffer + ID3v1_FIELD_ARTIST,    ID3v1_SIZE_OF_FIELD);
    id3v1_set_tag(props, "SDL_mixer.metadata.id3v1.album",   buffer + ID3v1_FIELD_ALBUM,     ID3v1_SIZE_OF_FIELD);
    id3v1_set_tag(props, "SDL_mixer.metadata.id3v1.comment", buffer + ID3v1_FIELD_COPYRIGHT, ID3v1_SIZE_OF_FIELD - (has_tracknum ? 2 : 0));
    id3v1_set_tag(props, "SDL_mixer.metadata.id3v1.year",    buffer + ID3v1_FIELD_YEAR, ID3v1_SIZE_OF_YEAR_FIELD);

    if (has_tracknum) {
        char str[12];
        SDL_snprintf(str, sizeof (str), "%u", (unsigned int) buffer[ID3v1_FIELD_TRACK+1]);
        id3v1_set_tag(props, "SDL_mixer.metadata.id3v1.track", (Uint8 *) str, SDL_strlen(str));
    }
}

/********************************************************
 *                       ID3v2                          *
 ********************************************************/

#define ID3v2_BUFFER_SIZE               1024

#define ID3v2_HEADER_SIZE               10

#define ID3v2_FIELD_VERSION_MAJOR       3
#define ID3v2_FIELD_VERSION_MINOR       4
#define ID3v2_FIELD_HEAD_FLAGS          5
#define ID3v2_FIELD_TAG_LENGTH          6
#define ID3v2_FIELD_EXTRA_HEADER_LENGTH 10

#define ID3v2_FLAG_HAS_FOOTER           0x10
#define ID3v2_FLAG_HAS_EXTRA_HEAD       0x40

#define ID3v2_3_FRAME_HEADER_SIZE       10
#define ID3v2_2_FRAME_HEADER_SIZE       6
#define ID3v2_FIELD_FRAME_SIZE          4
#define ID3v2_FIELD_FRAME_SIZEv2        3
#define ID3v2_FIELD_FLAGS               8

static bool is_id3v2(const Uint8 *data, size_t length)
{
    // ID3v2 header is 10 bytes:  https://id3.org/id3v2.4.0-structure
    // bytes 0-2: "ID3" identifier
    if ((length < ID3v2_HEADER_SIZE) || (SDL_memcmp(data, "ID3", 3) != 0)) {
        return false;
    }
    // bytes 3-4: version num (major,revision), each byte always less than 0xff.
    if (data[3] == 0xff || data[4] == 0xff) {
        return false;
    }
    // bytes 6-9 are the ID3v2 tag size: a 32 bit 'synchsafe' integer, i.e. the
    // highest bit 7 in each byte zeroed.  i.e.: 7 bit information in each byte ->
    // effectively a 28 bit value.
    if (data[6] >= 0x80 || data[7] >= 0x80 || data[8] >= 0x80 || data[9] >= 0x80) {
        return false;
    }
    return true;
}

static Sint32 id3v2_synchsafe_decode(const Uint8 *data)
{
    return ((data[0] << 21) + (data[1] << 14) + (data[2] << 7) + data[3]);
}

static Sint32 get_id3v2_len(const Uint8 *data, size_t length)
{
    // size is a 'synchsafe' integer (see above)
    size_t size = (size_t) id3v2_synchsafe_decode(data + 6);
    size += ID3v2_HEADER_SIZE; // header size
    // ID3v2 header[5] is flags (bits 4-7 only, 0-3 are zero).
    // bit 4 set: footer is present (a copy of the header but
    // with "3DI" as ident.)
    if (data[5] & 0x10) {
        size += ID3v2_HEADER_SIZE; // footer size
    }
    // optional padding (always zeroes)
    while ((size < length) && (data[size] == 0)) {
        ++size;
    }
    return (Sint32) size;
}

// Decode a string in the frame according to an encoding marker
static char *id3v2_decode_string(const Uint8 *string, size_t size)
{
    char *str_buffer = NULL;

    if (size == 0) {
        DBGLOG("id3v2_decode_string: Bad string size: a string should have at least 1 byte");
        return NULL;
    } else if (size < 2) {
        return NULL;
    }

    if (string[0] == '\x01') { // UTF-16 string with a BOM
        if (size <= 5) {
            if (size < 5) {
                DBGLOG("id3v2_decode_string: Bad BOM-UTF16 string size: %u < 5", (unsigned int)size);
            }
            return NULL;
        }

        const size_t copy_size = size - 3 + 2; // exclude 3 bytes of encoding hint, append 2 bytes for a NULL termination
        char *src_buffer = (char*)SDL_malloc(copy_size);
        if (!src_buffer) {
            return NULL; // Out of memory
        }
        SDL_memcpy(src_buffer, (string + 3), copy_size - 2);
        src_buffer[copy_size-2] = src_buffer[copy_size-1] = '\0';

        if (SDL_memcmp(string, "\x01\xFE\xFF", 3) == 0) { // UTF-16BE*/
            str_buffer = SDL_iconv_string("UTF-8", "UCS-2BE", src_buffer, copy_size);
        } else if (SDL_memcmp(string, "\x01\xFF\xFE", 3) == 0) {  // UTF-16LE*/
            str_buffer = SDL_iconv_string("UTF-8", "UCS-2LE", src_buffer, copy_size);
        }
        SDL_free(src_buffer);

    } else if (string[0] == '\x02') { // UTF-16BEstring without a BOM
        if (size <= 3) {
            if (size < 3) {
                DBGLOG("id3v2_decode_string: Bad UTF16BE string size: %u < 3", (unsigned int)size);
            }
            return NULL; // Blank string*/
        }

        const size_t copy_size = size - 1 + 2; // exclude 1 byte of encoding hint, append 2 bytes for a NULL termination
        char *src_buffer = (char*)SDL_malloc(copy_size);
        if (!src_buffer) {
            return NULL; // Out of memory
        }
        SDL_memcpy(src_buffer, (string + 1), copy_size - 2);
        src_buffer[copy_size-2] = src_buffer[copy_size-1] = '\0';

        str_buffer = SDL_iconv_string("UTF-8", "UCS-2BE", src_buffer, copy_size);
        SDL_free(src_buffer);

    } else if (string[0] == '\x03') { // UTF-8 string
        if (size <= 2) {
            return NULL; // Blank string*/
        }
        str_buffer = (char*)SDL_malloc(size);
        if (!str_buffer) {
            return NULL; // Out of memory
        }
        SDL_strlcpy(str_buffer, (const char*)(string + 1), size);

    } else if (string[0] == '\x00') { // Latin-1 string
        if (size <= 2) {
            return NULL; // Blank string*/
        }
        str_buffer = parse_id3v1_ansi_string((string + 1), size - 1);
    }

    return str_buffer;
}

// Write a tag string into internal meta-tags storage
static void set_id3v2_string_prop(SDL_PropertiesID props, const char *key, const Uint8 *string, size_t size)
{
    char *str_buffer = id3v2_decode_string(string, size);
    if (str_buffer) {
        SDL_SetStringProperty(props, key, str_buffer);
        SDL_free(str_buffer);
    }
}

// Identify a meta-key and decode the string (Note: input buffer should have at least 4 characters!)
static void handle_id3v2_string(SDL_PropertiesID props, const char *key, const Uint8 *string, size_t size)
{
    // put most text things in props in a generic "this is what the id3v2 key was" so apps can handle things we didn't pick out
    //  specifically, or new tags in the hypothetical future.
    if (SDL_memcmp(key+1, "XXX", 3) != 0) {   // !!! FIXME: we (currently) skip ?XXX because they aren't simple key/value pairs.
        char generic_key[64];
        SDL_snprintf(generic_key, sizeof (generic_key), "SDL_mixer.metadata.id3v2.%c%c%c%c", key[0], key[1], key[2], key[3]);
        if (key[0] == 'T') {  // all text keys start with 'T'
            set_id3v2_string_prop(props, generic_key, string, size);
        } else if (key[0] == 'W') {  // all URLs start with W.
            char *decoded = parse_id3v1_ansi_string(string, size);
            if (decoded) {
                SDL_SetStringProperty(props, generic_key, decoded);
                SDL_free(decoded);
            }
        }
    }

// TODO: Extract "Copyright message" from TXXX value: a KEY=VALUE string divided by a zero byte:*/
//  else if (SDL_memcmp(key, "TXXX", 4) == 0) {
//      set_id3v2_string_prop(props, MIX_META_COPYRIGHT, string, size);
//  }
}

// Identify a meta-key and decode the string (Note: input buffer should have at least 4 characters!)
static void handle_id3v2x2_string(SDL_PropertiesID props, const char *key, const Uint8 *string, size_t size)
{
    // put most text things in props in a generic "this is what the id3v2.2 key was" so apps can handle things we didn't pick out
    //  specifically, or new tags in the hypothetical future.
    if (SDL_memcmp(key+1, "XX", 2) != 0) {   // !!! FIXME: we (currently) skip ?XX because they aren't simple key/value pairs.
        char generic_key[64];
        SDL_snprintf(generic_key, sizeof (generic_key), "SDL_mixer.metadata.id3v2.%c%c%c", key[0], key[1], key[2]);
        if (key[0] == 'T') {  // all text keys start with 'T'
            set_id3v2_string_prop(props, generic_key, string, size);
        } else if (key[0] == 'W') {  // all URLs start with W.
            char *decoded = parse_id3v1_ansi_string(string, size);
            if (decoded) {
                SDL_SetStringProperty(props, generic_key, decoded);
                SDL_free(decoded);
            }
        }
    }
}

// Parse a frame in ID3v2.2 format
static size_t id3v22_parse_frame(SDL_PropertiesID props, SDL_IOStream *io, Uint8 *buffer)
{
    const Sint64 frame_begin = SDL_TellIO(io);
    size_t read_size = SDL_ReadIO(io, buffer, ID3v2_2_FRAME_HEADER_SIZE);

    if (read_size < ID3v2_2_FRAME_HEADER_SIZE) {
        DBGLOG("id3v22_parse_frame (1): Unexpected end of the file while frame header reading (had to read %u bytes, %u bytes wanted)",
                (unsigned int)read_size, (unsigned int)ID3v2_2_FRAME_HEADER_SIZE);
        SDL_SeekIO(io, frame_begin, SDL_IO_SEEK_SET);
        return 0; // Buffer size that left is too small
    }

    if (SDL_memcmp(buffer, "\0\0\0", 3) == 0) {
        SDL_SeekIO(io, frame_begin, SDL_IO_SEEK_SET);
        return 0;
    }

    const char key[4] = { buffer[0], buffer[1], buffer[2], 0 };   // Tag title (key)

    const size_t size = (size_t)read_sint24be(buffer + ID3v2_FIELD_FRAME_SIZEv2);
    const size_t expected_size = SDL_min(size, ID3v2_BUFFER_SIZE);
    read_size = SDL_ReadIO(io, buffer, expected_size);
    if (read_size < expected_size) {
        DBGLOG("id3v22_parse_frame (2): Unexpected end of the file while frame data reading (had to read %u bytes, %u bytes wanted)",
            (unsigned int)read_size, (unsigned int)expected_size);
        SDL_SeekIO(io, frame_begin, SDL_IO_SEEK_SET);
        return 0; // Can't read frame data, possibly, a file size was reached
    }
    SDL_SeekIO(io, frame_begin + (Sint64)size + ID3v2_2_FRAME_HEADER_SIZE, SDL_IO_SEEK_SET);

    handle_id3v2x2_string(props, key, buffer, read_size);

    return size + ID3v2_2_FRAME_HEADER_SIZE; // data size + size of the header
}

// Parse a frame in ID3v2.3 and ID3v2.4 formats
static size_t id3v2x_parse_frame(SDL_PropertiesID props, SDL_IOStream *io, Uint8 *buffer, Uint8 version)
{
    const Sint64 frame_begin = SDL_TellIO(io);

    size_t read_size = SDL_ReadIO(io, buffer, ID3v2_3_FRAME_HEADER_SIZE);

    if (read_size < ID3v2_3_FRAME_HEADER_SIZE) {
        DBGLOG("id3v2x_parse_frame (1): Unexpected end of the file while frame header reading (had to read %u bytes, %u bytes wanted)",
                (unsigned int)read_size,
                (unsigned int)ID3v2_3_FRAME_HEADER_SIZE);
        SDL_SeekIO(io, frame_begin, SDL_IO_SEEK_SET);
        return 0; // Can't read frame header, possibly, a file size was reached
    }

    if (SDL_memcmp(buffer, "\0\0\0\0", 4) == 0) {
        SDL_SeekIO(io, frame_begin, SDL_IO_SEEK_SET);
        return 0;
    }

    const char key[5] = { buffer[0], buffer[1], buffer[2], buffer[3], 0 };   // Tag title (key)

    size_t size;
    if (version == 4) {
        size = (size_t)id3v2_synchsafe_decode(buffer + ID3v2_FIELD_FRAME_SIZE);
    } else {
        size = (size_t)read_sint32be(buffer + ID3v2_FIELD_FRAME_SIZE);
    }

    const size_t expected_size = SDL_min(size, ID3v2_BUFFER_SIZE);
    read_size = SDL_ReadIO(io, buffer, expected_size);
    if (read_size < expected_size) {
        DBGLOG("id3v2x_parse_frame (2): Unexpected end of the file while frame data reading (had to read %u bytes, %u bytes wanted)",
                (unsigned int)read_size, (unsigned int)expected_size);
        SDL_SeekIO(io, frame_begin, SDL_IO_SEEK_SET);
        return 0; // Can't read frame data, possibly, a file size was reached
    }
    SDL_SeekIO(io, frame_begin + (Sint64)size + ID3v2_3_FRAME_HEADER_SIZE, SDL_IO_SEEK_SET);

    buffer[read_size] = '\0';  // make sure it's definitely null-terminated.
    handle_id3v2_string(props, key, buffer, read_size);

    return size + ID3v2_3_FRAME_HEADER_SIZE; // data size + size of the header
}

// Parse content of ID3v2. This expects the stream to be seeked to the start of the potential header.
static bool parse_id3v2(SDL_PropertiesID props, SDL_IOStream *io)
{
    Uint8 buffer[ID3v2_BUFFER_SIZE + 1];
    size_t read_size;
    size_t frame_length;

    Sint64 total_length = 0;

    read_size = SDL_ReadIO(io, buffer, ID3v2_HEADER_SIZE); // Retrieve the header
    if (read_size < ID3v2_HEADER_SIZE) {
        DBGLOG("parse_id3v2: fail to read a header (%u < 10)", (unsigned int)read_size);
        return false; // Unsupported version of the tag
    }

    total_length += ID3v2_HEADER_SIZE;

    const Uint8 version_major = buffer[ID3v2_FIELD_VERSION_MAJOR]; // Major version
    // version_minor = buffer[ID3v2_VERSION_MINOR]; // Minor version, UNUSED
    const Uint8 flags = buffer[ID3v2_FIELD_HEAD_FLAGS]; // Flags
    Sint64 tag_len = id3v2_synchsafe_decode(buffer + ID3v2_FIELD_TAG_LENGTH); // Length of a tag

    if (version_major != 2 && version_major != 3 && version_major != 4) {
        DBGLOG("parse_id3v2: Unsupported version %d", version_major);
        return false; // Unsupported version of the tag
    }

    Sint64 tag_extended_len = 0;
    if ((version_major > 2) && ((flags & ID3v2_FLAG_HAS_EXTRA_HEAD) == ID3v2_FLAG_HAS_EXTRA_HEAD)) {
        if (SDL_ReadIO(io, buffer + ID3v2_FIELD_EXTRA_HEADER_LENGTH, 4) != 4) {
            return false;
        } else if (SDL_SeekIO(io, -4, SDL_IO_SEEK_CUR) < 0) {
            return false;
        }
        tag_extended_len = id3v2_synchsafe_decode(buffer + ID3v2_FIELD_EXTRA_HEADER_LENGTH); // Length of an extended header
    }

    if (tag_extended_len) {
        tag_len -= tag_extended_len; // Subtract the size of extended header
        if (SDL_SeekIO(io, tag_extended_len, SDL_IO_SEEK_CUR) < 0) { // Skip extended header and it's size value
            return false;
        }
    }

    total_length += tag_len;

    if (flags & ID3v2_FLAG_HAS_FOOTER) {
        total_length += ID3v2_HEADER_SIZE; // footer size
    }

    Sint64 pos = SDL_TellIO(io);

    if ((pos + tag_len) > SDL_GetIOSize(io)) {
        DBGLOG("parse_id3v2: Tag size bigger than actual file size");
        return false; // Tag size is bigger than actual buffer data
    }

    while ((pos >= 0) && (pos < total_length)) {
        if (version_major == 2) {
            frame_length = id3v22_parse_frame(props, io, buffer);
        } else {
            frame_length = id3v2x_parse_frame(props, io, buffer, version_major);
        }
        if (!frame_length) {
            break;
        }
        pos = SDL_TellIO(io);
    }

    return true;
}

/********************************************************
 *                  APE v1 and v2                       *
 ********************************************************/

#define APE_BUFFER_SIZE     256

#define APE_V1              1000U
#define APE_V2              2000U
#define APE_HEADER_SIZE     32

#define APE_HEAD_FIELD_VERSION      8
#define APE_HEAD_FIELD_TAGSIZE      12
#define APE_HEAD_FIELD_ITEMS_COUNT  16
#define APE_HEAD_FIELD_FLAGS        20
#define APE_HEAD_FIELD_RESERVED     24

#define APE_FRAME_TAG_KEY       4

static bool is_apetag(const Uint8 *data, size_t length)
{
    // https://wiki.hydrogenaud.io/index.php?title=APEv2_specification
    // Header/footer is 32 bytes: bytes 0-7 ident, bytes 8-11 version,
    // bytes 12-17 size. bytes 24-31 are reserved: must be all zeroes.

    if ((length < 32) || (SDL_memcmp(data,"APETAGEX", 8) != 0)) {
        return false;
    }

    Uint32 v = (Uint32) read_sint32le(data + 8); // version
    if (v != APE_V2 && v != APE_V1) {
        return false;
    }
    v = 0; // reserved bits :
    if (SDL_memcmp(&data[24],&v,4) != 0 || SDL_memcmp(&data[28],&v,4) != 0) {
        return false;
    }
    return true;
}

static Sint64 get_ape_len(const Uint8 *data, Uint32 *version)
{
    Sint64 size = (Sint64) read_sint32le(data + APE_HEAD_FIELD_TAGSIZE);
    *version = (Uint32) read_sint32le(data + APE_HEAD_FIELD_VERSION);
    const Uint32 flags = (Uint32) read_sint32le(data + APE_HEAD_FIELD_FLAGS);
    if ((*version == APE_V2) && (flags & (1U<<31))) {
        size += APE_HEADER_SIZE; // header present.
    }
    return size;
}

static char *ape_find_value(char *key)
{
    const char *end = (key + APE_BUFFER_SIZE - 4);
    while (*key && (key != end)) {
        key++;
    }

    return (key >= end) ? NULL : key + 1;
}

static Uint32 ape_handle_tag(SDL_PropertiesID props, Uint8 *data, size_t valsize)
{
    /* https://wiki.hydrogenaud.io/index.php?title=APE_Tag_Item
     * Tag entry has unclear size because of no size value for a key field
     * However, we only know next sizes:
     * - 4 bytes is a [length] of value field
     * - 4 bytes of value-specific flags
     * - unknown length of a key field. To detect its size
     *   it's need to find a zero byte looking at begin of the key field
     * - 1 byte of a null-terminator
     * - [length] bytes a value content
     */
    char *key = (char*)(data + APE_FRAME_TAG_KEY);
    char *value = ape_find_value(key);
    if (!value) {
        return 0;
    }

    const Uint32 key_len = (Uint32)(value - key);

    if (valsize > (APE_BUFFER_SIZE - key_len)) {
        // maybe it's a list? convert embedded null chars to newlines. Note this will mess up binary data, but the APE spec doesn't currently list any binary keys.
        for (size_t i = 0; i < APE_BUFFER_SIZE; i++) {
            if (data[i] == '\0') {
                data[i] = '\n';
            }
        }
        data[APE_BUFFER_SIZE] = '\0';  // null-terminate the data.
    } else {
        // maybe it's a list? convert embedded null chars to newlines. Note this will mess up binary data, but the APE spec doesn't currently list any binary keys.
        for (size_t i = 0; i < valsize; i++) {
            if (value[i] == '\0') {
                value[i] = '\n';
            }
        }
        value[valsize] = '\0';  // null-terminate the data.
    }

    char apekey[256];
    if (key_len < sizeof (apekey)) {
        for (Uint32 i = 0; i < key_len; i++) {
            apekey[i] = SDL_tolower(key[i]);
        }
        apekey[key_len] = '\0';
        char generic_key[256];
        const int rc = SDL_snprintf(generic_key, sizeof (generic_key), "SDL_mixer.metadata.ape.%s", apekey);
        if ((rc > 0) && (rc < (int)sizeof(generic_key))) {
            if (!SDL_HasProperty(props, generic_key)) {  // in case there are multiple ID3v1 tags appended to a file, we'll take the last one, since we parse backwards from the end of file.
                SDL_SetStringProperty(props, generic_key, value);
            }
        }
    }

    return 4 + (Uint32)valsize + key_len;
}

// Parse content of APE tag
static bool parse_ape(SDL_PropertiesID props, SDL_IOStream *io, Sint64 ape_head_pos, Uint32 version)
{
    Uint8 buffer[APE_BUFFER_SIZE + 1];
    Uint32 v, i, tag_size, tag_items_count, tag_item_size;
    size_t read_size;

    if (SDL_SeekIO(io, ape_head_pos, SDL_IO_SEEK_SET) != ape_head_pos) {
        return false;
    }

    read_size = SDL_ReadIO(io, buffer, APE_HEADER_SIZE); // Retrieve the header
    if (read_size < APE_HEADER_SIZE) {
        SDL_SeekIO(io, ape_head_pos, SDL_IO_SEEK_SET);
        return false;
    }

    v = (Uint32)read_sint32le(buffer + APE_HEAD_FIELD_VERSION); // version
    if (v != APE_V2 && v != APE_V1) {
        return false;
    }

    tag_size = (Uint32)read_sint32le(buffer + APE_HEAD_FIELD_TAGSIZE); // tag size

    if (version == APE_V1) { // If version 1, we are at footer
        if (ape_head_pos - (tag_size - APE_HEADER_SIZE) < 0) {
            SDL_SeekIO(io, ape_head_pos, SDL_IO_SEEK_SET);
            return false;
        }
        SDL_SeekIO(io, ape_head_pos - (tag_size - APE_HEADER_SIZE), SDL_IO_SEEK_SET);
    }

    tag_items_count = (Uint32)read_sint32le(buffer + APE_HEAD_FIELD_ITEMS_COUNT); // count tag items

    //flags = (Uint32)read_sint32be(buffer + APE_HEAD_FIELD_FLAGS); // global flags, unused

    // reserved bits :
    if (*((Uint64 *) (buffer + APE_HEAD_FIELD_RESERVED)) != 0) {
        return false;
    }

    for(i = 0; i < tag_items_count; i++) {
        const Sint64 cur_tag = SDL_TellIO(io);
        if (cur_tag < 0) {
            break;
        }
        read_size = SDL_ReadIO(io, buffer, 4); // Retrieve the size
        if (read_size < 4) {
            SDL_SeekIO(io, ape_head_pos, SDL_IO_SEEK_SET);
            return false;
        }

        v = (Uint32)read_sint32le(buffer); // size of the tag's value field
        // (we still need to find key size by a null termination)

        // Retrieve the tag's data with an aproximal size as we can
        read_size = SDL_ReadIO(io, buffer, SDL_min(v + 40, APE_BUFFER_SIZE));
        buffer[read_size] = '\0';

        tag_item_size = ape_handle_tag(props, buffer, v);
        if (tag_item_size == 0) {
            break;
        }
        if (SDL_SeekIO(io, cur_tag + tag_item_size + 4, SDL_IO_SEEK_SET) < 0) {
            return false;
        }
    }

    return (SDL_SeekIO(io, ape_head_pos, SDL_IO_SEEK_SET) == ape_head_pos);
}


/********************************************************
 *                   Lyrics3 skip                       *
 ********************************************************/

/* Header    : "LYRICSBEGIN"   -- 11 bytes
 * Size field: (decimal) (v2 only) 6 bytes
 * End marker: "LYRICS200" (v2) -  9 bytes
 * End marker: "LYRICSEND" (v1) -  9 bytes
 *
 * The maximum length of Lyrics3v1 is 5100 bytes.
 */

#define LYRICS3v1_SEARCH_BUFFER     5120 // 5100 + 20 of tag begin and end keywords

#define LYRICS3v1_HEAD_SIZE         11
#define LYRICS3v1_TAIL_SIZE         9
#define LYRICS3v2_TAG_SIZE_VALUE    6
#define LYRICS3_FOOTER_SIZE         15

static SDL_INLINE int is_lyrics3tag(const Uint8 *data, size_t length)
{
    // https://id3.org/Lyrics3
    // https://id3.org/Lyrics3v2
    if (length < LYRICS3_FOOTER_SIZE) return 0;
    if (SDL_memcmp(data+LYRICS3v2_TAG_SIZE_VALUE,"LYRICS200",9) == 0) return 2; // v2
    if (SDL_memcmp(data+LYRICS3v2_TAG_SIZE_VALUE,"LYRICSEND",9) == 0) return 1; // v1
    return 0;
}
static Sint64 get_lyrics3v1_len(SDL_IOStream *io)
{
    // needs manual search:  https://id3.org/Lyrics3
    const Sint64 flen = SDL_GetIOSize(io);
    if (flen < 20) {
        return -1;
    }
    Sint64 len = SDL_min(flen, LYRICS3v1_SEARCH_BUFFER);
    if (SDL_SeekIO(io, -len, SDL_IO_SEEK_END) < 0) {
        return -1;
    }

    char buf[LYRICS3v1_SEARCH_BUFFER + 1];
    SDL_zeroa(buf);
    const size_t readamount = (size_t)(len -= LYRICS3v1_TAIL_SIZE); // exclude footer
    if (SDL_ReadIO(io, buf, readamount) != readamount) {
        return -1;
    }

    // strstr() won't work here.
    Sint64 i = -1;
    const char *p = buf;
    for (i = len - LYRICS3v1_HEAD_SIZE; i >= 0; --i, ++p) {
        if (SDL_memcmp(p, "LYRICSBEGIN", LYRICS3v1_HEAD_SIZE) == 0) {
            break;
        }
    }
    if (i < 0) {
        return -1;
    }
    return len - (Sint64)(p - buf) + LYRICS3v1_TAIL_SIZE; // footer
}

static Sint64 get_lyrics3v2_len(const Uint8 *data, size_t length)
{
    // 6 bytes before the end marker is size in decimal format -
    // does not include the 9 bytes end marker and size field.
    return (length != LYRICS3v2_TAG_SIZE_VALUE) ? 0 : (Sint64) (SDL_strtol((const char *)data, NULL, 10) + LYRICS3_FOOTER_SIZE);
}

static bool verify_lyrics3v2(const Uint8 *data, size_t length)
{
    return ((length >= LYRICS3v1_HEAD_SIZE) && (SDL_memcmp(data, "LYRICSBEGIN", LYRICS3v1_HEAD_SIZE) == 0));
}


/********************************************************
 *                    MusicMatch                        *
 ********************************************************/

#define MUSICMATCH_HEADER_SIZE          256
#define MUSICMATCH_VERSION_INFO_SIZE    256
#define MUSICMATCH_FOOTER_SIZE          48
#define MUSICMATCH_OFFSETS_SIZE         20

#define MMTAG_PARANOID

static bool is_musicmatch(const Uint8 *data, Sint64 length)
{
   /* From docs/musicmatch.txt in id3lib: https://sourceforge.net/projects/id3lib/
      Overall tag structure:

      +-----------------------------+
      |           Header            |
      |    (256 bytes, OPTIONAL)    |
      +-----------------------------+
      |  Image extension (4 bytes)  |
      +-----------------------------+
      |        Image binary         |
      |  (var. length >= 4 bytes)   |
      +-----------------------------+
      |      Unused (4 bytes)       |
      +-----------------------------+
      |  Version info (256 bytes)   |
      +-----------------------------+
      |       Audio meta-data       |
      | (var. length >= 7868 bytes) |
      +-----------------------------+
      |   Data offsets (20 bytes)   |
      +-----------------------------+
      |      Footer (48 bytes)      |
      +-----------------------------+
     */
    if (length < MUSICMATCH_FOOTER_SIZE) {
        return false;
    }
    // sig: 19 bytes company name + 13 bytes space
    else if (SDL_memcmp(data, "Brava Software Inc.             ", 32) != 0) {
        return false;
    }
    // 4 bytes version: x.xx
    else if (!SDL_isdigit(data[32]) || data[33] != '.' ||
             !SDL_isdigit(data[34]) ||!SDL_isdigit(data[35])) {
        return false;
    }
    #ifdef MMTAG_PARANOID
    // [36..47]: 12 bytes trailing space
    for (length = 36; length < MUSICMATCH_FOOTER_SIZE; ++length) {
        if (data[length] != ' ') return false;
    }
    #endif
    return true;
}

static Sint64 get_musicmatch_len(SDL_IOStream *io)
{
    static const Sint32 metasizes[4] = { 7868, 7936, 8004, 8132 };
    static const unsigned char syncstr[10] = {'1', '8', '2', '7', '3', '6', '4', '5', 0, 0};
    Uint8 buf[256];
    Sint64 len = 0;
    Sint32 i, j;

    if (SDL_SeekIO(io, -68, SDL_IO_SEEK_END) < 0) {
        return -1;
    } else if (SDL_ReadIO(io, buf, 20) != 20) {
        return -1;
    }

    const Sint32 imgext_ofs  = (Sint32)((buf[3] <<24) | (buf[2] <<16) | (buf[1] <<8) | buf[0] );
    const Sint32 version_ofs = (Sint32)((buf[15]<<24) | (buf[14]<<16) | (buf[13]<<8) | buf[12]);
    if (version_ofs <= imgext_ofs) {
        return -1;
    } else if ((version_ofs <= 0) || (imgext_ofs <= 0)) {
        return -1;
    }

    /* Try finding the version info section:
     * Because metadata section comes after it, and because metadata section
     * has different sizes across versions (format ver. <= 3.00: always 7868
     * bytes), we can _not_ directly calculate using deltas from the offsets
     * section. */
    for (i = 0; i < 4; ++i) {
        // 48: footer, 20: offsets, 256: version info
        len = metasizes[i] + MUSICMATCH_FOOTER_SIZE + MUSICMATCH_OFFSETS_SIZE + MUSICMATCH_VERSION_INFO_SIZE;
        if (SDL_SeekIO(io, -len, SDL_IO_SEEK_END) < 0) {
            return -1;
        } else if (SDL_ReadIO(io, buf, MUSICMATCH_VERSION_INFO_SIZE) != MUSICMATCH_VERSION_INFO_SIZE) {
            return -1;
        }
        // [0..9]: sync string, [30..255]: 0x20
        #ifdef MMTAG_PARANOID
        for (j = 30; j < MUSICMATCH_VERSION_INFO_SIZE; ++j) {
            if (buf[j] != ' ') {
                break;
            }
        }
        if (j < MUSICMATCH_VERSION_INFO_SIZE) continue;
        #endif
        if (SDL_memcmp(buf, syncstr, 10) == 0) {
            break;
        }
    }
    if (i == 4) {
        return -1; // no luck.
    }

    #ifdef MMTAG_PARANOID
    // unused section: (4 bytes of 0x00)
    if (SDL_SeekIO(io, -(len + 4), SDL_IO_SEEK_END) < 0) {
        return -1;
    } else if (SDL_ReadIO(io, &j, 4) != 4) {
        return -1;
    } else if (j != 0) {
        return -1;
    }
    #endif

    len += (version_ofs - imgext_ofs);
    if (SDL_SeekIO(io, -len, SDL_IO_SEEK_END) < 0) {
        return -1;
    } else if (SDL_ReadIO(io, buf, 8) != 8) {
        return -1;
    }
    j = (Sint32)((buf[7] <<24) | (buf[6] <<16) | (buf[5] <<8) | buf[4]);
    if (j < 0) return -1;
    // verify image size:
    // without this, we may land at a wrong place.
    if (j + 12 != version_ofs - imgext_ofs) return -1;
    // try finding the optional header
    if (SDL_SeekIO(io, -(len + MUSICMATCH_HEADER_SIZE), SDL_IO_SEEK_END) < 0) {
        return -1;
    } else if (SDL_ReadIO(io, buf, MUSICMATCH_HEADER_SIZE) != MUSICMATCH_HEADER_SIZE) {
        return -1;
    }
    // [0..9]: sync string, [30..255]: 0x20
    if (SDL_memcmp(buf, syncstr, 10) != 0) {
        return len;
    }
    #ifdef MMTAG_PARANOID
    for (j = 30; j < MUSICMATCH_HEADER_SIZE; ++j) {
        if (buf[j] != ' ') {
            return len;
        }
    }
    #endif
    return len + MUSICMATCH_HEADER_SIZE; // header is present.
}


#define TAG_FOUND       1
#define TAG_INVALID    -1
#define TAG_NOT_FOUND   0

static int probe_id3v1(SDL_PropertiesID props, SDL_IOStream *io, Uint8 *buf, int atend, MIX_IoClamp *clamp)
{
    if (SDL_SeekIO(io, -ID3v1_TAG_SIZE, SDL_IO_SEEK_END) < 0) {
        return TAG_INVALID;
    } else if (SDL_ReadIO(io, buf, ID3v1_TAG_SIZE) != ID3v1_TAG_SIZE) {
        return TAG_INVALID;
    } else if (is_id3v1(buf, ID3v1_TAG_SIZE)) {
        if (!atend) { // possible false positive?
            if (is_musicmatch(buf + 128 - 48, 48) ||
                is_apetag    (buf + 128 - 32, 32) ||
                is_lyrics3tag(buf + 128 - 15, 15)) {
                return TAG_NOT_FOUND;
            }
        }
        parse_id3v1(props, buf);
        clamp->length -= ID3v1_TAG_SIZE;
        return TAG_FOUND;
        // FIXME: handle possible double-ID3v1 tags??
    }

    return TAG_NOT_FOUND;
}

static int probe_mmtag(SDL_PropertiesID props, SDL_IOStream *io, Uint8 *buf, MIX_IoClamp *clamp)
{
    (void)props; // !!! FIXME: Implement reading tag contents.
    if (SDL_SeekIO(io, -MUSICMATCH_FOOTER_SIZE, SDL_IO_SEEK_END) < 0) {
        return TAG_INVALID;
    } else if (SDL_ReadIO(io, buf, MUSICMATCH_FOOTER_SIZE) != MUSICMATCH_FOOTER_SIZE) {
        return TAG_INVALID;
    } else if (is_musicmatch(buf, MUSICMATCH_FOOTER_SIZE)) {
        const Sint64 len = get_musicmatch_len(io);
        if (len < 0) {
            return TAG_INVALID;
        }
        clamp->length -= len;
        return TAG_FOUND;
    }
    return TAG_NOT_FOUND;
}

static int probe_apetag(SDL_PropertiesID props, SDL_IOStream *io, Uint8 *buf, MIX_IoClamp *clamp)
{
    // APE tag may be at the end: read the footer
    if (SDL_SeekIO(io, -APE_HEADER_SIZE, SDL_IO_SEEK_END) < 0) {
        return TAG_INVALID;
    } else if (SDL_ReadIO(io, buf, APE_HEADER_SIZE) != APE_HEADER_SIZE) {
        return TAG_INVALID;
    }

    // APE tag may be at end or before ID3v1 tag
    int retval = TAG_NOT_FOUND;
    if (is_apetag(buf, APE_HEADER_SIZE)) {
        Uint32 v = 0;
        const Sint64 len = get_ape_len(buf, &v);
        if (v == APE_V2) { // verify header :
            if (SDL_SeekIO(io, -len, SDL_IO_SEEK_END) < 0) {
                return TAG_INVALID;
            } else if (SDL_ReadIO(io, buf, APE_HEADER_SIZE) != APE_HEADER_SIZE) {
                return TAG_INVALID;
            } else if (!is_apetag(buf, APE_HEADER_SIZE)) {
                retval = TAG_NOT_FOUND;
            } else if (parse_ape(props, io, SDL_TellIO(io) - APE_HEADER_SIZE, APE_V2)) {
                retval = TAG_FOUND;
            }
        } else {
            if (SDL_SeekIO(io, -APE_HEADER_SIZE, SDL_IO_SEEK_END) < 0) {
                return TAG_INVALID;
            } else if (parse_ape(props, io, SDL_TellIO(io), APE_V1)) {
                retval = TAG_FOUND;
            }
        }
        clamp->length -= len;
    }

    return retval;
}

static int probe_lyrics3(SDL_IOStream *io, Uint8 *buf, MIX_IoClamp *clamp)
{
    if (SDL_SeekIO(io, -LYRICS3_FOOTER_SIZE, SDL_IO_SEEK_END) < 0) {
        return TAG_INVALID;
    } else if (SDL_ReadIO(io, buf, LYRICS3_FOOTER_SIZE) != LYRICS3_FOOTER_SIZE) {
        return TAG_INVALID;
    }

    const int ver = is_lyrics3tag(buf, LYRICS3_FOOTER_SIZE);
    if (ver == 2) {
        const Sint64 len = get_lyrics3v2_len(buf, LYRICS3v2_TAG_SIZE_VALUE);
        if (len < LYRICS3_FOOTER_SIZE) {
            return TAG_INVALID;
        } else if (SDL_SeekIO(io, -len, SDL_IO_SEEK_END) < 0) {
            return TAG_INVALID;
        } else if (SDL_ReadIO(io, buf, LYRICS3v1_HEAD_SIZE) != LYRICS3v1_HEAD_SIZE) {
            return TAG_INVALID;
        } else if (!verify_lyrics3v2(buf, LYRICS3v1_HEAD_SIZE)) {
            return TAG_INVALID;
        }
        clamp->length -= len;
        return TAG_FOUND;
    } else if (ver == 1) {
        const Sint64 len = get_lyrics3v1_len(io);
        if (len < 0) {
            return TAG_INVALID;
        }
        clamp->length -= len;
        return TAG_FOUND;
    }
    return TAG_NOT_FOUND;
}

static void ParseTrackNumString(const char *str, Sint64 *track, Sint64 *total_tracks)
{
    SDL_assert(track != NULL);
    SDL_assert(total_tracks != NULL);

    *track = *total_tracks = -1;

    if (!str) {
        return;
    }

    const char *trackstr = str;
    const char *totalstr = NULL;
    char *cpy = NULL;
    char *ptr = SDL_strchr(str, '/');  // see if it has both track _and_ total tracks.

    if (ptr) {
        cpy = SDL_strdup(str);
        if (!cpy) {
            return;  // oh well.
        }
        ptr = cpy + (ptr - str);
        *ptr = '\0';
        trackstr = cpy;
        totalstr = ptr + 1;
    }

    SDL_assert(trackstr != NULL);

    char *endp = NULL;
    Sint64 ivalue;

    ivalue = (Sint64) SDL_strtoll(trackstr, &endp, 10);
    if ((*trackstr != '\0') && (*endp == '\0')) {  // if true, entire string was a valid number.
        if (ivalue >= 0) {  // reject negative numbers, though.
            *track = ivalue;
        }
    }

    if ((*track > 0) && (totalstr != NULL)) {
        endp = NULL;
        ivalue = (Sint64) SDL_strtoll(trackstr, &endp, 10);
        if ((*totalstr != '\0') && (*endp == '\0')) {  // if true, entire string was a valid number.
            if (ivalue >= 0) {  // reject negative numbers, though.
                *total_tracks = ivalue;
            }
        }
    }

    SDL_free(cpy);
}


// !!! FIXME: several invalid tag things might cause a false return from here, but we should
// !!! FIXME:  just parse and clamp out what we can and only return false for legit i/o errors.
// !!! FIXME:  This will take a little work to clean out.
// !!! FIXME: as it stands, SDL_mixer ignores this return value and just accepts any new clamps
// !!! FIXME:  and properties. If there was a legit i/o error, it's going to find it shortly
// !!! FIXME:  as it tries to read the audio data anyhow.
bool MIX_ReadMetadataTags(SDL_IOStream *io, SDL_PropertiesID props, MIX_IoClamp *clamp)
{
    Uint8 buf[TAGS_INPUT_BUFFER_SIZE];

    /* MP3 standard has no metadata format, so everyone invented
     * their own thing, even with extensions, until ID3v2 became
     * dominant: Hence the impossible mess here.
     *
     * Note: I don't yet care about freaky broken mp3 files with
     * double tags. -- O.S.
     */

    // grab a block sufficient for initial tag discovery.
    const size_t readsize = SDL_ReadIO(io, buf, sizeof (buf));
    if (readsize == 0) {
        return false;
    }

    // ID3v2 tag is at the start
    if (is_id3v2(buf, readsize)) {
        const Sint32 id3len = get_id3v2_len(buf, readsize);
        if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) != 0) {
            return false;
        }
        parse_id3v2(props, io);
        clamp->start += id3len;
        clamp->length -= id3len;
    }

    // APE tag _might_ be at the start (discouraged
    // but not forbidden, either.)  read the header.
    else if (is_apetag(buf, readsize)) {
        Uint32 v = 0;
        const Sint64 apelen = get_ape_len(buf, &v);
        if ((v == APE_V1) || (v == APE_V2)) {
            parse_ape(props, io, 0, v);
        }
        clamp->start += apelen;
        clamp->length -= apelen;
    }

    // we do not know the order of ape or lyrics3 or musicmatch tags (or if an extra tag was appended after a previous one instead of replacing it), hence the loop here..
    bool found_any = false;
    bool found = false;
    do {
        found = false;
        // it's not impossible that _old_ MusicMatch tag
        // placing itself after ID3v1.
        if (probe_mmtag(props, io, buf, clamp) == TAG_FOUND) {
            found = true;
        }

        if (probe_id3v1(props, io, buf, !found_any, clamp) == TAG_FOUND) {
            found = true;
        }

        if (probe_lyrics3(io, buf, clamp) == TAG_FOUND) {
            found = true;
        }

        if (probe_mmtag(props, io, buf, clamp) == TAG_FOUND) {
            found = true;
        }

        if (probe_apetag(props, io, buf, clamp) == TAG_FOUND) {
            found = true;
        }

        if (found) {
            found_any = true;
        }
    } while (found);

    // Some tags we turn into standard SDL_mixer metadata keys...
    // favor them in this order: ID3v2.3+, ID3v2.2, APE, ID3v1 (maybe MusicMatch, eventually).
    static const char * const title_keys[] = { "SDL_mixer.metadata.id3v2.TIT2", "SDL_mixer.metadata.id3v2.TT2", "SDL_mixer.metadata.ape.title", "SDL_mixer.metadata.id3v1.title" };
    static const char * const artist_keys[] = { "SDL_mixer.metadata.id3v2.TPE1", "SDL_mixer.metadata.id3v2.TP1", "SDL_mixer.metadata.ape.artist", "SDL_mixer.metadata.id3v1.artist" };
    static const char * const album_keys[] = { "SDL_mixer.metadata.id3v2.TALB", "SDL_mixer.metadata.id3v2.TAL", "SDL_mixer.metadata.ape.album", "SDL_mixer.metadata.id3v1.album" };
    static const char * const copyright_keys[] = { "SDL_mixer.metadata.id3v2.TCOP", "SDL_mixer.metadata.id3v2.TCR", "SDL_mixer.metadata.ape.copyright", "SDL_mixer.metadata.id3v1.comment" };
    static const char * const year_keys[] = { "SDL_mixer.metadata.id3v2.TYER", "SDL_mixer.metadata.id3v2.TYE", "SDL_mixer.metadata.ape.year", "SDL_mixer.metadata.id3v1.year" };

    static const struct { const char *mixer; SDL_PropertyType type; const char * const * tags; } tagmap[] = {
        { MIX_PROP_METADATA_TITLE_STRING, SDL_PROPERTY_TYPE_STRING, title_keys },
        { MIX_PROP_METADATA_ARTIST_STRING, SDL_PROPERTY_TYPE_STRING, artist_keys },
        { MIX_PROP_METADATA_ALBUM_STRING, SDL_PROPERTY_TYPE_STRING, album_keys},
        { MIX_PROP_METADATA_COPYRIGHT_STRING, SDL_PROPERTY_TYPE_STRING, copyright_keys },
        { MIX_PROP_METADATA_YEAR_NUMBER, SDL_PROPERTY_TYPE_NUMBER, year_keys },
    };

    for (size_t i = 0; i < SDL_arraysize(tagmap); i++) {
        const char *mixer = tagmap[i].mixer;
        if (!SDL_HasProperty(props, mixer)) {
            for (size_t j = 0; j < SDL_arraysize(title_keys); j++) {
                const char *str = SDL_GetStringProperty(props, tagmap[i].tags[j], NULL);
                if (str) {
                    if (tagmap[i].type == SDL_PROPERTY_TYPE_STRING) {
                        SDL_SetStringProperty(props, mixer, str);
                    } else if (tagmap[i].type == SDL_PROPERTY_TYPE_NUMBER) {
                        char *endp = NULL;
                        const Sint64 ivalue = (Sint64) SDL_strtoll(str, &endp, 10);
                        if ((*str != '\0') && (*endp == '\0')) {  // if true, entire string was a valid number.
                            SDL_SetNumberProperty(props, mixer, ivalue);
                        }
                    }
                    break;
                }
            }
        }
    }

    static const char * const tracknum_keys[] = { "SDL_mixer.metadata.id3v2.TRCK", "SDL_mixer.metadata.id3v2.TRK", "SDL_mixer.metadata.ape.track", "SDL_mixer.metadata.id3v1.track" };
    for (size_t j = 0; j < SDL_arraysize(tracknum_keys); j++) {
        Sint64 track, total_tracks;
        ParseTrackNumString(SDL_GetStringProperty(props, tracknum_keys[j], NULL), &track, &total_tracks);
        if (track > 0) {
            SDL_SetNumberProperty(props, MIX_PROP_METADATA_TRACK_NUMBER, track);
            if (total_tracks > 0) {
                SDL_SetNumberProperty(props, MIX_PROP_METADATA_TOTAL_TRACKS_NUMBER, total_tracks);
            }
            break;
        }
    }

    return true;
}


static bool IsOggLoopTag(const char *tag)
{
    char buf[5];
    SDL_strlcpy(buf, tag, 5);
    return SDL_strcasecmp(buf, "LOOP") == 0;
}

static Sint64 ParseOggTime(char *time, long samplerate_hz)
{
    char *num_start, *p;
    Sint64 result;
    char c;
    int val;

    /* Time is directly expressed as a sample position */
    if (SDL_strchr(time, ':') == NULL) {
        return SDL_strtoll(time, NULL, 10);
    }

    result = 0;
    num_start = time;

    for (p = time; *p != '\0'; ++p) {
        if (*p == '.' || *p == ':') {
            c = *p; *p = '\0';
            if ((val = SDL_atoi(num_start)) < 0)
                return -1;
            result = result * 60 + val;
            num_start = p + 1;
            *p = c;
        }

        if (*p == '.') {
            double val_f = SDL_atof(p);
            if (val_f < 0) return -1;
            return result * samplerate_hz + (Sint64) (val_f * samplerate_hz);
        }
    }

    if ((val = SDL_atoi(num_start)) < 0) return -1;
    return (result * 60 + val) * samplerate_hz;
}

void MIX_ParseOggComments(SDL_PropertiesID props, int freq, const char *vendor, const char * const *user_comments, int num_comments, MIX_OggLoop *loop)
{
    if (vendor && *vendor) {
        SDL_SetStringProperty(props, "SDL_mixer.metadata.ogg.vendor", vendor);
    }

    loop->start = loop->len = loop->end = -1;
    loop->count = -1;  // loops are infinite if a loop was specified but iteration count was not.

    bool is_loop_length = false;
    for (int i = 0; i < num_comments; i++) {
        char *param = SDL_strdup(user_comments[i]);
        if (!param) {
            continue;  // maybe better luck on other comments. But you're probably in big trouble now.
        }

        char *argument = param;
        char *value = SDL_strchr(param, '=');
        if (value == NULL) {
            value = param + SDL_strlen(param);
        } else {
            *(value++) = '\0';
        }

        // Want to match LOOP-START, LOOP_START, etc. Remove - or _ from string if it is present at position 4.
        if (IsOggLoopTag(argument) && ((argument[4] == '_') || (argument[4] == '-'))) {
            SDL_memmove(argument + 4, argument + 5, SDL_strlen(argument) - 4);
        }

        char *generic_key = NULL;
        const char *basekey = "SDL_mixer.metadata.ogg.";
        if (SDL_asprintf(&generic_key, "%s%s", basekey, argument) > 0) {
            for (char *ptr = generic_key + SDL_strlen(basekey); *ptr; ptr++) {
                *ptr = SDL_tolower(*ptr);
            }
            SDL_SetStringProperty(props, generic_key, value);
            SDL_free(generic_key);
        }

        if (SDL_strcasecmp(argument, "LOOPSTART") == 0) {
            loop->start = ParseOggTime(value, freq);
        } else if (SDL_strcasecmp(argument, "LOOPLENGTH") == 0) {
            loop->len = SDL_strtoll(value, NULL, 10);
            is_loop_length = true;
        } else if (SDL_strcasecmp(argument, "LOOPEND") == 0) {
            loop->end = ParseOggTime(value, freq);
            is_loop_length = false;
        } else if (SDL_strcasecmp(argument, "LOOPCOUNT") == 0) {
            loop->count = SDL_strtoll(value, NULL, 10);
            if (loop->count <= 0) {
                loop->count = -1;  // normalize infinite loop value.
            }
        } else if (SDL_strcasecmp(argument, "TITLE") == 0) {
            SDL_SetStringProperty(props, MIX_PROP_METADATA_TITLE_STRING, value);
        } else if (SDL_strcasecmp(argument, "ARTIST") == 0) {
            SDL_SetStringProperty(props, MIX_PROP_METADATA_ARTIST_STRING, value);
        } else if (SDL_strcasecmp(argument, "ALBUM") == 0) {
            SDL_SetStringProperty(props, MIX_PROP_METADATA_ALBUM_STRING, value);
        } else if (SDL_strcasecmp(argument, "COPYRIGHT") == 0) {
            SDL_SetStringProperty(props, MIX_PROP_METADATA_COPYRIGHT_STRING, value);
        } else if (SDL_strcasecmp(argument, "TRACKNUMBER") == 0) {
            Sint64 track, total_tracks;
            ParseTrackNumString(value, &track, &total_tracks);
            if (track > 0) {
                SDL_SetNumberProperty(props, MIX_PROP_METADATA_TRACK_NUMBER, track);
                if (total_tracks > 0) {
                    SDL_SetNumberProperty(props, MIX_PROP_METADATA_TOTAL_TRACKS_NUMBER, total_tracks);
                }
            }
        } else if (SDL_strcasecmp(argument, "DATE") == 0) {
            int year, month, day;
            if (SDL_sscanf(value, "%d-%d-%d", &year, &month, &day) >= 1) {
                if (year > 0) {
                    SDL_SetNumberProperty(props, MIX_PROP_METADATA_YEAR_NUMBER, (Sint64) year);
                }
            }
        }

        SDL_free(param);
    }

    if (is_loop_length) {
        loop->end = loop->start + loop->len;
    } else {
        loop->len = loop->end - loop->start;
    }

    if (!loop->end || (loop->end < loop->start)) {
        loop->len = -1;  // invalidate the whole thing.
    }

    // Ignore invalid or missing loop tag
    loop->active = ((loop->start >= 0) && (loop->len >= 0) && (loop->end >= 0));
    if (!loop->active) {
        loop->start = loop->len = loop->end = 0;
        loop->count = -1;
    }
}

