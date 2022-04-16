/*
* MPD SACD Decoder plugin
* Copyright (c) 2011-2020 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with FFmpeg; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "LogLevel.hxx"
#include "util/Domain.hxx"
#include "Log.hxx"
#include <cstdarg>
#include "log_printf.h"

static constexpr Domain libdstdec_domain("libdstdec");

void log_printf(const char* fmt, ...) {
	char msg[1024];
	std::va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg,sizeof(msg), fmt, ap);
	Log(LogLevel::ERROR, libdstdec_domain, msg);
	va_end(ap);
}
