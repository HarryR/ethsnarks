// Copyright (c) 2018 fleupold
// License: LGPL-3.0+

#include "lookup_signed_3bit.hpp"

#include "utils.hpp"

namespace ethsnarks {

void lookup_signed_3bit_constraints( ProtoboardT& pb, const std::vector<FieldT> c, const VariableArrayT b, const VariableT r, const VariableT b0b1, const std::string& annotation_prefix )
{
	// b0b1 = b[0] * b[1]
	pb.add_r1cs_constraint(
        ConstraintT(b[0], b[1], b0b1),
            FMT(annotation_prefix, ".result"));

	// y_lc = c[0] + b[0] * (c[1]-c0) + b[1] * (c[2]-c[0]) + b[0]&b[1] * (c[3] - c[2] - c[1] + c[0])
	LinearCombinationT y_lc;
	y_lc.assign(pb, LinearTermT(libsnark::ONE, c[0]) +
				   	LinearTermT(b[0], c[1] - c[0]) +
				   	LinearTermT(b[1], c[2] - c[0]) +
				   	LinearTermT(b0b1, c[3] - c[2] - c[1] + c[0])
				   	);

	// (y_lc + y_lc) * b[2] == y_lc - r
    pb.add_r1cs_constraint(
        ConstraintT({y_lc + y_lc}, b[2], {y_lc - r}),
            FMT(annotation_prefix, ".result"));
}


lookup_signed_3bit_gadget::lookup_signed_3bit_gadget(
	ProtoboardT &in_pb,
	const std::vector<FieldT> in_constants,
	const VariableArrayT in_bits,
	const std::string& annotation_prefix
) :
	GadgetT(in_pb, annotation_prefix),
	c(in_constants),
	b(in_bits),
	b0b1(make_variable(in_pb, FMT(this->annotation_prefix, "b0 AND b1"))),
	r(make_variable(in_pb, FMT(this->annotation_prefix, ".r")))
{
	assert( in_constants.size() == 4 );
	assert( b.size() == 3 );
}


const VariableT& lookup_signed_3bit_gadget::result()
{
	return r;
}


void lookup_signed_3bit_gadget::generate_r1cs_constraints()
{
	lookup_signed_3bit_constraints(this->pb, c, b, r, b0b1, this->annotation_prefix);
}


void lookup_signed_3bit_gadget::generate_r1cs_witness ()
{
    auto i = b.get_field_element_from_bits(this->pb).as_ulong();
	FieldT result = c[i&3];
	if (i > 3) {
		result *= -1;
	}
	this->pb.val(b0b1) = this->pb.val(b[0]) * this->pb.val(b[1]);
    this->pb.val(r) = result;
}

// namespace ethsnarks
}
