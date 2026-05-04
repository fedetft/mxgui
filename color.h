/***************************************************************************
 *   Copyright (C) 2010, 2011 by Terraneo Federico                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#pragma once

#include <config/mxgui_settings.h>

namespace mxgui {

// Move RGB565Color definition above all uses
/**
 * \ingroup pub_iface
 * An RGB565Color is used to represent colors in the RGB565 format.
 * It is suggested to work with the Color typedef to keep the program portable
 * to different color formats.
 */
class RGB565Color
{
public:
    constexpr RGB565Color() : color(0) {}
    
    /**
     * Construct a color from red, green, blue values
     * \param r red value (0-255)
     * \param g green value (0-255)
     * \param b blue value (0-255)
     */
    constexpr RGB565Color(unsigned char r, unsigned char g, unsigned char b)
        : color(((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3)) {}

    /**
     * Get the red channel value, upscaled to 8 bits
     * \return red value (0-255)
     */
    constexpr unsigned char getR() const
    {
        unsigned char r = (color >> 8) & 0xf8;
        return r | (r >> 5);
    }

    /**
     * Get the green channel value, upscaled to 8 bits
     * \return green value (0-255)
     */
    constexpr unsigned char getG() const
    {
        unsigned char g = (color >> 3) & 0xfc;
        return g | (g >> 6);
    }

    /**
     * Get the blue channel value, upscaled to 8 bits
     * \return blue value (0-255)
     */
    constexpr unsigned char getB() const
    {
        unsigned char b = (color & 0x1f) << 3;
        return b | (b >> 5);
    }

    /** Comparison operators */
    constexpr bool operator==(const RGB565Color& other) const { return color == other.color; }
    constexpr bool operator!=(const RGB565Color& other) const { return color != other.color; }
    
    /** Explicit conversion to the raw data type */
    constexpr operator unsigned short() const { return color; }

    /**
     * Creates a color object from the raw data type
     * \param raw an unsigned short representing the raw color value
     */
    static constexpr RGB565Color fromRaw(unsigned short raw)
    {
        RGB565Color c;
        c.color = raw;
        return c;
    }

    /**
     * Creates a color object from the raw data type of an RGB565 color
     * \param c an unsigned short representing the raw color value of an RGB565 color
     */
    static constexpr RGB565Color fromRGB565(unsigned short c) { return fromRaw(c); }

    static constexpr RGB565Color black() { return fromRaw(0x0000); }
    static constexpr RGB565Color white() { return fromRaw(0xffff); }

    /**
     * Linearly interpolate between two colors
     * \param c1 the first color
     * \param c2 the second color
     * \param scale a blend factor (0-255), where 0 is entirely c1, 255 is entirely c2
     */
    static constexpr RGB565Color mix(RGB565Color c1, RGB565Color c2, unsigned char scale)
    {
        unsigned short s = scale;
        unsigned short inv = 255 - s;

        unsigned short r_val = (static_cast<unsigned short>(c1.color >> 11) * inv) +
                               (static_cast<unsigned short>(c2.color >> 11) * s);
        unsigned short r = (r_val + 1 + (r_val >> 8)) >> 8;

        unsigned short g_val = (static_cast<unsigned short>((c1.color & 0x07E0) >> 5) * inv) +
                               (static_cast<unsigned short>((c2.color & 0x07E0) >> 5) * s);
        unsigned short g = (g_val + 1 + (g_val >> 8)) >> 8;

        unsigned short b_val = (static_cast<unsigned short>(c1.color & 0x001F) * inv) +
                               (static_cast<unsigned short>(c2.color & 0x001F) * s);
        unsigned short b = (b_val + 1 + (b_val >> 8)) >> 8;

        return fromRaw( static_cast<unsigned short>((r << 11) | (g << 5) | b) );
    }

private:
    unsigned short color;
};

/**
 * \ingroup pub_iface
 * A Gray1Color is used to represent colors with 1 bit (black or white).
 * It is suggested to work with the Color typedef to keep the program portable
 * to different color formats.
 */
class Gray1Color
{
public:
    constexpr Gray1Color() : color(0) {}
    
    /**
     * Construct a color from red, green, blue values
     * \param r red value (0-255)
     * \param g green value (0-255)
     * \param b blue value (0-255)
     */
    constexpr Gray1Color(unsigned char r, unsigned char g, unsigned char b)
        : color(((static_cast<short>(r) + g + b) >= MXGUI_1BPP_THRESHOLD) ? 1 : 0) {}

    /**
     * Get the red channel value, upscaled to 8 bits
     * \return red value (0-255)
     */
    constexpr unsigned char getR() const { return color ? 255 : 0; }

    /**
     * Get the green channel value, upscaled to 8 bits
     * \return green value (0-255)
     */
    constexpr unsigned char getG() const { return color ? 255 : 0; }

    /**
     * Get the blue channel value, upscaled to 8 bits
     * \return blue value (0-255)
     */
    constexpr unsigned char getB() const { return color ? 255 : 0; }

    /** Comparison operators */
    constexpr bool operator==(const Gray1Color& other) const { return color == other.color; }
    constexpr bool operator!=(const Gray1Color& other) const { return color != other.color; }
    
    /** Explicit conversion to the raw data type */
    constexpr operator unsigned char() const { return color; }

    /**
     * Creates a color object from the raw data type
     * \param raw an unsigned char representing the raw color value
     */
    static constexpr Gray1Color fromRaw(unsigned char raw)
    {
        Gray1Color c;
        c.color = raw & 0x01;
        return c;
    }

    /**
     * Creates a color object from the raw data type of an RGB565 color
     * \param c an unsigned short representing the raw color value of an RGB565 color
     */
    static constexpr Gray1Color fromRGB565(unsigned short c)
    {
        RGB565Color col = RGB565Color::fromRaw(c);
        return Gray1Color(col.getR(), col.getG(), col.getB());
    }

    static constexpr Gray1Color black() { return fromRaw(0); }
    static constexpr Gray1Color white() { return fromRaw(1); }

    /**
     * Interpolate between two colors. Mix for 1bpp is thresholding.
     * \param c1 the first color
     * \param c2 the second color
     * \param scale a blend factor (0-255), where 0 is entirely c1, 255 is entirely c2
     */
    static constexpr Gray1Color mix(Gray1Color c1, Gray1Color c2, unsigned char scale)
    {
        if (c1 == c2) return c1;
        if (c1 == black())
            return (scale >= (MXGUI_1BPP_THRESHOLD / 3)) ? c2 : c1;
        else
            return ((255 - scale) >= (MXGUI_1BPP_THRESHOLD / 3)) ? c1 : c2;
    }

private:
    unsigned char color;
};

/**
 * \ingroup pub_iface
 * A Gray4Color is used to represent colors with 4 bits (grayscale).
 * It is suggested to work with the Color typedef to keep the program portable
 * to different color formats.
 */
class Gray4Color
{
public:
    constexpr Gray4Color() : color(0) {}
    
    /**
     * Construct a color from red, green, blue values
     * \param r red value (0-255)
     * \param g green value (0-255)
     * \param b blue value (0-255)
     */
    constexpr Gray4Color(unsigned char r, unsigned char g, unsigned char b)
        : color(static_cast<unsigned char>(
            (static_cast<unsigned short>(r) * 77u +
             static_cast<unsigned short>(g) * 150u +
             static_cast<unsigned short>(b) * 29u) >> 12)) {}

    /**
     * Get the red channel value, upscaled to 8 bits
     * \return red value (0-255)
     */
    constexpr unsigned char getR() const { return (color << 4) | color; }

    /**
     * Get the green channel value, upscaled to 8 bits
     * \return green value (0-255)
     */
    constexpr unsigned char getG() const { return (color << 4) | color; }

    /**
     * Get the blue channel value, upscaled to 8 bits
     * \return blue value (0-255)
     */
    constexpr unsigned char getB() const { return (color << 4) | color; }

    /** Comparison operators */
    constexpr bool operator==(const Gray4Color& other) const { return color == other.color; }
    constexpr bool operator!=(const Gray4Color& other) const { return color != other.color; }
    
    /** Explicit conversion to the raw data type */
    constexpr operator unsigned char() const { return color; }

    /**
     * Creates a color object from the raw data type
     * \param raw an unsigned char representing the raw color value
     */
    static constexpr Gray4Color fromRaw(unsigned char raw)
    {
        Gray4Color c;
        c.color = raw & 0x0F;
        return c;
    }

    /**
     * Creates a color object from the raw data type of an RGB565 color
     * \param c an unsigned short representing the raw color value of an RGB565 color
     */
    static constexpr Gray4Color fromRGB565(unsigned short c)
    {
        RGB565Color col = RGB565Color::fromRaw(c);
        return Gray4Color(col.getR(), col.getG(), col.getB());
    }
    
    static constexpr Gray4Color black() { return fromRaw(0x0); }
    static constexpr Gray4Color white() { return fromRaw(0xF); }

    /**
     * Linearly interpolate between two colors
     * \param c1 the first color
     * \param c2 the second color
     * \param scale a blend factor (0-255), where 0 is entirely c1, 255 is entirely c2
     */
    static constexpr Gray4Color mix(Gray4Color c1, Gray4Color c2, unsigned char scale)
    {
        unsigned short s = scale;
        unsigned short inv = 255 - s;
        unsigned short val = static_cast<unsigned short>(c1.color) * inv + static_cast<unsigned short>(c2.color) * s;
        unsigned short res = (val + 1 + (val >> 8)) >> 8;
        return fromRaw( static_cast<unsigned char>(res & 0x0F) );
    }

private:
    unsigned char color;
};

/**
 * \ingroup pub_iface
 * An RGB332Color is used to represent colors in the RGB332 format.
 * It is suggested to work with the Color typedef to keep the program portable
 * to different color formats.
 */
class RGB332Color
{
public:
    constexpr RGB332Color() : color(0) {}
    
    /**
     * Construct a color from red, green, blue values
     * \param r red value (0-255)
     * \param g green value (0-255)
     * \param b blue value (0-255)
     */
    constexpr RGB332Color(unsigned char r, unsigned char g, unsigned char b)
        : color((r & 0xe0) | ((g & 0xe0) >> 3) | (b >> 6)) {}

    /**
     * Get the red channel value, upscaled to 8 bits
     * \return red value (0-255)
     */
    constexpr unsigned char getR() const
    {
        unsigned char r = color & 0xe0;
        return r | (r >> 3) | (r >> 6);
    }

    /**
     * Get the green channel value, upscaled to 8 bits
     * \return green value (0-255)
     */
    constexpr unsigned char getG() const
    {
        unsigned char g = (color << 3) & 0xe0;
        return g | (g >> 3) | (g >> 6);
    }

    /**
     * Get the blue channel value, upscaled to 8 bits
     * \return blue value (0-255)
     */
    constexpr unsigned char getB() const
    {
        unsigned char b = (color & 0x03);
        b |= b << 2;
        return b | (b << 4);
    }

    /** Comparison operators */
    constexpr bool operator==(const RGB332Color& other) const { return color == other.color; }
    constexpr bool operator!=(const RGB332Color& other) const { return color != other.color; }

    /** Explicit conversion to the raw data type */
    constexpr operator unsigned char() const { return color; }

    /**
     * Creates a color object from the raw data type
     * \param raw an unsigned char representing the raw color value
     */
    static constexpr RGB332Color fromRaw(unsigned char raw)
    {
        RGB332Color c;
        c.color = raw;
        return c;
    }

    /**
     * Creates a color object from the raw data type of an RGB565 color
     * \param c an unsigned short representing the raw color value of an RGB565 color
     */
    static constexpr RGB332Color fromRGB565(unsigned short c)
    {
        RGB565Color col = RGB565Color::fromRaw(c);
        return RGB332Color(col.getR(), col.getG(), col.getB());
    }

    static constexpr RGB332Color black() { return fromRaw(0x00); }
    static constexpr RGB332Color white() { return fromRaw(0xff); }

    /**
     * Linearly interpolate between two colors
     * \param c1 the first color
     * \param c2 the second color
     * \param scale a blend factor (0-255), where 0 is entirely c1, 255 is entirely c2
     */
    static constexpr RGB332Color mix(RGB332Color c1, RGB332Color c2, unsigned char scale)
    {
        unsigned short s = scale;
        unsigned short inv = 255 - s;

        unsigned short r_val = (static_cast<unsigned short>(c1.color & 0xE0) * inv) + (static_cast<unsigned short>(c2.color & 0xE0) * s);
        unsigned short r = (r_val + 1 + (r_val >> 8)) >> 8;

        unsigned short g_val = (static_cast<unsigned short>(c1.color & 0x1C) * inv) + (static_cast<unsigned short>(c2.color & 0x1C) * s);
        unsigned short g = (g_val + 1 + (g_val >> 8)) >> 8;

        unsigned short b_val = (static_cast<unsigned short>(c1.color & 0x03) * inv) + (static_cast<unsigned short>(c2.color & 0x03) * s);
        unsigned short b = (b_val + 1 + (b_val >> 8)) >> 8;

        return fromRaw( static_cast<unsigned char>((r & 0xE0) | (g & 0x1C) | (b & 0x03)) );
    }

private:
    unsigned char color;
};

/**
 * \ingroup pub_iface
 * An RGB888Color is used to represent colors in the RGB888 format.
 * For now, as there are no drivers using this format,
 * this can be used to have a program perform computations on 24-bit color depth,
 * and then construct on the fly the correct Color type to interface with the display,
 * decoupling the program color precision from the display pixel format.
 */
class RGB888Color
{
public:
    constexpr RGB888Color() : color(0) {}
    
    /**
     * Construct a color from red, green, blue values
     * \param r red value (0-255)
     * \param g green value (0-255)
     * \param b blue value (0-255)
     */
    constexpr RGB888Color(unsigned char r, unsigned char g, unsigned char b)
        : color((static_cast<unsigned int>(r) << 16) |
                (static_cast<unsigned int>(g) << 8) | b) {}

    /**
     * Get the red channel value, upscaled to 8 bits
     * \return red value (0-255)
     */
    constexpr unsigned char getR() const { return (color >> 16) & 0xFF; }

    /**
     * Get the green channel value, upscaled to 8 bits
     * \return green value (0-255)
     */
    constexpr unsigned char getG() const { return (color >> 8) & 0xFF; }

    /**
     * Get the blue channel value, upscaled to 8 bits
     * \return blue value (0-255)
     */
    constexpr unsigned char getB() const { return color & 0xFF; }

    /** Comparison operators */
    constexpr bool operator==(const RGB888Color& other) const { return color == other.color; }
    constexpr bool operator!=(const RGB888Color& other) const { return color != other.color; }

    /** Explicit conversion to the raw data type */
    constexpr operator unsigned int() const { return color; }

    /**
     * Creates a color object from the raw data type
     * \param raw an unsigned int representing the raw color value
     */
    static constexpr RGB888Color fromRaw(unsigned int raw)
    {
        RGB888Color c;
        c.color = raw & 0x00FFFFFF;
        return c;
    }

    /**
     * Creates a color object from the raw data type of an RGB565 color
     * \param c an unsigned short representing the raw color value of an RGB565 color
     */
    static constexpr RGB888Color fromRGB565(unsigned short c)
    {
        RGB565Color col = RGB565Color::fromRaw(c);
        return RGB888Color(col.getR(), col.getG(), col.getB());
    }

    static constexpr RGB888Color black() { return fromRaw(0x000000); }
    static constexpr RGB888Color white() { return fromRaw(0xFFFFFF); }

    /**
     * Linearly interpolate between two colors
     * \param c1 the first color
     * \param c2 the second color
     * \param scale a blend factor (0-255), where 0 is entirely c1, 255 is entirely c2
     */
    static constexpr RGB888Color mix(RGB888Color c1, RGB888Color c2, unsigned char scale)
    {
        unsigned int v1 = static_cast<unsigned int>(c1), v2 = static_cast<unsigned int>(c2);
        unsigned short s = scale, inv = 255 - s;
        
        unsigned int r_val = ((v1 >> 16) & 0xFF) * inv + ((v2 >> 16) & 0xFF) * s;
        unsigned char r = (r_val + 1 + (r_val >> 8)) >> 8;
        
        unsigned int g_val = ((v1 >>  8) & 0xFF) * inv + ((v2 >>  8) & 0xFF) * s;
        unsigned char g = (g_val + 1 + (g_val >> 8)) >> 8;
        
        unsigned int b_val = (v1 & 0xFF) * inv + (v2 & 0xFF) * s;
        unsigned char b = (b_val + 1 + (b_val >> 8)) >> 8;
        
        return RGB888Color(r, g, b);
    }

private:
    unsigned int color;
};


// Gray1 Operations
constexpr Gray1Color operator+(Gray1Color a, Gray1Color b)
{
    return Gray1Color::fromRaw(static_cast<unsigned char>(a) | static_cast<unsigned char>(b));
}
constexpr Gray1Color operator-(Gray1Color a, Gray1Color b)
{
    return Gray1Color::fromRaw(static_cast<unsigned char>(a) & ~static_cast<unsigned char>(b));
}
constexpr Gray1Color operator|(Gray1Color a, Gray1Color b)
{
    return Gray1Color::fromRaw(static_cast<unsigned char>(a) | static_cast<unsigned char>(b));
}

// Gray4 Operations
constexpr Gray4Color operator+(Gray4Color a, Gray4Color b)
{
    unsigned short val = static_cast<unsigned char>(a) + static_cast<unsigned char>(b);
    if(val > 0xF) val = 0xF;
    return Gray4Color::fromRaw(static_cast<unsigned char>(val));
}
constexpr Gray4Color operator-(Gray4Color a, Gray4Color b)
{
    unsigned char ca = a;
    unsigned char cb = b;
    unsigned char val = ca - cb;
    if(val > ca) val = 0;
    return Gray4Color::fromRaw(val);
}
constexpr Gray4Color operator|(Gray4Color a, Gray4Color b)
{
    return Gray4Color::fromRaw(static_cast<unsigned char>(a) | static_cast<unsigned char>(b));
}

// RGB332 Operations
constexpr RGB332Color operator+(RGB332Color a, RGB332Color b)
{
    unsigned char ca = a;
    unsigned char cb = b;
    unsigned short intermediate = static_cast<unsigned short>(ca & 0xE0) + (cb & 0xE0);
    if (intermediate > 0xE0) intermediate = 0xE0;

    unsigned short g = static_cast<unsigned short>(ca & 0x1C) + (cb & 0x1C);
    if (g > 0x1C) g = 0x1C;
    intermediate |= g;
    unsigned short b_val = static_cast<unsigned short>(ca & 0x03) + (cb & 0x03);
    if (b_val > 0x03) b_val = 0x03;
    intermediate |= b_val;

    return RGB332Color::fromRaw(static_cast<unsigned char>(intermediate));
}
constexpr RGB332Color operator-(RGB332Color a, RGB332Color b)
{
    unsigned char ca = a;
    unsigned char cb = b;

    unsigned char ra = ca & 0xE0;
    unsigned char rb = cb & 0xE0;
    unsigned char r = (ra > rb) ? ra - rb : 0;

    unsigned char ga = ca & 0x1C;
    unsigned char gb = cb & 0x1C;
    unsigned char g = (ga > gb) ? ga - gb : 0;

    unsigned char ba = ca & 0x03;
    unsigned char bb = cb & 0x03;
    unsigned char b_val = (ba > bb) ? ba - bb : 0;

    return RGB332Color::fromRaw(r | g | b_val);
}
constexpr RGB332Color operator|(RGB332Color a, RGB332Color b)
{
    return RGB332Color::fromRaw(static_cast<unsigned char>(a) | static_cast<unsigned char>(b));
}

// RGB565 Operations
constexpr RGB565Color operator+(RGB565Color a, RGB565Color b)
{
    unsigned short ca = a;
    unsigned short cb = b;

    unsigned short ra = ca & 0xF800;
    unsigned short rb = cb & 0xF800;
    unsigned short r = ra + rb;
    if (r < ra || r > 0xF800) r = 0xF800;

    unsigned short ga = ca & 0x07E0;
    unsigned short gb = cb & 0x07E0;
    unsigned short g = ga + gb;
    if (g > 0x07E0) g = 0x07E0;

    unsigned short ba = ca & 0x001F;
    unsigned short bb = cb & 0x001F;
    unsigned short b_val = ba + bb;
    if (b_val > 0x001F) b_val = 0x001F;

    return RGB565Color::fromRaw(r | g | b_val);
}
constexpr RGB565Color operator-(RGB565Color a, RGB565Color b)
{
    unsigned short ca = a;
    unsigned short cb = b;
    unsigned short ra = ca & 0xF800;
    unsigned short rb = cb & 0xF800;
    unsigned short r = (ra > rb) ? ra - rb : 0;

    unsigned short ga = ca & 0x07E0;
    unsigned short gb = cb & 0x07E0;
    unsigned short g = (ga > gb) ? ga - gb : 0;

    unsigned short ba = ca & 0x001F;
    unsigned short bb = cb & 0x001F;
    unsigned short b_val = (ba > bb) ? ba - bb : 0;

    return RGB565Color::fromRaw(r | g | b_val);
}
constexpr RGB565Color operator|(RGB565Color a, RGB565Color b)
{
    return RGB565Color::fromRaw(static_cast<unsigned short>(a) | static_cast<unsigned short>(b));
}

// RGB888 Operations (computation aid, not a pixel format)
constexpr RGB888Color operator+(RGB888Color a, RGB888Color b)
{
    unsigned int va = static_cast<unsigned int>(a);
    unsigned int vb = static_cast<unsigned int>(b);
    unsigned short r = ((va >> 16) & 0xFF) + ((vb >> 16) & 0xFF);
    if (r > 255) r = 255;
    unsigned short g = ((va >> 8) & 0xFF) + ((vb >> 8) & 0xFF);
    if (g > 255) g = 255;
    unsigned short b_val = (va & 0xFF) + (vb & 0xFF);
    if (b_val > 255) b_val = 255;
    return RGB888Color(r, g, b_val);
}
constexpr RGB888Color operator-(RGB888Color a, RGB888Color b)
{
    unsigned int va = static_cast<unsigned int>(a);
    unsigned int vb = static_cast<unsigned int>(b);
    unsigned char ra = (va >> 16) & 0xFF, rb = (vb >> 16) & 0xFF;
    unsigned char ga = (va >>  8) & 0xFF, gb = (vb >>  8) & 0xFF;
    unsigned char ba =  va        & 0xFF, bb =  vb        & 0xFF;
    return RGB888Color(ra > rb ? ra - rb : 0,
                       ga > gb ? ga - gb : 0,
                       ba > bb ? ba - bb : 0);
}
constexpr RGB888Color operator|(RGB888Color a, RGB888Color b)
{
    return RGB888Color::fromRaw(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}

// Check there is only one active Color type
#if (defined(MXGUI_PIXEL_FORMAT_GRAY1) + defined(MXGUI_PIXEL_FORMAT_GRAY4) + \
     defined(MXGUI_PIXEL_FORMAT_RGB332) + defined(MXGUI_PIXEL_FORMAT_RGB565)) > 1
#error "Multiple pixel formats defined in mxgui_settings.h"
#endif

#if defined(MXGUI_PIXEL_FORMAT_GRAY1)
typedef Gray1Color Color;
#elif defined(MXGUI_PIXEL_FORMAT_GRAY4)
typedef Gray4Color Color;
#elif defined(MXGUI_PIXEL_FORMAT_RGB332)
typedef RGB332Color Color;
#elif defined(MXGUI_PIXEL_FORMAT_RGB565)
typedef RGB565Color Color;
#else
#error "No pixel format defined"
#endif

} // namespace mxgui
