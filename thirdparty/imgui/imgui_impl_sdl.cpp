// ImGui SDL2 + software rasterizer
// In this binding, ImTextureID is used to store a texture identifier represented as an SDL surface. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_opengl.h>
#include <imgui.h>
#include "imgui_impl_sdl.h"
#include <math.h>

// Uncomment desired texture mode
// Note: TEXTURE_MODE_NO_CHECK check will not wrap nor clamp but can crash the renderer for UVs outside [0, 1]
#define TEXTURE_MODE_NO_CHECK
//#define TEXTURE_MODE_REPEAT
//#define TEXTURE_MODE_CLAMP

struct render_data
{
    SDL_Surface* screen;
    SDL_Surface* texture;
    ImVec4 clip_rect;
};

struct color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct pixel
{
    int x;
    int y;
    color c;
    float u;
    float v;
};

struct line
{
    float x1;
    float x2;
    float y;
    color c1;
    color c2;
    float u1;
    float u2;
    float v;
};

struct triangle
{
    float x1;
    float y1;
    float x2;
    float y2;
    float x3;
    float y3;
    color c1;
    color c2;
    color c3;
    float u1;
    float v1;
    float u2;
    float v2;
    float u3;
    float v3;
};

struct rectangle
{
    float x1;
    float y1;
    float x2;
    float y2;
    color c;
    float u1;
    float v1;
    float u2;
    float v2;
};

// Data
static double       g_Time = 0.0f;
static bool         g_MousePressed[3] = { false, false, false };
static float        g_MouseWheel = 0.0f;
static SDL_Surface* g_FontTexture = 0;

static void draw_pixel(render_data* render_data, pixel* pixel)
{
    // Discard pixel if it is outside the frame buffer or clip area
    if ((pixel->x < 0) ||
        (pixel->x >= render_data->screen->w) ||
        (pixel->x < render_data->clip_rect.x) ||
        (pixel->x >= render_data->clip_rect.z))
        return;

    // Compute pixel offset
    uint32_t* p = (uint32_t*)render_data->screen->pixels + pixel->y * render_data->screen->w + pixel->x;

    // Get destination pixel
    uint32_t dst = *p;
    uint8_t dst_r = (dst >> 16) & 0xFF;
    uint8_t dst_g = (dst >> 8) & 0xFF;
    uint8_t dst_b = (dst >> 0) & 0xFF;

    // Get source pixel
    uint8_t src_r = pixel->c.r;
    uint8_t src_g = pixel->c.g;
    uint8_t src_b = pixel->c.b;
    uint8_t src_a = pixel->c.a;

    // Update source pixel if texture is set
    if (render_data->texture)
    {
#if defined(TEXTURE_MODE_REPEAT)
        int u = (int)((pixel->u * render_data->texture->w) + 0.5f) % render_data->texture->w;
        int v = (int)((pixel->v * render_data->texture->h) + 0.5f) % render_data->texture->h;
#elif defined(TEXTURE_MODE_CLAMP)
        int u = (int)((pixel->u * render_data->texture->w) + 0.5f);
        int v = (int)((pixel->v * render_data->texture->h) + 0.5f);
        if (u < 0)
            u = 0;
        else if (u >= render_data->texture->w)
            u = render_data->texture->w - 1;
        if (v < 0)
            v = 0;
        else if (v >= render_data->texture->h)
            v = render_data->texture->h - 1;
#elif defined(TEXTURE_MODE_NO_CHECK)
        int u = (int)((pixel->u * render_data->texture->w) + 0.5f);
        int v = (int)((pixel->v * render_data->texture->h) + 0.5f);
#endif
        uint32_t* pixels = (uint32_t*)render_data->texture->pixels;
        uint32_t texel = pixels[u + v * render_data->texture->w];
        uint8_t t_a = (texel >> 24) & 0xFF;
        uint8_t t_r = (texel >> 16) & 0xFF;
        uint8_t t_g = (texel >> 8) & 0xFF;
        uint8_t t_b = texel & 0xFF;
        src_r = src_r * t_r / 255;
        src_g = src_g * t_g / 255;
        src_b = src_b * t_b / 255;
        src_a = src_a * t_a / 255;
    }

    // Compute final pixel based on alpha-blending formula
    uint8_t r = (src_r * src_a + dst_r * (255 - src_a)) / 255;
    uint8_t g = (src_g * src_a + dst_g * (255 - src_a)) / 255;
    uint8_t b = (src_b * src_a + dst_b * (255 - src_a)) / 255;
    uint32_t col = (r << 16) | (g << 8) | b;

    // Set pixel in frame buffer
    *p = col;
}

static void draw_line(render_data* render_data, line* line)
{
    pixel pixel;
    color c;
    float u;
    float f;
    float cur_x;

    // Round X values
    line->x1 = roundf(line->x1);
    line->x2 = roundf(line->x2);

    // Discard line if it is outside the frame buffer
    if ((line->x2 < 0) ||
        (line->y < 0) ||
        (line->x1 >= render_data->screen->w) ||
        (line->y >= render_data->screen->h))
        return;

    // Discard line if it is outside of the clip area
    if ((line->x2 < render_data->clip_rect.x) ||
        (line->y < render_data->clip_rect.y) ||
        (line->x1 >= render_data->clip_rect.z) ||
        (line->y >= render_data->clip_rect.w))
        return;

    // Draw line
    for (cur_x = line->x1; cur_x < line->x2; cur_x++)
    {
        // Get interpolation factor
        f = fmaxf((cur_x - line->x1) / (line->x2 - line->x1), 0.0f);

        // Interpolate color
        c.r = line->c1.r + (uint8_t)(f * (line->c2.r - line->c1.r));
        c.g = line->c1.g + (uint8_t)(f * (line->c2.g - line->c1.g));
        c.b = line->c1.b + (uint8_t)(f * (line->c2.b - line->c1.b));
        c.a = line->c1.a + (uint8_t)(f * (line->c2.a - line->c1.a));

        // Interpolate texture U coordinate
        u = line->u1 + f * (line->u2 - line->u1);

        // Build and draw pixel
        pixel.x = (int)cur_x;
        pixel.y = (int)line->y;
        pixel.c = c;
        pixel.u = u;
        pixel.v = line->v;
        draw_pixel(render_data, &pixel);
    }
}

static void draw_triangle_flat_bottom(render_data* render_data, triangle* triangle)
{
    float x;
    float y;
    color c;
    float u;
    float v;
    line l;
    float t1;
    float t2;
    float t3;
    float t4;
    float f;
    double x1;
    double x2;
    double x1_inc;
    double x2_inc;
    float cur_y;

    // Swap second and third vertices if needed
    if (triangle->x3 < triangle->x2)
    {
        x = triangle->x2;
        y = triangle->y2;
        c = triangle->c2;
        u = triangle->u2;
        v = triangle->v2;
        triangle->x2 = triangle->x3;
        triangle->y2 = triangle->y3;
        triangle->c2 = triangle->c3;
        triangle->u2 = triangle->u3;
        triangle->v2 = triangle->v3;
        triangle->x3 = x;
        triangle->y3 = y;
        triangle->c3 = c;
        triangle->u3 = u;
        triangle->v3 = v;
    }

    // Calculate line X increments
    x1_inc = triangle->x2 - triangle->x1;
    x1_inc /= triangle->y2 - triangle->y1;
    x2_inc = triangle->x3 - triangle->x1;
    x2_inc /= triangle->y3 - triangle->y1;

    // Initialize X coordinates to first vertex X coordinate
    x1 = triangle->x1;
    x2 = triangle->x1;

    // Draw triangle
    for (cur_y = triangle->y1; cur_y < triangle->y2; cur_y++)
    {
        // Set line coordinates
        l.x1 = (float)x1;
        l.x2 = (float)x2;
        l.y = cur_y;

        // Compute first line vertex interpolation factor
        t1 = triangle->x1 - (float)x1;
        t2 = triangle->y1 - cur_y;
        t3 = triangle->x2 - triangle->x1;
        t4 = triangle->y2 - triangle->y1;
        f = sqrtf((t1 * t1) + (t2 * t2));
        if ((t3 != 0.0f) || (t4 != 0.0f))
            f /= sqrtf((t3 * t3) + (t4 * t4));

        // Compute first vertex color
        l.c1.r = triangle->c1.r + (uint8_t)(f * (triangle->c2.r - triangle->c1.r));
        l.c1.g = triangle->c1.g + (uint8_t)(f * (triangle->c2.g - triangle->c1.g));
        l.c1.b = triangle->c1.b + (uint8_t)(f * (triangle->c2.b - triangle->c1.b));
        l.c1.a = triangle->c1.a + (uint8_t)(f * (triangle->c2.a - triangle->c1.a));

        // Compute first vertex texture U coordinate
        l.u1 = triangle->u1 + f * (triangle->u2 - triangle->u1);

        // Compute second line vertex interpolation factor
        t1 = triangle->x1 - (float)x2;
        t2 = triangle->y1 - cur_y;
        t3 = triangle->x3 - triangle->x1;
        t4 = triangle->y3 - triangle->y1;
        f = sqrtf((t1 * t1) + (t2 * t2));
        if ((t3 != 0.0f) || (t4 != 0.0f))
            f /= sqrtf((t3 * t3) + (t4 * t4));

        // Compute second vertex color
        l.c2.r = triangle->c1.r + (uint8_t)(f * (triangle->c3.r - triangle->c1.r));
        l.c2.g = triangle->c1.g + (uint8_t)(f * (triangle->c3.g - triangle->c1.g));
        l.c2.b = triangle->c1.b + (uint8_t)(f * (triangle->c3.b - triangle->c1.b));
        l.c2.a = triangle->c1.a + (uint8_t)(f * (triangle->c3.a - triangle->c1.a));

        // Compute second vertex texture U coordinate
        l.u2 = triangle->u1 + f * (triangle->u3 - triangle->u1);

        // Compute line texture V coordinate
        f = cur_y - triangle->y1;
        if (triangle->y1 != triangle->y2)
            f /= triangle->y2 - triangle->y1;
        l.v = triangle->v1 + f * (triangle->v2 - triangle->v1);

        // Update X coordinates
        x1 += x1_inc;
        x2 += x2_inc;

        // Draw single line
        draw_line(render_data, &l);
    }
}

static void draw_triangle_flat_top(render_data* render_data, triangle* triangle)
{
    float x;
    float y;
    color c;
    float u;
    float v;
    line l;
    float t1;
    float t2;
    float t3;
    float t4;
    float f;
    double x1;
    double x2;
    double x1_inc;
    double x2_inc;
    float cur_y;

    // Swap first and second vertices if needed
    if (triangle->x2 < triangle->x1)
    {
        x = triangle->x1;
        y = triangle->y1;
        c = triangle->c1;
        u = triangle->u1;
        v = triangle->v1;
        triangle->x1 = triangle->x2;
        triangle->y1 = triangle->y2;
        triangle->c1 = triangle->c2;
        triangle->u1 = triangle->u2;
        triangle->v1 = triangle->v2;
        triangle->x2 = x;
        triangle->y2 = y;
        triangle->c2 = c;
        triangle->u2 = u;
        triangle->v2 = v;
    }

    // Calculate line X increments
    x1_inc = triangle->x1 - triangle->x3;
    x1_inc /= triangle->y1 - triangle->y3;
    x2_inc = triangle->x2 - triangle->x3;
    x2_inc /= triangle->y2 - triangle->y3;

    // Initialize X coordinates to third vertex X coordinate
    x1 = triangle->x3;
    x2 = triangle->x3;

    // Draw triangle
    for (cur_y = triangle->y3; cur_y >= triangle->y1; cur_y--)
    {
        // Set line coordinates
        l.x1 = (float)x1;
        l.x2 = (float)x2;
        l.y = cur_y;

        // Compute first line interpolation factor
        t1 = triangle->x3 - (float)x1;
        t2 = triangle->y3 - cur_y;
        t3 = triangle->x3 - triangle->x1;
        t4 = triangle->y3 - triangle->y1;
        f = sqrtf((t1 * t1) + (t2 * t2));
        if ((t3 != 0.0f) || (t4 != 0.0f))
            f /= sqrtf((t3 * t3) + (t4 * t4));

        // Compute first vertex color
        l.c1.r = triangle->c3.r + (uint8_t)(f * (triangle->c1.r - triangle->c3.r));
        l.c1.g = triangle->c3.g + (uint8_t)(f * (triangle->c1.g - triangle->c3.g));
        l.c1.b = triangle->c3.b + (uint8_t)(f * (triangle->c1.b - triangle->c3.b));
        l.c1.a = triangle->c3.a + (uint8_t)(f * (triangle->c1.a - triangle->c3.a));

        // Compute second vertex texture U coordinate
        l.u1 = triangle->u3 + f * (triangle->u1 - triangle->u3);

        // Compute second line interpolation factor
        t1 = triangle->x3 - (float)x2;
        t2 = triangle->y3 - cur_y;
        t3 = triangle->x3 - triangle->x2;
        t4 = triangle->y3 - triangle->y2;
        f = sqrtf((t1 * t1) + (t2 * t2));
        if ((t3 != 0.0f) || (t4 != 0.0f))
            f /= sqrtf((t3 * t3) + (t4 * t4));

        // Compute second vertex color
        l.c2.r = triangle->c3.r + (uint8_t)(f * (triangle->c2.r - triangle->c3.r));
        l.c2.g = triangle->c3.g + (uint8_t)(f * (triangle->c2.g - triangle->c3.g));
        l.c2.b = triangle->c3.b + (uint8_t)(f * (triangle->c2.b - triangle->c3.b));
        l.c2.a = triangle->c3.a + (uint8_t)(f * (triangle->c2.a - triangle->c3.a));

        // Compute second vertex texture U coordinate
        l.u2 = triangle->u3 + f * (triangle->u2 - triangle->u3);

        // Compute line texture V coordinate
        f = cur_y - triangle->y1;
        if (triangle->y1 != triangle->y3)
            f /= triangle->y3 - triangle->y1;
        l.v = triangle->v1 + f * (triangle->v3 - triangle->v1);

        // Update X coordinates
        x1 -= x1_inc;
        x2 -= x2_inc;

        // Draw single line
        draw_line(render_data, &l);
    }
}

static void draw_triangle(render_data* render_data, triangle* tri)
{
    float x;
    float y;
    color c;
    float u;
    float v;
    triangle flat;
    float f;

    // Round Y values
    tri->y1 = roundf(tri->y1);
    tri->y2 = roundf(tri->y2);
    tri->y3 = roundf(tri->y3);

    // Swap first and second vertices if needed
    if (tri->y2 < tri->y1)
    {
        x = tri->x1;
        y = tri->y1;
        c = tri->c1;
        u = tri->u1;
        v = tri->v1;
        tri->x1 =tri->x2;
        tri->y1 =tri->y2;
        tri->c1 =tri->c2;
        tri->u1 =tri->u2;
        tri->v1 =tri->v2;
        tri->x2 = x;
        tri->y2 = y;
        tri->c2 = c;
        tri->u2 = u;
        tri->v2 = v;
    }

    // Swap first and third vertices if needed
    if (tri->y3 < tri->y1)
    {
        x = tri->x1;
        y = tri->y1;
        c = tri->c1;
        u = tri->u1;
        v = tri->v1;
        tri->x1 = tri->x3;
        tri->y1 = tri->y3;
        tri->c1 = tri->c3;
        tri->u1 = tri->u3;
        tri->v1 = tri->v3;
        tri->x3 = x;
        tri->y3 = y;
        tri->c3 = c;
        tri->u3 = u;
        tri->v3 = v;
    }

    // Swap second and third vertices if needed
    if (tri->y3 < tri->y2)
    {
        x = tri->x2;
        y = tri->y2;
        c = tri->c2;
        u = tri->u2;
        v = tri->v2;
        tri->x2 = tri->x3;
        tri->y2 = tri->y3;
        tri->c2 = tri->c3;
        tri->u2 = tri->u3;
        tri->v2 = tri->v3;
        tri->x3 = x;
        tri->y3 = y;
        tri->c3 = c;
        tri->u3 = u;
        tri->v3 = v;
    }

    // Draw flat bottom triangle and return if possible
    if (tri->y2 == tri->y3)
    {
        draw_triangle_flat_bottom(render_data, tri);
        return;
    }

    // Draw flat top triangle and return if possible
    if (tri->y1 == tri->y2)
    {
        draw_triangle_flat_top(render_data, tri);
        return;
    }

    // Compute new vertex
    f = tri->y2 - tri->y1;
    if (tri->y1 != tri->y3)
        f /= tri->y3 - tri->y1;
    x = tri->x1 + f * (tri->x3 - tri->x1);
    y = tri->y2;
    c.r = tri->c1.r + (uint8_t)(f * (tri->c3.r - tri->c1.r));
    c.g = tri->c1.g + (uint8_t)(f * (tri->c3.g - tri->c1.g));
    c.b = tri->c1.b + (uint8_t)(f * (tri->c3.b - tri->c1.b));
    c.a = tri->c1.a + (uint8_t)(f * (tri->c3.a - tri->c1.a));
    u = tri->u1 + f * (tri->u3 - tri->u1);
    v = tri->v2;

    // Create flat bottom triangle and draw it
    flat.x1 = tri->x1;
    flat.y1 = tri->y1;
    flat.c1 = tri->c1;
    flat.u1 = tri->u1;
    flat.v1 = tri->v1;
    flat.x2 = tri->x2;
    flat.y2 = tri->y2;
    flat.c2 = tri->c2;
    flat.u2 = tri->u2;
    flat.v2 = tri->v2;
    flat.x3 = x;
    flat.y3 = y;
    flat.c3 = c;
    flat.u3 = u;
    flat.v3 = v;
    draw_triangle_flat_bottom(render_data, &flat);

    // Create flat top triangle and draw it
    flat.x1 = tri->x2;
    flat.y1 = tri->y2;
    flat.c1 = tri->c2;
    flat.u1 = tri->u2;
    flat.v1 = tri->v2;
    flat.x2 = x;
    flat.y2 = y;
    flat.c2 = c;
    flat.u2 = u;
    flat.v2 = v;
    flat.x3 = tri->x3;
    flat.y3 = tri->y3;
    flat.c3 = tri->c3;
    flat.u3 = tri->u3;
    flat.v3 = tri->v3;
    draw_triangle_flat_top(render_data, &flat);
}

static void draw_rectangle(render_data* render_data, rectangle* rectangle)
{
    line line;
    float y;
    float f;

    // Discard rectangle if it is outside the frame buffer
    if ((rectangle->x2 < 0) ||
        (rectangle->y2 < 0) ||
        (rectangle->x1 >= render_data->screen->w) ||
        (rectangle->y2 >= render_data->screen->h))
        return;

    // Discard rectangle if it is outside of the clip area
    if ((rectangle->x2 < render_data->clip_rect.x) ||
        (rectangle->y2 < render_data->clip_rect.y) ||
        (rectangle->x1 >= render_data->clip_rect.z) ||
        (rectangle->y1 >= render_data->clip_rect.w))
        return;

    // Draw rectangle
    for (y = rectangle->y1; y < rectangle->y2; y++)
    {
        // Fill general line data
        line.x1 = rectangle->x1;
        line.x2 = rectangle->x2;
        line.y = y;
        line.c1 = rectangle->c;
        line.c2 = rectangle->c;
        line.u1 = rectangle->u1;
        line.u2 = rectangle->u2;

        // Compute interpolation factor
        f = (y - rectangle->y1) / (rectangle->y2 - rectangle->y1);

        // Compute line texture V coordinate
        line.v = rectangle->v1 + f * (rectangle->v2 - rectangle->v1);

        // Draw single line
        draw_line(render_data, &line);
    }
}

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_ImplSdl_RenderDrawLists(ImDrawData* draw_data)
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
        const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Set render data
                render_data render_data;
                render_data.screen = (SDL_Surface*)io.UserData;
                render_data.texture = (SDL_Surface*)pcmd->TextureId;
                render_data.clip_rect = pcmd->ClipRect;

                // Draw triangles
                for (unsigned int index = 0; index < pcmd->ElemCount; index += 3)
                {
                    const ImDrawVert* vertices[] =
                    {
                        &vtx_buffer[idx_buffer[index]],
                        &vtx_buffer[idx_buffer[index + 1]],
                        &vtx_buffer[idx_buffer[index + 2]]
                    };

                    // Check if rectangle rendering is possible
                    if (index < pcmd->ElemCount - 3)
                    {
                        // Get top left/bottom right coordinates from current triangle
                        ImVec2 tl_pos = vertices[0]->pos;
                        ImVec2 br_pos = vertices[0]->pos;
                        ImVec2 tl_uv = vertices[0]->uv;
                        ImVec2 br_uv = vertices[0]->uv;
                        for (int v = 1; v < 3; v++)
                        {
                            if (vertices[v]->pos.x < tl_pos.x)
                            {
                                tl_pos.x = vertices[v]->pos.x;
                                tl_uv.x = vertices[v]->uv.x;
                            } else if (vertices[v]->pos.x > br_pos.x)
                            {
                                br_pos.x = vertices[v]->pos.x;
                                br_uv.x = vertices[v]->uv.x;
                            }
                            if (vertices[v]->pos.y < tl_pos.y)
                            {
                                tl_pos.y = vertices[v]->pos.y;
                                tl_uv.y = vertices[v]->uv.y;
                            }
                            else if (vertices[v]->pos.y > br_pos.y)
                            {
                                br_pos.y = vertices[v]->pos.y;
                                br_uv.y = vertices[v]->uv.y;
                            }
                        }

                        // Get next set of vertices
                        const ImDrawVert* next_vertices[] =
                        {
                            &vtx_buffer[idx_buffer[index + 3]],
                            &vtx_buffer[idx_buffer[index + 4]],
                            &vtx_buffer[idx_buffer[index + 5]]
                        };

                        // Check if triangle pair is a rectangle
                        bool is_rect = true;
                        for (int v = 0; v < 3; v++)
                            if (((next_vertices[v]->pos.x != tl_pos.x) && (next_vertices[v]->pos.x != br_pos.x)) ||
                                ((next_vertices[v]->pos.y != tl_pos.y) && (next_vertices[v]->pos.y != br_pos.y)) ||
                                ((next_vertices[v]->uv.x != tl_uv.x) && (next_vertices[v]->uv.x != br_uv.x)) ||
                                ((next_vertices[v]->uv.y != tl_uv.y) && (next_vertices[v]->uv.y != br_uv.y)))
                            {
                                is_rect = false;
                                break;
                            }

                        // Handle rectangle fast path
                        if (is_rect)
                        {
                            // Build rectangle
                            rectangle rectangle;
                            rectangle.x1 = tl_pos.x;
                            rectangle.y1 = tl_pos.y;
                            rectangle.x2 = br_pos.x;
                            rectangle.y2 = br_pos.y;
                            rectangle.u1 = tl_uv.x;
                            rectangle.v1 = tl_uv.y;
                            rectangle.u2 = br_uv.x;
                            rectangle.v2 = br_uv.y;
                            rectangle.c.r = (vertices[0]->col >> IM_COL32_R_SHIFT) & 0xFF;
                            rectangle.c.g = (vertices[0]->col >> IM_COL32_G_SHIFT) & 0xFF;
                            rectangle.c.b = (vertices[0]->col >> IM_COL32_B_SHIFT) & 0xFF;
                            rectangle.c.a = (vertices[0]->col >> IM_COL32_A_SHIFT) & 0xFF;
                            draw_rectangle(&render_data, &rectangle);

                            // Increment command index and skip triangle rendering
                            index += 3;
                            continue;
                        }
                    }

                    // Build and draw single triangle
                    triangle triangle;
                    triangle.x1 = vertices[0]->pos.x;
                    triangle.y1 = vertices[0]->pos.y;
                    triangle.u1 = vertices[0]->uv.x;
                    triangle.v1 = vertices[0]->uv.y;
                    triangle.c1.r = (vertices[0]->col >> IM_COL32_R_SHIFT) & 0xFF;
                    triangle.c1.g = (vertices[0]->col >> IM_COL32_G_SHIFT) & 0xFF;
                    triangle.c1.b = (vertices[0]->col >> IM_COL32_B_SHIFT) & 0xFF;
                    triangle.c1.a = (vertices[0]->col >> IM_COL32_A_SHIFT) & 0xFF;
                    triangle.x2 = vertices[1]->pos.x;
                    triangle.y2 = vertices[1]->pos.y;
                    triangle.u2 = vertices[1]->uv.x;
                    triangle.v2 = vertices[1]->uv.y;
                    triangle.c2.r = (vertices[1]->col >> IM_COL32_R_SHIFT) & 0xFF;
                    triangle.c2.g = (vertices[1]->col >> IM_COL32_G_SHIFT) & 0xFF;
                    triangle.c2.b = (vertices[1]->col >> IM_COL32_B_SHIFT) & 0xFF;
                    triangle.c2.a = (vertices[1]->col >> IM_COL32_A_SHIFT) & 0xFF;
                    triangle.x3 = vertices[2]->pos.x;
                    triangle.y3 = vertices[2]->pos.y;
                    triangle.u3 = vertices[2]->uv.x;
                    triangle.v3 = vertices[2]->uv.y;
                    triangle.c3.r = (vertices[2]->col >> IM_COL32_R_SHIFT) & 0xFF;
                    triangle.c3.g = (vertices[2]->col >> IM_COL32_G_SHIFT) & 0xFF;
                    triangle.c3.b = (vertices[2]->col >> IM_COL32_B_SHIFT) & 0xFF;
                    triangle.c3.a = (vertices[2]->col >> IM_COL32_A_SHIFT) & 0xFF;
                    draw_triangle(&render_data, &triangle);
                }
            }
            idx_buffer += pcmd->ElemCount;
        }
    }
}

static const char* ImGui_ImplSdl_GetClipboardText(void*)
{
    return SDL_GetClipboardText();
}

static void ImGui_ImplSdl_SetClipboardText(void*, const char* text)
{
    SDL_SetClipboardText(text);
}

bool ImGui_ImplSdl_ProcessEvent(SDL_Event* event)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (event->type)
    {
    case SDL_MOUSEWHEEL:
        {
            if (event->wheel.y > 0)
                g_MouseWheel = 1;
            if (event->wheel.y < 0)
                g_MouseWheel = -1;
            return true;
        }
    case SDL_MOUSEBUTTONDOWN:
        {
            if (event->button.button == SDL_BUTTON_LEFT) g_MousePressed[0] = true;
            if (event->button.button == SDL_BUTTON_RIGHT) g_MousePressed[1] = true;
            if (event->button.button == SDL_BUTTON_MIDDLE) g_MousePressed[2] = true;
            return true;
        }
    case SDL_TEXTINPUT:
        {
            io.AddInputCharactersUTF8(event->text.text);
            return true;
        }
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        {
            int key = event->key.keysym.sym & ~SDLK_SCANCODE_MASK;
            io.KeysDown[key] = (event->type == SDL_KEYDOWN);
            io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
            io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
            io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
            io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
            return true;
        }
    }
    return false;
}

bool ImGui_ImplSdl_CreateDeviceObjects()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // Upload texture to graphics system
    const int depth = 4;
    g_FontTexture = SDL_CreateRGBSurfaceFrom(pixels,
        width,
        height,
        depth,
        width,
        (Uint32)(0xFF << IM_COL32_R_SHIFT),
        (Uint32)(0xFF << IM_COL32_G_SHIFT),
        (Uint32)(0xFF << IM_COL32_B_SHIFT),
        (Uint32)(0xFF << IM_COL32_A_SHIFT));

    // Store our identifier
    io.Fonts->TexID = (void*)(intptr_t)g_FontTexture;

    return true;
}

void    ImGui_ImplSdl_InvalidateDeviceObjects()
{
    if (g_FontTexture)
    {
        SDL_FreeSurface(g_FontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        g_FontTexture = 0;
    }
}

bool    ImGui_ImplSdl_Init(SDL_Window* window, SDL_Surface* screen)
{
	ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;                     // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDLK_a;
    io.KeyMap[ImGuiKey_C] = SDLK_c;
    io.KeyMap[ImGuiKey_V] = SDLK_v;
    io.KeyMap[ImGuiKey_X] = SDLK_x;
    io.KeyMap[ImGuiKey_Y] = SDLK_y;
    io.KeyMap[ImGuiKey_Z] = SDLK_z;

    io.RenderDrawListsFn = ImGui_ImplSdl_RenderDrawLists;   // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
    io.UserData = screen;
    io.SetClipboardTextFn = ImGui_ImplSdl_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplSdl_GetClipboardText;
    io.ClipboardUserData = NULL;

#ifdef _WIN32
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    io.ImeWindowHandle = wmInfo.info.win.window;
#else
    (void)window;
#endif

    return true;
}

void ImGui_ImplSdl_Shutdown()
{
    ImGui_ImplSdl_InvalidateDeviceObjects();
    //ImGui::Shutdown();
}

void ImGui_ImplSdl_NewFrame(SDL_Window* window)
{
    if (!g_FontTexture)
        ImGui_ImplSdl_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    // Note: frame buffer scaling is not supported in this example
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // Setup time step
    Uint32 time = SDL_GetTicks();
    double current_time = time / 1000.0;
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f/60.0f);
    g_Time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from SDL_PollEvent())
    int mx, my;
    Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)
        io.MousePos = ImVec2((float)mx, (float)my);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
    else
        io.MousePos = ImVec2(-1,-1);

    io.MouseDown[0] = g_MousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;        // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
    io.MouseDown[1] = g_MousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    io.MouseDown[2] = g_MousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    g_MousePressed[0] = g_MousePressed[1] = g_MousePressed[2] = false;

    io.MouseWheel = g_MouseWheel;
    g_MouseWheel = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    SDL_ShowCursor(io.MouseDrawCursor ? 0 : 1);

    // Start the frame
    ImGui::NewFrame();
}
