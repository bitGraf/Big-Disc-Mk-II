#include "dds_file_reader.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Asserts.h"
#include "Engine/Platform/Platform.h"

#ifndef HRESULT
#define HRESULT int32
#define SUCCEEDED(hr)  (((HRESULT)(hr)) >= 0)
#define FAILED(hr)     (((HRESULT)(hr)) < 0)
#define E_POINTER      ((HRESULT)0x80004003L)
#define E_FAIL         ((HRESULT)0x80004005L)
#define S_OK           ((HRESULT)0L)
#endif

//--------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |       \
	((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
#endif /* defined(MAKEFOURCC) */

// HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW)
#define HRESULT_E_ARITHMETIC_OVERFLOW static_cast<HRESULT>(0x80070216L)

// HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED)
#define HRESULT_E_NOT_SUPPORTED static_cast<HRESULT>(0x80070032L)

// HRESULT_FROM_WIN32(ERROR_HANDLE_EOF)
#define HRESULT_E_HANDLE_EOF static_cast<HRESULT>(0x80070026L)

// HRESULT_FROM_WIN32(ERROR_INVALID_DATA)
#define HRESULT_E_INVALID_DATA static_cast<HRESULT>(0x8007000DL)

//--------------------------------------------------------------------------------------
// DDS file structure definitions
//
// See DDS.h in the 'Texconv' sample and the 'DirectXTex' library
//--------------------------------------------------------------------------------------
#pragma pack(push,1)

constexpr uint32_t DDS_MAGIC = 0x20534444; // "DDS "

struct DDS_PIXELFORMAT
{
    uint32_t    size;
    uint32_t    flags;
    uint32_t    fourCC;
    uint32_t    RGBBitCount;
    uint32_t    RBitMask;
    uint32_t    GBitMask;
    uint32_t    BBitMask;
    uint32_t    ABitMask;
};

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV

#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

enum DDS_MISC_FLAGS2
{
    DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
};

struct DDS_HEADER {
    uint32_t        size;
    uint32_t        flags;
    uint32_t        height;
    uint32_t        width;
    uint32_t        pitchOrLinearSize;
    uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
    uint32_t        mipMapCount;
    uint32_t        reserved1[11];
    DDS_PIXELFORMAT ddspf;
    uint32_t        caps;
    uint32_t        caps2;
    uint32_t        caps3;
    uint32_t        caps4;
    uint32_t        reserved2;
};

struct DDS_HEADER_DXT10 {
    DXGI_FORMAT     dxgiFormat;
    uint32_t        resourceDimension;
    uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
    uint32_t        arraySize;
    uint32_t        miscFlags2;
};

#pragma pack(pop)

HRESULT LoadTextureDataFromMemory(
	uint8_t* ddsData,
	size_t ddsDataSize,
	const DDS_HEADER** header,
	uint8_t** bitData,
	size_t* bitSize) noexcept;
DXGI_FORMAT GetDXGIFormat(const DDS_PIXELFORMAT& ddpf) noexcept;

Texture_Format DX2TF(DXGI_FORMAT) noexcept;

unsigned char* dds_load_from_memory(uint8 const *buffer, int buf_len, 
                                    int *out_width, int *out_height, Texture_Format* out_format,
                                    int32* out_is_cube, int32* out_mip_levels) {

    const DDS_HEADER* header = nullptr;
    uint8* bitData   = nullptr;
    uint64 bitSize     = 0;

    HRESULT hr = LoadTextureDataFromMemory(const_cast<uint8*>(buffer), buf_len, &header, &bitData, &bitSize);
    if (FAILED(hr)) {
        RH_FATAL("Failed to parse data as .dds");
        return nullptr;
    }

    const uint32 width = header->width;
    uint32 height = header->height;
    uint32 depth = header->depth;

    uint32 arraySize = 1;
    uint32 resDim    = 0;
    bool isCubeMap   = false;

    uint32 mipCount = header->mipMapCount;
    if (0 == mipCount) {
        mipCount = 1;
    }

    DXGI_FORMAT format = GetDXGIFormat(header->ddspf);

    // Check for 3D texture
    if (header->flags & DDS_HEADER_FLAGS_VOLUME){
        //resDim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        resDim = 3;
        RH_FATAL("3D Texture not supported!");
        return nullptr;
    }

    if (header->caps2 & DDS_CUBEMAP) {
        // We require all six faces to be defined
        if ((header->caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES) {
            RH_FATAL("Cubemap with not all faces defined!");
            return false;
        }

        arraySize = 6;
        isCubeMap = true;
    }

    resDim = 2;  //D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    // Note there's no way for a legacy Direct3D 9 DDS to express a '1D' texture

    Assert(BitsPerPixel(format) != 0);

    // Bound sizes (for security purposes we don't trust DDS file metadata larger than the Direct3D hardware requirements)
    //   #define D3D12_REQ_MIP_LEVELS 15
    if (mipCount > 15) {
        return false;
    }

    // switch on resource dimmension
    switch (resDim) {
        case 1: // D3D12_RESOURCE_DIMENSION_TEXTURE1D
            if ((arraySize > 2048) || (width > 16384))
                return false;
            break;
        case 2: // D3D12_RESOURCE_DIMENSION_TEXTURE2D
            if (isCubeMap) {
                // This is the right bound because we set arraySize to (NumCubes*6) above
                if ((arraySize > 2048) ||
                    (width > 16384) ||
                    (height > 16384)) {
                    return false;
                }
            } else if ((arraySize > 2048) ||
                       (width > 16384) ||
                       (height > 16384)) {
                return false;
            }
            break;
        case 3: // D3D12_RESOURCE_DIMENSION_TEXTURE3D
            if ((arraySize > 1) ||
                (width  > 2048) ||
                (height > 2048) ||
                (depth  > 2048)) {
                return false;
            }
            break;
        default:
            return false;
    }

    //if (outIsCubeMap != nullptr)
    //{
    //    *outIsCubeMap = isCubeMap;
    //}
	*out_width = width;
	*out_height = height;
	*out_is_cube = (int32)isCubeMap;
	*out_mip_levels = mipCount;
	*out_format = DX2TF(format);

    return bitData;
}


//--------------------------------------------------------------------------------------
HRESULT LoadTextureDataFromMemory(
	uint8_t* ddsData,
	size_t ddsDataSize,
	const DDS_HEADER** header,
	uint8_t** bitData,
	size_t* bitSize) noexcept
{
	if (!header || !bitData || !bitSize)
	{
		return E_POINTER;
	}

	*bitSize = 0;

	if (ddsDataSize > UINT32_MAX)
	{
		return E_FAIL;
	}

	if (ddsDataSize < (sizeof(uint32_t) + sizeof(DDS_HEADER)))
	{
		return E_FAIL;
	}

	// DDS files always start with the same magic number ("DDS ")
	auto const dwMagicNumber = *reinterpret_cast<const uint32_t*>(ddsData);
	if (dwMagicNumber != DDS_MAGIC)
	{
		return E_FAIL;
	}

	auto hdr = reinterpret_cast<const DDS_HEADER*>(ddsData + sizeof(uint32_t));

	// Verify header to validate DDS file
	if (hdr->size != sizeof(DDS_HEADER) ||
		hdr->ddspf.size != sizeof(DDS_PIXELFORMAT))
	{
		return E_FAIL;
	}

	// Check for DX10 extension
	bool bDXT10Header = false;
	if ((hdr->ddspf.flags & DDS_FOURCC) &&
		(MAKEFOURCC('D', 'X', '1', '0') == hdr->ddspf.fourCC))
	{
		// Must be long enough for both headers and magic value
		if (ddsDataSize < (sizeof(uint32_t) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10)))
		{
			return E_FAIL;
		}

		bDXT10Header = true;
	}

	// setup the pointers in the process request
	*header = hdr;
	auto offset = sizeof(uint32_t)
	+ sizeof(DDS_HEADER)
	+ (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0u);
	*bitData = ddsData + offset;
	*bitSize = ddsDataSize - offset;

	return S_OK;
}


//--------------------------------------------------------------------------------------
#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

DXGI_FORMAT GetDXGIFormat(const DDS_PIXELFORMAT& ddpf) noexcept
{
    if (ddpf.flags & DDS_RGB)
    {
        // Note that sRGB formats are written using the "DX10" extended header

        switch (ddpf.RGBBitCount)
        {
            case 32:
                if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
                {
                    return DXGI_FORMAT_R8G8B8A8_UNORM;
                }

                if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
                {
                    return DXGI_FORMAT_B8G8R8A8_UNORM;
                }

                if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0))
                {
                    return DXGI_FORMAT_B8G8R8X8_UNORM;
                }

                // No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0) aka D3DFMT_X8B8G8R8

                // Note that many common DDS reader/writers (including D3DX) swap the
                // the RED/BLUE masks for 10:10:10:2 formats. We assume
                // below that the 'backwards' header mask is being used since it is most
                // likely written by D3DX. The more robust solution is to use the 'DX10'
                // header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

                // For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
                if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
                {
                    return DXGI_FORMAT_R10G10B10A2_UNORM;
                }

                // No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

                if (ISBITMASK(0x0000ffff, 0xffff0000, 0, 0))
                {
                    return DXGI_FORMAT_R16G16_UNORM;
                }

                if (ISBITMASK(0xffffffff, 0, 0, 0))
                {
                    // Only 32-bit color channel format in D3D9 was R32F
                    return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
                }
                break;

            case 24:
                // No 24bpp DXGI formats aka D3DFMT_R8G8B8
                break;

            case 16:
                if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
                {
                    return DXGI_FORMAT_B5G5R5A1_UNORM;
                }
                if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0))
                {
                    return DXGI_FORMAT_B5G6R5_UNORM;
                }

                // No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0) aka D3DFMT_X1R5G5B5

                if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
                {
                    return DXGI_FORMAT_B4G4R4A4_UNORM;
                }

                // NVTT versions 1.x wrote this as RGB instead of LUMINANCE
                if (ISBITMASK(0x00ff, 0, 0, 0xff00))
                {
                    return DXGI_FORMAT_R8G8_UNORM;
                }
                if (ISBITMASK(0xffff, 0, 0, 0))
                {
                    return DXGI_FORMAT_R16_UNORM;
                }

                // No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0) aka D3DFMT_X4R4G4B4

                // No 3:3:2:8 or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_A8P8, etc.
                break;

            case 8:
                // NVTT versions 1.x wrote this as RGB instead of LUMINANCE
                if (ISBITMASK(0xff, 0, 0, 0))
                {
                    return DXGI_FORMAT_R8_UNORM;
                }

                // No 3:3:2 or paletted DXGI formats aka D3DFMT_R3G3B2, D3DFMT_P8
                break;

            default:
                return DXGI_FORMAT_UNKNOWN;
        }
    }
    else if (ddpf.flags & DDS_LUMINANCE)
    {
        switch (ddpf.RGBBitCount)
        {
            case 16:
                if (ISBITMASK(0xffff, 0, 0, 0))
                {
                    return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
                }
                if (ISBITMASK(0x00ff, 0, 0, 0xff00))
                {
                    return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
                }
                break;

            case 8:
                if (ISBITMASK(0xff, 0, 0, 0))
                {
                    return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
                }

                // No DXGI format maps to ISBITMASK(0x0f,0,0,0xf0) aka D3DFMT_A4L4

                if (ISBITMASK(0x00ff, 0, 0, 0xff00))
                {
                    return DXGI_FORMAT_R8G8_UNORM; // Some DDS writers assume the bitcount should be 8 instead of 16
                }
                break;

            default:
                return DXGI_FORMAT_UNKNOWN;
        }
    }
    else if (ddpf.flags & DDS_ALPHA)
    {
        if (8 == ddpf.RGBBitCount)
        {
            return DXGI_FORMAT_A8_UNORM;
        }
    }
    else if (ddpf.flags & DDS_BUMPDUDV)
    {
        switch (ddpf.RGBBitCount)
        {
            case 32:
                if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
                {
                    return DXGI_FORMAT_R8G8B8A8_SNORM; // D3DX10/11 writes this out as DX10 extension
                }
                if (ISBITMASK(0x0000ffff, 0xffff0000, 0, 0))
                {
                    return DXGI_FORMAT_R16G16_SNORM; // D3DX10/11 writes this out as DX10 extension
                }

                // No DXGI format maps to ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000) aka D3DFMT_A2W10V10U10
                break;

            case 16:
                if (ISBITMASK(0x00ff, 0xff00, 0, 0))
                {
                    return DXGI_FORMAT_R8G8_SNORM; // D3DX10/11 writes this out as DX10 extension
                }
                break;

            default:
                return DXGI_FORMAT_UNKNOWN;
        }

        // No DXGI format maps to DDPF_BUMPLUMINANCE aka D3DFMT_L6V5U5, D3DFMT_X8L8V8U8
    }
    else if (ddpf.flags & DDS_FOURCC)
    {
        if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC1_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC2_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC3_UNORM;
        }

        // While pre-multiplied alpha isn't directly supported by the DXGI formats,
        // they are basically the same as these BC formats so they can be mapped
        if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC2_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC3_UNORM;
        }

        if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC4_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC4_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC4_SNORM;
        }

        if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC5_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC5_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
        {
            return DXGI_FORMAT_BC5_SNORM;
        }

        // BC6H and BC7 are written using the "DX10" extended header

        if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
        {
            return DXGI_FORMAT_R8G8_B8G8_UNORM;
        }
        if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
        {
            return DXGI_FORMAT_G8R8_G8B8_UNORM;
        }

        if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.fourCC)
        {
            return DXGI_FORMAT_YUY2;
        }

        // Check for D3DFORMAT enums being set here
        switch (ddpf.fourCC)
        {
            case 36: // D3DFMT_A16B16G16R16
                return DXGI_FORMAT_R16G16B16A16_UNORM;

            case 110: // D3DFMT_Q16W16V16U16
                return DXGI_FORMAT_R16G16B16A16_SNORM;

            case 111: // D3DFMT_R16F
                return DXGI_FORMAT_R16_FLOAT;

            case 112: // D3DFMT_G16R16F
                return DXGI_FORMAT_R16G16_FLOAT;

            case 113: // D3DFMT_A16B16G16R16F
                return DXGI_FORMAT_R16G16B16A16_FLOAT;

            case 114: // D3DFMT_R32F
                return DXGI_FORMAT_R32_FLOAT;

            case 115: // D3DFMT_G32R32F
                return DXGI_FORMAT_R32G32_FLOAT;

            case 116: // D3DFMT_A32B32G32R32F
                return DXGI_FORMAT_R32G32B32A32_FLOAT;

                // No DXGI format maps to D3DFMT_CxV8U8

            default:
                return DXGI_FORMAT_UNKNOWN;
        }
    }

    return DXGI_FORMAT_UNKNOWN;
}

#undef ISBITMASK

//--------------------------------------------------------------------------------------
// Return the BPP for a particular format
//--------------------------------------------------------------------------------------
size_t BitsPerPixel(DXGI_FORMAT fmt) noexcept
{
    switch (fmt)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return 128;

        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
            return 96;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        case DXGI_FORMAT_Y416:
        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
            return 64;

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_AYUV:
        case DXGI_FORMAT_Y410:
        case DXGI_FORMAT_YUY2:
            return 32;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
        case DXGI_FORMAT_V408:
            return 24;

        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        case DXGI_FORMAT_A8P8:
        case DXGI_FORMAT_B4G4R4A4_UNORM:
        case DXGI_FORMAT_P208:
        case DXGI_FORMAT_V208:
            return 16;

        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
        case DXGI_FORMAT_NV11:
            return 12;

        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
        case DXGI_FORMAT_AI44:
        case DXGI_FORMAT_IA44:
        case DXGI_FORMAT_P8:
            return 8;

        case DXGI_FORMAT_R1_UNORM:
            return 1;

        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            return 4;

        default:
            return 0;
    }
}

Texture_Format DX2TF(DXGI_FORMAT dx_format) noexcept {
	Texture_Format format = TEXTURE_FORMAT_UNKNOWN;
	switch (dx_format) {
		case DXGI_FORMAT_UNKNOWN: 					{format = TEXTURE_FORMAT_UNKNOWN; } break;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:		{format = TEXTURE_FORMAT_RGBA_FLOAT32; } break;
		case DXGI_FORMAT_R32G32B32_FLOAT:			{format = TEXTURE_FORMAT_RGB_FLOAT32; } break;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:		{format = TEXTURE_FORMAT_RGBA_FLOAT16; } break;
		case DXGI_FORMAT_R32G32_FLOAT:    			{format = TEXTURE_FORMAT_RG_FLOAT32; } break;
		case DXGI_FORMAT_R8G8B8A8_UNORM: 			{format = TEXTURE_FORMAT_RGBA_U8_NORM; } break;
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: 		{format = TEXTURE_FORMAT_RGBA_U8_NORM_SRGB; } break;
		case DXGI_FORMAT_R16G16_FLOAT: 				{format = TEXTURE_FORMAT_RG_FLOAT16; } break;
		case DXGI_FORMAT_R16G16_UNORM: 				{format = TEXTURE_FORMAT_RG_U16_NORM; } break;
		case DXGI_FORMAT_D32_FLOAT: 				{format = TEXTURE_FORMAT_DEPTH_32F; } break;
		case DXGI_FORMAT_R32_FLOAT: 				{format = TEXTURE_FORMAT_R_FLOAT32; } break;
		case DXGI_FORMAT_D24_UNORM_S8_UINT: 		{format = TEXTURE_FORMAT_DEPTH_24F_STENCIL_U8; } break;
		case DXGI_FORMAT_R8G8_UNORM: 				{format = TEXTURE_FORMAT_RG_U8_NORM; } break;
		case DXGI_FORMAT_R16_FLOAT: 				{format = TEXTURE_FORMAT_R_FLOAT16; } break;
		case DXGI_FORMAT_D16_UNORM: 				{format = TEXTURE_FORMAT_DEPTH_16F; } break;
		case DXGI_FORMAT_R16_UNORM: 				{format = TEXTURE_FORMAT_R_U16_NORM; } break;
		case DXGI_FORMAT_R8_UNORM:					{format = TEXTURE_FORMAT_R_U8_NORM; } break;
		case DXGI_FORMAT_BC1_UNORM: 				{format = TEXTURE_FORMAT_BC1_UNORM; } break;
		case DXGI_FORMAT_BC1_UNORM_SRGB:			{format = TEXTURE_FORMAT_BC1_UNORM_SRGB; } break;
		case DXGI_FORMAT_BC2_UNORM: 				{format = TEXTURE_FORMAT_BC2_UNORM; } break;
		case DXGI_FORMAT_BC2_UNORM_SRGB: 			{format = TEXTURE_FORMAT_BC2_UNORM_SRGB; } break;
		case DXGI_FORMAT_BC3_UNORM:					{format = TEXTURE_FORMAT_BC3_UNORM; } break;
		case DXGI_FORMAT_BC3_UNORM_SRGB: 			{format = TEXTURE_FORMAT_BC3_UNORM_SRGB; } break;
		case DXGI_FORMAT_BC4_UNORM: 				{format = TEXTURE_FORMAT_BC4_UNORM; } break;
		case DXGI_FORMAT_BC4_SNORM: 				{format = TEXTURE_FORMAT_BC4_SNORM; } break;
		case DXGI_FORMAT_BC5_UNORM:  				{format = TEXTURE_FORMAT_BC5_UNORM; } break;
		case DXGI_FORMAT_BC5_SNORM:  				{format = TEXTURE_FORMAT_BC5_SNORM; } break;
		case DXGI_FORMAT_BC7_UNORM:      	  		{format = TEXTURE_FORMAT_BC7_UNORM; } break;
		case DXGI_FORMAT_BC7_UNORM_SRGB: 	  		{format = TEXTURE_FORMAT_BC7_UNORM_SRGB; } break;
	}

	return format;
}