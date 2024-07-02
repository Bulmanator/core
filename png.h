#if !defined(PNG_IMPL)

#if !defined(PNG_H_)
#define PNG_H_

c_linkage_begin

#if !defined(CORE_H_)
    #error "core.h is required to use this library"
#endif

typedef U32 PNG_DecodeFlags;
enum {
    // CRC checks are skipped by default to speed things up, if you want
    // the decoder to be a bit more robust with its checking this can be
    // turned on
    //
    PNG_DECODE_FLAG_VALIDATE_CRC = (1 << 0)
};

typedef struct PNG_Image PNG_Image;
struct PNG_Image {
    U32 width;
    U32 height;

    U32 bit_depth;
    U32 channels;

    U8 *pixels;
};

// This will decode PNG data storing the resulting information in "image"
// Decoded pixels will be allocated from the supplied arena
//
// If decode fails returns false and more information can be acquired
// via the log messages
//
function B32 PNG_Decode(M_Arena *arena, PNG_Image *image, Str8 data, PNG_DecodeFlags flags);
function B32 PNG_DecodeFromFile(M_Arena *arena, PNG_Image *image, OS_Handle file, PNG_DecodeFlags flags);
function B32 PNG_DecodeFromPath(M_Arena *arena, PNG_Image *image, Str8 path, PNG_DecodeFlags flags);

c_linkage_end

#endif  // PNG_H_

#endif  // !PNG_IMPL

#if defined(PNG_MODULE) || defined(PNG_IMPL)

#if !defined(PNG_C_)
#define PNG_C_

//
// --------------------------------------------------------------------------------
// :impl_png
// --------------------------------------------------------------------------------
//

#define __PNG_SIGNATURE   0x0A1A0A0D474E5089
#define __PNG_NUM_SYMBOLS 288
#define __PNG_MAX_BITS    16

typedef U32 __PNG_ChunkID;
enum {
    __PNG_CHUNK_IHDR = FourCC('I', 'H', 'D', 'R'),
    __PNG_CHUNK_PLTE = FourCC('P', 'L', 'T', 'E'), // currently unsupported
    __PNG_CHUNK_IDAT = FourCC('I', 'D', 'A', 'T'),
    __PNG_CHUNK_IEND = FourCC('I', 'E', 'N', 'D'),
};

#pragma pack(push, 1)

// Structures that match their in-file representation
//

typedef struct __PNG_Chunk __PNG_Chunk;
struct __PNG_Chunk {
    U32 length;

    union {
        __PNG_ChunkID id;
        U8  text[4];
    };
};

typedef struct __PNG_ChunkIHDR __PNG_ChunkIHDR;
struct __PNG_ChunkIHDR {
    U32 width;
    U32 height;
    U8  bit_depth;
    U8  type;
    U8  compression;
    U8  filter;
    U8  interlace;
};

#pragma pack(pop)

typedef struct __PNG_Decoder __PNG_Decoder;
struct __PNG_Decoder {
    B32 valid;
    Stream_Context stream;

    __PNG_Chunk *idat;
};

// --------------------------------------------------------------------------------
// :idat processing
//

internal STREAM_REFILL(__PNG_RefillIDAT) {
    Stream_Error result;

    U8 *end = stream->end;

    // As we know there are several IDAT chunks (that by spec) must be
    // consecutive we move to the next chunk in the chain. This requires
    // skipping the 4 byte CRC at the end (we have already pre-validated
    // these if the user chose to do so).
    //
    // We can also guarantee that there is at least one chunk after the last
    // IDAT chunk as the IEND chunk must be last. This means we can check
    // if there are more IDAT chunks for decompressing or we have reached
    // the end of the stream. In the case we reach the end of the stream we
    // fail to the zero stream so it can pad the end
    //
    __PNG_Chunk *next = cast(__PNG_Chunk *) (end + 4);
    if (next->id == __PNG_CHUNK_IDAT) {
        U32 length = ReadBE_U32(next->length);

        stream->start = cast(U8 *) (next + 1);
        stream->end   = stream->start + length;
        stream->pos   = stream->start;
    }
    else {
        Stream_Fail(stream, STREAM_ERROR_READ_PAST_END);
    }

    result = stream->error;
    return result;
}

internal void __PNG_StreamFromIDAT(Stream_Context *stream, __PNG_Chunk *idat) {
    stream->error = STREAM_ERROR_NONE;

    Assert(idat->id == __PNG_CHUNK_IDAT);

    U32 length = ReadBE_U32(idat->length);

    stream->start = cast(U8 *) (idat + 1);
    stream->end   = stream->start + length;
    stream->pos   = stream->start;

    stream->bit_buffer = 0;
    stream->bit_count  = 0;

    stream->RefillFunc = __PNG_RefillIDAT;
}

// --------------------------------------------------------------------------------
// :png helpers
//

#define __PNG_Error(d, f, ...) __PNG_ErrorArgs((d), THIS_LINE, THIS_FUNCTION, (f), ##__VA_ARGS__)
internal void __PNG_ErrorArgs(__PNG_Decoder *decoder, U32 line, Str8 func, const char *format, ...) {
    decoder->valid = false;

    va_list args;
    va_start(args, format);

    Log_PushMessageArgs(LOG_ERROR, THIS_FILE, line, func, format, args);

    va_end(args);
}

internal B32 __PNG_ValidateCRC(U32 crc, Str8 data) {
    B32 result;

    local_persist const U32 crc_lut[256] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
        0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
        0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
        0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
        0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
        0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
        0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
        0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
        0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
        0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
        0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
        0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
        0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
        0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
        0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
        0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
        0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
        0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
        0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
        0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
    };

    U32 checksum = 0xFFFFFFFF;

    for (S64 it = 0; it < data.count; ++it) {
        checksum = crc_lut[(checksum ^ data.data[it]) & 0xFF] ^ (checksum >> 8);
    }

    checksum ^= 0xFFFFFFFF;

    result = (checksum == crc);
    return result;
}

// --------------------------------------------------------------------------------
// zlib/deflate decompressor
//

// Inspired by stb_image. 9-bits is enough to encode the fixed huffman entirely
// in the lookup table while still being relatively minimal and reducing
// cache/memory pressure
//
#define __ZLIB_LUT_BITS 9
#define __ZLIB_LUT_MASK ((1 << __ZLIB_LUT_BITS) - 1)

typedef struct __ZLIB_Huffman __ZLIB_Huffman;
struct __ZLIB_Huffman {
    U16 lut[1 << __ZLIB_LUT_BITS]; // composition ((len << __ZLIB_LUT_BITS) | symbol)

    U16 base[__PNG_MAX_BITS];
    U32 max_code[__PNG_MAX_BITS]; // pre-shifted to be 16-bit aligned

    // huffman "tree"
    //
    U8  lengths[__PNG_NUM_SYMBOLS];
    U16 symbols[__PNG_NUM_SYMBOLS];
};

internal B32 __ZLIB_ConstructHuffman(__ZLIB_Huffman *huffman, U8 *lengths, U32 count) {
    B32 result = true;

    // 1. Count the number of codes for each length
    //

    U8 bl_count[__PNG_MAX_BITS] = { 0 };
    for (U32 it = 0; it < count; ++it) {
        bl_count[lengths[it]] += 1;
    }

    // 2. Find the smallest code for each length and find the maximum
    //    code (pre-shifting it to 16-bit alignment)
    //

    U16 next_code[__PNG_MAX_BITS];
    U16 symbol = 0, code = 0;

    for (U32 it = 1; it < __PNG_MAX_BITS; ++it) {
        next_code[it]     = code;
        huffman->base[it] = symbol - code; // don't worry about wrapping

        code += bl_count[it];

        if (bl_count[it] != 0) {
            if ((code - 1) >= (1 << it)) {
                Log_Error("Length %d bits had too many codes assigned in huffman construction");

                result = false;
                break;
            }
        }

        // 16-bit align maximum code for fast comparison when LUT doesn't
        // cover the code
        //
        huffman->max_code[it] = code << (16 - it);

        code  <<= 1;
        symbol += bl_count[it];
    }

    // 3. Assign numerical values to the codes and construct LUT for fast decode
    //

    for (U32 it = 0; it < count; ++it) {
        U32 len = lengths[it];
        if (len != 0) {
            U16 code  = next_code[len]++;
            U16 index = huffman->base[len] + code;

            Assert(index < __PNG_NUM_SYMBOLS); // we control this, thus assert

            if (len <= __ZLIB_LUT_BITS) {
                // Can fit in the LUT, place the code in every location
                // that matches i.e. code 010 matches xxxxxx010 where xxxxxx
                // can be any combination of bits
                //
                // We have to reverse the bits first because huffman codes
                // are stored most-significant-bit first in the stream
                //
                U16 luti = ReverseBits_U16(code) >> (__PNG_MAX_BITS - len);
                U16 lutv = (len << __ZLIB_LUT_BITS) | it;

                while (luti < (1 << __ZLIB_LUT_BITS)) {
                    huffman->lut[luti] = lutv;
                    luti += (1 << len);
                }
            }

            huffman->lengths[index] = cast(U8)  len;
            huffman->symbols[index] = cast(U16) it;
        }
    }

    return result;
}

internal U32 __ZLIB_HuffmanDecode(__PNG_Decoder *decoder, __ZLIB_Huffman *huffman) {
    U32 result = 0;

    Stream_Context *stream = &decoder->stream;

    // Make sure there is at least 16-bits in the bit buffer
    //
    Stream_RefillBits(stream);

    U32 len = 0;

    U16 bits = cast(U16) Stream_PeekBits(stream, 16);
    U32 lutv = huffman->lut[bits & __ZLIB_LUT_MASK];
    if (lutv != 0) {
        // Extract values from the LUT
        //
        len    = lutv >> __ZLIB_LUT_BITS;
        result = lutv &  __ZLIB_LUT_MASK;
    }
    else {
        // decode via the slowpath because the code was too long
        // to fit in the lookup table
        //
        bits = ReverseBits_U16(bits);

        for (len = __ZLIB_LUT_BITS + 1; len < __PNG_MAX_BITS; ++len) {
            if (bits < huffman->max_code[len]) {
                break;
            }
        }

        len &= 15;

        if (len != 0) {
            U16 index = huffman->base[len] + (bits >> (16 - len));

            if (index >= __PNG_NUM_SYMBOLS) {
                __PNG_Error(decoder, "Symbol index out-of-bounds: %d", index);
            }
            else if (huffman->lengths[index] != len) {
                __PNG_Error(decoder, "Symbol length mismatch: %d != %d", len, huffman->lengths[index]);
            }
            else {
                result = huffman->symbols[index];
            }
        }
        else {
            __PNG_Error(decoder, "Huffman decode failed, code was too long");
        }
    }

    Stream_ConsumeBits(stream, len);

    return result;
}

internal B32 __ZLIB_Decompress(__PNG_Decoder *decoder, Str8 zbuffer) {
    B32 result;

    Stream_Context *stream = &decoder->stream;
    __PNG_StreamFromIDAT(stream, decoder->idat);

    // Read ZLIB headers
    //
    U32 cmf    = Stream_ReadBits(stream, 8);
    U32 flg    = Stream_ReadBits(stream, 8);
    U32 fcheck = ((cmf << 8) + flg);

    if ((cmf & 0xF) != 8) {
        __PNG_Error(decoder, "Invalid CM value in ZLIB stream: %d", cmf & 0xF);
    }
    else if ((fcheck % 31) != 0) {
        __PNG_Error(decoder, "Invalid FCHECK value in ZLIB stream: %d", fcheck);
    }
    else if ((flg & (1 << 5)) != 0) {
        __PNG_Error(decoder, "FDICT is not allowed for PNG image");
    }

    U8 *zpos = zbuffer.data;
    U8 *zend = zbuffer.data + zbuffer.count;

    for (;decoder->valid;) {
        if (zpos >= zend) {
            // This is always an error because we should catch the end of parsing
            // with BFINAL == 1 which happens at the end of the loop. If we
            // make it back up here with zpos >= zend it will attempt to parse
            // more data when there is no space to write decoded data to
            //
            __PNG_Error(decoder, "ZLIB decode buffer too small, read past end");
            break;
        }

        U32 bfinal = Stream_ReadBits(stream, 1);
        U32 btype  = Stream_ReadBits(stream, 2);

        if (btype == 0x0) {
            // BTYPE == 0x0 : Uncompressed literal copy
            //
            // Align bit buffer to the next byte boundary
            //
            Stream_ConsumeBits(stream, stream->bit_count & 7);

            U32 len  = Stream_ReadBits(stream, 16);
            U32 nlen = Stream_ReadBits(stream, 16);

            len  = ReadBE_U32(len);
            nlen = ReadBE_U32(nlen);

            if ((len ^ 0xFFFF) != nlen) {
                __PNG_Error(decoder, "LEN/NLEN mismatch on literal copy in ZLIB stream");
            }
            else if (zpos + len > zend) {
                __PNG_Error(decoder, "ZLIB decode buffer too small, attempted to copy %lld literals with %lld space remaining", len, (S64) (zend - zpos));
            }
            else {
                while (len != 0) {
                    if (stream->pos == stream->end) {
                        Stream_Error err = stream->RefillFunc(stream);
                        if (err != STREAM_ERROR_NONE) {
                            __PNG_Error(decoder, "Read past end of ZLIB stream data");
                            break;
                        }
                    }

                    U8 *literals  = stream->pos;
                    S64 remaining = Stream_Remaining(stream);
                    S64 to_copy   = Min(remaining, len);

                    M_CopySize(zpos, literals, to_copy);

                    stream->pos += to_copy;
                    zpos        += to_copy;
                    len         -= to_copy;
                }
            }
        }
        else if (btype <= 0x2) {
            __ZLIB_Huffman hlit  = { 0 };
            __ZLIB_Huffman hdist = { 0 };

            if (btype == 0x1) {
                // BTYPE == 0x1 : Fixed Huffman encoded block
                //
                local_persist U8 len_huffman_fixed[__PNG_NUM_SYMBOLS] = {
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
                    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
                    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
                    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
                    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
                    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
                    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
                    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                    7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8
                };

                local_persist U8 dist_huffman_fixed[32] = {
                    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
                };

                // We don't check errors on this because they are fixed by
                // the spec so can't fail
                //

                __ZLIB_ConstructHuffman(&hlit,  len_huffman_fixed,  __PNG_NUM_SYMBOLS);
                __ZLIB_ConstructHuffman(&hdist, dist_huffman_fixed, 32);
            }
            else {
                // BTYPE == 0x2 : Dynamic Huffman encoded block
                //
                local_persist const U32 hclen_swizzle[] = {
                    16, 17, 18, 0,  8, 7,  9, 6,  10,
                    5,  11, 4,  12, 3, 13, 2, 14, 1, 15
                };

                U8 hclen_lengths[19] = { 0 };
                U8 htotal_lengths[__PNG_NUM_SYMBOLS + 32] = { 0 };

                U32 hlit_count  = Stream_ReadBits(stream, 5) + 257;
                U32 hdist_count = Stream_ReadBits(stream, 5) + 1;
                U32 hclen_count = Stream_ReadBits(stream, 4) + 4;

                // Construct HCLEN Huffman tree
                //
                for (U32 it = 0; it < hclen_count; ++it) {
                    hclen_lengths[hclen_swizzle[it]] = Stream_ReadBits(stream, 3);
                }

                __ZLIB_Huffman hclen = { 0 };
                if (!__ZLIB_ConstructHuffman(&hclen, hclen_lengths, 19)) {
                    __PNG_Error(decoder, "Failed to construct dynamic HCLEN Huffman tree");
                    break;
                }

                U32 htotal = hlit_count + hdist_count;
                for (U32 it = 0; it < htotal;) {
                    U32 v = __ZLIB_HuffmanDecode(decoder, &hclen);

                    U32 count   = 1;
                    U8  literal = 0;

                    if (v <= 15) {
                        // Copy literal once
                        //
                        literal = v;
                    }
                    else if (v == 16) {
                        // Copy previous code 3-6 times
                        //
                        count   = Stream_ReadBits(stream, 2) + 3;
                        literal = htotal_lengths[it - 1];
                    }
                    else if (v == 17) {
                        // Copy zero 3-10 times
                        //
                        count = Stream_ReadBits(stream, 3) + 3;
                    }
                    else if (v == 18) {
                        // Copy zero 11-138 times
                        //
                        count = Stream_ReadBits(stream, 7) + 11;
                    }
                    else {
                        __PNG_Error(decoder, "Invalid length %d from HCLEN Huffman", v);
                        break;
                    }

                    M_FillSize(&htotal_lengths[it], literal, count);
                    it += count;
                }


                if (!__ZLIB_ConstructHuffman(&hlit, &htotal_lengths[0], hlit_count)) {
                    __PNG_Error(decoder, "Failed to construct dynamic HLIT Huffman tree");
                }
                else if (!__ZLIB_ConstructHuffman(&hdist, &htotal_lengths[hlit_count], hdist_count)) {
                    __PNG_Error(decoder, "Failed to construct dynamic HDIST Huffman tree");
                }
            }

            local_persist const U16 length_base[] = {
                3,   4,   5,   6,   7,   8,  9,  10,
                11,  13,  15,  17,  19,  23, 27, 31,
                35,  43,  51,  59,  67,  83, 99, 115,
                131, 163, 195, 227, 258, 0,  0
            };

            local_persist const U8 length_extra[] = {
                0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
                2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
            };

            local_persist const U16 dist_base[] = {
                1,    2,    3,     4,     5,     7,     9,    13,
                17,   25,   33,    49,    65,    97,    129,  193,
                257,  385,  513,   769,   1025,  1537,  2049, 3073,
                4097, 6145, 8193,  12289, 16385, 24577, 0,    0
            };

            local_persist const U8 dist_extra[] = {
                0, 0, 0,  0,  1,  1,  2,  2,  3,  3,
                4, 4, 5,  5,  6,  6,  7,  7,  8,  8,
                9, 9, 10, 10, 11, 11, 12, 12, 13, 13
            };

            for (;decoder->valid;) {
                U32 v = __ZLIB_HuffmanDecode(decoder, &hlit);

                if (v == 256) {
                    // End-of-block marker
                    //
                    break;
                }
                else if (zpos >= zend) {
                    __PNG_Error(decoder, "ZLIB decode buffer too small, read past end during Huffman decode");
                }
                else if (v < 256) {
                    // Literal copy
                    //
                    *zpos++ = v;
                }
                else if (v >= 286) {
                    __PNG_Error(decoder, "Invalid literal/length in DEFLATE stream: %d", v);
                }
                else {
                    U32 len, dist;

                    // Get the number of times to copy
                    //
                    v   -= 257;
                    len  = length_base[v];
                    len += Stream_ReadBits(stream, length_extra[v]);

                    // Get the distance to look back in previously decoded data
                    //
                    v     = __ZLIB_HuffmanDecode(decoder, &hdist);
                    dist  = dist_base[v];
                    dist += Stream_ReadBits(stream, dist_extra[v]);

                    if (zpos + len <= zend) { M_CopySize(zpos, zpos - dist, len); }

                    // Always update the decode buffer position to catch overwrite
                    // errors
                    //
                    zpos += len;
                }
            }
        }
        else {
            // BTYPE must be 0x3 which is invalid
            //
            __PNG_Error(decoder, "Invalid BTYPE in ZLIB stream: 0x%x", btype);
        }

        // Finished parsing the final block
        //
        if (bfinal == 0x1) { break; }
    }

    result = decoder->valid;
    return result;
}

// --------------------------------------------------------------------------------
// png de-filter
//
enum {
    __PNG_FILTER_NONE = 0,
    __PNG_FILTER_SUB,
    __PNG_FILTER_UP,
    __PNG_FILTER_AVERAGE,
    __PNG_FILTER_PAETH
};

internal void __PNG_Defilter(__PNG_Decoder *decoder, PNG_Image *image, Str8 zbuffer) {
    U32 pixel_size    = (image->bit_depth >> 3) * image->channels;
    U32 scanline_size = pixel_size * image->width;

    U8 *zpos = zbuffer.data;
    U8 *ppos = image->pixels;
    U8 *pend = ppos + (scanline_size * image->height);

    // Initial scanline, no prior scanline collapses some of the filters down
    //
    U8 filter = *zpos++;
    switch (filter) {
        case __PNG_FILTER_NONE:
        case __PNG_FILTER_UP: {
            // Out(x) = Filtered(x)
            //
            M_CopySize(ppos, zpos, scanline_size);
        }
        break;
        case __PNG_FILTER_SUB:
        case __PNG_FILTER_PAETH: {
            // Out(x) = Filtered(x) + Out(x - bpp)
            //
            switch (pixel_size) {
                case 4: { ppos[3] = zpos[3]; }
                case 3: { ppos[2] = zpos[2]; }
                case 2: { ppos[1] = zpos[1]; }
                case 1: { ppos[0] = zpos[0]; }
            }

            for (U32 x = pixel_size; x < scanline_size; ++x) {
                ppos[x] = zpos[x] + ppos[x - pixel_size];
            }
        }
        break;
        case __PNG_FILTER_AVERAGE: {
            switch (pixel_size) {
                case 4: { ppos[3] = zpos[3]; }
                case 3: { ppos[2] = zpos[2]; }
                case 2: { ppos[1] = zpos[1]; }
                case 1: { ppos[0] = zpos[0]; }
            }

            for (U32 it = pixel_size; it < scanline_size; ++it) {
                ppos[it] = zpos[it] + (ppos[it - pixel_size] >> 1);
            }
        }
        break;
        default: {
            __PNG_Error(decoder, "Invalid PNG filter value: %d", filter);
        }
        break;
    }

    U8 *prior = ppos;

    zpos += scanline_size;
    ppos += scanline_size;

    // @todo: this can be done (somewhat) in SIMD
    //

    for (;decoder->valid;) {
        if (ppos >= pend) {
            if (ppos > pend) {
                __PNG_Error(decoder, "Read past end of pixel data when de-filtering");
            }

            // Finished
            //
            break;
        }

        filter = *zpos++;
        switch (filter) {
            case __PNG_FILTER_NONE: {
                M_CopySize(ppos, zpos, scanline_size);
            }
            break;
            case __PNG_FILTER_SUB: {
                switch (pixel_size) {
                    case 4: { ppos[3] = zpos[3]; }
                    case 3: { ppos[2] = zpos[2]; }
                    case 2: { ppos[1] = zpos[1]; }
                    case 1: { ppos[0] = zpos[0]; }
                }

                for (U32 x = pixel_size; x < scanline_size; ++x) {
                    ppos[x] = zpos[x] + ppos[x - pixel_size];
                }
            }
            break;
            case __PNG_FILTER_UP: {
                for (U32 x = 0; x < scanline_size; ++x) {
                    ppos[x] = zpos[x] + prior[x];
                }
            }
            break;
            case __PNG_FILTER_AVERAGE: {
                switch (pixel_size) {
                    case 4: { ppos[3] = zpos[3] + (prior[3] >> 1); }
                    case 3: { ppos[2] = zpos[2] + (prior[2] >> 1); }
                    case 2: { ppos[1] = zpos[1] + (prior[1] >> 1); }
                    case 1: { ppos[0] = zpos[0] + (prior[0] >> 1); }
                }

                for (U32 x = pixel_size; x < scanline_size; ++x) {
                    U16 sum = ((ppos[x - pixel_size] + prior[x]) >> 1) & 0xFF;
                    ppos[x] = zpos[x] + cast(U8) sum;
                }
            }
            break;
            case __PNG_FILTER_PAETH: {
                switch (pixel_size) {
                    case 4: { ppos[3] = zpos[3] + prior[3]; }
                    case 3: { ppos[2] = zpos[2] + prior[2]; }
                    case 2: { ppos[1] = zpos[1] + prior[1]; }
                    case 1: { ppos[0] = zpos[0] + prior[0]; }
                }

                for (U32 x = pixel_size; x < scanline_size; ++x) {
                    S32 a = ppos[x - 4];
                    S32 b = prior[x];
                    S32 c = prior[x - 4];

                    // faster paeth
                    //
                    S32 t  = (3 * c) - (a + b);
                    S32 lo = Min(a, b);
                    S32 hi = Max(a, b);

                    S32 t0 = (hi <= t) ? lo : c;
                    S32 t1 = (t <= lo) ? hi : t0;

                    ppos[x] = zpos[x] + cast(U8) (t1 & 0xFF);
                }
            }
            break;
            default: {
                __PNG_Error(decoder, "Invalid PNG filter value: %d", filter);
            }
            break;
        }

        prior = ppos;

        zpos += scanline_size;
        ppos += scanline_size;
    }
}

// --------------------------------------------------------------------------------
// png main decode
//

B32 PNG_Decode(M_Arena *arena, PNG_Image *image, Str8 data, PNG_DecodeFlags flags) {
    B32 result;

    __PNG_Decoder _decoder = { 0 };
    __PNG_Decoder *decoder = &_decoder;

    // Setup the initial stream for parsing PNG chunk information
    //
    Stream_Context *stream = &decoder->stream;
    Stream_FromMemory(stream, data);

    // At least enough to look at the signature, if the stream is too
    // short other error checking (i.e. IHDR/IDAT/IEND missing) will
    // kick in to invalidate the decoder
    //
    decoder->valid = Stream_Remaining(stream) >= 8;

    if (decoder->valid) {
        U64 *signature = Stream_Read(stream, U64);
        if (signature[0] != __PNG_SIGNATURE) {
            __PNG_Error(decoder, "Invalid PNG signature '0x%x'", signature[0]);
        }
    }

    B32 first = true, seen_iend = false, prev_idat = false;
    B32 validate_crc = (flags & PNG_DECODE_FLAG_VALIDATE_CRC) != 0;

    for (;decoder->valid;) {
        // Check if we have reached the end of the stream, in this case
        // there is no refilling so break out of loop
        //
        if (stream->pos >= stream->end) {
            if (stream->pos > stream->end) {
                __PNG_Error(decoder, "Read past end of stream");
            }

            break;
        }

        __PNG_Chunk *chunk = Stream_Read(stream, __PNG_Chunk);
        U64 length = ReadBE_U32(chunk->length);

        switch (chunk->id) {
            case __PNG_CHUNK_IHDR: {
                __PNG_ChunkIHDR *ihdr = Stream_Read(stream, __PNG_ChunkIHDR);

                image->width     = ReadBE_U32(ihdr->width);
                image->height    = ReadBE_U32(ihdr->height);
                image->bit_depth = 8;
                image->channels  = 4;

                // Support will expand in the future!!
                //

                if (ihdr->bit_depth != 8) {
                    __PNG_Error(decoder, "Unsupported bit depth: %d", ihdr->bit_depth);
                }
                else if (ihdr->type != 6) {
                    __PNG_Error(decoder, "Unsupported colour type: %d", ihdr->type);
                }
                else if (ihdr->interlace == 1) {
                    __PNG_Error(decoder, "Adam7 interlacing is not currently supported");
                }

                // Actual error checking
                //
                U64 pixel_size  = (image->bit_depth >> 3) * image->channels;
                U64 total_bytes = (image->width * image->height * pixel_size);

                if (ihdr->compression != 0) {
                    __PNG_Error(decoder, "Invalid compression method: %d", ihdr->compression);
                }
                else if (ihdr->filter != 0) {
                    __PNG_Error(decoder, "Invalid filter method: %d", ihdr->filter);
                }
                else if (ihdr->interlace > 1) {
                    __PNG_Error(decoder, "Invalid interlace method: %d", ihdr->interlace);
                }
                else if (total_bytes > (1 << 30)) {
                    __PNG_Error(decoder, "Image too large to decode");
                }
                else if (image->width == 0 || image->height == 0) {
                    __PNG_Error(decoder, "zero pixels in image");
                }

                first = false;
            }
            break;
            case __PNG_CHUNK_PLTE: {
                if (first) {
                    __PNG_Error(decoder, "PLTE chunk was found before IHDR");
                }

                // Skip for now, it should be caught when parsing IHDR as we
                // don't support palette images yet
                //
                stream->pos += length;
            }
            break;
            case __PNG_CHUNK_IDAT: {
                if (first) {
                    __PNG_Error(decoder, "IDAT chunk was found before IHDR");
                }
                else if (decoder->idat == 0) {
                    // First IDAT seen so store the pointer to the chunk as we
                    // will use it as the basis of our ZLIB parsing
                    //
                    decoder->idat = chunk;
                }
                else if (!prev_idat) {
                    // decoder->idat has been set that means we have seen at
                    // least one IDAT chunk. However, the last chunk we processed
                    // was not an IDAT chunk meaning they were not consecutive.
                    // This is invalid as per the PNG spec
                    //
                    __PNG_Error(decoder, "Non-consecutive IDAT chunk placement is invalid");
                }

                stream->pos += length;
            }
            break;
            case __PNG_CHUNK_IEND: {
                if (first) {
                    __PNG_Error(decoder, "IEND chunk was found before IHDR");
                }
                else if (decoder->idat == 0) {
                    __PNG_Error(decoder, "No IDAT chunks were found");
                }
                else if (length != 0) {
                    __PNG_Error(decoder, "Invalid IEND length: %d", length);
                }

                seen_iend = true;
            }
            break;
            default: {
                Log_Info("Skipping unknown PNG chunk: %.*s", 4, chunk->text);
                stream->pos += length;
            }
            break;
        }

        // If the this (now prev) chunk was an IDAT chunk we set this
        // flag to true. This validates that if there are multiple
        // IDAT chunks in the file they are all consecutive (as per spec)
        //
        prev_idat = (chunk->id == __PNG_CHUNK_IDAT);

        U32 *crc = Stream_Read(stream, U32);
        if (validate_crc && Stream_Remaining(stream) >= 0) {
            // CRC includes id but not length
            //
            Str8 crc_data;
            crc_data.data  = cast(U8 *) chunk + 4;
            crc_data.count = length + 4;

            U32 crc_sum = ReadBE_U32(crc[0]);

            if (!__PNG_ValidateCRC(crc_sum, crc_data)) {
                __PNG_Error(decoder, "CRC check failed for '%.*s' chunk", 4, chunk->text);
            }
        }
    }

    decoder->valid &= seen_iend;

    if (decoder->valid) {
        M_Temp temp = M_AcquireTemp(1, &arena);

        U32 pixel_size = (image->bit_depth >> 3) * image->channels;

        // Allocate a buffer to decompress into, we have to add an extra column
        // of bytes to fit the single-byte filter values that are prepended to
        // the beginning of each scanline
        //
        Str8 zbuffer;
        zbuffer.count = (image->width * image->height * pixel_size) + image->height;
        zbuffer.data  = M_ArenaPush(temp.arena, U8, zbuffer.count, M_ARENA_NO_ZERO);

        if (__ZLIB_Decompress(decoder, zbuffer)) {
            // Finally allocate the output pixels, this has been chosen to be
            // allocated rather than supplied as we cannot have this be in
            // GPU mapped memory as de-filtering requires reading from the
            // output buffer which should not be done to write-combined memory
            //
            image->pixels = M_ArenaPush(arena, U8, pixel_size * image->width * image->height, M_ARENA_NO_ZERO);
            __PNG_Defilter(decoder, image, zbuffer);
        }

        M_ReleaseTemp(temp);
    }

    result = decoder->valid;
    return result;
}

B32 PNG_DecodeFromFile(M_Arena *arena, PNG_Image *image, OS_Handle file, PNG_DecodeFlags flags) {
    B32 result;

    M_Temp temp = M_AcquireTemp(1, &arena);

    Str8 data;
    data.count = FS_SizeFromHandle(file);
    data.data  = M_ArenaPush(temp.arena, U8, data.count, M_ARENA_NO_ZERO);

    // In case the file read gets truncated
    //
    data.count = FS_ReadFile(file, data, 0);
    result     = PNG_Decode(arena, image, data, flags);

    M_ReleaseTemp(temp);
    return result;
}

B32 PNG_DecodeFromPath(M_Arena *arena, PNG_Image *image, Str8 path, PNG_DecodeFlags flags) {
    OS_Handle file = FS_OpenFile(path, FS_ACCESS_READ);
    B32 result     = PNG_DecodeFromFile(arena, image, file, flags);

    FS_CloseFile(file);
    return result;
}

#endif  // PNG_C_

#endif  // PNG_MODULE || PNG_IMPL
