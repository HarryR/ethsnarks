#include "jubjub/fixed_base_mul_zcash.hpp"
#include "utils.hpp"


namespace ethsnarks {


bool test_jubjub_mul()
{
    jubjub::Params params;
    ProtoboardT pb;

    VariableArrayT scalar;
    scalar.allocate(pb, 252, "scalar");
    scalar.fill_with_bits_of_field_element(pb, FieldT("6453482891510615431577168724743356132495662554103773572771861111634748265227"));

    auto x = FieldT("13819220147556003423829648734536813647484299520101079752658527049348033428680");
    auto y = FieldT("18418392512101013735016656943391868405135207372553011567997823284229347734793");

    auto expected_x = FieldT("9475273318116948306485634257134587710963219450927319367756034463623580530136");
    auto expected_y = FieldT("1945841616097002480312205509604510190125303442992456923845187958701342969776");

    jubjub::fixed_base_mul_zcash the_gadget(pb, params, x, y, scalar, "the_gadget");

    the_gadget.generate_r1cs_witness();
    the_gadget.generate_r1cs_constraints();

    if( pb.val(the_gadget.result_x()) != expected_x ) {
        std::cerr << "x mismatch: ";
		pb.val(the_gadget.result_x()).print();
		std::cerr << std::endl;
        //return false;
    }

    if( pb.val(the_gadget.result_y()) != expected_y ) {
        std::cerr << "y mismatch: ";
		pb.val(the_gadget.result_y()).print();
		std::cerr<< std::endl;
        //return false;
    }

    std::cout << pb.num_constraints() << " constraints" << std::endl;

    return pb.is_satisfied();
}


// namespace ethsnarks
}


int main( int argc, char **argv )
{
    // Types for board 
    ethsnarks::ppT::init_public_params();

    if( ! ethsnarks::test_jubjub_mul() )
    {
        std::cerr << "FAIL\n";
        return 1;
    }

    std::cout << "OK\n";
    return 0;
}
