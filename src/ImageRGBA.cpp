#include <ImageRGBA.hpp>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include <png.h>
#include <qrcodegen.hpp>

static void abort_(const char *s, ...)
{
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

ImageRGBA::ImageRGBA()
{
    width_ = 0;
    height_ = 0;
    padW_ = 0;
    padH_ = 0;
}

ImageRGBA::ImageRGBA(int width, int height)
{
    width_ = width;
    height_ = height;
    padW_ = 0;
    padH_ = 0;
    data_.resize(width_ * height_ * 4);
}

void ImageRGBA::PadToPowerOfTwo()
{
    // If the image is empty, abort
    if (width_ == 0 || height_ == 0)
        return;

    // Calculate the nearest larger-or-equal power of two dimensions
    int potWidth = exp2(ceil(log2(width_)));
    int potHeight = exp2(ceil(log2(height_)));
    
    // If the image already has power-of-two dims, abort
    if (width_ == potWidth && height_ == potHeight)
        return;

    // Assert that stupid hasn't gone wrong and the image area is growing
    if (width_ <= potWidth && height_ <= potHeight)
    {
        // Finally, expand the image row by row
        std::vector<uint8_t> paddedData(potWidth * potHeight * 4);
        uint8_t* paddedPtr = paddedData.data();
        uint8_t* ptr = data_.data();
        for (int y=0; y < height_; y++)
        {
            //void* memcpy( void* dest, const void* src, std::size_t count );
            memcpy(paddedPtr, ptr, width_*4);
            paddedPtr += (potWidth*4);
            ptr += (width_*4);
        }
        padH_ = potHeight - height_;
        padW_ = potWidth - width_;
        width_ = potWidth;
        height_ = potHeight;
        data_ = std::move(paddedData);
        return;
    }
    
    throw std::runtime_error("Image padding error!");
}

std::shared_ptr<ImageRGBA> ImageRGBA::FromPngFile(const std::string& imagePath)
{
    auto image = std::make_shared<ImageRGBA>();
    image->read_png_file(imagePath.c_str());
    return image;
}

std::shared_ptr<ImageRGBA> ImageRGBA::FromQrPayload(const std::string& qrPayload)
{
    auto image = std::make_shared<ImageRGBA>();
    
    auto qr = qrcodegen::QrCode::encodeText(qrPayload.c_str(), qrcodegen::QrCode::Ecc::MEDIUM);
    const int quietZoneSize = 2;
    image->width_ = qr.getSize() + quietZoneSize * 2;
    image->height_ = qr.getSize() + quietZoneSize * 2;
    image->data_.resize(image->width_ * image->height_ * 4);
    auto dataPtr = image->data();

    for (int y = -quietZoneSize; y < qr.getSize() + quietZoneSize; y++)
    {
        for (int x = -quietZoneSize; x < qr.getSize() + quietZoneSize; x++)
        {
            bool pxIsDark = qr.getModule(x,y);
            dataPtr[0] = pxIsDark ? 0 : 255;
            dataPtr[1] = pxIsDark ? 0 : 255;
            dataPtr[2] = pxIsDark ? 0 : 255;
            dataPtr[3] = 255;
            dataPtr += 4;
        }
    }
    
    return image;
}

void ImageRGBA::read_png_file(const char* file_name)
{
    png_byte color_type;
    png_byte bit_depth;

    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep* row_pointers;

    char header[8]; // 8 is the maximum size that can be checked

    /* open file and test for it being a png */
    FILE *fp = fopen(file_name, "rb");
    if (!fp)
        abort_("[read_png_file] File %s could not be opened for reading", file_name);
    fread(header, 1, 8, fp);
    if (png_sig_cmp((png_bytep)header, 0, 8))
        abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);

    /* initialize stuff */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        abort_("[read_png_file] png_create_read_struct failed");

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        abort_("[read_png_file] png_create_info_struct failed");

    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[read_png_file] Error during init_io");

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    width_ = png_get_image_width(png_ptr, info_ptr);
    height_ = png_get_image_height(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    /* read file */
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[read_png_file] Error during read_image");

    // Size dest appropriately
    int strideInBytes = png_get_rowbytes(png_ptr, info_ptr);
    data_.resize(height_ * strideInBytes);

    row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height_);
    for (int y = 0; y < height_; y++)
    {
        row_pointers[y] = data_.data() + strideInBytes * y;
    }

    png_read_image(png_ptr, row_pointers);

    fclose(fp);

    // It is possible that the image was read in as RGB instead of RGBA. Let's fix that here.
    if (strideInBytes == width_ * 3)
    {
        std::vector<uint8_t> rgbaVec(height_ * width_ * 4);
        uint8_t* rgbData = data_.data();
        uint8_t* rgbaData = rgbaVec.data();
        int pixels = width_ * height_;
        for (int p=0; p < pixels; p++)
        {
            rgbaData[0] = rgbData[0];
            rgbaData[1] = rgbData[1];
            rgbaData[2] = rgbData[2];
            rgbaData[3] = 255;
            rgbaData += 4;
            rgbData += 3;
        }
        data_ = std::move(rgbaVec);
    }
}

uint8_t* ImageRGBA::data()
{
    return data_.data();
}

const uint8_t* ImageRGBA::data() const
{
    return data_.data();
}

int ImageRGBA::width() const
{
    return width_;
}

int ImageRGBA::height() const
{
    return height_;
}

int ImageRGBA::padW() const
{
    return padW_;
}

int ImageRGBA::padH() const
{
    return padH_;
}

uint8_t& ImageRGBA::operator[](std::size_t idx)
{
    return data_[idx];
}