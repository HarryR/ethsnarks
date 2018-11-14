#include "jubjub/adder.hpp"
#include "utils.hpp"


namespace ethsnarks {

namespace jubjub {

PointAdder::PointAdder(
    ProtoboardT& in_pb,
    const Params& in_params,
    const VariableT in_X1,
    const VariableT in_Y1,
    const VariableT in_X2,
    const VariableT in_Y2,
    const std::string &annotation_prefix
) :
    GadgetT(in_pb, annotation_prefix),
    m_params(in_params),
    m_X1(in_X1), m_Y1(in_Y1),
    m_X2(in_X2), m_Y2(in_Y2),
    m_beta(make_variable(in_pb, FMT(annotation_prefix, ".beta"))),
    m_gamma(make_variable(in_pb, FMT(annotation_prefix, ".gamma"))),
    m_delta(make_variable(in_pb, FMT(annotation_prefix, ".delta"))),
    m_epsilon(make_variable(in_pb, FMT(annotation_prefix, ".epsilon"))),
    m_tau(make_variable(in_pb, FMT(annotation_prefix, ".tau"))),
    m_X3(make_variable(in_pb, FMT(annotation_prefix, ".X3"))),
    m_Y3(make_variable(in_pb, FMT(annotation_prefix, ".Y3")))
{}

void PointAdder::generate_r1cs_constraints()
{
    this->pb.add_r1cs_constraint(
        ConstraintT(m_X1, m_Y2, m_beta),
            FMT(annotation_prefix, ".beta = X1 * Y2"));

    this->pb.add_r1cs_constraint(
        ConstraintT(m_Y1, m_X2, m_gamma),
            FMT(annotation_prefix, ".gamma = Y1 * X2"));

    this->pb.add_r1cs_constraint(
        ConstraintT(m_Y1, m_Y2, m_delta),
            FMT(annotation_prefix, ".delta = Y1 * Y2"));

    this->pb.add_r1cs_constraint(
        ConstraintT(m_X1, m_X2, m_epsilon),
            FMT(annotation_prefix, ".epsilon = X1 * X2"));

    this->pb.add_r1cs_constraint(
        ConstraintT(m_delta, m_epsilon, m_tau),
            FMT(annotation_prefix, ".tau = delta * epsilon"));

    this->pb.add_r1cs_constraint(
        ConstraintT(m_X3, 1 + (m_params.d*m_tau), m_beta + m_gamma),
            FMT(annotation_prefix, ".x3 * (1 + (d*tau)) == (beta + gamma) "));

    this->pb.add_r1cs_constraint(
        ConstraintT(m_Y3, 1 - (m_params.d*m_tau), m_delta + ((-m_params.a)*m_epsilon)),
            FMT(annotation_prefix, ".y3 * (1 - (d*tau)) == (delta - a*epsilon) "));
}

const VariableT& PointAdder::result_x() const
{
    return m_X3;
}

const VariableT& PointAdder::result_y() const
{
    return m_Y3;
}

void PointAdder::generate_r1cs_witness()
{
    auto one = FieldT("1");

    this->pb.val(m_beta) = this->pb.val(m_X1) * this->pb.val(m_Y2);

    this->pb.val(m_gamma) = this->pb.val(m_Y1) * this->pb.val(m_X2);

    this->pb.val(m_delta) = this->pb.val(m_Y1) * this->pb.val(m_Y2);

    this->pb.val(m_epsilon) = this->pb.val(m_X1) * this->pb.val(m_X2);

    this->pb.val(m_tau) = this->pb.val(m_delta) * this->pb.val(m_epsilon);

    auto x3_rhs = (one + (m_params.d * this->pb.val(m_tau))).inverse();
    this->pb.val(m_X3) = (this->pb.val(m_beta)+this->pb.val(m_gamma)) * x3_rhs;

    auto y3_rhs = (one - (m_params.d * this->pb.val(m_tau))).inverse();
    this->pb.val(m_Y3) = (this->pb.val(m_delta)+( -m_params.a * this->pb.val(m_epsilon))) * y3_rhs;
}

namespace montgomery {

PointAdder::PointAdder(
    ProtoboardT& in_pb,
    const Params& in_params,
    const VariableT in_X1,
    const VariableT in_Y1,
    const VariableT in_X2,
    const VariableT in_Y2,
    const std::string &annotation_prefix
) :
    GadgetT(in_pb, annotation_prefix),
    m_params(in_params),
    m_X1(in_X1), m_Y1(in_Y1),
    m_X2(in_X2), m_Y2(in_Y2),
    lambda(make_variable(in_pb, FMT(annotation_prefix, ".lambda"))),
    m_X3(make_variable(in_pb, FMT(annotation_prefix, ".X3"))),
    m_Y3(make_variable(in_pb, FMT(annotation_prefix, ".Y3")))
{}

void PointAdder::generate_r1cs_constraints()
{
    this->pb.add_r1cs_constraint(
        ConstraintT({m_X2 - m_X1}, lambda, {m_Y2 - m_Y1}),
            FMT(annotation_prefix, ".lambda = (y' - y) / (x' - x)"));
    this->pb.add_r1cs_constraint(
        ConstraintT(lambda, lambda, {m_params.A + m_X1 + m_X2 + m_X3}),
            FMT(annotation_prefix, ".(lambda) * (lambda) = (A + x + x' + x'')"));
    this->pb.add_r1cs_constraint(
        ConstraintT({m_X1 - m_X3}, lambda, {m_Y3 + m_Y1}),
            FMT(annotation_prefix, ".y'' = -(y + lambda(x'' - x))"));
}

const VariableT& PointAdder::result_x() const
{
    return m_X3;
}

const VariableT& PointAdder::result_y() const
{
    return m_Y3;
}

void PointAdder::generate_r1cs_witness()
{
    this->pb.val(lambda) = (this->pb.val(m_Y2) - this->pb.val(m_Y1)) * (this->pb.val(m_X2) - this->pb.val(m_X1)).inverse();
    this->pb.val(m_X3) = this->pb.val(lambda).squared() - m_params.A - this->pb.val(m_X1) - this->pb.val(m_X2);
    this->pb.val(m_Y3) = -(this->pb.val(m_Y1) + (this->pb.val(lambda)*(this->pb.val(m_X3) - this->pb.val(m_X1))));
}

// namespace montgomery
}

// namespace jubjub
}

// namespace ethsnarks
}
