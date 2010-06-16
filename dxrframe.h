#pragma once

void gdi_frame();
void _render_black(unsigned char *dst, unsigned pitch);

void rend_frame_x1_16s(unsigned char *dst, unsigned pitch);
void rend_frame_x2_8s(unsigned char *dst, unsigned pitch);
void rend_frame_x2_8d(unsigned char *dst, unsigned pitch);
void rend_frame_x2_16(unsigned char *dst, unsigned pitch);
void rend_frame_x2_16s(unsigned char *dst, unsigned pitch);
void rend_frame_x2_16d(unsigned char *dst, unsigned pitch);
void rend_frame_x2_32(unsigned char *dst, unsigned pitch);
void rend_frame_x2_32s(unsigned char *dst, unsigned pitch);
void rend_frame_x2_32d(unsigned char *dst, unsigned pitch);
