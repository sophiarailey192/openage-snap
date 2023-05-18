// Copyright 2022-2023 the openage authors. See copying.md for legal info.

#include "world_object.h"

#include <eigen3/Eigen/Dense>

#include "renderer/camera/camera.h"
#include "renderer/definitions.h"
#include "renderer/resources/animation/angle_info.h"
#include "renderer/resources/animation/animation_info.h"
#include "renderer/resources/animation/frame_info.h"
#include "renderer/resources/assets/asset_manager.h"
#include "renderer/resources/assets/texture_manager.h"
#include "renderer/resources/mesh_data.h"
#include "renderer/resources/texture_data.h"
#include "renderer/stages/world/world_render_entity.h"
#include "renderer/uniform_input.h"


namespace openage::renderer::world {

WorldObject::WorldObject(const std::shared_ptr<renderer::resources::AssetManager> &asset_manager) :
	require_renderable{true},
	changed{false},
	camera{nullptr},
	asset_manager{asset_manager},
	render_entity{nullptr},
	ref_id{0},
	position{nullptr, 0, "", nullptr, SCENE_ORIGIN},
	animation_info{nullptr, 0},
	uniforms{nullptr} {
}

void WorldObject::set_render_entity(const std::shared_ptr<WorldRenderEntity> &entity) {
	this->render_entity = entity;
	this->fetch_updates();
}

void WorldObject::set_camera(const std::shared_ptr<renderer::camera::Camera> &camera) {
	this->camera = camera;
}

void WorldObject::fetch_updates() {
	if (not this->render_entity->is_changed()) {
		// exit early because there is nothing to do
		return;
	}
	// Get data from render entity
	this->ref_id = this->render_entity->get_id();
	this->position.sync(this->render_entity->get_position());
	this->animation_info.sync(this->render_entity->get_animation_path(),
	                          std::function<std::shared_ptr<renderer::resources::Animation2dInfo>(const util::Path &)>(
								  [&](const util::Path &path) {
									  try {
										  return this->asset_manager->request_animation(path);
									  }
									  catch (const error::Error &) {
										  // TODO: We have to catch this error only because the default value in the source
										  //       curve is a plain Path() object (which does not resolve).
										  //       This should be either handled by the asset manager (return a "missing"
										  //       placeholder asset if the path is invalid) or by setting the default value in
										  //       the source curve to a better value.
										  return std::shared_ptr<renderer::resources::Animation2dInfo>{nullptr};
									  }
								  }));

	// Set self to changed so that world renderer can update the renderable
	this->changed = true;
	this->render_entity->clear_changed_flag();
}

void WorldObject::update_uniforms(const curve::time_t &time) {
	// TODO: Only update uniforms that changed since last update
	if (this->uniforms == nullptr) [[unlikely]] {
		return;
	}

	// Object world position
	auto current_pos = this->position.get(time);
	this->uniforms->update("obj_world_position", current_pos.to_world_space());

	// Direction angle the object is facing towards
	// calculated from the positional waypoints before and after the current time
	auto pos_frame = this->position.frame(time).second;
	auto pos_next_frame = this->position.next_frame(time).second;
	auto pos_delta = pos_next_frame - pos_frame;
	auto angle_degrees = pos_delta.to_angle();

	// Frame subtexture
	auto animation_info = this->animation_info.get(time);
	auto layer = animation_info->get_layer(0); // TODO: Support multiple layers
	auto angle = layer.get_direction_angle(angle_degrees);

	// Flip subtexture horizontally if angle is mirrored
	if (angle->is_mirrored()) {
		this->uniforms->update("flip_x", true);
	}
	else {
		this->uniforms->update("flip_x", false);
	}

	// Current frame index considering current time
	auto timing = layer.get_frame_timing();
	size_t frame_idx = timing->get_mod(time, this->render_entity->get_update_time());

	// Index of texture and subtexture where the frame's pixels are located
	auto frame_info = angle->get_frame(frame_idx);
	auto tex_idx = frame_info->get_texture_idx();
	auto subtex_idx = frame_info->get_subtexture_idx();

	auto tex_info = animation_info->get_texture(tex_idx);
	auto tex_manager = this->asset_manager->get_texture_manager();
	auto texture = tex_manager->request(tex_info->get_image_path().value());
	this->uniforms->update("tex", texture);

	// Subtexture coordinates.inside texture
	auto coords = tex_info->get_subtex_info(subtex_idx).get_tile_params();
	this->uniforms->update("tile_params", coords);

	// scale and keep width x height ratio of texture
	// when the viewport size changes
	auto scale = animation_info->get_scalefactor() / this->camera->get_zoom();
	auto screen_size = this->camera->get_viewport_size();
	auto subtex_size = tex_info->get_subtex_info(subtex_idx).get_size();

	// Scaling with viewport size and zoom
	auto scale_vec = Eigen::Vector2f{
		scale * (static_cast<float>(subtex_size[0]) / screen_size[0]),
		scale * (static_cast<float>(subtex_size[1]) / screen_size[1])};
	this->uniforms->update("scale", scale_vec);

	// Move subtexture in scene so that its anchor point is at the object's position
	auto anchor = tex_info->get_subtex_info(subtex_idx).get_anchor_params();
	auto anchor_offset = Eigen::Vector2f{
		scale * (static_cast<float>(anchor[0]) / screen_size[0]),
		scale * (static_cast<float>(anchor[1]) / screen_size[1])};
	this->uniforms->update("anchor_offset", anchor_offset);

	// Transformation matrices from camera
	this->uniforms->update("view", this->camera->get_view_matrix());
	this->uniforms->update("proj", this->camera->get_projection_matrix());
}

uint32_t WorldObject::get_id() {
	return this->ref_id;
}

const curve::Continuous<coord::scene3> WorldObject::get_position() {
	return this->position;
}

const renderer::resources::MeshData WorldObject::get_mesh() {
	return resources::MeshData::make_quad();
}

bool WorldObject::requires_renderable() {
	return this->require_renderable;
}

void WorldObject::clear_requires_renderable() {
	this->require_renderable = false;
}

bool WorldObject::is_changed() {
	return this->changed;
}

void WorldObject::clear_changed_flag() {
	this->changed = false;
}

void WorldObject::set_uniforms(const std::shared_ptr<renderer::UniformInput> &uniforms) {
	this->uniforms = uniforms;
}

} // namespace openage::renderer::world
