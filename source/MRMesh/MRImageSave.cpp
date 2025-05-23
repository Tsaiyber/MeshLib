#include "MRImageSave.h"

#include "MRFile.h"
#include "MRIOFormatsRegistry.h"
#include "MRImage.h"
#include "MRStringConvert.h"

#include <filesystem>
#include <fstream>

namespace MR
{
namespace ImageSave
{

#pragma pack(push, 1)
struct BMPHeader
{
    const char sign[2] = {'B','M'};
    uint32_t size = 0;
    const uint32_t reserved = 0;
    const uint32_t dataOffset = 54;
    const uint32_t sizeOfHeader = 40;
    uint32_t width = 0;
    uint32_t height = 0;
    const uint16_t planes = 1;
    const uint16_t bitsPerPixel = 32;
    const uint32_t compression = 0;//no compression
    const uint32_t imageSize = 0; //can be 0 as far as there is no compression
    const uint32_t pixelsPerMeterX = 11811; // 300 dpi
    const uint32_t pixelsPerMeterY = 11811; // 300 dpi
    const uint32_t colorUsed = 0; // all
    const uint32_t importantColors = 0; // all
};
#pragma pack(pop)

Expected<void> toBmp( const Image& image, const std::filesystem::path& file )
{
    std::ofstream out( file, std::ofstream::binary );
    if ( !out )
        return unexpected( std::string( "Cannot open file for writing " ) + utf8string( file ) );

    BMPHeader header;
    static_assert( sizeof( header ) == 54 );
    static_assert( sizeof( Color ) == 4 );
    header.size = uint32_t( image.pixels.size() ) * sizeof( Color ) + header.dataOffset;
    header.width = image.resolution.x;
    header.height = image.resolution.y;

    out.write( (const char*) &header, sizeof( header ) );
    for ( const auto& p : image.pixels )
    {
        out.write( (const char*) &p[2], 1 );
        out.write( (const char*) &p[1], 1 );
        out.write( (const char*) &p[0], 1 );
        out.write( (const char*) &p[3], 1 );
        if ( !out )
            return unexpected( std::string( "Error saving image" ) );
    }

    return {};
}

Expected<void> toAnySupportedFormat( const Image& image, const std::filesystem::path& file )
{
    auto ext = utf8string( file.extension() );
    for ( auto& c : ext )
        c = (char) tolower( c );
    ext = "*" + ext;

    auto saver = getImageSaver( ext );
    if ( !saver )
        return unexpectedUnsupportedFileExtension();

    return saver( image, file );
}

MR_ADD_IMAGE_SAVER( IOFilter( "BitMap Picture (.bmp)",  "*.bmp" ), toBmp )

}
}
