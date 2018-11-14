#ifndef ETHSNARKS_LOOKUP_SIGNED_3BIT_HPP_
#define ETHSNARKS_LOOKUP_SIGNED_3BIT_HPP_

// Copyright (c) 2018 fleupold
// License: LGPL-3.0+

#include "ethsnarks.hpp"

namespace ethsnarks {

/**
* Three-bit window lookup (2bits + signature bit) in 2bit table
* using two constraints. Maps the bits `b` to a list of constants `c`
*/
class lookup_signed_3bit_gadget : public GadgetT
{
public:
    const std::vector<FieldT> c;
    const VariableArrayT b;
	const VariableT b0b1;
    VariableT r;

    lookup_signed_3bit_gadget(
        ProtoboardT &in_pb,
        const std::vector<FieldT> in_constants,
        const VariableArrayT in_bits,
        const std::string& annotation_prefix
    );

    const VariableT& result();

    void generate_r1cs_constraints();

    void generate_r1cs_witness ();
};


// namespace ethsnarks
}

// ETHSNARKS_LOOKUP_SIGNED_3BIT_HPP_
#endif
