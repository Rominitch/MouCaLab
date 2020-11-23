#include "Dependancies.h"

#include <LibRT/include/RTBufferCPU.h>
#include <LibRT/include/RTImage.h>

#include <LibGUI/include/GUIFont.h>

namespace GUI
{

void Font::CreateTextureFont(const Core::Path& strFontTTLUnicode, Core::FileWrapperBase& FileOutput)
{
    BT_ASSERT(!strFontTTLUnicode.empty());

    try
    {
        //Create font file
        FileOutput.open(L"w");

        std::map<unsigned int, STextureGlyph> mapSymbols;

        const size_t szTextureSize = 1024;
        const int iHeightInPixel = 52;

        // Create And Initialize A FreeType Font Library.
        FT_Library FreeFontLibrary;
        if(FT_Init_FreeType(&FreeFontLibrary))
            BT_THROW_ERROR(u8"ModuleError", u8"InitializeError");
 
        // The Object In Which FreeType Holds Information On A Given
        // Font Is Called A "face".
        FT_Face face;
 
        // This Is Where We Load In The Font Information From The File.
        // Of All The Places Where The Code Might Die, This Is The Most Likely,
        // As FT_New_Face Will Fail If The Font File Does Not Exist Or Is Somehow Broken.
        Core::String u8FontTTLUnicode = Core::convertToU8(strFontTTLUnicode);
        if(FT_New_Face(FreeFontLibrary, u8FontTTLUnicode.c_str(), 0, &face))
            BT_THROW_ERROR_1(u8"ModuleError", u8"FontFileMissingError", u8FontTTLUnicode);
 
        //Set size for face rendering
        FT_Set_Pixel_Sizes(face, 0, iHeightInPixel);
        
        //Create picture
        RT::BufferCPU pictureImage;
        RT::BufferDescriptor bufferDescriptor;
        bufferDescriptor.addDescriptor(RT::ComponentDescriptor(3, RT::Type::UnsignedChar, RT::ComponentUsage::Color));
        Core::SPictureUC24* pPicture = (Core::SPictureUC24*)pictureImage.create(bufferDescriptor, szTextureSize*szTextureSize);

        /*
        for(size_t szHeight=0; szHeight < szTextureSize; ++szHeight)
        {
            const float fColorH = ((float)szHeight/(float)szTextureSize)*255.0f;

            for(size_t szWidth=0; szWidth < szTextureSize; ++szWidth)
            {
                const float fColorW = ((float)szWidth/(float)szTextureSize)*255.0f;

                pPicture[szWidth + szHeight*szTextureSize].m_color[0] = (unsigned char)fColorW;
                pPicture[szWidth + szHeight*szTextureSize].m_color[1] = (unsigned char)fColorH;
                pPicture[szWidth + szHeight*szTextureSize].m_color[2] = 0;
            }
        }
        */

        size_t szStartPointer[2] = {0, szTextureSize-1};

        const unsigned int iSymboleList[] = 
        {
            0x21, 0x7E,	//ASCII
            0xA1, 0xFF, //Latin 1
            0x100, 0x17F, //Latin Etendu A
/*
            0x3041, 0x309F,	//Hiragana
            0x30A0, 0x30FF,	//Katakana
            0x4E00, 0x9FBB  //Chinois
*/
        };
        const size_t szNbSymboles = (sizeof(iSymboleList)/sizeof(unsigned int));

        for(size_t szSymbole=0; szSymbole < szNbSymboles; szSymbole+=2)
        {
            try
            {
                for(unsigned int iCharCode=iSymboleList[szSymbole]; iCharCode<iSymboleList[szSymbole+1]; iCharCode++)
                {
                    FT_Glyph glyph = CreateGlyph(face, iCharCode);
                    const FT_BitmapGlyph bitmap_glyph = (const FT_BitmapGlyph)glyph;

                    // This Reference Will Make Accessing The Bitmap Easier.
                    FT_Bitmap& bitmap = bitmap_glyph->bitmap;

                    //Add new glyph to symbols Map
                    BT_ASSERT(mapSymbols.find(iCharCode) == mapSymbols.cend());
                    //mapSymbols[iCharCode] = STextureGlyph(iCharCode, );

                    if(szStartPointer[0] + bitmap.width > szTextureSize)
                    {
                        szStartPointer[0] = 0;
                        if(szStartPointer[1] <= 2*iHeightInPixel)
                        {
                            FT_Done_Glyph(glyph);
                            throw "quit";
                        }

                        szStartPointer[1] -= iHeightInPixel;
                    }

                    for(size_t szY=0; szY < bitmap.rows; ++szY)
                    {
                        const size_t YPos = (szStartPointer[1] - szY)*szTextureSize;
                        const size_t YGlyph = bitmap.width*szY;

                        for(size_t szX=0; szX < bitmap.width; ++szX)
                        {
                            const size_t XPos = szStartPointer[0] + szX;
                            pPicture[XPos + YPos].m_color[0] = bitmap.buffer[szX + YGlyph];
                            pPicture[XPos + YPos].m_color[1] = bitmap.buffer[szX + YGlyph];
                            pPicture[XPos + YPos].m_color[2] = bitmap.buffer[szX + YGlyph];
                        }
                    }

                    szStartPointer[0] += bitmap.width;

        // 			RTCopy2D transferToBuffer;
        // 			transferToBuffer.Initialize();

                    FT_Done_Glyph(glyph);
                }
            }
            catch(...)
            {
                szSymbole = szNbSymboles;
            }
        }
/*
        RT::Image fontImage;
        fontImage.createImage(pictureImage, szTextureSize, szTextureSize);
        fontImage.saveImage(L"MyFontPicture.png");
*/
        FT_Done_Face(face);
        FT_Done_FreeType(FreeFontLibrary);
    }
    catch(const Core::Exception&)
    {
        FileOutput.close();
        throw;
    }

    FileOutput.close();
}

FT_Glyph Font::CreateGlyph(const FT_Face& face, unsigned int iCharCode)
{
    // The First Thing We Do Is Get FreeType To Render Our Character
    // Into A Bitmap.  This Actually Requires A Couple Of FreeType Commands:

    // Load The Glyph For Our Character.
    if(FT_Load_Glyph(face, FT_Get_Char_Index(face, iCharCode), FT_LOAD_DEFAULT))
        BT_THROW_ERROR_1(u8"ModuleError", u8"LoadGlyphError", std::to_string(iCharCode));
 
    // Move The Face's Glyph Into A Glyph Object.
    FT_Glyph glyph;
    if(FT_Get_Glyph( face->glyph, &glyph ))
        BT_THROW_ERROR_1(u8"ModuleError", u8"GetGlyphError", std::to_string(iCharCode));
 
    // Convert The Glyph To A Bitmap.
    FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
    return glyph;
/*	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

    // This Reference Will Make Accessing The Bitmap Easier.
    FT_Bitmap& bitmap=bitmap_glyph->bitmap;
    
    int width = bitmap.width;
    int height = bitmap.rows;
 
    // Allocate Memory For The Texture Data.
    GLubyte* expanded_data = new GLubyte[ 2 * width * height];
 
    // Here We Fill In The Data For The Expanded Bitmap.
    // Notice That We Are Using A Two Channel Bitmap (One For
    // Channel Luminosity And One For Alpha), But We Assign
    // Both Luminosity And Alpha To The Value That We
    // Find In The FreeType Bitmap.
    // We Use The ?: Operator To Say That Value Which We Use
    // Will Be 0 If We Are In The Padding Zone, And Whatever
    // Is The FreeType Bitmap Otherwise.
    for(int j=0; j <height;j++)
    {
        for(int i=0; i < width; i++)
        {
            expanded_data[2*(i+j*width)]= expanded_data[2*(i+j*width)+1] =
                (i>=bitmap.width || j>=bitmap.rows) ?
                0 : bitmap.buffer[i + bitmap.width*j];
        }
    }
*/
}

}

void GUIFontAtlasOld::CreateFont(const Core::FileWrapperBase& FontFile)
{
}

void GUIFontAtlasOld::Draw()
{
}