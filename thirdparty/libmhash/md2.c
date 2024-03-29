/* md2.c
 *
 * The MD2 hash function, described in RFC 1319.
 *
 * This code was originally written by M?ler/Sigfridsson for
 * libnettle. It was altered by B. Poettering to fit the mhash
 * interface. The original copyright notice follows.
 */

/* nettle, low-level cryptographics library
 *
 * Copyright (C) 2003 Niels M?ler, Andreas Sigfridsson
 *  
 * The nettle library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 * 
 * The nettle library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with the nettle library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

/* This code originates from the Python Cryptography Toolkit, version 1.0.1.
   Further hacked by Andreas Sigfridsson and Niels M?ler. Original license:

   ===================================================================
   Distribute and use freely; there are no restrictions on further
   dissemination and usage except those imposed by the laws of your
   country of residence.  This software is provided "as is" without
   warranty of fitness for use or suitability for any purpose, express
   or implied. Use at your own risk or not at all.
   ===================================================================
   
   Incorporating the code into commercial products is permitted; you do
   not have to make source available or contribute your changes back
   (though that would be nice).
   
   --amk                                                    (www.amk.ca) */
   

#include "libdefs.h"

#ifdef ENABLE_MD2

#include "mhash_md2.h"

static __const mutils_word8
S[256] = {
  41, 46, 67, 201, 162, 216, 124, 1, 61, 54, 84, 161, 236, 240, 6,
  19, 98, 167, 5, 243, 192, 199, 115, 140, 152, 147, 43, 217, 188,
  76, 130, 202, 30, 155, 87, 60, 253, 212, 224, 22, 103, 66, 111, 24,
  138, 23, 229, 18, 190, 78, 196, 214, 218, 158, 222, 73, 160, 251,
  245, 142, 187, 47, 238, 122, 169, 104, 121, 145, 21, 178, 7, 63,
  148, 194, 16, 137, 11, 34, 95, 33, 128, 127, 93, 154, 90, 144, 50,
  39, 53, 62, 204, 231, 191, 247, 151, 3, 255, 25, 48, 179, 72, 165,
  181, 209, 215, 94, 146, 42, 172, 86, 170, 198, 79, 184, 56, 210,
  150, 164, 125, 182, 118, 252, 107, 226, 156, 116, 4, 241, 69, 157,
  112, 89, 100, 113, 135, 32, 134, 91, 207, 101, 230, 45, 168, 2, 27,
  96, 37, 173, 174, 176, 185, 246, 28, 70, 97, 105, 52, 64, 126, 15,
  85, 71, 163, 35, 221, 81, 175, 58, 195, 92, 249, 206, 186, 197,
  234, 38, 44, 83, 13, 110, 133, 40, 132, 9, 211, 223, 205, 244, 65,
  129, 77, 82, 106, 220, 55, 200, 108, 193, 171, 250, 36, 225, 123,
  8, 12, 189, 177, 74, 120, 136, 149, 139, 227, 99, 232, 109, 233,
  203, 213, 254, 59, 0, 29, 57, 242, 239, 183, 14, 102, 88, 208, 228,
  166, 119, 114, 248, 235, 117, 75, 10, 49, 68, 80, 180, 143, 237,
  31, 26, 219, 153, 141, 51, 159, 17, 131, 20
};

static void
md2_transform(struct md2_ctx *ctx, __const mutils_word8 *data)
{
	mutils_word8 i;
	mutils_word8 j;
	mutils_word8 t;
  
	mutils_memcpy(ctx->X + 16, data, MD2_DATA_SIZE);

	for (i = 0, t = ctx->C[15]; i<MD2_DATA_SIZE; i++)
	{
		ctx->X[2 * MD2_DATA_SIZE + i]
		  = ctx->X[i] ^ ctx->X[MD2_DATA_SIZE + i];
		t = (ctx->C[i] ^= S[data[i]^t]);
	}
	for (i = t = 0; i< MD2_DATA_SIZE + 2; t = (t + i) & 0xff, i++)
	{
		for (j = 0; j < 3 * MD2_DATA_SIZE; j++)
			t = (ctx->X[j] ^= S[t]);
	}
}

void
md2_init(struct md2_ctx *ctx)
{
	mutils_bzero(ctx, sizeof(*ctx));
}

void
md2_update(struct md2_ctx *ctx,
	   __const mutils_word8 *data,
	   mutils_word32 length)
{
	if (ctx->index != 0)
	{
		/* Try to fill partial block */
		mutils_word32 left = MD2_DATA_SIZE - ctx->index;
		if (length < left)
		{
			mutils_memcpy(ctx->buffer + ctx->index, data, length);
			ctx->index += length;
			return; /* Finished */
		}
		else
		{
			mutils_memcpy(ctx->buffer + ctx->index, data, left);
			md2_transform(ctx, ctx->buffer);
			data += left;
			length -= left;
		}
	}
	while (length >= MD2_DATA_SIZE)
	{
		md2_transform(ctx, data);
		data += MD2_DATA_SIZE;
		length -= MD2_DATA_SIZE;
	}
	if ((ctx->index = length) != 0)     /* This assignment is intended */
		/* Buffer leftovers */
		mutils_memcpy(ctx->buffer, data, length);
}

void
md2_digest(struct md2_ctx *ctx, mutils_word8 *digest)
{
	mutils_word8 left;
  
	left = MD2_DATA_SIZE - ctx->index;
	mutils_memset(ctx->buffer + ctx->index, left, left);
	md2_transform(ctx, ctx->buffer);
  
	md2_transform(ctx, ctx->C);
	mutils_memcpy(digest, ctx->X, MD2_DIGEST_SIZE);
	md2_init(ctx);
}

#endif /* ENABLE_MD2 */
