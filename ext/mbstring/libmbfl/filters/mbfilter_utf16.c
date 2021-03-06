/*
 * "streamable kanji code filter and converter"
 * Copyright (c) 1998-2002 HappySize, Inc. All rights reserved.
 *
 * LICENSE NOTICES
 *
 * This file is part of "streamable kanji code filter and converter",
 * which is distributed under the terms of GNU Lesser General Public
 * License (version 2) as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with "streamable kanji code filter and converter";
 * if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA
 *
 * The author of this file:
 *
 */
/*
 * The source code included in this files was separated from mbfilter.c
 * by moriyoshi koizumi <moriyoshi@php.net> on 4 dec 2002.
 *
 */

#include "mbfilter.h"
#include "mbfilter_utf16.h"

static int mbfl_filt_ident_utf16(int c, mbfl_identify_filter *filter);
static int mbfl_filt_ident_utf16le(int c, mbfl_identify_filter *filter);
static int mbfl_filt_ident_utf16be(int c, mbfl_identify_filter *filter);

static const char *mbfl_encoding_utf16_aliases[] = {"utf16", NULL};

const mbfl_encoding mbfl_encoding_utf16 = {
	mbfl_no_encoding_utf16,
	"UTF-16",
	"UTF-16",
	mbfl_encoding_utf16_aliases,
	NULL,
	MBFL_ENCTYPE_MWC2BE,
	&vtbl_utf16_wchar,
	&vtbl_wchar_utf16
};

const mbfl_encoding mbfl_encoding_utf16be = {
	mbfl_no_encoding_utf16be,
	"UTF-16BE",
	"UTF-16BE",
	NULL,
	NULL,
	MBFL_ENCTYPE_MWC2BE,
	&vtbl_utf16be_wchar,
	&vtbl_wchar_utf16be
};

const mbfl_encoding mbfl_encoding_utf16le = {
	mbfl_no_encoding_utf16le,
	"UTF-16LE",
	"UTF-16LE",
	NULL,
	NULL,
	MBFL_ENCTYPE_MWC2LE,
	&vtbl_utf16le_wchar,
	&vtbl_wchar_utf16le
};

const struct mbfl_identify_vtbl vtbl_identify_utf16 = {
	mbfl_no_encoding_utf16,
	mbfl_filt_ident_common_ctor,
	mbfl_filt_ident_utf16
};

const struct mbfl_identify_vtbl vtbl_identify_utf16le = {
	mbfl_no_encoding_utf16le,
	mbfl_filt_ident_common_ctor,
	mbfl_filt_ident_utf16le
};

const struct mbfl_identify_vtbl vtbl_identify_utf16be = {
	mbfl_no_encoding_utf16be,
	mbfl_filt_ident_common_ctor,
	mbfl_filt_ident_utf16be
};

const struct mbfl_convert_vtbl vtbl_utf16_wchar = {
	mbfl_no_encoding_utf16,
	mbfl_no_encoding_wchar,
	mbfl_filt_conv_common_ctor,
	NULL,
	mbfl_filt_conv_utf16_wchar,
	mbfl_filt_conv_common_flush,
	NULL,
};

const struct mbfl_convert_vtbl vtbl_wchar_utf16 = {
	mbfl_no_encoding_wchar,
	mbfl_no_encoding_utf16,
	mbfl_filt_conv_common_ctor,
	NULL,
	mbfl_filt_conv_wchar_utf16be,
	mbfl_filt_conv_common_flush,
	NULL,
};

const struct mbfl_convert_vtbl vtbl_utf16be_wchar = {
	mbfl_no_encoding_utf16be,
	mbfl_no_encoding_wchar,
	mbfl_filt_conv_common_ctor,
	NULL,
	mbfl_filt_conv_utf16be_wchar,
	mbfl_filt_conv_common_flush,
	NULL,
};

const struct mbfl_convert_vtbl vtbl_wchar_utf16be = {
	mbfl_no_encoding_wchar,
	mbfl_no_encoding_utf16be,
	mbfl_filt_conv_common_ctor,
	NULL,
	mbfl_filt_conv_wchar_utf16be,
	mbfl_filt_conv_common_flush,
	NULL,
};

const struct mbfl_convert_vtbl vtbl_utf16le_wchar = {
	mbfl_no_encoding_utf16le,
	mbfl_no_encoding_wchar,
	mbfl_filt_conv_common_ctor,
	NULL,
	mbfl_filt_conv_utf16le_wchar,
	mbfl_filt_conv_common_flush,
	NULL,
};

const struct mbfl_convert_vtbl vtbl_wchar_utf16le = {
	mbfl_no_encoding_wchar,
	mbfl_no_encoding_utf16le,
	mbfl_filt_conv_common_ctor,
	NULL,
	mbfl_filt_conv_wchar_utf16le,
	mbfl_filt_conv_common_flush,
	NULL,
};

#define CK(statement)	do { if ((statement) < 0) return (-1); } while (0)

/*
 * UTF-16 => wchar
 */
int mbfl_filt_conv_utf16_wchar(int c, mbfl_convert_filter *filter)
{
	int n, endian;

	endian = filter->status & 0xff00;
	switch (filter->status & 0x0f) {
	case 0:
		if (endian) {
			n = c & 0xff;
		} else {
			n = (c & 0xff) << 8;
		}
		filter->cache |= n;
		filter->status++;
		break;
	default:
		if (endian) {
			n = (c & 0xff) << 8;
		} else {
			n = c & 0xff;
		}
		n |= filter->cache & 0xffff;
		filter->status &= ~0x0f;
		if (n >= 0xd800 && n < 0xdc00) {
			filter->cache = ((n & 0x3ff) << 16) + 0x400000;
		} else if (n >= 0xdc00 && n < 0xe000) {
			n &= 0x3ff;
			n |= (filter->cache & 0xfff0000) >> 6;
			filter->cache = 0;
			if (n >= MBFL_WCSPLANE_SUPMIN && n < MBFL_WCSPLANE_SUPMAX) {
				CK((*filter->output_function)(n, filter->data));
			} else {		/* illegal character */
				n &= MBFL_WCSGROUP_MASK;
				n |= MBFL_WCSGROUP_THROUGH;
				CK((*filter->output_function)(n, filter->data));
			}
		} else {
			int is_first = filter->status & 0x10;
			filter->cache = 0;
			filter->status |= 0x10;
			if (!is_first) {
				if (n == 0xfffe) {
					if (endian) {
						filter->status &= ~0x100;		/* big-endian */
					} else {
						filter->status |= 0x100;		/* little-endian */
					}
					break;
				} else if (n == 0xfeff) {
					break;
				}
			}
			CK((*filter->output_function)(n, filter->data));
		}
		break;
	}

	return c;
}

/*
 * UTF-16BE => wchar
 */
int mbfl_filt_conv_utf16be_wchar(int c, mbfl_convert_filter *filter)
{
	int n;

	switch (filter->status) {
	case 0:
		filter->status = 1;
		n = (c & 0xff) << 8;
		filter->cache |= n;
		break;
	default:
		filter->status = 0;
		n = (filter->cache & 0xff00) | (c & 0xff);
		if (n >= 0xd800 && n < 0xdc00) {
			filter->cache = ((n & 0x3ff) << 16) + 0x400000;
		} else if (n >= 0xdc00 && n < 0xe000) {
			n &= 0x3ff;
			n |= (filter->cache & 0xfff0000) >> 6;
			filter->cache = 0;
			if (n >= MBFL_WCSPLANE_SUPMIN && n < MBFL_WCSPLANE_SUPMAX) {
				CK((*filter->output_function)(n, filter->data));
			} else {		/* illegal character */
				n &= MBFL_WCSGROUP_MASK;
				n |= MBFL_WCSGROUP_THROUGH;
				CK((*filter->output_function)(n, filter->data));
			}
		} else {
			filter->cache = 0;
			CK((*filter->output_function)(n, filter->data));
		}
		break;
	}

	return c;
}

/*
 * wchar => UTF-16BE
 */
int mbfl_filt_conv_wchar_utf16be(int c, mbfl_convert_filter *filter)
{
	int n;

	if (c >= 0 && c < MBFL_WCSPLANE_UCS2MAX) {
		CK((*filter->output_function)((c >> 8) & 0xff, filter->data));
		CK((*filter->output_function)(c & 0xff, filter->data));
	} else if (c >= MBFL_WCSPLANE_SUPMIN && c < MBFL_WCSPLANE_SUPMAX) {
		n = ((c >> 10) - 0x40) | 0xd800;
		CK((*filter->output_function)((n >> 8) & 0xff, filter->data));
		CK((*filter->output_function)(n & 0xff, filter->data));
		n = (c & 0x3ff) | 0xdc00;
		CK((*filter->output_function)((n >> 8) & 0xff, filter->data));
		CK((*filter->output_function)(n & 0xff, filter->data));
	} else {
		CK(mbfl_filt_conv_illegal_output(c, filter));
	}

	return c;
}

/*
 * UTF-16LE => wchar
 */
int mbfl_filt_conv_utf16le_wchar(int c, mbfl_convert_filter *filter)
{
	switch (filter->status) {
	case 0:
		filter->cache = c & 0xff;
		filter->status = 1;
		break;

	case 1:
		if ((c & 0xfc) == 0xd8) {
			/* Looks like we have a surrogate pair here */
			filter->cache += ((c & 0x3) << 8);
			filter->status = 2;
		} else if ((c & 0xfc) == 0xdc) {
			/* This is wrong; the second part of the surrogate pair has come first
			 * Flag it with `MBFL_WCSGROUP_THROUGH`; the following filter will handle
			 * the error */
			int n = (filter->cache + ((c & 0xff) << 8)) | MBFL_WCSGROUP_THROUGH;
			filter->status = 0;
			CK((*filter->output_function)(n, filter->data));
		} else {
			filter->status = 0;
			CK((*filter->output_function)(filter->cache + ((c & 0xff) << 8), filter->data));
		}
		break;

	case 2:
		filter->cache = (filter->cache << 10) + (c & 0xff);
		filter->status = 3;
		break;

	case 3:
		filter->status = 0;
		int n = filter->cache + ((c & 0x3) << 8) + 0x10000;
		CK((*filter->output_function)(n, filter->data));
		break;
	}

	return c;
}

/*
 * wchar => UTF-16LE
 */
int mbfl_filt_conv_wchar_utf16le(int c, mbfl_convert_filter *filter)
{
	int n;

	if (c >= 0 && c < MBFL_WCSPLANE_UCS2MAX) {
		CK((*filter->output_function)(c & 0xff, filter->data));
		CK((*filter->output_function)((c >> 8) & 0xff, filter->data));
	} else if (c >= MBFL_WCSPLANE_SUPMIN && c < MBFL_WCSPLANE_SUPMAX) {
		n = ((c >> 10) - 0x40) | 0xd800;
		CK((*filter->output_function)(n & 0xff, filter->data));
		CK((*filter->output_function)((n >> 8) & 0xff, filter->data));
		n = (c & 0x3ff) | 0xdc00;
		CK((*filter->output_function)(n & 0xff, filter->data));
		CK((*filter->output_function)((n >> 8) & 0xff, filter->data));
	} else {
		CK(mbfl_filt_conv_illegal_output(c, filter));
	}

	return c;
}

static int mbfl_filt_ident_utf16(int c, mbfl_identify_filter *filter)
{
	if (filter->status == 0) {
		if (c >= 0xfe) { /* could be a byte-order mark */
			filter->status = c;
		} else {
			/* no byte-order mark at beginning of input; assume UTF-16BE */
			filter->filter_function = mbfl_filt_ident_utf16be;
			return (filter->filter_function)(c, filter);
		}
	} else {
		unsigned short n = (filter->status << 8) | c;
		filter->status = 0;

		if (n == 0xfeff) {
			/* it was a big-endian byte-order mark */
			filter->filter_function = mbfl_filt_ident_utf16be;
		} else if (n == 0xfffe) {
			/* it was a little-endian byte-order mark */
			filter->filter_function = mbfl_filt_ident_utf16le;
		} else {
			/* it wasn't a byte-order mark */
			filter->filter_function = mbfl_filt_ident_utf16be;
			(filter->filter_function)(n >> 8, filter);
			return (filter->filter_function)(c, filter);
		}
	}
	return c;
}

static int mbfl_filt_ident_utf16le(int c, mbfl_identify_filter *filter)
{
	switch (filter->status) {
	case 0: /* 1st byte */
		filter->status = 1;
		break;

	case 1: /* 2nd byte */
		if ((c & 0xfc) == 0xd8) {
			/* Looks like a surrogate pair */
			filter->status = 2;
		} else if ((c & 0xfc) == 0xdc) {
			/* This is wrong; the second part of the surrogate pair has come first */
			filter->flag = 1;
		} else {
			filter->status = 0; /* Just an ordinary 2-byte character */
		}
		break;

	case 2: /* 3rd byte */
		filter->status = 3;
		break;

	case 3: /* 4th byte */
		if ((c & 0xfc) == 0xdc) {
			filter->status = 0;
		} else {
			filter->flag = 1; /* Surrogate pair wrongly encoded */
		}
		break;
	}

	return c;
}

static int mbfl_filt_ident_utf16be(int c, mbfl_identify_filter *filter)
{
	switch (filter->status) {
	case 0: /* 1st byte */
		if ((c & 0xfc) == 0xd8) {
			/* Looks like a surrogate pair */
			filter->status = 2;
		} else if ((c & 0xfc) == 0xdc) {
			/* This is wrong; the second part of the surrogate pair has come first */
			filter->flag = 1;
		} else {
			/* Just an ordinary 2-byte character */
			filter->status = 1;
		}
		break;

	case 1: /* 2nd byte, not surrogate pair */
		filter->status = 0;
		break;

	case 2: /* 2nd byte, surrogate pair */
		filter->status = 3;
		break;

	case 3: /* 3rd byte, surrogate pair */
		if ((c & 0xfc) == 0xdc) {
			filter->status = 4;
		} else {
			filter->flag = 1; /* Surrogate pair wrongly encoded */
		}
		break;

	case 4: /* 4th byte, surrogate pair */
		filter->status = 0;
		break;
	}

	return c;
}
