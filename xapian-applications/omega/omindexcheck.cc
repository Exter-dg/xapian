/** @file omindexcheck.cc
 * @brief Auxiliary program of omindextest
 */
/* Copyright (C) 2019 Bruno Baruffaldi
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
#include "omindexcheck.h"
#include "worker.h"

//#include <xapian.h>
#include <iostream>
//#include <string>
//#include <unordered_map>
#include <cstdlib>

#include "gnu_getopt.h"
#include "hashterm.h"
#include "common/stringutils.h"

using namespace std;

//typedef vector<string> testcase;

//unordered_map<string, testcase> tests;
unordered_map<string, testcase> omindexcheck::tests;

void
omindexcheck::register_tests() {
#if defined HAVE_LIBARCHIVE
    // blank file
    tests.insert({"blank.odt", {}});
    // corrupted file (ODP)
    // "no text extracted from document body, but indexing metadata anyway"
    // error on searching for any term - term not found
    // tests.insert({"corrupt_file.odp", {"ZSnatur"}});

    // ODF
    tests.insert({"test.odt", {"Zедой", "Z喬伊不分享食物"}});
    tests.insert({"libarchive_text.odt", {"Stesttitle", "Atestauthor",
                  "Zsampl", "Zhead", "Ztext", "Zhello",
		  "Zworld"}});
    tests.insert({"libarchive_text_template.ott", {"Zjane", "Zdoe",
                  "Zstructur"}});
    tests.insert({"libarchive_presentation.odp", {"Zfascin", "Zfact",
                  "Zpustak", "Zmahal", "Zmillion", "Zpeopl", "Zbirthday",
                  "501"}});
    tests.insert({"libarchive_presentation_template.otp", {"ZSalizarin",
                  "Zhead", "Zworld", "Ztext"}});
    tests.insert({"libarchive_spreadsheet.ods", {"Zhello", "Zworld",
                  "Zsampl", "2"}});
    tests.insert({"libarchive_spreadsheet_template.ots", {"Zfood", "Zpasta",
                  "Zpercentag", "40"}});
    tests.insert({"libarchive_draw.odg", {"Zparth", "Zkapadia"}});

    // Apache OpenOffice
    tests.insert({"libarchive_openoffice_calc.sxc", {"Ztoy", "Zproduct",
                  "Zcost", "Zquantiti", "Zcardboard"}});
    tests.insert({"libarchive_openoffice_calc_template.stc", {"ZSpurchas",
                  "ZStemplat", "Zproduct", "Zquantiti", "Zsampl"}});
    tests.insert({"libarchive_openoffice_text.sxw", {"Zhello", "Zsampl",
                  "Zopenoffic", "Zwriter"}});
    tests.insert({"libarchive_openoffice_text_template.stw", {"Zhello",
                  "Zworld", "Zsampl", "Zhead", "ZStemplat", "ZStext"}});
    tests.insert({"libarchive_openoffice_presentation.sxi", {"Zhead",
                  "Zhello", "Zopenoffic", "Zimpress"}});
    tests.insert({"libarchive_openoffice_presentation_template.sti",
                 {"ZSproject", "ZSresearch", "Zhead", "Ztext"}});
#endif
}

bool
omindexcheck::compare_test(testcase& test, const Xapian::Document& doc, const string& file) {
    sort(test.begin(), test.end());
    Xapian::TermIterator term_iterator = doc.termlist_begin();
    for (auto& t : test) {
	term_iterator.skip_to(t);
	if (term_iterator == doc.termlist_end() || *term_iterator != t) {
	    cerr << "Error in " << file << ": Term " << t <<
		 " does not belong to this file" << endl;
	    return false;
	}
    }
    return true;
}

bool
omindexcheck::check_terms(const Xapian::Document& doc, const string& file) {
    Xapian::TermIterator term_iterator = doc.termlist_begin();
    // no data
    if (file == "blank.odt"){
        for (; term_iterator != doc.termlist_end() ; term_iterator++){
            if ((*term_iterator).find("Z") == 0 &&
                ((*term_iterator)[1]>='a' && (*term_iterator)[1]<='z')){
                return false;
            }
        }
    }
    // no author/title
    else if (file == "libarchive_text_template.ott"){
	    for (; term_iterator != doc.termlist_end() ; term_iterator++){
		if ((*term_iterator).find("A") != string::npos ||
                    (*term_iterator).find("S") != string::npos ) {
			return false;
	        }
            }
	}

    return true;

}

int
main(int argc, char** argv) {
    Xapian::Database db;
    bool succeed = true;
    if (argc <= 2)
	return 1;
    db.add_database(Xapian::Database(argv[2]));
    string url, current_dir(argv[1]);

    omindexcheck::register_tests();
    for (auto t = db.allterms_begin("U"); t != db.allterms_end("U"); ++t) {
	const string& term = *t;
	Xapian::PostingIterator p = db.postlist_begin(term);
	if (p == db.postlist_end(term)) {
	    continue;
	}
	Xapian::docid did = *p;
	Xapian::Document doc = db.get_document(did);
	auto data = doc.get_data();
	size_t start;
	if (startswith(data, "url=")) {
	    start = CONST_STRLEN("url=");
	} else {
	    start = data.find("\nurl=");
	    if (start == string::npos)
		continue;
	    start += CONST_STRLEN("\nurl=");
	}
	url.assign(data, start, data.find('\n', start) - start);
	start = url.find(current_dir) + current_dir.length();
	url = url.substr(start, url.length());
	auto it = omindexcheck::tests.find(url);
	if (it != omindexcheck::tests.end())
	    succeed &= omindexcheck::compare_test(it->second, doc, url);
        succeed &= omindexcheck::check_terms(doc, url);
    }
    return succeed ? 0 : 1;
}
