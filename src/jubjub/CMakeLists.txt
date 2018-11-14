include_directories(.)

add_library(ethsnarks_jubjub STATIC
  curve.cpp
  pedersen_commitment.cpp
  fixed_base_mul.cpp
  fixed_base_mul_zcash.cpp
  conditional_point.cpp
  scalarmult.cpp
  adder.cpp
  doubler.cpp
  isoncurve.cpp
)
