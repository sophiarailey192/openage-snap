// Copyright 2021-2021 the openage authors. See copying.md for legal info.

#pragma once

#include "base_component.h"

namespace openage::gamestate::component {

/**
 * Interface for components that track ingame information about
 * a game entity, but don't use information from the nyan API.
 */
class InternalComponent : Component {};

} // namespace openage::gamestate::component