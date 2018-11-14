#ifndef JUBJUB_FIXEDMULT_HPP_
#define JUBJUB_FIXEDMULT_HPP_

#include "gadgets/lookup_2bit.hpp"
#include "gadgets/lookup_signed_3bit.hpp"
#include "jubjub/adder.hpp"


namespace ethsnarks {

namespace jubjub {


/**
* Implements scalar multiplication using a fixed base point and a 2-bit lookup window 
*
*     input bits 0...n   with 2 bit lookup  windows
*   +--------+----------+------------+-----------+---....
*   |  0-2   |    3-4   |    5-6     |    7-8    |   9-.
*   +--------+----------+------------+-----------+---....
*      | |        | |        | |          | |         | |
*   +------+   +------+    +------+     +------+     +...
*   | LUT0 |   | LUT1 |    | LUT2 |     | LUTn |     |
*   +------+   +------+    +------+     +------+     +...
*       |          |          |            |            |
*   +-----------------+   +---------+  +---------+   +...
*   |      ADDER 0    |---| ADDER 1 |--| ADDER n |---|
*   +-----------------+   +---------+  +---------+   +...
*
*/
class fixed_base_mul_zcash : public GadgetT {
public:
	const VariableArrayT m_scalar;

	std::vector<PointAdder> m_adders;
	std::vector<lookup_signed_3bit_gadget> m_windows_x;
	std::vector<lookup_2bit_gadget> m_windows_y;

	fixed_base_mul_zcash(
		ProtoboardT &in_pb,
		const Params& in_params,
		const FieldT& in_base_x,
		const FieldT& in_base_y,
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
