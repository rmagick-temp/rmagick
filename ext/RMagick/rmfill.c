/* $Id: rmfill.c,v 1.11 2005/03/05 16:18:39 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2005 by Timothy P. Hunter
| Name:     rmfill.c
| Author:   Tim Hunter
| Purpose:  GradientFill, TextureFill class definitions for RMagick
\============================================================================*/

#include "rmagick.h"

typedef struct
{
    double x1, y1, x2, y2;
    PixelPacket start_color;
    PixelPacket stop_color;
} GradientFill;

typedef struct
{
    Image *texture;
} TextureFill;

/*
    Static:     free_Fill
    Purpose:    free Fill or Fill subclass object (except for
                TextureFill)
*/
static void free_Fill(void *fill)
{
    xfree(fill);
}

/*
    Extern:     GradientFill.new(x1, y1, x2, y2, start_color, stop_color)
    Purpose:    Create new GradientFill object
*/
#if !defined(HAVE_RB_DEFINE_ALLOC_FUNC)
VALUE
GradientFill_new(
    VALUE class,
    VALUE x1,
    VALUE y1,
    VALUE x2,
    VALUE y2,
    VALUE start_color,
    VALUE stop_color)
{
    GradientFill *fill;
    volatile VALUE new_fill;
    VALUE argv[6];

    argv[0] = x1;
    argv[1] = y1;
    argv[2] = x2;
    argv[3] = y2;
    argv[4] = start_color;
    argv[5] = stop_color;

    new_fill = Data_Make_Struct(class
                              , GradientFill
                              , NULL
                              , free_Fill
                              , fill);

    rb_obj_call_init((VALUE)new_fill, 6, argv);
    return new_fill;
}
#else
VALUE
GradientFill_alloc(VALUE class)
{
    GradientFill *fill;

    return Data_Make_Struct(class, GradientFill, NULL, free_Fill, fill);
}
#endif

/*
    Extern:     GradientFill#initialize(start_color, stop_color)
    Purpose:    store the vector points and the start and stop colors
*/
VALUE
GradientFill_initialize(
    VALUE self,
    VALUE x1,
    VALUE y1,
    VALUE x2,
    VALUE y2,
    VALUE start_color,
    VALUE stop_color)
{
    GradientFill *fill;

    Data_Get_Struct(self, GradientFill, fill);

    fill->x1 = NUM2DBL(x1);
    fill->y1 = NUM2DBL(y1);
    fill->x2 = NUM2DBL(x2);
    fill->y2 = NUM2DBL(y2);
    Color_to_PixelPacket(&fill->start_color, start_color);
    Color_to_PixelPacket(&fill->stop_color, stop_color);

    return self;
}

/*
    Static:     point_fill
    Purpose:    do a gradient that radiates from a point
*/
static void
point_fill(
    Image *image,
    double x0,
    double y0,
    PixelPacket *start_color,
    PixelPacket *stop_color)
{
    double steps, distance;
    long x, y;
    double red_step, green_step, blue_step;

    steps = sqrt((double)((image->columns-x0)*(image->columns-x0)
                + (image->rows-y0)*(image->rows-y0)));
    red_step = ((double)(stop_color->red - start_color->red)) / steps;
    green_step = ((double)(stop_color->green - start_color->green)) / steps;
    blue_step = ((double)(stop_color->blue - start_color->blue)) / steps;

    for (y = 0; y < image->rows; y++)
    {
        PixelPacket *row_pixels;

        if (!(row_pixels = SetImagePixels(image, 0, y, image->columns, 1)))
        {
            rb_raise(rb_eNoMemError, "not enough memory to continue");
        }
        for (x = 0; x < image->columns; x++)
        {
            distance = sqrt((double)((x-x0)*(x-x0)+(y-y0)*(y-y0)));
            row_pixels[x].red     = start_color->red   + (distance * red_step);
            row_pixels[x].green   = start_color->green + (distance * green_step);
            row_pixels[x].blue    = start_color->blue  + (distance * blue_step);
            row_pixels[x].opacity = OpaqueOpacity;
        }
        if (!SyncImagePixels(image))
        {
            rb_raise(Class_ImageMagickError, "can't set image pixels");
        }
    }
}

/*
    Static:     vertical_fill
    Purpose:    do a gradient fill that proceeds from a vertical line to the
                right and left sides of the image
*/
static void
vertical_fill(
    Image *image,
    double x1,
    PixelPacket *start_color,
    PixelPacket *stop_color)
{
    double steps;
    long x, y;
    volatile PixelPacket *master;
    double red_step, green_step, blue_step;

    // Keep in mind that x1 could be < 0 or > image->columns. If steps
    // is negative, swap the start and end colors and use the absolute
    // value.
    steps = fmax(x1, ((long)image->columns)-x1);
    if (steps < 0)
    {
        PixelPacket t = *start_color;
        *start_color = *stop_color;
        *stop_color = t;
        steps = -steps;
    }

    // If x is to the left of the x-axis, add that many steps so that
    // the color at the right side will be that many steps away from
    // the stop color.
    if (x1 < 0)
    {
        steps -= x1;
    }

    red_step = (stop_color->red - start_color->red) / steps;
    green_step = (stop_color->green - start_color->green) / steps;
    blue_step = (stop_color->blue - start_color->blue) / steps;

    // All the rows are the same. Make a "master row" and simply copy
    // it to each actual row.
    master = ALLOC_N(PixelPacket, image->columns);

    for (x = 0; x < image->columns; x++)
    {
        double distance   = fabs(x1 - x);
        master[x].red     = start_color->red   + (red_step * distance);
        master[x].green   = start_color->green + (green_step * distance);
        master[x].blue    = start_color->blue  + (blue_step * distance);
        master[x].opacity = OpaqueOpacity;
    }

    // Now copy the master row to each actual row.
    for (y = 0; y < image->rows; y++)
    {
        PixelPacket *row_pixels;

        if (!(row_pixels = SetImagePixels(image, 0, y, image->columns, 1)))
        {
            rb_raise(rb_eNoMemError, "not enough memory to continue");
        }

        memcpy(row_pixels, (PixelPacket *)master, image->columns * sizeof(PixelPacket));
        if (!SyncImagePixels(image))
        {
            rb_raise(Class_ImageMagickError, "can't set image pixels");
        }
    }

    xfree((PixelPacket *)master);
}

/*
    Static:     horizontal_fill
    Purpose:    do a gradient fill that starts from a horizontal line
*/
static void
horizontal_fill(
    Image *image,
    double y1,
    PixelPacket *start_color,
    PixelPacket *stop_color)
{
    double steps;
    long x, y;
    volatile PixelPacket *master;
    double red_step, green_step, blue_step;

    // Bear in mind that y1 could be < 0 or > image->rows. If steps is
    // negative, swap the start and end colors and use the absolute value.
    steps = fmax(y1, ((long)image->rows)-y1);
    if (steps < 0)
    {
        PixelPacket t = *start_color;
        *start_color = *stop_color;
        *stop_color = t;
        steps = -steps;
    }

    // If the line is below the y-axis, add that many steps so the color
    // at the bottom of the image is that many steps away from the stop color
    if (y1 < 0)
    {
        steps -= y1;
    }

    red_step = (stop_color->red - start_color->red) / steps;
    green_step = (stop_color->green - start_color->green) / steps;
    blue_step = (stop_color->blue - start_color->blue) / steps;

    // All the columns are the same, so make a master column and copy it to
    // each of the "real" columns.
    master = ALLOC_N(volatile PixelPacket, image->rows);

    for (y = 0; y < image->rows; y++)
    {
        double distance   = fabs(y1 - y);
        master[y].red     = start_color->red   + (distance * red_step);
        master[y].green   = start_color->green + (distance * green_step);
        master[y].blue    = start_color->blue  + (distance * blue_step);
        master[y].opacity = OpaqueOpacity;
    }

    for (x = 0; x < image->columns; x++)
    {
        PixelPacket *col_pixels;

        if (!(col_pixels = SetImagePixels(image, x, 0, 1, image->rows)))
        {
            rb_raise(rb_eNoMemError, "not enough memory to continue");
        }
        memcpy(col_pixels, (PixelPacket *)master, image->rows * sizeof(PixelPacket));
        if (!SyncImagePixels(image))
        {
            rb_raise(Class_ImageMagickError, "can't set image pixels");
        }
    }

    xfree((PixelPacket *)master);
}

/*
    Static:     v_diagonal_fill
    Purpose:    do a gradient fill that starts from a diagonal line and
                ends at the top and bottom of the image
*/
static void
v_diagonal_fill(
    Image *image,
    double x1,
    double y1,
    double x2,
    double y2,
    PixelPacket *start_color,
    PixelPacket *stop_color)
{
    long x, y;
    double red_step, green_step, blue_step;
    double m, b, steps = 0.0;
    double d1, d2;

    // Compute the equation of the line: y=mx+b
    m = ((double)(y2 - y1))/((double)(x2 - x1));
    b = y1 - (m * x1);

    // The number of steps is the greatest distance between the line and
    // the top or bottom of the image between x=0 and x=image->columns
    // When x=0, y=b. When x=image->columns, y = m*image->columns+b
    d1 = b;
    d2 = m * image->columns + b;

    if (d1 < 0 && d2 < 0)
    {
        steps += fmax(fabs(d1),fabs(d2));
    }
    else if (d1 > image->rows && d2 > image->rows)
    {
        steps += fmax(d1-image->rows, d2-image->rows);
    }

    d1 = fmax(b, image->rows-b);
    d2 = fmax(d2, image->rows-d2);
    steps += fmax(d1, d2);

    // If the line is entirely > image->rows, swap the start & end color
    if (steps < 0)
    {
        PixelPacket t = *stop_color;
        *stop_color = *start_color;
        *start_color = t;
        steps = -steps;
    }

    red_step = (stop_color->red - start_color->red) / steps;
    green_step = (stop_color->green - start_color->green) / steps;
    blue_step = (stop_color->blue - start_color->blue) / steps;

    for (y = 0; y < image->rows; y++)
    {
        PixelPacket *row_pixels;

        if (!(row_pixels = SetImagePixels(image, 0, y, image->columns, 1)))
        {
            rb_raise(rb_eNoMemError, "not enough memory to continue");
        }
        for (x = 0; x < image->columns; x++)
        {
            double distance = fabs(y-(m * x + b));
            row_pixels[x].red     = start_color->red   + (distance * red_step);
            row_pixels[x].green   = start_color->green + (distance * green_step);
            row_pixels[x].blue    = start_color->blue  + (distance * blue_step);
            row_pixels[x].opacity = OpaqueOpacity;
        }
        if (!SyncImagePixels(image))
        {
            rb_raise(Class_ImageMagickError, "can't set image pixels");
        }
    }
}

/*
    Static:     h_diagonal_fill
    Purpose:    do a gradient fill that starts from a diagonal line and
                ends at the sides of the image
*/
static void
h_diagonal_fill(
    Image *image,
    double x1,
    double y1,
    double x2,
    double y2,
    PixelPacket *start_color,
    PixelPacket *stop_color)
{
    long x, y;
    double m, b, steps = 0.0;
    double red_step, green_step, blue_step;
    double d1, d2;

    // Compute the equation of the line: y=mx+b
    m = ((double)(y2 - y1))/((double)(x2 - x1));
    b = y1 - (m * x1);

    // The number of steps is the greatest distance between the line and
    // the left or right side of the image between y=0 and y=image->rows.
    // When y=0, x=-b/m. When y=image->rows, x = (image->rows-b)/m.
    d1 = -b/m;
    d2 = (image->rows-b)/m;

    // If the line is entirely to the right or left of the image, increase
    // the number of steps.
    if (d1 < 0 && d2 < 0)
    {
        steps += fmax(fabs(d1),fabs(d2));
    }
    else if (d1 > image->columns && d2 > image->columns)
    {
        steps += fmax(fabs(image->columns-d1),fabs(image->columns-d2));
    }

    d1 = fmax(d1, image->columns-d1);
    d2 = fmax(d2, image->columns-d2);
    steps += fmax(d1, d2);

    // If the line is entirely > image->columns, swap the start & end color
    if (steps < 0)
    {
        PixelPacket t = *stop_color;
        *stop_color = *start_color;
        *start_color = t;
        steps = -steps;
    }

    red_step = (stop_color->red - start_color->red) / steps;
    green_step = (stop_color->green - start_color->green) / steps;
    blue_step = (stop_color->blue - start_color->blue) / steps;

    for (y = 0; y < image->rows; y++)
    {
        PixelPacket *row_pixels;

        if (!(row_pixels = SetImagePixels(image, 0, y, image->columns, 1)))
        {
            rb_raise(rb_eNoMemError, "not enough memory to continue");
        }
        for (x = 0; x < image->columns; x++)
        {
            double distance = fabs(x-((y-b)/m));
            row_pixels[x].red     = start_color->red   + (distance * red_step);
            row_pixels[x].green   = start_color->green + (distance * green_step);
            row_pixels[x].blue    = start_color->blue  + (distance * blue_step);
            row_pixels[x].opacity = OpaqueOpacity;
        }
        if (!SyncImagePixels(image))
        {
            rb_raise(Class_ImageMagickError, "can't set image pixels");
        }
    }
}

/*
    Extern:     GradientFill_fill(image_obj)
    Purpose:    the GradientFill#fill method - call GradientFill with the
                start and stop colors specified when this fill object
                was created.
*/
VALUE
GradientFill_fill(VALUE self, VALUE image_obj)
{
    GradientFill *fill;
    Image *image;
    PixelPacket start_color, stop_color;
    double x1, y1, x2, y2;          // points on the line

    Data_Get_Struct(self, GradientFill, fill);
    Data_Get_Struct(image_obj, Image, image);

    x1 = fill->x1;
    y1 = fill->y1;
    x2 = fill->x2;
    y2 = fill->y2;
    start_color = fill->start_color;
    stop_color  = fill->stop_color;

    if (fabs(x2-x1) < 0.5)       // vertical?
    {
        // If the x1,y1 and x2,y2 points are essentially the same
        if (fabs(y2-y1) < 0.5)
        {
            point_fill(image, x1, y1, &start_color, &stop_color);
        }

        // A vertical line is a special case. (Yes, really do pass x1
        // as both the 2nd and 4th arguments!)
        else
        {
            vertical_fill(image, x1, &start_color, &stop_color);
        }
    }

    // A horizontal line is a special case.
    else if (fabs(y2-y1) < 0.5)
    {
        // Pass y1 as both the 3rd and 5th arguments!
        horizontal_fill(image, y1, &start_color, &stop_color);
    }

    // This is the general case - a diagonal line. If the line is more horizontal
    // than vertical, use the top and bottom of the image as the ends of the
    // gradient, otherwise use the sides of the image.
    else
    {
        double m = ((double)(y2 - y1))/((double)(x2 - x1));
        double diagonal = ((double)image->rows)/image->columns;
        if (fabs(m) <= diagonal)
        {
            v_diagonal_fill(image, x1, y1, x2, y2, &start_color, &stop_color);
        }
        else
        {
            h_diagonal_fill(image, x1, y1, x2, y2, &start_color, &stop_color);
        }
    }

    return self;
}


/*
    Static:     free_TextureFill
    Purpose:    free the TextureFill struct and the texture image it points to
    Notes:      called from GC
*/
static void
free_TextureFill(void *fill_obj)
{
    TextureFill *fill = (TextureFill *)fill_obj;

    DestroyImage(fill->texture);
    xfree(fill);
}

/*
    Extern:     TextureFill.new(texture)
    Purpose:    Create new TextureFill object
    Notes:      the texture is an Image or Image *object
*/
#if !defined(HAVE_RB_DEFINE_ALLOC_FUNC)
VALUE
TextureFill_new(VALUE class, VALUE texture)
{
    TextureFill *fill;
    VALUE argv[1];
    volatile VALUE new_fill;

    new_fill = Data_Make_Struct(class
                              , TextureFill
                              , NULL
                              , free_TextureFill
                              , fill);
    argv[0] = texture;
    rb_obj_call_init((VALUE)new_fill, 1, argv);
    return new_fill;
}
#else
VALUE
TextureFill_alloc(VALUE class)
{
    TextureFill *fill;
    return Data_Make_Struct(class
                        , TextureFill
                        , NULL
                        , free_TextureFill
                        , fill);
}
#endif

/*
    Extern:     TextureFill#initialize
    Purpose:    Store the texture image
*/
VALUE
TextureFill_initialize(VALUE self, VALUE texture_arg)
{
    TextureFill *fill;
    Image *texture;
    volatile VALUE texture_image;

    Data_Get_Struct(self, TextureFill, fill);

    texture_image = ImageList_cur_image(texture_arg);

    // Bump the reference count on the texture image.
    Data_Get_Struct(texture_image, Image, texture);
    ReferenceImage(texture);

    fill->texture = texture;
    return self;
}

/*
    Extern:     TextureFill_fill(image_obj)
    Purpose:    the TextureFill#fill method
*/
VALUE
TextureFill_fill(VALUE self, VALUE image_obj)
{
    TextureFill *fill;
    Image *image;

    Data_Get_Struct(image_obj, Image, image);
    Data_Get_Struct(self, TextureFill, fill);

    TextureImage(image, fill->texture);
    return self;
}
