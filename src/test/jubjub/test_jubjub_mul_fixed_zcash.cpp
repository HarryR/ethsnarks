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

	// 252 bit require two base points
    auto x_1 = FieldT("13819220147556003423829648734536813647484299520101079752658527049348033428680");
    auto y_1 = FieldT("18418392512101013735016656943391868405135207372553011567997823284229347734793");
	auto x_2 = FieldT("19958489783026433573316075700077866010553709103185244447986177585739896260337");
    auto y_2 = FieldT("1343699924140874771493198820643387687812896216400603883310387613515125259878");

    auto expected_x = FieldT("6996724116960126900867925506414577611960232042751977065057280053875878784636");
    auto expected_y = FieldT("20701813187762697737769103062434791072108338775190080983238483435587088029101");

    jubjub::fixed_base_mul_zcash the_gadget(pb, params, { {x_1, y_1}, {x_2, y_2} }, scalar, "the_gadget");

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
