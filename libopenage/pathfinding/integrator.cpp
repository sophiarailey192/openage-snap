// Copyright 2024-2024 the openage authors. See copying.md for legal info.

#include "integrator.h"

#include "pathfinding/cost_field.h"
#include "pathfinding/flow_field.h"
#include "pathfinding/integration_field.h"


namespace openage::path {

void Integrator::set_cost_field(const std::shared_ptr<CostField> &cost_field) {
	this->cost_field = cost_field;
}

std::shared_ptr<FlowField> Integrator::build(const coord::tile &target) {
	auto flow_field = std::make_shared<FlowField>(this->cost_field->get_size());
	auto integrate_field = std::make_shared<IntegrationField>(this->cost_field->get_size());

	auto wavefront_blocked = integrate_field->integrate_los(this->cost_field, target);
	integrate_field->integrate_cost(this->cost_field, std::move(wavefront_blocked));
	flow_field->build(integrate_field);

	return flow_field;
}

} // namespace openage::path
