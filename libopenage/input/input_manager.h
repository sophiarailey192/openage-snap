// Copyright 2015-2023 the openage authors. See copying.md for legal info.

#pragma once

#include <memory>
#include <vector>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include "coord/pixel.h"
#include "input/action.h"

namespace openage::input {

class BindingContext;
class Controller;
class InputContext;

/**
 * The input manager tracks input signals from peripherals or the
 * GUI and turns them into input events that execute some action.
 */
class InputManager {
public:
	InputManager();

	/**
	 * returns the global keybind context.
	 * actions bound here will be retained even when override_context is called.
	 */
	const std::shared_ptr<InputContext> &get_global_context();

	/**
	 * Returns the context on top.
	 * Note there is always a top context
	 * since the global context will be
	 * considered on top when none are registered
	 */
	const std::shared_ptr<InputContext> &get_top_context();

	/**
	 * a list of all keys of the active context
	 * which are bound currently.
	 */
	std::vector<std::string> active_binds() const;

	/**
	 * Push a context on top of the stack, making it the
     * current top context.
	 *
	 * if other contexts are registered afterwards,
	 * it wanders down the stack, i.e. loses priority.
	 */
	void push_context(const std::shared_ptr<InputContext> &context);

	/**
	 * Push the context with the specified ID on top of the stack,
     * making it the current top context.
	 *
	 * if other contexts are registered afterwards,
	 * it wanders down the stack, i.e. loses priority.
	 */
	void push_context(const std::string &id);

	/**
	 * Remove the current top context from the stack.
	 */
	void pop_context();

	/**
	 * Removes any matching registered context from the stack.
	 *
	 * the removal is done by finding the given pointer
	 * in the `active_contexts` lists, then deleting it in there.
	 */
	void remove_context(const std::shared_ptr<InputContext> &context);

	/**
	 * Removes any registered context matching the specified ID from the stack.
	 *
	 * the removal is done by finding the given pointer
	 * in the `active_contexts` lists, then deleting it in there.
	 */
	void remove_context(const std::string &id);

	/**
	 * updates mouse position state and motion
	 */
	void set_mouse(int x, int y);

	/**
	 * updates mouse motion only
	 */
	void set_motion(int x, int y);

	bool process(const QKeyEvent &ev);
	bool process(const QMouseEvent &ev);
	bool process(const QWheelEvent &ev);
	// bool process(/* GUI events like button clicks, etc. */);


private:
	void process_action(const input::Event &ev,
	                    const InputAction &action,
	                    const std::shared_ptr<BindingContext> &bind_ctx = nullptr);

	/**
	 * The global context. Used as fallback.
	 */
	std::shared_ptr<InputContext> global_context;

	/**
	 * Stack of active input contexts.
	 * The most recent entry is pushed on top of the stack.
	 */
	std::vector<std::shared_ptr<InputContext>> active_contexts;

	/**
	 * Map of all available contexts, referencable by an ID.
     *
     * TODO: Move this to cvar manager?
	 */
	std::unordered_map<std::string, std::shared_ptr<InputContext>> available_contexts;

	/**
     * Interface to the engine.
     */
	std::shared_ptr<Controller> controller;

	/**
	 * mouse position in the window
	 */
	coord::input mouse_position{0, 0};

	/**
	 * mouse position relative to the last frame position.
	 */
	coord::input_delta mouse_motion{0, 0};
};

} // namespace openage::input
