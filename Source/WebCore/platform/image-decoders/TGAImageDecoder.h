#ifndef TGA_IMAGE_DECODER_H
#define TGA_IMAGE_DECODER_H

#include "ImageDecoder.h"
#include <wtf/OwnPtr.h>
#include <wtf/PassOwnPtr.h>

namespace WebCore {

    class TGAImageReader;

    // This class decodes the tga image format.
    class TGAImageDecoder : public ImageDecoder {
    public:
        TGAImageDecoder(ImageSource::AlphaOption alphaOption, ImageSource::GammaAndColorProfileOption gammaAndColorProfileOption)
            :ImageDecoder(alphaOption, gammaAndColorProfileOption)
        {
        }

        // ImageDecoder
        virtual String filenameExtension() const { return "tga"; }
        virtual void setData(SharedBuffer*, bool allDataReceived);
        virtual bool isSizeAvailable();
        virtual ImageFrame* frameBufferAtIndex(size_t index);

    private:
        // Decodes the image.  If |onlySize| is true, stops decoding after
        // calculating the image size.  If decoding fails but there is no more
        // data coming, sets the "decode failure" flag.
        void decode(bool onlySize);

        // The reader used to do most of the tga decoding.
        OwnPtr<TGAImageReader> m_reader;
    };

    class TGAImageReader
    {
    public:
        TGAImageReader(TGAImageDecoder* decoder)
            : m_parent(decoder)
            , m_readOffset(0)
			, m_flipVert(false)
			, m_bytesPerPixel(0)
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

		bool hasMoreData(size_t count)
		{
			return (m_data->size() >= m_readOffset + count) ? true : false;
		}

        enum 
        {
            TGA_Map = 1,
            TGA_RGB = 2,
            TGA_Mono = 3,
            TGA_RLEMap = 9,
            TGA_RLERGB = 10,
            TGA_RLEMono = 11
        };

        enum 
        {
            MAXCOLORS = 16384
        };
        
        bool decode(bool sizeOnly)
        {
            m_readOffset = 0;

            if (m_data->size() < 18)
                return false;

            unsigned char idLength = readUint8();           
            unsigned char colorMapType = readUint8();
            unsigned char imageType = readUint8();
            unsigned short minPalIndex = readUint16();
            unsigned short palLength = readUint16();
            unsigned char colorMapSize = readUint8();
            unsigned short xOrigin = readUint16();
            unsigned short yOrigin = readUint16();
            unsigned short width = readUint16();
            unsigned short height = readUint16();
            unsigned char pixelSize = readUint8();
            unsigned char attBits = readUint8();
            m_readOffset += idLength;

			m_flipVert = (attBits & 0x20) ? false : true;
                        
            m_parent->setSize(width, height);
            if (sizeOnly)
                return true;

            bool colormapped = false;
            bool rle = false;
            
            switch (imageType)
            {
                case TGA_Map:
                    colormapped = true;
                    rle = false;
                    break;
                
                case TGA_Mono:
                    colormapped = false;
                    rle = false;
                    break;
                    
                case TGA_RGB:
                    colormapped = false;
                    rle = false;
                    break;
                    
                case TGA_RLEMap:
                    colormapped = true;
                    rle = true;
                    break;
                    
                case TGA_RLEMono:
                    colormapped = false;
                    rle = true;
                    break;
                    
                case TGA_RLERGB:
                    colormapped = false;
                    rle = true;
                    break;
                    
                default:
                    return false;
            }
            
            if (colormapped)
                return false;
                
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

			if (pixelSize == 32)
				m_buffer->setHasAlpha(true);
			else if (pixelSize == 24)
				m_buffer->setHasAlpha(false);
			else
				return false;

			m_bytesPerPixel = pixelSize / 8;

			if (rle)
			{
				if (!decodeRLEPixels())
					return false;
			}
			else
			{
				if (!decodePixels())
					return false;
			}
                            
            // Done!
            m_buffer->setStatus(ImageFrame::FrameComplete);
            return true;
        }
        
        bool decodePixels()
        {
            int width = m_parent->size().width();
            int height = m_parent->size().height();

			if (!hasMoreData(width * height * m_bytesPerPixel))
				return false;

			unsigned char r, g, b, a;
			for (int y = 0; y < height; ++y)
			{
				for (int x = 0; x < width; ++x)
				{
					b = readUint8();
					g = readUint8();
					r = readUint8();

					if (m_bytesPerPixel == 4)
						a = readUint8();
					else
						a = 255;

					if (m_flipVert)
						m_buffer->setRGBA(x, height - y - 1, r, g, b, a);
					else
						m_buffer->setRGBA(x, y, r, g, b, a);
				}
			}

            return true;
        }

		bool decodeRLEPixels()
		{
            int width = m_parent->size().width();
            int height = m_parent->size().height();


			int pixel = 0;
			unsigned count;
			unsigned char r, g, b, a;
			int x, y;
			while (pixel < width * height)
			{
				if (!hasMoreData(1))
					return false;

				count = readUint8();
				if (count & 0x80)
				{
					count -= 127;

					if (!hasMoreData(m_bytesPerPixel))
						return false;

					b = readUint8();
					g = readUint8();
					r = readUint8();

					if (m_bytesPerPixel == 4)
						a = readUint8();
					else
						a = 255;

					for (int i = 0; i < count; ++i)
					{
						x = pixel % width;
						if (m_flipVert)
							y = height - (pixel / width) - 1;
						else
							y = pixel / width;
						++pixel;
						
						m_buffer->setRGBA(x, y, r, g, b, a);
					}
				}
				else
				{
					++count;
					if (!hasMoreData(count*m_bytesPerPixel))
						return false;

					for (int i = 0; i < count; ++i)
					{
						b = readUint8();
						g = readUint8();
						r = readUint8();

						if (m_bytesPerPixel == 4)
							a = readUint8();
						else
							a = 255;

						x = pixel % width;
						if (m_flipVert)
							y = height - (pixel / width) - 1;
						else
							y = pixel / width;
						++pixel;
						
						m_buffer->setRGBA(x, y, r, g, b, a);
					}
				}
			}

			return true;
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

		bool m_flipVert;

		unsigned char m_bytesPerPixel;
    };

    void TGAImageDecoder::setData(SharedBuffer* data, bool allDataReceived)
    {
        if (failed())
            return;

        ImageDecoder::setData(data, allDataReceived);
        if (m_reader)
            m_reader->setData(data);
    }

    bool TGAImageDecoder::isSizeAvailable()
    {
        if (!ImageDecoder::isSizeAvailable())
            decode(true);

        return ImageDecoder::isSizeAvailable();
    }

    ImageFrame* TGAImageDecoder::frameBufferAtIndex(size_t index)
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

    void TGAImageDecoder::decode(bool onlySize)
    {
        if (failed())
            return;

        if (!m_reader)
        {
            m_reader = adoptPtr(new TGAImageReader(this));
            m_reader->setData(m_data.get());
        }

        if (!m_frameBufferCache.isEmpty())
            m_reader->setBuffer(&m_frameBufferCache.first());

        // If we couldn't decode the image but we've received all the data, decoding
        // has failed.
        if (!m_reader->decode(onlySize) && isAllDataReceived())
            setFailed();
        // If we're done decoding the image, we don't need the TGAImageReader
        // anymore.  (If we failed, |m_reader| has already been cleared.)
        else if (!m_frameBufferCache.isEmpty() && (m_frameBufferCache[0].status() == ImageFrame::FrameComplete))
            m_reader.clear();
    }

} // namespace WebCore

#endif
