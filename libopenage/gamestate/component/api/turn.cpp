// Copyright 2021-2021 the openage authors. See copying.md for legal info.

#include "turn.h"
#include "../component_type.h"


namespace openage::gamestate::component {

component_t Turn::get_component_type() const {
	return component_t::TURN;
}

} // namespace openage::gamestate::component