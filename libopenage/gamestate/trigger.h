#ifndef TRIGGER_H
#define TRIGGER_H

#include <vector>
#include <cstdint>

#include "condition.h"
#include "action.h"

namespace openage {

	class Trigger
	{
		public:
			Trigger();
			~Trigger();

			enum class Gate { AND, OR, XOR };

			void update(uint32_t gametime,uint32_t update);

			uint32_t id      = 0;
			bool isActivated = true;
			bool isDeleted   = false;
			Gate gate        = Gate::OR;
			std::vector<Condition> conditions;
			std::vector<Action>    actions;
		private:
			bool check(uint32_t gametime,uint32_t update);
	};
}
#endif // TRIGGER_H
