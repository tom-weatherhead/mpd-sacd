/*
* MPD SACD Decoder plugin
* Copyright (c) 2011-2021 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>
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

#ifndef _SACD_METABASE_H_INCLUDED
#define _SACD_METABASE_H_INCLUDED

#include <string>
#include <upnp/ixml.h>

#include "config.h"
#include "tag/Tag.hxx"
#include "sacd_disc.h"

constexpr char const* MB_TAG_ROOT       = "root";
constexpr char const* MB_TAG_STORE      = "store";
constexpr char const* MB_TAG_TRACK      = "track";
constexpr char const* MB_TAG_INFO       = "info";
constexpr char const* MB_TAG_META       = "meta";
constexpr char const* MB_TAG_REPLAYGAIN = "replaygain";
constexpr char const* MB_TAG_ALBUMART   = "albumart";

constexpr char const* MB_ATT_ID      = "id";
constexpr char const* MB_ATT_NAME    = "name";
constexpr char const* MB_ATT_TYPE    = "type";
constexpr char const* MB_ATT_VALUE   = "value";
constexpr char const* MB_ATT_VALSEP  = ";";
constexpr char const* MB_ATT_VERSION = "version";

constexpr char const* METABASE_TYPE    = "SACD";
constexpr char const* METABASE_VERSION = "1.2";

class sacd_metabase_t {
	std::string    store_id;
	std::string    store_path;
	std::string    store_file;
	std::string    xmlfile;
	IXML_Document* xmldoc;
	bool           initialized;
public:
	sacd_metabase_t(sacd_disc_t* sacd_disc, const char* tags_path = nullptr, const char* tags_file = nullptr);
	~sacd_metabase_t();
	bool get_track_info(unsigned track_number, TagHandler& handler);
	bool get_albumart(TagHandler& handler);
private:
	bool init_xmldoc();
	IXML_Node* get_node(const char* tag_name, const char* att_id);
};

#endif
