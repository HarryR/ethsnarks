/*    
    copyright 2018 to the baby_jubjub_ecc Authors

    This file is part of baby_jubjub_ecc.

    baby_jubjub_ecc is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    baby_jubjub_ecc is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with baby_jubjub_ecc.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef JUBJUB_CURVE_HPP_
#define JUBJUB_CURVE_HPP_

#include <cassert>
#include <memory>

#include "ethsnarks.hpp"

#include "jubjub/adder.hpp"
#include "jubjub/params.hpp"

namespace ethsnarks {

namespace jubjub {


class isOnCurve : public GadgetT {
private:
    const Params &m_params;

public:
    const VariableT x;
    const VariableT y;

    //intermeditate variables 
    const VariableT xx;
    const VariableT yy;
    const VariableT lhs;
    const VariableT rhs;

    std::string annotation_prefix = "isonCurve";

    isOnCurve(ProtoboardT &pb,
              const Params &in_params,
              const VariableT &in_x, const VariableT &in_y,
              const std::string &annotation_prefix);

    void generate_r1cs_constraints();
    void generate_r1cs_witness();

};


class pointAddition : public GadgetT {
//greater than gadget
private:
    /* no internal variables */
    const Params& m_params;

public:
    VariableT a;
    VariableT d;

    std::shared_ptr<isOnCurve> jubjub_isOnCurve;

    // Inputs
    VariableT x1;
    VariableT y1;
    VariableT x2;
    VariableT y2;

    // Outputs
    VariableT x3;
    VariableT y3;

    //intermeditate variables
    VariableT x1x2;
    VariableT x1y2;
    VariableT y1y2;
    VariableT y1x2;
    VariableT x1x2y1y2;
    VariableT dx1x2y1y2;
    VariableT ax1x2;

    std::string annotation_prefix = "point Addition ";


    pointAddition(ProtoboardT &pb,
                    const Params& in_params,
                   /*const pb_linear_combination_array<FieldT> &bits,*/
                   const VariableT &a, const VariableT &d,

                   const VariableT &x1, const VariableT &y1,
                   const VariableT &x2, const VariableT &y2,
                   const VariableT &x3, const VariableT &y3,

                   const std::string &annotation_prefix);

    void generate_r1cs_constraints();
    void generate_r1cs_witness();
};


class conditionalPointAddition : public GadgetT {
//greater than gadget
private:
    /* no internal variables */
    const Params& m_params;

public:
    VariableT a;
    VariableT d;


    //input variables 
    VariableT x1;
    VariableT y1;
    VariableT x2;
    VariableT y2;
    VariableT x3;
    VariableT y3;
    VariableT canAdd;

    //intermediate variables
    VariableT x_toAdd;
    VariableT y_toAdd;
    VariableT y_intermediate_toAdd1;
    VariableT y_intermediate_toAdd2;
    VariableT not_canAdd;


    std::string annotation_prefix = "conditioanl adiditon";

    std::shared_ptr<pointAddition> jubjub_pointAddition;


    conditionalPointAddition(ProtoboardT &pb,
                    const Params& in_params,
                   const VariableT &a, const VariableT &d,
                   const VariableT &x1, const VariableT &y1,
                   const VariableT &x2, const VariableT &y2,
                   const VariableT &x3, const VariableT &y3,
                   const VariableT &canAdd, const std::string &_annotation_prefix);

    void generate_r1cs_constraints();
    void generate_r1cs_witness();
};


class pointMultiplication : public GadgetT {
//greater than gadget
private:
    /* no internal variables */
public:
    VariableT a;
    VariableT d;

    VariableT x;
    VariableT y;

    VariableArrayT coef;

    // outputs
    VariableArrayT x_ret;
    VariableArrayT y_ret;

    int coef_size; //coeffient size

    //intermeditate variables
    VariableT x_zero;
    VariableT y_zero;


    std::shared_ptr<isOnCurve> jubjub_isOnCurve;

    // store the result of the current iteration
    VariableArrayT x_intermediary;
    VariableArrayT y_intermediary;


    std::vector<std::shared_ptr<pointAddition > > doub; //double
    std::vector<std::shared_ptr<conditionalPointAddition > > add; //double

    pointMultiplication(ProtoboardT &pb,
                   const Params& in_params,
                   const VariableT &a, const VariableT &d,
                   const VariableT &x_base, const VariableT &y_base,
                   const VariableArrayT &coef, const VariableArrayT x_ret,
                   const VariableArrayT y_ret, const std::string &annotation_prefix, 
                   int coef_size);

    void generate_r1cs_constraints();
    void generate_r1cs_witness();
};

namespace montgomery {

class EdwardConversion : public GadgetT {
public:
    const Params& m_params;
    
    // Input point
    const VariableT m_X1;
    const VariableT m_Y1;

    // Output point
    const VariableT m_X2;
    const VariableT m_Y2;

    EdwardConversion(
        ProtoboardT &in_pb,
        const Params& in_params,
        const VariableT in_X,
        const VariableT in_Y,
        const std::string &annotation_prefix
    );

    const VariableT& result_x() const;
    const VariableT& result_y() const;

    void generate_r1cs_constraints();
    void generate_r1cs_witness();
}; 
}

// namespace jubjub
}

// namespace ethsnarks
}

// JUBJUB_CURVE_HPP_
#endif
