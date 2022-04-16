/*
 * Copyright (C) 2003-2022 The Music Player Daemon Project
 * http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"
#include <sacd_media.h>
#include <sacd_reader.h>
#include <sacd_dsdiff.h>
#include <dst_decoder_mt.h>
#undef MAX_CHANNELS
#include "DffDecoderPlugin.hxx"
#include "../DecoderAPI.hxx"
#include "input/InputStream.hxx"
#include "pcm/CheckAudioFormat.hxx"
#include "tag/Handler.hxx"
#include "tag/Builder.hxx"
#include "song/DetachedSong.hxx"
#include "fs/Path.hxx"
#include "fs/AllocatedPath.hxx"
#include "fs/FileSystem.hxx"
#include "thread/Cond.hxx"
#include "thread/Mutex.hxx"
#include "util/BitReverse.hxx"
#include "util/StringFormat.hxx"
#include "util/StringView.hxx"
#include "util/UriExtract.hxx"
#include "util/Domain.hxx"
#include "Log.hxx"

#include <assert.h>
#include <stdio.h>
#include <memory>
#include <vector>

static constexpr Domain dsdiff_domain("dsdiff");

namespace dsdiff {

constexpr const char* DSDIFF_TRACKXXX_FMT{ "%cC_AUDIO__TRACK%03u.%3s" };

unsigned  param_dstdec_threads;
bool      param_edited_master;
bool      param_single_track;
bool      param_lsbitfirst;
area_id_e param_playable_area;
bool      param_use_stdio;

AllocatedPath                  dsdiff_path{ nullptr };
std::unique_ptr<sacd_media_t>  sacd_media;
std::unique_ptr<sacd_reader_t> sacd_reader;

static unsigned
get_subsong(Path path_fs) {
	auto ptr = path_fs.GetBase().c_str();
	char area = '\0';
	unsigned index = 0;
	char suffix[4];
	auto params = sscanf(ptr, DSDIFF_TRACKXXX_FMT, &area, &index, suffix);
	if (area == 'M') {
		index += sacd_reader->get_tracks(AREA_TWOCH);
	}
	index--;
	return (params == 3) ? index : 0;
}

static bool
container_update(Path path_fs) {
	auto curr_path = AllocatedPath(path_fs);
	if (!FileExists(curr_path)) {
		curr_path = path_fs.GetDirectoryName();
	}
	if (dsdiff_path == curr_path) {
		return true;
	}
	if (sacd_reader) {
		sacd_reader->close();
		sacd_reader.reset();
	}
	if (sacd_media) {
		sacd_media->close();
		sacd_media.reset();
	}
	dsdiff_path.SetNull();
	if (FileExists(curr_path)) {
		if (param_use_stdio) {
			sacd_media = std::make_unique<sacd_media_file_t>();
		}
		else {
			sacd_media = std::make_unique<sacd_media_stream_t>();
		}
		if (!sacd_media) {
			LogError(dsdiff_domain, "new sacd_media_t() failed");
			return false;
		}
		sacd_reader = std::make_unique<sacd_dsdiff_t>();
		if (!sacd_reader) {
			LogError(dsdiff_domain, "new sacd_dsdiff_t() failed");
			return false;
		}
		if (!sacd_media->open(curr_path.c_str())) {
			std::string err;
			err  = "sacd_media->open('";
			err += curr_path.c_str();
			err += "') failed";
			LogWarning(dsdiff_domain, err.c_str());
			return false;
		}
		if (!sacd_reader->open(sacd_media.get(), param_single_track ? MODE_SINGLE_TRACK : MODE_MULTI_TRACK)) {
			//LogWarning(dsdiff_domain, "sacd_reader->open(...) failed");
			return false;
		}
		dsdiff_path = curr_path;
	}
	return static_cast<bool>(sacd_reader);
}

static void
scan_info(unsigned track, TagHandler& handler) {
	std::string tag_value = std::to_string(track + 1);
	handler.OnTag(TAG_TRACK, tag_value.c_str());
	handler.OnDuration(SongTime::FromS(sacd_reader->get_duration(track)));
	sacd_reader->get_info(track, handler);
	auto track_format = sacd_reader->is_dst() ? "DST" : "DSD";
	handler.OnPair("codec", track_format);
}

static bool
init(const ConfigBlock& block) {
	param_dstdec_threads = block.GetBlockValue("dstdec_threads", thread::hardware_concurrency());
	param_edited_master  = block.GetBlockValue("edited_master", false);
	param_single_track   = block.GetBlockValue("single_track", false);
	param_lsbitfirst     = block.GetBlockValue("lsbitfirst", false);
	auto playable_area = block.GetBlockValue("playable_area", nullptr);
	param_playable_area = AREA_BOTH;
	if (playable_area != nullptr) {
		if (strcmp(playable_area, "stereo") == 0) {
			param_playable_area = AREA_TWOCH;
		}
		if (strcmp(playable_area, "multichannel") == 0) {
			param_playable_area = AREA_MULCH;
		}
	}
	param_use_stdio = block.GetBlockValue("use_stdio", true);
	return true;
}

static void
finish() noexcept {
	container_update(nullptr);
}

static std::forward_list<DetachedSong>
container_scan(Path path_fs) {
	std::forward_list<DetachedSong> list;
	if (path_fs.IsNull()) {
		if (!param_single_track) {
			list.emplace_after(list.before_begin(), "multitrack_dsdiff");
		}
		return list;
	}
	if (!container_update(path_fs)) {
		return list;
	}
	TagBuilder tag_builder;
	auto tail = list.before_begin();
	auto suffix = path_fs.GetSuffix();
	auto twoch_count = sacd_reader->get_tracks(AREA_TWOCH);
	auto mulch_count = sacd_reader->get_tracks(AREA_MULCH);
	if (twoch_count + mulch_count < 2) {
		return list;
	}
	if (twoch_count > 0 && param_playable_area != AREA_MULCH) {
		sacd_reader->select_area(AREA_TWOCH);
		for (auto track = 0u; track < twoch_count; track++) {
			AddTagHandler handler(tag_builder);
			scan_info(track, handler);
			tail = list.emplace_after(
				tail,
				StringFormat<64>(DSDIFF_TRACKXXX_FMT, '2', track + 1, suffix),
				tag_builder.Commit()
			);
		}
	}
	if (mulch_count > 0 && param_playable_area != AREA_TWOCH) {
		sacd_reader->select_area(AREA_MULCH);
		for (auto track = 0u; track < mulch_count; track++) {
			AddTagHandler h(tag_builder);
			scan_info(track, h);
			tail = list.emplace_after(
				tail,
				StringFormat<64>(DSDIFF_TRACKXXX_FMT, 'M', track + twoch_count + 1, suffix),
				tag_builder.Commit()
			);
		}
	}
	return list;
}

static void
bit_reverse_buffer(uint8_t* p, uint8_t* end) {
	for (; p < end; ++p) {
		*p = bit_reverse(*p);
	}
}

static void
file_decode(DecoderClient &client, Path path_fs) {
	if (!container_update(FileExists(path_fs) ? AllocatedPath(path_fs) : path_fs.GetDirectoryName())) {
		return;
	}
	auto twoch_count = sacd_reader->get_tracks(AREA_TWOCH);
	auto mulch_count = sacd_reader->get_tracks(AREA_MULCH);
	auto track = (twoch_count + mulch_count > 1) ? get_subsong(path_fs) : 0;

	// initialize reader
	sacd_reader->set_emaster(param_edited_master);
	if (track < twoch_count) {
		if (!sacd_reader->select_track(track, AREA_TWOCH, 0)) {
			LogError(dsdiff_domain, "cannot select track in stereo area");
			return;
		}
	}
	else {
		track -= twoch_count;
		if (track < sacd_reader->get_tracks(AREA_MULCH)) {
			if (!sacd_reader->select_track(track, AREA_MULCH, 0)) {
				LogError(dsdiff_domain, "cannot select track in multichannel area");
				return;
			}
		}
	}
	auto dsd_channels = sacd_reader->get_channels();
	auto dsd_samplerate = sacd_reader->get_samplerate();
	auto dsd_framerate = sacd_reader->get_framerate();
	auto dsd_buf_size = dsd_samplerate / 8 / dsd_framerate * dsd_channels;
	auto dst_buf_size = dsd_samplerate / 8 / dsd_framerate * dsd_channels;
	std::vector<uint8_t> dsd_buf;
	std::vector<uint8_t> dst_buf;
	dsd_buf.resize(param_dstdec_threads * dsd_buf_size);
	dst_buf.resize(param_dstdec_threads * dst_buf_size);

	// initialize decoder
	auto audio_format = CheckAudioFormat(dsd_samplerate / 8, SampleFormat::DSD, dsd_channels);
	auto songtime = SongTime::FromS(sacd_reader->get_duration(track));
	client.Ready(audio_format, true, songtime);

	// play
	uint8_t* dsd_data;
	uint8_t* dst_data;
	size_t dsd_size = 0;
	size_t dst_size = 0;
	dst_decoder_t* dst_decoder = nullptr;
	auto cmd = client.GetCommand();
	for (;;) {
		auto slot_nr = dst_decoder ? dst_decoder->get_slot_nr() : 0u;
		dsd_data = dsd_buf.data() + dsd_buf_size * slot_nr;
		dst_data = dst_buf.data() + dst_buf_size * slot_nr;
		dst_size = dst_buf_size;
		frame_type_e frame_type;
		if (sacd_reader->read_frame(dst_data, &dst_size, &frame_type)) {
			if (dst_size > 0) {
				if (frame_type == FRAME_INVALID) {
					dst_size = dst_buf_size;
					memset(dst_data, 0xAA, dst_size);
				}
				if (frame_type == FRAME_DST) {
					if (!dst_decoder) {
						dst_decoder = new dst_decoder_t(param_dstdec_threads);
						if (!dst_decoder) {
							LogError(dsdiff_domain, "new dst_decoder_t() failed");
							break;
						}
						if (dst_decoder->init(dsd_channels, dsd_samplerate, dsd_framerate) != 0) {
							LogError(dsdiff_domain, "dst_decoder_t.init() failed");
							break;
						}
					}
					dst_decoder->decode(dst_data, dst_size, &dsd_data, &dsd_size);
				}
				else {
					dsd_data = dst_data;
					dsd_size = dst_size;
				}
				if (dsd_size > 0) {
					if (param_lsbitfirst) {
						bit_reverse_buffer(dsd_data, dsd_data + dsd_size);
					}
					cmd = client.SubmitData(nullptr, dsd_data, dsd_size, 8 * dst_size / 1000);
				}
			}
		}
		else {
			for (;;) {
				dst_data = nullptr;
				dst_size = 0;
				dsd_data = nullptr;
				dsd_size = 0;
				if (dst_decoder) {
					dst_decoder->decode(dst_data, dst_size, &dsd_data, &dsd_size);
				}
				if (dsd_size > 0) {
					if (param_lsbitfirst) {
						bit_reverse_buffer(dsd_data, dsd_data + dsd_size);
					}
					cmd = client.SubmitData(nullptr, dsd_data, dsd_size, 0);
					if (cmd == DecoderCommand::STOP || cmd == DecoderCommand::SEEK) {
						break;
					}
				}
				else {
					break;
				}
			}
			break;
		}
		if (cmd == DecoderCommand::STOP) {
			break;
		}
		if (cmd == DecoderCommand::SEEK) {
			auto seconds = client.GetSeekTime().ToDoubleS();
			if (sacd_reader->seek(seconds)) {
				client.CommandFinished();
			}
			else {
				client.SeekError();
			}
			cmd = client.GetCommand();
		}
	}
	if (dst_decoder) {
		delete dst_decoder;
		dst_decoder = nullptr;
	}
}

static bool
scan_file(Path path_fs, TagHandler& handler) noexcept {
	if (!container_update(FileExists(path_fs) ? AllocatedPath(path_fs) : path_fs.GetDirectoryName())) {
		return false;
	}
	auto twoch_count = sacd_reader->get_tracks(AREA_TWOCH);
	auto mulch_count = sacd_reader->get_tracks(AREA_MULCH);
	auto track = (twoch_count + mulch_count > 1) ? get_subsong(path_fs) : 0;
	if (track < twoch_count) {
		sacd_reader->select_area(AREA_TWOCH);
	}
	else {
		track -= twoch_count;
		if (track < mulch_count) {
			sacd_reader->select_area(AREA_MULCH);
		}
		else {
			LogError(dsdiff_domain, "subsong index is out of range");
			return false;
		}
	}
	scan_info(track, handler);
	return true;
}

static const char* const suffixes[] {
	"dff",
	nullptr
};

static const char* const mime_types[] {
	"application/x-dff",
	"audio/x-dff",
	"audio/x-dsd",
	nullptr
};

}

constexpr DecoderPlugin dff_decoder_plugin =
	DecoderPlugin("dsdiff", dsdiff::file_decode, dsdiff::scan_file)
	.WithInit(dsdiff::init, dsdiff::finish)
	.WithContainer(dsdiff::container_scan)
	.WithSuffixes(dsdiff::suffixes)
	.WithMimeTypes(dsdiff::mime_types);
