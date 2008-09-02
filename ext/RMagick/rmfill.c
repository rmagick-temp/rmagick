/* $Id: rmfill.c,v 1.30 2008/09/02 23:40:19 rmagick Exp $ */
/*============================================================================\
|                Copyright (C) 2008 by Timothy P. Hunter
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
} rm_GradientFill;

typedef struct
{
    Image *texture;
} rm_TextureFill;

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
    Extern:     GradientFill.allocate(x1, y1, x2, y2, start_color, stop_color)
    Purpose:    Create new GradientFill object
*/
VALUE
GradientFill_alloc(VALUE class)
{
    rm_GradientFill *fill;

    return Data_Make_Struct(class, rm_GradientFill, NULL, free_Fill, fill);
}


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
    rm_GradientFill *fill;

    Data_Get_Struct(self, rm_GradientFill, fill);

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
    unsigned long x, y;
    MagickRealType red_step, green_step, blue_step;

    steps = sqrt((double)((image->columns-x0)*(image->columns-x0)
                          + (image->rows-y0)*(image->rows-y0)));

    red_step   = ((MagickRealType)stop_color->red   - (MagickRealType)start_color->red)   / steps;
    green_step = ((MagickRealType)stop_color->green - (MagickRealType)start_color->green) / steps;
    blue_step  = ((MagickRealType)stop_color->blue  - (MagickRealType)start_color->blue)  / steps;

    for (y = 0; y < image->rows; y++)
    {
        PixelPacket *row_pixels;

        if (!(row_pixels = SetImagePixels(image, 0, (long int)y, image->columns, 1)))
        {
            rm_check_image_exception(image, RetainOnError);
        }
        for (x = 0; x < image->columns; x++)
        {
            distance = sqrt((double)((x-x0)*(x-x0)+(y-y0)*(y-y0)));
            row_pixels[x].red     = ROUND_TO_QUANTUM(start_color->red   + (distance * red_step));
            row_pixels[x].green   = ROUND_TO_QUANTUM(start_color->green + (distance * green_step));
            row_pixels[x].blue    = ROUND_TO_QUANTUM(start_color->blue  + (distance * blue_step));
            row_pixels[x].opacity = OpaqueOpacity;
        }
        if (!SyncImagePixels(image))
        {
            rm_check_image_exception(image, RetainOnError);
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
    unsigned long x, y;
    PixelPacket *master;
    MagickRealType red_step, green_step, blue_step;

    steps = FMAX(x1, ((long)image->columns)-x1);

    // If x is to the left of the x-axis, add that many steps so that
    // the color at the right side will be that many steps away from
    // the stop color.
    if (x1 < 0)
    {
        steps -= x1;
    }

    red_step   = ((MagickRealType)stop_color->red   - (MagickRealType)start_color->red)   / steps;
    green_step = ((MagickRealType)stop_color->green - (MagickRealType)start_color->green) / steps;
    blue_step  = ((MagickRealType)stop_color->blue  - (MagickRealType)start_color->blue)  / steps;

    // All the rows are the same. Make a "master row" and simply copy
    // it to each actual row.
    master = ALLOC_N(PixelPacket, image->columns);

    for (x = 0; x < image->columns; x++)
    {
        double distance   = fabs(x1 - x);
        master[x].red     = ROUND_TO_QUANTUM(start_color->red   + (red_step * distance));
        master[x].green   = ROUND_TO_QUANTUM(start_color->green + (green_step * distance));
        master[x].blue    = ROUND_TO_QUANTUM(start_color->blue  + (blue_step * distance));
        master[x].opacity = OpaqueOpacity;
    }

    // Now copy the master row to each actual row.
    for (y = 0; y < image->rows; y++)
    {
        PixelPacket *row_pixels;

        if (!(row_pixels = SetImagePixels(image, 0, (long int)y, image->columns, 1)))
        {
            rm_check_image_exception(image, RetainOnError);
        }

        memcpy(row_pixels, master, image->columns * sizeof(PixelPacket));
        if (!SyncImagePixels(image))
        {
            rm_check_image_exception(image, RetainOnError);
        }
    }

    xfree((void *)master);
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
    unsigned long x, y;
    PixelPacket *master;
    MagickRealType red_step, green_step, blue_step;

    steps = FMAX(y1, ((long)image->rows)-y1);

    // If the line is below the y-axis, add that many steps so the color
    // at the bottom of the image is that many steps away from the stop color
    if (y1 < 0)
    {
        steps -= y1;
    }

    red_step   = ((MagickRealType)stop_color->red   - (MagickRealType)start_color->red)   / steps;
    green_step = ((MagickRealType)stop_color->green - (MagickRealType)start_color->green) / steps;
    blue_step  = ((MagickRealType)stop_color->blue  - (MagickRealType)start_color->blue)  / steps;

    // All the columns are the same, so make a master column and copy it to
    // each of the "real" columns.
    master = ALLOC_N(PixelPacket, image->rows);

    for (y = 0; y < image->rows; y++)
    {
        double distance   = fabs(y1 - y);
        master[y].red     = ROUND_TO_QUANTUM(start_color->red   + (distance * red_step));
        master[y].green   = ROUND_TO_QUANTUM(start_color->green + (distance * green_step));
        master[y].blue    = ROUND_TO_QUANTUM(start_color->blue  + (distance * blue_step));
        master[y].opacity = OpaqueOpacity;
    }

    for (x = 0; x < image->columns; x++)
    {
        PixelPacket *col_pixels;

        if (!(col_pixels = SetImagePixels(image, (long int)x, 0, 1, image->rows)))
        {
            rm_check_image_exception(image, RetainOnError);
        }
        memcpy(col_pixels, master, image->rows * sizeof(PixelPacket));
        if (!SyncImagePixels(image))
        {
            rm_check_image_exception(image, RetainOnError);
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
    unsigned long x, y;
    MagickRealType red_step, green_step, blue_step;
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
        steps += FMAX(fabs(d1),fabs(d2));
    }
    else if (d1 > (double)image->rows && d2 > (double)image->rows)
    {
        steps += FMAX(d1-image->rows, d2-image->rows);
    }

    d1 = FMAX(b, image->rows-b);
    d2 = FMAX(d2, image->rows-d2);
    steps += FMAX(d1, d2);

    // If the line is entirely > image->rows, swap the start & end color
    if (steps < 0)
    {
        PixelPacket t = *stop_color;
        *stop_color = *start_color;
        *start_color = t;
        steps = -steps;
    }

    red_step =   ((MagickRealType)stop_color->red   - (MagickRealType)start_color->red)   / steps;
    green_step = ((MagickRealType)stop_color->green - (MagickRealType)start_color->green) / steps;
    blue_step =  ((MagickRealType)stop_color->blue  - (MagickRealType)start_color->blue)  / steps;

    for (y = 0; y < image->rows; y++)
    {
        PixelPacket *row_pixels;

        if (!(row_pixels = SetImagePixels(image, 0, (long int)y, image->columns, 1)))
        {
            rm_check_image_exception(image, RetainOnError);
        }
        for (x = 0; x < image->columns; x++)
        {
            double distance = (double) abs((int)(y-(m * x + b)));
            row_pixels[x].red     = ROUND_TO_QUANTUM(start_color->red   + (distance * red_step));
            row_pixels[x].green   = ROUND_TO_QUANTUM(start_color->green + (distance * green_step));
            row_pixels[x].blue    = ROUND_TO_QUANTUM(start_color->blue  + (distance * blue_step));
            row_pixels[x].opacity = OpaqueOpacity;
        }
        if (!SyncImagePixels(image))
        {
            rm_check_image_exception(image, RetainOnError);
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
    unsigned long x, y;
    double m, b, steps = 0.0;
    MagickRealType red_step, green_step, blue_step;
    double d1, d2;

    // Compute the equation of the line: y=mx+b
    m = ((double)(y2 - y1))/((double)(x2 - x1));
    b = y1 - (m * x1);

    // The number of steps is the greatest distance between the line and
    // the left or right side of the image between y=0 and y=image->rows.
    // When y=0, x=-b/m. When y=image->rows, x = (image->rows-b)/m.
    d1 = -b/m;
    d2 = (double) ((image->rows-b) / m);

    // If the line is entirely to the right or left of the image, increase
    // the number of steps.
    if (d1 < 0 && d2 < 0)
    {
        steps += FMAX(fabs(d1),fabs(d2));
    }
    else if (d1 > (double)image->columns && d2 > (double)image->columns)
    {
        steps += FMAX(abs((int)(image->columns-d1)),abs((int)(image->columns-d2)));
    }

    d1 = FMAX(d1, image->columns-d1);
    d2 = FMAX(d2, image->columns-d2);
    steps += FMAX(d1, d2);

    // If the line is entirely > image->columns, swap the start & end color
    if (steps < 0)
    {
        PixelPacket t = *stop_color;
        *stop_color = *start_color;
        *start_color = t;
        steps = -steps;
    }

    red_step =   ((MagickRealType)stop_color->red   - (MagickRealType)start_color->red)   / steps;
    green_step = ((MagickRealType)stop_color->green - (MagickRealType)start_color->green) / steps;
    blue_step =  ((MagickRealType)stop_color->blue  - (MagickRealType)start_color->blue)  / steps;

    for (y = 0; y < image->rows; y++)
    {
        PixelPacket *row_pixels;

        if (!(row_pixels = SetImagePixels(image, 0, (long int)y, image->columns, 1)))
        {
            rm_check_image_exception(image, RetainOnError);
        }
        for (x = 0; x < image->columns; x++)
        {
            double distance = (double) abs((int)(x-((y-b)/m)));
            row_pixels[x].red     = ROUND_TO_QUANTUM(start_color->red   + (distance * red_step));
            row_pixels[x].green   = ROUND_TO_QUANTUM(start_color->green + (distance * green_step));
            row_pixels[x].blue    = ROUND_TO_QUANTUM(start_color->blue  + (distance * blue_step));
            row_pixels[x].opacity = OpaqueOpacity;
        }
        if (!SyncImagePixels(image))
        {
            rm_check_image_exception(image, RetainOnError);
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
    rm_GradientFill *fill;
    Image *image;
    PixelPacket start_color, stop_color;
    double x1, y1, x2, y2;          // points on the line

    Data_Get_Struct(self, rm_GradientFill, fill);
    image = rm_check_destroyed(image_obj);

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

        // A vertical line is a special case.
        else
        {
            vertical_fill(image, x1, &start_color, &stop_color);
        }
    }

    // A horizontal line is a special case.
    else if (fabs(y2-y1) < 0.5)
    {
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
    rm_TextureFill *fill = (rm_TextureFill *)fill_obj;

    // Do not trace destruction
    (void) DestroyImage(fill->texture);
    xfree(fill);
}

/*
    Extern:     TextureFill.allocate(texture)
    Purpose:    Create new TextureFill object
    Notes:      the texture is an Image or Image *object
*/
VALUE
TextureFill_alloc(VALUE class)
{
    rm_TextureFill *fill;
    return Data_Make_Struct(class
                            , rm_TextureFill
                            , NULL
                            , free_TextureFill
                            , fill);
}

/*
    Extern:     TextureFill#initialize
    Purpose:    Store the texture image
*/
VALUE
TextureFill_initialize(VALUE self, VALUE texture_arg)
{
    rm_TextureFill *fill;
    Image *texture;
    volatile VALUE texture_image;

    Data_Get_Struct(self, rm_TextureFill, fill);

    texture_image = rm_cur_image(texture_arg);

    // Bump the reference count on the texture image.
    texture = rm_check_destroyed(texture_image);
    (void) ReferenceImage(texture);

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
    rm_TextureFill *fill;
    Image *image;

    image = rm_check_destroyed(image_obj);
    Data_Get_Struct(self, rm_TextureFill, fill);

    (void) TextureImage(image, fill->texture);
    rm_check_image_exception(image, RetainOnError);

    return self;
}

