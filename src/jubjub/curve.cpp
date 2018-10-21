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


#include "jubjub/curve.hpp"
#include "utils.hpp"

namespace ethsnarks
{


isOnCurve::isOnCurve(ProtoboardT &pb,
                   /*const pb_linear_combination_array<FieldT> &bits,*/
                   const VariableT &x, const VariableT &y,
                   const VariableT &a, const VariableT &d,
                   const std::string &annotation_prefix):
        GadgetT(pb, annotation_prefix), x(x), y(y), a(a), d(d) 
{
    xx.allocate(this->pb, "xx");
    yy.allocate(this->pb, "yy");
    dxx.allocate(this->pb, "dxx");
    axx.allocate(this->pb, "axx");
    dxxyy.allocate(this->pb, "dxxyy");
    lhs.allocate(this->pb, "lhs");
    rhs.allocate(this->pb, "rhs");
 

}



void isOnCurve::generate_r1cs_constraints()
{
    // checks that a*x*x + y*y = 1 + d*x*x*y*y
    // where x , y are curve points
    // and a , d are curve parameters.

    this->pb.add_r1cs_constraint(ConstraintT({x} , {x}, {xx}),
                           FMT("find x*x", ""));
    this->pb.add_r1cs_constraint(ConstraintT({y} , {y}, {yy}),
                           FMT("find y*y", ""));
    this->pb.add_r1cs_constraint(ConstraintT({a} , {xx}, {axx}),
                           FMT( annotation_prefix, "find a*x*x"));
    this->pb.add_r1cs_constraint(ConstraintT({axx, yy} , {1}, {lhs}),
                           FMT("find lhs", ""));
    this->pb.add_r1cs_constraint(ConstraintT({d} , {xx}, {dxx}),
                           FMT("find dxx", ""));
    this->pb.add_r1cs_constraint(ConstraintT({dxx} , {yy}, {dxxyy}),
                           FMT("find rhs", ""));
    this->pb.add_r1cs_constraint(ConstraintT({dxxyy , 1} , {1}, {rhs}),
                           FMT("find rhs", ""));
    this->pb.add_r1cs_constraint(ConstraintT({lhs} , {1}, {rhs}),
                           FMT("lhs == rhs", ""));
}


void isOnCurve::generate_r1cs_witness()
{
    //linear_combination<FieldT> lc; 


    FieldT _x = this->pb.lc_val(this->x);
    FieldT _y = this->pb.lc_val(this->y);
    FieldT _a = this->pb.lc_val(this->a);
    FieldT _d = this->pb.lc_val(this->d);

    this->pb.val(x) = FieldT(_x); 
    this->pb.val(y) = FieldT(_y);
    this->pb.val(a) = FieldT(_a);
    this->pb.val(d) = FieldT(_d);

    this->pb.val(xx) = FieldT(_x*_x);
    this->pb.val(yy) = FieldT(_y*_y);
    this->pb.val(axx) = FieldT(_a*_x*_x);
    this->pb.val(lhs) = FieldT(_a*_x*_x + _y*_y);
    this->pb.val(dxx) = FieldT(_x*_x*_d);
    this->pb.val(dxxyy) = FieldT(_d*_x*_x*_y*_y);

    this->pb.val(rhs) = FieldT(_d*_x*_x*_y*_y + 1); 
}


FasterPointAddition::FasterPointAddition(
    ProtoboardT &in_pb,
    const jubjub_params &in_params,
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
    m_beta(make_variable(in_pb, FMT(this->annotation_prefix, ".beta"))),
    m_gamma(make_variable(in_pb, FMT(this->annotation_prefix, ".gamma"))),
    m_delta(make_variable(in_pb, FMT(this->annotation_prefix, ".delta"))),
    m_epsilon(make_variable(in_pb, FMT(this->annotation_prefix, ".epsilon"))),
    m_tau(make_variable(in_pb, FMT(this->annotation_prefix, ".tau"))),
    m_X3(make_variable(in_pb, FMT(this->annotation_prefix, ".X3"))),
    m_Y3(make_variable(in_pb, FMT(this->annotation_prefix, ".Y3")))
{

}


void FasterPointAddition::generate_r1cs_constraints()
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
        ConstraintT(m_delta, m_delta, m_tau),
            FMT(annotation_prefix, ".tau = delta * epsilon"));

    this->pb.add_r1cs_constraint(
        ConstraintT(m_X3, 1 + (m_params.d*m_tau), m_beta + m_gamma),
            FMT(annotation_prefix, ".x3 * (1 + (d*tau)) == (beta + gamma) "));

    this->pb.add_r1cs_constraint(
        ConstraintT(m_Y3, 1 - (m_params.d*m_tau), m_delta + ((-m_params.a)*m_epsilon)),
            FMT(annotation_prefix, ".y3 * (1 - (d*tau)) == (delta + a*epsilon) "));
}

void FasterPointAddition::generate_r1cs_witness()
{
    auto one = FieldT("1");

    this->pb.val(m_beta) = this->pb.val(m_X1) * this->pb.val(m_Y2);

    this->pb.val(m_gamma) = this->pb.val(m_Y1) * this->pb.val(m_X2);

    this->pb.val(m_delta) = this->pb.val(m_Y1) * this->pb.val(m_Y2);

    this->pb.val(m_epsilon) = this->pb.val(m_X1) * this->pb.val(m_X2);

    auto x3_rhs = (one + (m_params.d * this->pb.val(m_tau))).inverse();
    this->pb.val(m_X3) = (this->pb.val(m_beta)+this->pb.val(m_gamma)) * x3_rhs;

    auto y3_rhs = (one - (m_params.d * this->pb.val(m_tau))).inverse();
    this->pb.val(m_Y3) = (this->pb.val(m_delta)+( -m_params.a * this->pb.val(m_epsilon))) * y3_rhs;
}


pointAddition::pointAddition(
    ProtoboardT &pb,
    /*const pb_linear_combination_array<FieldT> &bits,*/
    const VariableT &a, const VariableT &d,
    const VariableT &x1, const VariableT &y1,
    const VariableT &x2, const VariableT &y2,
    const VariableT &x3, const VariableT &y3,
    const std::string &annotation_prefix
) :
    GadgetT(pb, annotation_prefix),
    a(a),
    d(d),
    x1(x1), y1(y1),
    x2(x2), y2(y2),
    x3(x3), y3(y3)
{


    jubjub_isOnCurve.reset( new isOnCurve (this->pb, x3, y3, a, d, "modulo"));
    x1x2.allocate(this->pb, "x1x2");
    x1y2.allocate(this->pb, "x1y2");
    y1y2.allocate(this->pb, "y1y2");
    y1x2.allocate(this->pb, "y1x2");
    x1x2y1y2.allocate(this->pb, "x1x2y1y2");
    dx1x2y1y2.allocate(this->pb, "dx1x2y1y2");
    ax1x2.allocate(this->pb, "ax1x2");
}


void pointAddition::generate_r1cs_constraints()
{
    // checks that   
    //  x3 = (x1*y2 + y1*x2) / (Fq.ONE + JUBJUB_D*x1*x2*y1*y2)
    //  y3 = (y1*y2 - JUBJUB_A*x1*x2) / (Fq.ONE - JUBJUB_D*x1*x2*y1*y2)
  
    // where x , y are curve points
    // and a , d are curve parameters.
    this->pb.add_r1cs_constraint(ConstraintT({y1} , {x2}, {y1x2}),
                           FMT("find y1*x2 == y1x2", ""));
    this->pb.add_r1cs_constraint(ConstraintT({x1} , {y2}, {x1y2}),
                           FMT("find x1y2 == x1y2", ""));
    this->pb.add_r1cs_constraint(ConstraintT({x1} , {x2}, {x1x2}),
                           FMT("x1*x2 == x1x2", ""));
    this->pb.add_r1cs_constraint(ConstraintT({y1} , {y2}, {y1y2}),
                           FMT(annotation_prefix, "find y1*y2= y1y2"));
    this->pb.add_r1cs_constraint(ConstraintT({x1x2} , {y1y2}, {x1x2y1y2}),
                           FMT("find lhs", ""));
    this->pb.add_r1cs_constraint(ConstraintT({d} , {x1x2y1y2}, {dx1x2y1y2}),
                           FMT("confirm dx1x2y1y2", ""));
    this->pb.add_r1cs_constraint(ConstraintT({a} , {x1x2}, {ax1x2}),
                           FMT("confirm ax1x2", ""));
    this->pb.add_r1cs_constraint(ConstraintT({y3} , {1, -dx1x2y1y2}  ,  {y1y2 , -ax1x2}),
                           FMT("confirm y3", ""));
    this->pb.add_r1cs_constraint(ConstraintT({x3} , {1, dx1x2y1y2}, {x1y2, y1x2}),
                           FMT("confirm x3", ""));



    //jubjub_isOnCurve->generate_r1cs_constraints();
}


void pointAddition::generate_r1cs_witness()
{
    //linear_combination<FieldT> lc; 

    FieldT _x1 = this->pb.lc_val(this->x1); 
    FieldT _y1 = this->pb.lc_val(this->y1); 
    FieldT _x2 = this->pb.lc_val(this->x2);
    FieldT _y2 = this->pb.lc_val(this->y2);
    FieldT _a = this->pb.lc_val(this->a);
    FieldT _d = this->pb.lc_val(this->d);


    this->pb.val(x1x2) = FieldT(_x1*_x2);
    this->pb.val(x1y2) = FieldT(_x1*_y2);
    this->pb.val(y1y2) = FieldT(_y1*_y2);
    this->pb.val(y1x2) = FieldT(_y1*_x2);
    this->pb.val(x1x2y1y2) = FieldT(_x1*_x2*_y1*_y2);
    this->pb.val(dx1x2y1y2) = FieldT(_d*_x1*_x2*_y1*_y2);
    this->pb.val(ax1x2) = FieldT(_a*_x1*_x2);


    /* 
    //DEBUG
    std::cout << " x3     "  << _d <<  " " << _y1 << " " << _y2 << " " << _x1 << " " << _x2 << " " <<  " " << (FieldT(1) - (_d*_x1*_x2*_y1*_y2))  << std::endl ;
    std::cout <<"  lhs    " << FieldT(_x1*_y2 + _y1*_x2) << std::endl;
    std::cout <<"  rhs    " << FieldT((FieldT(1) +  (_d*_x1*_x2*_y1*_y2)).inverse()) << std::endl;
    std::cout <<"  x3    " << FieldT(_x1*_y2 + _y1*_x2) * FieldT((FieldT(1) +  (_d*_x1*_x2*_y1*_y2)).inverse()) << std::endl;
    std::cout << FieldT(_x1*_y2 + _y1*_x2) * FieldT((FieldT(1) +  (_d*_x1*_x2*_y1*_y2)).inverse()) * FieldT((FieldT(1) +  (_d*_x1*_x2*_y1*_y2))) << std::endl;
    */  

    this->pb.val(x3) = FieldT(_x1*_y2 + _y1*_x2) * FieldT((FieldT(1) +  (_d*_x1*_x2*_y1*_y2)).inverse()); 
    this->pb.val(y3) = FieldT(_y1*_y2 - _a*_x1*_x2) * FieldT((FieldT(1) -  (_d*_x1*_x2*_y1*_y2)).inverse());

    //jubjub_isOnCurve->generate_r1cs_witness(x3, y3, &_a_str[0u] , &_d_str[0u]);



}



conditionalPointAddition::conditionalPointAddition(ProtoboardT &pb,
                   /*const pb_linear_combination_array<FieldT> &bits,*/
                   const VariableT &a, const VariableT &d,
                   const VariableT &x1, const VariableT &y1,
                   const VariableT &x2, const VariableT &y2,
                   const VariableT &x3, const VariableT &y3,
                   const VariableT &canAdd, const std::string &_annotation_prefix):
        GadgetT(pb, _annotation_prefix) , a(a), d(d) , x1(x1), y1(y1), x2(x2), y2(y2), x3(x3), y3(y3), canAdd(canAdd)
{

      
    x_toAdd.allocate(this->pb, "x_toAdd");
    y_toAdd.allocate(this->pb, "y_toAdd");

    y_intermediate_toAdd1.allocate(this->pb, "y_intermatied_toAdd1");
    y_intermediate_toAdd2.allocate(this->pb, "y_intermatied_toAdd2");

    not_canAdd.allocate(this->pb, "not_canAdd");

    jubjub_pointAddition.reset( new pointAddition (this->pb, a, d, x1, y1, x_toAdd , y_toAdd , x3, y3, "x1, y1 + x2 , y2"));

}


void conditionalPointAddition::generate_r1cs_constraints()
{
    // if coef == 1 then x_ret[i] + x_base 
    //x_add[i] = coef[i] * x_base;
    this->pb.add_r1cs_constraint(ConstraintT({x2} , {canAdd}, {x_toAdd}),
                           FMT("add 0 if coef == 0 else x_base", ""));

    // else do nothing. Ie add the zero point (0, 1)
    //y_add[i] = coef[i] * y_base + !coef[i];    
    this->pb.add_r1cs_constraint(ConstraintT({y2} , {canAdd}, {y_intermediate_toAdd1}),
                          FMT("add 1 if coef == 0 else y_base", ""));
    
    //not coef
    // make sure canAdd == 0 or canAdd == 1
    this->pb.add_r1cs_constraint(ConstraintT(canAdd, 1-canAdd, 0),
                           FMT(annotation_prefix, " boolean_r1cs_constraint canAdd"));
     // make sure not_canAdd == 0 or not_canAdd == 1
    this->pb.add_r1cs_constraint(ConstraintT(not_canAdd, 1-not_canAdd, 0),
                           FMT(annotation_prefix, " boolean_r1cs_constraint not_canAdd"));
    // make sure that the sum of canAdd, not_canAdd == 1 which means canAdd!=not_canAdd
    this->pb.add_r1cs_constraint(ConstraintT({not_canAdd, canAdd}, {1}, {1}),
                           FMT(annotation_prefix, " not_canAdd == ! canAdd"));

    // because the are bool and because they are not equal we know that the inverse of one
    // is the other. 

    this->pb.add_r1cs_constraint(ConstraintT({not_canAdd} , {1}, {y_intermediate_toAdd2}),
                          FMT("add 0 if coef == 0 else x_base", ""));

    this->pb.add_r1cs_constraint(ConstraintT({y_intermediate_toAdd1, y_intermediate_toAdd2} , {1}, {y_toAdd}),
                          FMT("add 1 if coef == 0 else y_base", ""));
    // do the addition of either y1 , y1 plus x2, y2 if canAdd == true else x1 , y1 + 0
    jubjub_pointAddition->generate_r1cs_constraints();    
}


void conditionalPointAddition::generate_r1cs_witness()
{   
    this->pb.val(x_toAdd) = this->pb.lc_val(this->x2) * this->pb.lc_val(this->canAdd);
    this->pb.val(this->y_intermediate_toAdd1) = this->pb.lc_val(this->y2) * this->pb.lc_val(this->canAdd);

    if (this->pb.lc_val(this->canAdd) == 1) {
        this->pb.val(this->not_canAdd) = FieldT(0);
        this->pb.val(this->y_intermediate_toAdd2) = this->pb.lc_val(this->not_canAdd) * FieldT(1);
        this->pb.val(this->y_toAdd) = FieldT(this->pb.lc_val(this->y_intermediate_toAdd1));
    } else {
        this->pb.val(this->not_canAdd) = FieldT(1);
        this->pb.val(this->y_intermediate_toAdd2) = this->pb.lc_val(this->not_canAdd) * FieldT(1);
        this->pb.val(this->y_toAdd) = FieldT(1);//this->pb.lc_val(this->y_intermediate_toAdd2));
    }
    jubjub_pointAddition->generate_r1cs_witness();  

}


pointMultiplication::pointMultiplication(ProtoboardT &pb,
                   /*const pb_linear_combination_array<FieldT> &bits,*/
                   const VariableT &a, const VariableT &d,
                   const VariableT &x, const VariableT &y,
                   const VariableArrayT &coef, const VariableArrayT x_ret, 
                   const VariableArrayT y_ret, const std::string &annotation_prefix, 
                   int coef_size):
        GadgetT(pb, annotation_prefix) , a(a), d(d) , x(x), y(y), coef(coef), x_ret(x_ret), y_ret(y_ret), coef_size(coef_size)
{

    //performs point multiplicaion using double and add method
    this->doub.resize(coef_size);
    this->add.resize(coef_size);

    x_zero.allocate(pb, "x_zero");
    y_zero.allocate(pb, "y_zero");

    x_intermediary.allocate(pb, coef_size, FMT("annotation_prefix", " x intermediary"));
    y_intermediary.allocate(pb, coef_size, FMT("annotation_prefix", " y intermediary"));

    pb.val(x_zero) = FieldT("0");
    pb.val(y_zero) = FieldT("1");

    this->doub[0].reset( new pointAddition (this->pb, a, d, x_zero, y_zero , x_zero , y_zero, x_intermediary[0], y_intermediary[0], "x1, y1 + x2 , y2"));
    this->add[0].reset( new conditionalPointAddition (this->pb, a, d, x_intermediary[0], y_intermediary[0] , x, y, x_ret[0], y_ret[0], coef[0], "x1, y1 + x2 , y2"));
    //boolean constrain coef[0]
    this->pb.add_r1cs_constraint(ConstraintT(coef[0], 1-coef[0], 0),
                           FMT(annotation_prefix, " boolean_r1cs_constraint canAdd"));

    for(int i=1; i<coef_size; i++) { 
        //boolean constarin coef[i]       
        this->pb.add_r1cs_constraint(ConstraintT(coef[i], 1-coef[i], 0),
                       FMT(annotation_prefix, " boolean_r1cs_constraint canAdd"));
        //double
        this->doub[i].reset( new pointAddition (this->pb, a, d, x_ret[i-1], y_ret[i-1] , x_ret[i-1] , y_ret[i-1], x_intermediary[i], y_intermediary[i], "x1, y1 + x2 , y2"));
        //add
        this->add[i].reset( new conditionalPointAddition (this->pb, a, d, x_intermediary[i], y_intermediary[i] , x, y, x_ret[i], y_ret[i], coef[i], "x1, y1 + x2 , y2"));
    }
}


void pointMultiplication::generate_r1cs_constraints()
{
    for(int i=0; i<coef_size; i++) {

       this->doub[i]->generate_r1cs_constraints();
       this->add[i]->generate_r1cs_constraints();   
    }
}


void pointMultiplication::generate_r1cs_witness()
{
    for(int i=0; i<coef_size; i++) {
        this->doub[i]->generate_r1cs_witness();
        this->add[i]->generate_r1cs_witness();
        /*
        //debug
        std::cout << i << " i  " << this->pb.lc_val(coef[i]) << std::endl;
        std::cout << this->pb.lc_val(x_ret[i]) << " output " << this->pb.lc_val(y_ret[i]) << std::endl;
        std::cout << this->pb.lc_val(x_intermediary[i]) << " intermediary " << this->pb.lc_val(y_intermediary[i]) << std::endl;
        */
    }
}


// namespace ethsnarks
}
