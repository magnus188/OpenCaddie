#pragma once

#include "domain/Types.h"

#include <optional>
#include <span>

namespace opencaddie::domain {

std::optional<ClubAdvice> recommendClub(std::span<const Club> clubs,
                                       double targetMetres,
                                       double recommendationBiasMetres = 0.0);

} // namespace opencaddie::domain

