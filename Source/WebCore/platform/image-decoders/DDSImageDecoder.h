#ifndef DDS_IMAGE_DECODER_H
#define DDS_IMAGE_DECODER_H

#include "ImageDecoder.h"
#include <wtf/OwnPtr.h>
#include <wtf/PassOwnPtr.h>

namespace WebCore {

    class DDSImageReader;

    // This class decodes the dds image format.
    class DDSImageDecoder : public ImageDecoder {
    public:
        DDSImageDecoder(ImageSource::AlphaOption alphaOption, ImageSource::GammaAndColorProfileOption gammaAndColorProfileOption)
            :ImageDecoder(alphaOption, gammaAndColorProfileOption)
        {
        }

        // ImageDecoder
        virtual String filenameExtension() const { return "dds"; }
        virtual void setData(SharedBuffer*, bool allDataReceived);
        virtual bool isSizeAvailable();
        virtual ImageFrame* frameBufferAtIndex(size_t index);

    private:
        // Decodes the image.  If |onlySize| is true, stops decoding after
        // calculating the image size.  If decoding fails but there is no more
        // data coming, sets the "decode failure" flag.
        void decode(bool onlySize);

        // The reader used to do most of the DDS decoding.
        OwnPtr<DDSImageReader> m_reader;
    };

    class DDSImageReader
    {
    public:
        DDSImageReader(DDSImageDecoder* decoder)
            : m_parent(decoder)
            , m_readOffset(0)
        {
        }

        void setBuffer(ImageFrame* buffer) { m_buffer = buffer; }
        void setData(SharedBuffer* data) { m_data = data; }

        uint8_t readUint8()
        {
            uint8_t result;
            memcpy(&result, &m_data->data()[m_readOffset], 1);
            ++m_readOffset;

            return result;
        }

        uint16_t readUint16()
        {
            uint16_t result;
            memcpy(&result, &m_data->data()[m_readOffset], 2);
            m_readOffset += 2;

            return result;
        }
        
        uint32_t readUint32()
        {
            uint32_t result;
            memcpy(&result, &m_data->data()[m_readOffset], 4);
            m_readOffset += 4;

            return result;
        }

        enum
        {
            DDSD_CAPS = 0x00000001,
            DDSD_HEIGHT = 0x00000002,
            DDSD_WIDTH = 0x00000004,
            DDSD_PIXELFORMAT = 0x00001000,
            DDSD_LINEARSIZE = 0x00080000,
            DDSD_DEPTH = 0x00800000
        };

        enum
        {
            DDSCAPS_COMPLEX = 0x00000008,
            DDSCAPS_TEXTURE = 0x00001000,
            DDSCAPS_MIPMAP = 0x00400000
        };

        enum 
        {
            FMT_FLAG_ALPHAPIXELS  = 0x00000001,
            FMT_FLAG_FOURCC       = 0x00000004,
            FMT_FLAG_RGB          = 0x00000040 
        };

        enum
        {
            FOURCC_DXTC1  = 0x31545844,
            FOURCC_DXTC3  = 0x33545844,
            FOURCC_DXTC5  = 0x35545844,
        };

        enum
        {
            FMT_UNKNOWN = 0,
            FMT_DXT1,
            FMT_DXT3,
            FMT_DXT5,
            FMT_A8R8G8B8,
            FMT_X8R8G8B8,
            FMT_R8G8B8,
            FMT_A4R4G4B4,
            FMT_X4R4G4B4,
            FMT_A1R5G5B5,
            FMT_X1R5G5B5,
            FMT_R5G6B5,
            FMT_A8R3G3B2,
        };

        bool decode(bool sizeOnly)
        {
            m_readOffset = 0;

            if (m_data->size() < 20)
                return false;

            unsigned int tag;
            tag = readUint32();
            if (tag != ' SDD')
                return false;

            // surface description header
            unsigned int size = readUint32();
            if (size != 124)
                return false;

            unsigned int flags = readUint32();
            const unsigned int requiredFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
            if ((flags & requiredFlags) != requiredFlags)
                return false;

            unsigned int height = readUint32();
            unsigned int width = readUint32();

            m_parent->setSize(width, height);
            if (sizeOnly)
                return true;

            if (m_data->size() < m_readOffset + 12)
                return false;

            //data size
            unsigned int temp = readUint32();
            
            unsigned int depth = readUint32();

            // Cannot handle volume textures
            if ((flags & DDSD_DEPTH) && depth)
                return false;

            unsigned int mipmapCount = readUint32();

            //Reserved
            m_readOffset += 4*11;

            if (m_data->size() < m_readOffset + 32)
                return false;

            temp = readUint32();

            unsigned int formatFlag = readUint32();
            unsigned int fourcc = readUint32();

            unsigned int format = FMT_UNKNOWN;
            if (formatFlag & FMT_FLAG_FOURCC)
            {
                // Ignore the following fields for compressed textures:
                //  RGBBitCount
                //  RBitMask
                //  GBitMask
                //  BBitMask
                //  RGBAlphaBitMask
                m_readOffset += 4*5;

                switch (fourcc)
                {
                case FOURCC_DXTC1:
                    format = FMT_DXT1;
                    break;

                case FOURCC_DXTC3:
                    format = FMT_DXT3;
                    break;

                case FOURCC_DXTC5:
                    format = FMT_DXT5;
                    break;
                }
            }
            else if (formatFlag & FMT_FLAG_RGB)
            {
                unsigned int bitCount = readUint32();
                unsigned int redBitCount = CalcMaskSize(readUint32());
                unsigned int greenBitCount = CalcMaskSize(readUint32());
                unsigned int blueBitCount = CalcMaskSize(readUint32());
                unsigned int alphaBitCount = CalcMaskSize(readUint32());
                if ((flags & FMT_FLAG_ALPHAPIXELS) == 0)
                    alphaBitCount = 0;

                if (bitCount == 32)
                {
                    if (alphaBitCount == 8)
                        format = FMT_A8R8G8B8;
                    else
                        format = FMT_X8R8G8B8;
                }
                else if (bitCount == 24)
                {
                    format = FMT_R8G8B8;
                }
                else if (bitCount == 16)
                {
                    if (redBitCount == 4 && greenBitCount == 4 && blueBitCount == 4 && alphaBitCount == 4)
                        format = FMT_A4R4G4B4;
                    else if (redBitCount == 4 && greenBitCount == 4 && blueBitCount == 4 && alphaBitCount == 0)
                        format = FMT_X4R4G4B4;
                    else if (redBitCount == 5 && greenBitCount == 5 && blueBitCount == 5 && alphaBitCount == 1)
                        format = FMT_A1R5G5B5;
                    else if (redBitCount == 5 && greenBitCount == 5 && blueBitCount == 5 && alphaBitCount == 0)
                        format = FMT_X1R5G5B5;
                    else if (redBitCount == 5 && greenBitCount == 6 && blueBitCount == 5)
                        format = FMT_R5G6B5;
                    else if (redBitCount == 3 && greenBitCount == 3 && blueBitCount == 2 && alphaBitCount == 8)
                        format = FMT_A8R3G3B2;
                }
            }

            if (format == FMT_UNKNOWN)
                return false;

            if (m_data->size() < m_readOffset + 8)
                return false;

            unsigned int caps = readUint32();
            if ((caps & DDSCAPS_COMPLEX) ||
                (caps & DDSCAPS_TEXTURE) ||
                (caps & DDSCAPS_MIPMAP))
            {
            }
            else
            {
                return false;
            }

            unsigned int caps2 = readUint32();
            if (caps2)
                return false;

            //Reserved[2]
            //Reserved2
            m_readOffset += 4*3;

            // Initialize the framebuffer if needed.
            ASSERT(m_buffer);  // Parent should set this before asking us to decode!

            if (m_buffer->status() == ImageFrame::FrameEmpty) {
                if (!m_buffer->setSize(width, height))
                    return m_parent->setFailed(); // Unable to allocate.
                m_buffer->setStatus(ImageFrame::FramePartial);
                // setSize() calls eraseARGB(), which resets the alpha flag, so we force
                // it back to false here.  We'll set it true below in all cases where
                // these 0s could actually show through.
                m_buffer->setHasAlpha(false);

                //the frame always fills the entire image.
                m_buffer->setOriginalFrameRect(IntRect(0, 0, width, height));
            }

            bool result = false;
            switch(format)
            {
            case FMT_DXT1:
            case FMT_DXT3:
            case FMT_DXT5:
                result = decodeDXTC(format);
                break;

            case FMT_A8R8G8B8:
                result = decode32Bit(true);
                break;

            case FMT_X8R8G8B8:
                result = decode32Bit(false);
                break;

            case FMT_R8G8B8:
                result = decode24Bit();
                break;

            case FMT_A4R4G4B4:
                result = decode16Bit(4, 4, 4, true);
                break;

            case FMT_X4R4G4B4:
                result = decode16Bit(4, 4, 4, false);
                break;

            case FMT_A1R5G5B5:
                result = decode16Bit(5, 5, 5, true);
                break;

            case FMT_X1R5G5B5:
                result = decode16Bit(5, 5, 5, false);
                break;

            case FMT_R5G6B5:
                result = decode16Bit(5, 6, 5, false);
                break;

            case FMT_A8R3G3B2:
                result = decode16Bit(3, 3, 2, true);
                break;
            }

            if (!result)
                return false;

            // Done!
            m_buffer->setStatus(ImageFrame::FrameComplete);
            return true;
        }

        unsigned int CalcMaskSize(unsigned int mask)
        {
            unsigned int total = 0;
            while (mask != 0)
            {
                total += (mask & 1);
                mask = mask >> 1;
            }
            return total;
        }

        bool decode32Bit(bool hasAlpha)
        {
            int width = m_parent->size().width();
            int height = m_parent->size().height();

            if (m_data->size() < m_readOffset + width * height * 4)
                return false;

            m_buffer->setHasAlpha(hasAlpha);

            unsigned char r, g, b, a;
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    b = readUint8();
                    g = readUint8();
                    r = readUint8();
                    a = readUint8();

                    if (!hasAlpha)
                        a = 255;

                    m_buffer->setRGBA(x, y, r, g, b, a); 
                }
            }

            return true;
        }

        bool decode24Bit()
        {
            int width = m_parent->size().width();
            int height = m_parent->size().height();

            if (m_data->size() < m_readOffset + width * height * 3)
                return false;

            m_buffer->setHasAlpha(false);

            unsigned char r, g, b;
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    b = readUint8();
                    g = readUint8();
                    r = readUint8();

                    m_buffer->setRGBA(x, y, r, g, b, 255);
                }
            }

            return true;
        }

        unsigned char getColor(unsigned int color, int bit, int count)
        {
            ASSERT(bit <= 31);
            ASSERT(count <= 8 && count > 0);

            color >>= bit;
            color &= ((1 << count) - 1);

            return unsigned char(color / float((1 << count) - 1.f) * 255.f);
        }

        bool decode16Bit(int redBitCount, int greenBitCount, int blueBitCount, bool hasAlpha)
        {
            int width = m_parent->size().width();
            int height = m_parent->size().height();

            if (m_data->size() < m_readOffset + width * height * 2)
                return false;

            m_buffer->setHasAlpha(hasAlpha);

            int alphaBitCount = 16 - redBitCount - greenBitCount - blueBitCount;

            unsigned char a, r, g, b;
            unsigned int color;
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    color = readUint16();

                    b = getColor(color, 0, blueBitCount);
                    g = getColor(color, blueBitCount, greenBitCount);
                    r = getColor(color, blueBitCount + greenBitCount, redBitCount);
                    if (hasAlpha)
                        a = getColor(color, blueBitCount + greenBitCount + redBitCount, 16 - (blueBitCount + greenBitCount + redBitCount));
                    else
                        a = 255;

                    m_buffer->setRGBA(x, y, r, g, b, a);
                }
            }

            return true;
        }

        enum { DEST_PIXEL_STRIDE = 4 };
        bool decodeDXTC(unsigned int format)
        {
            switch(format)
            {
            case FMT_DXT1:
                m_buffer->setHasAlpha(false);
                break;

            case FMT_DXT3:
                m_buffer->setHasAlpha(true);
                break;

            case FMT_DXT5:
                m_buffer->setHasAlpha(true);
                break;

            default:
                return false;
            }

            int width = m_parent->size().width();
            int height = m_parent->size().height();

            if (format == FMT_DXT1)
            {
                if (m_data->size() < m_readOffset + (((width + 3) & ~0x3) * ((height + 3) & ~0x3) >> 4) * 8)
                    return false;
            }
            else
            {
                if (m_data->size() < m_readOffset + (((width + 3) & ~0x3) * ((height + 3) & ~0x3) >> 4) * 16)
                    return false;
            }


            // Compute block-based info (a block is 4x4 pixels)
            unsigned int blockCols = width >> 2;
            unsigned int blockRows = height >> 2;
            unsigned int extraCols = width & 0x3;
            unsigned int extraRows = height & 0x3;
            
            int rowStride = width * DEST_PIXEL_STRIDE;
            unsigned char colorBlock[DEST_PIXEL_STRIDE * 16];

            unsigned int col, row;
            for (row = 0; row < blockRows; row++)
            {
                // decode the current block
                for (col = 0; col < blockCols; col++)
                {
                    decodeBlock(format, colorBlock);

                    unsigned char* colors = colorBlock;
                    for (int i = 0; i < 4; ++i)
                    {
                        for (int j = 0; j < 4; ++j)
                        {
                            m_buffer->setRGBA(col * 4 + j, row * 4 + i, colors[0], colors[1], colors[2], colors[3]);
                            colors += 4;
                        }
                    }
                }

                // If the width was not divisible by 4, then there will be one
                // partial block at the end of each row. The input block will 
                // be complete, but the output only covers a subset of the 
                // block pixels
                if (extraCols)
                {
                    decodeBlock(format, colorBlock);

                    unsigned char* colors = colorBlock;
                    for (int i = 0; i < 4; ++i)
                    {
                        for (int j = 0; j < extraCols; ++j)
                        {
                            m_buffer->setRGBA(blockCols * 4 + j, row * 4 + i, colors[0], colors[1], colors[2], colors[3]);
                            colors += 4;
                        }

                        colors += (4 - extraCols) * 4;
                    }
                }
            }

            // If the height was not divisible by 4, then there will be one
            // partial row of blocks at the end of each row.  The input blocks
            // will be complete, but the output only covers a subset of the 
            // block pixels
            if (extraRows)
            {
                // decode the current block
                for (col = 0; col < blockCols; col++)
                {
                    decodeBlock(format, colorBlock);

                    unsigned char* colors = colorBlock;
                    for (int i = 0; i < extraRows; ++i)
                    {
                        for (int j = 0; j < 4; ++j)
                        {
                            m_buffer->setRGBA(col * 4 + j, blockRows * 4 + i, colors[0], colors[1], colors[2], colors[3]);
                            colors += 4;
                        }
                    }
                }

                // If the width was not divisible by 4, then there will be one 
                // partial block at the end of each row.  The input block will
                // be complete, but the output only covers a subset of the 
                // block pixels
                if (extraCols)
                {
                    decodeBlock(format, colorBlock);

                    unsigned char* colors = colorBlock;
                    for (int i = 0; i < extraRows; ++i)
                    {
                        for (int j = 0; j < extraCols; ++j)
                        {
                            m_buffer->setRGBA(blockCols * 4 + j, blockRows * 4 + i, colors[0], colors[1], colors[2], colors[3]);
                            colors += 4;
                        }

                        colors += (4 - extraCols) * 4;
                    }
                }
            }

            return true;
        }

        void decodeBlock(int format, unsigned char* colorBlock)
        {
            switch (format)
            {
            case FMT_DXT1:
                decodeColorAlphaBlock(colorBlock);
                break;

            case FMT_DXT3:
                decodeT3AlphaBlock(colorBlock + 3);
                decodeColorBlock(colorBlock);
                break;

            case FMT_DXT5:
                decodeT5AlphaBlock(colorBlock + 3);
                decodeColorBlock(colorBlock);
            }
        }

        //---------------------------------------------------------------------------
        void decodeT3AlphaBlock(unsigned char* alphas)
        {
            unsigned char a;
            for (unsigned int i = 0; i < 8; i++)
            {
                a = readUint8();

                *alphas = unsigned char((a & 0xf) / 15.f * 255.f);
                alphas += DEST_PIXEL_STRIDE;

                *alphas = unsigned char(((a & 0xf0) >> 4) / 15.f * 255.f);
                alphas += DEST_PIXEL_STRIDE;
            }
        }
        //---------------------------------------------------------------------------
        void decodeT5AlphaBlock(unsigned char* alphas)
        {
            // Pull the two base alpha values from the block data
            unsigned char alphaTable[8];

            unsigned int A0 = alphaTable[0] = readUint8();
            unsigned int A1 = alphaTable[1] = readUint8();

            // Determine the interpolated alphas by looking at the sorted order of 
            // the two base alphas - these select between the two sub-modes
            if (A0 > A1)
            {
                // 8-alpha block
                alphaTable[2] = (unsigned char)((A0 * 6 + A1 * 1) / 7);
                alphaTable[3] = (unsigned char)((A0 * 5 + A1 * 2) / 7);
                alphaTable[4] = (unsigned char)((A0 * 4 + A1 * 3) / 7);
                alphaTable[5] = (unsigned char)((A0 * 3 + A1 * 4) / 7);
                alphaTable[6] = (unsigned char)((A0 * 2 + A1 * 5) / 7);
                alphaTable[7] = (unsigned char)((A0 * 1 + A1 * 6) / 7);
            }
            else
            {
                // 6-alpha block
                alphaTable[2] = (unsigned char)((A0 * 4 + A1 * 1) / 5);
                alphaTable[3] = (unsigned char)((A0 * 3 + A1 * 2) / 5);
                alphaTable[4] = (unsigned char)((A0 * 2 + A1 * 3) / 5);
                alphaTable[5] = (unsigned char)((A0 * 1 + A1 * 4) / 5);

                alphaTable[6] = 0;
                alphaTable[7] = 255;
            }

            // since the 3-bits per pixel situations cross byte boundaries, we must
            // take care to avoid endian-unfriendly situations.  This only happens
            // four times in a 16-pixel block, however.
            for (unsigned int i = 0; i < 2; i++)
            {
                unsigned char idx0 = readUint8();
                unsigned char idx1 = readUint8();
                unsigned char idx2 = readUint8();

                *alphas = alphaTable[idx0 & 0x7];
                alphas += DEST_PIXEL_STRIDE;

                *alphas = alphaTable[(idx0 >> 3) & 0x7];
                alphas += DEST_PIXEL_STRIDE;

                *alphas = alphaTable[((idx0 >> 6) & 0x3) + ((idx1 << 2) & 0x4)];
                alphas += DEST_PIXEL_STRIDE;

                *alphas = alphaTable[(idx1 >> 1) & 0x7];
                alphas += DEST_PIXEL_STRIDE;

                *alphas = alphaTable[(idx1 >> 4) & 0x7];
                alphas += DEST_PIXEL_STRIDE;

                *alphas = alphaTable[((idx1 >> 7) & 0x1) + ((idx2 << 1) & 0x6)];
                alphas += DEST_PIXEL_STRIDE;

                *alphas = alphaTable[(idx2 >> 2) & 0x7];
                alphas += DEST_PIXEL_STRIDE;

                *alphas = alphaTable[(idx2 >> 5) & 0x7];
                alphas += DEST_PIXEL_STRIDE;
            }
        }

        //---------------------------------------------------------------------------
        void decodeColorBlock(unsigned char* colorBlock)
        {
            // Read the two block colors and compute the others (endian-friendly)
            unsigned short color0 = readUint8() + (((unsigned short)readUint8()) << 8);
            unsigned short color1 = readUint8() + (((unsigned short)readUint8()) << 8);

            unsigned char colors[4][3];
            colors[0][0] = (unsigned char)((color0 & 0xf800) >> 8);
            colors[0][1] = (unsigned char)((color0 & 0x07f0) >> 3);
            colors[0][2] = (unsigned char)((color0 & 0x001f) << 3);

            colors[1][0] = (unsigned char)((color1 & 0xf800) >> 8);
            colors[1][1] = (unsigned char)((color1 & 0x07f0) >> 3);
            colors[1][2] = (unsigned char)((color1 & 0x001f) << 3);

            ASSERT(color0 >= color1);

            // Opaque, four color block
            // compute the two intermediate colors
            colors[2][0] = (unsigned char)(((((unsigned int)colors[0][0]) << 1) + ((unsigned int)colors[1][0])) / 3);
            colors[2][1] = (unsigned char)(((((unsigned int)colors[0][1]) << 1) + ((unsigned int)colors[1][1])) / 3);
            colors[2][2] = (unsigned char)(((((unsigned int)colors[0][2]) << 1) + ((unsigned int)colors[1][2])) / 3);

            colors[3][0] = (unsigned char)(((((unsigned int)colors[1][0]) << 1) + ((unsigned int)colors[0][0])) / 3);
            colors[3][1] = (unsigned char)(((((unsigned int)colors[1][1]) << 1) + ((unsigned int)colors[0][1])) / 3);
            colors[3][2] = (unsigned char)(((((unsigned int)colors[1][2]) << 1) + ((unsigned int)colors[0][2])) / 3);

            for (int row = 0; row < 4; ++row)
            {
                unsigned char pixel = readUint8();

                for (int col = 0; col < 4; ++col)
                {
                    unsigned char* finalPixel = colors[(pixel & 0x03)];
                    *colorBlock = *(finalPixel++);
                    *(colorBlock + 1) = *(finalPixel++);
                    *(colorBlock + 2) = *(finalPixel++);

                    colorBlock += DEST_PIXEL_STRIDE;

                    pixel >>= 2;
                }
            }
        }

        void decodeColorAlphaBlock(unsigned char* colorBlock)
        {
            // Read the two block colors and compute the others (endian-friendly)
            unsigned short color0 = readUint8() + (((unsigned short)readUint8())<<8);
            unsigned short color1 = readUint8() + (((unsigned short)readUint8())<<8);

            unsigned char colors[4][4];
            colors[0][0] = (unsigned char)((color0 & 0xf800) >> 8);
            colors[0][1] = (unsigned char)((color0 & 0x07f0) >> 3);
            colors[0][2] = (unsigned char)((color0 & 0x001f) << 3);
            colors[0][3] = 255;

            colors[1][0] = (unsigned char)((color1 & 0xf800) >> 8);
            colors[1][1] = (unsigned char)((color1 & 0x07f0) >> 3);
            colors[1][2] = (unsigned char)((color1 & 0x001f) << 3);
            colors[1][3] = 255;

            if (color0 > color1)
            {
                // Opaque, four color block
                // compute the two intermediate colors
                colors[2][0] = (unsigned char)(((((unsigned int)colors[0][0]) << 1) + ((unsigned int)colors[1][0])) / 3);
                colors[2][1] = (unsigned char)(((((unsigned int)colors[0][1]) << 1) + ((unsigned int)colors[1][1])) / 3);
                colors[2][2] = (unsigned char)(((((unsigned int)colors[0][2]) << 1) + ((unsigned int)colors[1][2])) / 3);
                colors[2][3] = 255;

                colors[3][0] = (unsigned char)(((((unsigned int)colors[1][0]) << 1) + ((unsigned int)colors[0][0])) / 3);
                colors[3][1] = (unsigned char)(((((unsigned int)colors[1][1]) << 1) + ((unsigned int)colors[0][1])) / 3);
                colors[3][2] = (unsigned char)(((((unsigned int)colors[1][2]) << 1) + ((unsigned int)colors[0][2])) / 3);
                colors[3][3] = 255;
            }
            else
            {
                // Translucent, three color block
                // compute the intermediate and translucent colors
                colors[2][0] = (unsigned char)((((unsigned int)colors[0][0]) + ((unsigned int)colors[1][0])) >> 1);
                colors[2][1] = (unsigned char)((((unsigned int)colors[0][1]) + ((unsigned int)colors[1][1])) >> 1);
                colors[2][2] = (unsigned char)((((unsigned int)colors[0][2]) + ((unsigned int)colors[1][2])) >> 1);
                colors[2][3] = 255;

                // use the intermediate color for the translucent pixels to avoid
                // potentially ugly interpolation to black or white at edges
                colors[3][0] = colors[2][0];
                colors[3][1] = colors[2][1];
                colors[3][2] = colors[2][2];
                colors[3][3] = 0;
            }

            for (int row = 0; row < 4; ++row)
            {
                unsigned char pixel = readUint8();

                for (int col = 0; col < 4; ++col)
                {
                    unsigned char* finalPixel = colors[(pixel & 0x03)];
                    *colorBlock = *(finalPixel++);
                    *(colorBlock + 1) = *(finalPixel++);
                    *(colorBlock + 2) = *(finalPixel++);
                    *(colorBlock + 3) = *(finalPixel++);

                    colorBlock += DEST_PIXEL_STRIDE;

                    pixel >>= 2;
                }
            }
        }

    private:
        // The decoder that owns us.
        ImageDecoder* m_parent;

        // The destination for the pixel data.
        ImageFrame* m_buffer;

        // An index into |m_data| representing how much we've already decoded.
        size_t m_readOffset;

        // The file to decode.
        RefPtr<SharedBuffer> m_data;
    };

    void DDSImageDecoder::setData(SharedBuffer* data, bool allDataReceived)
    {
        if (failed())
            return;

        ImageDecoder::setData(data, allDataReceived);
        if (m_reader)
            m_reader->setData(data);
    }

    bool DDSImageDecoder::isSizeAvailable()
    {
        if (!ImageDecoder::isSizeAvailable())
            decode(true);

        return ImageDecoder::isSizeAvailable();
    }

    ImageFrame* DDSImageDecoder::frameBufferAtIndex(size_t index)
    {
        if (index)
            return 0;

        if (m_frameBufferCache.isEmpty()) {
            m_frameBufferCache.resize(1);
            m_frameBufferCache[0].setPremultiplyAlpha(m_premultiplyAlpha);
        }

        ImageFrame* buffer = &m_frameBufferCache.first();
        if (buffer->status() != ImageFrame::FrameComplete)
            decode(false);
        return buffer;
    }

    void DDSImageDecoder::decode(bool onlySize)
    {
        if (failed())
            return;

        if (!m_reader)
        {
            m_reader = adoptPtr(new DDSImageReader(this));
            m_reader->setData(m_data.get());
        }

        if (!m_frameBufferCache.isEmpty())
            m_reader->setBuffer(&m_frameBufferCache.first());

        // If we couldn't decode the image but we've received all the data, decoding
        // has failed.
        if (!m_reader->decode(onlySize) && isAllDataReceived())
            setFailed();
        // If we're done decoding the image, we don't need the DDSImageReader
        // anymore.  (If we failed, |m_reader| has already been cleared.)
        else if (!m_frameBufferCache.isEmpty() && (m_frameBufferCache[0].status() == ImageFrame::FrameComplete))
            m_reader.clear();
    }

} // namespace WebCore

#endif
