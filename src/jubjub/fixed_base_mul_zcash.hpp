#ifndef JUBJUB_FIXEDMULT_HPP_
#define JUBJUB_FIXEDMULT_HPP_

#include "gadgets/lookup_2bit.hpp"
#include "gadgets/lookup_signed_3bit.hpp"
#include "jubjub/adder.hpp"
#include "jubjub/curve.hpp"

namespace ethsnarks {

namespace jubjub {

struct Point {
	FieldT x;
	FieldT y;
};

class fixed_base_mul_zcash : public GadgetT {
public:
	const VariableArrayT m_scalar;

	std::vector<montgomery::PointAdder> montgomery_adders;
	std::vector<montgomery::EdwardConversion> point_converters;
	std::vector<PointAdder> edward_adders;
	std::vector<lookup_2bit_gadget> m_windows_x;
	std::vector<lookup_signed_3bit_gadget> m_windows_y;

	fixed_base_mul_zcash(
		ProtoboardT &in_pb,
		const Params& in_params,
		const std::vector<Point> base_points,
		const VariableArrayT in_scalar,
		const std::string &annotation_prefix
	);

	void generate_r1cs_constraints ();

	void generate_r1cs_witness ();

	const VariableT& result_x();

	const VariableT& result_y();
};

// namespace jubjub
}

// namespace ethsnarks
}

// JUBJUB_FIXEDMULT_HPP_
#endif
