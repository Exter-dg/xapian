/* ommakerangenode.cc: Node which makes a numeric range
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "config.h"
#include "om/omindexernode.h"
#include "node_reg.h"

class OmMakeRangeNode : public OmIndexerNode {
    public:
	OmMakeRangeNode(const OmSettings &config)
		: OmIndexerNode(config),
		  first(config.get_int("first", 1)),
		  step(config.get_int("step", 1)),
		  count(config.get_int("count", 1))
	{
	}
    private:
	int first;
	int step;
	int count;
	// FIXME: implement config_modified()
	void calculate() {
	    OmIndexerMessage ints(new OmIndexerData(std::vector<OmIndexerData>()));
	    for (int i=0; i<count; ++i) {
		ints->append_element(OmIndexerData(first + i*step));
	    }
	    set_output("out", ints);
	}
};

NODE_BEGIN(OmMakeRangeNode, ommakerange)
NODE_OUTPUT("out", "ints", mt_vector)
NODE_END()
