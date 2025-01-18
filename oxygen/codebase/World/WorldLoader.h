#pragma once

namespace oxygen
{
	auto LoadWorld(std::string_view name) -> std::shared_ptr<struct World>;
}; // namespace oxygen