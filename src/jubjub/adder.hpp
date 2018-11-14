#ifndef JUBJUB_ADDER_HPP_
#define JUBJUB_ADDER_HPP_

#include "jubjub/params.hpp"


namespace ethsnarks {

namespace jubjub {

class PointAdder : public GadgetT {
public:
    const Params& m_params;

    // First input point
    const VariableT m_X1;
    const VariableT m_Y1;

    // Second input point
    const VariableT m_X2;
    const VariableT m_Y2;

    // Intermediate variables
    const VariableT m_beta;
    const VariableT m_gamma;
    const VariableT m_delta;
    const VariableT m_epsilon;
    const VariableT m_tau;
    const VariableT m_X3;
    const VariableT m_Y3;

    PointAdder(
        ProtoboardT& in_pb,
        const Params& in_params,
        const VariableT in_X1,
        const VariableT in_Y1,
        const VariableT in_X2,
        const VariableT in_Y2,
        const std::string& annotation_prefix
    );

    const VariableT& result_x() const;

    const VariableT& result_y() const;

    void generate_r1cs_constraints();

    void generate_r1cs_witness();
};

namespace montgomery {

class PointAdder : public GadgetT {
public:
    const Params& m_params;

    // First input point
    const VariableT m_X1;
    const VariableT m_Y1;

    // Second input point
    const VariableT m_X2;
    const VariableT m_Y2;

    // Intermediate variables
    const VariableT lambda;
    const VariableT m_X3;
    const VariableT m_Y3;

    PointAdder(
        ProtoboardT& in_pb,
        const Params& in_params,
        const VariableT in_X1,
        const VariableT in_Y1,
        const VariableT in_X2,
        const VariableT in_Y2,
        const std::string& annotation_prefix
    );

    const VariableT& result_x() const;

    const VariableT& result_y() const;

    void generate_r1cs_constraints();

    void generate_r1cs_witness();
};

// namespace montgomery
}

// namespace jubjub
}

// namespace ethsnarks
}

// JUBJUB_ADDER_HPP_
#endif
