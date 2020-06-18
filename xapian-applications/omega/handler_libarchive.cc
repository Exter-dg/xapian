/** @file handler_libarchive.cc
 * @brief Extract text and metadata using libarchive.
 */
/* Copyright (C) 2020 Parth Kapadia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
#include <config.h>
#include "handler.h"

#include "metaxmlparse.h"
#include "opendocparse.h"
#include "str.h"

#include <archive.h>
#include <archive_entry.h>

#include <cstring>

using namespace std;

bool
extract(const string& filename,
	string& dump,
	string& title,
	string& keywords,
	string& author,
	string& pages,
	string& error)
{
    try {
	struct archive* archive_obj;
	struct archive_entry* entry;
	int status_code;
	const char* file = filename.c_str();
	archive_obj = archive_read_new();
	archive_read_support_filter_all(archive_obj);
	archive_read_support_format_all(archive_obj);
	status_code = archive_read_open_filename(archive_obj, file, 10240);

	if (status_code != ARCHIVE_OK) {
	    error = "Libarchive failed to open the file " + filename;
		return false;
	}

	size_t total;
	ssize_t size;
	string s_content,s_meta,s_styles;

	// extracting data from content.xml and styles.xml
	while (archive_read_next_header(archive_obj, &entry) == ARCHIVE_OK) {
	    if (strcmp(archive_entry_pathname(entry), "content.xml") == 0) {
		total = archive_entry_size(entry);
		char* buf1 = new char[total];
		size = archive_read_data(archive_obj, buf1, total);

		if (size <= 0) {
		    error = "Libarchive was not able to extract data from "
		    "content.xml";
		    return false;
		}

		s_content += str(buf1);
		delete []buf1;
		} else if (strcmp(
			archive_entry_pathname(entry), "styles.xml") == 0) {
		total = archive_entry_size(entry);
		char* buf2 = new char[total];
		size = archive_read_data(archive_obj, buf2, total);

		if (size <= 0) {
		    error = "Libarchive was not able to extract data from "
		    "styles.xml";
		    return false;
		}

		s_styles += str(buf2);
		delete []buf2;
		} else if (strcmp(
			archive_entry_pathname(entry), "meta.xml") == 0) {
		total = archive_entry_size(entry);
		char* buf3 = new char[total];
		size = archive_read_data(archive_obj, buf3, total);

		if (size > 0) {
		    // index the file even if this fails
		    // hence doesn't return false for size<=0
		    s_meta = str(buf3);
		    MetaXmlParser metaxmlparser;
		    metaxmlparser.parse(s_meta);
		    title = metaxmlparser.title;
		    keywords = metaxmlparser.keywords;
		    author = metaxmlparser.author;
		}
		delete []buf3;
		}
	}
	s_content += s_styles;
	OpenDocParser parser;
	parser.parse(s_content);
	dump = parser.dump;
	status_code = archive_read_free(archive_obj);
	if (status_code != ARCHIVE_OK) {
	    //return false;
	}

	} catch (...) {
	    error = "Libarchive threw an exception";
	    return false;
	}
	(void)pages;
	return true;
}
