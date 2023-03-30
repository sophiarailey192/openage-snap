// Copyright 2023-2023 the openage authors. See copying.md for legal info.

#pragma once

#include <unordered_map>

#include "event/state.h"
#include "gamestate/types.h"

namespace openage::gamestate {

class GameEntity;

/**
 * State of the game.
 *
 * Contains index structures for looking up game entities and other
 * information for the current game.
 */
class GameState : public event::State {
public:
	/**
     * Create a new game state.
     *
     * @param event_loop Event loop for the game state.
     */
	explicit GameState(const std::shared_ptr<event::Loop> &event_loop);

	/**
     * Get a unique ID for creating a game entity.
     *
     * @return Unique ID for a game entity.
     */
	entity_id_t get_next_id();

	/**
     * Add a new game entity to the index.
     *
     * @param entity New game entity.
     */
	void add_game_entity(const std::shared_ptr<GameEntity> &entity);

	/**
     * Get a game entity by its ID.
     *
     * @param id ID of the game entity.
     * @return Game entity with the given ID.
     */
	const std::shared_ptr<GameEntity> &get_game_entity(entity_id_t id) const;

	/**
     * Get all game entities in the current game.
     *
     * @return Map of all game entities in the current game by their ID.
     */
	const std::unordered_map<entity_id_t, std::shared_ptr<GameEntity>> &get_game_entities() const;

private:
	/**
     * ID of the next game entity to be created.
     */
	entity_id_t next_id;

	/**
     * Map of all game entities in the current game by their ID.
     */
	std::unordered_map<entity_id_t, std::shared_ptr<GameEntity>> game_entities;
};
} // namespace openage::gamestate
